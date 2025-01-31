#!/bin/bash

# Exemplo de script: setup_environment_X.sh

# Habilitar modo de debug opcional (comente se não for necessário)
#set -x

echo "Iniciando setup_environment_X..."

# 1. Iniciar o Xvfb em background na tela :99
#    -screen 0 1024x768x24   Define resolução e profundidade de cor
#    -ac                     Desativa o controle de acesso
#    +extension GLX          Habilita extensão GLX (caso plugins ou apps 3D precisem)
#    +render                 Habilita extensão de renderização
echo "Iniciando Xvfb na tela :99..."
Xvfb :99 -screen 0 1024x768x24 -ac +extension GLX +render &

# 2. Exportar DISPLAY para que aplicações usem o X virtual
export DISPLAY=:99
echo "DISPLAY definido como :99"

# 3. (Opcional) Iniciar serviços adicionais, como Nginx ou Supervisor (exemplo)
# echo "Iniciando Nginx..."
# service nginx start

# echo "Iniciando Supervisor..."
# service supervisor start

# 4. Rodar seu binário principal (exemplo: AudioProcessingProject)
# Observação: se este script for entrypoint de um contêiner, você pode deixá-lo em primeiro plano (não background).
echo "Iniciando AudioProcessingProject..."
/home/user/web-server-audio/build/bin/AudioProcessingProject /home/user/web-server-audio &

# 5. Manter o script em execução ou sair, dependendo do seu fluxo
# Se você quiser que o contêiner ou ambiente fique em execução, pode usar um comando que bloqueie:
echo "Setup concluído. Aguardando..."
tail -f /dev/null
