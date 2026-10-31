# Architecture

## Layers

1. **Configuration:** source root, image/video destinations, recursion, dry-run, and verification preferences.
2. **Classification:** extension-to-category mapping only.
3. **Metadata resolution:** embedded JPEG/TIFF EXIF and QuickTime date extraction with explicit provenance and filesystem-mtime fallback.
4. **Planning:** candidate records with proposed destination, date provenance, and explicit state.
5. **Session policy:** root validation, preview freshness, confirmation, no-overwrite, and source-preservation rules.
6. **SLeeLa UI:** native SleelaUI window and controls; SLeeLa owns event meaning and workflow state.
7. **SLeeLa process bridge:** `ImagingWindow.sleela` invokes `sleela-imaging-bridge` through the VM's `osRun` / `OP_OS_RUN` process service.
8. **Native host and filesystem provider:** explicit-path dynamic loading, canonical-root validation, enumeration, preview-time digest capture, verified copy, and atomic no-overwrite publication.
9. **Reporting:** immutable machine-readable review manifest and local per-file report.

## Dependency direction

The active UI validates paths and constructs a restricted, quoted bridge command; it invokes that command through SLeeLa `osRun`. The bridge loads the host adapter by explicit library path, and the adapter loads the provider by explicit path. Native filesystem operations remain in the provider process. The `ImagingFileSystem.sleela` facade is not yet wired to those native calls.

## State progression

discovered -> classified -> dated -> planned -> copied-temporary -> verified -> published

Any error transitions to a recorded failure state. A file must not be reported as published before destination verification and publication both succeed. unknown-date, unsupported, collision, and provider-unavailable are explicit non-publishable outcomes.

## UI path

src/ui/ImagingWindow.sleela uses documented ui* built-ins to create a native SleelaUI window. Preview and Apply invoke the native bridge through `osRun`. Apply requires a successful preview, unchanged configuration and executable paths, SHA-256 enabled, Dry run disabled, and explicit confirmation. The bridge independently recomputes and compares the reviewed manifest.

## Current boundary

The active scan/preview/apply path is implemented as a VM-mediated process bridge: the SLeeLa UI calls `osRun`, the bridge calls the host adapter, and the adapter loads the native C ABI provider. This is not an in-process VM opcode binding; the SLeeLa `ImagingFileSystem` facade remains unbound. The provider now parses the supported embedded capture-time fields; vendor-specific metadata and platform-hardening work remain, and the exact branch has not yet been built or tested. Do not describe the feature as release-verified until the SLeeLa source, bridge, provider, and cross-platform gates pass.
