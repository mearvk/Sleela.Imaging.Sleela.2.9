# Porting notes

The source workflow in Java.Imaging.Java.21 is to inspect a source folder, classify image/video extensions, resolve a date, sort chronologically, generate date-based names, and route media to destination folders.

The SLeeLa implementation separates the workflow into configuration, classification, date provenance, naming, planning/session state, safety policy, UI, and an OS filesystem provider.

## Behavior changes made for safety

- The Java entry points copy media and then delete the source without verifying the destination digest. SLeeLa preserves the source and requires byte-count/SHA-256 verification before no-overwrite publication.
- The configured Java entry point includes usage-triggered public-key transmission, remote key validation, delayed external certificate retrieval, GitHub/Ollama entertainment polling, and optional MySQL logging with a hard-coded credential. These behaviors are excluded from the local organizer core.
- The Java XML parser is not shown with external-entity hardening. Secure configuration loading remains a separate requirement.
- The visible Java metadata reader dispatches JPEG EXIF and TIFF, despite broader format claims. Metadata support is not inferred from extension recognition.
- When capture time and filesystem mtime are both unavailable, SLeeLa records unknown rather than inventing the current time.

## SLeeLa implementation

- src/ImagingConfig.sleela — typed local paths and safe defaults.
- src/MediaClassifier.sleela — extension classification only.
- src/MediaDateResolver.sleela — date precedence and provenance.
- src/MediaNaming.sleela — deterministic filename construction.
- src/MediaRecord.sleela — candidate record.
- src/ImagingSafetyPolicy.sleela — fail-closed apply and privacy rules.
- src/ImagingSession.sleela — preview and apply gates.
- src/ui/ImagingWindow.sleela — native SleelaUI workspace.
- src/ImagingFileSystem.sleela — provider contract, currently unbound.

The SLeeLa UI uses the documented ui* VM bridge. The filesystem provider does not yet have a real binding; it returns explicit BRIDGE_REQUIRED sentinels and cannot scan or publish files. The native C++ companion is not a substitute for that VM provider.

## Quality and traceability

- conversion/CONVERSION_MANIFEST.json records source-to-destination mappings and status.
- conversion/BEHAVIOR_MAPPING.md distinguishes preserved behavior from unfinished work.
- conversion/DECISIONS.md records safety and design decisions.
- conversion/KNOWN_DIFFERENCES.md records known gaps.
- docs/PROTECTED_FINAL_CODING_ANALYSIS.md is the change-controlled security/coding baseline.
- docs/UI_WORKFLOW.md describes the native UI.
- docs/TRANSACTION_MODEL.md defines the non-destructive copy/verify/publish target.
- docs/FORMAT_SUPPORT.md distinguishes extension recognition from metadata support.
- tests/ contains behavioral and native integration cases.
- validation/ contains release gates and platform evidence requirements.

These documents define requirements; they do not constitute passing tests. Keep SLeeLa compilation, UI rendering, VM provider binding, and platform evidence as separate gates.
