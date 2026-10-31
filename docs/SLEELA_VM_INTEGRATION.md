# SLeeLa VM integration

The SLeeLa desktop application runs under the SLeeLa runtime and invokes the
native imaging bridge through the existing VM OS process built-in (`osRun`).
The bridge then loads the imaging provider in its own process. This is a real
VM-mediated process boundary, not an in-process VM opcode or native provider
binding. No VM opcode or runtime rebuild is required, and no native pointer,
library handle, or file descriptor crosses into SLeeLa values. The SLeeLa
`ImagingFileSystem` facade documents a future capability surface; the active UI
calls the process bridge directly.

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
and every planned source/destination pair with byte count, modification
timestamp, and preview-time SHA-256 digest. The provider-library path is also bound into the manifest. Path
strings are hex-encoded so unusual characters cannot change record boundaries.
A human-readable companion report is written to `REVIEW.plan.review.txt`.

Apply is a separate invocation:

```sh
sleela-imaging-bridge --provider PROVIDER --source SOURCE --images IMAGES \
  --mov MOV --mp4 MP4 --mpeg MPEG --avi AVI --expect-plan REVIEW.plan \
  --apply --confirm [--recursive]
```

The bridge reopens the provider, recomputes the current preview, and compares
the complete deterministic manifest byte-for-byte with the reviewed file.
Any changed root, option, file, destination, collision state, size, timestamp,
source digest, or summary denies apply. Only after an exact match does it apply
that same in-process plan. The provider re-hashes each source before copying
and verifies the temporary copy against the reviewed digest before publishing
without replacing an existing destination.

Keep the review manifest private to the local operation. Do not edit it. If
anything changes, choose a new unused plan path and run Preview again. Existing
manifests and reports are never overwritten, even when their headers match the
imaging format. Temporary artifacts are created exclusively and published
without replacing an existing destination.
The manifest is a comparison record, not a cryptographic signature or proof
of who approved it; the GUI's explicit confirmation is still required. The GUI
also binds the confirmation to the same bridge, provider, plan-file, and
configuration paths used for the successful preview.

## VM safety boundary

SLeeLa source passes a command string to the existing OS process service and
receives an integer exit status. No provider pointer, library handle, file
descriptor, or native address is exposed to SLeeLa code. The process boundary
does not add a native dependency to the VM.

The GUI must keep Apply disabled until a successful preview, an unchanged
configuration, SHA-256 verification, and the user's explicit confirmation.

## Integration test

After configuring and building, run the focused test with:

```sh
ctest --test-dir build -R imaging-bridge-reviewed-plan --output-on-failure
```

The test verifies that an unrelated existing plan file is not overwritten,
that a changed source invalidates a reviewed plan without publishing anything,
and that a fresh exact plan applies while preserving the original source.
