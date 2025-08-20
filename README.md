# KeyboardEmulator

# Testing
socat -d -d pty,raw,link=/tmp/ttyV1 pty,raw,link=/tmp/ttyV2

python3 controller_emulator.py --port /tmp/ttyV2 --baud 115200 --period 2 -v