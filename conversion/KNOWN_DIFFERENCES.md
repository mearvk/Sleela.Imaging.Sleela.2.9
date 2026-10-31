# Known differences and unfinished work

- The current SLeeLa application creates candidate plans; it does not yet enumerate a directory or execute a complete organization run.
- The filesystem adapter returns bridge-required sentinels/defaults. It is not a functioning filesystem implementation.
- JPEG/TIFF parser behavior in the Java source was narrower than the extension claims. Other formats require verified metadata adapters.
- Date parsing, timezone normalization, EXIF orientation, video creation-time precedence, and malformed metadata handling are not yet implemented end to end.
- Naming collision allocation and cross-volume publication semantics need implementation and tests.
- Configuration loading, validation beyond basic fields, and migration are incomplete.
- SLeeLa compilation and runtime execution have not been verified in this conversion stage.
