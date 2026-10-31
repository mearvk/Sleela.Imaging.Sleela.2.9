#include "imaging_provider.h"
#include <openssl/evp.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cctype>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>
namespace fs = std::filesystem;
namespace {
struct Plan { fs::path src, dst; uint64_t bytes=0; int64_t modified=0; std::string state="planned"; };
std::atomic<uint64_t> temp_sequence{1};
bool put(char *dst,size_t cap,const std::string &s){if(s.size()>=cap)return false;std::memcpy(dst,s.c_str(),s.size()+1);return true;}
std::string lower(std::string s){for(char &c:s)c=char(std::tolower(static_cast<unsigned char>(c)));return s;}
std::string category(const fs::path &p){const auto e=lower(p.extension().string());static const std::vector<std::string> images={".jpg",".jpeg",".heic",".png",".tif",".tiff",".bmp",".gif",".webp"};static const std::vector<std::string> videos={".mov",".mp4",".mpeg",".mpg",".avi",".m4v"};if(std::find(images.begin(),images.end(),e)!=images.end())return "image";if(std::find(videos.begin(),videos.end(),e)!=videos.end())return "video";return {};}
bool inside(const fs::path &p,const fs::path &root){auto rel=p.lexically_relative(root);return !rel.empty()&&!rel.is_absolute()&&*rel.begin()!="..";}
bool overlaps(const fs::path &a,const fs::path &b){return a==b||inside(a,b)||inside(b,a);}
fs::path canonical_root(const char *s){if(!s||!std::memchr(s,'\\0',SLI_PATH_CAPACITY)||!*s)throw std::runtime_error("empty directory root");std::error_code ec;auto p=fs::canonical(fs::path(s),ec);if(ec||!fs::is_directory(p))throw std::runtime_error("all six roots must be existing directories");return p;}
int64_t modified_ns(const fs::path &p){std::error_code ec;auto t=fs::last_write_time(p,ec);if(ec)throw std::runtime_error("cannot read modification time");return std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch()).count();}
std::string timestamp(int64_t ns){auto ft=fs::file_time_type(std::chrono::nanoseconds(ns));auto st=std::chrono::time_point_cast<std::chrono::system_clock::duration>(ft-fs::file_time_type::clock::now()+std::chrono::system_clock::now());auto raw=std::chrono::system_clock::to_time_t(st);std::tm tm{};
#ifdef _WIN32
 localtime_s(&tm,&raw);
#else
 localtime_r(&raw,&tm);
#endif
 std::ostringstream out;out<<std::put_time(&tm,"%Y-%m-%d_%H-%M-%S");return out.str();}
std::string sha256(const fs::path &p){std::ifstream in(p,std::ios::binary);if(!in)throw std::runtime_error("cannot open file for SHA-256");EVP_MD_CTX *ctx=EVP_MD_CTX_new();if(!ctx)throw std::runtime_error("cannot allocate digest context");unsigned char digest[EVP_MAX_MD_SIZE];unsigned int length=0;bool ok=EVP_DigestInit_ex(ctx,EVP_sha256(),nullptr)==1;char buffer[65536];while(ok&&in){in.read(buffer,sizeof(buffer));auto n=in.gcount();if(n>0)ok=EVP_DigestUpdate(ctx,buffer,static_cast<size_t>(n))==1;}ok=ok&&in.eof()&&EVP_DigestFinal_ex(ctx,digest,&length)==1;EVP_MD_CTX_free(ctx);if(!ok||length!=32)throw std::runtime_error("SHA-256 operation failed");std::ostringstream out;out<<std::hex<<std::setfill('0');for(unsigned i=0;i<length;++i)out<<std::setw(2)<<static_cast<unsigned>(digest[i]);return out.str();}
}
struct sli_provider { std::vector<fs::path> roots; std::vector<Plan> plan; bool ready=false; bool digest=false; bool dry_run=true; };
extern "C" {
uint32_t sli_provider_abi_version(void){return SLI_PROVIDER_ABI_VERSION;}
const char *sli_provider_version(void){return "1.0.0";}
sli_provider *sli_provider_create(void){try{return new sli_provider();}catch(...){return nullptr;}}
void sli_provider_destroy(sli_provider *p){delete p;}
int32_t sli_provider_preview(sli_provider *p,const sli_config *c,sli_result *r,sli_item *out,uint64_t capacity,uint64_t *required){
 if(!p||!c||!r||!required)return -1;
 std::memset(r,0,sizeof(*r));*required=0;p->ready=false;p->plan.clear();
 try{
  p->roots={canonical_root(c->source),canonical_root(c->images),canonical_root(c->mov),canonical_root(c->mp4),canonical_root(c->mpeg),canonical_root(c->avi)};
  for(size_t i=0;i<p->roots.size();++i)for(size_t j=i+1;j<p->roots.size();++j)if(overlaps(p->roots[i],p->roots[j]))throw std::runtime_error("canonical directory roots overlap");
  auto add=[&](const fs::directory_entry &entry){
   std::error_code ec;if(entry.is_symlink(ec)||ec)return;if(!entry.is_regular_file(ec)||ec||category(entry.path()).empty())return;
   auto source=fs::canonical(entry.path(),ec);if(ec||!inside(source,p->roots[0]))return;
   Plan item;item.src=source;item.bytes=fs::file_size(source,ec);if(ec)return;
   try{item.modified=modified_ns(source);}catch(...){++r->unknown_timestamp;return;}
   p->plan.push_back(std::move(item));
  };
  if(c->recursive){fs::recursive_directory_iterator it(p->roots[0],fs::directory_options::skip_permission_denied),end;for(;it!=end;){std::error_code ec;if(it->is_symlink(ec))it.disable_recursion_pending();else if(!ec)add(*it);it.increment(ec);}}
  else {fs::directory_iterator it(p->roots[0]),end;for(;it!=end;++it)add(*it);}
  r->files_seen=p->plan.size()+r->unknown_timestamp;r->eligible=p->plan.size();
  std::sort(p->plan.begin(),p->plan.end(),[](const Plan&a,const Plan&b){return a.modified==b.modified?a.src<b.src:a.modified<b.modified;});
  std::map<std::string,unsigned> sequence;
  for(auto &item:p->plan){
   const auto ext=lower(item.src.extension().string());const auto type=category(item.src);
   fs::path destination=type=="image"?p->roots[1]:(ext==".mov"?p->roots[2]:(ext==".mp4"||ext==".m4v")?p->roots[3]:(ext==".mpeg"||ext==".mpg")?p->roots[4]:p->roots[5]);
   const auto stamp=timestamp(item.modified);const auto key=stamp+"|"+ext;std::ostringstream name;name<<stamp<<"_"<<std::setw(3)<<std::setfill('0')<<++sequence[key]<<ext;item.dst=destination/name.str();
   if(fs::exists(item.dst)){item.state="collision-skipped";++r->collisions;}
  }
  *required=p->plan.size();if(out&&capacity<*required){put(r->status,sizeof(r->status),"BUFFER_TOO_SMALL");return -2;}
  if(out)for(size_t i=0;i<p->plan.size();++i){const auto &item=p->plan[i];if(!put(out[i].source,sizeof(out[i].source),item.src.u8string())||!put(out[i].destination,sizeof(out[i].destination),item.dst.u8string())||!put(out[i].state,sizeof(out[i].state),item.state))throw std::runtime_error("UTF-8 path exceeds ABI buffer");out[i].bytes=item.bytes;out[i].modified_time_ns=item.modified;}
  p->digest=c->verify_sha256!=0;p->dry_run=c->dry_run!=0;p->ready=(out!=nullptr);put(r->status,sizeof(r->status),c->dry_run?"Preview complete; no files changed.":"Preview complete; apply requires explicit confirmation.");return 0;
 }catch(const std::exception &e){put(r->status,sizeof(r->status),e.what());return -3;}
}
int32_t sli_provider_apply(sli_provider *p,uint32_t confirmed,sli_result *r){
 if(!p||!r) return -1;
 std::memset(r,0,sizeof(*r));
 if(!p->ready||p->dry_run||!confirmed||!p->digest){put(r->status,sizeof(r->status),"DENIED: fresh preview, confirmation, and SHA-256 are required.");return -2;}
 for(auto &item:p->plan){
  if(item.state=="collision-skipped")continue;
  fs::path temporary;try{
   std::error_code ec;auto status=fs::symlink_status(item.src,ec);
   if(ec||fs::is_symlink(status)||!fs::is_regular_file(status))throw std::runtime_error("source changed type or became a symbolic link");
   if(fs::file_size(item.src)!=item.bytes||modified_ns(item.src)!=item.modified)throw std::runtime_error("source changed since preview");
   if(fs::exists(item.dst))throw std::runtime_error("destination collision appeared after preview");
   temporary=item.dst.parent_path()/(item.dst.filename().string()+".sleela-tmp-"+std::to_string(temp_sequence.fetch_add(1)));
   fs::copy_file(item.src,temporary,fs::copy_options::none);
   if(fs::file_size(temporary)!=item.bytes||sha256(item.src)!=sha256(temporary))throw std::runtime_error("copy byte-count or SHA-256 verification failed");
   fs::create_hard_link(temporary,item.dst,ec);
   if(ec) throw std::runtime_error("atomic no-overwrite publication failed: "+ec.message());
   fs::remove(temporary,ec);item.state="published-verified";++r->published;
  }catch(const std::exception &){if(!temporary.empty()){std::error_code ignored;fs::remove(temporary,ignored);}item.state="failed";++r->failed;}
 }
 p->ready=false;put(r->status,sizeof(r->status),"Apply finished. Originals were not deleted or modified.");return r->failed?1:0;
}
}
