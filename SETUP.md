# Setup

## Cel
Przygotowanie środowiska dla pluginu VCV Rack 2: `MultiModeFilterLab`.

## Wymagania
- macOS/Linux shell
- `git`
- `make`
- kompilator C++17
- VCV Rack SDK (`RACK_DIR`)

## Ścieżki
Workspace:
- `/Users/lazuli/Documents/PROGRAMMING/VCV_PROGRAMMING/MULITI_MODE_FILTER`

Makefile próbuje kolejno:
1. `../Rack-SDK`
2. `/Users/lazuli/Documents/PROGRAMMING/TEENSY/KSZ_TEENSY_PLATFORMIO/Teensy_Chord_Gen/Rack-SDK`

Możesz nadpisać:
```bash
make -j4 RACK_DIR=/absolute/path/to/Rack-SDK
```
