#!/bin/bash

# Exit on error
set -e

# Update and install basic tools
echo "Updating packages and installing required tools..."
sudo apt-get update -y
sudo apt-get install -y build-essential cmake git wget tar libsndfile1-dev

# Set up directories
PROJECT_DIR=$(pwd)
EXTERNAL_DIR="$PROJECT_DIR/external"
VST_SDK_DIR="$EXTERNAL_DIR/vstsdk2.4"
SNDFILE_DIR="$EXTERNAL_DIR/libsndfile"

# Ensure external directory exists
mkdir -p "$EXTERNAL_DIR"

# Download the VST2.4 SDK if not already present
if [ ! -d "$VST_SDK_DIR" ]; then
    echo "Downloading VST2.4 SDK..."
    git clone https://github.com/steinbergmedia/vst2sdk.git "$VST_SDK_DIR"
else
    echo "VST2.4 SDK already exists, skipping download."
fi

# Download libsndfile if not already present

# Create build directory if not exists
BUILD_DIR="$PROJECT_DIR/build"
mkdir -p "$BUILD_DIR"

# Move to the build directory and run CMake
cd "$BUILD_DIR"
echo "Running CMake..."
cmake .. -DLIBSNDFILE_DIR="$SNDFILE_DIR/build"
echo "Building the project..."
make

# Notify success
echo "Build complete. Executable can be found in the bin/ directory."
