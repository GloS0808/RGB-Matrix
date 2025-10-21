# RGB-Matrix
Raspberry Pi project driving a 32x32 RGB LED matrix using Adafruit RGB Matrix Bonnet

## Project Overview
This project contains multiple C++ scenes and animations for a 32x32 RGB LED matrix driven by a Raspberry Pi with an Adafruit RGB Matrix Bonnet. The code is built on the `rpi-rgb-led-matrix` library:cite[1] and includes various seasonal, holiday, and special event animations.
**Development Note:** This project utilized AI assistance during development. All generated code has been thoroughly tested and validated to ensure functionality and reliability.

## Hardware Setup
- **Controller**: Raspberry Pi 3
- **Display**: 32x32 RGB LED Matrix with HUB75 interface
- **Interface**: Adafruit RGB Matrix Bonnet:cite[4]

### Wiring & Configuration
Follow Adafruit's official hardware setup guide: [Adafruit RGB Matrix Bonnet for Raspberry Pi](https://learn.adafruit.com/adafruit-rgb-matrix-bonnet-for-raspberry-pi/overview)

**Important configuration notes**:
- For 64x64 matrices, use parameters like `--led-cols=64 --led-rows=64`
- Some panels may require specific multiplexing settings: `--led-multiplexing`
- If experiencing timing issues, try `--led-slowdown-gpio=2`

## Software Installation

1. **Install dependencies**:
```bash
sudo apt-get update
sudo apt install libgraphicsmagick++-dev libwebp-dev python2.7-dev python3-dev python3-pillow
