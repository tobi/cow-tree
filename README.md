# COW-Tree - Clone a directory

## Overview

The COW Tree Cloning Program is a C utility that creates a copy-on-write (COW) clone of a directory tree at a different mount location. It uses the `clonefile` system call to efficiently clone files and directories while preserving copy-on-write semantics. (OSX Only)

## Usecases

## Use Cases

One particularly powerful use case for COW-Tree is hydrating a local cache from a network mount, especially when using content-addressable storage package managers like uv (Universal Versioning).

1. **Efficient Local Caching**: When working with large repositories or package collections stored on network mounts, COW-Tree can quickly create a local, copy-on-write clone of the entire directory structure. This allows for fast, local access to files without duplicating the entire dataset.

2. **Optimizing Package Manager Performance**: Content-addressable storage package managers like uv benefit greatly from COW-Tree. By cloning the package cache locally:
   - Initial setup time is significantly reduced, as only metadata is copied, not the full content.
   - Subsequent package operations become much faster, as they can work with local files instead of network-mounted ones.
   - Storage space is optimized, as only modified or new files consume additional space.

3. **Reduced Network Load**: By creating a local COW clone, network traffic is minimized. Only changes and new files need to be synced, reducing strain on network resources.

4. **Improved Build and CI/CD Performance**: In development and CI/CD environments, having a local, efficiently cloned cache of packages can dramatically speed up build times and reduce dependencies on network reliability.

5. **Quick Environment Replication**: COW-Tree allows for rapid creation of multiple local environments, each starting as an efficient clone but able to diverge as needed without affecting the original or other clones.

These use cases make COW-Tree an invaluable tool for developers and system administrators working with large, network-stored repositories or package collections, especially in conjunction with modern package management systems like uv.

## Features

- Recursively clones directory structures from source to target
- Uses copy-on-write semantics for efficient storage utilization
- Handles repeated runs by updating the target location while preserving modifications
- Supports symbolic links, preserving their targets in the cloned structure
- Maintains file timestamps and permissions, except it makes files user writeable if they aren't already

## Requirements

- macOS operating system (the program uses the `clonefile` system call, which is specific to macOS)
- GCC or Clang compiler

## Usage

Usage: `cow-tree [from] [to]`

Please note that this will make files writeable, if you don't want this add --no-permission-changes

Also, be careful, this isn't exactly the most battle hardened project as its written 99% by Cloude 3.5 :-)

## Compilation

To compile the program, use the following command: `make`
