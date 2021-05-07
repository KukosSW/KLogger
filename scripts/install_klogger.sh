#!/bin/bash

# Author: Michal Kukowski
# email: michalkukowski10@gmail.com

# Full path of this script
THIS_DIR=`readlink -f "${BASH_SOURCE[0]}" 2>/dev/null||echo $0`

# This directory path
DIR=`dirname "${THIS_DIR}"`

# Chose your install directory, or install to default path
DEFAULT_DIR=~/klogger
lib_dir=

# Check existing of argument
if [ ! -z "$1" ]; then
    lib_dir=$1
else
    lib_dir=$DEFAULT_DIR
fi

echo "DIR = $lib_dir"

# Compile KLogger
cd "$DIR"
cd ../
make lib

# Now we have compiled klogger into libklogger.a we need also a inc/ directory

# Create dirs and copy required files
echo "Installing klogger to $lib_dir ..."
mkdir -p "$lib_dir"
cp ./libklogger.a $lib_dir/
cp -R ./inc/ $lib_dir

echo "DONE"