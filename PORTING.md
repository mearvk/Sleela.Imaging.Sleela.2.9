# Porting notes

The Java application's useful workflow is retained: inspect a source folder, classify image/video extensions, resolve a date, sort chronologically, generate date-based names, and route media to destination folders.

Java pennywise.Main becomes ImagingApplication; ImageMetadataReader becomes MediaDateResolver plus a metadata bridge; XMLHandler becomes ImagingConfig; extension checks become MediaClassifier; date formatting becomes MediaNaming.

Date precedence in the planned SLeeLa model: embedded capture timestamp, positive filesystem modification timestamp, then unknown (zero). No current timestamp is invented. The Java parser advertised JPEG/TIFF/HEIC/PNG but only dispatched JPEG and TIFF signatures; this port documents that boundary.

## Native build status

A first portable C++17 filesystem engine is now checked in under native/.
It provides the runnable discovery, classification, mtime-based naming,
dry-run, confirmed copy, byte-count/SHA-256 verification, no-overwrite publish,
and CSV reporting path. Build and test instructions are in
docs/NATIVE_BUILD.md.

This is a native companion engine, not yet a registered SLeeLa VM filesystem
module. src/ImagingFileSystem.sleela remains an unbound contract. The native
engine currently uses filesystem modification time only; embedded EXIF and
QuickTime capture-time parsing remain future work.

## Quality and traceability

- conversion/CONVERSION_MANIFEST.json records source-to-destination mappings and conversion status.
- conversion/BEHAVIOR_MAPPING.md distinguishes preserved behavior from unfinished work.
- conversion/DECISIONS.md records safety and design decisions.
- conversion/KNOWN_DIFFERENCES.md records known gaps.
- docs/TRANSACTION_MODEL.md defines the non-destructive copy/verify/publish sequence.
- docs/FORMAT_SUPPORT.md distinguishes extension recognition from tested metadata extraction.
- tests/ contains behavioral and integration case definitions plus the synthetic native integration test.
- validation/ contains release gates, a checklist, and a cross-platform evidence matrix.

These documents define requirements; they do not constitute passing tests. Keep
SLeeLa source compilation, VM bridge binding, and Windows/macOS CI evidence as
separate gates.
