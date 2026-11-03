# November 3, 2026 — MediaBrowser debugging session log

This document is a faithful write-up of the full working session: the user's
requests (paraphrased in the headings), the investigation, every fix, and the
final resolution. It covers two repositories:

- **mearvk/Sleela.Imaging.Sleela.2.9** — the imaging apps (ImagingOrganizer +
  MediaBrowser).
- **mearvk/SLeeLa** — the SLeeLa language toolkit (compiler/VM + SleelaUI).

---

## 1. "Check the apps in Sleela. We need them both to run. MediaBrowser is having compile problems."

**Goal.** The repo https://github.com/mearvk/Sleela.Imaging.Sleela.2.9 has two
apps; both must run. MediaBrowser was failing.

**Investigation.**

- Cloned the imaging repo (default branch `main`) and mapped it. The two apps:
  1. **ImagingOrganizer** (`src/ImagingOrganizer.sleela`) — the media organizer.
  2. **MediaBrowser** (`gui-interface/MediaBrowser.sleela`) — the native
     SleelaUI picture/video browser (the one failing).
- To validate syntax 2.9 (the prebuilt 1.11 binary cannot run 2.x sources),
  built the compiler from `mearvk/SLeeLa`:
  `cd impl && SLEELA_SHA256_MANIFEST=... make build/sleela`
  (needs g++, X11/fontconfig/freetype dev libs).
- `sleela check` passed for **both** apps. But `sleela run` on MediaBrowser
  failed:

  ```
  sleelvac: Semantic error: could not register class layout 'MediaBrowser'.
  A 'layout' is one distinct class/struct type registered with the VM (not
  bytes); the program defines/pulls more than the 65,536 distinct class+struct
  types the VM admits. (If this fires with only a handful of classes, the
  running sleelvac's VM core is stale/mismatched ...)
  ```

- ImagingOrganizer ran perfectly (`published=6 failed=0 skipped=0`).
- `SLEELA_DEBUG_LAYOUTS=1` reported only **2 classes** total — so the
  "65,536 layouts" message was misleading.
- `SLEELA_TRACE_LAYOUTS=1` printed **no** registration lines before the error,
  which meant `slvm_declare_struct` returned `-1` at its *first* guard, before
  any trace — i.e. `nfields > SL_MAX_STRUCT_FIELDS`.

**Root cause.** The VM caps a class/struct layout at **64 fields**
(`SL_MAX_STRUCT_FIELDS = 64`). The `MediaBrowser` class declared **68 instance
fields**. `slvm_declare_struct()` returns `-1` when `nfields > 64` — before the
layout-table-growth path — and the compiler surfaced it as the generic
"could not register class layout … 65,536 types" error. `check` passed because
it does not register VM layouts; only `run`/`compile` do, so the app never
reached `main()`.

Proven minimally: a class with 65 `int` fields produced the identical error;
64 fields ran fine.

**Fix (MediaBrowser).** Packed the **19 menu widget handles** into a single
`int[] menuH` array, reached through small typed accessor methods
(`mFileBtn()`, `mFilePanel()`, …). Methods cost **no layout slot**, so the
field count dropped **68 → 50** with headroom. No behavior change — widget
tree, event handling, YouTube handoff, codec gating, and the Metadata Class
metapattern all unchanged.

```sleela
<< 19 menu widget handles packed into one int[] so the class stays under the
   VM's 64-field-per-layout cap. Each handle is reached through a named
   accessor; the index legend documents what each slot holds. >>
int[] menuH;

int mFileBtn()   { return menuH[0]; }
int mFilePanel() { return menuH[1]; }
<< ... one accessor per slot ... >>

<< allocate once at the top of main(), fill each slot as its widget is built >>
menuH = new int[19];
menuH[0] = uiButton(menuStrip, "File");
```

**Verification.** MediaBrowser now reached `main()`, printed its backend banner,
and exited cleanly on a headless host (documented no-display behavior).
ImagingOrganizer still ran. Both apps ran.

**Delivery.** Branch `fix/media-browser-64-field-limit` → **PR #4** into `main`.

---

## 2. "Improve the notes or code." (Make the toolkit's error honest)

The "65,536 layouts" message was misleading, so the fix moved to the toolkit
(`mearvk/SLeeLa`).

**Two latent defects found:**

1. **Wrong caps in `impl/frontend/compiler.cpp`.** It checked classes and
   structs against `>128` fields — but the VM's real cap is **64**. So a class
   with 65–128 fields slipped past the compiler's own check and fell through to
   `slvm_declare_struct()` returning `-1`, misreported as a layout-*count*
   overflow.
2. **`check`/`run` divergence in `impl/frontend/semantic.cpp`.** The semantic
   analyzer (used by `sleela check`) enforced the 64-field cap for *structs*
   but had **no check for classes** — so `check` passed an over-wide class while
   `run`/`compile` failed. That silent divergence is what made the original bug
   hard to diagnose.

**Fixes.**

- `compiler.cpp`: check both classes and structs against `SL_MAX_STRUCT_FIELDS`
  (64) **before** calling `slvm_declare_struct`, with a message naming the
  type, its field count, the cap, and a concrete remedy. A `-1` from the VM now
  genuinely means a layout-*count* overflow.
- `semantic.cpp`: added the matching 64-field check for classes so `check`
  agrees with `run`/`compile`; upgraded the struct message to match.

Before:
```
Semantic error: could not register class layout 'MediaBrowser'. ... more than
the 65,536 distinct class+struct types the VM admits.
```
After (fires in both `check` and `run`):
```
Semantic error: class 'FieldTest' has 65 instance fields, but the VM admits at
most 64 fields per class/struct layout. Reduce the field count -- for example,
pack a group of related fields into a single array field reached through
accessor methods (methods cost no layout slot), or split the class.
```

**Verification.** 65-field class/struct errors clearly in both `check` and
`run`; 64-field class still works; both imaging apps still run; all known-good
`example/` demos still run. Rebuilt compiler, regenerated
`security/sha256-manifest.json`, `make check-manifest` → OK. Confirmed the
stricter check does not regress any *runnable* code (the only lib classes it
newly flags — `CitizenStats` ~277 fields, `CharacterStats` ~182,
`SLXltRefBank`— already exceeded 64 and could never run).

**Delivery.** Committed directly to SLeeLa `master` (per repo convention).
(First push mangled the commit message because backticks in the heredoc were
shell-expanded; amended with a `-F file` message and force-pushed with
`--force-with-lease` to fix only that just-pushed commit.)

---

## 3. "Put this note in the repo's Kiro settings."

Created repo-scoped steering so the guidance travels with the project.

**File:** `.kiro/steering/sleela-64-field-layout-cap.md` (imaging repo). It
documents: the 64-field cap; how the symptom appears (`check` passes, `run`
fails; older toolkits misreport it as a 65,536-layout overflow); how to
recognize it; the fix pattern (pack related handles into an `int[]` reached by
accessor methods); and repo-specific guidance to keep SleelaUI app classes
under the cap and to preserve the Metadata Class metapattern.

**Delivery.** Committed to the `fix/media-browser-64-field-limit` branch so it
rode along with PR #4. (A `.md` is not manifest-covered, so no manifest
regeneration was needed.)

---

## 4. "Push to main, master."

- **SLeeLa** was already on `master`, fully pushed — nothing to do.
- **Sleela.Imaging** — merged **PR #4** into `main`:
  `gh api -X PUT repos/mearvk/Sleela.Imaging.Sleela.2.9/pulls/4/merge`.
  `merged=true`. Local `main` fast-forwarded to match. Verified the steering
  note and the `menuH` fix are on `main`.

---

## 5. "The GUI loads after compile now. That's improvement!"

Confirmed the 64-field fix held end to end. Recap of why it works: the class
was over the VM's 64-field cap (68 fields), so it failed at layout registration
before `main()` ran; packing the menu handles into one array brought it to 50.
Noted both toolkit fixes are landed (64-field diagnostic + the clearer error).

---

## 6. "So we can download and try to run the apps?"

Reported state and gave the download-and-run flow:

- SLeeLa `master`: both fixes merged (64-field diagnostic + `uiWindowClosed`).
- Imaging `main`: has the compile fix (PR #4), but the event-loop fix was still
  in open PR #5.
- Two caveats: the event-loop fix wasn't on `main` yet, and the toolkit
  compiler must be rebuilt from `master` (syntax 2.9 + the new built-in).

```sh
# 1. Build the toolkit compiler from current master
git clone https://github.com/mearvk/SLeeLa.git
cd SLeeLa/impl
SLEELA_SHA256_MANIFEST="$(pwd)/../security/sha256-manifest.json" make build/sleela
cd ../..

# 2. Get the imaging apps
git clone https://github.com/mearvk/Sleela.Imaging.Sleela.2.9.git
cd Sleela.Imaging.Sleela.2.9
SLEELA=../SLeeLa

# 3a. App 1 — the organizer (headless, works anywhere)
sh tests/make-fixture.sh
SLEELA_HOME=$SLEELA $SLEELA/impl/build/sleela run src/ImagingOrganizer.sleela

# 3b. App 2 — MediaBrowser (needs a real display: X11 / Cocoa / Win32)
SLEELA_HOME=$SLEELA $SLEELA/impl/build/sleela run gui-interface/MediaBrowser.sleela
```

Honest limitation stated: only the headless path was verifiable in the sandbox
(no display server); the on-screen behavior could only be confirmed on the
user's machine.

---

## 7. "New pull still shows the GUIs both exiting early upon loading. The app loading is visible."

This meant the loop fix had not addressed the real cause. Investigation of the
event loop:

- Both interactive GUIs (`MediaBrowser` and `src/ui/ImagingWindow.sleela`)
  share the identical loop idiom — and so does the toolkit's reference demo
  `gui-dual-media-studio` — so the behavior was app-agnostic.
- Read the toolkit's `uiAppPump`/`slui_app_pump`/backend `pump()`:
  - `uiAppPump(app, 1)` dispatches events **once** and returns the dispatched
    count (0 when idle), or a **negative error code**. On X11/Win32, idle
    returns 0, not negative — so `if (dispatched < 0) running = false;` should
    not fire on a normal frame.
  - There was **no** `uiWindowClosed`/`should_quit` built-in exposed to SLeeLa,
    so a manual `uiAppPump` loop could not detect the user closing the window —
    only `File → Quit` exited.

Two real weaknesses identified, though not yet the confirmed cause:
(1) the loop quit on any single negative pump result; (2) no way to exit by
closing the window.

**Toolkit improvement.** Added a new `uiWindowClosed(window) -> int` built-in
wiring the existing C++ `Window::closed()` through all layers:
- C API `slui_window_closed` (`include/sleela_ui.h`, `src/slui_api.cpp`);
- VM id `SLUI_FN_WINDOW_CLOSED` (`sleela_ui_ids.h`) + dispatch
  (`sleela_ui_vm.c`);
- compiler name map `uiWindowClosed` (`compiler.cpp`);
- docs (`lib/user-interface/USER-INTERFACE.md`).

**MediaBrowser loop change.** Exit when `uiWindowClosed(window) == 1`, and stop
treating a transient negative pump as fatal.

**Delivery.** `uiWindowClosed` committed to SLeeLa `master`; MediaBrowser loop
change pushed as **PR #5** (`fix/media-browser-event-loop-exit`).

Because the real cause was still unknown, a **diagnostic build** was prepared
to trace, per frame, the `uiAppPump` return, the `uiWindowClosed` state, and the
`running` flag, plus loop entry/exit prints.

---

## 8. "Linux OS Ubuntu 24." / "wayland. and the other is :0."

Environment: Ubuntu 24.04, `XDG_SESSION_TYPE=wayland`, `DISPLAY=:0` — a Wayland
session running the X11 app through **XWayland**.

**Toolkit finding.** The SleelaUI X11 backend installed **no X error handlers**.
Xlib's default handler prints the failing request and calls `exit()` on any X
protocol error; XWayland is stricter/raceier around window map and first
present, a plausible trigger for "window appears, then process dies."

**Fix.** Installed, before `XOpenDisplay`, a non-fatal `XSetErrorHandler` that
logs the failing request (error text, request/minor code, resource id, serial)
and returns 0 (so Xlib does not `exit()`), plus an `XSetIOErrorHandler` that
reports a lost connection clearly. Committed to SLeeLa `master`
(`user-interface/src/backend/x11.cpp`).

This was a correct, defensible improvement — but it was still a hypothesis, not
a confirmed cause.

---

## 9. "pulled and the GUI still checks and exits as loads."

The error handlers did not change the behavior, ruling out the Xlib-`exit()`
theory. The decisive artifact was the console output. The user ran the app and
provided it:

```
SLeeLa Imaging MediaBrowser (native SleelaUI) - backend x11, toolkit 1.0.0
Window shown. Gallery and transport controls change the view and player state.
sleelvac: runtime error: checked cast failed: runtime value tag does not match target
```

Two facts jumped out:
- The window **is** shown; then a **SLeeLa runtime error** aborts the program —
  not a display/loop issue.
- There were **no `[loop]` lines**, meaning the user was running the `main`
  branch (pre-diagnostic) MediaBrowser, and the crash was in its loop body.

---

## 10. "No, address the software bug."

The user correctly pushed to fix the actual bug rather than attribute it to a
stale installed binary.

**Localizing the cast.** The error text comes from `OP_CAST` in
`impl/core/sleela_core.c`:
```
TERR("checked cast failed: runtime value tag does not match target");
```
The cast block handled widening (`int -> double`, `int -> int128`) and
`int128 -> int` narrowing, but had **no case for `double -> int`**. A
double-to-int cast fell through to the fail-closed tag check and aborted.

**Why it fired after "Window shown."** The crash is on the **first event-loop
frame**: `pollSliders()` runs (because `playable == true` and
`seekSlider >= 0`) and executes:
```sleela
double sv = uiWidgetGetValue(seekSlider);
int pos = clampInt((int) sv, 0, clipSeconds);   // (int) on a double -> abort
```
The second GUI's slider handling has the same `(int) uiWidgetGetValue(...)`
pattern, which is why **both** GUIs exited.

**Reproduced minimally:**
```sleela
double sv = 42.7;
int pos = clampInt((int) sv, 0, 180);   // -> checked cast failed
```

**The fix (VM).** In `impl/core/sleela_core.c`, `OP_CAST` now narrows a double
to an integer target with C's truncation-toward-zero (matching Java's `(int)`):

```c
if(targetTag==SL_INT && value.type==SL_DOUBLE) {
    PUSH(slval_int((int64_t)value.as.d)); break;
}
if(targetTag==SL_INT128 && value.type==SL_DOUBLE) {
    PUSH(slval_int128((sl_int128_t)value.as.d)); break;
}
```

**Verification.**
- `(int) 42.7` → `42`, `(int) -3.9` → `-3` (truncation toward zero).
- `clampInt((int) a, ...)` — the exact MediaBrowser pattern — works.
- `int -> double` widening and integer division semantics unchanged.
- `example/` demos still run; manifest regenerated; `check-manifest` → OK.

**Delivery.** Committed to SLeeLa `master`
(`VM: support narrowing cast from double to int/int128 instead of aborting`).

---

## 11. "Can you push the fix to Sleela's repo now?"

The VM fix was already pushed to SLeeLa `master` in the previous step; verified
on the remote:
- Local and remote `master` identical; no unpushed commits.
- The double→int narrowing is present in `origin/master`'s `sleela_core.c`.

**State of SLeeLa `master` after the session (all four fixes landed):**

| Commit (subject) | Fix |
|---|---|
| VM: support narrowing cast from double to int/int128 | **double→int cast** (the real MediaBrowser crash) |
| x11 backend: install X error + IO-error handlers | survive X protocol errors (XWayland) |
| Add uiWindowClosed built-in | window X-button close for manual loops |
| Report a >64-field class/struct clearly | clear error for the original compile issue |

**User action to pick up the fix (compiler/VM change, so rebuild + reinstall):**
```sh
cd ~/path/to/SLeeLa && git checkout master && git pull
cd impl && SLEELA_SHA256_MANIFEST="$(pwd)/../security/sha256-manifest.json" make build/sleela
make install
sleela run gui-interface/MediaBrowser.sleela
```

---

## Timeline of root causes (what was actually wrong, in order)

1. **Compile failure:** `MediaBrowser` had 68 instance fields > the VM's
   64-field-per-layout cap, misreported as a 65,536-layout overflow.
   → Fixed by packing menu handles into an `int[]` (68 → 50) and by making the
   toolkit's error message honest and consistent between `check` and `run`.
2. **"Exits right after loading":** NOT a display/loop/XWayland problem. The VM
   lacked a `double -> int` cast, so `pollSliders()`'s
   `(int) uiWidgetGetValue(slider)` aborted on the first event-loop frame.
   → Fixed by adding double→int / double→int128 narrowing to `OP_CAST`.

Along the way, three additional toolkit improvements landed (clearer field-cap
diagnostic, `uiWindowClosed` built-in, X11 error handlers). The `uiWindowClosed`
and error-handler work were not the cause of the early exit but are genuine,
retained improvements.

---

## Open follow-up

PR #5 (`fix/media-browser-event-loop-exit`) still carries temporary `[loop]`
diagnostic prints. Now that the real crash is fixed in the VM, those should be
removed; the `uiWindowClosed`-based window-close exit is a genuine improvement
and should be kept. (Pending cleanup.)

---

## Repositories and references

- Imaging apps: https://github.com/mearvk/Sleela.Imaging.Sleela.2.9
  - PR #4 (merged): 64-field compile fix + steering note.
  - PR #5 (open): MediaBrowser event-loop / window-close change.
- Toolkit: https://github.com/mearvk/SLeeLa — four fixes on `master`
  (see table above).

*Session date: November 3, 2026 (Tuesday).*
