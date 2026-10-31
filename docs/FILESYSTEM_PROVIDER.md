# Native filesystem provider ABI

The executable provider is implemented in native/imaging_provider.cpp and exposed by native/imaging_provider.h as ABI version 1. CMake builds it as a shared library named sleela_imaging_provider and links OpenSSL Crypto for SHA-256.

## Contract
- Preview canonicalizes all six existing roots and rejects overlapping roots.
- Enumeration accepts supported regular media files and excludes symlinks. Recursive traversal does not descend through symlink entries.
- Files with unavailable timestamps are skipped, never assigned the current date.
- Preview produces a process-local plan, proposed destinations, and collision states.
- Apply requires a fresh plan, explicit confirmation, and SHA-256 enabled.
- Apply rechecks source type, size, and modification time; stages a copy beside the destination; verifies byte count and SHA-256; and publishes using atomic no-overwrite hard-link creation.
- Original files are never deleted or modified. No network, polling, upload, or metadata-transmission API is present.

## Build and test
CMake requires OpenSSL Crypto and Python 3. Build with the existing CMake instructions, then run CTest. The provider contract test uses synthetic files only.

## Integration boundary and limitations
The shared library is a native C ABI provider, not a registered SLeeLa VM provider. A SLeeLa native host adapter and UI callback binding are still required; until then ImagingFileSystem.sleela remains unbound and Apply remains disabled in the SLeeLa UI.

The provider uses filesystem modification time, not embedded EXIF/QuickTime capture time. Destination directories must exist. Hard-link publication is intentionally fail-closed on filesystems that do not support it. Source checks reduce stale-plan risk, but standard C++ filesystem paths do not provide race-free descriptor-relative no-follow operations on every platform; hostile concurrent filesystem mutation remains a platform-hardening requirement. The provider is synchronous and must not run on the UI thread for large collections.
