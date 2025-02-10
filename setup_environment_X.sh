#!/bin/bash

echo "Iniciando Xvfb na tela :99..."
Xvfb :99 -screen 0 1024x768x24 -ac +extension GLX +render &
export DISPLAY=:99

echo "Iniciando AudioProcessingProject..."
DISPLAY=:99 ./build/bin/AudioProcessingProject /app

echo "Setup concluído. Aguardando..."
tail -f /dev/null