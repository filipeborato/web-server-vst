FROM ubuntu:22.04

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
    lsp-plugins-vst \   
    curl \
    libxrandr2 \      
  && apt-get clean

WORKDIR /app

COPY . .

COPY nginx.conf /etc/nginx/nginx.conf

RUN mkdir -p /var/log/supervisor
COPY supervisord.conf /etc/supervisor/supervisord.conf

# Configurar script de inicialização
COPY setup_environment_X.sh /app/setup_environment_X.sh
RUN chmod +x /app/setup_environment_X.sh

RUN mkdir -p /app/tmp 
RUN chmod +x /app/tmp

ENV LD_LIBRARY_PATH=""
ENV LD_LIBRARY_PATH="/app/vst:${LD_LIBRARY_PATH}"

EXPOSE 8080

ENTRYPOINT ["/app/setup_environment_X.sh"]
