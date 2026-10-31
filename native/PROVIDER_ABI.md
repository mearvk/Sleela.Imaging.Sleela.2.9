# Native provider ABI

Implemented in `native/imaging_provider.cpp` and declared in `native/imaging_provider.h` (ABI v3). CMake builds the shared library and synthetic-fixture contract tests.

ABI v2 added the 65-byte lowercase SHA-256 field to each preview item. ABI v3 adds the effective organizer timestamp in Unix nanoseconds and its provenance string plus a timezone-confidence flag, while retaining the original filesystem modification time separately for stale-source checks. The ABI version changes whenever the public `sli_item` layout changes so older hosts cannot misread the structure.

The provider uses `native/media_metadata.cpp` to read embedded capture dates from JPEG EXIF, TIFF EXIF, and QuickTime/ISO-BMFF MOV, MP4, and M4V containers. Date priority is EXIF DateTimeOriginal, DateTimeDigitized, then TIFF DateTime; QuickTime `©day` metadata takes priority over the movie-header creation time. When embedded metadata is absent or malformed, filesystem modification time is used and labeled `filesystem-mtime`. EXIF timestamps without an explicit offset are interpreted as local wall time and marked as timezone-unknown; QuickTime UTC timestamps retain known timezone provenance. Filesystem modification times are absolute instants and are marked timezone-known.

The review manifest (format v3) binds the effective organizer timestamp and provenance alongside paths, file size, modification time, and SHA-256. Thus a changed metadata date also changes the reviewed plan.

The provider is loaded by `sleela-imaging-bridge` through the explicit-path host adapter. The SLeeLa desktop UI invokes that bridge through the SLeeLa VM's existing `osRun` process built-in; this is VM-mediated process integration, not a native VM opcode or in-process provider binding.
