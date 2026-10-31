#ifndef SLI_IMAGING_HOST_ADAPTER_H
#define SLI_IMAGING_HOST_ADAPTER_H
#include "imaging_provider.h"
#ifdef __cplusplus
extern "C" {
#endif
#if defined(_WIN32)
# if defined(SLI_HOST_ADAPTER_BUILD)
#  define SLI_HOST_API __declspec(dllexport)
# else
#  define SLI_HOST_API __declspec(dllimport)
# endif
#else
# define SLI_HOST_API __attribute__((visibility("default")))
#endif
#define SLI_HOST_ABI_VERSION 1u
typedef struct sli_host sli_host;
/* Loads the provider by explicit path; no search-path probing is performed. */
SLI_HOST_API uint32_t sli_host_abi_version(void);
SLI_HOST_API sli_host *sli_host_open(const char *provider_library_path, char *error, uint64_t error_capacity);
SLI_HOST_API void sli_host_close(sli_host *host);
SLI_HOST_API int32_t sli_host_preview(sli_host *host, const sli_config *config, sli_result *result,
                                      sli_item *items, uint64_t capacity, uint64_t *required);
SLI_HOST_API int32_t sli_host_apply(sli_host *host, uint32_t explicit_confirmation, sli_result *result);
#ifdef __cplusplus
}
#endif
#endif
