# Quality gates

A gate is PASS only when its evidence is recorded for the exact commit under review. Otherwise use NOT RUN, BLOCKED, or FAIL.

| Gate | Required evidence | Current status |
|---|---|---|
| Source review | Reviewed diff and mapping | Updated in this coding pass; review still required |
| Native C++ build | CMake configure/build with compiler details | NOT RUN in this coding pass; prior prototype evidence is not carried forward as a commit-level PASS |
| Native integration test | CTest output on synthetic fixtures | NOT RUN in this coding pass |
| SLeeLa source compile | Compiler version and successful source check/compile for all src/*.sleela, src/ui/*.sleela, and tests | NOT RUN |
| SleelaUI render | Native window and interaction check on each target OS | NOT RUN |
| Window close lifecycle | Window-manager close and explicit Quit tests | NOT RUN |
| Filesystem VM provider | Bound implementation and contract tests | BLOCKED: source contract remains unbound |
| Canonical path safety | Symlink, traversal, overlap, and alias tests | BLOCKED: requires provider |
| Dry-run integration | Before/after filesystem comparison | NOT RUN for exact reviewed commit |
| Copy/verify integration | Source/destination byte count and SHA-256 evidence | NOT RUN for exact reviewed commit |
| Recovery tests | Injected-failure and stale-temp results | NOT RUN |
| Linux native platform | OS/filesystem test report | NOT RUN for exact reviewed commit |
| Windows 10+ native platform | OS/filesystem test report | NOT RUN |
| macOS native platform | OS/filesystem test report | NOT RUN |
| Embedded metadata | EXIF/QuickTime parsing and fixture evidence | BLOCKED: not implemented |
| Security review | Threat checklist and reviewed findings | Analysis drafted; independent review NOT RUN |
| Documentation | Links, support claims, and version consistency | Updated; review required |

Do not promote a gate based on documentation alone. Keep evidence tied to a commit SHA. A local prototype result is not evidence for a later commit. These entries do not imply Windows/macOS, SLeeLa compiler, SleelaUI runtime, or VM bridge validation.
