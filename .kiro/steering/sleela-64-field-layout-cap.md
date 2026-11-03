# SLeeLa gotcha: the 64-field-per-class/struct layout cap

The SLeeLa VM admits **at most 64 named fields per class or struct layout**
(`SL_MAX_STRUCT_FIELDS = 64`, defined in the toolkit's `impl/core/sleela_core.h`).
A single class/struct that declares more than 64 instance fields cannot be
registered with the VM and will not run.

## Why this matters here

`gui-interface/MediaBrowser.sleela` originally declared **68 instance fields**
and hit this cap. The failure was easy to misread:

- `sleela check gui-interface/MediaBrowser.sleela` **passed** ("ok").
- `sleela run gui-interface/MediaBrowser.sleela` **failed** at class-layout
  registration, so the app never reached `main()`.

On older toolkit builds the runtime error was also misleading — it read like a
*layout-count* overflow:

```
Semantic error: could not register class layout 'MediaBrowser'. ... the program
defines/pulls more than the 65,536 distinct class+struct types the VM admits.
```

That message pointed at the wrong thing (a 65,536 *layout-count* ceiling or a
stale VM core). The real cause was simply **too many fields in one class**.

## How to recognize it

- `check` says ok but `run`/`compile` fails to register a class/struct layout.
- You count more than 64 instance fields in the offending class/struct.

On a current source-built toolkit (syntax 2.9) the diagnostic is now explicit
and fires in **both** `check` and `run`, e.g.:

```
Semantic error: class 'MediaBrowser' has 65 instance fields, but the VM admits
at most 64 fields per class/struct layout. Reduce the field count -- ...
```

(That clearer message and the `check`/`run` agreement come from a fix in the
`mearvk/SLeeLa` toolkit; if you see the old "65,536 layouts" wording, you are on
an older `sleela` binary — rebuild `impl/build/sleela` from a recent checkout.)

Note: `sleela check` does **not** register VM layouts on older toolkits, which
is why it used to pass an over-wide class. Always validate GUI/stateful classes
with `sleela run`, not `check` alone.

## The fix pattern (keep classes <= 64 fields)

Pack a group of related fields — especially many `ui*` widget handles — into a
single first-class array field reached through small accessor **methods**.
**Methods cost no layout slot**, so this is free against the cap. This is how
`MediaBrowser` was brought from 68 fields down to 50:

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

Alternatives when packing does not fit: split the class into two, or move
single-use locals out of the field list into method-local variables.

## Design guidance for this repo's SleelaUI apps

- Keep every SleelaUI window/app class at or under 64 instance fields. When a
  GUI grows past ~50 handles, pack the related handles into arrays up front
  rather than adding more named fields.
- This applies to any new class here, not just `MediaBrowser` — the organizer
  classes under `src/` and the `gui-interface/` scanners are all well under the
  cap and should stay that way.
- Preserve the Metadata Class metapattern fields (the `meta*` block) — do not
  drop them to save field slots; pack the UI handles instead.
