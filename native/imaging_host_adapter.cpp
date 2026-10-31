#include "imaging_host_adapter.h"
#include <cstdio>
#include <cstring>
#include <new>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace {
#ifdef _WIN32
using Library = HMODULE;
static Library load_library(const char *p) { return LoadLibraryA(p); }
static void *load_symbol(Library h, const char *n) { return reinterpret_cast<void *>(GetProcAddress(h,n)); }
static void unload_library(Library h) { if(h) FreeLibrary(h); }
static void error_text(char *out, uint64_t cap, const char *prefix) {
 if(!out || !cap) return;
 DWORD e=GetLastError(); std::snprintf(out,(size_t)cap,"%s (Windows loader error %lu)",prefix,(unsigned long)e);
}
#else
using Library = void *;
static Library load_library(const char *p) { dlerror(); return dlopen(p,RTLD_NOW|RTLD_LOCAL); }
static void *load_symbol(Library h, const char *n) { dlerror(); return dlsym(h,n); }
static void unload_library(Library h) { if(h) dlclose(h); }
static void error_text(char *out, uint64_t cap, const char *prefix) {
 if(!out || !cap) return;
 const char *e=dlerror(); std::snprintf(out,(size_t)cap,"%s: %s",prefix,e?e:"unknown loader error");
}
#endif
using AbiFn=uint32_t (*)();
using VersionFn=const char *(*)();
using CreateFn=sli_provider *(*)();
using DestroyFn=void (*)(sli_provider *);
using PreviewFn=int32_t (*)(sli_provider *,const sli_config *,sli_result *,sli_item *,uint64_t,uint64_t *);
using ApplyFn=int32_t (*)(sli_provider *,uint32_t,sli_result *);
}
struct sli_host {
 Library library{};
 sli_provider *provider{};
 DestroyFn destroy{};
 PreviewFn preview{};
 ApplyFn apply{};
};
extern "C" {
uint32_t sli_host_abi_version(void) { return SLI_HOST_ABI_VERSION; }
sli_host *sli_host_open(const char *path,char *error,uint64_t cap) {
 if(error && cap) error[0]='\0';
 if(!path || !*path) { if(error&&cap) std::snprintf(error,(size_t)cap,"provider path is required"); return nullptr; }
 Library lib=load_library(path);
 if(!lib) { error_text(error,cap,"could not load imaging provider"); return nullptr; }
 auto abi=reinterpret_cast<AbiFn>(load_symbol(lib,"sli_provider_abi_version"));
 auto create=reinterpret_cast<CreateFn>(load_symbol(lib,"sli_provider_create"));
 auto destroy=reinterpret_cast<DestroyFn>(load_symbol(lib,"sli_provider_destroy"));
 auto preview=reinterpret_cast<PreviewFn>(load_symbol(lib,"sli_provider_preview"));
 auto apply=reinterpret_cast<ApplyFn>(load_symbol(lib,"sli_provider_apply"));
 if(!abi || !create || !destroy || !preview || !apply || abi()!=SLI_PROVIDER_ABI_VERSION) {
  if(error&&cap) std::snprintf(error,(size_t)cap,"provider ABI mismatch or required export missing");
  unload_library(lib); return nullptr;
 }
 sli_provider *p=create();
 if(!p) { if(error&&cap) std::snprintf(error,(size_t)cap,"provider initialization failed"); unload_library(lib); return nullptr; }
 auto *host=new(std::nothrow) sli_host{};
 if(!host) { destroy(p); unload_library(lib); if(error&&cap) std::snprintf(error,(size_t)cap,"host allocation failed"); return nullptr; }
 host->library=lib; host->provider=p; host->destroy=destroy; host->preview=preview; host->apply=apply;
 return host;
}
void sli_host_close(sli_host *h) {
 if(!h) return;
 if(h->provider && h->destroy) h->destroy(h->provider);
 unload_library(h->library);
 delete h;
}
int32_t sli_host_preview(sli_host *h,const sli_config *c,sli_result *r,sli_item *items,uint64_t cap,uint64_t *required) {
 if(!h || !h->preview || !c || !r || !required) return -1;
 return h->preview(h->provider,c,r,items,cap,required);
}
int32_t sli_host_apply(sli_host *h,uint32_t confirmed,sli_result *r) {
 if(!h || !h->apply || !r) return -1;
 /* The provider remains the authority for exact-plan and safety validation. */
 return h->apply(h->provider,confirmed?1u:0u,r);
}
}
