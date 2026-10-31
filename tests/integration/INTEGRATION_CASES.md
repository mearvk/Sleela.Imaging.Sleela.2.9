# Integration cases

For Linux, Windows 10+, and macOS:

1. Run a dry-run against a synthetic directory tree and verify zero filesystem mutations.
2. Confirm the plan, apply to a separate destination, and verify every published digest.
3. Inject an error and verify source preservation and recovery reporting.
4. Repeat the same plan to verify idempotence and collision handling.
5. Exercise Unicode paths, nested directories, and unsupported formats.

Record OS version, filesystem type, SLeeLa compiler/runtime versions, bridge version, test command, exit status, and artifact/report location.
