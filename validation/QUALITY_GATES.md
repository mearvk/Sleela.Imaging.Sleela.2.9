# Quality gates

A gate is **PASS** only when its evidence is recorded for the exact commit under review. Otherwise use **NOT RUN**, **BLOCKED**, or **FAIL**.

| Gate | Required evidence | Current seed status |
|---|---|---|
| Source review | Reviewed diff and mapping | Seeded; review required |
| SLeeLa compile | Compiler version and successful build log | NOT RUN |
| Static checks | Reproducible checker output | NOT RUN |
| Unit tests | Test command and result | NOT RUN |
| Filesystem bridge | Bound implementation and contract tests | BLOCKED: unbound |
| Dry-run integration | Before/after filesystem comparison | NOT RUN |
| Copy/verify integration | Source/destination digest evidence | BLOCKED: bridge |
| Recovery tests | Injected-failure results | NOT RUN |
| Linux | OS/filesystem test report | NOT RUN |
| Windows 10+ | OS/filesystem test report | NOT RUN |
| macOS | OS/filesystem test report | NOT RUN |
| Security review | Threat checklist and reviewed findings | NOT RUN |
| Documentation | Links, support claims, and version consistency | Seeded; review required |

Do not promote a gate based on documentation alone. Keep evidence tied to a commit SHA.
