# Native CMake example

This example uses the same `Stream` interface as Arduino clients, but supplies
its own loopback implementation. It can be built without an Arduino core:

```sh
cmake -S . -B build -DDCCEX_BUILD_TESTS=OFF -DDCCEX_BUILD_EXAMPLES=ON
cmake --build build --target DCCEXProtocolNativeExample
build/DCCEXProtocolNativeExample
```

The example sends a server-version request, feeds a version response back
through the stream, and verifies the parsed result.
