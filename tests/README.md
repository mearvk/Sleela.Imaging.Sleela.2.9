# Test strategy

Tests are split between pure domain tests and filesystem integration tests.

- `ImagingPolicyTests.sleela`: existing classifier/date/safety policy checks.
- `classification/`: extension case, aliases, unknown extensions, misleading filenames.
- `dates/`: metadata precedence, missing/invalid values, timezone and daylight-saving boundaries.
- `naming/`: invalid timestamps, Unicode, reserved names, collisions, extension normalization.
- `filesystem/`: canonical roots, symlinks, permissions, no-overwrite, digest verification.
- `recovery/`: interrupted staging, failed publication, retries, stale temporary files.
- `integration/`: full dry-run and confirmed copy flows on each OS.
- `fixtures/`: fixture manifest and instructions; use synthetic or licensed media only.

A test case is a requirement until it is executed by the relevant harness and its result is recorded. Do not label this suite passing merely because the files exist.
