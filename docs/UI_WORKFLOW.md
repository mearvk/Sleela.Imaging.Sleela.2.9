# SLeeLa Imaging UI and workflow

## Entry point

src/ui/ImagingWindow.sleela is the SLeeLa desktop entry point. It creates a native SleelaUI application/window through the documented ui* VM built-ins. The layout uses a vertical page, grouped source/destination controls, checkboxes, status text, and Preview / Apply / Quit actions.

## User flow

1. Enter a source directory.
2. Enter separate destinations for images, MOV, MP4/M4V, MPEG/MPG, and AVI.
3. Choose whether to recurse into subdirectories.
4. Leave Dry run enabled (default).
5. Keep digest verification enabled (default).
6. Select Preview. The SLeeLa session validates the configuration and checks provider readiness.
7. Review the plan and explicitly confirm it before any future apply operation.
8. Apply remains unavailable until the filesystem provider and all transaction gates are implemented and verified.

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
| Preview | Enabled | Validates state; currently reports the unbound provider |
| Apply | Disabled | No operation can bypass the missing provider/release gates |
| Quit | — | Closes the UI session |

## SleelaUI composition

The source uses documented built-ins: uiAppCreate, uiThemePreset, uiWindowCreate, uiWindowRoot, uiBox, uiFrame, uiHeading, uiLabel, uiEntry, uiCheckBox, uiButton, uiWidgetMargin, uiWidgetSetText, uiWidgetGetText, uiWidgetGetToggle, uiWidgetTakeActivated, uiWidgetSensitive, uiWindowShow, uiAppPump, uiAppQuit, and uiAppDestroy.

The native toolkit owns rendering and input. SLeeLa owns the meaning of each control and the workflow state. The filesystem provider is a separate boundary.

## Current limitation

src/ImagingFileSystem.sleela is a contract only. Its operations return BRIDGE_REQUIRED or false, and it cannot be marked bound from application code. Consequently the current Preview action deliberately does not enumerate files and Apply cannot be enabled. The next implementation step is a real SLeeLa VM/OS filesystem provider with canonical path handling, enumeration, metadata reads, verified copy, and atomic no-overwrite publication.

The native window-manager close lifecycle has not yet been exercised. Until that runtime check is completed, use the explicit Quit control to close the application.
