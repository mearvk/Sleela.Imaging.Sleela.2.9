# gui-interface — SLeeLa Media Browser

A standalone native SleelaUI desktop application for **browsing pictures and
playing videos**, written in SLeeLa (syntax 2.9) and driven by the SLeeLa VM's
`ui*` built-ins. It is a general media-application shell with a File menu and
the usual media controls, and it matches the look and feel of the project's
other SleelaUI windows (`src/ui/ImagingWindow.sleela`) and the reference
`gui-dual-media-studio` demo in the SLeeLa toolkit.

![SLeeLa Media Browser window](media-browser.png)

*A real render of `MediaBrowser.sleela` by the SleelaUI software rasterizer with
no display server — produced headlessly from `screenshot-mediabrowser.sleela`,
not a mockup.*

## Files

- `MediaBrowser.sleela` — the interactive application. A single, self-contained
  `MediaBrowser` class that opens a native window and runs an event loop. It
  inlines the folder-scanner logic because the SLeeLa runtime runs one merged
  translation unit (the same reason `src/ImagingOrganizer.sleela` inlines its
  classes).
- `GalleryScanner.sleela` — the folder scanner kept as a separate, modular class
  (the same logic that `MediaBrowser` inlines). It mirrors the project's
  `src/ImagingFileSystem.sleela` provider style and carries the Metadata Class
  metapattern.
- `screenshot-mediabrowser.sleela` — a capture driver that builds the identical
  widget tree with pre-filled state and pumps a few non-blocking frames so a
  display-less run can render a real screenshot. Mirrors the repo's
  `tests/screenshot-window.sleela`.

## Features

- **Menu bar**: File (Open Picture / Open Folder / Media Information / Quit),
  View (Picture Gallery / Video Player), and Help (Media Guide / About),
  implemented as a horizontal button strip with collapsible dropdown panels.
- **Navigation header**: Prev / Next, a location field that triples as the
  folder-path input, the current-file display, and the YouTube URL field, plus
  a `local - browser handoff` badge.
- **Real on-disk folder browsing**: type a folder path in the location bar and
  choose **File - Open Folder**. The gallery enumerates the real image files in
  that directory (jpg/jpeg/png/gif/bmp/webp/tiff/tif, case-insensitive) and
  pages through them; with no folder loaded it falls back to a built-in demo
  set. The scan is **read-only** — it lists files, never creates, moves, or
  deletes media, and never follows symbolic links. It uses the VM `osRun`
  built-in with fixed POSIX commands and the scalar-field technique of
  `src/ImagingFileSystem.sleela`.
- **Picture gallery**: an image canvas with a paging caption and Previous /
  Next controls; the location bar updates to the current file as you browse.
- **Video player**: a video canvas with full, **live** transport — Play /
  Pause / Stop, a seek slider that drives the position readout and the playback
  level bar, and a volume slider with a live `volume NN%` readout.
- **YouTube streaming (browser handoff)**: paste a YouTube link in the location
  bar and choose **Stream → Load URL**. The browser extracts the 11-character
  video id (recognizing `watch?v=`, `youtu.be/`, `/embed/`, and `/shorts/`
  shapes), shows the recognized video in the player card, and offers **Open in
  browser** — a platform-aware launch (`xdg-open` / `open` / `cmd start`) that
  plays the video in the user's real browser. It does **not** decode or embed
  YouTube video in-app: SleelaUI has no video surface or network codec, so the
  in-app transport is honestly gated off for a stream and playback is delegated
  to the system browser rather than faked in the canvas. The URL id is parsed by
  a small fixed shell command via `osRun` (the same scalar-field technique the
  folder scanner uses); no media bytes are fetched by the program.
- **State-aware controls**: the transport buttons enable and disable to match
  the player state (Play is off while playing; Pause/Stop are off while
  stopped), so the controls never misrepresent what will happen.
- **Real codec gating**: if the player's media type is not playable, the whole
  transport is disabled — not merely labelled. The UI never offers media it
  cannot play.
- **Tooltips** on the navigation, transport, and slider controls.

Transport presentation is centralized in one `syncTransport()` routine so the
caption, enabled controls, level bar, and status line can never drift apart.

The browser owns no filesystem-mutating capability and fetches no media bytes
itself. Consistent with the project's security posture there is no telemetry,
key upload, database logging, or source deletion; originals are never modified.
The one outbound action is the explicit, user-initiated **Open in browser**
handoff, which launches the system browser on a YouTube URL via the OS opener
(`xdg-open` / `open` / `cmd start`); the program neither downloads nor embeds
the video.

## Run it

Requires the source-built `sleela` compiler (syntax 2.9) from
[mearvk/SLeeLa](https://github.com/mearvk/SLeeLa) and a native display
(X11 / Cocoa / Win32). With no display it reports that and exits cleanly.

```sh
SLEELA=/path/to/SLeeLa
SLEELA_HOME=$SLEELA $SLEELA/impl/build/sleela run gui-interface/MediaBrowser.sleela
```

## Reproduce the screenshot (headless)

The screenshot is a real software-rasterizer render produced with no display,
using the one-time `SLEELA_UI_DUMP` capture hook described in
[`../docs/SCREENSHOT.md`](../docs/SCREENSHOT.md). Build the compiler with the
headless SleelaUI backend, then:

```sh
SLEELA_UI_DUMP=gui-interface/mediabrowser.ppm SLEELA_HOME=$SLEELA \
  $SLEELA/impl/build/sleela run gui-interface/screenshot-mediabrowser.sleela
python3 tests/ppm_to_png.py gui-interface/mediabrowser.ppm docs/images/media-browser.png
```
