# Contributing to TermuOS

Thank you for contributing to TermuOS

## Requirements

You should have:
- Linux environment (Windows -> WSL)
- clang or gcc (clang prefered)
- NASM
- xorriso
- qemu-system-x86_64
- make

## Building

Clone the repository:

```bash
git clone https://github.com/TermuOS-Project/TermuOS.git
cd TermuOS
```
Build the OS:
```bash
make
```
Run in QEMU:
```bash
make run
```

## Coding Style
- Use tabs or 4 spaces consistently
- Keep kernel code modular
- Avoid unnecessary global variables
- Comment low-level or hardware-specific code
- Use snake_case for functions and variables

## Branch Naming
Examples:
- feat/network-stack
- fix/paging-bug
- docs/readme-update

## Pull Requests
Before opening a pull request
- Make sure the OS builds successfully
- Test changes in QEMU
- Describe what was changed
- Keep pull requests focused on one feature/fix

## Areas That Need Help
- Networking
- Drivers
- Filesystems
- Scheduler improvements
- User programs
- Terminal enhancements

## Reporting Bugs
Include:
- screenshots/logs
- steps to reproduce
- expected behavior
- actual behavior

## Project Goals
TermuOS aims to be a Linux-inspired operating system focused on:
- low-level programming
- multitasking
- modular kernel design
- terminal-first workflow
