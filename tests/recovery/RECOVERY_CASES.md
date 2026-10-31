# Recovery cases

Inject failure after each transaction phase: discovery, plan, staging, partial write, verification, publication, and report writing.

For every injection point verify: source remains intact; pre-existing destination remains intact; unverified temporary content is never published; retry does not duplicate or overwrite; ambiguous temporary files are reported; and the final report distinguishes failed, pending, skipped, and published records.
