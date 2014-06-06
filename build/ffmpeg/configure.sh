platform='Win32'
suffix='x86'
if [[ $LIB =~ amd64 ]]; then
  platform='x64'
  suffix='x64'
fi

./configure $@ \
  --disable-avfilter \
  --disable-avresample \
  --disable-bzlib \
  --disable-decoder=atrac3p,indeo2,indeo3,indeo4,indeo5,twinvq \
  --disable-devices \
  --disable-doc \
  --disable-encoders \
  --disable-ffmpeg \
  --disable-ffplay \
  --disable-ffprobe \
  --disable-ffserver \
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
  --enable-zlib \
  --extra-cflags=-D_SYSCRT \
  --extra-cflags=-I../../include \
  --extra-cflags=-MD \
  --extra-cflags=-wd4005 \
  --extra-cflags=-wd4189 \
  --extra-ldflags=-LIBPATH:../../lib/$platform/Release \
  --toolchain=msvc

mv config.h ../../build/ffmpeg/config-$suffix.h
mv config.asm ../../build/ffmpeg/config-$suffix.asm
