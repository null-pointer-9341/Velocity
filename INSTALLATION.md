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

### Set up meson

This is only required to be done once.
```bash
meson setup build 
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

### Set up meson

This is only required to be done once per branch.
```bash
meson setup build 
```

### Build and install

Build from the root folder using `meson`.
```bash
meson compile -C build 
```

Install it (so you can use the CLI via `velocity <args>`)
```bash
sudo meson install -C build
```

# Usage Guide

## CLI

### Help

In order to get a list of commands from the Command-Line Interface you can use:
```bash
velocity -help
```

### Initiation

To create config files, you will have to initiate the root folder of your source code.
```bash
velocity init
```

This will create three folders in the `/src` subdirectory:

* server
* client
* shared

The `server` folder represents `ServerScriptService`, the `client` folder represents `StarterPlayerScripts` and `shared` represents `ReplicatedStorage`.

### Other functions

From there, you will be able to use:
```bash
velocity build
```

Which will output the finished `.vrtx` file into the `/build` directory.

You can run:
```bash
velocity config
```

In order to generate the `/config` folder, which will contain all configurable data, like the project name, the project version etc.