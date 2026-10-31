# Date resolution cases

Required precedence: valid embedded capture time, then positive filesystem modification time, then unknown.

Test embedded time winning over a newer filesystem time; missing embedded time; zero and negative timestamps; malformed metadata; timezone offsets; daylight-saving transitions; ambiguous local time; leap day; out-of-range values; video container creation time; and timestamp precision differences.

Unknown must remain unknown. No test may accept the current clock as an implicit fallback.
