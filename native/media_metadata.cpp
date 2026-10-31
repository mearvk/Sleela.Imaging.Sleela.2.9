#include "media_metadata.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

namespace sleela_media {
namespace {
class Reader {
    std::ifstream file_;
    uint64_t size_ = 0;
public:
    explicit Reader(const std::string &path) : file_(path, std::ios::binary) {
        if (!file_) return;
        file_.seekg(0, std::ios::end);
        const auto end = file_.tellg();
        if (end < 0) return;
        size_ = static_cast<uint64_t>(end);
    }
    bool good() const { return file_.is_open() && size_ != 0; }
    uint64_t size() const { return size_; }
    bool read(uint64_t offset, void *out, size_t count) {
        if (!good() || offset > size_ || count > size_ - offset ||
            offset > static_cast<uint64_t>(std::numeric_limits<std::streamoff>::max())) return false;
        file_.clear();
        file_.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
        if (!file_) return false;
        file_.read(static_cast<char *>(out), static_cast<std::streamsize>(count));
        return file_.gcount() == static_cast<std::streamsize>(count);
    }
    bool u16(uint64_t p, bool le, uint16_t &v) {
        uint8_t b[2]; if (!read(p,b,2)) return false;
        v=le?uint16_t(b[0]|(uint16_t(b[1])<<8)):uint16_t((uint16_t(b[0])<<8)|b[1]); return true;
    }
    bool u32(uint64_t p, bool le, uint32_t &v) {
        uint8_t b[4]; if (!read(p,b,4)) return false;
        v=le?(uint32_t(b[0])|(uint32_t(b[1])<<8)|(uint32_t(b[2])<<16)|(uint32_t(b[3])<<24)):
             ((uint32_t(b[0])<<24)|(uint32_t(b[1])<<16)|(uint32_t(b[2])<<8)|uint32_t(b[3])); return true;
    }
    bool u64be(uint64_t p, uint64_t &v) {
        uint8_t b[8]; if (!read(p,b,8)) return false; v=0;
        for (uint8_t x:b) v=(v<<8)|x; return true;
    }
    std::string text(uint64_t p, size_t n) {
        if (!n || n>256) return {};
        std::string s(n,'\\0'); if (!read(p,&s[0],n)) return {};
        while (!s.empty() && (s.back()=='\\0' || s.back()==' ')) s.pop_back();
        return s;
    }
};

bool digits(const std::string &s,size_t p,size_t n,int &v) {
    if(p+n>s.size()) return false; v=0;
    for(size_t i=0;i<n;++i){const unsigned char c=static_cast<unsigned char>(s[p+i]);if(!std::isdigit(c))return false;v=v*10+(c-'0');}
    return true;
}
int64_t make_ns(int year,int month,int day,int hour,int minute,int second,int nanos,
                bool has_zone,int zone_minutes,bool &zone_known) {
    if(year<1900||year>2200||month<1||month>12||day<1||day>31||hour>23||minute>59||second>60) return 0;
    std::tm tm{};tm.tm_year=year-1900;tm.tm_mon=month-1;tm.tm_mday=day;tm.tm_hour=hour;tm.tm_min=minute;tm.tm_sec=std::min(second,59);
    std::time_t value;
    if(has_zone) {
#ifdef _WIN32
        value=_mkgmtime(&tm);
#else
        value=timegm(&tm);
#endif
        value-=static_cast<std::time_t>(zone_minutes)*60;
        zone_known=true;
    } else {
        value=std::mktime(&tm);
        zone_known=false;
    }
    if(value==static_cast<std::time_t>(-1)) return 0;
    const auto seconds=static_cast<int64_t>(value);
    if(seconds>std::numeric_limits<int64_t>::max()/1000000000LL || seconds<std::numeric_limits<int64_t>::min()/1000000000LL) return 0;
    return seconds*1000000000LL+nanos;
}
bool parse_date(std::string s,int64_t &ns,bool &zone_known) {
    if(s.size()<19) return false;
    int y=0,mo=0,d=0,h=0,mi=0,se=0;
    const bool exif=s.size()>=19 && s[4]==':' && s[7]==':';
    if(!digits(s,0,4,y)||!digits(s,5,2,mo)||!digits(s,8,2,d)||
       !digits(s,11,2,h)||!digits(s,14,2,mi)||!digits(s,17,2,se)) return false;
    if(exif) { if(s[10]!=' '||s[13]!=':'||s[16]!=':') return false; }
    else if((s[10]!='T'&&s[10]!=' ')||s[13]!=':'||s[16]!=':') return false;
    size_t p=19;int nanos=0;
    if(p<s.size()&&s[p]=='.'){++p;int scale=100000000;size_t count=0;while(p<s.size()&&std::isdigit(static_cast<unsigned char>(s[p]))&&count<9){nanos+=(s[p++]-'0')*scale;scale/=10;++count;}while(p<s.size()&&std::isdigit(static_cast<unsigned char>(s[p])))++p;}
    bool has_zone=false;int zone=0;
    if(p<s.size()&&s[p]=='Z'){has_zone=true;++p;}
    else if(p<s.size()&&(s[p]=='+'||s[p]=='-')){const int sign=s[p++]=='+'?1:-1;int zh=0,zm=0;if(!digits(s,p,2,zh))return false;p+=2;if(p<s.size()&&s[p]==':')++p;if(!digits(s,p,2,zm))return false;p+=2;if(zh>23||zm>59)return false;zone=sign*(zh*60+zm);has_zone=true;}
    while(p<s.size()&&(s[p]==' '||s[p]=='\\0'))++p;
    if(p!=s.size()) return false;
    ns=make_ns(y,mo,d,h,mi,se,nanos,has_zone,zone,zone_known);
    return ns!=0;
}
std::string tiff_ascii(Reader &r,uint64_t base,uint64_t entry,bool le) {
    uint16_t type=0;uint32_t count=0;
    if(!r.u16(entry+2,le,type)||!r.u32(entry+4,le,count)||type!=2||count==0||count>128)return {};
    uint32_t offset=0;
    if(count<=4) return r.text(entry+8,count);
    if(!r.u32(entry+8,le,offset))return {};
    return r.text(offset,count);
}
bool tiff_date(Reader &r,uint64_t tiff,uint64_t end,CaptureTime &out) {
    uint8_t order[2];uint16_t magic=0;
    if(!r.read(tiff,order,2))return false;
    const bool le=order[0]=='I'&&order[1]=='I';
    if(!le&&!(order[0]=='M'&&order[1]=='M'))return false;
    if(!r.u16(tiff+2,le,magic)||magic!=42)return false;
    uint32_t ifd=0;if(!r.u32(tiff+4,le,ifd))return false;
    std::string fallback,original,offset_original,offset_fallback;uint32_t exif_ifd=0;
    auto scan=[&](uint32_t relative_off,bool exif){
        if(relative_off==0||uint64_t(relative_off)>end-tiff)return;
        const uint64_t off=tiff+relative_off;
        if(off+2>end)return;
        uint16_t count=0;if(!r.u16(off,le,count)||count>1024||off+2+uint64_t(count)*12+4>end)return;
        for(uint16_t i=0;i<count;++i){const uint64_t e=off+2+uint64_t(i)*12;uint16_t tag=0;if(!r.u16(e,le,tag))continue;
            if(tag==0x8769&&!exif)r.u32(e+8,le,exif_ifd);
            if(tag==0x0132&&!exif)fallback=tiff_ascii(r,tiff,e,le);
            if(tag==0x9003&&exif)original=tiff_ascii(r,e,le);
            if(tag==0x9004&&exif&&original.empty())fallback=tiff_ascii(r,e,le);
            if(tag==0x9011&&exif)offset_original=tiff_ascii(r,e,le);
            if(tag==0x9010&&!exif)offset_fallback=tiff_ascii(r,e,le);
        }
    };
    scan(ifd,false);if(exif_ifd)scan(exif_ifd,true);
    std::string date=!original.empty()?original:fallback;if(date.empty())return false;
    std::string zone=!offset_original.empty()?offset_original:offset_fallback;
    if(date.size()==19&&!zone.empty())date+=zone;
    bool known=false;int64_t value=0;if(!parse_date(date,value,known))return false;
    out.found=true;out.unix_time_ns=value;out.source=!original.empty()?"exif-original": "exif-datetime";out.timezone_known=known;return true;
}
bool jpeg_exif(Reader &r,CaptureTime &out) {
    uint8_t soi[2];if(!r.read(0,soi,2)||soi[0]!=0xff||soi[1]!=0xd8)return false;
    uint64_t p=2;
    while(p+4<=r.size()){
        uint8_t marker[2];if(!r.read(p,marker,2)||marker[0]!=0xff)return false;p+=2;
        while(marker[1]==0xff){if(!r.read(p,&marker[1],1))return false;++p;}
        if(marker[1]==0xd9||marker[1]==0xda)return false;
        if(marker[1]==0x01||(marker[1]>=0xd0&&marker[1]<=0xd7))continue;
        uint8_t lb[2];if(!r.read(p,lb,2))return false;const uint16_t len=uint16_t((lb[0]<<8)|lb[1]);if(len<2||p+len>r.size())return false;
        if(marker[1]==0xe1&&len>=8){char sig[6];if(r.read(p+2,sig,6)&&std::string(sig,6)==std::string("Exif\\0\\0",6))return tiff_date(r,p+8,r.size(),out);}
        p+=len;
    }
    return false;
}
uint32_t be32(Reader &r,uint64_t p,bool &ok){uint32_t v=0;ok=r.u32(p,false,v);return v;}
void qt_scan(Reader &r,uint64_t start,uint64_t end,unsigned depth,CaptureTime &best,int &rank) {
    if(depth>10||end>r.size()||start>end)return;
    uint64_t p=start;unsigned boxes=0;
    while(p+8<=end&&boxes++<100000){
        bool ok=false;uint32_t size32=be32(r,p,ok);char typebuf[4];if(!ok||!r.read(p+4,typebuf,4))return;
        const std::string type(typebuf,4);uint64_t size=size32,header=8;
        if(size32==1){if(!r.u64be(p+8,size)||size<16)return;header=16;}
        else if(size32==0)size=end-p;
        if(size<header||size>end-p)return;
        const uint64_t payload=p+header,box_end=p+size;
        if(type=="mvhd"&&rank<50&&payload+4<=box_end){
            uint8_t version=0;if(!r.read(payload,&version,1))return;uint64_t creation=0;
            if(version==0){uint32_t v=0;if(r.u32(payload+4,false,v))creation=v;}
            else if(version==1)r.u64be(payload+4,creation);
            if(creation>2082844800ULL&&creation<6000000000ULL){best.found=true;best.unix_time_ns=static_cast<int64_t>(creation-2082844800ULL)*1000000000LL;best.source="quicktime-mvhd";best.timezone_known=true;rank=50;}
        }
        if(type==std::string(1,static_cast<char>(0xa9))+std::string("day")){
            uint64_t q=payload;
            while(q+8<=box_end){bool child_ok=false;uint32_t cs=be32(r,q,child_ok);char ct[4];if(!child_ok||!r.read(q+4,ct,4)||cs<8||cs>box_end-q)break;
                if(std::string(ct,4)=="data"&&cs>16){std::string date=r.text(q+16,static_cast<size_t>(std::min<uint64_t>(cs-16,128)));int64_t ns=0;bool known=false;if(parse_date(date,ns,known)&&rank<100){best.found=true;best.unix_time_ns=ns;best.source="quicktime-day";best.timezone_known=known;rank=100;}}
                q+=cs;
            }
        }
        const bool container=type=="moov"||type=="trak"||type=="mdia"||type=="minf"||type=="stbl"||type=="udta"||type=="ilst"||type=="meta"||type=="edts"||type=="dinf";
        if(container){uint64_t child=payload;if(type=="meta")child+=4;if(child<=box_end)qt_scan(r,child,box_end,depth+1,best,rank);}
        p=box_end;
    }
}
bool quicktime(Reader &r,CaptureTime &out) {
    CaptureTime best;int rank=0;qt_scan(r,0,r.size(),0,best,rank);if(best.found)out=best;return best.found;
}
}
CaptureTime read_capture_time(const std::string &path) noexcept {
    CaptureTime out;
    try {
        Reader r(path);if(!r.good())return out;
        const auto ext=std::filesystem::path(path).extension().string();
        std::string lower=ext;std::transform(lower.begin(),lower.end(),lower.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
        if(lower==".jpg"||lower==".jpeg"){jpeg_exif(r,out);return out;}
        if(lower==".tif"||lower==".tiff"){tiff_date(r,0,r.size(),out);return out;}
        if(lower==".mov"||lower==".mp4"||lower==".m4v"){quicktime(r,out);return out;}
    } catch(...) {}
    return out;
}
}
