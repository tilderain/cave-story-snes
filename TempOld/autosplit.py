import os
import re

# CONFIGURATION
INPUT_FILE = "data.asm"       # Your original asm file name
OUTPUT_FILE = "data_split.asm" # The new file to generate
MAX_SECTION_SIZE = 65500      # 64KB is 65536, keeping a small buffer for safety

# Regex to find BIN macro calls: BIN Label "Path"
regex_bin = re.compile(r'^\s*BIN\s+\w+\s+"([^"]+)"')
# Regex to find standard .incbin: .incbin "Path"
regex_incbin = re.compile(r'^\s*\.incbin\s+"([^"]+)"')

def get_file_size(filepath):
    # Remove quotes and handle relative paths
    clean_path = filepath.strip('"')
    # Fix windows/unix path separators if necessary
    clean_path = clean_path.replace('\\', '/')
    
    if not os.path.exists(clean_path):
        print(f"Warning: File not found: {clean_path}. Assuming 0 bytes.")
        return 0
    return os.path.getsize(clean_path)

def process_file():
    if not os.path.exists(INPUT_FILE):
        print(f"Error: {INPUT_FILE} not found.")
        return

    with open(INPUT_FILE, 'r') as f_in:
        lines = f_in.readlines()

    output_lines = []
    
    # Write the macros and header (assumed to be at the top)
    # We will just copy lines until we hit the first section or data
    header_done = False
    
    # Counters
    current_bank_size = 0
    bank_index = 1
    
    # Start the first section
    output_lines.append(f'.section ".rodata{bank_index}" superfree\n')

    for line in lines:
        line_stripped = line.strip()
        
        # Skip existing section/ends tags to re-organize them ourselves
        if line_stripped.startswith('.section') or line_stripped.startswith('.ends'):
            continue
            
        # Preserve Macros and includes at the very top (Header detection logic)
        if line_stripped.startswith('.macro') or line_stripped.startswith('.include'):
            output_lines.append(line)
            continue
        
        # Calculate size of the current line
        added_size = 0
        
        # Check for BIN macro
        match_bin = regex_bin.search(line)
        if match_bin:
            added_size = get_file_size(match_bin.group(1))
            # Add small overhead for labels/alignment (safe estimate)
            added_size += 4 

        # Check for .incbin
        elif not match_bin: # Only check if not already matched
            match_inc = regex_incbin.search(line)
            if match_inc:
                added_size = get_file_size(match_inc.group(1))

        # Check for .db/.dw tables (rough estimate)
        if line_stripped.startswith('.db'):
            # Estimate 1 byte per entry (count commas + 1)
            added_size = line_stripped.count(',') + 1
            if '"' in line_stripped: added_size += len(line_stripped) # String estimate

        # DECISION TIME: Will this fit?
        if current_bank_size + added_size > MAX_SECTION_SIZE:
            # It won't fit. Close current section.
            output_lines.append('\n.ends\n')
            
            # Reset counters
            bank_index += 1
            current_bank_size = 0
            
            # Start new section
            output_lines.append(f'\n; --- Auto-split: Switching to Bank {bank_index} ---\n')
            output_lines.append(f'.section ".rodata{bank_index}" superfree\n')

        # Add the line to output and update size
        output_lines.append(line)
        current_bank_size += added_size

    # Close the final section
    output_lines.append('\n.ends\n')

    # Write result
    with open(OUTPUT_FILE, 'w') as f_out:
        f_out.writelines(output_lines)

    print(f"Success! Processed {bank_index} banks.")
    print(f"Output saved to {OUTPUT_FILE}")

if __name__ == "__main__":
    process_file()
