#!/bin/bash

set -e

# Configurações
XVFB_INIT_SCRIPT="/etc/init.d/xvfb"
XVFB_SERVICE_NAME="audio_processing_api.service"
PROJECT_DIR="/path/to/your/project" # Substitua pelo diretório real do projeto
BINARY_NAME="bin/AudioProcessingProject"

# Instala as dependências necessárias
echo "Instalando dependências necessárias..."
sudo apt-get update
sudo apt-get install -y xvfb x11-apps libx11-dev

# Criar o script init.d para o Xvfb
echo "Configurando o script init.d para o Xvfb..."
cat <<EOL | sudo tee $XVFB_INIT_SCRIPT > /dev/null
#!/bin/bash
### BEGIN INIT INFO
# Provides:          xvfb
# Required-Start:    \$remote_fs \$syslog
# Required-Stop:     \$remote_fs \$syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Start Xvfb at boot time
### END INIT INFO

XVFB=/usr/bin/Xvfb
XVFBARGS=":99 -screen 0 1024x768x24 -ac +extension GLX +render"
PIDFILE=/var/run/xvfb.pid

case "\$1" in
  start)
    echo "Starting Xvfb..."
    start-stop-daemon --start --quiet --pidfile \$PIDFILE --exec \$XVFB -- \$XVFBARGS &
    ;;
  stop)
    echo "Stopping Xvfb..."
    start-stop-daemon --stop --quiet --pidfile \$PIDFILE
    ;;
  restart)
    echo "Restarting Xvfb..."
    \$0 stop
    sleep 1
    \$0 start
    ;;
  *)
    echo "Usage: /etc/init.d/xvfb {start|stop|restart}"
    exit 1
    ;;
esac
exit 0
EOL

# Permissões e habilitação do script
echo "Configurando permissões para o script init.d do Xvfb..."
sudo chmod +x $XVFB_INIT_SCRIPT
sudo update-rc.d xvfb defaults

# Iniciar o Xvfb
echo "Iniciando o Xvfb..."
sudo service xvfb start

# Criar o serviço do AudioProcessingProject
echo "Criando o serviço $XVFB_SERVICE_NAME..."
cat <<EOL | sudo tee /etc/systemd/system/$XVFB_SERVICE_NAME > /dev/null
[Unit]
Description=Audio Processing API Service
After=network.target

[Service]
Type=simple
Environment=DISPLAY=:99
WorkingDirectory=$PROJECT_DIR
ExecStart=$PROJECT_DIR/$BINARY_NAME
Restart=always
User=ubuntu
Group=ubuntu

[Install]
WantedBy=multi-user.target
EOL

# Recarregar o daemon do systemd
echo "Recarregando o daemon do systemd..."
sudo systemctl daemon-reload

# Habilitar e iniciar o serviço
echo "Habilitando e iniciando o serviço $XVFB_SERVICE_NAME..."
sudo systemctl enable $XVFB_SERVICE_NAME
sudo systemctl start $XVFB_SERVICE_NAME

echo "Configuração concluída com sucesso!"
