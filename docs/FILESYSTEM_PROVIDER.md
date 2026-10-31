# Native filesystem provider ABI

The provider is implemented in native/imaging_provider.cpp and exposed by native/imaging_provider.h as ABI version 1. CMake builds it as a shared library. SHA-256 reuses the project's dependency-free C++ implementation; no external crypto package is required.

## Contract
- Preview canonicalizes all six existing roots and rejects overlapping roots.
- Enumeration accepts supported regular media files and excludes symbolic links. Recursive traversal disables descent into symlink entries.
- Files with unavailable timestamps are skipped, never assigned the current date.
- Preview produces a process-local plan, proposed destinations, and collision states.
- Apply requires a fresh plan, explicit confirmation, and SHA-256 enabled.
- Apply rechecks source type, size, and modification time; stages a copy beside the destination; verifies byte count and SHA-256; and publishes using atomic no-overwrite hard-link creation.
- Original files are never deleted or modified. No network, polling, upload, or metadata-transmission API is present.

## Build and test
CMake requires OpenSSL Crypto and Python 3. Build using docs/NATIVE_BUILD.md, then run CTest. The provider contract test uses synthetic files only.

## SLeeLa integration boundary
The shared library is a native C ABI provider, not a registered SLeeLa VM provider. A SLeeLa native host adapter and UI callback binding are still required; until then ImagingFileSystem.sleela remains unbound and Apply remains disabled in the SLeeLa UI.

## Limitations
The provider uses filesystem modification time, not embedded EXIF/QuickTime capture time. Destination directories must exist. Hard-link publication is intentionally fail-closed on filesystems that do not support it. Source checks reduce stale-plan risk, but standard C++ filesystem paths do not provide race-free descriptor-relative no-follow operations on every platform; hostile concurrent filesystem mutation remains a platform-hardening requirement. The provider is synchronous and must not run on the UI thread for large collections.
