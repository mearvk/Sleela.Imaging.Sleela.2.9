#include "media_metadata.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs=std::filesystem;
static void put16le(std::vector<uint8_t>&v,size_t p,uint16_t n){v[p]=uint8_t(n);v[p+1]=uint8_t(n>>8);}
static void put32le(std::vector<uint8_t>&v,size_t p,uint32_t n){for(unsigned i=0;i<4;++i)v[p+i]=uint8_t(n>>(i*8));}
static void put32be(std::vector<uint8_t>&v,size_t p,uint32_t n){v[p]=uint8_t(n>>24);v[p+1]=uint8_t(n>>16);v[p+2]=uint8_t(n>>8);v[p+3]=uint8_t(n);}
static std::vector<uint8_t> tiff_exif(){
 std::vector<uint8_t> v(64,0);v[0]='I';v[1]='I';put16le(v,2,42);put32le(v,4,8);
 put16le(v,8,1);put16le(v,10,0x8769);put16le(v,12,4);put32le(v,14,1);put32le(v,18,26);put32le(v,22,0);
 put16le(v,26,1);put16le(v,28,0x9003);put16le(v,30,2);put32le(v,32,20);put32le(v,36,44);put32le(v,40,0);
 const std::string date="2020:05:04 03:02:01";for(size_t i=0;i<date.size();++i)v[44+i]=uint8_t(date[i]);
 return v;
}
static std::vector<uint8_t> atom(const std::string&type,const std::vector<uint8_t>&payload){
 std::vector<uint8_t> out(8+payload.size());put32be(out,0,uint32_t(out.size()));
 for(size_t i=0;i<4;++i)out[4+i]=uint8_t(type[i]);
 std::copy(payload.begin(),payload.end(),out.begin()+8);return out;
}
static void write(const fs::path&p,const std::vector<uint8_t>&v){std::ofstream f(p,std::ios::binary);f.write(reinterpret_cast<const char*>(v.data()),static_cast<std::streamsize>(v.size()));assert(bool(f));}
int main(){
 const auto root=fs::temp_directory_path()/"sleela-media-metadata-fixtures";fs::remove_all(root);fs::create_directories(root);
 const auto tiff=tiff_exif();const auto jpeg_path=root/"capture.jpg";
 std::vector<uint8_t> jpeg={0xff,0xd8,0xff,0xe1};const uint16_t segment=uint16_t(2+6+tiff.size());jpeg.push_back(uint8_t(segment>>8));jpeg.push_back(uint8_t(segment));const char sig[6]={'E','x','i','f',0,0};jpeg.insert(jpeg.end(),sig,sig+6);jpeg.insert(jpeg.end(),tiff.begin(),tiff.end());jpeg.push_back(0xff);jpeg.push_back(0xd9);write(jpeg_path,jpeg);
 auto exif=sleela_media::read_capture_time(jpeg_path.string());assert(exif.found);assert(exif.source=="exif-original");assert(!exif.timezone_known);assert(exif.unix_time_ns!=0);
 const auto tiff_path=root/"capture.tiff";write(tiff_path,tiff);auto direct=sleela_media::read_capture_time(tiff_path.string());assert(direct.found&&direct.source=="exif-original");
 const std::string qt_date="2022-06-07T08:09:10Z";std::vector<uint8_t> data(8,0);data.insert(data.end(),qt_date.begin(),qt_date.end());
 const auto data_atom=atom("data",data);const std::string day_type(1,static_cast<char>(0xa9))+std::string("day");
 const auto day_atom=atom(day_type,data_atom);const auto udta=atom("udta",day_atom);const auto moov=atom("moov",udta);
 const auto mov_path=root/"capture.mov";write(mov_path,moov);auto quicktime=sleela_media::read_capture_time(mov_path.string());
 assert(quicktime.found);assert(quicktime.source=="quicktime-day");assert(quicktime.timezone_known);assert(quicktime.unix_time_ns==1654589350000000000LL);
 const auto bad_path=root/"broken.jpg";write(bad_path,std::vector<uint8_t>{0xff,0xd8,0xff,0xe1,0,4});auto bad=sleela_media::read_capture_time(bad_path.string());assert(!bad.found);
 fs::remove_all(root);std::cout<<"media metadata fixtures passed\\n";
}
