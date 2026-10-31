# Native build

The first runnable filesystem engine is a portable C++17 executable that sits
beside the SLeeLa domain-model sources. It is deliberately not described as a
bound VM module: the existing SLeeLa filesystem bridge contract remains
unbound pending a supported VM/native-module binding.

## Build and test

Requirements: CMake 3.16+, a C++17 compiler, and Python 3.

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release --parallel
    ctest --test-dir build -C Release --output-on-failure

GitHub Actions runs the same configure/build/test sequence on Linux, Windows,
and macOS. A platform is verified only after its job passes.

## Run

Create the source and all five destination directories first. The six roots
must be canonical, distinct, and non-overlapping:

    ./build/sleela-imaging \
      --source /path/to/inbox --images /path/to/images \
      --mov /path/to/mov --mp4 /path/to/mp4 \
      --mpeg /path/to/mpeg --avi /path/to/avi --recursive

Dry-run is the default. To apply, pass both --apply and --confirm.
--report FILE creates a new CSV report and refuses to overwrite an existing
file. Reports must be outside the source and destination roots.

## First-build boundaries

- Dates currently come from filesystem modification time only. Embedded EXIF,
  QuickTime, and other capture-time metadata readers are not implemented yet.
  The CSV records filesystem-modified-time as the date source.
- Extension recognition does not assert metadata extraction support.
- Symlinks are skipped; recursively resolved paths must remain under the
  canonical source root.
- Copy is staged in the destination directory, checked for byte count and
  SHA-256 equality, and published by atomic hard-link creation. If the target
  filesystem cannot provide that no-overwrite publication operation, the item
  fails closed; no overwrite-prone fallback is attempted.
- The source is never deleted or modified. No network, telemetry, or shell
  command is used by the engine.
- The native executable and SLeeLa source policy classes are not yet connected
  through a registered VM bridge. Do not call the source-level bridge bound.

The current integration tests use synthetic bytes only. Real personal media
should not be used as a test fixture.
