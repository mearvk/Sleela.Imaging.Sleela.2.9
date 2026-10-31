#include "imaging_host_adapter.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {
bool copy_path(char *dst, const std::string &src) {
    if (src.empty() || src.size() >= SLI_PATH_CAPACITY) return false;
    std::memcpy(dst, src.c_str(), src.size() + 1);
    return true;
}
void usage() {
    std::fprintf(stderr,
        "Usage: sleela-imaging-bridge --provider PATH --source DIR --images DIR "
        "--mov DIR --mp4 DIR --mpeg DIR --avi DIR [--recursive] [--apply --confirm]\n"
        "Default mode is preview-only. Apply requires both --apply and --confirm.\n");
}
}

int main(int argc, char **argv) {
    std::string provider, source, images, mov, mp4, mpeg, avi;
    bool recursive = false, apply = false, confirmed = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        auto value = [&](std::string &out) -> bool {
            if (i + 1 >= argc) return false;
            out = argv[++i];
            return !out.empty();
        };
        if (arg == "--provider") { if (!value(provider)) { usage(); return 2; } }
        else if (arg == "--source") { if (!value(source)) { usage(); return 2; } }
        else if (arg == "--images") { if (!value(images)) { usage(); return 2; } }
        else if (arg == "--mov") { if (!value(mov)) { usage(); return 2; } }
        else if (arg == "--mp4") { if (!value(mp4)) { usage(); return 2; } }
        else if (arg == "--mpeg") { if (!value(mpeg)) { usage(); return 2; } }
        else if (arg == "--avi") { if (!value(avi)) { usage(); return 2; } }
        else if (arg == "--recursive") recursive = true;
        else if (arg == "--apply") apply = true;
        else if (arg == "--confirm") confirmed = true;
        else { usage(); return 2; }
    }
    if (provider.empty() || (apply != confirmed)) { usage(); return 2; }

    sli_config config{};
    if (!copy_path(config.source, source) || !copy_path(config.images, images) ||
        !copy_path(config.mov, mov) || !copy_path(config.mp4, mp4) ||
        !copy_path(config.mpeg, mpeg) || !copy_path(config.avi, avi)) {
        std::fprintf(stderr, "ERROR: all six paths are required and must be shorter than %u bytes.\n",
                     SLI_PATH_CAPACITY);
        return 2;
    }
    config.recursive = recursive ? 1u : 0u;
    config.dry_run = apply ? 0u : 1u;
    config.verify_sha256 = 1u;

    char error[SLI_STATUS_CAPACITY]{};
    sli_host *host = sli_host_open(provider.c_str(), error, sizeof(error));
    if (!host) {
        std::fprintf(stderr, "ERROR: %s\n", error[0] ? error : "provider open failed");
        return 3;
    }

    sli_result result{};
    uint64_t required = 0;
    int32_t rc = sli_host_preview(host, &config, &result, nullptr, 0, &required);
    if (rc < 0 || required > 1000000u) {
        std::fprintf(stderr, "ERROR: preview sizing failed (status=%d, required=%llu).\n",
                     (int)rc, (unsigned long long)required);
        sli_host_close(host);
        return 4;
    }
    // Keep a non-null output buffer even for an empty plan: the provider marks
    // the preview ready only when the caller supplies an output buffer.
    std::vector<sli_item> items((size_t)(required ? required : 1u));
    rc = sli_host_preview(host, &config, &result, items.data(),
                          (uint64_t)items.size(), &required);
    if (rc < 0) {
        std::fprintf(stderr, "ERROR: preview failed (status=%d): %s\n", (int)rc, result.status);
        sli_host_close(host);
        return 4;
    }
    std::printf("PREVIEW %s\nseen=%llu eligible=%llu unknown_timestamp=%llu collisions=%llu failed=%llu\n",
        result.status, (unsigned long long)result.files_seen, (unsigned long long)result.eligible,
        (unsigned long long)result.unknown_timestamp, (unsigned long long)result.collisions,
        (unsigned long long)result.failed);
    for (uint64_t i = 0; i < required; ++i) {
        const auto &item = items[(size_t)i];
        std::printf("%s -> %s [%s, %llu bytes]
", item.source, item.destination,
                    item.state, (unsigned long long)item.bytes);
    }

    if (apply) {
        sli_result applied{};
        rc = sli_host_apply(host, confirmed ? 1u : 0u, &applied);
        std::printf("APPLY %s (status=%d, published=%llu, failed=%llu)\n", applied.status,
                    (int)rc, (unsigned long long)applied.published, (unsigned long long)applied.failed);
        sli_host_close(host);
        return rc < 0 || applied.failed ? 5 : 0;
    }
    std::puts("PREVIEW ONLY: no files were changed. Re-run with --apply --confirm to authorize this exact plan.");
    sli_host_close(host);
    return 0;
}
