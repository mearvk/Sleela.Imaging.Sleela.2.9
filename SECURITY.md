# Security and privacy

The organizer handles personal photographs and videos. Core behavior is local and non-destructive.

- No network calls, telemetry, key uploads, or certificate harvesting.
- No embedded database credentials.
- Planning does not modify files.
- Applying requires explicit confirmation and destination verification.
- Source media is never deleted by the organizer.
- The OS adapter must canonicalize paths, reject unsafe roots, and avoid following symbolic links by default.
- Verify destination byte count and digest before atomic publication. If atomic publication is unavailable, report it and leave source untouched.
- Cloud sync, database logging, external metadata services, and AI integrations are optional, consent-based modules.
