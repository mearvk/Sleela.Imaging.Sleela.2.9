# Native provider ABI

Implemented in `native/imaging_provider.cpp` and declared in `native/imaging_provider.h` (ABI v2). CMake builds the shared library and a synthetic-fixture contract test.

ABI v2 adds a 65-byte lowercase SHA-256 field to each preview item. When digest verification is enabled, the provider hashes each source during preview, retains that digest in its process-local plan, rechecks the source digest before copying, and verifies the staged copy against the reviewed digest. The ABI version is incremented so an ABI v1 host cannot misread the expanded `sli_item` structure.

The provider is loaded by `sleela-imaging-bridge` through the explicit-path host adapter. The SLeeLa desktop UI invokes that bridge through the SLeeLa VM's existing `osRun` process built-in; this is VM-mediated process integration, not a native VM opcode or in-process provider binding.
