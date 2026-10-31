# Naming cases

Test stable chronological ordering, deterministic collision suffixes, same-second bursts, more than 999 collisions, case-preserving or normalized extensions (document the chosen rule), Unicode names, Windows reserved names, trailing dots/spaces, invalid path characters, maximum path length, and timezone conversion.

The planner must detect collisions before applying. Never silently replace an existing file. If a name cannot be represented safely on the target filesystem, reject it and report why.
