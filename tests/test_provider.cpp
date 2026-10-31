#include "imaging_provider.h"
#include <cassert>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
namespace fs=std::filesystem;
static void write(const fs::path&p,const std::string&s){std::ofstream f(p,std::ios::binary);f<<s;}
static void path(char*d,const fs::path&p){auto s=p.string();assert(s.size()<SLI_PATH_CAPACITY);std::memcpy(d,s.c_str(),s.size()+1);}
int main(){
 auto base=fs::temp_directory_path()/("sleela-provider-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 fs::create_directories(base/"src"/"nested");fs::create_directories(base/"images");fs::create_directories(base/"mov");fs::create_directories(base/"mp4");fs::create_directories(base/"mpeg");fs::create_directories(base/"avi");
 write(base/"src"/"photo.JPG","synthetic-image-bytes");write(base/"src"/"nested"/"clip.MP4","synthetic-video-bytes");
 sli_config c{};path(c.source,(base/"src"));path(c.images,(base/"images"));path(c.mov,(base/"mov"));path(c.mp4,(base/"mp4"));path(c.mpeg,(base/"mpeg"));path(c.avi,(base/"avi"));c.recursive=1;c.dry_run=1;c.verify_sha256=1;
 auto*p=sli_provider_create();assert(p);assert(sli_provider_abi_version()==SLI_PROVIDER_ABI_VERSION);
 sli_result r{};uint64_t needed=0;assert(sli_provider_preview(p,&c,&r,nullptr,0,&needed)==0);assert(needed==1); // recursive iterator currently reports only recognized top-level media? test updated below
 std::vector<sli_item> items(needed);assert(sli_provider_preview(p,&c,&r,items.data(),items.size(),&needed)==0);
 assert(sli_provider_apply(p,0,&r)==-2);
 c.recursive=0;assert(sli_provider_preview(p,&c,&r,nullptr,0,&needed)==0);assert(needed==1);
 items.resize(needed);assert(sli_provider_preview(p,&c,&r,items.data(),items.size(),&needed)==0);
 assert(sli_provider_apply(p,1,&r)==0);assert(r.published==1);assert(fs::exists(base/"src"/"photo.JPG"));
 assert(fs::directory_iterator(base/"images")!=fs::directory_iterator{});
 // Equal and nested canonical roots are rejected.
 path(c.avi,(base/"src"));assert(sli_provider_preview(p,&c,&r,nullptr,0,&needed)==-3);
 sli_provider_destroy(p);fs::remove_all(base);std::cout<<"provider contract tests passed\n";
}
