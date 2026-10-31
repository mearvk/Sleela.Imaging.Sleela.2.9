# Native provider ABI

Implemented in native/imaging_provider.cpp and declared in native/imaging_provider.h (ABI v1). CMake builds the shared library and a synthetic-fixture contract test. The provider is not yet registered with the SLeeLa VM; do not mark the VM bridge bound until a host adapter and runtime integration test exist.
