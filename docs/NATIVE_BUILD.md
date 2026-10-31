# Native build

The project builds two C++17 targets: the existing standalone CLI and the shared C ABI provider library.

## Requirements

- CMake 3.16+
- C++17 compiler
- Python 3
- OpenSSL development headers and Crypto library

## Build and test

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release --parallel
    ctest --test-dir build -C Release --output-on-failure

The suite includes the CLI integration test and a native provider contract test using synthetic files. CI is manual-dispatch only; no CI was dispatched for this coding pass.

## Provider library

CMake builds the shared library target sleela_imaging_provider. Its public C ABI is in native/imaging_provider.h. The opaque provider handle supports version query, preview, and confirmed apply. See docs/FILESYSTEM_PROVIDER.md.

The shared library is not yet registered inside the SLeeLa VM. The SLeeLa filesystem source contract remains unbound until a native host adapter is implemented and runtime-tested.
