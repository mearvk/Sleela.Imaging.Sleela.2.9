#!/bin/sh
# Create a deterministic media fixture for the SLeeLa-native organizer demo.
# Files carry known modification times so the chronological ordering,
# collision-sequence naming, and classification are reproducible.
#
# Usage: tests/make-fixture.sh [fixture-dir]   (default: ./fixture)
set -eu

DIR="${1:-fixture}"
SRC="$DIR/src"
OUT="$DIR/out"

rm -rf "$DIR"
mkdir -p "$SRC" "$OUT/Images" "$OUT/MOV" "$OUT/MP4" "$OUT/MPEG" "$OUT/AVI"

printf 'fake-jpeg-content-A' > "$SRC/photoA.JPG"
printf 'fake-png-content-B'  > "$SRC/imageB.png"
printf 'fake-mov-content-C'  > "$SRC/clipC.MOV"
printf 'fake-mp4-content-D'  > "$SRC/clipD.mp4"
printf 'fake-avi-content-E'  > "$SRC/clipE.avi"
printf 'not-media'           > "$SRC/notes.txt"

# A same-timestamp twin of imageB to exercise the _002 collision sequence.
printf 'collision-twin-F'    > "$SRC/twinF.png"

touch -d '2023-01-15 10:30:00 UTC' "$SRC/photoA.JPG"
touch -d '2023-03-20 14:45:10 UTC' "$SRC/imageB.png"
touch -d '2022-11-05 09:00:00 UTC' "$SRC/clipC.MOV"
touch -d '2024-06-01 23:59:59 UTC' "$SRC/clipD.mp4"
touch -d '2021-07-04 12:00:00 UTC' "$SRC/clipE.avi"
touch -d '2023-03-20 14:45:10 UTC' "$SRC/twinF.png"

echo "fixture ready at $DIR"
ls -l "$SRC"
