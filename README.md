# Cat Feeder Arduino Setup

This project uses Arduino CLI for reproducible compile/upload workflows.

## Prerequisites

- `arduino-cli` (installed)
- USB cable + board connected

## One-time setup

From this folder:

```bash
arduino-cli core update-index --config-file arduino-cli.yaml
arduino-cli core install adafruit:avr --config-file arduino-cli.yaml
arduino-cli core install adafruit:samd --config-file arduino-cli.yaml
arduino-cli lib install "Adafruit Motor Shield V2 Library" --config-file arduino-cli.yaml
```

If you only use the Metro 328, the `adafruit:samd` install is optional.

## Compile

Adafruit Metro (ATmega328):

```bash
arduino-cli compile -b adafruit:avr:metro --build-path build/metro328 --config-file arduino-cli.yaml .
```

Adafruit Grand Central M4 Express:

```bash
arduino-cli compile -b adafruit:samd:adafruit_grandcentral_m4 --build-path build/grandcentral_m4 --config-file arduino-cli.yaml .
```

## Detect serial port

```bash
arduino-cli board list --config-file arduino-cli.yaml
```

## Upload

Replace `/dev/cu.usbmodem101` with your actual port.

```bash
arduino-cli upload -b adafruit:avr:metro -p /dev/cu.usbmodem101 --config-file arduino-cli.yaml .
```

For Grand Central M4 Express:

```bash
arduino-cli upload -b adafruit:samd:adafruit_grandcentral_m4 -p /dev/cu.usbmodem101 --config-file arduino-cli.yaml .
```

## VS Code tasks

Run from Command Palette: `Tasks: Run Task`

- `Arduino: Init Toolchain`
- `Arduino: Install Adafruit AVR Core`
- `Arduino: Install Adafruit SAMD Core`
- `Arduino: Install Required Libraries`
- `Arduino: Compile (Metro 328)`
- `Arduino: Compile (Grand Central M4)`
- `Arduino: Board List`
- `Arduino: Upload (Metro 328 @ /dev/cu.usbmodem101)`
- `Arduino: Upload (Grand Central M4 @ /dev/cu.usbmodem101)`

Before upload, edit the upload task port in `.vscode/tasks.json` to match your board.

## Notes

- Arduino sketches require the `.ino` filename to match the folder name. In this repo, the sketch is `cat-feeder.ino`.
- Keep desktop C++ simulator files outside the sketch root. The simulator source is in `sim/sim_schedule.cpp`.
