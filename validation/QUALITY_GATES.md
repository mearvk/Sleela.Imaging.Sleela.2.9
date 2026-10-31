# Quality gates

A gate is PASS only when its evidence is recorded for the exact commit under review. Otherwise use NOT RUN, BLOCKED, or FAIL.

| Gate | Required evidence | Current status |
|---|---|---|
| Source review | Reviewed diff and mapping | Updated in this coding pass; review still required |
| Native C++ build | CMake configure/build with compiler details | NOT RUN |
| Native integration test | CTest output on synthetic fixtures | NOT RUN |
| Provider ABI contract test | Synthetic fixture CTest result | NOT RUN |
| SLeeLa source compile | Compiler version and successful source check/compile | NOT RUN |
| SleelaUI render | Native window and interaction check on each target OS | NOT RUN |
| Window close lifecycle | Window-manager close and explicit Quit tests | NOT RUN |
| Native provider shared library | C ABI build and ABI version check | IMPLEMENTED; BUILD NOT RUN |
| Filesystem VM provider | Bound implementation and contract tests | BLOCKED: SLeeLa host adapter is not implemented |
| Canonical path safety | Symlink, traversal, overlap, and alias tests | NOT RUN |
| Dry-run integration | Before/after filesystem comparison | NOT RUN |
| Copy/verify integration | Source/destination byte count and SHA-256 evidence | NOT RUN |
| Recovery tests | Injected-failure and stale-temp results | NOT RUN |
| Linux native platform | OS/filesystem test report | NOT RUN |
| Windows 10+ native platform | OS/filesystem test report | NOT RUN |
| macOS native platform | OS/filesystem test report | NOT RUN |
| Embedded metadata | EXIF/QuickTime parsing and fixture evidence | BLOCKED: not implemented |
| Security review | Threat checklist and reviewed findings | Analysis drafted; independent review NOT RUN |
| Documentation | Links, support claims, and version consistency | Updated; review required |

The provider uses a dependency-free SHA-256 implementation and C++17 filesystem operations. Do not mark the SLeeLa VM bridge bound or enable UI Apply until a host adapter and runtime integration test exist. Keep evidence tied to the exact commit SHA.
