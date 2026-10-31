# Known differences and unfinished work

- The SLeeLa native UI is authored against the documented SleelaUI ui* VM bridge. It has not yet been compiled or run on a display.
- The UI can collect local configuration and report provider status. It does not yet enumerate media because the filesystem VM provider remains unbound.
- Apply is disabled and the SLeeLa session denies publication until a complete preview, explicit confirmation, canonical root proof, digest verification, source preservation, and atomic no-overwrite provider are available.
- The filesystem adapter returns BRIDGE_REQUIRED sentinels; it is a contract, not an implementation.
- The Java reference copies and deletes originals without destination digest verification. SLeeLa deliberately preserves source files and requires verified publication.
- The Java configured entry point also includes public-key transmission, remote key/certificate checks, entertainment polling, and optional database logging with embedded credentials. These behaviors are rejected from the local organizer core.
- The Java EXIF parser's visible implementation handles JPEG/TIFF paths only; it does not demonstrate HEIC/PNG metadata support or QuickTime capture-time parsing.
- Date parsing, timezone normalization, EXIF orientation, video creation-time precedence, malformed metadata handling, and GPS privacy behavior need format-specific implementations and tests.
- Naming collision allocation must inspect the real destination and fail closed.
- Secure configuration loading, schema validation, and migration are incomplete.
- SLeeLa compilation, SleelaUI rendering, VM provider binding, and Windows/macOS verification have not been demonstrated.
