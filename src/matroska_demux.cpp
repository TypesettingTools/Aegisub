// Copyright (c) 2026 Aegisub contributors
// Aegisub-owned Matroska demux adapter. The public ABI intentionally matches
// MatroskaParser.h while EBML parsing and Matroska semantics come from the
// maintained upstream libraries.
#include <cstddef>
#include "MatroskaParser.h"

#include <ebml/EbmlBinary.h>
#include <ebml/EbmlFloat.h>
#include <ebml/EbmlHead.h>
#include <ebml/EbmlMaster.h>
#include <ebml/EbmlStream.h>
#include <ebml/EbmlString.h>
#include <ebml/EbmlUInteger.h>
#include <ebml/EbmlUnicodeString.h>
#include <ebml/IOCallback.h>
#include <matroska/KaxBlock.h>
#include <matroska/KaxBlockData.h>
#include <matroska/KaxCluster.h>
#include <matroska/KaxSegment.h>
#include <matroska/KaxSemantic.h>
#include <matroska/KaxTracks.h>
#include <zlib.h>

#include <algorithm>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace libebml;
using namespace libmatroska;

namespace {
class InputCallback final : public IOCallback {
    InputStream *in;
    uint64_t pos = 0;
public:
    explicit InputCallback(InputStream *input, uint64_t base) : in(input), pos(base) {}
    uint32 read(void *buffer, size_t size) override {
        if (!in || !in->read) throw std::runtime_error("Matroska input has no read callback");
        if (size > INT_MAX) size = INT_MAX;
        int got = in->read(in, pos, buffer, static_cast<int>(size));
        if (got < 0) throw std::runtime_error(in->geterror ? in->geterror(in) : "input error");
        if (static_cast<size_t>(got) > size) throw std::runtime_error("Matroska input returned too much data");
        pos += got; return got;
    }
    void setFilePointer(int64 offset, seek_mode mode) override {
        int64 next = offset;
        if (mode == seek_current) next += pos;
        else if (mode == seek_end) next += in->getfilesize(in);
        if (next < 0) throw std::runtime_error("invalid negative seek");
        pos = static_cast<uint64_t>(next);
    }
    size_t write(void const *, size_t) override { throw std::runtime_error("read-only input"); }
    uint64 getFilePointer() override { return pos; }
    void close() override {}
};

template<class T> T *child(EbmlMaster &m) { return static_cast<T *>(m.FindFirstElt(EBML_INFO(T), false)); }
template<class T> uint64_t uint_value(EbmlMaster &m, uint64_t fallback = 0) {
    if (auto *v = child<T>(m)) return static_cast<uint64_t>(*v);
    return fallback;
}
template<class T> std::string string_value(EbmlMaster &m, std::string fallback = {}) {
    if (auto *v = child<T>(m)) return v->GetValue();
    return fallback;
}
template<class T> std::string unicode_value(EbmlMaster &m) {
    if (auto *v = child<T>(m)) return v->GetValue().GetUTF8();
    return {};
}

struct TrackRecord { TrackInfo info{}; std::string name, codec, language = "und"; std::vector<unsigned char> priv, comp_private; KaxTrackEntry *entry{}; };
struct Frame { unsigned track{}; uint64_t start{}, end{}, pos{}; unsigned size{}, flags{}; };
struct AttachmentRecord { Attachment info{}; std::string name, description, mime; };
}

struct MatroskaFile {
    InputStream *input{}; std::unique_ptr<InputCallback> io; std::unique_ptr<EbmlStream> stream;
    std::unique_ptr<EbmlElement> segment; std::vector<std::unique_ptr<EbmlElement>> top;
    SegmentInfo info{}; std::vector<TrackRecord> tracks; std::vector<Frame> frames;
    std::vector<AttachmentRecord> attachments; std::vector<Attachment> attachment_view;
    std::unordered_map<uint64_t, unsigned> track_by_number; size_t cursor{}; unsigned mask{};
    uint64_t segment_top{}; std::string error;
};

namespace {
void parse_track(MatroskaFile &f, KaxTrackEntry &entry) {
    TrackRecord r; r.entry = &entry;
    r.info.Number = static_cast<unsigned char>(uint_value<KaxTrackNumber>(entry));
    r.info.UID = uint_value<KaxTrackUID>(entry);
    r.info.Type = static_cast<unsigned char>(uint_value<KaxTrackType>(entry));
    r.info.Enabled = uint_value<KaxTrackFlagEnabled>(entry, 1); r.info.Default = uint_value<KaxTrackFlagDefault>(entry, 1);
    r.info.Lacing = uint_value<KaxTrackFlagLacing>(entry, 1); r.info.MinCache = uint_value<KaxTrackMinCache>(entry);
    r.info.MaxCache = uint_value<KaxTrackMaxCache>(entry); r.info.DefaultDuration = uint_value<KaxTrackDefaultDuration>(entry);
    r.info.TimecodeScale = child<KaxTrackTimecodeScale>(entry) ? static_cast<double>(*child<KaxTrackTimecodeScale>(entry)) : 1.0;
    r.name = unicode_value<KaxTrackName>(entry); r.codec = string_value<KaxCodecID>(entry); r.language = string_value<KaxTrackLanguage>(entry, "eng");
    if (auto *p = child<KaxCodecPrivate>(entry)) r.priv.assign(p->GetBuffer(), p->GetBuffer() + p->GetSize());
    if (auto *encodings = child<KaxContentEncodings>(entry)) if (auto *encoding = child<KaxContentEncoding>(*encodings)) {
        if (uint_value<KaxContentEncodingType>(*encoding, 0) != 0) throw std::runtime_error("encrypted tracks are unsupported");
        if (auto *compression = child<KaxContentCompression>(*encoding)) {
            r.info.CompEnabled = 1; r.info.CompMethod = static_cast<unsigned>(uint_value<KaxContentCompAlgo>(*compression));
            if (auto *settings = child<KaxContentCompSettings>(*compression)) r.comp_private.assign(settings->GetBuffer(), settings->GetBuffer()+settings->GetSize());
        }
    }
    f.track_by_number[r.info.Number] = f.tracks.size(); f.tracks.push_back(std::move(r));
}

void parse_master(MatroskaFile &f, EbmlMaster &master, KaxCluster *cluster = nullptr) {
    for (auto *element : master.GetElementList()) {
        if (auto *entry = dynamic_cast<KaxTrackEntry *>(element)) parse_track(f, *entry);
        else if (auto *attached = dynamic_cast<KaxAttached *>(element)) {
            AttachmentRecord a; a.name=unicode_value<KaxFileName>(*attached); a.description=unicode_value<KaxFileDescription>(*attached); a.mime=string_value<KaxMimeType>(*attached);
            a.info.UID=uint_value<KaxFileUID>(*attached); if(auto*d=child<KaxFileData>(*attached)){a.info.Position=d->GetElementPosition()+EBML_ID_LENGTH(static_cast<EbmlId const&>(*d))+d->GetSizeLength();a.info.Length=d->GetSize();} f.attachments.push_back(std::move(a));
        }
        else if (auto *block = dynamic_cast<KaxInternalBlock *>(element)) {
            auto found=f.track_by_number.find(block->TrackNum()); if(found==f.track_by_number.end()) continue;
            unsigned ti=found->second; uint64_t duration=f.tracks[ti].info.DefaultDuration;
            if (auto *group=dynamic_cast<KaxBlockGroup *>(&master)) if(auto*d=child<KaxBlockDuration>(*group)) duration=static_cast<uint64_t>(*d)*f.info.TimecodeScale;
            unsigned count=block->NumberFrames(); if(count && duration && dynamic_cast<KaxBlockGroup *>(&master)) duration/=count;
            int64_t cluster_ticks=cluster?static_cast<int64_t>(uint_value<KaxClusterTimecode>(*cluster)):0;
            int64_t ticks=cluster_ticks+block->GetRelativeTimestamp();
            uint64_t base=ticks<0?0:static_cast<uint64_t>(ticks)*f.info.TimecodeScale;
            bool key=true;if(auto*s=dynamic_cast<KaxSimpleBlock*>(block))key=s->IsKeyframe();else if(auto*g=dynamic_cast<KaxBlockGroup*>(&master))key=child<KaxReferenceBlock>(*g)==nullptr;
            for(unsigned n=0;n<count;++n){Frame fr;fr.track=ti;fr.start=base+n*duration;fr.end=fr.start+duration;fr.pos=block->GetDataPosition(n);fr.size=static_cast<unsigned>(block->GetFrameSize(n));fr.flags=key?FRAME_KF:0;f.frames.push_back(fr);}
        }
        if (auto *nested=dynamic_cast<EbmlMaster *>(element)) parse_master(f,*nested,cluster ? cluster : dynamic_cast<KaxCluster *>(nested));
    }
}

void finalize(MatroskaFile &f) {
    for (auto &r:f.tracks) { r.info.Name=r.name.empty()?nullptr:r.name.data(); r.info.CodecID=r.codec.data(); std::memset(r.info.Language,0,4); std::memcpy(r.info.Language,r.language.data(),std::min<size_t>(3,r.language.size())); r.info.CodecPrivate=r.priv.empty()?nullptr:r.priv.data();r.info.CodecPrivateSize=r.priv.size();r.info.CompMethodPrivate=r.comp_private.empty()?nullptr:r.comp_private.data();r.info.CompMethodPrivateSize=r.comp_private.size(); }
    for(auto&r:f.attachments){r.info.Name=r.name.data();r.info.Description=r.description.empty()?nullptr:r.description.data();r.info.MimeType=r.mime.data();f.attachment_view.push_back(r.info);}
    std::stable_sort(f.frames.begin(),f.frames.end(),[](auto&a,auto&b){return a.start<b.start;});
    if (!f.info.Duration) for (auto const &frame : f.frames) f.info.Duration=std::max(f.info.Duration,frame.end);
}
}

extern "C" {
MatroskaFile *mkv_Open(InputStream *io,char *msg,unsigned n){return mkv_OpenEx(io,0,0,msg,n);}
MatroskaFile *mkv_OpenEx(InputStream *input,uint64_t base,unsigned,char *msg,unsigned n){
    try { if(!input||!input->read||!input->getfilesize)throw std::runtime_error("invalid Matroska input stream");
        auto input_size=input->getfilesize(input);if(input_size<0||base>static_cast<uint64_t>(input_size))throw std::runtime_error("invalid Matroska input size");
        auto f=std::make_unique<MatroskaFile>();f->input=input;f->io=std::make_unique<InputCallback>(input,base);f->stream=std::make_unique<EbmlStream>(*f->io);
        int upper=0;std::unique_ptr<EbmlElement> head(f->stream->FindNextID(EBML_INFO(EbmlHead),std::numeric_limits<uint64_t>::max()));if(!head)throw std::runtime_error("EBML header not found");head->SkipData(*f->stream,EBML_CONTEXT(head.get()));
        std::unique_ptr<EbmlElement> e(f->stream->FindNextID(EBML_INFO(KaxSegment),std::numeric_limits<uint64_t>::max()));if(!e)throw std::runtime_error("Matroska segment not found");f->segment_top=e->GetElementPosition();
        if (std::strcmp(EBML_NAME(e.get()), "Segment")) throw std::runtime_error(std::string("expected Segment, got ")+EBML_NAME(e.get()));
        auto*segment=static_cast<KaxSegment*>(e.get()); f->segment=std::move(e); f->info.TimecodeScale=1000000;
        EbmlElement *next=f->stream->FindNextElement(EBML_CONTEXT(segment),upper,std::numeric_limits<uint64_t>::max(),true);
        while(next&&upper<=0){std::unique_ptr<EbmlElement> current(next);next=nullptr;
            bool subtitle_frames=std::any_of(f->tracks.begin(),f->tracks.end(),[](auto const&t){return t.info.Type==TT_SUB;});
            if (dynamic_cast<KaxCluster*>(current.get()) && !subtitle_frames) break;
            bool wanted=dynamic_cast<KaxInfo*>(current.get())||dynamic_cast<KaxTracks*>(current.get())||(dynamic_cast<KaxCluster*>(current.get())&&subtitle_frames)||dynamic_cast<KaxAttachments*>(current.get());
            if(wanted){auto*master=dynamic_cast<EbmlMaster*>(current.get());master->Read(*f->stream,EBML_CONTEXT(current.get()),upper,next,true,SCOPE_ALL_DATA);
                if(auto*info=dynamic_cast<KaxInfo*>(master)){f->info.TimecodeScale=uint_value<KaxTimecodeScale>(*info,1000000);if(auto*d=child<KaxDuration>(*info))f->info.Duration=static_cast<uint64_t>(static_cast<double>(*d)*f->info.TimecodeScale);}
                parse_master(*f,*master,dynamic_cast<KaxCluster*>(master));f->top.push_back(std::move(current));
            }else current->SkipData(*f->stream,EBML_CONTEXT(segment));
            if(!next)next=f->stream->FindNextElement(EBML_CONTEXT(segment),upper,std::numeric_limits<uint64_t>::max(),true);
        }
        finalize(*f);return f.release();
    } catch(std::exception const&e){if(msg&&n){std::strncpy(msg,e.what(),n-1);msg[n-1]=0;}return nullptr;}
      catch(...){if(msg&&n){std::strncpy(msg,"unknown Matroska parsing error",n-1);msg[n-1]=0;}return nullptr;}}
void mkv_Close(MatroskaFile*f){delete f;} const char*mkv_GetLastError(MatroskaFile*f){return f?f->error.c_str():"invalid Matroska handle";}
SegmentInfo*mkv_GetFileInfo(MatroskaFile*f){return &f->info;} unsigned mkv_GetNumTracks(MatroskaFile*f){return f->tracks.size();} TrackInfo*mkv_GetTrackInfo(MatroskaFile*f,unsigned i){return i<f->tracks.size()?&f->tracks[i].info:nullptr;}
void mkv_GetAttachments(MatroskaFile*f,Attachment**a,unsigned*n){*a=f->attachment_view.empty()?nullptr:f->attachment_view.data();*n=f->attachment_view.size();}
void mkv_GetChapters(MatroskaFile*,Chapter**c,unsigned*n){*c=nullptr;*n=0;} void mkv_GetTags(MatroskaFile*,Tag**t,unsigned*n){*t=nullptr;*n=0;}
uint64_t mkv_GetSegmentTop(MatroskaFile*f){return f->segment_top;} void mkv_SetTrackMask(MatroskaFile*f,unsigned m){f->mask=m;f->cursor=0;}
int mkv_ReadFrame(MatroskaFile*f,unsigned mask,unsigned*track,uint64_t*start,uint64_t*end,uint64_t*pos,unsigned*size,unsigned*flags){while(f->cursor<f->frames.size()){auto&r=f->frames[f->cursor++];if(((mask|f->mask)>>r.track)&1)continue;*track=r.track;*start=r.start;*end=r.end;*pos=r.pos;*size=r.size;*flags=r.flags;return 0;}return -1;}
void mkv_Seek(MatroskaFile*f,uint64_t t,unsigned){f->cursor=std::lower_bound(f->frames.begin(),f->frames.end(),t,[](auto&a,uint64_t b){return a.start<b;})-f->frames.begin();}
void mkv_SkipToKeyframe(MatroskaFile*){} uint64_t mkv_GetLowestQTimecode(MatroskaFile*f){return f->cursor<f->frames.size()?f->frames[f->cursor].start:0;} int mkv_TruncFloat(MKFLOAT v){return static_cast<int>(v);}
}

struct CompressedStream { MatroskaFile*f;unsigned track;uint64_t pos{};unsigned size{};std::vector<unsigned char> data;size_t cursor{};std::string error; };
extern "C" {
CompressedStream*cs_Create(MatroskaFile*f,unsigned t,char*msg,unsigned n){if(!f||t>=f->tracks.size()){if(msg&&n)std::strncpy(msg,"invalid track",n);return nullptr;}return new CompressedStream{f,t,0,0,{},{},{}};}
void cs_Destroy(CompressedStream*c){delete c;} void cs_NextFrame(CompressedStream*c,uint64_t p,unsigned n){c->pos=p;c->size=n;c->data.clear();c->cursor=0;}
int cs_ReadData(CompressedStream*c,char*out,unsigned cap){try{if(c->data.empty()&&c->size){std::vector<unsigned char>raw(c->size);if(c->f->input->read(c->f->input,c->pos,raw.data(),c->size)!=(int)c->size)throw std::runtime_error("short frame read");auto&r=c->f->tracks[c->track];if(!r.info.CompEnabled)c->data=std::move(raw);else if(r.info.CompMethod==COMP_PREPEND){c->data=r.comp_private;c->data.insert(c->data.end(),raw.begin(),raw.end());}else if(r.info.CompMethod==COMP_ZLIB){z_stream z{};if(inflateInit(&z)!=Z_OK)throw std::runtime_error("zlib initialization failed");z.next_in=raw.data();z.avail_in=raw.size();unsigned char buf[4096];int rc;do{z.next_out=buf;z.avail_out=sizeof buf;rc=inflate(&z,Z_NO_FLUSH);c->data.insert(c->data.end(),buf,buf+sizeof(buf)-z.avail_out);}while(rc==Z_OK);inflateEnd(&z);if(rc!=Z_STREAM_END)throw std::runtime_error("invalid zlib frame");}else throw std::runtime_error("unsupported Matroska compression method");}size_t take=std::min<size_t>(cap,c->data.size()-c->cursor);std::memcpy(out,c->data.data()+c->cursor,take);c->cursor+=take;return take;}catch(std::exception const&e){c->error=e.what();return-1;}}
const char*cs_GetLastError(CompressedStream*c){return c->error.c_str();}
}
