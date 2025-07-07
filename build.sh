#!/bin/bash

# Build script for Galactic Shmup on macOS
# This script compiles all source files and links them with raylib

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Building Galactic Shmup...${NC}"

# Check if raylib is installed
if ! pkg-config --exists raylib; then
    echo -e "${RED}Error: raylib not found!${NC}"
    echo "Please install raylib first:"
    echo "  brew install raylib"
    echo "Or:"
    echo "  git clone https://github.com/raysan5/raylib.git"
    echo "  cd raylib/src && make PLATFORM=PLATFORM_DESKTOP"
    exit 1
fi

# Get raylib flags
RAYLIB_CFLAGS=$(pkg-config --cflags raylib)
RAYLIB_LIBS=$(pkg-config --libs raylib)

# Compiler settings
CC=clang
CFLAGS="-Wall -Wextra -std=c99 -O2"
TARGET="galactic_shmup"

# Source files (all .c files in current directory)
SOURCES="shmup.c game.c player.c enemy.c enemy_ai.c collision.c score.c menu.c render.c utils.c"

# Check if all source files exist
for src in $SOURCES; do
    if [ ! -f "$src" ]; then
        echo -e "${RED}Error: Source file $src not found!${NC}"
        exit 1
    fi
done

echo -e "${YELLOW}Compiling source files...${NC}"

# Compile and link
$CC $CFLAGS $RAYLIB_CFLAGS $SOURCES $RAYLIB_LIBS -o $TARGET

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build successful!${NC}"
    echo -e "${GREEN}Executable: ./$TARGET${NC}"
    echo
    echo "To run the game:"
    echo "  ./$TARGET"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi
