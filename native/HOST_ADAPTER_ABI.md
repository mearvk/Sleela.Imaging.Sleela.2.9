# Imaging host adapter ABI

The host adapter is the explicit boundary between SLeeLa's host-side native
loader and the imaging provider. It dynamically loads only the provider path
passed by the caller; it does not search the process path or load a provider
implicitly.

## Contract

- Adapter ABI: `SLI_HOST_ABI_VERSION 1`.
- Provider ABI: `SLI_PROVIDER_ABI_VERSION 1`; all required exports are checked.
- Provider path is explicit and must be trusted by the host.
- The adapter owns the loaded library and provider instance until
  `sli_host_close`.
- Preview and apply are delegated unchanged to the provider. The provider
  retains its process-local plan and enforces confirmation, dry-run, digest,
  revalidation, and no-overwrite rules.
- The adapter exposes no raw OS handle or provider pointer to SLeeLa source.

## SLeeLa loader handoff

The SLeeLa core already has a cross-platform dynamic-library loader in
`impl/core/sleela_library.h` (`sllibrary_open`, `sllibrary_symbol`,
`sllibrary_close`). A VM native-call binding can load this adapter and resolve
these stable C exports:

`sli_host_abi_version`, `sli_host_open`, `sli_host_close`,
`sli_host_preview`, and `sli_host_apply`.

The adapter is now implemented, but it is **not yet registered as a VM
opcode/builtin**. The SLeeLa compiler's existing `ui*` built-ins are
statically mapped to `OP_UI_CALL`; that mechanism does not automatically
discover arbitrary shared-library exports. The remaining integration is a
dedicated imaging VM bridge with bounded VM-local handles, compiler name/arity
registration, core dispatch, and build linkage. Until that is merged and
tested, the desktop UI must continue to report the filesystem provider as
unbound and Apply must remain disabled.

## Build

The adapter is built alongside the provider as a shared library. On POSIX it
uses `dlopen/dlsym/dlclose`; on Windows it uses `LoadLibraryA/GetProcAddress/
FreeLibrary`. No provider symbols are linked directly.
