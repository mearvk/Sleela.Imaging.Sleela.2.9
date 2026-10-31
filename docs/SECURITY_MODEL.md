# Security model

## Assets and risks

The application handles private photographs, videos, filenames, directory structures, timestamps, and potentially location metadata. Threats include path traversal, symlink escapes, malicious metadata, archive/container parser vulnerabilities, accidental overwrite, partial copy, disk exhaustion, and unintended disclosure.

## Required controls

- Canonicalize source and destination paths using the OS bridge and enforce containment after canonicalization.
- Reject source/destination overlap unless a separately reviewed workflow explicitly supports it.
- Do not follow symbolic links by default; detect loops and re-check resolved targets if opt-in is ever added.
- Treat metadata as untrusted input. Bound parser memory, recursion, and processing time.
- Do not execute content or invoke external programs based on file metadata.
- Use unique temporary names and exclusive creation.
- Never overwrite existing destinations.
- Verify byte count and SHA-256 before publication.
- Keep source files unchanged.
- Do not transmit media, metadata, filenames, or hashes over a network by default.
- Redact sensitive paths and metadata from logs unless the user explicitly requests a detailed report.

## Security review triggers

Any network feature, telemetry, cloud upload, external metadata lookup, database integration, credential handling, or source-deletion feature requires a separate design, explicit opt-in, threat review, and tests.
