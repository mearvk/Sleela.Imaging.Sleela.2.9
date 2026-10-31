#include "imaging_provider.h"
#include <cassert>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
namespace fs=std::filesystem;
static void write(const fs::path&p,const std::string&s){std::ofstream f(p,std::ios::binary);f<<s;}
static void path(char*d,const fs::path&p){auto s=p.string();assert(s.size()<SLI_PATH_CAPACITY);std::memcpy(d,s.c_str(),s.size()+1);}
int main(){
 auto base=fs::temp_directory_path()/("sleela-provider-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 fs::create_directories(base/"src"/"nested");fs::create_directories(base/"images");fs::create_directories(base/"mov");fs::create_directories(base/"mp4");fs::create_directories(base/"mpeg");fs::create_directories(base/"avi");
 write(base/"src"/"photo.JPG","synthetic-image-bytes");write(base/"src"/"nested"/"clip.MP4","synthetic-video-bytes");
 sli_config c{};path(c.source,(base/"src"));path(c.images,(base/"images"));path(c.mov,(base/"mov"));path(c.mp4,(base/"mp4"));path(c.mpeg,(base/"mpeg"));path(c.avi,(base/"avi"));c.recursive=1;c.dry_run=1;c.verify_sha256=1;
 auto*p=sli_provider_create();assert(p);assert(sli_provider_abi_version()==SLI_PROVIDER_ABI_VERSION);
 sli_result r{};uint64_t needed=0;assert(sli_provider_preview(p,&c,&r,nullptr,0,&needed)==0);assert(needed==2);
 std::vector<sli_item> items(needed);assert(sli_provider_preview(p,&c,&r,items.data(),1,&needed)==-2);assert(sli_provider_apply(p,1,&r)==-2);assert(sli_provider_preview(p,&c,&r,items.data(),items.size(),&needed)==0);
 assert(sli_provider_apply(p,1,&r)==-2); // dry-run plans cannot be applied
 assert(sli_provider_apply(p,0,&r)==-2);
 c.recursive=0;c.dry_run=0;assert(sli_provider_preview(p,&c,&r,nullptr,0,&needed)==0);assert(needed==1);
 items.resize(needed);assert(sli_provider_preview(p,&c,&r,items.data(),items.size(),&needed)==0);
 assert(std::strlen(items[0].sha256)==64);\n assert(std::string(items[0].timestamp_source)=="filesystem-mtime");\n assert(items[0].organize_time_ns!=0);
 assert(std::string(items[0].sha256)=="ea154f2b845db9cf192c94024a7883180edb89a64dbfab16e4109619c43750a2"); // SHA-256 known vector
 auto reviewed_time=fs::last_write_time(base/"src"/"photo.JPG");
 write(base/"src"/"photo.JPG","Synthetic-image-bytes"); // same length; restore mtime to defeat metadata-only checks
 fs::last_write_time(base/"src"/"photo.JPG",reviewed_time);
 assert(sli_provider_apply(p,1,&r)==1);assert(r.failed==1&&r.published==0);
 assert(!fs::exists(fs::path(items[0].destination)));
 write(base/"src"/"photo.JPG","synthetic-image-bytes");
 assert(sli_provider_preview(p,&c,&r,nullptr,0,&needed)==0);assert(needed==1);
 items.resize(needed);assert(sli_provider_preview(p,&c,&r,items.data(),items.size(),&needed)==0);
 assert(sli_provider_apply(p,1,&r)==0);assert(r.published==1);assert(fs::exists(base/"src"/"photo.JPG"));
 assert(fs::exists(fs::path(items[0].destination)));assert(fs::file_size(fs::path(items[0].destination))==std::string("synthetic-image-bytes").size());
 // Equal and nested canonical roots are rejected.
 path(c.avi,(base/"src"));assert(sli_provider_preview(p,&c,&r,nullptr,0,&needed)==-3);
 sli_provider_destroy(p);fs::remove_all(base);std::cout<<"provider contract tests passed\n";
}
