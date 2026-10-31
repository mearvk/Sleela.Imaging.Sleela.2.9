# SLeeLa Imaging 2.9

A SLeeLa-native port of the media-organizing core from Java.Imaging.Java.21.

Language: SLeeLa syntax 2.9.  
Language reference: https://github.com/mearvk/SLeeLa  
Source application: https://github.com/mearvk/Java.Imaging.Java.21

Features: classify common image/video extensions; prefer embedded capture time then filesystem mtime; plan chronological names; route videos by extension; dry-run by default; apply only after explicit confirmation and verified copy; never delete source media.

The SLeeLa filesystem SDK front ends cross a native VM/OS bridge. This repository is a source port and integration contract, not a claim of an end-to-end executable until those operations are bound and tested. Check the installed compiler with: sleelvac --help.

The port removes automatic public-key uploads, background certificate collection, unsolicited HTTP calls, embedded database credentials, and copy-then-delete behavior. See PORTING.md and SECURITY.md.
