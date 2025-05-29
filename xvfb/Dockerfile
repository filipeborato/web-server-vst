FROM ubuntu:22.04

RUN apt-get update && apt-get install -y xvfb x11-apps libx11-dev

# Configurar ambiente Xvfb
CMD ["Xvfb", ":99", "-screen", "0", "1024x768x24", "-ac"]