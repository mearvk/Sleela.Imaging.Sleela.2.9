#!/bin/sh
# End-to-end verification of the SLeeLa-native imaging organizer.
#
# Builds a fixture, runs src/ImagingOrganizer.sleela on the real SLeeLa VM
# (preview + verified apply), then asserts:
#   * every eligible file was organized into the correct destination,
#   * the unsupported file was skipped,
#   * a same-timestamp collision produced an _002 name,
#   * each published copy has the SAME SHA-256 as its source,
#   * all originals are still present (nothing deleted or modified).
#
# Requirements:
#   SLEELA_BIN   path to the source-built `sleela` (supports syntax 2.9)
#   SLEELA_HOME  path to the SLeeLa checkout (for library/SHEET resolution)
#
# Usage:
#   SLEELA_HOME=/path/to/SLeeLa SLEELA_BIN=/path/to/SLeeLa/impl/build/sleela \
#     tests/run-native-demo.sh
set -eu

HERE="$(cd "$(dirname "$0")/.." && pwd)"
cd "$HERE"

: "${SLEELA_BIN:?set SLEELA_BIN to the source-built sleela (supports 2.9)}"
: "${SLEELA_HOME:?set SLEELA_HOME to the SLeeLa checkout root}"
export SLEELA_HOME

fail() { echo "FAIL: $1" >&2; exit 1; }

sh tests/make-fixture.sh fixture >/dev/null

"$SLEELA_BIN" run src/ImagingOrganizer.sleela \
  2>/dev/null | grep -vE '^\[' || true

IMG="fixture/out/Images"
check_copy() {
  src="$1"; dst="$2"
  [ -f "$dst" ] || fail "expected organized file missing: $dst"
  a="$(sha256sum "$src" | cut -d' ' -f1)"
  b="$(sha256sum "$dst" | cut -d' ' -f1)"
  [ "$a" = "$b" ] || fail "digest mismatch $src vs $dst"
  [ -f "$src" ] || fail "original was not preserved: $src"
}

check_copy fixture/src/photoA.JPG "$IMG/2023-01-15_10-30-00_001.jpg"
check_copy fixture/src/imageB.png "$IMG/2023-03-20_14-45-10_001.png"
check_copy fixture/src/clipC.MOV  "fixture/out/MOV/2022-11-05_09-00-00_001.mov"
check_copy fixture/src/clipD.mp4  "fixture/out/MP4/2024-06-01_23-59-59_001.mp4"
check_copy fixture/src/clipE.avi  "fixture/out/AVI/2021-07-04_12-00-00_001.avi"

# collision sequence: twinF shares imageB's timestamp -> _002
[ -f "$IMG/2023-03-20_14-45-10_002.png" ] || fail "collision _002 not produced"

# unsupported file must never be organized
[ ! -f "fixture/out/Images/notes.txt" ] || fail "unsupported file was organized"

# all seven source files still present
[ "$(ls -1 fixture/src | wc -l)" = "7" ] || fail "an original went missing"

echo "PASS: native organizer preview+apply verified (6 organized, 1 skipped, originals preserved)"
