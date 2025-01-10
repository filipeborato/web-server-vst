# Use uma imagem base com suporte a C++
FROM ubuntu:20.04

# Evitar interações durante a instalação
ENV DEBIAN_FRONTEND=noninteractive

# Atualizar e instalar dependências
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libsndfile1-dev \
    libssl-dev \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*

# Diretório de trabalho
WORKDIR /app

# Copiar arquivos de configuração e código
COPY CMakeLists.txt .
COPY src/ ./src/

# Construir o projeto
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make

# Expor a porta que o Crow utilizará
EXPOSE 18080

# Comando para rodar a aplicação
CMD ["./build/AudioProcessingAPI"]
