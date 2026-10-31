# Filesystem cases

Required integration cases:

- Canonical source and destination roots.
- Source/destination equality, nesting, and overlap rejection.
- Symlink and Windows reparse-point escape attempts.
- Permission denied and read-only destination.
- Existing destination and race-created collision.
- Short read/write, disk full, interrupted copy, and process interruption.
- Byte-count mismatch and digest mismatch.
- Same-volume atomic publication and cross-volume staged publication.
- Source content unchanged after success and every failure path.
- No-overwrite behavior under concurrent writers.
