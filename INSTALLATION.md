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

### Fedora:
```bash
sudo dnf install meson ninja-build
```

### GCC

You'll also require a C++ Compiler like `g++`.

### Ubuntu, Debian, Mint:
```bash
sudo apt install g++
```

### Arch
```bash
sudo pacman -S gcc
```

### Fedora:
```bash
sudo dnf install gcc-c++
```

### libzstd and json-glib

You will require those 2 libraries, they should usually be installed on your OS, but if they aren't you can use this:

### Ubuntu, Debian, Mint:
```bash
sudo apt install libzstd1 libjson-glib-1.0-0
```

### Arch
```bash
sudo pacman -S zstd json-glib
```

### Fedora
```bash
sudo dnf install libzstd json-glib
```

## GitHub

### Install a copy

Install a copy of the Repository by using the `gh clone` command from the GitHub CLI.
```bash
gh repo clone null-pointer-9341/Velocity
cd Velocity
```

### Important notice

Whenever you are creating a precompiled binary (for a release) then use this setup!
```bash
meson setup build --buildtype=release
```

After that you can simply compile like normal.

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

### Important notice

Whenever you are creating a precompiled binary (for a release) then use this setup!
```bash
meson setup build --buildtype=release
```

After that you can simply compile like normal.

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

### Tar it

From the repository root, choose a new version folder and copy the template into it.
```bash
VERSION=v3 # PICK THE NAME PLEASE!
mkdir -p "precompiled-binaries/$VERSION"
cp -r precompiled-binaries/template/. "precompiled-binaries/$VERSION/" # Copy the template contents into the version folder
install -m 755 build/velocity "precompiled-binaries/$VERSION/velocity" # Copy the build result into the version folder and give 755 permissions.
cd "precompiled-binaries/$VERSION" # Move into it.
tar -czvf velocity-linux-x86_64.tar.gz velocity installer.sh # Zip it.
```

## Precompiled Binary

### Get latest

You can get one of the [precompiled binaries](https://github.com/null-pointer-9341/Velocity/releases/latest).

Since I use Arch Linux, most of these precompiled binaries will work on Arch.

Feel free to build the binary in your operating system and contribute with the precompiled binary.

### Install latest

After you extract the `install.sh` and `velocity` compiled files, you can run the `installer.sh` in order to put `velocity` on your path.

```bash
./install.sh
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