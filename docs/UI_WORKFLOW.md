# SLeeLa Imaging UI and workflow

## Entry point

src/ui/ImagingWindow.sleela is the SLeeLa desktop entry point. It creates a native SleelaUI application/window through the documented ui* VM built-ins. The layout uses a vertical page, grouped source/destination controls, checkboxes, status text, and Preview / Apply / Quit actions.

## User flow

1. Enter a source directory.
2. Enter separate destinations for images, MOV, MP4/M4V, MPEG/MPG, and AVI.
3. Choose whether to recurse into subdirectories.
4. Leave Dry run enabled (default).
5. Keep digest verification enabled (default).
6. Select Preview. The UI validates the six paths and invokes `sleela-imaging-bridge` through the SLeeLa VM's `osRun` built-in. The bridge scans read-only and writes a new review manifest plus a human-readable report.
7. Review the complete `.review.txt` report and keep the generated manifest unchanged. Existing review files are never overwritten; choose a new plan path for another preview.
8. To apply, turn Dry run off and Preview again (the changed option invalidates the prior review), then explicitly confirm the new plan. Apply becomes available only while the configuration, bridge, provider, plan path, digest setting, and confirmation still match.
9. Apply recomputes the plan, compares the full manifest byte-for-byte, checks the preview-time SHA-256 of each source, verifies each staged copy, and publishes without overwriting.

## UI controls

| Control | Default | Meaning |
|---|---:|---|
| Source directory | Empty | Local directory to inspect |
| Image destination | Empty | Destination for recognized image extensions |
| MOV / MP4 / MPEG / AVI destinations | Empty | Separate video destinations |
| Recursive scan | Off | Include nested directories only when selected |
| Dry run | On | Preview-only; no writes |
| Verify SHA-256 | On | Required before future publication |
| Confirm exact plan | Off | Explicit user authorization; never pre-checked |
| Preview | Enabled | Invokes the configured process bridge for a read-only scan and reviewed-plan report |
| Apply | Disabled until gates pass | Requires a fresh matching preview, Dry run off, SHA-256 on, and explicit confirmation |
| Quit | — | Closes the UI session |

## SleelaUI composition

The source uses documented built-ins: uiAppCreate, uiThemePreset, uiWindowCreate, uiWindowRoot, uiBox, uiFrame, uiHeading, uiLabel, uiEntry, uiCheckBox, uiButton, uiWidgetMargin, uiWidgetSetText, uiWidgetGetText, uiWidgetGetToggle, uiWidgetTakeActivated, uiWidgetSensitive, uiWindowShow, uiAppPump, uiAppQuit, and uiAppDestroy.

The native toolkit owns rendering and input. SLeeLa owns the meaning of each control and the workflow state. The filesystem provider is a separate boundary.

## Current integration boundary

`src/ui/ImagingWindow.sleela` uses the SLeeLa VM's existing `osRun` built-in to invoke `sleela-imaging-bridge`. The compiler lowers `osRun` to `OP_OS_RUN`, and the VM core dispatches it through the host OS process layer. The bridge loads the native C ABI host adapter and filesystem provider in the child process. This is a real VM-mediated process boundary, but not an in-process VM opcode/native binding.

`src/ImagingFileSystem.sleela` remains a design facade whose direct filesystem capability methods return false or `BRIDGE_REQUIRED`; the active UI does not call those methods. The bridge/provider performs canonical-root validation, read-only enumeration, digest-bound review, verified copy, and no-overwrite publication. Remaining release blockers include EXIF/QuickTime metadata parsing, race-free descriptor-relative operations, asynchronous cancellation, SLeeLa source compilation, and cross-platform runtime evidence.

The native window-manager close lifecycle has not yet been exercised. Until that runtime check is completed, use the explicit Quit control to close the application.
