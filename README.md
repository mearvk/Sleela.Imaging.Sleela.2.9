# SLeeLa Imaging 2.9

The SLeeLa-native edition of the local media-organizing workflow from Java.Imaging.Java.21. The organizer is implemented entirely in SLeeLa and runs on the SLeeLa VM: it enumerates, classifies, dates, names, copies, and verifies media through the VM's own OS built-ins — there is no out-of-process native provider in the organizer path. A standalone C++17 filesystem companion remains in `native/` as optional reference material.

- Language target: SLeeLa syntax 2.9 (requires the source-built `sleela` compiler; the prebuilt 1.11 binary cannot run 2.x sources)
- Reference implementation: https://github.com/mearvk/Java.Imaging.Java.21
- SLeeLa runtime and toolkit: https://github.com/mearvk/SLeeLa
- Conversion record: conversion/
- Protected coding analysis: docs/PROTECTED_FINAL_CODING_ANALYSIS.md
- UI and workflow: docs/UI_WORKFLOW.md
- Native provider ABI: native/imaging_provider.h and native/imaging_provider.cpp
- Provider requirements and limitations: docs/FILESYSTEM_PROVIDER.md
- Architecture and safety contracts: docs/
- Test plan and fixtures: tests/
- Release gates: validation/

## Application layers

- `src/ImagingOrganizer.sleela`: single-file, runnable program (all classes inlined + a headless demo `main`). This is the executable form, because the SLeeLa runtime runs one merged translation unit.
- `src/*.sleela`: the same logic kept modular — classification, date policy, naming, record, configuration, safety policy, the native `ImagingFileSystem` provider, the two-stage session engine, and the top-level application.
- `src/ui/ImagingWindow.sleela`: native SleelaUI desktop workspace that drives the in-VM session directly (requires a native display).
- `tests/run-native-demo.sh` + `tests/make-fixture.sh`: build a deterministic fixture and verify preview + apply end to end on the real VM.
- `native/`: the earlier standalone C++17 companion and C ABI provider, retained as optional reference. The SLeeLa organizer does not depend on it.

## How the SLeeLa-native provider works

SLeeLa has no in-language string manipulation (no substring, split, case folding, or char access) and no built-ins for directory listing, modification time, SHA-256, calendar date formatting, or file copy. The organizer therefore keeps all decision logic in SLeeLa — eligibility, chronological collision-sequenced naming, date-source provenance, safety gating, and digest verification — and delegates only the unavoidable text/byte work to small, fixed POSIX shell commands launched through the VM `osRun` built-in:

- enumeration: `find -P ... ! -type l` (symlinks never followed), sorted by modification time;
- extension + case folding, modification time (`stat`), SHA-256 (`sha256sum`), and `yyyy-MM-dd_HH-mm-ss` formatting (`date`);
- binary-safe copy (`cp`) and no-overwrite publication (`mv -n` guarded by an existence check).

Every shell result is returned as a single scalar value (no trailing newline) written to a per-run work file and read back with `openFile`/`read`, so SLeeLa compares it with `==`. The VM's file-I/O built-ins are not binary-safe (NUL-truncating, 4 KiB reads), which is exactly why media bytes are copied by `cp` rather than streamed through SLeeLa.

## Running it

```sh
# source-built compiler that supports syntax 2.9
SLEELA=/path/to/SLeeLa
sh tests/make-fixture.sh                      # creates ./fixture
SLEELA_HOME=$SLEELA $SLEELA/impl/build/sleela run src/ImagingOrganizer.sleela

# full preview + verified-apply assertion harness
SLEELA_HOME=$SLEELA SLEELA_BIN=$SLEELA/impl/build/sleela tests/run-native-demo.sh
```

## Safety defaults

- Dry-run is enabled by default.
- SHA-256 verification is required for apply.
- Original media is never deleted or modified.
- Symbolic links are not followed during enumeration.
- Existing destinations are never silently overwritten.
- Apply requires a fresh preview, explicit confirmation, source revalidation, verified copy, and atomic no-overwrite publication.
- No key upload, remote certificate collection, database logging, network polling, telemetry, or media upload is included in the organizer core.
- Missing or malformed embedded capture dates fall back to filesystem modification time with explicit provenance; the application never invents the current date.

## Current implementation status

This is the SLeeLa-native original edition. The organizer compiles and runs on the source-built SLeeLa VM (`sleela run src/ImagingOrganizer.sleela`) and performs real work: `tests/run-native-demo.sh` builds a fixture, runs a read-only preview and a verified apply, and asserts that every eligible file is organized to the correct date-named destination, that an unsupported file is skipped, that a same-timestamp collision produces an `_002` name, that each published copy's SHA-256 equals its source, and that every original is preserved.

`ImagingFileSystem.sleela` is now a real provider backed by the VM OS built-ins (`osRun`/`osExists`/`osIsDir`/`osFileSize`/`osMakeDir`/`openFile`/`read`/`close` plus fixed POSIX shell helpers), not a facade. The two-stage reviewed plan binds each eligible source to its preview-time SHA-256 digest; apply recomputes the source digest (rejecting same-size/same-timestamp content substitution), verifies the staged copy against the reviewed digest, and publishes without replacing any existing file.

Honest limitations of the pure-SLeeLa path: capture-date provenance is filesystem modification time (no in-language EXIF/QuickTime parser is available, matching the Java reference's fallback; the application never invents the current date). The shell helpers are POSIX (Linux/macOS); Windows would need equivalent `cmd`/PowerShell helpers. No-overwrite publication is existence-guarded `mv -n` rather than a kernel-atomic O_EXCL primitive, because the VM exposes no exclusive-create built-in. Embedded-metadata dates, HEIF/HEIC item metadata, race-free descriptor-relative operations, asynchronous cancellation, and Windows runtime evidence remain open work.

## Native companion (optional reference)

The earlier C++17 companion and C ABI provider under `native/` are retained as reference material. They are NOT on the organizer's execution path in this edition; the SLeeLa program drives the filesystem itself. See docs/NATIVE_BUILD.md for building that companion.
