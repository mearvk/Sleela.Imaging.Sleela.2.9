#ifndef SLEELA_IMAGING_PROVIDER_H
#define SLEELA_IMAGING_PROVIDER_H
#include <stdint.h>
#if defined(_WIN32)
# if defined(SLI_PROVIDER_BUILD)
#  define SLI_API __declspec(dllexport)
# else
#  define SLI_API __declspec(dllimport)
# endif
#else
# define SLI_API __attribute__((visibility("default")))
#endif
#ifdef __cplusplus
extern "C" {
#endif
#define SLI_PROVIDER_ABI_VERSION 2u
#define SLI_PATH_CAPACITY 4096u
#define SLI_STATUS_CAPACITY 256u
typedef struct sli_provider sli_provider;
typedef struct {
 char source[SLI_PATH_CAPACITY], images[SLI_PATH_CAPACITY], mov[SLI_PATH_CAPACITY];
 char mp4[SLI_PATH_CAPACITY], mpeg[SLI_PATH_CAPACITY], avi[SLI_PATH_CAPACITY];
 uint32_t recursive, dry_run, verify_sha256;
} sli_config;
typedef struct {
 uint64_t files_seen, eligible, unknown_timestamp, collisions, published, failed;
 char status[SLI_STATUS_CAPACITY];
} sli_result;
typedef struct {
 char source[SLI_PATH_CAPACITY], destination[SLI_PATH_CAPACITY], state[32];
 char sha256[65]; /* lowercase SHA-256 captured during preview; empty when disabled */
 uint64_t bytes;
 int64_t modified_time_ns;
} sli_item;
SLI_API uint32_t sli_provider_abi_version(void);
SLI_API const char *sli_provider_version(void);
SLI_API sli_provider *sli_provider_create(void);
SLI_API void sli_provider_destroy(sli_provider *provider);
/* Call with items=nullptr, then allocate required entries and call again. */
SLI_API int32_t sli_provider_preview(sli_provider *, const sli_config *, sli_result *,
                             sli_item *, uint64_t capacity, uint64_t *required);
/* Apply only accepts the exact process-local plan produced by the last preview. */
SLI_API int32_t sli_provider_apply(sli_provider *, uint32_t confirmed, sli_result *);
#ifdef __cplusplus
}
#endif
#endif
