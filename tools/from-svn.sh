#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Usage: $0 <svn-url> [folder-name]"
    echo "  folder-name - optional, if not specified uses last part of URL"
    exit 1
fi

URL="$1"

# Если указан второй параметр, используем его как имя папки
if [ -n "$2" ]; then
    REPO_NAME="$2"
else
    # Иначе извлекаем последний сегмент URL
    REPO_NAME=$(basename "$URL" | sed 's/\.svn$//')
fi

if [ ! -d "libraries" ]; then
    mkdir libraries || {
        echo "Error: Failed to create 'libraries' directory"
        exit 1
    }
fi

if [ -d "libraries/$REPO_NAME" ]; then
    echo "Note: Directory 'libraries/$REPO_NAME' already exists. Skipping checkout."
    exit 0
fi

echo "Checking out $REPO_NAME..."
svn checkout "$URL" "libraries/$REPO_NAME" || {
    echo "Error: Failed to checkout $REPO_NAME repository"
    exit 1
}

echo "Successfully checked out $REPO_NAME"
