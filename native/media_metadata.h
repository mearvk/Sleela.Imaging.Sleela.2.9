#ifndef SLEELA_MEDIA_METADATA_H
#define SLEELA_MEDIA_METADATA_H
#include <cstdint>
#include <string>

namespace sleela_media {
struct CaptureTime {
    bool found = false;
    int64_t unix_time_ns = 0;
    std::string source;
    bool timezone_known = false;
};

// Reads embedded capture/creation metadata only. Malformed or unsupported metadata
// is treated as absent; callers must retain their filesystem-time fallback.
CaptureTime read_capture_time(const std::string &path) noexcept;
}
#endif
