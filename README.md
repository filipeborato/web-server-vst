# web-server-vst

This project consists of an audio processing service that loads VST2 plugins to perform DSP (Digital Signal Processing) on ​​demand. You can send an audio file to the service, which loads the desired plugin, processes the audio and returns the result.

---

## Required Technologies and Configurations

1. **VST2 SDK 2.4**: Required to compile the host that loads VST plugins.
2. **libsndfile**: To read/write WAV, FLAC, and other file formats.
3. **Socket IO in C++**: (Optional) to communicate with clients. (If it is part of your workflow.)
4. **Network libraries** (depending on the sockets framework used).
5. **Xvfb** (when running on servers without a graphical interface): Allows you to emulate X display.
6. **lsp-plugins-vst** (package on Linux): If you use LSP plugins, this package installs graphical dependencies.

---

## Usage Instructions (Local)

1. **Install** the dependencies:
- vstsdk2.4 (headers and lib)
- libsndfile-dev (or `libsndfile1-dev`)
- and any other libraries your project needs.

2. **Compile** the project:
- Adjust your Makefile/CMake to include the VST2 SDK and libsndfile.
- Compile the main binary (e.g. `AudioProcessingProject`).

3. **Run** the binary:
```bash
./AudioProcessingProject /path/to/config
```
- It will start a server that listens for audio requests.

4. **Send** audio:
- Via HTTP, cURL or other method, send an audio file to be processed.
- The service loads the specified VST2 plugin and processes the file.

5. **Receive** the processed audio.

---
## Execution via Docker

### 1. Dockerfile

Simplified example:
```dockerfile
FROM ubuntu:22.04

# Enable repositories and install dependencies
RUN apt-get update && apt-get install -y \
libsndfile1-dev \
# Packages to run LSP, Xvfb etc. plugins.
xvfb \
lsp-plugins-vst \
# ... and any other libs
&& apt-get clean

WORKDIR /app

COPY . .

# Optional: Add Xvfb script
# COPY setup_environment_X.sh /app/setup_environment_X.sh
# RUN chmod +x /app/setup_environment_X.sh

# Compile or copy the main binary
RUN make # or cmake . && make

EXPOSE 18080 # Or the port your project uses

CMD ["./AudioProcessingProject"]
```

### 2. Build the Image

```bash
docker build -t web-server-vst . ```

### 3. Run the Container

```bash
docker run -d --name audio_processing_api \
-p 80:18080 \ # maps host port 80 to container port 18080
web-server-vst
```

- If your binary listens on port 18080 inside the container, map `-p 80:18080`.
- If you need **Xvfb**, set up a script that starts Xvfb before running `AudioProcessingProject`.

### 4. Send Requests

- Now, access via `http://localhost:80/process` (or the route you defined) and send the audio file.

---

## Step by Step

1. **Install** Docker.
2. **Create** a Dockerfile including all dependencies (libsndfile, vstsdk2.4, etc.).
3. **Build** (or copy) the binary into the container.
4. **Expose** the port of your service.
5. **Run** the container with `docker run -d -p <host-port>:<container-port> image`.
6. **Test** by sending audio to the endpoint.

---

## Notes
- External VST2 plugins (like LSP) may require graphical libraries (Cairo, Fontconfig) and be in `/usr/lib/lsp-plugins`. Adjust `LD_LIBRARY_PATH` if necessary.
- If a plugin doesn't work, use `ldd plugin.so` or `strace` to see which libraries it tries to load.
- Xvfb is useful if the plugin requires X for interfacing.

---

**Contact**: For more details, check service logs and startup scripts.
