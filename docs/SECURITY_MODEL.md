# Security model

## Assets and risks

The application handles private photographs, videos, filenames, directory structures, timestamps, and potentially location metadata. Threats include path traversal, symlink escapes, malicious metadata, parser vulnerabilities, accidental overwrite, partial copy, disk exhaustion, and unintended disclosure.

## Required controls

- Canonicalize source and destination paths using the OS provider and enforce containment after canonicalization.
- Reject source/destination overlap and prove all destination roots are mutually disjoint.
- Do not follow symbolic links; no user option may enable them in the current safety baseline.
- Treat metadata and filenames as untrusted input. Bound parser memory, nesting, recursion, and processing time.
- Do not collect GPS/location tags for date-based organization.
- Do not execute content or invoke external programs based on file metadata.
- Use unique temporary names and exclusive creation.
- Never overwrite existing destinations.
- Verify byte count and SHA-256 before publication.
- Keep source files unchanged; source deletion is prohibited by policy.
- Do not transmit media, metadata, filenames, paths, or hashes over a network.
- Redact sensitive paths and metadata from logs unless the user explicitly requests a detailed local report.

## Reference-code findings

The Java configured entry point includes optional remote key validation, usage-triggered public-key transmission, delayed external certificate retrieval, GitHub/Ollama entertainment polling, and MySQL logging with a hard-coded credential. These features are excluded from the SLeeLa organizer core. The Java XML parser is not shown with secure external-entity settings. The Java EXIF reader advertises formats beyond its implemented JPEG/TIFF dispatch and processes untrusted TIFF offsets/IFD pointers without a demonstrated fuzz/resource-limit suite.

The Java organizer copies and then deletes source media without destination digest verification. The SLeeLa transaction model intentionally preserves sources and requires verified, no-overwrite publication.

## Security review triggers

Any network feature, telemetry, cloud upload, external metadata lookup, database integration, credential handling, public-key/certificate behavior, or source-deletion feature requires a separate design, explicit opt-in, threat review, and tests. Such functionality must not be added to the organizer core by incidental porting.
