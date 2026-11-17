# Installation Guide

## Dependencies

### Meson

In order to compile the CLI, you'll require to install `meson` which is a build system.

### Ubuntu, Debian, Mint:
```bash
sudo apt install meson ninja-build
```

* Alternatively, you can use `pip3`.
```bash
pip3 install --user meson
```

### Arch:
```bash
sudo pacman -S meson ninja   
```

### GCC

You'll also require a C++ Compiler like `g++`.

### Ubuntu, Debian, Mint:
```bash
sudo apt install g++
```

## Arch
```bash
sudo pacman -S gcc
```

## GitHub

### Install a copy

Install a copy of the Repository by using the `gh clone` command from the GitHub CLI.
```bash
gh repo clone null-pointer-9341/Velocity
cd Velocity
```

### Build and install

Build using `meson`.
```bash
meson compile -C build 
```

Install it (so you can use the CLI via `velocity <args>`)
```bash
sudo meson install -C build
```

## Local Files

### Build and install

Build from the root folder using `meson`.
```bash
meson compile -C build 
```

Install it (so you can use the CLI via `velocity <args>`)
```bash
sudo meson install -C build
```