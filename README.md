# SLeeLa Imaging 2.9

A SLeeLa-native source port of the local media-organizing workflow from Java.Imaging.Java.21, with a standalone C++17 filesystem companion and a native SLeeLa desktop UI.

- Language target: SLeeLa syntax 2.9
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

- src/: SLeeLa domain model, configuration, date policy, naming, safety, and run-session logic.
- src/ui/ImagingWindow.sleela: native SleelaUI desktop workspace.
- native/imaging.cpp: standalone C++17 filesystem companion.
- native/imaging_provider.cpp: shared native C ABI filesystem provider, ABI version 1.
- native/imaging_provider.h: stable C-compatible interface.
- native/imaging_host_adapter.cpp/.h: explicit-path, cross-platform dynamic host adapter for the provider.
- native/HOST_ADAPTER_ABI.md: host-loader handoff and remaining VM binding contract.

## Safety defaults

- Dry-run is enabled by default.
- SHA-256 verification is required for apply.
- Original media is never deleted or modified.
- Symbolic links are not followed during enumeration.
- Existing destinations are never silently overwritten.
- Apply requires a fresh preview, explicit confirmation, source revalidation, verified copy, and atomic no-overwrite publication.
- No key upload, remote certificate collection, database logging, network polling, telemetry, or media upload is included in the organizer core.
- Missing timestamps remain unknown; the application does not invent current dates.

## Current implementation status

The native provider exists as a shared C ABI library and has a synthetic-fixture contract test. An explicit-path cross-platform dynamic host adapter now loads and validates provider ABI v1 and delegates preview/apply while preserving provider-side authorization. The adapter is not yet registered as a SLeeLa VM opcode/builtin: the VM-local handle table, compiler name/arity mapping, core dispatch, and UI callback binding remain outstanding. The SLeeLa application UI therefore continues to report the filesystem VM bridge as unbound and does not call the native provider directly. Do not enable Apply until that VM binding is implemented and tested.

The provider uses filesystem modification time only; embedded EXIF/QuickTime capture-time parsing, race-free descriptor-relative no-follow operations, asynchronous cancellation, and Windows/macOS runtime evidence remain open engineering work. See docs/FILESYSTEM_PROVIDER.md and validation/QUALITY_GATES.md.

## Native companion build

See docs/NATIVE_BUILD.md for build instructions. The provider uses the same dependency-free SHA-256 implementation as the existing native engine. A successful native build does not prove SLeeLa source compilation or VM integration.
