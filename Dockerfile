FROM debian:bullseye-slim

# Instalar as dependências necessárias
RUN apt-get update && apt-get install -y \
    build-essential \
    libsndfile1-dev \
    libc6 \
    wget

# Instalar o glibc 2.38
WORKDIR /opt
RUN wget http://ftp.gnu.org/gnu/libc/glibc-2.38.tar.gz \
    && tar -xvf glibc-2.38.tar.gz \
    && cd glibc-2.38 \
    && mkdir build && cd build \
    && ../configure --prefix=/opt/glibc-2.38 \
    && make -j$(nproc) \
    && make install

# Configurar a variável de ambiente
ENV LD_LIBRARY_PATH=/opt/glibc-2.38/lib:$LD_LIBRARY_PATH

# Copiar os arquivos do projeto
WORKDIR /app
COPY . .

# Comando para executar o programa
CMD ["./bin/AudioProcessingProject"]
