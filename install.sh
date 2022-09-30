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
if ! [ -x "$(command -v aflat)" ]; then

  echo 'Exporting PATH'
  PATH=$PATH:~/.aflat/aflat/bin

  echo 'Adding aflat to PATH'
  # Check if .bashrc exists
  if [ -f ~/.bashrc ]; then
    echo 'export PATH=$PATH:~/.aflat/aflat/bin' >> ~/.bashrc
  fi
  # Check if .zshrc exists
  if [ -f ~/.zshrc ]; then
    echo 'export PATH=$PATH:~/.aflat/aflat/bin' >> ~/.zshrc
  fi
fi

# Check for ~/.aflat directory
if [ -d ~/.aflat ]; then
  (cd ~/.aflat/aflat && git pull && cd build && cmake . && cd .. && cmake --build build && bash rebuild-libs.sh)
  echo 'aflat updated'
  exit
fi

# Cd into the aflat directory
mkdir ~/.aflat
(cd ~/.aflat && git clone https://github.com/DeForestt/aflat.git)

# Build aflat
(cd ~/.aflat/aflat && mkdir bin && mkdir build && cd build && cmake ../. && cd .. && cmake --build build> buildLog.log)

# Build libs
(cd ~/.aflat/aflat && bash rebuild-libs.sh)

echo 'Done Please restart your terminal or run source ~/.bashrc or source ~/.zshrc'
