#!/bin/bash

# Construir a imagem Docker
docker build -t audio_processing_api .

# Executar o contêiner Docker
docker run -d --name audio_processing_api \
    -p 80:18080 \
    audio_processing_api

g++ -g -O0 -I/home/filipec/projetos/web-server-vst/external/vstsdk2.4 \ 
    Source/TestProcessAudio.cpp Source/PluginHost.cpp -o test_process_audio
