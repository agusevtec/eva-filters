#coding: utf-8
#!/usr/bin/env python3

import os
import sys
import argparse
from pathlib import Path

def combine_files(source_dir, output_file, extensions=None):
    if extensions is None:
        extensions = ['.h', '.cpp']
    
    source_path = Path(source_dir)
    if not source_path.exists():
        print(f"Error: directory {source_dir} does not exist")
        return False
    
    files_to_combine = []
    for ext in extensions:
        files_to_combine.extend(source_path.glob(f'**/*{ext}'))
    
    files_to_combine.sort()
    
    if not files_to_combine:
        print(f"No files found with extensions {extensions}")
        return False
    
    print(f"Found {len(files_to_combine)} files to combine:")
    for f in files_to_combine[:5]:
        print(f"  {f}")
    if len(files_to_combine) > 5:
        print(f"  ... and {len(files_to_combine) - 5} more")
    
    with open(output_file, 'w', encoding='utf-8') as out:
        out.write(f"// Combined project file\n")
        out.write(f"// Generated from {source_dir}\n")
        out.write(f"// Total files: {len(files_to_combine)}\n\n")
        
        for file_path in files_to_combine:
            rel_path = file_path.relative_to(source_path)
            
            print(f"Adding {rel_path}...")
            
            out.write(f"<{rel_path}>\n")
            
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                    out.write(content)
                    if not content.endswith('\n'):
                        out.write('\n')
            except Exception as e:
                print(f"  Error reading {file_path}: {e}")
                out.write(f"// ERROR reading file: {e}\n")
            
            out.write(f"</{rel_path}>\n\n")
    
    print(f"Done! Combined file: {output_file}")
    return True

def main():
    parser = argparse.ArgumentParser(description='Combine .h and .cpp files into one')
    parser.add_argument('source_dir', help='Source directory with files')
    parser.add_argument('-o', '--output', default='combined.txt', 
                        help='Output file (default: combined.txt)')
    parser.add_argument('--ext', nargs='+', default=['.h', '.cpp'],
                        help='File extensions to combine')
    
    args = parser.parse_args()
    
    success = combine_files(args.source_dir, args.output, args.ext)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()