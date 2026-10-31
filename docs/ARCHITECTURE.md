# Architecture

## Layers

1. **Configuration:** source root, image/video destinations, recursion, dry-run, and verification preferences.
2. **Classification:** extension-to-category mapping only.
3. **Metadata resolution:** capture-time selection through a future format-aware adapter.
4. **Planning:** immutable-intent candidate records with proposed destination and state.
5. **Safety policy:** root containment, confirmation, no-overwrite, and source-preservation rules.
6. **Filesystem bridge:** OS-specific enumeration, canonicalization, metadata reads, verified copy, atomic publication, and report output.
7. **Reporting:** machine-readable summary and per-file outcomes.

## Dependency direction

Domain logic must not call platform APIs directly. It calls the filesystem contract. The native bridge implements that contract for Linux, Windows, and macOS.

## State progression

`discovered -> classified -> dated -> planned -> copied-temporary -> verified -> published`

Any error transitions to a recorded failure state. A file must not be reported as published before destination verification and publication both succeed.

## Current boundary

The domain and planning source exists. The bridge is unbound, and the full state machine is a target architecture rather than a completed runtime workflow.
