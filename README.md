# COW Tree Cloning Program

## Overview

The COW Tree Cloning Program is a C utility that creates a copy-on-write (COW) clone of a directory tree at a different mount location. It uses the `clonefile` system call to efficiently clone files and directories while preserving copy-on-write semantics. (OSX Only)

## Features

- Recursively clones directory structures from source to target
- Uses copy-on-write semantics for efficient storage utilization
- Handles repeated runs by updating the target location while preserving modifications
- Supports symbolic links, preserving their targets in the cloned structure
- Maintains file timestamps and permissions, except it makes files user writeable if they aren't already

## Requirements

- macOS operating system (the program uses the `clonefile` system call, which is specific to macOS)
- GCC or Clang compiler

## Compilation

To compile the program, use the following command:
