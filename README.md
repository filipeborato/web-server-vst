# web-server-vst

Este projeto consiste em um serviço de processamento de áudio que carrega plugins VST2 para realizar DSP (Digital Signal Processing) sob demanda. Você pode enviar um arquivo de áudio para o serviço, que carrega o plugin desejado, processa o áudio e retorna o resultado.

---

## Tecnologias e Configurações Necessárias

1. **VST2 SDK 2.4**: Necessário para compilar o host que carrega plugins VST.
2. **libsndfile**: Para ler/escrever arquivos WAV, FLAC, e outros formatos.
3. **Socket IO em C++**: (Opcional) para comunicação com clientes. (Se for parte do seu fluxo.)
4. **Bibliotecas de rede** (conforme o framework de sockets utilizado).
5. **Xvfb** (quando rodar em servidores sem interface gráfica): Permite emular display X.
6. **lsp-plugins-vst** (pacote no Linux): Caso use plugins LSP, esse pacote instala dependências gráficas.

---

## Instruções de Uso (Local)

1. **Instale** as dependências:
   - vstsdk2.4 (headers e lib)
   - libsndfile-dev (ou `libsndfile1-dev`)
   - e qualquer outra biblioteca que seu projeto precise.

2. **Compile** o projeto:
   - Ajuste seu Makefile/CMake para incluir o VST2 SDK e libsndfile.
   - Compile o binário principal (ex.: `AudioProcessingProject`).

3. **Rode** o binário:
   ```bash
   ./AudioProcessingProject /caminho/para/config
   ```
   - Ele iniciará um servidor que escuta requisições de áudio.

4. **Envie** áudio:
   - Via HTTP, cURL ou outro método, envie arquivo de áudio para ser processado.
   - O serviço carrega o plugin VST2 especificado e processa o arquivo.

5. **Receba** o áudio processado.

---

## Execução via Docker

### 1. Dockerfile

Exemplo simplificado:
```dockerfile
FROM ubuntu:22.04

# Habilitar repositórios e instalar dependências
RUN apt-get update && apt-get install -y \
    libsndfile1-dev \
    # Pacotes para rodar plugins LSP, Xvfb etc.
    xvfb \
    lsp-plugins-vst \
    # ... e quaisquer outras libs
  && apt-get clean

WORKDIR /app

COPY . .

# Opcional: Adicione script de Xvfb
# COPY setup_environment_X.sh /app/setup_environment_X.sh
# RUN chmod +x /app/setup_environment_X.sh

# Compile ou copie o binário principal
RUN make  # ou cmake . && make

EXPOSE 18080  # Ou a porta que seu projeto usa

CMD ["./AudioProcessingProject"]
```

### 2. Build da Imagem

```bash
docker build -t web-server-vst .
```

### 3. Rodar o Contêiner

```bash
docker run -d --name audio_processing_api \
    -p 80:18080 \  # mapeia porta 80 do host para 18080 do contêiner
    web-server-vst
```

- Se seu binário escuta na porta 18080 dentro do contêiner, mapeie `-p 80:18080`.
- Se precisa de **Xvfb**, configure um script que inicie o Xvfb antes de rodar `AudioProcessingProject`.

### 4. Enviar Requisições

- Agora, acesse via `http://localhost:80/process` (ou rota que você definiu) e envie o arquivo de áudio.

---

## Passo a Passo

1. **Instale** Docker.
2. **Crie** um Dockerfile incluindo todas as dependências (libsndfile, vstsdk2.4, etc.).
3. **Compile** (ou copie) o binário para dentro do contêiner.
4. **Exponha** a porta do seu serviço.
5. **Rode** o contêiner com `docker run -d -p <host-port>:<container-port> imagem`.
6. **Teste** enviando áudio para o endpoint.

---

## Observações
- Plugins VST2 externos (como LSP) podem exigir bibliotecas gráficas (Cairo, Fontconfig) e estar em `/usr/lib/lsp-plugins`. Ajuste `LD_LIBRARY_PATH` se necessário.
- Se um plugin não funcionar, use `ldd plugin.so` ou `strace` para ver quais bibliotecas ele tenta carregar.
- Xvfb é útil se o plugin requisita X para interface.

---

**Contato**: Para mais detalhes, verifique logs do serviço e scripts de inicialização.

