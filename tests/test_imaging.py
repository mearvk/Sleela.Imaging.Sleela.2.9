import hashlib
import os
import pathlib
import subprocess
import sys
import tempfile

BIN = pathlib.Path(sys.argv[1]).resolve()

def run(args, expected=0):
    p = subprocess.run([str(BIN), *map(str, args)], text=True, capture_output=True)
    assert p.returncode == expected, (p.returncode, p.stdout, p.stderr)
    return p

with tempfile.TemporaryDirectory(prefix="sleela-imaging-") as td:
    root = pathlib.Path(td)
    source = root / "source"
    source.mkdir()
    destinations = {n: root / n for n in ("images", "mov", "mp4", "mpeg", "avi")}
    for d in destinations.values():
        d.mkdir()
    original = source / "holiday.JPG"
    payload = b"SLeeLa Imaging synthetic fixture\x00\x01"
    original.write_bytes(payload)
    os.utime(original, (1700000000, 1700000000))
    args = ["--source", source, "--images", destinations["images"],
            "--mov", destinations["mov"], "--mp4", destinations["mp4"],
            "--mpeg", destinations["mpeg"], "--avi", destinations["avi"]]

    # Dry-run is the default and does not mutate either side.
    p = run(args)
    assert "PLAN" in p.stdout
    assert original.read_bytes() == payload
    assert list(destinations["images"].iterdir()) == []

    # Apply requires a distinct explicit confirmation.
    run(args + ["--apply"], expected=2)
    p = run(args + ["--apply", "--confirm"])
    outputs = list(destinations["images"].iterdir())
    assert len(outputs) == 1
    assert outputs[0].read_bytes() == payload
    assert hashlib.sha256(payload).hexdigest() in p.stdout
    assert original.read_bytes() == payload

    # Existing target is never overwritten.
    outputs[0].write_bytes(b"preserve existing target")
    p = run(args + ["--apply", "--confirm"])
    assert outputs[0].read_bytes() == b"preserve existing target"
    assert "SKIP" in p.stdout

    # Reports are new files and identify the timestamp provenance.
    report = root / "run.csv"
    run(args + ["--report", report])
    assert "filesystem-modified-time" in report.read_text()
    run(args + ["--report", report], expected=2)

with tempfile.TemporaryDirectory(prefix="sleela-imaging-roots-") as td:
    root = pathlib.Path(td)
    source = root / "source"
    source.mkdir()
    destinations = {n: root / n for n in ("images", "mov", "mp4", "mpeg", "avi")}
    for d in destinations.values():
        d.mkdir()
    nested = source / "output"
    nested.mkdir()
    args = ["--source", source, "--images", nested, "--mov", destinations["mov"],
            "--mp4", destinations["mp4"], "--mpeg", destinations["mpeg"],
            "--avi", destinations["avi"]]
    run(args, expected=2)

print("PASS: dry-run, confirmation, SHA-256 copy verification, source preservation, no-overwrite, reports, and root isolation")
