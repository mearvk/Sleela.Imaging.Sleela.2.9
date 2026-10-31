# SLeeLa VM integration

The imaging provider is callable from an existing SLeeLa runtime without changing
or rebuilding the VM. This integration uses the VM's existing OS process built-ins
(`osRun` / `osSpawn`) as the boundary and keeps the native imaging ABI in a
separate executable.

## Build outputs

- `sleela_imaging_provider`: filesystem provider shared library
- `sleela_imaging_host`: explicit-path dynamic provider loader and ABI adapter
- `sleela-imaging-bridge`: command-line process adapter callable by SLeeLa

The bridge accepts six distinct filesystem roots and an explicit provider-library
path. It does not search the process library path for the provider.

## Preview

Invoke the bridge with `--provider`, `--source`, `--images`, `--mov`,
`--mp4`, `--mpeg`, and `--avi`. Add `--recursive` to scan subdirectories.
Without apply flags, the bridge is preview-only and does not change files.

## Apply

Use `--apply --confirm` together in one invocation. The bridge opens the provider,
creates a fresh preview, and applies that same process-local plan before closing
the provider. Apply is rejected unless the provider accepts the explicit
confirmation, SHA-256 verification is enabled, and its source revalidation and
atomic no-overwrite publication checks succeed.

Do not split preview and apply across separate bridge processes: the provider's
preview authorization is deliberately process-local.

## VM safety boundary

SLeeLa source passes strings and receives an integer exit status through its
existing OS bridge. No provider pointer, library handle, file descriptor, or
native address is exposed to Sleela code. The process boundary also means the
runtime itself does not acquire a new native dependency.

The current change adds the executable bridge and build target. The graphical
`ImagingWindow` still needs its Preview and Apply event handlers wired to invoke
this process with correctly quoted arguments before it can claim an operational
GUI filesystem session. Until that wiring is complete, the GUI must continue to
show the filesystem bridge as unbound and keep its Apply control disabled.
