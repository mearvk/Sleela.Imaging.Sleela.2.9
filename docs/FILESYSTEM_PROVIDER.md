# Native filesystem provider

This document defines the next integration milestone: a versioned C ABI provider for local filesystem enumeration, preview, verified copy, and atomic no-overwrite publication. The provider must not delete or modify source files, follow symbolic links, access the network, or transmit metadata. The SLeeLa VM binding remains unbound until a native host adapter is registered and runtime-tested.
