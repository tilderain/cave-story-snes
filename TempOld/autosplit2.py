import os
import re

# CONFIGURATION
INPUT_FILE = "data.asmr"       # Your source file
OUTPUT_BASE = "data_part"     # Output prefix (data_part0.asm, data_part1.asm...)
MAX_BANK_SIZE = 65535         # Safe limit per file (under 64KB to be safe)

# Regex setup
regex_bin = re.compile(r'^\s*BIN\s+\w+\s+"([^"]+)"')
regex_incbin = re.compile(r'^\s*\.incbin\s+"([^"]+)"')

def get_file_size(filepath):
    clean_path = filepath.strip('"').replace('\\', '/')
    if not os.path.exists(clean_path):
        print(f"Warning: Asset not found: {clean_path} (Assuming 0 bytes)")
        return 0
    return os.path.getsize(clean_path)

def main():
    if not os.path.exists(INPUT_FILE):
        print(f"Error: {INPUT_FILE} missing.")
        return

    with open(INPUT_FILE, 'r') as f:
        lines = f.readlines()

    header_lines = []
    content_start_index = 0

    # 1. Extract Header (Macros + Includes)
    # We assume the header ends when the first .section appears
    for i, line in enumerate(lines):
        if line.strip().startswith('.section'):
            content_start_index = i
            break
        header_lines.append(line)

    # 2. Process Data Lines
    file_counter = 0
    current_size = 0
    current_file_lines = []

    # Prepare first file with header
    current_file_lines.extend(header_lines)
    current_file_lines.append(f'\n; --- Part {file_counter} ---\n')
    current_file_lines.append(f'.section ".rodata_p{file_counter}" superfree\n')

    for i in range(content_start_index, len(lines)):
        line = lines[i]
        stripped = line.strip()

        # Skip existing section/ends tags (we handle them)
        if stripped.startswith('.section') or stripped.startswith('.ends'):
            continue
        
        # Calculate size
        added_size = 0
        match_bin = regex_bin.search(line)
        match_inc = regex_incbin.search(line)
        
        if match_bin:
            added_size = get_file_size(match_bin.group(1)) + 4 # +overhead
        elif match_inc:
            added_size = get_file_size(match_inc.group(1))
        elif stripped.startswith('.db'):
            added_size = stripped.count(',') + 1

        # Check if we need to split
        if current_size + added_size > MAX_BANK_SIZE:
            # Close current file
            current_file_lines.append('\n.ends\n')
            fname = f"{OUTPUT_BASE}{file_counter}.asm"
            with open(fname, 'w') as f_out:
                f_out.writelines(current_file_lines)
            print(f"Created {fname} ({current_size} bytes)")

            # Start new file
            file_counter += 1
            current_size = 0
            current_file_lines = []
            current_file_lines.extend(header_lines) # Add macros/hdr to new file
            current_file_lines.append(f'\n; --- Part {file_counter} ---\n')
            current_file_lines.append(f'.section ".rodata_p{file_counter}" superfree\n')

        current_file_lines.append(line)
        current_size += added_size

    # Save the last file
    if current_file_lines:
        current_file_lines.append('\n.ends\n')
        fname = f"{OUTPUT_BASE}{file_counter}.asm"
        with open(fname, 'w') as f_out:
            f_out.writelines(current_file_lines)
        print(f"Created {fname} ({current_size} bytes)")

    print(f"\nDone! Created {file_counter + 1} files.")
    print("Please add these new .asm files (or their .obj versions) to your Makefile.")

if __name__ == "__main__":
    main()
