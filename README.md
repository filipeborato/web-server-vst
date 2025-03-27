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

### 1. Docker-compose

Run:
```
docker-compose up --build
```

### 2. Send Requests

- Now, access via `http://localhost:80/process` (or the route you defined) and send the audio file.

**Contact**: For more details, check service logs and startup scripts.
