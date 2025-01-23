# Base Ubuntu
FROM ubuntu:22.04

RUN sed -i 's|http://archive.ubuntu.com/ubuntu|http://mirror.example.com/ubuntu|g' /etc/apt/sources.list && \
    apt-get update -o Acquire::ForceIPv4=true

# Instalar dependências necessárias
RUN apt-get -o Acquire::ForceIPv4=true install -y \
    xvfb \   
    libsndfile1-dev \
    nginx \
    supervisor \
    cmake \
    build-essential \
    uuid-runtime \
    curl    

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
