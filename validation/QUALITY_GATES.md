# Quality gates

A gate is PASS only when its evidence is recorded for the exact commit under review. Otherwise use NOT RUN, BLOCKED, or FAIL.

| Gate | Required evidence | Current status |
|---|---|---|
| Source review | Reviewed diff and mapping | Updated in this coding pass; review still required |
| Native C++ build | CMake configure/build with compiler details | NOT RUN |
| Native integration test | CTest output on synthetic fixtures | NOT RUN |
| Provider ABI contract test | Synthetic fixture CTest result, including a known SHA-256 vector | REGRESSION TEST ADDED; NOT RUN |
| Bridge plan test | Exact-plan comparison, same-size/same-timestamp mutation, immutable artifact checks | REGRESSION TEST ADDED; NOT RUN |
| SLeeLa source compile | Compiler version and successful source check/compile | NOT RUN |
| SleelaUI render | Native window and interaction check on each target OS | NOT RUN |
| Window close lifecycle | Window-manager close and explicit Quit tests | NOT RUN |
| Native provider shared library | C ABI v2 source and ABI version check | IMPLEMENTED IN SOURCE; BUILD NOT RUN |
| Native host adapter | Explicit-path dynamic load, ABI validation, delegated preview/apply | IMPLEMENTED IN SOURCE; BUILD NOT RUN |
| SLeeLa VM imaging process bridge | osRun -> OP_OS_RUN -> host OS process service -> bridge executable | IMPLEMENTED IN SOURCE; EXACT-COMMIT BUILD/RUNTIME NOT RUN |
| In-process VM filesystem binding | Native VM opcode, compiler registration, VM-local handles | NOT IMPLEMENTED; NOT REQUIRED BY CURRENT PROCESS-BRIDGE DESIGN |
| Canonical path safety | Symlink, traversal, overlap, and alias tests | PARTIAL SOURCE COVERAGE; FULL SUITE NOT RUN |
| Dry-run integration | Before/after filesystem comparison | TEST SOURCE PRESENT; NOT RUN |
| Copy/verify integration | Source/destination byte count and SHA-256 evidence | TEST SOURCE PRESENT; NOT RUN |
| Recovery tests | Injected-failure and stale-temp results | NOT RUN |
| Linux native platform | OS/filesystem test report | NOT RUN |
| Windows 10+ native platform | OS/filesystem test report | NOT RUN |
| macOS native platform | OS/filesystem test report | NOT RUN |
| Embedded metadata | EXIF/QuickTime parsing and fixture evidence | NOT IMPLEMENTED |
| Security review | Threat checklist and independent findings | Analysis drafted; independent review NOT RUN |
| Documentation | Links, support claims, and version consistency | Updated; review required |

The provider uses a dependency-free SHA-256 implementation and C++17 filesystem operations. The active SLeeLa integration is a process bridge, not an in-process provider binding. No claim of a successful build, test, compiler run, or platform runtime is made until exact-commit evidence is recorded. Keep all evidence tied to the exact commit SHA.
