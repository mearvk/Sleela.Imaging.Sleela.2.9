# Native filesystem provider ABI

The provider is implemented in `native/imaging_provider.cpp` and exposed by `native/imaging_provider.h` as ABI version 3. CMake builds it as a shared library. SHA-256 uses the project's dependency-free C++ implementation; no external crypto package is required.

## Contract
- Preview canonicalizes all six existing roots and rejects overlapping roots.
- Enumeration accepts supported regular media files and excludes symbolic links. Recursive traversal disables descent into symlink entries.
- Preview resolves an effective organizer timestamp from embedded metadata when supported, otherwise from filesystem modification time. The original modification time remains separately recorded for stale-source validation.
- JPEG/TIFF EXIF priority is DateTimeOriginal, DateTimeDigitized, then TIFF DateTime. QuickTime/ISO-BMFF priority is the `©day` date item, then `mvhd` creation time. Unsupported, malformed, or absent metadata falls back to filesystem mtime; it is never replaced with the current date.
- EXIF date strings with no offset are treated as local wall time and explicitly marked timezone-unknown. EXIF offset tags and QuickTime UTC/offset values are honored when present.
- Preview produces a process-local plan, proposed destinations, collision states, effective timestamp, and timestamp provenance.
- Apply requires a fully materialized fresh plan (not a size-query or undersized-buffer preview), dry_run=0, explicit confirmation, and SHA-256 enabled.
- Preview captures each source SHA-256 digest in the process-local plan and reviewed manifest. Apply rechecks source type, size, modification time, and digest; stages a copy beside the destination; verifies byte count and reviewed SHA-256; and publishes using atomic no-overwrite hard-link creation.
- Review manifests (format v3) bind effective timestamp and provenance as well as source/destination, size, modification time, and digest. Reports show the selected timestamp source.
- Review manifests and reports are written to exclusively created temporary files and published without replacing any existing file.
- Original files are never deleted or modified. No network, polling, upload, or metadata-transmission API is present.

## Build and test
CMake requires Python 3; SHA-256 is implemented without an external crypto dependency. Build using docs/NATIVE_BUILD.md, then run CTest. Metadata fixtures are synthetic and cover JPEG EXIF, TIFF EXIF, QuickTime `©day`, and malformed JPEG metadata.

## SLeeLa integration boundary
The shared library is a native C ABI provider loaded by `sleela-imaging-bridge`. The SLeeLa desktop UI invokes that bridge through the runtime's existing `osRun` process built-in. This is VM-mediated process integration, not a native VM opcode or in-process provider binding. `ImagingFileSystem.sleela` remains a design facade with unbound capability placeholders; the active UI uses the process bridge.

## Limitations
The initial parser supports the stated EXIF and QuickTime date fields only; it does not decode every vendor-specific metadata convention, HEIF/HEIC item metadata, AVI metadata, GPS, or camera-specific maker notes. Some QuickTime `mvhd` creation dates describe container creation rather than the exposure instant, so their provenance remains explicitly `quicktime-mvhd` and is lower priority than `©day`. Destination directories must exist. Hard-link publication is intentionally fail-closed on filesystems that do not support it. Standard C++ filesystem paths do not provide race-free descriptor-relative no-follow operations on every platform; hostile concurrent filesystem mutation remains a platform-hardening requirement. The provider is synchronous and must not run on the UI thread for large collections.
