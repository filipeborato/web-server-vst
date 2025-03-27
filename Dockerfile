FROM ubuntu:22.04

RUN apt-get update
RUN apt-get install -y \
    apt-transport-https \
    ca-certificates \
    software-properties-common \
    xvfb \
    libsndfile1-dev \
    supervisor \
    cmake \
    build-essential \
    ffmpeg \
    curl \
    libxrandr2 \
  && apt-get clean

WORKDIR /app

COPY . .

RUN mkdir -p /var/log/supervisor
COPY supervisord.conf /etc/supervisor/supervisord.conf

# Configurar script de inicialização
COPY setup_environment_X.sh /app/setup_environment_X.sh
RUN chmod +x /app/setup_environment_X.sh

RUN mkdir -p /app/tmp 
RUN chmod +x /app/tmp

ENV LD_LIBRARY_PATH=""
ENV LD_LIBRARY_PATH="/app/vst:${LD_LIBRARY_PATH}"

# Atualize para a porta correta onde sua aplicação está rodando
EXPOSE 18080

ENTRYPOINT ["/app/setup_environment_X.sh"]
