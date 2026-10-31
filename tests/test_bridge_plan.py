#!/usr/bin/env python3
"""End-to-end tests for the bridge's two-stage exact-plan authorization."""
import pathlib
import subprocess
import sys
import tempfile


def run(bridge, provider, roots, *extra):
    source, images, mov, mp4, mpeg, avi = roots
    command = [
        str(bridge), "--provider", str(provider),
        "--source", str(source), "--images", str(images),
        "--mov", str(mov), "--mp4", str(mp4), "--mpeg", str(mpeg),
        "--avi", str(avi), *map(str, extra),
    ]
    return subprocess.run(command, text=True, capture_output=True, check=False)


def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: test_bridge_plan.py BRIDGE PROVIDER")
    bridge, provider = map(pathlib.Path, sys.argv[1:])
    with tempfile.TemporaryDirectory(prefix="sleela-imaging-plan-") as temporary:
        root = pathlib.Path(temporary)
        names = ("source", "images", "mov", "mp4", "mpeg", "avi")
        roots = [root / name for name in names]
        for directory in roots:
            directory.mkdir()
        original = roots[0] / "sample.jpg"
        original.write_bytes(b"reviewed image bytes")
        plan = root / "review.plan"

        preview = run(bridge, provider, roots, "--plan-file", plan)
        assert preview.returncode == 0, preview.stdout + preview.stderr
        assert plan.is_file(), "preview did not write the exact-plan manifest"
        report = pathlib.Path(str(plan) + ".review.txt")
        assert report.is_file(), "preview did not write the human-readable report"
        assert "sample.jpg" in report.read_text(encoding="utf-8"), "report omits planned source"

        # A changed source must invalidate the reviewed manifest and prevent writes.
        original.write_bytes(b"changed after review")
        denied = run(bridge, provider, roots, "--expect-plan", plan, "--apply", "--confirm")
        assert denied.returncode == 7, denied.stdout + denied.stderr
        assert not any(path.is_file() for directory in roots[1:] for path in directory.iterdir())

        # A fresh preview authorizes only its exact recomputed plan.
        preview = run(bridge, provider, roots, "--plan-file", plan)
        assert preview.returncode == 0, preview.stdout + preview.stderr
        applied = run(bridge, provider, roots, "--expect-plan", plan, "--apply", "--confirm")
        assert applied.returncode == 0, applied.stdout + applied.stderr
        published = [path for directory in roots[1:] for path in directory.iterdir() if path.is_file()]
        assert len(published) == 1, f"expected one published file, got {published}"
        assert published[0].read_bytes() == original.read_bytes()
        assert original.is_file(), "source was not preserved"
    print("PASS: reviewed-plan mismatch denied; exact reviewed plan applied without deleting source")


if __name__ == "__main__":
    main()
