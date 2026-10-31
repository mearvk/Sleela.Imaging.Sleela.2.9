# SLeeLa Imaging 2.9

A SLeeLa-native source port of the media-organizing core from [Java.Imaging.Java.21](https://github.com/mearvk/Java.Imaging.Java.21), with a first portable native filesystem engine.

- Language target: SLeeLa syntax 2.9
- Reference implementation: [SLeeLa](https://github.com/mearvk/SLeeLa)
- Conversion record: [conversion/](conversion/)
- Architecture and safety contracts: [docs/](docs/)
- Test plan and fixtures: [tests/](tests/)
- Release gates: [validation/](validation/)

## Build the native engine

See [docs/NATIVE_BUILD.md](docs/NATIVE_BUILD.md) for CMake instructions, CLI
usage, safety behavior, and current limitations. A Linux/Windows/macOS GitHub
Actions matrix builds and runs synthetic integration tests.

## Current implementation status

The repository now has a runnable C++17 filesystem engine alongside the SLeeLa
domain model and policy sources. The engine supports deterministic extension
classification, filesystem-mtime naming, dry-run planning, confirmed copy,
byte-count and SHA-256 verification, and no-overwrite publication. It does not
yet bind the SLeeLa VM filesystem bridge, and embedded capture-time metadata
readers remain unimplemented. Consult [docs/FORMAT_SUPPORT.md](docs/FORMAT_SUPPORT.md)
and [docs/NATIVE_BUILD.md](docs/NATIVE_BUILD.md).

## Safety defaults

- Dry-run by default; applying requires both --apply and --confirm.
- No source deletion or source mutation.
- No silent overwrite; publication fails closed if atomic no-overwrite creation is unavailable.
- No implicit network activity, telemetry, key upload, or certificate collection.
- No invented capture timestamps; the first native build labels filesystem mtime honestly.

## Quality status

The native C++ engine has been built and its synthetic integration test has
passed locally on Linux. Windows and macOS evidence is pending the CI matrix.
SLeeLa source compilation and VM bridge binding remain separate release gates;
a documented requirement or test case is not a passing test. See
[validation/QUALITY_GATES.md](validation/QUALITY_GATES.md).
