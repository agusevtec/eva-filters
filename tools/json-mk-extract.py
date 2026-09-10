#!/usr/bin/env python3

import json
import os
import sys
from pathlib import Path

def create_files_from_json(json_file, base_dir="docs"):
    try:
        with open(json_file, 'r', encoding='utf-8') as f:
            data = json.load(f)
    except FileNotFoundError:
        print(f"Error: File {json_file} not found")
        return False
    except json.JSONDecodeError as e:
        print(f"JSON parse error: {e}")
        return False
    
    base_path = Path(base_dir)
    base_path.mkdir(exist_ok=True)
    
    created_files = 0
    created_dirs = set()
    
    for file_path, content in data.items():
        full_path = base_path / file_path
        
        full_path.parent.mkdir(parents=True, exist_ok=True)
        created_dirs.add(str(full_path.parent))
        
        try:
            with open(full_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"  ✓ {full_path}")
            created_files += 1
        except Exception as e:
            print(f"  ✗ Error creating {full_path}: {e}")
    
    print("\n" + "="*50)
    print(f"Done!")
    print(f"Directories created: {len(created_dirs)}")
    print(f"Files created: {created_files}")
    print(f"Base directory: {base_path.absolute()}")
    
    return True

def main():
    if len(sys.argv) > 1:
        json_file = sys.argv[1]
    else:
        json_file = "docs.json"
    
    if len(sys.argv) > 2:
        base_dir = sys.argv[2]
    else:
        base_dir = "docs_candidate"
    
    print(f"Creating files from {json_file} to {base_dir}/...")
    create_files_from_json(json_file, base_dir)

if __name__ == "__main__":
    main()