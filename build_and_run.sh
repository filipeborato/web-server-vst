#!/bin/bash

# Construir a imagem Docker
docker build -t audio_processing_api .

# Executar o contêiner Docker
docker run -d --name audio_processing_api \
    -p 80:80 \
    audio_processing_api
