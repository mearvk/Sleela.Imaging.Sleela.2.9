# Conversion decisions

1. Keep the organizer local-first. No background network activity is part of the core.
2. Treat extension recognition and content/metadata parsing as separate capabilities.
3. Prefer verified embedded capture time, then a valid filesystem modification timestamp; otherwise retain an explicit unknown value.
4. Never fabricate a timestamp to make a file eligible.
5. Default to dry-run and preserve every source.
6. Never overwrite an existing destination silently.
7. Copy to a unique temporary destination, verify byte count and SHA-256, then publish with exclusive/no-overwrite semantics.
8. Keep OS-specific filesystem operations behind a SLeeLa VM/OS provider; do not imitate a provider with sentinel values.
9. Canonicalize paths and prove root separation with the provider; lexical string-prefix checks are insufficient.
10. Do not follow symbolic links in the current baseline.
11. Do not carry over key transmission, remote certificate collection, entertainment polling, database logging, embedded credentials, or telemetry.
12. Do not claim compilation, runtime, or platform support until corresponding evidence exists.
13. Keep optional cloud, database, AI, or metadata-service integrations out of the core and require separate consent and security review.
14. Treat the protected final coding analysis as change-controlled engineering guidance, not as a claim that GitHub branch protection is active.
