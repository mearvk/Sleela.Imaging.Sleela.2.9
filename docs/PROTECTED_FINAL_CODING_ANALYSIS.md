# PROTECTED FINAL CODING ANALYSIS
## SLeeLa Imaging 2.9 — security-reviewed implementation baseline

**Record class:** Protected engineering decision / change-controlled  
**Status:** Approved design baseline; implementation remains partially gated  
**Reference:** https://github.com/mearvk/Java.Imaging.Java.21  
**Target:** SLeeLa syntax 2.9 and the existing SleelaUI native toolkit

> **Protection meaning.** “Protected” means this is the change-controlled engineering baseline: changes to its safety decisions require an explicit review entry and matching tests. It does not claim GitHub branch protection, signed commits, or cryptographic document sealing is enabled.

## 1. Executive decision

The application is a local-first media organizer. SLeeLa owns configuration, policy, date precedence, naming, planning state, user confirmation, and audit decisions. SleelaUI owns native presentation. A narrow filesystem provider is responsible for canonical paths, directory enumeration, metadata reads, copy/verification, no-overwrite publication, and report output.

The safe default is **preview-only**. A copy operation may be enabled only after all of the following are true:

1. The user explicitly confirms the exact plan.
2. The user turns dry-run off.
3. A complete preview exists for the current configuration and source snapshot.
4. Source and destination roots have been canonicalized and proven disjoint.
5. The provider supports exclusive/no-overwrite publication.
6. The destination copy passes byte-count and SHA-256 verification.
7. The source is still present and unchanged.
8. The provider is bound, versioned, and validated on the current platform.

Source deletion is not part of the SLeeLa application. Network access, telemetry, key transmission, remote certificate collection, database logging, and AI polling are not part of the organizer core.

## 2. Reference implementation findings

The Java reference contains two entry points (src/Main.java and src/pennywise/Main.java) and additional optional handlers. They are not behaviorally identical.

### 2.1 Destructive file handling

Both entry points copy a media file and then delete the source. They do not establish destination byte-count equality or a digest match before deletion. A partial, corrupt, or otherwise incorrect destination can therefore be followed by loss of the original. The SLeeLa design intentionally changes this behavior: copy to a unique temporary file, verify it, publish without overwrite, and preserve the source.

### 2.2 Private-data egress and embedded credentials

The configured Java entry point wires usage tracking, remote public-key validation, delayed certificate retrieval, entertainment polling, a local Ollama endpoint, and optional MySQL logging. The README describes public-key transmission after a usage threshold and external certificate retrieval. The database handler contains a hard-coded credential and stores filenames and absolute source/destination paths.

These behaviors are excluded from the SLeeLa core. The application must not transmit photographs, videos, filenames, directory paths, timestamps, hashes, public keys, or other media-derived information. Any future integration requires a separate proposal, explicit user consent, independent credential management, threat review, and opt-in tests. It must not be enabled by a configuration default.

### 2.3 Configuration parsing

The Java XML handler constructs a default DocumentBuilderFactory without showing secure parser hardening. The SLeeLa baseline does not carry that parser forward. A future configuration reader must disable external entities and external DTD access, bound input size and nesting, validate a schema, and reject unknown security-sensitive options.

### 2.4 Metadata parser limits

The Java ImageMetadataReader advertises more formats than it actually dispatches: its visible implementation handles JPEG APP1/EXIF and TIFF signatures, not HEIC or PNG metadata. It reads untrusted offsets and counts from media bytes and recursively follows EXIF and GPS IFD pointers. It has no demonstrated parser fuzzing, resource limits, timezone policy, or malformed-file corpus.

The SLeeLa port therefore distinguishes extension classification from actual metadata parsing. No format may be advertised as metadata-supported until a bounded parser, malformed fixtures, real-format fixtures, and security review exist. GPS/location tags are not needed for date-based organization and should not be collected by default.

### 2.5 Timestamp fallback

The configured Java entry point falls back to the current date when the filesystem modification time is unavailable. That invents a timestamp and can silently misdate a file. SLeeLa uses an explicit unknown date source and leaves such a file out of an automatic date-based plan until the user resolves it.

### 2.6 Scope and collision behavior

The Java organizer scans only the immediate source directory in its visible entry points. The conversion makes recursion an explicit user option. The Java sequence counter is in-memory and does not by itself prove that a destination name is unused. The SLeeLa provider must check the actual destination and fail closed on any collision.

## 3. SLeeLa implementation boundary

| Responsibility | SLeeLa location | Authority |
|---|---|---|
| Configuration and user choices | src/ImagingConfig.sleela | SLeeLa |
| Application orchestration | src/ImagingApplication.sleela | SLeeLa |
| Per-run state and release gates | src/ImagingSession.sleela | SLeeLa |
| Extension classification | src/MediaClassifier.sleela | SLeeLa |
| Date precedence and provenance | src/MediaDateResolver.sleela | SLeeLa |
| Filename generation | src/MediaNaming.sleela | SLeeLa |
| Planned media record | src/MediaRecord.sleela | SLeeLa |
| Safety policy | src/ImagingSafetyPolicy.sleela | SLeeLa |
| Native window and controls | src/ui/ImagingWindow.sleela | SLeeLa intent + SleelaUI renderer |
| Filesystem operations | src/ImagingFileSystem.sleela contract | OS/VM provider; currently unbound |
| Existing native organizer | native/imaging.cpp | Standalone C++ companion, not a VM binding |

The UI uses the documented SLeeLa ui* built-ins and the SleelaUI C ABI. It is not a mock page. The filesystem contract remains explicitly unbound: no SLeeLa source method may pretend that it enumerated or changed the user's files.

## 4. Data and transaction rules

- Classify by extension only; never infer that an extension proves valid media content.
- Resolve dates in this order: verified embedded capture time, valid filesystem modification time, unknown.
- Retain date source and timezone/provenance alongside the selected value.
- Never substitute current wall-clock time for missing metadata.
- Keep source paths canonical and within the configured source root.
- Reject source/destination overlap and symlink escapes after canonicalization.
- Treat metadata and filenames as untrusted input.
- Bound parser bytes, nesting, recursion, and processing time.
- Never execute file contents or metadata-derived commands.
- Use exclusive temporary creation and no-overwrite final publication.
- Verify byte count and SHA-256 before publication.
- Never delete or rewrite source media.
- Keep reports local and redact absolute paths unless the user requests a detailed local report.

## 5. UI behavior

The first native window provides source and destination inputs, recursion, dry-run, digest-verification, an explicit confirmation control, Preview, Apply, and Quit. The default is dry-run enabled. Apply remains disabled until the provider, canonical-root proof, preview, digest verification, and no-overwrite capability are all available.

With the current repository state, Preview reports that the filesystem VM bridge is unbound. It does not scan. Apply is disabled and the session policy denies it. This is an intentional safe boundary, not a completed end-to-end organizer.

## 6. Release gates

| Gate | Required proof | Current result |
|---|---|---|
| SLeeLa source syntax | Compile all new .sleela files with the target 2.9 compiler | NOT RUN |
| SleelaUI rendering | Run the window on Linux, Windows 10+, and macOS | NOT RUN |
| Filesystem VM provider | Bound provider with canonicalization and disjoint-root tests | BLOCKED / unbound |
| Metadata support | Format-specific parser and malformed/real fixture suites | BLOCKED / not implemented |
| Preview determinism | Same source/config snapshot yields same plan | NOT RUN |
| Apply transaction | Injected failure, verification, collision, and recovery tests | NOT RUN |
| Native companion | Build and CTest for the exact reviewed commit | NOT RUN in this coding pass |
| Cross-platform | Exact-commit results on all three OSes | NOT RUN |
| Privacy/security review | Review report, redaction checks, no-egress tests | NOT RUN |

A documentation statement is not a passing test. A local prototype result is not evidence for a later commit. Every PASS must identify the exact commit SHA, toolchain, platform, and test output.

## 7. Change-control checklist

- [ ] A reason and a named behavior being changed.
- [ ] A source-to-target mapping update.
- [ ] A threat-model impact assessment.
- [ ] A test that fails before the change and passes after it.
- [ ] Confirmation that source preservation and no-overwrite behavior remain intact.
- [ ] An explicit review of private-data egress and log redaction.
- [ ] Exact-commit evidence for every claimed platform or compiler result.

**Final decision:** retain the SLeeLa-native UI and policy/application layer; keep the native filesystem provider as a release blocker until it is genuinely bound and tested. Do not reintroduce the Java network, database, key, certificate, or source-deletion behavior into this application core.
