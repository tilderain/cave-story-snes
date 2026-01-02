#!/usr/bin/env python3
"""
Convert data.asmr (ASM file with binary includes) to C const unsigned char arrays.
Input file is hardcoded to data.asmr in the same directory as this script.

Usage: python dataConvert.py [output.c] [base_dir]
  output.c:   Output C header file (default: data_converted.h in script directory)
  base_dir:   Base directory for resolving file paths (default: project root)
"""

import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

# Try to import PIL/Pillow for image conversion
try:
    from PIL import Image
    HAS_PIL = True
except ImportError:
    HAS_PIL = False


def find_gfxconv():
    """Find the GFXCONV tool from pvsneslib.
    
    Tries to find gfxconv wrapper first (which handles parameter conversion),
    then falls back to gfx2snes.exe directly.
    """
    # Check environment variables first
    pvsneslib_home = os.environ.get('PVSNESLIB_HOME')
    if pvsneslib_home:
        tools_path = os.path.join(pvsneslib_home, 'devkitsnes', 'tools')
        # Try gfxconv wrapper first (handles parameter conversion)
        for name in ['gfxconv.exe', 'gfxconv.bat', 'gfxconv.cmd']:
            gfxconv_path = os.path.join(tools_path, name)
            if os.path.exists(gfxconv_path):
                return gfxconv_path
        # Fall back to gfx2snes
        gfxconv_path = os.path.join(tools_path, 'gfx2snes.exe')
        if os.path.exists(gfxconv_path):
            return gfxconv_path
    
    # Check TOOLS_PATH environment variable
    tools_path = os.environ.get('TOOLS_PATH')
    if tools_path:
        for name in ['gfxconv.exe', 'gfxconv.bat', 'gfxconv.cmd', 'gfx2snes.exe']:
            gfxconv_path = os.path.join(tools_path, name)
            if os.path.exists(gfxconv_path):
                return gfxconv_path
    
    # Try common Windows paths
    common_paths = [
        r'C:\pvsneslib\devkitsnes\tools\gfxconv.exe',
        r'C:\pvsneslib\devkitsnes\tools\gfxconv.bat',
        r'C:\pvsneslib\devkitsnes\tools\gfx2snes.exe',
        r'C:\devkitsnes\tools\gfxconv.exe',
        r'C:\devkitsnes\tools\gfx2snes.exe',
    ]
    for path in common_paths:
        if os.path.exists(path):
            return path
    
    # Try to find it in PATH (gfxconv wrapper first, then gfx2snes)
    import shutil
    for name in ['gfxconv.exe', 'gfxconv', 'gfx2snes.exe', 'gfx2snes']:
        gfxconv_exe = shutil.which(name)
        if gfxconv_exe:
            return gfxconv_exe
    
    return None


def get_gfxconv_params(filename, source_file, is_wrapper=True):
    """Determine GFXCONV/gfx2snes parameters based on filename pattern.
    
    If is_wrapper is True, uses original Makefile format (for gfxconv wrapper).
    If False, converts to gfx2snes native format.
    """
    filename_lower = filename.lower()
    source_ext = os.path.splitext(source_file)[1].lower()
    
    # If using gfxconv wrapper, use original Makefile format
    if is_wrapper:
        file_type_flag = ['-t', source_ext[1:]] if source_ext else ['-t', 'bmp']
        
        # Font files
        if 'font' in filename_lower or 'pvsneslibfont' in filename_lower:
            if 'mariofont' in filename_lower:
                # mariofont: -s 8 -o 2 -u 16 -e 1 -p -t bmp -m -R -i
                return ['-s', '8', '-o', '2', '-u', '16', '-e', '1', '-p'] + file_type_flag + ['-m', '-R', '-i']
            else:
                # pvsneslibfont: -s 8 -o 2 -u 16 -p -e 1 -t bmp -i
                return ['-s', '8', '-o', '2', '-u', '16', '-p', '-e', '1'] + file_type_flag + ['-i']
        
        # Sprite files
        if 'sprite' in filename_lower or 'mario_sprite' in filename_lower:
            # mario_sprite: -s 16 -o 16 -u 16 -p -t bmp -i
            return ['-s', '16', '-o', '16', '-u', '16', '-p'] + file_type_flag + ['-i']
        
        # Stage graphics (default rule for PNG files)
        # -s 8 -o 16 -u 16 -p -m -R -i
        return ['-s', '8', '-o', '16', '-u', '16', '-p', '-m', '-R', '-i']
    
    else:
        # Direct gfx2snes format (if wrapper not found)
        # Note: gfx2snes uses different parameter format
        # For PNG files, we need to specify format and let gfx2snes auto-detect bit depth
        file_type_flag = ['-fpng'] if source_ext == '.png' else (['-fbmp'] if source_ext == '.bmp' else ['-fbmp'])
        
        # Font files
        if 'font' in filename_lower or 'pvsneslibfont' in filename_lower:
            if 'mariofont' in filename_lower:
                # For direct gfx2snes, use simplified parameters
                # -gs8 = graphics size 8, -mn2 = mode 2, -pe1 = palette entry 1, -p = palette, -m = mode, -mR! = mode R
                return ['-gs8', '-mn2', '-pe1', '-p'] + file_type_flag + ['-m', '-mR!']
            else:
                return ['-gs8', '-mn2', '-pe1', '-p'] + file_type_flag
        
        # Sprite files
        if 'sprite' in filename_lower or 'mario_sprite' in filename_lower:
            return ['-gs16', '-mn16', '-p'] + file_type_flag
        
        # Stage graphics - try with auto bit depth detection
        # Use -gs8 -mn16 -p for stage graphics, let gfx2snes handle bit depth
        return ['-gs8', '-mn16', '-p'] + file_type_flag + ['-m', '-mR!']


def convert_png_to_snes_tiles(source_png_path, output_pic_path, output_pal_path=None, max_colors=16):
    """Convert PNG image directly to SNES 4bpp tile format (.pic) and palette (.pal).
    
    SNES 4bpp tile format:
    - Each 8x8 tile is 32 bytes
    - 4 bitplanes: b0, b1, b2, b3
    - Format: b0[0-7], b1[0-7], b2[0-7], b3[0-7] (8 bytes per bitplane per tile)
    
    Palette format:
    - 16 colors * 2 bytes = 32 bytes
    - Each color is 15-bit BGR format: -bbb-bbgg-gggr-rrrr
    """
    if not HAS_PIL:
        return False, "PIL/Pillow not available"
    
    try:
        img = Image.open(source_png_path)
        
        # Convert to indexed mode with max_colors
        if img.mode not in ('P', 'L'):
            if img.mode == 'RGBA':
                background = Image.new('RGB', img.size, (255, 255, 255))
                background.paste(img, mask=img.split()[3])
                img = background
            if img.mode == 'RGB':
                img = img.convert('P', palette=Image.ADAPTIVE, colors=max_colors)
        
        # Ensure we have a palette mode image
        if img.mode == 'L':
            img = img.convert('P', palette=Image.ADAPTIVE, colors=max_colors)
        elif img.mode != 'P':
            return False, f"Unsupported image mode: {img.mode}"
        
        width, height = img.size
        
        # Ensure dimensions are multiples of 8 (tile size)
        if width % 8 != 0 or height % 8 != 0:
            return False, f"Image size {width}x{height} must be multiple of 8x8"
        
        # Get pixel data
        pixels = list(img.getdata())
        num_tiles_x = width // 8
        num_tiles_y = height // 8
        num_tiles = num_tiles_x * num_tiles_y
        
        # Convert to SNES 4bpp format
        tile_data = bytearray()
        
        for tile_y in range(num_tiles_y):
            for tile_x in range(num_tiles_x):
                # Extract 8x8 tile
                tile_pixels = []
                for y in range(8):
                    row_start = (tile_y * 8 + y) * width + (tile_x * 8)
                    tile_pixels.append(pixels[row_start:row_start+8])
                
                # Convert to 4 bitplanes (8 bytes per bitplane)
                bitplanes = [[0] * 8 for _ in range(4)]
                
                for y in range(8):
                    for x in range(8):
                        pixel = tile_pixels[y][x]
                        # Extract bits for each bitplane
                        for bp in range(4):
                            bit = (pixel >> bp) & 1
                            bitplanes[bp][y] |= (bit << (7 - x))
                
                # Write bitplanes in order: b0, b1, b2, b3
                for bp in range(4):
                    tile_data.extend(bitplanes[bp])
        
        # Write .pic file
        with open(output_pic_path, 'wb') as f:
            f.write(tile_data)
        
        # Extract and convert palette
        if output_pal_path and img.mode == 'P':
            palette = img.getpalette()
            if palette:
                # Convert palette to SNES format (15-bit BGR)
                pal_data = bytearray()
                # Get first max_colors colors
                for i in range(min(max_colors, len(palette) // 3)):
                    r = palette[i * 3]
                    g = palette[i * 3 + 1]
                    b = palette[i * 3 + 2]
                    
                    # Convert to 15-bit BGR: -bbb-bbgg-gggr-rrrr
                    r5 = (r >> 3) & 0x1F
                    g5 = (g >> 3) & 0x1F
                    b5 = (b >> 3) & 0x1F
                    color = (b5 << 10) | (g5 << 5) | r5
                    
                    # Write as little-endian 16-bit
                    pal_data.append(color & 0xFF)
                    pal_data.append((color >> 8) & 0xFF)
                
                # Pad to 32 bytes (16 colors)
                while len(pal_data) < 32:
                    pal_data.append(0)
                    pal_data.append(0)
                
                # Write .pal file
                with open(output_pal_path, 'wb') as f:
                    f.write(pal_data[:32])  # Exactly 32 bytes (16 colors)
        
        return True, f"Converted {num_tiles} tiles ({width}x{height})"
        
    except Exception as e:
        return False, str(e)


def convert_png_to_indexed(source_png_path, output_png_path=None):
    """Convert RGB PNG to indexed/palette mode for gfx2snes compatibility.
    
    Returns the path to the converted PNG (or original if already indexed).
    """
    if not HAS_PIL:
        return source_png_path  # Can't convert without PIL
    
    try:
        img = Image.open(source_png_path)
        
        # Check if image is already in palette/indexed mode
        if img.mode in ('P', 'L'):  # Palette or Grayscale (already indexed)
            return source_png_path
        
        # Convert RGB/RGBA to indexed palette mode
        # Use 'P' mode with optimized palette (up to 256 colors)
        if img.mode in ('RGB', 'RGBA'):
            # Convert RGBA to RGB first if needed
            if img.mode == 'RGBA':
                # Create white background and composite
                background = Image.new('RGB', img.size, (255, 255, 255))
                background.paste(img, mask=img.split()[3])  # Use alpha channel as mask
                img = background
            
            # Convert to palette mode with dithering for better quality
            # Use Image.ADAPTIVE for good quality with 256 colors
            img_indexed = img.convert('P', palette=Image.ADAPTIVE, colors=256)
            
            # Save to temporary file if output path not specified
            if output_png_path is None:
                output_png_path = source_png_path + '.indexed.png'
            
            img_indexed.save(output_png_path, 'PNG')
            return output_png_path
        else:
            # Unknown format, return original
            return source_png_path
            
    except Exception as e:
        # If conversion fails, return original path
        print(f"Warning: Could not convert {os.path.basename(source_png_path)} to indexed: {e}", file=sys.stderr)
        return source_png_path


def generate_intermediate_file(filepath, base_dir):
    """Generate intermediate .pic/.pal file from source .png/.bmp if needed."""
    filepath_lower = filepath.lower()
    
    # Only process .pic and .pal files
    if not (filepath_lower.endswith('.pic') or filepath_lower.endswith('.pal')):
        return
    
    # Check if file already exists at the expected path
    if not os.path.isabs(filepath):
        full_path = os.path.join(base_dir, filepath)
    else:
        full_path = filepath
    full_path = os.path.normpath(full_path)
    
    if os.path.exists(full_path):
        return  # File already exists, no need to generate
    
    # Also check if .pic/.pal file exists in same directory as source PNG/BMP
    # (sometimes they're generated alongside the source files)
    base_name = os.path.basename(full_path)
    if base_name.endswith('.pic'):
        base_name_noext = base_name[:-4]
        check_ext = '.pic'
    elif base_name.endswith('.pal'):
        base_name_noext = base_name[:-4]
        check_ext = '.pal'
    else:
        return
    
    # Check in the directory where the file should be
    check_dir = os.path.dirname(full_path)
    if os.path.exists(os.path.join(check_dir, base_name)):
        return  # Already exists in expected location
    
    # Also check in the source directory (where Makefile might have generated it)
    # This handles the case where Makefile generated files in the same dir as source PNG
    base_name = os.path.basename(full_path)
    if base_name.endswith('.pic') or base_name.endswith('.pal'):
        base_name_noext = base_name.rsplit('.', 1)[0]
        # Check in common source locations
        check_dirs = [
            os.path.dirname(full_path),  # Expected output directory
            base_dir,  # Base directory
        ]
        # Also check where source PNG might be
        for check_dir in check_dirs:
            if check_dir:
                alt_path = os.path.join(check_dir, base_name)
                if os.path.exists(alt_path) and alt_path != full_path:
                    # Found an existing file - copy it to expected location
                    os.makedirs(os.path.dirname(full_path), exist_ok=True)
                    shutil.copy2(alt_path, full_path)
                    return
    
    # Determine the output directory and base name
    output_dir = os.path.dirname(full_path)
    base_name = os.path.basename(full_path)
    if base_name.endswith('.pic'):
        base_name_noext = base_name[:-4]
        output_ext = '.pic'
    elif base_name.endswith('.pal'):
        base_name_noext = base_name[:-4]
        output_ext = '.pal'
    else:
        return
    
    # Try to find source file (.png first, then .bmp)
    # Check multiple potential locations in order of likelihood
    source_paths = []
    
    # 1. Same directory as output file (most common case)
    source_paths.extend([
        os.path.join(output_dir, base_name_noext + '.png'),
        os.path.join(output_dir, base_name_noext + '.bmp'),
    ])
    
    # 2. Base directory (for files like pvsneslibfont.pic in root/TempOld)
    source_paths.extend([
        os.path.join(base_dir, base_name_noext + '.png'),
        os.path.join(base_dir, base_name_noext + '.bmp'),
    ])
    
    # 3. TempOld directory (where source files often are)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    if script_dir:
        source_paths.extend([
            os.path.join(script_dir, base_name_noext + '.png'),
            os.path.join(script_dir, base_name_noext + '.bmp'),
        ])
    
    # 4. Parent directory of output (in case output is in a subdirectory like res/Stage)
    if output_dir != base_dir:
        parent_dir = os.path.dirname(output_dir)
        if parent_dir and parent_dir != output_dir:
            source_paths.extend([
                os.path.join(parent_dir, base_name_noext + '.png'),
                os.path.join(parent_dir, base_name_noext + '.bmp'),
            ])
    
    source_file = None
    for src_path in source_paths:
        if os.path.exists(src_path):
            source_file = src_path
            break
    
    if not source_file:
        # Silently skip if source file not found (might be optional)
        return
    
    # Try direct conversion first (if PIL is available and source is PNG/BMP)
    if HAS_PIL and source_file and (source_file.lower().endswith('.png') or source_file.lower().endswith('.bmp')):
        # Determine output paths
        if base_name.endswith('.pic'):
            expected_pic = full_path
            expected_pal = os.path.join(output_dir, base_name_noext + '.pal')
        elif base_name.endswith('.pal'):
            expected_pal = full_path
            expected_pic = os.path.join(output_dir, base_name_noext + '.pic')
        else:
            expected_pic = None
            expected_pal = None
        
        if expected_pic:
            # Try direct conversion (works for both PNG and BMP)
            success, message = convert_png_to_snes_tiles(source_file, expected_pic, expected_pal, max_colors=16)
            if success:
                print(f"Converted {base_name_noext} directly: {message}", file=sys.stderr)
                # If we were looking for .pal and it was generated, we're done
                if base_name.endswith('.pal') and os.path.exists(expected_pal):
                    return
                # If we were looking for .pic and it was generated, check if .pal is needed
                if base_name.endswith('.pic') and os.path.exists(expected_pic):
                    # .pal might have been generated, but we only needed .pic
                    # If both files exist now, we're done
                    if expected_pal and os.path.exists(expected_pal):
                        return
                    # Otherwise continue to gfx2snes for .pal generation
                    if not base_name.endswith('.pal'):
                        return  # We got the .pic file, done
    
    # Fall back to gfx2snes conversion
    gfxconv = find_gfxconv()
    if not gfxconv:
        print(f"Warning: GFXCONV tool not found and direct conversion failed. Cannot generate {filepath}", file=sys.stderr)
        if not HAS_PIL:
            print("  Install PIL/Pillow for direct conversion: pip install Pillow", file=sys.stderr)
        print("  Or set PVSNESLIB_HOME environment variable or install pvsneslib", file=sys.stderr)
        return
    
    # Determine if we're using a wrapper script (handles parameter conversion)
    # gfxconv wrapper uses one format, gfx2snes.exe uses a different format
    is_wrapper = 'gfxconv' in os.path.basename(gfxconv).lower()
    
    # Determine parameters based on which tool we're using
    params = get_gfxconv_params(base_name_noext, source_file, is_wrapper=is_wrapper)
    
    # GFXCONV outputs files to the same directory as the input file
    # We need to move them to the expected output directory
    source_dir = os.path.dirname(source_file)
    old_cwd = os.getcwd()
    
    # Convert PNG to indexed mode if needed (for PNG files only)
    converted_source_file = source_file
    temp_converted_file = None
    if source_file.lower().endswith('.png'):
        converted_source_file = convert_png_to_indexed(source_file)
        # If conversion created a new file, remember to clean it up
        if converted_source_file != source_file:
            temp_converted_file = converted_source_file
    
    try:
        # Change to source directory so GFXCONV can find the input file
        os.chdir(source_dir)
        
        # Use the converted filename (or original if no conversion happened)
        if temp_converted_file:
            # Use the converted file (might be in same dir or different)
            converted_dir = os.path.dirname(temp_converted_file)
            if converted_dir != source_dir:
                # Copy converted file to source dir temporarily
                temp_in_source_dir = os.path.join(source_dir, os.path.basename(temp_converted_file))
                shutil.copy2(temp_converted_file, temp_in_source_dir)
                source_filename = os.path.basename(temp_in_source_dir)
                # Update temp file to clean up the copy too
                temp_converted_file = temp_in_source_dir
            else:
                source_filename = os.path.basename(temp_converted_file)
        else:
            source_filename = os.path.basename(source_file)
        
        # Run GFXCONV (input is just the filename since we're in that directory)
        cmd = [gfxconv] + params + [source_filename]
        # Only print if verbose mode or if it's a critical file
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode != 0:
            # Check if it's a bit depth error (common with PNG files that aren't indexed)
            error_msg = result.stderr or result.stdout or ""
            if "not a valid bbp value" in error_msg.lower() or "not a valid bpp value" in error_msg.lower():
                # PNG file is not in the correct format (not indexed/palette mode)
                # Print detailed error for debugging
                print(f"Warning: {base_name_noext}: PNG not in indexed/palette format (need 4bpp/8bpp indexed)", file=sys.stderr)
            else:
                # Other errors - print detailed error message
                print(f"Warning: Could not convert {base_name_noext}", file=sys.stderr)
                if error_msg:
                    # Print first few lines of error
                    error_lines = error_msg.strip().split('\n')[:3]
                    for line in error_lines:
                        if line.strip():
                            print(f"  {line}", file=sys.stderr)
        else:
            # GFXCONV with -p flag generates both .pic and .pal files in source directory
            # Move them to the expected output directory if different
            generated_pic = os.path.join(source_dir, base_name_noext + '.pic')
            generated_pal = os.path.join(source_dir, base_name_noext + '.pal')
            expected_pic = os.path.join(output_dir, base_name_noext + '.pic')
            expected_pal = os.path.join(output_dir, base_name_noext + '.pal')
            
            # Move .pic file if it was generated and we need it in a different location
            if os.path.exists(generated_pic):
                if generated_pic != expected_pic:
                    os.makedirs(output_dir, exist_ok=True)
                    shutil.move(generated_pic, expected_pic)
            
            # Move .pal file if it was generated and we need it in a different location
            if os.path.exists(generated_pal):
                if generated_pal != expected_pal:
                    os.makedirs(output_dir, exist_ok=True)
                    shutil.move(generated_pal, expected_pal)
            
            # Verify the requested file exists (silently skip if not - may be optional)
            if not os.path.exists(full_path):
                pass  # File not created, will be handled as missing file later
    finally:
        os.chdir(old_cwd)
        # Clean up temporary converted PNG file if we created one
        if temp_converted_file and os.path.exists(temp_converted_file):
            try:
                os.remove(temp_converted_file)
            except:
                pass  # Ignore cleanup errors
        # Also clean up original converted file if different
        if temp_converted_file and temp_converted_file != source_file:
            converted_orig = converted_source_file
            if converted_orig != temp_converted_file and os.path.exists(converted_orig):
                try:
                    os.remove(converted_orig)
                except:
                    pass  # Ignore cleanup errors


def read_binary_file(filepath, base_dir=None, warn_on_missing=True):
    """Read a binary file and return its contents as bytes."""
    try:
        with open(filepath, 'rb') as f:
            data = f.read()
            if len(data) == 0 and warn_on_missing:
                print(f"Warning: File is empty: {filepath}", file=sys.stderr)
            return data
    except FileNotFoundError:
        if warn_on_missing:
            print(f"Warning: File not found: {filepath}", file=sys.stderr)
            # Try to find it in other locations
            if base_dir:
                filename = os.path.basename(filepath)
                alt_paths = [
                    os.path.join(base_dir, filename),
                    os.path.join(base_dir, "res", filename),
                    os.path.join(base_dir, "res", "back", filename),
                    os.path.join(base_dir, "res", "Stage", filename),
                    os.path.join(base_dir, "res", "sprite", filename),
                ]
                for alt_path in alt_paths:
                    if os.path.exists(alt_path):
                        try:
                            with open(alt_path, 'rb') as f:
                                data = f.read()
                                if len(data) > 0:
                                    print(f"  Found at: {alt_path}", file=sys.stderr)
                                    return data
                        except:
                            pass
        # Return empty data for missing files (they may be optional)
        return b''


def bytes_to_c_array(data, symbol_name, max_line_length=80, header_only=False):
    """Convert binary data to a C const unsigned char array.
    
    If header_only is True, generates extern declaration instead of definition.
    """
    if header_only:
        return f"extern const unsigned char {symbol_name}[];\nextern const unsigned int {symbol_name}_size;\n"
    
    if not data:
        return f"const unsigned char {symbol_name}[1] = {{0}};\nconst unsigned int {symbol_name}_size = 0;\n"
    
    # Truncate to 64KB (65536 bytes) to avoid linker bank size errors
    # For specific arrays that cause linker issues, use a smaller limit to account for overhead
    MAX_ARRAY_SIZE = 65536
    
    original_size = len(data)
    if len(data) > MAX_ARRAY_SIZE:
        print(f"Warning: Truncating {symbol_name} from {len(data)} bytes to {MAX_ARRAY_SIZE} bytes", file=sys.stderr)
        data = data[:MAX_ARRAY_SIZE]
    
    # Start the array declaration
    lines = [f"const unsigned char {symbol_name}[] = {{"]
    
    # Convert bytes to hex values
    hex_values = []
    for byte in data:
        hex_values.append(f"0x{byte:02x}")
    
    # Format into lines
    current_line = "    "
    for i, hex_val in enumerate(hex_values):
        if i == len(hex_values) - 1:
            # Last value
            current_line += hex_val
        else:
            current_line += hex_val + ","
        
        # Check if we should start a new line
        if len(current_line) >= max_line_length and i < len(hex_values) - 1:
            lines.append(current_line)
            current_line = "    "
        elif i < len(hex_values) - 1:
            current_line += " "
    
    if current_line.strip():
        lines.append(current_line)
    
    lines.append("};")
    # Always report original size in _size variable, even if truncated
    lines.append(f"const unsigned int {symbol_name}_size = {original_size};")
    lines.append("")
    
    return "\n".join(lines)


def parse_asm_file(asm_file_path, base_dir=None):
    """Parse the ASM file and extract binary file includes."""
    if base_dir is None:
        base_dir = os.path.dirname(asm_file_path)
    
    entries = []
    
    with open(asm_file_path, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()
    
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        
        # Skip empty lines and comments
        if not line or line.startswith(';') or line.startswith('/*') or line.startswith('*/'):
            i += 1
            continue
        
        # Match BIN macro: BIN symbol_name "filepath"
        bin_match = re.match(r'BIN\s+(\w+)\s+"([^"]+)"', line)
        if bin_match:
            symbol_name = bin_match.group(1)
            filepath = bin_match.group(2)
            entries.append((symbol_name, filepath))
            i += 1
            continue
        
        # Match label followed by .incbin: label: .incbin "filepath"
        label_incbin_match = re.match(r'(\w+):\s*\.incbin\s+"([^"]+)"', line)
        if label_incbin_match:
            symbol_name = label_incbin_match.group(1)
            filepath = label_incbin_match.group(2)
            entries.append((symbol_name, filepath))
            i += 1
            continue
        
        # Match standalone .incbin on next line after a label
        # Check if current line is a label and next line has .incbin
        label_match = re.match(r'(\w+):\s*$', line)
        if label_match and i + 1 < len(lines):
            next_line = lines[i + 1].strip()
            incbin_match = re.match(r'\.incbin\s+"([^"]+)"', next_line)
            if incbin_match:
                symbol_name = label_match.group(1)
                filepath = incbin_match.group(1)
                entries.append((symbol_name, filepath))
                i += 2
                continue
        
        # Match standalone .incbin: .incbin "filepath"
        incbin_match = re.match(r'\.incbin\s+"([^"]+)"', line)
        if incbin_match:
            # Try to find a label on previous non-empty line
            filepath = incbin_match.group(1)
            # Generate a symbol name from the filename
            filename = os.path.basename(filepath)
            symbol_name = re.sub(r'[^a-zA-Z0-9]', '_', filename)
            symbol_name = re.sub(r'_+', '_', symbol_name)
            if symbol_name and symbol_name[0].isdigit():
                symbol_name = '_' + symbol_name
            entries.append((symbol_name, filepath))
            i += 1
            continue
        
        i += 1
    
    return entries


def convert_asm_to_c(asm_file_path, output_file_path=None, base_dir=None):
    """Convert ASM file to C header/source files split into 2 compile units."""
    if base_dir is None:
        base_dir = os.path.dirname(asm_file_path) or '.'
    
    # Parse the ASM file
    entries = parse_asm_file(asm_file_path, base_dir)
    
    if not entries:
        print("No binary file entries found in ASM file.", file=sys.stderr)
        return
    
    # Split entries into two groups:
    # Part 1: All entries except BG_Moon and BG_Fog
    # Part 2: BG_Moon and BG_Fog (these are large and need separate compile units)
    entries_part1 = []
    entries_part2 = []
    
    for symbol_name, filepath in entries:
        if symbol_name in ('BG_Moon', 'BG_Fog'):
            entries_part2.append((symbol_name, filepath))
        else:
            entries_part1.append((symbol_name, filepath))
    
    # Determine output file paths
    script_dir = os.path.dirname(os.path.abspath(asm_file_path))
    if output_file_path is None:
        # Generate default paths
        header_part1 = os.path.join(script_dir, "data_converted_part1.h")
        source_part1 = os.path.join(script_dir, "data_converted_part1.c")
        header_part2 = os.path.join(script_dir, "data_converted_part2.h")
        source_part2 = os.path.join(script_dir, "data_converted_part2.c")
    else:
        # Use provided path as base
        base_path = os.path.splitext(output_file_path)[0]
        header_part1 = base_path + "_part1.h"
        source_part1 = base_path + "_part1.c"
        header_part2 = base_path + "_part2.h"
        source_part2 = base_path + "_part2.c"
    
    # Process entries and generate files
    def generate_files(entries_list, header_path, source_path, part_num):
        # Generate header file with extern declarations
        header_lines = []
        header_lines.append(f"/* Auto-generated from data.asmr - Part {part_num} */")
        header_lines.append(f"#ifndef DATA_CONVERTED_PART{part_num}_H")
        header_lines.append(f"#define DATA_CONVERTED_PART{part_num}_H")
        header_lines.append("")
        header_lines.append("#include <stdint.h>")
        header_lines.append("")
        
        # Generate source file with definitions
        source_lines = []
        source_lines.append(f"/* Auto-generated from data.asmr - Part {part_num} */")
        source_lines.append(f"#include \"{os.path.basename(header_path)}\"")
        source_lines.append("")
        
        # Process each entry
        for symbol_name, filepath in entries_list:
            # Resolve the file path relative to base_dir
            if not os.path.isabs(filepath):
                full_path = os.path.join(base_dir, filepath)
            else:
                full_path = filepath
            
            # Normalize the path
            full_path = os.path.normpath(full_path)
            
            # Generate intermediate file if needed (.pic/.pal from .png/.bmp)
            generate_intermediate_file(filepath, base_dir)
            
            # Re-check path after intermediate file generation (it might have been created)
            if not os.path.exists(full_path):
                full_path = os.path.normpath(os.path.join(base_dir, filepath))
            
            # Read the binary file
            data = read_binary_file(full_path, base_dir, warn_on_missing=True)
            
            # Add extern declaration to header
            header_code = bytes_to_c_array(data, symbol_name, header_only=True)
            header_lines.append(header_code)
            
            # Add definition to source
            source_code = bytes_to_c_array(data, symbol_name, header_only=False)
            source_lines.append(source_code)
        
        header_lines.append(f"#endif /* DATA_CONVERTED_PART{part_num}_H */")
        
        # Write header file
        header_output = "\n".join(header_lines)
        with open(header_path, 'w', encoding='utf-8') as f:
            f.write(header_output)
        print(f"Generated header: {header_path} ({len(entries_list)} entries)")
        
        # Write source file
        source_output = "\n".join(source_lines)
        with open(source_path, 'w', encoding='utf-8') as f:
            f.write(source_output)
        print(f"Generated source: {source_path} ({len(entries_list)} entries)")
    
    # Generate both parts
    generate_files(entries_part1, header_part1, source_part1, 1)
    generate_files(entries_part2, header_part2, source_part2, 2)
    
    print(f"\nTotal: {len(entries)} entries split into 2 compile units")
    print(f"  Part 1: {len(entries_part1)} entries")
    print(f"  Part 2: {len(entries_part2)} entries")


def main():
    """Main entry point."""
    # Hardcoded input file path (relative to script location)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    asm_file = os.path.join(script_dir, "data.asmr")
    
    # Optional output file and base directory from command line
    output_file = sys.argv[1] if len(sys.argv) > 1 else None
    base_dir = sys.argv[2] if len(sys.argv) > 2 else None
    
    # Default output file to data_converted.h in script directory
    if output_file is None:
        output_file = os.path.join(script_dir, "data_converted.h")
    
    # Default base_dir to project root (parent of TempOld)
    if base_dir is None:
        base_dir = os.path.dirname(script_dir) or '.'
    
    if not os.path.exists(asm_file):
        print(f"Error: Input file not found: {asm_file}", file=sys.stderr)
        sys.exit(1)
    
    convert_asm_to_c(asm_file, output_file, base_dir)


if __name__ == '__main__':
    main()

