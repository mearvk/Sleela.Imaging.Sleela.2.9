# Behavior mapping

This document separates behavior preserved from implementation details intentionally changed.

| Original responsibility | SLeeLa destination | Conversion status | Evidence still required |
|---|---|---|---|
| Application entry/orchestration | `src/ImagingApplication.sleela` | Adapted | Full scan, sort, plan, apply integration |
| Image/video extension checks | `src/MediaClassifier.sleela` | Ported | Full extension matrix |
| Capture date resolution | `src/MediaDateResolver.sleela` | Adapted | Real metadata adapter tests |
| Filename construction | `src/MediaNaming.sleela` | Adapted | Locale, timezone, collision tests |
| Configuration model | `src/ImagingConfig.sleela` | Adapted | Parser, validation, persistence |
| File enumeration and copying | `src/ImagingFileSystem.sleela` | Deferred | Native bridge on all target OSes |
| Safe operation policy | `src/ImagingSafetyPolicy.sleela` | Adapted | Path containment and transaction tests |
| Network/key/certificate/database side effects | Not carried into core | Rejected | Any future proposal reviewed separately |

## Status meanings

- **Ported:** the original behavior is represented with equivalent intent.
- **Adapted:** the user-visible intent is retained but implementation or safety behavior changes.
- **Deferred:** a required implementation or verification step is not present.
- **Rejected:** intentionally excluded from this application core.

A source file existing in the destination does not by itself prove feature completeness.
