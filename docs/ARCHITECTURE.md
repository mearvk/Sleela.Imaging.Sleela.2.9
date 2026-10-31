# Architecture

## Layers

1. **Configuration:** source root, image/video destinations, recursion, dry-run, and verification preferences.
2. **Classification:** extension-to-category mapping only.
3. **Metadata resolution:** capture-time selection through a future format-aware adapter.
4. **Planning:** candidate records with proposed destination, date provenance, and explicit state.
5. **Session policy:** root validation, preview freshness, confirmation, no-overwrite, and source-preservation rules.
6. **SLeeLa UI:** native SleelaUI window and controls; SLeeLa owns event meaning and workflow state.
7. **Filesystem provider:** OS-specific enumeration, canonicalization, metadata reads, verified copy, atomic no-overwrite publication, and local report output.
8. **Reporting:** machine-readable summary and per-file outcomes with path redaction.

## Dependency direction

Domain logic must not call platform APIs directly. It calls the filesystem contract. The native provider implements that contract for Linux, Windows 10+, and macOS. UI code calls SLeeLa session methods and never calls filesystem functions directly.

## State progression

discovered -> classified -> dated -> planned -> copied-temporary -> verified -> published

Any error transitions to a recorded failure state. A file must not be reported as published before destination verification and publication both succeed. unknown-date, unsupported, collision, and provider-unavailable are explicit non-publishable outcomes.

## UI path

src/ui/ImagingWindow.sleela uses the documented ui* built-ins to create a native SleelaUI window. The UI supports path entry, recursion, dry-run, digest verification, explicit confirmation, preview, and quit. Apply is disabled until the session reports every required gate satisfied.

## Current boundary

The SLeeLa domain, policy, and UI source now exist. The filesystem contract remains unbound and deliberately fails closed. The native C++ organizer is a standalone companion executable, not a registered SLeeLa VM filesystem provider. The complete scan/preview/apply state machine is therefore not yet a released end-to-end feature.
