#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Usage: $0 <git-url>"
    exit 1
fi

url="$1"
repo_name=$(basename "$url" .git)

if [ ! -d "libraries" ]; then
    mkdir libraries
    if [ $? -ne 0 ]; then
        echo "Error: Failed to create 'libraries' directory"
        exit 1
    fi
fi

if [ -d "libraries/$repo_name" ]; then
    echo "Note: Directory 'libraries/$repo_name' already exists. Skipping clone."
    exit 0
fi

echo "Cloning $repo_name..."
git clone "$url" "libraries/$repo_name"
if [ $? -ne 0 ]; then
    echo "Error: Failed to clone $repo_name repository"
    exit 1
fi

echo "Successfully cloned $repo_name"
