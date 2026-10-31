# SLeeLa Imaging 2.9

A SLeeLa-native source port of the local media-organizing workflow from Java.Imaging.Java.21, with a standalone C++17 filesystem companion and a native SLeeLa desktop UI.

- Language target: SLeeLa syntax 2.9
- Reference implementation: https://github.com/mearvk/Java.Imaging.Java.21
- SLeeLa runtime and toolkit: https://github.com/mearvk/SLeeLa
- Conversion record: conversion/
- Protected coding analysis: docs/PROTECTED_FINAL_CODING_ANALYSIS.md
- UI and workflow: docs/UI_WORKFLOW.md
- Native provider requirements: docs/FILESYSTEM_PROVIDER.md
- Provider ABI status: native/PROVIDER_ABI.md
- Architecture and safety contracts: docs/
- Test plan and fixtures: tests/
- Release gates: validation/

## Application layers

- src/: SLeeLa domain model, configuration, date policy, naming, safety, and run-session logic.
- src/ui/ImagingWindow.sleela: native SleelaUI desktop workspace, authored in SLeeLa through the documented ui* built-ins.
- native/imaging.cpp: standalone C++17 filesystem companion; it is not a registered SLeeLa VM provider.
- native/PROVIDER_ABI.md: native provider interface implementation status and integration boundary.
- docs/PROTECTED_FINAL_CODING_ANALYSIS.md: change-controlled security and coding decision record.

## Safety defaults

- Dry-run is enabled by default.
- SHA-256 verification is required by default.
- Original media is never deleted or modified.
- Symbolic links are not followed.
- Existing destinations are never silently overwritten.
- Apply is disabled until a complete preview, explicit confirmation, canonical root proof, digest verification, and atomic no-overwrite provider are available.
- No key upload, remote certificate collection, database logging, network polling, telemetry, or media upload is included in the organizer core.
- Missing timestamps remain unknown; the application does not invent current dates.

## Current implementation status

The SLeeLa application and UI source define the configuration, safety policy, session gates, and native-window workflow. The UI deliberately reports that the filesystem VM bridge is unbound and performs no scan or copy in that state. The existing C++ engine remains a separate executable and is not silently treated as the SLeeLa provider.

The native provider's required ABI and safety behavior are documented, but the provider implementation and SLeeLa host binding are not yet committed. SLeeLa source compilation, UI runtime testing, filesystem VM binding, embedded metadata parsing, recovery testing, and Windows/macOS verification remain release gates. See validation/QUALITY_GATES.md.

## Native companion build

See docs/NATIVE_BUILD.md for CMake build instructions, CLI usage, and native-engine limitations. Do not infer that a successful native build proves SLeeLa source compilation or VM integration.
