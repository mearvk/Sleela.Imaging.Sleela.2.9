# Behavior mapping

This document separates behavior preserved from implementation details intentionally changed.

| Original responsibility | SLeeLa destination | Conversion status | Evidence still required |
|---|---|---|---|
| Application entry/orchestration | src/ImagingApplication.sleela and src/ImagingSession.sleela | Adapted | Full scan, preview, apply integration |
| Image/video extension checks | src/MediaClassifier.sleela | Ported | Full extension matrix |
| Capture date resolution | src/MediaDateResolver.sleela | Adapted | Bounded metadata adapter tests |
| Filename construction | src/MediaNaming.sleela | Adapted | Locale, timezone, collision tests |
| Configuration model | src/ImagingConfig.sleela | Adapted | Secure parser, validation, migration |
| Filesystem enumeration and copying | src/ImagingFileSystem.sleela | Deferred | Native VM provider on all target OSes |
| Safe operation policy | src/ImagingSafetyPolicy.sleela | Adapted | Canonical path and transaction tests |
| Native desktop interface | src/ui/ImagingWindow.sleela | Added in SLeeLa | SLeeLa compile and three-platform render tests |
| Network key/certificate/database/entertainment behavior | Excluded from organizer core | Rejected | Separate opt-in design and security review if ever proposed |
| Source deletion after copy | Prohibited by SLeeLa policy | Rejected | Source-preservation and failure-injection tests |

## Status meanings

- **Ported:** the original behavior is represented with equivalent intent.
- **Adapted:** the user-visible intent is retained but implementation or safety behavior changes.
- **Deferred:** a required implementation or verification step is not present.
- **Rejected:** intentionally excluded from this application core.

A source file existing in the destination does not by itself prove feature completeness.
