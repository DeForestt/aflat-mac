#!/bin/bash

# Check for git 
if ! [ -x "$(command -v git)" ]; then
  echo 'Error: git is not installed.' >&2
  exit 1
fi

# Check for cmake
if ! [ -x "$(command -v cmake)" ]; then
  echo 'Error: cmake is not installed.' >&2
  exit 1
fi


# Check if aflat is in the PATH
if ! [[ $AFLAT_INSTALLED == "SET" ]]; then
    echo 'Adding aflat to PATH'
    echo 'export PATH=$PATH:~/.aflat/aflat/bin' >> ~/.bashrc
    echo 'export AFLAT_INSTALLED=SET' >> ~/.bashrc
    echo 'export PATH=$PATH:~/.aflat/aflat/bin' >> ~/.zshrc
    echo 'export AFLAT_INSTALLED=SET' >> ~/.zshrc
fi

# Check for ~/.aflat directory
if [ -d ~/.aflat ]; then
  echo 'Remove ~/.aflat directory'
  rm -rf ~/.aflat
fi


# Cd into the aflat directory
mkdir ~/.aflat
(cd ~/.aflat && git clone https://github.com/DeForestt/aflat.git)

# Build aflat
(cd ~/.aflat/aflat && mkdir bin && mkdir build && cd build && cmake ../. && cd .. && cmake --build build> buildLog.log)

# Build libs
(cd ~/.aflat/aflat && bash rebuild-libs.sh)

echo "Successfully installed aflat!"