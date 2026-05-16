# ESP-IDF Examples

This directory contains a standalone ESP-IDF project for DCCEXProtocol examples.

## Layout

- `main/` contains the application entry point.
- `DCCEXProtocol_Basic/` contains the first example as its own ESP-IDF component.
- Additional ESP-IDF examples should be added as sibling component directories beside `DCCEXProtocol_Basic/`.

## Build

From this directory, run:

```sh
idf.py set-target esp32
idf.py build
idf.py flash monitor
```

The project consumes the local library from the repository root via `EXTRA_COMPONENT_DIRS`, so changes to the main DCCEXProtocol source are picked up directly.