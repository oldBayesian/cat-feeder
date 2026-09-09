# Cat Feeder Arduino Setup

This project runs on an Adafruit Grand Central M4 Express and uses Arduino CLI
for reproducible compile and upload workflows.

## Prerequisites

- `arduino-cli` (installed)
- Adafruit Grand Central M4 Express
- USB cable + board connected

## One-time setup

From this folder:

```bash
arduino-cli core update-index --config-file arduino-cli.yaml
arduino-cli core install adafruit:samd --config-file arduino-cli.yaml
arduino-cli lib install "Adafruit Motor Shield V2 Library" --config-file arduino-cli.yaml
```

## Compile

Compile for the Grand Central M4 Express:

```bash
arduino-cli compile -b adafruit:samd:adafruit_grandcentral_m4 --build-path build/grandcentral_m4 --config-file arduino-cli.yaml .
```

## Detect serial port

```bash
arduino-cli board list --config-file arduino-cli.yaml
```

## Upload

The upload command requires a connected Grand Central M4 and a prior compile for
the same board.

Replace `/dev/ttyACM0` with the port reported on your system.

```bash
arduino-cli upload -b adafruit:samd:adafruit_grandcentral_m4 -p /dev/ttyACM0 --input-dir build/grandcentral_m4 --config-file arduino-cli.yaml .
```

## VS Code tasks

Run from Command Palette: `Tasks: Run Task`

- `Arduino: Init Toolchain`
- `Arduino: Install Adafruit SAMD Core`
- `Arduino: Install Required Libraries`
- `Arduino: Compile (Grand Central M4)`
- `Arduino: Board List`
- `Arduino: Upload (Grand Central M4 @ /dev/ttyACM0)`

Before uploading, edit the Grand Central upload task port in
`.vscode/tasks.json` to match the port reported by `Arduino: Board List`.

## Telemetry run

After uploading, open the serial port at `115200` baud and enter:

```text
ALLT
5
```

`ALLT` arms telemetry mode. The numeric command is the motor duration in
seconds. Each run emits 5 seconds of pre-motor data, the motor run, and 15
seconds of post-motor data as CSV. No sensor rows are emitted while telemetry
is idle.

## Legacy Metro support

The repository retains Metro 328 tasks for older hardware, but the current
sketch uses Grand Central M4 analog pin mappings. Use the Grand Central M4
workflow above for this project.

## Notes

- Arduino sketches require the `.ino` filename to match the folder name. In this repo, the sketch is `cat-feeder.ino`.
- Keep desktop C++ simulator files outside the sketch root. The simulator source is in `sim/sim_schedule.cpp`.
