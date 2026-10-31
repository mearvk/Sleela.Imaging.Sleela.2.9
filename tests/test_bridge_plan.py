#!/usr/bin/env python3
"""End-to-end tests for the bridge's two-stage exact-plan authorization."""
import os
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
        protected_plan = root / "user-data.plan"
        protected_plan.write_text("do not overwrite this file\\n", encoding="utf-8")
        protected = run(bridge, provider, roots, "--plan-file", protected_plan)
        assert protected.returncode == 6, protected.stdout + protected.stderr
        assert protected_plan.read_text(encoding="utf-8") == "do not overwrite this file\\n"

        # A prior imaging manifest is also immutable; matching its header is not permission to replace it.
        same_header_plan = root / "same-header.plan"
        same_header_plan.write_text("SLEELA-IMAGING-PLAN-2\\nuser-owned-bytes\\n", encoding="utf-8")
        blocked = run(bridge, provider, roots, "--plan-file", same_header_plan)
        assert blocked.returncode == 6, blocked.stdout + blocked.stderr
        assert same_header_plan.read_text(encoding="utf-8") == "SLEELA-IMAGING-PLAN-2\\nuser-owned-bytes\\n"

        protected_report_plan = root / "report-protected.plan"
        protected_report = pathlib.Path(str(protected_report_plan) + ".review.txt")
        protected_report.write_text("SLeeLa Imaging review report\\nuser-owned-report\\n", encoding="utf-8")
        blocked = run(bridge, provider, roots, "--plan-file", protected_report_plan)
        assert blocked.returncode == 6, blocked.stdout + blocked.stderr
        assert protected_report.read_text(encoding="utf-8") == "SLeeLa Imaging review report\\nuser-owned-report\\n"

        preview = run(bridge, provider, roots, "--plan-file", plan)
        assert preview.returncode == 0, preview.stdout + preview.stderr
        assert plan.is_file(), "preview did not write the exact-plan manifest"
        report = pathlib.Path(str(plan) + ".review.txt")
        assert report.is_file(), "preview did not write the human-readable report"
        assert "sample.jpg" in report.read_text(encoding="utf-8"), "report omits planned source"

        # Same-size content mutation with the original timestamp restored must invalidate review.
        reviewed_stat = original.stat()
        changed_bytes = b"changed after review"
        assert len(changed_bytes) == len(b"reviewed image bytes")
        original.write_bytes(changed_bytes)
        os.utime(original, ns=(reviewed_stat.st_atime_ns, reviewed_stat.st_mtime_ns))
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
