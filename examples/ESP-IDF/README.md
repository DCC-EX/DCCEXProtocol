# ESP-IDF Examples

This directory contains a standalone ESP-IDF project for DCCEXProtocol examples.

## Layout

- `main/` contains the application entry point.
- `main/` also contains the first example application and its local configuration template.
- Additional ESP-IDF examples can be added as sibling projects beside this directory.

## Build

From this directory, run:

```sh
idf.py set-target esp32
idf.py build
idf.py flash monitor
```

The project consumes the local library from the repository root via
`EXTRA_COMPONENT_DIRS`, so changes to the main DCCEXProtocol source are picked
up directly. The generated `sdkconfig` file is intentionally not committed;
`idf.py set-target` creates the target-specific configuration locally.
