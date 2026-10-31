# Conversion decisions

1. Keep the organizer local-first. No background network activity is part of the core.
2. Treat extension recognition and content/metadata parsing as separate capabilities.
3. Prefer embedded capture time, then a valid filesystem modification time; otherwise retain an explicit unknown value.
4. Never fabricate a timestamp to make a file eligible.
5. Default to dry-run and preserve every source.
6. Never overwrite an existing destination silently.
7. Copy to a temporary destination, verify content, then publish; report when the platform cannot provide the required atomic operation.
8. Keep OS-specific filesystem operations behind the SLeeLa VM/OS bridge.
9. Do not claim compilation, runtime, or platform support until corresponding evidence exists.
10. Keep optional cloud, database, AI, or metadata-service integrations out of the core and require separate consent and security review.
