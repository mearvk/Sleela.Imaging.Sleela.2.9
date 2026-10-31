# Native filesystem provider ABI

The provider is implemented in native/imaging_provider.cpp and exposed by native/imaging_provider.h as ABI version 2. CMake builds it as a shared library. SHA-256 reuses the project's dependency-free C++ implementation; no external crypto package is required.

## Contract
- Preview canonicalizes all six existing roots and rejects overlapping roots.
- Enumeration accepts supported regular media files and excludes symbolic links. Recursive traversal disables descent into symlink entries.
- Files with unavailable timestamps are skipped, never assigned the current date.
- Preview produces a process-local plan, proposed destinations, and collision states.
- Apply requires a fully materialized fresh plan (not a size-query or undersized-buffer preview), dry_run=0, explicit confirmation, and SHA-256 enabled.
- Preview captures each source SHA-256 digest in the process-local plan and the reviewed manifest. Apply rechecks source type, size, modification time, and digest; stages a copy beside the destination; verifies byte count and the reviewed SHA-256; and publishes using atomic no-overwrite hard-link creation.
- Review manifests and reports are written to exclusively created temporary files and published without replacing any existing file. Existing files are protected regardless of their header contents.
- Original files are never deleted or modified. No network, polling, upload, or metadata-transmission API is present.

## Build and test
CMake requires Python 3; SHA-256 is implemented without an external crypto dependency. Build using docs/NATIVE_BUILD.md, then run CTest. The provider contract test uses synthetic files only.

## SLeeLa integration boundary
The shared library is a native C ABI provider loaded by `sleela-imaging-bridge`. The SLeeLa desktop UI invokes that bridge through the runtime's existing `osRun` process built-in. This is VM-mediated process integration, not a native VM opcode or in-process provider binding. `ImagingFileSystem.sleela` remains a design facade with unbound capability placeholders; the active UI uses the process bridge.

## Limitations
The provider uses filesystem modification time, not embedded EXIF/QuickTime capture time. Destination directories must exist. Hard-link publication is intentionally fail-closed on filesystems that do not support it. Source checks reduce stale-plan risk, but standard C++ filesystem paths do not provide race-free descriptor-relative no-follow operations on every platform; hostile concurrent filesystem mutation remains a platform-hardening requirement. The provider is synchronous and must not run on the UI thread for large collections.
