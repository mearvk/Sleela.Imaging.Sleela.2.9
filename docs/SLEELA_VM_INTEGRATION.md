# SLeeLa VM integration

The imaging application uses the existing SLeeLa VM process built-ins
(`osRun` / `osSpawn`). No VM opcode, runtime rebuild, or native pointer
crossing is required.

## Build outputs

- `sleela_imaging_provider`: filesystem provider shared library
- `sleela_imaging_host`: explicit-path dynamic provider loader and ABI adapter
- `sleela-imaging-bridge`: process adapter invoked by SLeeLa

The bridge requires an explicit provider-library path and six distinct,
existing filesystem roots. It does not search the process library path.

## Two-stage reviewed plan

Preview is read-only and writes a deterministic review manifest:

```sh
sleela-imaging-bridge --provider PROVIDER --source SOURCE --images IMAGES \
  --mov MOV --mp4 MP4 --mpeg MPEG --avi AVI --plan-file REVIEW.plan [--recursive]
```

The manifest records the six configured roots, scan options, summary counts,
and every planned source/destination pair with byte count and modification
timestamp. Path strings are hex-encoded in the manifest so unusual characters
cannot change its record boundaries.

Apply is a separate invocation:

```sh
sleela-imaging-bridge --provider PROVIDER --source SOURCE --images IMAGES \
  --mov MOV --mp4 MP4 --mpeg MPEG --avi AVI --expect-plan REVIEW.plan \
  --apply --confirm [--recursive]
```

The bridge reopens the provider, recomputes the current preview, and compares
the complete deterministic manifest byte-for-byte with the reviewed file.
Any changed root, option, file, destination, collision state, size, timestamp,
or summary denies apply. Only after an exact match does it apply that same
in-process plan. The provider independently revalidates each source, hashes
the copy, and publishes without overwriting an existing destination.

Keep the review manifest private to the local operation. Do not edit it. If
anything changes, run Preview again and review the replacement manifest.
The manifest is a comparison record, not a cryptographic signature or proof
of who approved it; the GUI's explicit confirmation is still required.

## VM safety boundary

SLeeLa source passes a command string to the existing OS process service and
receives an integer exit status. No provider pointer, library handle, file
descriptor, or native address is exposed to SLeeLa code. The process boundary
does not add a native dependency to the VM.

The GUI must keep Apply disabled until a successful preview, an unchanged
configuration, SHA-256 verification, and the user's explicit confirmation.
