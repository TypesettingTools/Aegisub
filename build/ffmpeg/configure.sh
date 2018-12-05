platform='Win32'
suffix='x86'
if [[ $LIB =~ x64 ]]; then
  platform='x64'
  suffix='x64'
fi

./configure $@ \
  --disable-avfilter \
  --disable-avresample \
  --disable-bzlib \
  --disable-d3d11va \
  --disable-dxva2 \
  --disable-decoder=atrac3p,indeo2,indeo3,indeo4,indeo5,twinvq \
  --disable-devices \
  --disable-doc \
  --disable-encoders \
  --disable-ffmpeg \
  --disable-ffplay \
  --disable-ffprobe \
  --disable-filters \
  --disable-hwaccels \
  --disable-muxers \
  --disable-network \
  --disable-postproc \
  --disable-pthreads \
  --disable-shared \
  --enable-gpl \
  --enable-runtime-cpudetect \
  --enable-static \
  --enable-small \
  --enable-x86asm \
  --x86asmexe=yasm \
  --enable-zlib \
  --extra-cflags=-D_SYSCRT \
  --extra-cflags=-I../../include \
  --extra-cflags=-MD \
  --extra-cflags=-wd4005 \
  --extra-cflags=-wd4189 \
  --extra-ldflags=-LIBPATH:../../lib/$platform/Release \
  --toolchain=msvc

mv config.asm ../../build/ffmpeg/config-$suffix.asm
mv config.h ../../build/ffmpeg/config-$suffix.h
mv libavcodec/bsf_list.c ../../build/ffmpeg/libavcodec/bsf_list.c
mv libavcodec/codec_list.c ../../build/ffmpeg/libavcodec/codec_list.c
mv libavcodec/parser_list.c ../../build/ffmpeg/libavcodec/parser_list.c
mv libavdevice/indev_list.c ../../build/ffmpeg/libavdevice/indev_list.c
mv libavdevice/outdev_list.c ../../build/ffmpeg/libavdevice/outdev_list.c
mv libavformat/demuxer_list.c ../../build/ffmpeg/libavformat/demuxer_list.c
mv libavformat/muxer_list.c ../../build/ffmpeg/libavformat/muxer_list.c
mv libavformat/protocol_list.c ../../build/ffmpeg/libavformat/protocol_list.c
mv libavutil/avconfig.h ../../build/ffmpeg/libavutil/avconfig.h
mv libavutil/ffversion.h ../../build/ffmpeg/libavutil/ffversion.h
