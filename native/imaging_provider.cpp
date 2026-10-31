#include "imaging_provider.h"
#include "media_metadata.h"
#include <array>
#include <cstdint>
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
struct Plan { fs::path src, dst; uint64_t bytes=0; int64_t modified=0; int64_t organize_time=0; std::string timestamp_source="filesystem-mtime"; std::string digest; std::string state="planned"; };
std::atomic<uint64_t> temp_sequence{1};
bool put(char *dst,size_t cap,const std::string &s){if(s.size()>=cap)return false;std::memcpy(dst,s.c_str(),s.size()+1);return true;}
std::string lower(std::string s){for(char &c:s)c=char(std::tolower(static_cast<unsigned char>(c)));return s;}
std::string category(const fs::path &p){const auto e=lower(p.extension().string());static const std::vector<std::string> images={".jpg",".jpeg",".heic",".png",".tif",".tiff",".bmp",".gif",".webp"};static const std::vector<std::string> videos={".mov",".mp4",".mpeg",".mpg",".avi",".m4v"};if(std::find(images.begin(),images.end(),e)!=images.end())return "image";if(std::find(videos.begin(),videos.end(),e)!=videos.end())return "video";return {};}
bool inside(const fs::path &p,const fs::path &root){auto rel=p.lexically_relative(root);return !rel.empty()&&!rel.is_absolute()&&*rel.begin()!="..";}
bool overlaps(const fs::path &a,const fs::path &b){return a==b||inside(a,b)||inside(b,a);}
fs::path canonical_root(const char *s){if(!s||!std::memchr(s,0,SLI_PATH_CAPACITY)||!*s)throw std::runtime_error("empty directory root");std::error_code ec;auto p=fs::canonical(fs::u8path(s),ec);if(ec||!fs::is_directory(p))throw std::runtime_error("all six roots must be existing directories");return p;}
int64_t modified_ns(const fs::path &p){std::error_code ec;auto t=fs::last_write_time(p,ec);if(ec)throw std::runtime_error("cannot read modification time");return std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch()).count();}
int64_t system_ns_from_file_ns(int64_t ns){auto ft=fs::file_time_type(std::chrono::nanoseconds(ns));auto st=std::chrono::time_point_cast<std::chrono::nanoseconds>(ft-fs::file_time_type::clock::now()+std::chrono::system_clock::now());return st.time_since_epoch().count();}
std::string timestamp(int64_t ns){auto st=std::chrono::system_clock::time_point(std::chrono::nanoseconds(ns));auto raw=std::chrono::system_clock::to_time_t(st);std::tm tm{};
#ifdef _WIN32
 localtime_s(&tm,&raw);
#else
 localtime_r(&raw,&tm);
#endif
 std::ostringstream out;out<<std::put_time(&tm,"%Y-%m-%d_%H-%M-%S");return out.str();}
class Sha256 {
 static constexpr std::array<uint32_t,64> K={0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
 std::array<uint32_t,8> h{0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
 std::array<uint8_t,64>b{};size_t used=0;uint64_t bits=0;
 static uint32_t r(uint32_t x,unsigned n){return(x>>n)|(x<<(32-n));}
 void transform(){uint32_t w[64];for(int i=0;i<16;i++)w[i]=(uint32_t(b[i*4])<<24)|(uint32_t(b[i*4+1])<<16)|(uint32_t(b[i*4+2])<<8)|b[i*4+3];for(int i=16;i<64;i++){uint32_t a=r(w[i-15],7)^r(w[i-15],18)^(w[i-15]>>3),z=r(w[i-2],17)^r(w[i-2],19)^(w[i-2]>>10);w[i]=w[i-16]+a+w[i-7]+z;}auto v=h;for(int i=0;i<64;i++){uint32_t s1=r(v[4],6)^r(v[4],11)^r(v[4],25),ch=(v[4]&v[5])^(~v[4]&v[6]),t1=v[7]+s1+ch+K[i]+w[i],s0=r(v[0],2)^r(v[0],13)^r(v[0],22),maj=(v[0]&v[1])^(v[0]&v[2])^(v[1]&v[2]),t2=s0+maj;v={t1+t2,v[0],v[1],v[2],v[3]+t1,v[4],v[5],v[6]};}for(int i=0;i<8;i++)h[i]+=v[i];used=0;}
public:
 void update(const char*p,size_t n){bits+=uint64_t(n)*8;while(n){size_t z=std::min(n,64-used);std::memcpy(b.data()+used,p,z);used+=z;p+=z;n-=z;if(used==64)transform();}}
 std::string finish(){uint64_t n=bits;uint8_t one=0x80,zero=0;update(reinterpret_cast<char*>(&one),1);while(used!=56)update(reinterpret_cast<char*>(&zero),1);uint8_t len[8];for(int i=0;i<8;i++)len[7-i]=uint8_t(n>>(i*8));update(reinterpret_cast<char*>(len),8);std::ostringstream o;o<<std::hex<<std::setfill('0');for(auto x:h)o<<std::setw(8)<<x;return o.str();}
};
std::string sha256(const fs::path&p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("cannot open for hashing: "+p.string());Sha256 s;std::array<char,65536>b{};while(f){f.read(b.data(),b.size());auto n=f.gcount();if(n>0)s.update(b.data(),size_t(n));}if(!f.eof())throw std::runtime_error("hash read failed");return s.finish();}
}
struct sli_provider { std::vector<fs::path> roots; std::vector<Plan> plan; bool ready=false; bool digest=false; bool dry_run=true; };
extern "C" {
uint32_t sli_provider_abi_version(void){return SLI_PROVIDER_ABI_VERSION;}
const char *sli_provider_version(void){return "3.0.0";}
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
   try{item.modified=modified_ns(source);item.organize_time=system_ns_from_file_ns(item.modified);const auto metadata=sleela_media::read_capture_time(source.u8string());if(metadata.found){item.organize_time=metadata.unix_time_ns;item.timestamp_source=metadata.source;}if(c->verify_sha256)item.digest=sha256(source);}catch(...){++r->unknown_timestamp;return;}
   p->plan.push_back(std::move(item));
  };
  if(c->recursive){fs::recursive_directory_iterator it(p->roots[0],fs::directory_options::skip_permission_denied),end;for(;it!=end;){std::error_code ec;if(it->is_symlink(ec))it.disable_recursion_pending();else if(!ec)add(*it);it.increment(ec);}}
  else {fs::directory_iterator it(p->roots[0]),end;for(;it!=end;++it)add(*it);}
  r->files_seen=p->plan.size()+r->unknown_timestamp;r->eligible=p->plan.size();
  std::sort(p->plan.begin(),p->plan.end(),[](const Plan&a,const Plan&b){return a.organize_time==b.organize_time?a.src<b.src:a.organize_time<b.organize_time;});
  std::map<std::string,unsigned> sequence;
  for(auto &item:p->plan){
   const auto ext=lower(item.src.extension().string());const auto type=category(item.src);
   fs::path destination=type=="image"?p->roots[1]:(ext==".mov"?p->roots[2]:(ext==".mp4"||ext==".m4v")?p->roots[3]:(ext==".mpeg"||ext==".mpg")?p->roots[4]:p->roots[5]);
   const auto stamp=timestamp(item.organize_time);const auto key=stamp+"|"+ext;std::ostringstream name;name<<stamp<<"_"<<std::setw(3)<<std::setfill('0')<<++sequence[key]<<ext;item.dst=destination/name.str();
   if(fs::exists(item.dst)){item.state="collision-skipped";++r->collisions;}
  }
  *required=p->plan.size();if(out&&capacity<*required){put(r->status,sizeof(r->status),"BUFFER_TOO_SMALL");return -2;}
  if(out)for(size_t i=0;i<p->plan.size();++i){const auto &item=p->plan[i];if(!put(out[i].source,sizeof(out[i].source),item.src.u8string())||!put(out[i].destination,sizeof(out[i].destination),item.dst.u8string())||!put(out[i].state,sizeof(out[i].state),item.state)||!put(out[i].sha256,sizeof(out[i].sha256),item.digest)||!put(out[i].timestamp_source,sizeof(out[i].timestamp_source),item.timestamp_source))throw std::runtime_error("UTF-8 path or digest exceeds ABI buffer");out[i].bytes=item.bytes;out[i].modified_time_ns=item.modified;out[i].organize_time_ns=item.organize_time;}
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
   if(item.digest.empty()||sha256(item.src)!=item.digest)throw std::runtime_error("source content changed since preview");
   if(fs::exists(item.dst))throw std::runtime_error("destination collision appeared after preview");
   temporary=item.dst.parent_path()/(item.dst.filename().string()+".sleela-tmp-"+std::to_string(temp_sequence.fetch_add(1)));
   fs::copy_file(item.src,temporary,fs::copy_options::none);
   if(fs::file_size(temporary)!=item.bytes||sha256(temporary)!=item.digest)throw std::runtime_error("copy byte-count or reviewed SHA-256 verification failed");
   fs::create_hard_link(temporary,item.dst,ec);
   if(ec) throw std::runtime_error("atomic no-overwrite publication failed: "+ec.message());
   fs::remove(temporary,ec);item.state="published-verified";++r->published;
  }catch(const std::exception &){if(!temporary.empty()){std::error_code ignored;fs::remove(temporary,ignored);}item.state="failed";++r->failed;}
 }
 p->ready=false;put(r->status,sizeof(r->status),"Apply finished. Originals were not deleted or modified.");return r->failed?1:0;
}
}
