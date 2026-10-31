# Cross-platform requirements

Target operating systems: Linux, Windows 10+, and macOS.

| Concern | Linux | Windows | macOS |
|---|---|---|---|
| Canonical path and containment | Required | Required | Required |
| Case sensitivity behavior | Filesystem-dependent | Commonly case-insensitive; do not assume | Filesystem-dependent |
| Unicode filenames | Required | Required | Required |
| Permission/error mapping | Required | ACL/error mapping | POSIX permissions/ACL mapping |
| Symlink/reparse-point handling | Required | Reparse-point handling | Required |
| Atomic publication | Same-filesystem semantics tested | Replace/rename semantics tested | Same-filesystem semantics tested |
| Timestamp precision/timezone | Probe and document | Probe and document | Probe and document |
| Long paths | Native limits documented | Long-path support tested | Native limits documented |
| Disk-full/interrupted writes | Recovery test | Recovery test | Recovery test |

The bridge must report capabilities rather than pretending that all filesystems have identical semantics. Cross-volume moves must not be represented as atomic renames; use staged copy, verification, and publication on the destination volume.
