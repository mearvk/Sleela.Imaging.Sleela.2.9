# Native build

The project builds the standalone organizer CLI, the C ABI v3 filesystem provider, the explicit-path host adapter, and the SLeeLa process bridge. All native targets use C++17.

## Requirements

- CMake 3.16+
- C++17 compiler (GCC, Clang, or MSVC)
- Python 3

## Build and test

Linux and macOS:

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --parallel
    ctest --test-dir build --output-on-failure

Windows (Visual Studio generator):

    cmake -S . -B build -A x64
    cmake --build build --config Release --parallel
    ctest --test-dir build -C Release --output-on-failure

The suite includes the CLI integration test, provider contract test, reviewed-plan bridge test, and synthetic metadata fixtures for JPEG EXIF, TIFF EXIF, QuickTime dates, and malformed metadata. Tests use synthetic files in temporary directories. The bridge test checks immutable plan/report publication, stale-plan rejection, and source preservation. The provider test checks a known SHA-256 vector and rejects a same-size content mutation after restoring the original modification timestamp.

CI is manual-dispatch only. No CI was dispatched for this coding pass. A source change or a local compile probe is not a substitute for a successful exact-commit build and CTest run on each supported OS.

## Native targets

- sleela-imaging: standalone organizer CLI.
- sleela_imaging_provider: shared C ABI v3 filesystem provider plus embedded metadata parser.
- sleela_imaging_host: explicit-path dynamic loader and ABI adapter.
- sleela-imaging-bridge: child-process bridge invoked by SLeeLa's existing osRun built-in.

The provider ABI is declared in native/imaging_provider.h. The bridge requires an explicit provider-library path; it does not search arbitrary process library paths. See docs/FILESYSTEM_PROVIDER.md and docs/SLEELA_VM_INTEGRATION.md.

## Integration boundary

The SLeeLa application invokes the bridge through the VM's existing OS process service. This is VM-mediated process integration, not an in-process VM native binding. The native provider is loaded by the bridge process, and no native pointer, library handle, or file descriptor crosses into SLeeLa values.
