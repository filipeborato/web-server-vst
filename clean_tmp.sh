#!/bin/bash

# Diretório da pasta tmp do projeto
TMP_DIR="/root/projects/web-server-vst/tmp"

# Remover arquivos mais antigos que 7 dias
find "$TMP_DIR" -type f -cmin +1 -exec rm -f {} \;

echo "$(date): Limpeza de arquivos na pasta tmp concluída." >> /root/projects/web-server-vst/logs/cleanup.log
