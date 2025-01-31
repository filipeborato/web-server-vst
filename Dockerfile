FROM ubuntu:22.04

# Instalar dependências: apt-get update e instalar pacotes
RUN apt-get update && apt-get install -y \
    apt-transport-https \
    ca-certificates \
    software-properties-common \
    xvfb \
    libsndfile1-dev \
    nginx \
    supervisor \
    cmake \
    build-essential \
    # uuid-runtime pode estar em outro pacote (ver abaixo)
    lsp-plugins-vst \
    curl \
  && apt-get clean

# Definir diretório de trabalho no contêiner
WORKDIR /app

# Copiar arquivos do projeto para o contêiner
COPY . .

# Configurar entrada do Nginx
COPY nginx.conf /etc/nginx/nginx.conf

# Configurar script de inicialização
COPY setup_environment_X.sh /app/setup_environment_X.sh
RUN chmod +x /app/setup_environment_X.sh

# Configurar Supervisor para gerenciar serviços
RUN mkdir -p /var/log/supervisor
COPY supervisord.conf /etc/supervisor/supervisord.conf

# Expor as portas necessárias
EXPOSE 8080

# Comando de entrada para inicializar serviços
ENTRYPOINT ["/app/setup_environment_X.sh"]
