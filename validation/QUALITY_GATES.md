# Quality gates

A gate is PASS only when its evidence is recorded for the exact commit under review. Otherwise use NOT RUN, BLOCKED, or FAIL.

| Gate | Required evidence | Current status |
|---|---|---|
| Source review | Reviewed diff and mapping | Updated in this coding pass; review still required |
| Native C++ build | CMake configure/build with compiler details | NOT RUN |
| Native integration test | CTest output on synthetic fixtures | NOT RUN |
| SLeeLa source compile | Compiler version and successful source check/compile for all src/*.sleela, src/ui/*.sleela, and tests | NOT RUN |
| SleelaUI render | Native window and interaction check on each target OS | NOT RUN |
| Window close lifecycle | Window-manager close and explicit Quit tests | NOT RUN |
| Native provider implementation | C ABI implementation, ABI version checks, and contract tests | BLOCKED: only the interface/requirements are documented; executable provider source has not been committed |
| Filesystem VM provider | Bound implementation and contract tests | BLOCKED: source contract remains unbound |
| Canonical path safety | Symlink, traversal, overlap, and alias tests | BLOCKED: requires executable provider |
| Dry-run integration | Before/after filesystem comparison | NOT RUN |
| Copy/verify integration | Source/destination byte count and SHA-256 evidence | NOT RUN |
| Recovery tests | Injected-failure and stale-temp results | NOT RUN |
| Linux native platform | OS/filesystem test report | NOT RUN |
| Windows 10+ native platform | OS/filesystem test report | NOT RUN |
| macOS native platform | OS/filesystem test report | NOT RUN |
| Embedded metadata | EXIF/QuickTime parsing and fixture evidence | BLOCKED: not implemented |
| Security review | Threat checklist and reviewed findings | Analysis drafted; independent review NOT RUN |
| Documentation | Links, support claims, and version consistency | Updated; review required |

The native provider ABI requirements are recorded in docs/FILESYSTEM_PROVIDER.md and native/PROVIDER_ABI.md. Documentation is not implementation evidence. Do not mark the VM bridge bound or enable Apply until the native provider, host adapter, runtime integration, and tests are complete. Keep all evidence tied to the exact commit SHA.
