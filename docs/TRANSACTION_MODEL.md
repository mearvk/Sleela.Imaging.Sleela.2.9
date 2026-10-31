# Transaction and recovery model

## Required phases

1. **Discover:** enumerate regular files under the canonical source root. Do not follow symbolic links by default.
2. **Classify:** accept only explicitly supported categories; preserve unknown files untouched.
3. **Resolve metadata:** record value, source, and confidence/provenance. Unknown remains unknown.
4. **Plan:** compute final paths, detect collisions, and capture a SHA-256 digest for every eligible source before any write. The reviewed manifest binds paths, options, sizes, timestamps, and those digests.
5. **Confirm:** require an explicit user confirmation for the exact plan. Dry-run remains the default.
6. **Stage:** copy to a unique temporary file in the destination filesystem.
7. **Verify:** re-hash the source against its preview-time digest, then compare staged byte count and SHA-256 against that same reviewed digest.
8. **Publish:** atomically create a no-replace hard link from the verified staged file. If unsupported, fail closed. Review manifests and reports also use exclusive temporary creation and no-replace publication.
9. **Report:** record each state and failure; retain source media.

## Recovery rules

- A staged file without a successful verification is not publishable.
- A verified staged file not yet published may be resumed only after rechecking source identity and destination collision state.
- Never delete the source as part of this transaction.
- Do not blindly delete temporary files after an ambiguous crash; identify them by transaction ID and report them for safe recovery.
- A retry must be idempotent or detect that the prior transaction already published the same content.

## Invariants

- Source bytes remain unchanged.
- Existing destination bytes remain unchanged.
- A published item has a successful content verification record.
- Every discovered supported item has a terminal result or an explicitly pending recovery record.
- Failure to guarantee a safety invariant means abort and report.
