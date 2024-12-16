#!/bin/bash

# Nome do serviço
SERVICE_NAME="xvfb"
DISPLAY_NUM=":99"
SCREEN_RESOLUTION="1024x768x24"

# Caminho para o arquivo de serviço
SERVICE_FILE="/etc/systemd/system/${SERVICE_NAME}.service"

# Verifica se o script está sendo executado como root
if [ "$EUID" -ne 0 ]; then
  echo "Por favor, execute como root ou usando sudo."
  exit 1
fi

echo "Criando o serviço $SERVICE_NAME..."

# Cria o arquivo de serviço do Xvfb
cat <<EOL > $SERVICE_FILE
[Unit]
Description=XVFB Service
After=network.target

[Service]
ExecStart=/usr/bin/Xvfb $DISPLAY_NUM -screen 0 $SCREEN_RESOLUTION
Restart=always

[Install]
WantedBy=default.target
EOL

echo "Serviço criado em $SERVICE_FILE."

# Habilita e inicia o serviço
echo "Habilitando e iniciando o serviço..."
sudo systemctl daemon-reload
sudo systemctl enable $SERVICE_NAME
sudo systemctl start $SERVICE_NAME

# Verifica o status do serviço
echo "Verificando o status do serviço..."
sudo systemctl status $SERVICE_NAME

echo "Configuração do $SERVICE_NAME concluída com sucesso!"

export PROJECT_DIR=/workspaces/web-server-vst