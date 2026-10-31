# Format support registry

Classification means an extension is recognized. Metadata support means a tested adapter can extract and normalize a trustworthy capture/creation timestamp. These are not equivalent.

| Format | Extension classification in source | Metadata extraction status | Release evidence |
|---|---|---|---|
| JPEG | Recognized | Adapter required; inspect EXIF | Real fixtures and malformed-input tests |
| TIFF | Recognized | Adapter required | Real fixtures and malformed-input tests |
| PNG | Recognized | Not established | Adapter and fixtures required |
| HEIC | Recognized | Not established | Adapter and fixtures required |
| BMP | Recognized | Not established | Adapter and fixtures required |
| GIF | Recognized | Not established | Adapter and fixtures required |
| WebP | Recognized | Not established | Adapter and fixtures required |
| MOV | Recognized | Not established | Container metadata adapter required |
| MP4/M4V | Recognized | Not established | Container metadata adapter required |
| MPEG/MPG | Recognized | Not established | Container metadata adapter required |
| AVI | Recognized | Not established | Container metadata adapter required |

## Rules

- Never infer metadata support from an extension check.
- Record which tag/field supplied the date and preserve the raw value for diagnostics where privacy permits.
- Normalize timezone-aware values consistently; do not silently interpret ambiguous local times as UTC.
- Reject malformed or out-of-range timestamps.
- Keep unsupported formats unchanged and report them as skipped.
- Add a format to the supported-metadata list only after fixtures, parser tests, and security review pass.
