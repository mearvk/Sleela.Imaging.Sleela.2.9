#ifndef SLEELA_IMAGING_PROVIDER_H
#define SLEELA_IMAGING_PROVIDER_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define SLI_PROVIDER_ABI_VERSION 1u
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
 uint64_t bytes;
 int64_t modified_time_ns;
} sli_item;
uint32_t sli_provider_abi_version(void);
const char *sli_provider_version(void);
sli_provider *sli_provider_create(void);
void sli_provider_destroy(sli_provider *provider);
/* Call with items=nullptr, then allocate required entries and call again. */
int32_t sli_provider_preview(sli_provider *, const sli_config *, sli_result *,
                             sli_item *, uint64_t capacity, uint64_t *required);
/* Apply only accepts the exact process-local plan produced by the last preview. */
int32_t sli_provider_apply(sli_provider *, uint32_t confirmed, sli_result *);
#ifdef __cplusplus
}
#endif
#endif
