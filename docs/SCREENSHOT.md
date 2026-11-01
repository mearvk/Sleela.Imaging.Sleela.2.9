# Generating the desktop-window screenshot

`docs/images/imaging-window.png` is a real render of the SLeeLa Imaging desktop
window (`src/ui/ImagingWindow.sleela`), produced with **no display server** by
the SleelaUI software rasterizer. It is not a mockup.

## How it works

SleelaUI owns its pixels: every window keeps one 32-bit `Canvas` that the
portable core lays out and paints in software, which a platform backend then
blits to screen. SleelaUI ships a **headless backend** (no window system) for
building and smoke-testing on a display-less CI runner. Rendering the window
through that backend yields a fully painted `Canvas` in memory, which can be
written straight to an image file.

`tests/screenshot-window.sleela` builds the exact same widget tree as the real
`ImagingWindow` (heading, the three framed sections, the six path entries, the
safety checkboxes, the Preview/Apply/Quit buttons, and the status info-bar),
fills the fields with realistic example values, and pumps a few frames so the
layout settles.

## Reproduce it

The committed PNG is already in `docs/images/`. To regenerate it you need a
source-built `sleela` whose SleelaUI uses the **headless** backend, plus a tiny
one-time capture hook in SleelaUI's `Window::render()` that writes the rendered
canvas when `SLEELA_UI_DUMP` is set. That hook lives in the SLeeLa toolkit
(upstream `mearvk/SLeeLa`), not in this repo; apply this patch to
`user-interface/src/slui_window.cpp`, just before `Window::render()` returns
`canvas_`:

```cpp
// requires: #include <cstdio>, <cstdlib>, <cstdint>
if (const char* dumpPath = std::getenv("SLEELA_UI_DUMP")) {
    if (*dumpPath) {
        const int dw = canvas_.width(), dh = canvas_.height();
        const uint32_t* dpx = canvas_.pixels();   // 0xAARRGGBB
        if (dw > 0 && dh > 0 && dpx) {
            if (std::FILE* df = std::fopen(dumpPath, "wb")) {
                std::fprintf(df, "P6\n%d %d\n255\n", dw, dh);
                for (long di = 0; di < (long)dw * dh; ++di) {
                    unsigned char rgb[3] = {
                        (unsigned char)((dpx[di] >> 16) & 0xFF),
                        (unsigned char)((dpx[di] >> 8) & 0xFF),
                        (unsigned char)(dpx[di] & 0xFF)};
                    std::fwrite(rgb, 1, 3, df);
                }
                std::fclose(df);
            }
        }
    }
}
```

The headless backend never calls `present()`, so also render open windows once
per pump. In `user-interface/src/slui_api.cpp`, `slui_app_pump()` should call
`w->render()` for each open window after `backend->pump(...)` (a no-op cost on
windowed backends, which blit separately).

Then build the headless compiler and render + convert from this repo:

```sh
SLEELA=/path/to/SLeeLa
cd "$SLEELA/impl"
make build/sleela \
  SLUI_BACKEND_SRC='../user-interface/src/backend/headless.cpp' \
  SLUI_BACKEND_DEF='-DSLUI_BACKEND_HEADLESS_BUILD' \
  SLUI_BACKEND_CFLAGS='' SLUI_LIBS=''

cd /path/to/this/repo
SLEELA_UI_DUMP="window.ppm" SLEELA_HOME="$SLEELA" \
  "$SLEELA/impl/build/sleela" run tests/screenshot-window.sleela
python3 tests/ppm_to_png.py window.ppm docs/images/imaging-window.png
```

The default (windowed) `sleela` build renders the identical pixels to a real
X11 / Cocoa / Win32 window; the headless backend simply lets the same paint path
run where there is no display. The capture hook is intentionally kept out of
this repo so the organizer does not fork or depend on a modified toolkit.

## Notes

- `tests/ppm_to_png.py` is a dependency-free (stdlib `zlib`) binary-PPM → PNG
  converter, so no image library is required.
- The window the screenshot shows is the real program UI; the only difference
  from an interactive session is that the fields are pre-filled and the status
  bar carries an example "Preview ready" message so the capture is informative.
