# Porting notes

The Java application's useful workflow is retained: inspect a source folder, classify image/video extensions, resolve a date, sort chronologically, generate date-based names, and route media to destination folders.

Java pennywise.Main becomes ImagingApplication; ImageMetadataReader becomes MediaDateResolver plus a metadata bridge; XMLHandler becomes ImagingConfig; extension checks become MediaClassifier; date formatting becomes MediaNaming.

Date precedence: embedded capture timestamp, positive filesystem modification timestamp, then unknown (zero). No current timestamp is invented. The Java parser advertised JPEG/TIFF/HEIC/PNG but only dispatched JPEG and TIFF signatures; this port documents that boundary.

Operation is two-phase: plan and report; after explicit confirmation, copy to a temporary destination, verify byte count and digest, then atomically publish. Keep source intact. Cleanup is separate future work.

ImagingFileSystem defines the OS/VM boundary. Bind directory enumeration, canonical paths, metadata reads, copy/verification, atomic publication, and report writing. Until bound, this is source-level code plus integration contract, not a tested binary.
