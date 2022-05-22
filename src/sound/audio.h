#pragma once

#define SDL_AUDIO_BUFFER_SIZE 1024
#define AUDIO_DIFF_AVG_NB 20
#define HW_SAMPE_RATE 44100
#define MAX_AUDIO_FRAME_SIZE 192000
#define AUDIO_DIFF_AVG_NB 20
#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0
#define SAMPLE_CORRECTION_PERCENT_MAX 1

#include <string>

extern "C"
{
#include <libavutil/log.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/avstring.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libavcodec/avfft.h>
#include <libavutil/ffversion.h>

}

#ifdef _MSC_VER
// MSVC isn't C99 compliant...
# ifdef AV_TIME_BASE_Q
#  undef AV_TIME_BASE_Q
# endif
#define AV_TIME_BASE_Q  GetAVTimeBaseQ()

__inline AVRational GetAVTimeBaseQ()
{
	const AVRational av = { 1, AV_TIME_BASE };
	return av;
}
#endif

#ifndef _MSC_VER
#undef av_err2str
#define av_err2str(errnum) \
av_make_error_string((char*)__builtin_alloca(AV_ERROR_MAX_STRING_SIZE), AV_ERROR_MAX_STRING_SIZE, errnum)
#else
#undef av_err2str
#define av_err2str(errnum) \
av_make_error_string((char*)_alloca(AV_ERROR_MAX_STRING_SIZE), AV_ERROR_MAX_STRING_SIZE, errnum)
#endif

enum {
	AV_SYNC_AUDIO_MASTER,
	AV_SYNC_VIDEO_MASTER,
	AV_SYNC_EXTERNAL_CLOCK,
};

enum class StreamStates : unsigned char {
	loading,
	bufferfull,
	allloaded,
	finish,
	error
};

enum ShowMode {
	SHOW_MODE_NONE = -1,
	SHOW_MODE_VIDEO = 0,
	SHOW_MODE_WAVES,
	SHOW_MODE_RDFT,
	SHOW_MODE_NB
};

struct AudioStreamInfo {
	std::string metaTitle_;
	std::string metaArtist_;
	std::string metaAlbum_;
};

