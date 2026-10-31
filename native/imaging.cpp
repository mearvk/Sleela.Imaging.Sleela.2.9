/* SLeeLa Imaging 2.9: portable, non-destructive native filesystem engine. */\n#include "media_metadata.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <stdexcept>
#include <system_error>
#include <vector>
namespace fs=std::filesystem;
namespace {
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
std::string hashFile(const fs::path&p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("cannot open for hashing: "+p.string());Sha256 s;std::array<char,65536>b{};while(f){f.read(b.data(),b.size());auto n=f.gcount();if(n>0)s.update(b.data(),size_t(n));}if(!f.eof())throw std::runtime_error("hash read failed");return s.finish();}
std::string lower(std::string s){for(char&c:s)if(c>='A'&&c<='Z')c=char(c-'A'+'a');return s;}
std::string extOf(const fs::path&p){return lower(p.extension().string());}
std::string category(const std::string&e){static const std::vector<std::string>i={".jpg",".jpeg",".heic",".png",".tiff",".tif",".bmp",".gif",".webp"},v={".mov",".mp4",".mpeg",".mpg",".avi",".m4v"};if(std::find(i.begin(),i.end(),e)!=i.end())return"image";if(std::find(v.begin(),v.end(),e)!=v.end())return"video";return"unsupported";}
std::string stamp_system_ns(int64_t ns){auto st=std::chrono::system_clock::time_point(std::chrono::nanoseconds(ns));auto t=std::chrono::system_clock::to_time_t(st);if(t<=0)return{};std::tm tm{};
#ifdef _WIN32
 localtime_s(&tm,&t);
#else
 localtime_r(&t,&tm);
#endif
 std::ostringstream o;o<<std::put_time(&tm,"%Y-%m-%d_%H-%M-%S");return o.str();}
std::string csv(const std::string&s){std::string o="\"";for(char c:s){if(c=='"')o+='"';o+=c;}return o+'"';}
bool inside(const fs::path&p,const fs::path&r){auto x=p.lexically_relative(r);return !x.empty()&&!x.is_absolute()&&*x.begin()!="..";}
bool nested(const fs::path&a,const fs::path&b){return inside(a,b);}
struct Opt{fs::path src,img,mov,mp4,mpeg,avi,report;bool recursive=false,apply=false,confirm=false;};
void usage(){std::cout<<"SLeeLa Imaging 2.9\nRequired: --source DIR --images DIR --mov DIR --mp4 DIR --mpeg DIR --avi DIR\nOptional: --recursive --apply --confirm --report FILE\nDry-run is the default; source files are never deleted or modified.\n";}
Opt parse(int ac,char**av){Opt o;for(int i=1;i<ac;i++){std::string a=av[i];auto val=[&](){if(i+1>=ac)throw std::runtime_error("missing value: "+a);return fs::path(av[++i]);};if(a=="--source")o.src=val();else if(a=="--images")o.img=val();else if(a=="--mov")o.mov=val();else if(a=="--mp4")o.mp4=val();else if(a=="--mpeg")o.mpeg=val();else if(a=="--avi")o.avi=val();else if(a=="--report")o.report=val();else if(a=="--recursive")o.recursive=true;else if(a=="--apply")o.apply=true;else if(a=="--confirm")o.confirm=true;else if(a=="--help"||a=="-h"){usage();std::exit(0);}else throw std::runtime_error("unknown option: "+a);}return o;}
fs::path canonDir(const fs::path&p,const char*n){std::error_code e;auto c=fs::canonical(p,e);if(e||!fs::is_directory(c))throw std::runtime_error(std::string(n)+" must be an existing directory: "+p.string());return c;}
struct Item{fs::path src,dst;std::string date,ext,date_source,hash,state="planned";uintmax_t bytes=0;};
void run(Opt&o){
 o.src=canonDir(o.src,"source");o.img=canonDir(o.img,"images");o.mov=canonDir(o.mov,"MOV");o.mp4=canonDir(o.mp4,"MP4");o.mpeg=canonDir(o.mpeg,"MPEG");o.avi=canonDir(o.avi,"AVI");
 std::vector<fs::path>roots={o.src,o.img,o.mov,o.mp4,o.mpeg,o.avi};for(size_t i=0;i<roots.size();i++)for(size_t j=i+1;j<roots.size();j++)if(roots[i]==roots[j]||nested(roots[i],roots[j])||nested(roots[j],roots[i]))throw std::runtime_error("source and destination roots must be distinct and non-overlapping");
 if(o.apply&&!o.confirm)throw std::runtime_error("--apply requires the separate --confirm flag");
 if(!o.report.empty()){auto p=o.report.parent_path();if(p.empty())p=fs::current_path();p=fs::canonical(p);for(auto&r:roots)if(p==r||inside(p,r))throw std::runtime_error("report must be outside all source and destination roots");if(fs::exists(o.report))throw std::runtime_error("report exists; refusing overwrite");}
 std::vector<fs::path>found;auto add=[&](const fs::directory_entry&e){std::error_code ec;if(e.is_symlink(ec)||ec||!e.is_regular_file(ec)||ec)return;auto x=extOf(e.path());if(category(x)!="unsupported"){auto c=fs::canonical(e.path(),ec);if(!ec&&inside(c,o.src))found.push_back(c);}};
 if(o.recursive){fs::recursive_directory_iterator i(o.src,fs::directory_options::skip_permission_denied),end;for(;i!=end;){std::error_code ec;if(i->is_symlink(ec))i.disable_recursion_pending();else if(!ec)add(*i);i.increment(ec);}}else for(fs::directory_iterator i(o.src),end;i!=end;++i)add(*i);
 std::sort(found.begin(),found.end());std::vector<Item>items;
 for(auto&p:found){auto e=extOf(p),cat=category(e);fs::path d=cat=="image"?o.img:(e==".mov"?o.mov:(e==".mp4"||e==".m4v")?o.mp4:(e==".mpeg"||e==".mpg")?o.mpeg:o.avi);std::error_code ec;auto ft=fs::last_write_time(p,ec);if(ec)continue;const auto metadata=sleela_media::read_capture_time(p.string());const auto t=metadata.found?stamp_system_ns(metadata.unix_time_ns):stamp_system_ns(std::chrono::time_point_cast<std::chrono::nanoseconds>(ft-fs::file_time_type::clock::now()+std::chrono::system_clock::now()).time_since_epoch().count());if(t.empty())continue;auto z=fs::file_size(p,ec);if(ec)continue;items.push_back({p,d,t,e,metadata.found?metadata.source:"filesystem-mtime","","planned",z});}
 std::sort(items.begin(),items.end(),[](const Item&a,const Item&b){return a.date==b.date?a.src<b.src:a.date<b.date;});std::map<std::string,unsigned>seq;
 for(auto&x:items){unsigned n=++seq[x.date+"|"+x.ext];std::ostringstream s;s<<x.date<<"_"<<std::setw(3)<<std::setfill('0')<<n<<x.ext;x.dst/=s.str();if(fs::exists(x.dst)){x.state="collision-skipped";std::cout<<"SKIP "<<x.dst<<"\n";continue;}if(!o.apply){std::cout<<"PLAN "<<x.src<<" -> "<<x.dst<<"\n";continue;}
  fs::path tmp=x.dst.parent_path()/(x.dst.filename().string()+".sleela-tmp-"+std::to_string(items.size()+n));try{if(fs::exists(tmp))throw std::runtime_error("temporary path exists");fs::copy_file(x.src,tmp,fs::copy_options::none);if(fs::file_size(tmp)!=x.bytes)throw std::runtime_error("byte-count verification failed");auto a=hashFile(x.src),b=hashFile(tmp);if(a!=b)throw std::runtime_error("SHA-256 verification failed");x.hash=b;std::error_code ec;fs::create_hard_link(tmp,x.dst,ec);if(ec)throw std::runtime_error("atomic no-overwrite publish failed: "+ec.message());x.state="published-verified";fs::remove(tmp,ec);if(ec)std::cerr<<"WARN temporary link remains: "<<tmp<<"\n";std::cout<<"DONE "<<x.dst<<" sha256="<<x.hash<<"\n";}catch(const std::exception&e){std::error_code ec;fs::remove(tmp,ec);x.state="failed";std::cerr<<"FAIL "<<x.src<<": "<<e.what()<<"\n";}}
 if(!o.report.empty()){if(fs::exists(o.report))throw std::runtime_error("report exists; refusing overwrite");std::ofstream f(o.report,std::ios::binary);if(!f)throw std::runtime_error("cannot create report");f<<"source,destination,bytes,date_source,status,sha256\n";for(auto&x:items)f<<csv(x.src.string())<<','<<csv(x.dst.string())<<','<<x.bytes<<','<<csv(x.date_source)<<','<<csv(x.state)<<','<<csv(x.hash)<<'\n';if(!f)throw std::runtime_error("report write failed");}
 std::cout<<"Summary: found="<<found.size()<<" eligible="<<items.size()<<" mode="<<(o.apply?"apply":"dry-run")<<"\n";
}
}
int main(int ac,char**av){try{Opt o=parse(ac,av);if(o.src.empty()){usage();return 2;}run(o);return 0;}catch(const std::exception&e){std::cerr<<"SLeeLa Imaging: "<<e.what()<<"\n";return 2;}}
