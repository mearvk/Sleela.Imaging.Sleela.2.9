# Classification cases

Required cases:

- Lowercase and uppercase extensions classify identically.
- Leading-dot extensions are recognized.
- Missing extension and empty input are unsupported.
- A misleading extension does not establish file content or metadata validity.
- Every extension in `docs/FORMAT_SUPPORT.md` is covered.
- Unknown executable/archive/document extensions are skipped and left untouched.
- Image extensions never classify as video and vice versa.
