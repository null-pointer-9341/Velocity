#!/bin/bash
# Alright so this script just copies velocity into a folder thats on your PATH

set -e

# Just for style
RED_BOLD='\033[1;31m'
BLUE_BOLD='\033[1;36m'
RESET='\033[0m'

# The velocity file needs to be next to this script
if [ ! -f "./velocity" ]; then
  echo "${RED_BOLD}ERROR:${RESET} Cant find the velocity file, make sure its in the same folder as this script."
  exit 1
fi

# If you ran this with sudo it installs for everyone, otherwise just for you
if [ "$(id -u)" -eq 0 ]; then
  target="/usr/local/bin"
else
  target="$HOME/.local/bin"
  mkdir -p "$target"
fi

install -m 755 ./velocity "$target/velocity"
echo "${BLUE_BOLD}INFO:${RESET} installed velocity into $target"

# Some distros dont have ~/.local/bin on the PATH, so this checks for that
case ":$PATH:" in
  *":$target:"*) ;;
  *) echo "${BLUE_BOLD}INFO:${RESET} $target is not on your PATH yet, you might need to log out and back in." ;;
esac