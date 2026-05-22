#!/bin/bash

set -e

echo " Removing old build directory..."
rm -rf build

echo "Creating build directory..."
mkdir build
cd build

echo "Configuring project..."
cmake ..

echo "Building project..."
make 

echo "Executing Test Cases..."
ctest
echo "Done!"