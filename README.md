# SLeeLa Imaging 2.9

A SLeeLa-native source port of the media-organizing core from [Java.Imaging.Java.21](https://github.com/mearvk/Java.Imaging.Java.21).

- Language target: SLeeLa syntax 2.9
- Reference implementation: [SLeeLa](https://github.com/mearvk/SLeeLa)
- Conversion record: [conversion/](conversion/)
- Architecture and safety contracts: [docs/](docs/)
- Test plan and fixtures: [tests/](tests/)
- Release gates: [validation/](validation/)

## Current implementation status

This repository currently contains the source-level domain model, planning logic, policy checks, and an unbound filesystem bridge contract. It is **not yet an end-to-end executable**. Directory enumeration, canonical-path enforcement, metadata adapters, copy verification, atomic publication, and report writing still require native VM/OS bridge implementations and integration tests.

Extension classification is not proof of metadata support. Consult [FORMAT_SUPPORT.md](docs/FORMAT_SUPPORT.md).

## Safety defaults

- Dry-run by default.
- No source deletion.
- No silent overwrite.
- No implicit network activity, telemetry, key upload, or certificate collection.
- No invented capture timestamps.
- Apply only after explicit confirmation, successful copy verification, and destination publication checks.

## Quality status

The quality seed defines traceability, behavioral expectations, cross-platform requirements, and release gates. A documented requirement or test case is not a passing test. See [validation/QUALITY_GATES.md](validation/QUALITY_GATES.md) for the evidence required before release.
