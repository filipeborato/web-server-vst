#!/bin/bash

# Define the project directory
PROJECT_DIR="/home/filipe/projects/retro-vst/web-server-vst"

# Ensure tmp directory exists
mkdir -p "$PROJECT_DIR/tmp"

# Check if Xvfb is already running on display :99
if ! ps aux | grep -v grep | grep -q "Xvfb :99"; then
    echo "Starting Xvfb on display :99..."
    Xvfb :99 -screen 0 1024x768x24 -ac +extension GLX +render &
    # Give Xvfb a moment to start
    sleep 1
else
    echo "Xvfb is already running on display :99."
fi

# Export DISPLAY variable
export DISPLAY=:99
echo "DISPLAY is set to $DISPLAY"

# Start the web server
echo "Starting AudioProcessingProject..."
exec "$PROJECT_DIR/build/bin/AudioProcessingProject" "$PROJECT_DIR"
