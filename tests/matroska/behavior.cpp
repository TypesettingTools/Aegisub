#include <cstddef>
#include "MatroskaParser.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

struct FileInput final : InputStream {
    FILE *file{}; std::string error; uint64_t size{};
    explicit FileInput(char const *name) {
        file = std::fopen(name, "rb"); if (!file) return;
        std::fseek(file, 0, SEEK_END); size = std::ftell(file); std::fseek(file, 0, SEEK_SET);
        read=[](InputStream*s,uint64_t p,void*b,int n){auto&i=*static_cast<FileInput*>(s);if(p>=i.size)return 0;n=std::min<uint64_t>(n,i.size-p);if(std::fseek(i.file,p,SEEK_SET))return-1;return(int)std::fread(b,1,n,i.file);};
        scan=[](InputStream*s,uint64_t p,unsigned sig)->int64_t{unsigned v=0;unsigned char c;for(;static_cast<FileInput*>(s)->read(s,p,&c,1)==1;++p){v=(v<<8)|c;if(v==sig)return p-3;}return-1;};
        getcachesize=[](InputStream*){return 1U<<20;}; geterror=[](InputStream*s){return static_cast<FileInput*>(s)->error.c_str();};
        memalloc=[](InputStream*,size_t n){return std::malloc(n);}; memrealloc=[](InputStream*,void*p,size_t n){return std::realloc(p,n);}; memfree=[](InputStream*,void*p){std::free(p);};
        progress=[](InputStream*,uint64_t,uint64_t){return 1;}; getfilesize=[](InputStream*s){return(int64_t)static_cast<FileInput*>(s)->size;};
    }
    ~FileInput(){if(file)std::fclose(file);}
};
static uint64_t hash(FileInput&i,uint64_t p,unsigned n){uint64_t h=1469598103934665603ULL;std::vector<unsigned char>d(n);if(n&&i.read(&i,p,d.data(),n)!=(int)n)return 0;for(auto c:d){h^=c;h*=1099511628211ULL;}return h;}
static uint64_t decoded_hash(MatroskaFile*f,unsigned t,uint64_t p,unsigned n){char error[256]{};auto*c=cs_Create(f,t,error,sizeof error);if(!c)return 0;cs_NextFrame(c,p,n);uint64_t h=1469598103934665603ULL;char data[256];for(;;){int got=cs_ReadData(c,data,sizeof data);if(got<=0)break;for(int i=0;i<got;++i){h^=static_cast<unsigned char>(data[i]);h*=1099511628211ULL;}}cs_Destroy(c);return h;}
int main(int ac,char**av){if(ac!=2)return 2;FileInput in(av[1]);char e[2048]{};auto*f=mkv_Open(&in,e,sizeof e);if(!f){std::cout<<"error\t"<<e<<'\n';return 1;}auto*s=mkv_GetFileInfo(f);std::cout<<"segment\t"<<s->TimecodeScale<<'\t'<<s->Duration<<'\n';for(unsigned i=0;i<mkv_GetNumTracks(f);++i){auto*t=mkv_GetTrackInfo(f,i);std::cout<<"track\t"<<i<<'\t'<<unsigned(t->Number)<<'\t'<<unsigned(t->Type)<<'\t'<<(t->CodecID?t->CodecID:"")<<'\t'<<t->Language<<'\t'<<(t->Name?t->Name:"")<<'\t'<<t->CodecPrivateSize<<'\t'<<t->CompEnabled<<'\t'<<t->CompMethod<<'\n';}Attachment*at;unsigned count;mkv_GetAttachments(f,&at,&count);for(unsigned i=0;i<count;++i)std::cout<<"attachment\t"<<(at[i].Name?at[i].Name:"")<<'\t'<<(at[i].MimeType?at[i].MimeType:"")<<'\t'<<at[i].Length<<'\t'<<std::hex<<hash(in,at[i].Position,at[i].Length)<<std::dec<<'\n';unsigned tr,n,fl;uint64_t a,b,p;while(!mkv_ReadFrame(f,0,&tr,&a,&b,&p,&n,&fl)){std::cout<<"frame\t"<<tr<<'\t'<<a<<'\t'<<b<<'\t'<<n<<'\t'<<fl<<'\t'<<std::hex<<hash(in,p,n);if(mkv_GetTrackInfo(f,tr)->CompEnabled)std::cout<<'\t'<<decoded_hash(f,tr,p,n);std::cout<<std::dec<<'\n';}mkv_Close(f);}
