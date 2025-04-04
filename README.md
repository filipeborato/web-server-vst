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

- **Post** request to **/process**
- parameter **plugin**: plugin name (string)
- parameter **p[n]**: plugin paramter (string/float)
- parameter **previewStartTime**: preview start time (string/float)
- parameter **preview**: audio preview flag (string/bool)
#### Curl example:
```curl
curl --location '127.0.0.1:18080/process?plugin=filter-stereo&preview=true&previewStartTime=0.08&p0=0.2&p1=0.444531&p2=0.444531&p3=0.66&p4=0.001000&p5=0.333302&p6=1.000000&p7=1&p8=1&p9=1&p10=1&p11=0.500000&p12=1&p13=1&p14=1&p15=0.001667&p16=0.250000&p17=0.500000&p18=0.200000' \
--form 'audio_file=@"/home/xuxuzinho/Downloads/Alesis-Sanctuary-QCard-Tines-Aahs-C4.wav"'
``` 
