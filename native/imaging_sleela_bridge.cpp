#include "imaging_host_adapter.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
bool copy_path(char *dst, const std::string &src) {
    if (src.empty() || src.size() >= SLI_PATH_CAPACITY) return false;
    std::memcpy(dst, src.c_str(), src.size() + 1);
    return true;
}
std::string hex(const std::string &value) {
    static const char digits[] = "0123456789abcdef";
    std::string out;
    out.reserve(value.size() * 2);
    for (unsigned char c : value) {
        out.push_back(digits[c >> 4]);
        out.push_back(digits[c & 15]);
    }
    return out;
}
std::string manifest(const sli_config &config, const sli_result &result,
                     const std::vector<sli_item> &items, uint64_t count) {
    std::ostringstream out;
    out << "SLEELA-IMAGING-PLAN-1\n";
    out << "source=" << hex(config.source) << "\n";
    out << "images=" << hex(config.images) << "\n";
    out << "mov=" << hex(config.mov) << "\n";
    out << "mp4=" << hex(config.mp4) << "\n";
    out << "mpeg=" << hex(config.mpeg) << "\n";
    out << "avi=" << hex(config.avi) << "\n";
    out << "recursive=" << config.recursive << "\n";
    out << "sha256=" << config.verify_sha256 << "\n";
    out << "seen=" << result.files_seen << "\neligible=" << result.eligible
        << "\nunknown_timestamp=" << result.unknown_timestamp
        << "\ncollisions=" << result.collisions << "\nfailed=" << result.failed << "\n";
    for (uint64_t i = 0; i < count; ++i) {
        const auto &item = items[static_cast<size_t>(i)];
        out << "item=" << hex(item.source) << "|" << hex(item.destination) << "|"
            << hex(item.state) << "|" << item.bytes << "|" << item.modified_time_ns << "\n";
    }
    return out.str();
}
bool write_manifest(const std::string &path, const std::string &data) {
    if (path.empty()) return false;
    const std::string temporary = path + ".sleela-tmp";
    {
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
        if (!file || !file.write(data.data(), static_cast<std::streamsize>(data.size()))) return false;
    }
    std::remove(path.c_str());
    if (std::rename(temporary.c_str(), path.c_str()) != 0) {
        std::remove(temporary.c_str());
        return false;
    }
    return true;
}
std::string display_path(const char *value) {
    std::string out;
    for (const unsigned char *p = reinterpret_cast<const unsigned char *>(value); *p; ++p) {
        if (*p == '\\n') out += "\\\\n";
        else if (*p == '\\r') out += "\\\\r";
        else if (*p == '\\t') out += "\\\\t";
        else if (*p == '\\\\') out += "\\\\\\\\";
        else out.push_back(static_cast<char>(*p));
    }
    return out;
}
bool write_review_text(const std::string &path, const sli_result &result,
                       const std::vector<sli_item> &items, uint64_t count) {
    std::ofstream file(path + ".review.txt", std::ios::binary | std::ios::trunc);
    if (!file) return false;
    file << "SLeeLa Imaging review report\\n"
         << "Files seen: " << result.files_seen << "\\nEligible: " << result.eligible
         << "\\nUnknown timestamp: " << result.unknown_timestamp
         << "\\nDestination collisions: " << result.collisions
         << "\\nPreview failures: " << result.failed << "\\n\\n";
    for (uint64_t i = 0; i < count; ++i) {
        const auto &item = items[static_cast<size_t>(i)];
        file << display_path(item.source) << " -> " << display_path(item.destination)
             << " [" << item.state << ", " << item.bytes << " bytes]\\n";
    }
    return static_cast<bool>(file);
}
bool read_manifest(const std::string &path, std::string &data) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    std::ostringstream buffer;
    buffer << file.rdbuf();
    if (!file.good() && !file.eof()) return false;
    data = buffer.str();
    return !data.empty();
}
void usage() {
    std::fprintf(stderr,
        "Preview: sleela-imaging-bridge --provider PATH --source DIR --images DIR "
        "--mov DIR --mp4 DIR --mpeg DIR --avi DIR --plan-file FILE [--recursive]\n"
        "Apply:   sleela-imaging-bridge --provider PATH --source DIR --images DIR "
        "--mov DIR --mp4 DIR --mpeg DIR --avi DIR --expect-plan FILE --apply --confirm [--recursive]\n"
        "Apply recomputes the plan and requires byte-for-byte equality with the reviewed manifest.\n");
}
}

int main(int argc, char **argv) {
    std::string provider, source, images, mov, mp4, mpeg, avi, plan_file, expect_plan;
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
        else if (arg == "--plan-file") { if (!value(plan_file)) { usage(); return 2; } }
        else if (arg == "--expect-plan") { if (!value(expect_plan)) { usage(); return 2; } }
        else if (arg == "--recursive") recursive = true;
        else if (arg == "--apply") apply = true;
        else if (arg == "--confirm") confirmed = true;
        else { usage(); return 2; }
    }
    if (provider.empty() || (apply != confirmed) ||
        (apply ? (!plan_file.empty() || expect_plan.empty()) : (plan_file.empty() || !expect_plan.empty()))) {
        usage(); return 2;
    }

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
                     static_cast<int>(rc), static_cast<unsigned long long>(required));
        sli_host_close(host);
        return 4;
    }
    std::vector<sli_item> items(static_cast<size_t>(required ? required : 1u));
    rc = sli_host_preview(host, &config, &result, items.data(),
                          static_cast<uint64_t>(items.size()), &required);
    if (rc < 0) {
        std::fprintf(stderr, "ERROR: preview failed (status=%d): %s\n", static_cast<int>(rc), result.status);
        sli_host_close(host);
        return 4;
    }
    const std::string current_manifest = manifest(config, result, items, required);
    std::printf("PREVIEW %s\nseen=%llu eligible=%llu unknown_timestamp=%llu collisions=%llu failed=%llu\n",
        result.status, static_cast<unsigned long long>(result.files_seen),
        static_cast<unsigned long long>(result.eligible),
        static_cast<unsigned long long>(result.unknown_timestamp),
        static_cast<unsigned long long>(result.collisions),
        static_cast<unsigned long long>(result.failed));
    for (uint64_t i = 0; i < required; ++i) {
        const auto &item = items[static_cast<size_t>(i)];
        std::printf("%s -> %s [%s, %llu bytes]\n", item.source, item.destination,
                    item.state, static_cast<unsigned long long>(item.bytes));
    }

    if (!apply) {
        if (!write_manifest(plan_file, current_manifest) ||\n            !write_review_text(plan_file, result, items, required)) {
            std::fprintf(stderr, "ERROR: could not write review manifest: %s\n", plan_file.c_str());
            sli_host_close(host);
            return 6;
        }
        std::printf("REVIEW MANIFEST: %s\nPREVIEW ONLY: no files were changed.\n", plan_file.c_str());
        sli_host_close(host);
        return 0;
    }

    std::string reviewed_manifest;
    if (!read_manifest(expect_plan, reviewed_manifest) || reviewed_manifest != current_manifest) {
        std::fprintf(stderr, "DENIED: current filesystem plan differs from the reviewed manifest. Preview again.\n");
        sli_host_close(host);
        return 7;
    }
    sli_result applied{};
    rc = sli_host_apply(host, 1u, &applied);
    std::printf("APPLY %s (status=%d, published=%llu, failed=%llu)\n", applied.status,
                static_cast<int>(rc), static_cast<unsigned long long>(applied.published),
                static_cast<unsigned long long>(applied.failed));
    sli_host_close(host);
    return rc < 0 || applied.failed ? 5 : 0;
}
