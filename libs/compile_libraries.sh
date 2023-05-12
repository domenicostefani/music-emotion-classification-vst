#!/usr/bin/env bash

ARCHNAME=$(uname | tr [:upper:] [:lower:])-$(uname -i) && echo $ARCHNAME
BUILD_DIR_NAME=build-$ARCHNAME



if [ ! -d "eigen" ]; then
    echo "Eigen does not exist here. Cloning..."
    git clone https://gitlab.com/libeigen/eigen/
    cd eigen
    git checkout 3.3.4
    cd ..
else 
    echo "Eigen exists here."
fi
echo "Building Eigen..."
cd eigen && mkdir -p $BUILD_DIR_NAME && cd $BUILD_DIR_NAME && cmake .. && sudo make -j`nproc` install
cd ../..
echo "Done building Eigen."


if [ ! -d "FFmpeg" ]; then
    echo "FFmpeg does not exist here. Cloning..."
    git clone https://github.com/FFmpeg/FFmpeg
    cd FFmpeg
    git checkout n3.4.12
else
    echo "FFmpeg exists here."
fi

FFMPEG_AUDIO_FLAGS=" --enable-avresample --disable-doc --disable-debug --disable-avdevice --disable-swscale --disable-avfilter --disable-network --disable-indevs --disable-outdevs --disable-muxers --disable-demuxers --disable-encoders --disable-decoders --disable-bsfs --disable-filters --disable-parsers --disable-protocols --disable-hwaccels --enable-protocol=file --enable-protocol=pipe --enable-demuxer=image2 --enable-demuxer=aac --enable-demuxer=ac3 --enable-demuxer=aiff --enable-demuxer=ape --enable-demuxer=asf --enable-demuxer=au --enable-demuxer=avi --enable-demuxer=flac --enable-demuxer=flv --enable-demuxer=matroska --enable-demuxer=mov --enable-demuxer=m4v --enable-demuxer=mp3 --enable-demuxer=mpc --enable-demuxer=mpc8 --enable-demuxer=ogg --enable-demuxer=pcm_alaw --enable-demuxer=pcm_mulaw --enable-demuxer=pcm_f64be --enable-demuxer=pcm_f64le --enable-demuxer=pcm_f32be --enable-demuxer=pcm_f32le --enable-demuxer=pcm_s32be --enable-demuxer=pcm_s32le --enable-demuxer=pcm_s24be --enable-demuxer=pcm_s24le --enable-demuxer=pcm_s16be --enable-demuxer=pcm_s16le --enable-demuxer=pcm_s8 --enable-demuxer=pcm_u32be --enable-demuxer=pcm_u32le --enable-demuxer=pcm_u24be --enable-demuxer=pcm_u24le --enable-demuxer=pcm_u16be --enable-demuxer=pcm_u16le --enable-demuxer=pcm_u8 --enable-demuxer=rm --enable-demuxer=shorten --enable-demuxer=tak --enable-demuxer=tta --enable-demuxer=wav --enable-demuxer=wv --enable-demuxer=xwma  --enable-decoder=aac --enable-decoder=aac_latm --enable-decoder=ac3 --enable-decoder=alac --enable-decoder=als --enable-decoder=ape --enable-decoder=atrac1 --enable-decoder=atrac3 --enable-decoder=eac3 --enable-decoder=flac --enable-decoder=gsm --enable-decoder=gsm_ms --enable-decoder=mp1 --enable-decoder=mp1float --enable-decoder=mp2 --enable-decoder=mp2float --enable-decoder=mp3 --enable-decoder=mp3float --enable-decoder=mp3adu --enable-decoder=mp3adufloat --enable-decoder=mp3on4 --enable-decoder=mp3on4float --enable-decoder=mpc7 --enable-decoder=mpc8 --enable-decoder=ra_144 --enable-decoder=ra_288 --enable-decoder=ralf --enable-decoder=shorten --enable-decoder=tak --enable-decoder=truehd --enable-decoder=tta --enable-decoder=vorbis --enable-decoder=wavpack --enable-decoder=wmalossless --enable-decoder=wmapro --enable-decoder=wmav1 --enable-decoder=wmav2 --enable-decoder=wmavoice --enable-decoder=pcm_alaw --enable-decoder=pcm_bluray --enable-decoder=pcm_dvd --enable-decoder=pcm_f32be --enable-decoder=pcm_f32le --enable-decoder=pcm_f64be --enable-decoder=pcm_f64le --enable-decoder=pcm_lxf --enable-decoder=pcm_mulaw --enable-decoder=pcm_s8 --enable-decoder=pcm_s8_planar --enable-decoder=pcm_s16be --enable-decoder=pcm_s16be_planar --enable-decoder=pcm_s16le --enable-decoder=pcm_s16le_planar --enable-decoder=pcm_s24be --enable-decoder=pcm_s24daud --enable-decoder=pcm_s24le --enable-decoder=pcm_s24le_planar --enable-decoder=pcm_s32be --enable-decoder=pcm_s32le --enable-decoder=pcm_s32le_planar --enable-decoder=pcm_u8 --enable-decoder=pcm_u16be --enable-decoder=pcm_u16le --enable-decoder=pcm_u24be --enable-decoder=pcm_u24le --enable-decoder=pcm_u32be --enable-decoder=pcm_u32le --enable-decoder=pcm_zork  --enable-parser=aac --enable-parser=aac_latm --enable-parser=ac3 --enable-parser=cook --enable-parser=dca --enable-parser=flac --enable-parser=gsm --enable-parser=mlp --enable-parser=mpegaudio --enable-parser=tak --enable-parser=vorbis --enable-parser=vp3 --enable-parser=vp8  --disable-alsa --disable-bzlib --disable-iconv --disable-lzma --disable-libxcb --disable-sdl2 --disable-xlib --disable-zlib --disable-cuda --disable-cuvid --disable-nvenc --disable-vaapi --disable-vdpau --disable-vda --disable-audiotoolbox --disable-libv4l2 --disable-fft "

echo "Building FFmpeg..."
cd FFmpeg
./configure \
        $FFMPEG_AUDIO_FLAGS \ --prefix=/usr/local \ --enable-static --disable-shared
make -j`nproc`
cd ..
echo "Done building FFmpeg..."
mkdir FFmpeg-build-linux-aarch64
cp FFmpeg/lib*/*.a FFmpeg-build-linux-$ARCHNAME #TODO: fix for amd64



if [ ! -d "essentia" ]; then
    git clone https://github.com/MTG/essentia
    git checkout 32376db9b39d8692509ac58036d0b539b7e
fi
cd essentia

BASEPATH=$(realpath $(pwd))
export PKG_CONFIG_PATH="$BASEPATH/eigen/build:$BASEPATH/eigen:$BASEPATH/FFmpeg/libavcodec:$BASEPATH/FFmpeg/libavdevice:$BASEPATH/FFmpeg/libavformat:$BASEPATH/FFmpeg/libswresample:$BASEPATH/FFmpeg/libavutil:$BASEPATH/FFmpeg/libavfilter:$BASEPATH/FFmpeg/libswscale"

python3 waf configure --build-static &&\
python3 waf
mv build $BUILD_DIR_NAME
cd ..

