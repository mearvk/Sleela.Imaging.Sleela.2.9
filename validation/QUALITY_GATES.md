# Quality gates

A gate is PASS only when its evidence is recorded for the exact commit under review. Otherwise use NOT RUN, BLOCKED, or FAIL.

| Gate | Required evidence | Current status |
|---|---|---|
| Source review | Reviewed diff and mapping | In progress |
| Native C++ build | CMake configure/build with compiler details | PASS on local development prototype: Linux, GCC 14.2, C++17, warnings-as-errors; checked-in revision awaits CI |
| Native integration test | CTest output on synthetic fixtures | PASS on local development prototype: 1/1 test; checked-in revision awaits CI |
| SLeeLa source compile | Compiler version and successful source check/compile | NOT RUN |
| Filesystem VM bridge | Bound implementation and contract tests | BLOCKED: source contract remains unbound |
| Dry-run integration | Before/after filesystem comparison | PASS in local synthetic test |
| Copy/verify integration | Source/destination byte count and SHA-256 evidence | PASS in local synthetic test |
| Recovery tests | Injected-failure and stale-temp results | NOT RUN |
| Linux native platform | OS/filesystem test report | Prototype passed locally; checked-in revision CI pending |
| Windows 10+ native platform | OS/filesystem test report | NOT RUN: CI pending |
| macOS native platform | OS/filesystem test report | NOT RUN: CI pending |
| Embedded metadata | EXIF/QuickTime parsing and fixture evidence | BLOCKED: not implemented |
| Security review | Threat checklist and reviewed findings | NOT RUN |
| Documentation | Links, support claims, and version consistency | Updated; review required |

Do not promote a gate based on documentation alone. Keep evidence tied to a
commit SHA. The local PASS entries describe the development prototype, not an independently
reproduced run of the exact committed file contents. The checked-in revision
awaits its GitHub Actions result. These entries do not imply Windows/macOS,
SLeeLa compiler, or VM bridge validation.
