#coding: utf-8
#!/usr/bin/env python3

import os
import sys
import re
import argparse
from pathlib import Path

def split_file(combined_file, output_dir=None):
    combined_path = Path(combined_file)
    if not combined_path.exists():
        print(f"Error: file {combined_file} does not exist")
        return False
    
    if output_dir is None:
        output_dir = combined_path.parent / 'split'
    else:
        output_dir = Path(output_dir)
    
    output_dir.mkdir(parents=True, exist_ok=True)
    
    print(f"Output directory: {output_dir}")
    
    tag_pattern = re.compile(r'<([^>]+)>\n?(.*?)</\1>', re.DOTALL | re.MULTILINE)
    
    extracted_files = 0
    
    with open(combined_file, 'r', encoding='utf-8') as f:
        content = f.read()
        
        for match in tag_pattern.finditer(content):
            filename = match.group(1)
            file_content = match.group(2)
            
            file_content = file_content.strip()
            if not file_content.endswith('\n'):
                file_content += '\n'
            
            file_path = output_dir / filename
            
            file_path.parent.mkdir(parents=True, exist_ok=True)
            
            try:
                with open(file_path, 'w', encoding='utf-8') as out:
                    out.write(file_content)
                print(f"  Created {file_path}")
                extracted_files += 1
            except Exception as e:
                print(f"  Error creating {file_path}: {e}")
    
    print(f"Done! Extracted files: {extracted_files}")
    return True

def main():
    parser = argparse.ArgumentParser(description='Split combined file back into .h/.cpp files')
    parser.add_argument('combined_file', help='Combined file with tags')
    parser.add_argument('-o', '--output', help='Output directory (default: combined_file_dir/split)')
    
    args = parser.parse_args()
    
    success = split_file(args.combined_file, args.output)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()