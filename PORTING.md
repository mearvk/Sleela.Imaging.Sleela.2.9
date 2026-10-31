# Porting notes

The Java application's useful workflow is retained: inspect a source folder, classify image/video extensions, resolve a date, sort chronologically, generate date-based names, and route media to destination folders.

Java pennywise.Main becomes ImagingApplication; ImageMetadataReader becomes MediaDateResolver plus a metadata bridge; XMLHandler becomes ImagingConfig; extension checks become MediaClassifier; date formatting becomes MediaNaming.

Date precedence: embedded capture timestamp, positive filesystem modification timestamp, then unknown (zero). No current timestamp is invented. The Java parser advertised JPEG/TIFF/HEIC/PNG but only dispatched JPEG and TIFF signatures; this port documents that boundary.

Operation is two-phase: plan and report; after explicit confirmation, copy to a temporary destination, verify byte count and digest, then atomically publish. Keep source intact. Cleanup is separate future work.

ImagingFileSystem defines the OS/VM boundary. Bind directory enumeration, canonical paths, metadata reads, copy/verification, atomic publication, and report writing. Until bound, this is source-level code plus integration contract, not a tested binary.

## Quality and traceability

- `conversion/CONVERSION_MANIFEST.json` records source-to-destination mappings and conversion status.
- `conversion/BEHAVIOR_MAPPING.md` distinguishes preserved behavior from unfinished work.
- `conversion/DECISIONS.md` records safety and design decisions.
- `conversion/KNOWN_DIFFERENCES.md` records known gaps.
- `docs/TRANSACTION_MODEL.md` defines the non-destructive copy/verify/publish sequence.
- `docs/FORMAT_SUPPORT.md` distinguishes extension recognition from tested metadata extraction.
- `tests/` contains behavioral and integration case definitions.
- `validation/` contains release gates, a checklist, and a cross-platform evidence matrix.

These documents define requirements; they do not constitute passing tests. Compilation, runtime behavior, filesystem integration, and platform compatibility remain unverified until evidence is recorded for the exact release commit.
