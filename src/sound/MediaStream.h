#pragma once

#define MAX_AUDIO_FRAME_SIZE 192000
#include <SDL_mutex.h>
#include <SDL.h>
#include "audio.h"

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

/* NOTE: the size must be big enough to compensate the hardware audio buffersize size */
/* TODO: We assume that a decoded and resampled frame fits into this buffer */
#define SAMPLE_ARRAY_SIZE (8 * 65536)

class GUIRenderer;
class GUITexture;

typedef struct MyAvPacketList {
	AVPacket pkt;
	struct MyAvPacketList *next;
	int serial;
} MyAVPacketList;

typedef struct PacketQueue {
	MyAVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	int64_t duration;
	int abort_request;
	int serial;
	SDL_mutex *mutex;
	SDL_cond *cond;
} PacketQueue;

/* Common struct for handling all types of decoded data and allocated render buffers. */
typedef struct Frame {
	AVFrame *frame;
	AVSubtitle sub;
	int serial;
	double pts;           /* presentation timestamp for the frame */
	double duration;      /* estimated duration of the frame */
	int64_t pos;          /* byte position of the frame in the input file */
	int width;
	int height;
	int format;
	AVRational sar;
	int uploaded;
	int flip_v;
} Frame;

typedef struct FrameQueue {
	Frame queue[FRAME_QUEUE_SIZE];
	int rindex;
	int windex;
	int size;
	int max_size;
	int keep_last;
	int rindex_shown;
	SDL_mutex *mutex;
	SDL_cond *cond;
	PacketQueue *pktq;
} FrameQueue;

typedef struct Clock {
	double pts;           /* clock base */
	double pts_drift;     /* clock base minus time at which we updated the clock */
	double last_updated;
	double speed;
	int serial;           /* clock is based on a packet with this serial */
	int paused;
	int *queue_serial;    /* pointer to the current packet queue serial, used for obsolete clock detection */
} Clock;

typedef struct Decoder {
	AVPacket pkt;
	AVPacket pkt_temp;
	PacketQueue *queue;
	AVCodecContext *avctx;
	int pkt_serial;
	int finished;
	int packet_pending;
	SDL_cond *empty_queue_cond;
	int64_t start_pts;
	AVRational start_pts_tb;
	int64_t next_pts;
	AVRational next_pts_tb;
	SDL_Thread *decoder_tid;
} Decoder;

typedef struct AudioParams {
	int freq;
	int channels;
	int64_t channel_layout;
	enum AVSampleFormat fmt;
	int frame_size;
	int bytes_per_sec;
} AudioParams;

class MediaStream
{
	AVStream*		audio_stream_;
	AVStream*		video_stream_;
	AVStream*		subtitle_stream_;
	double          audio_diff_avg_coef;
	int				audio_diff_avg_count;
	double          audio_diff_threshold;
	AVPacket        audio_packet_;
	double			audio_clock;
	PacketQueue     audioQueue_;
	PacketQueue     videoQueue_;
	SwrContext*     pSwrCtx_;
	AVPacket        flush_pkt;
	int             av_sync_type;
	bool			is_ready;
	bool            streamdown;
	int64_t			totalTime_;//But it was in ? (audio)
	int64_t         currentTime_;//But it was in ? (audio)
	int16_t			sample_array[SAMPLE_ARRAY_SIZE];
	int				sample_array_index;
	int				last_i_start; // Helper from sampel drawing
	AudioParams		audio_tgt;
	AudioParams		audio_src;
	uint64_t        audio_callback_time;
	uint8_t*        audio_buffer_;
	uint8_t*        audio_buf1;
	unsigned int	audio_buf1_size;
	unsigned int	audio_buffer_size_;
	unsigned int	audio_buffer_index_;
	int				audio_write_buf_size;
	int				audio_clock_serial;
	int				decoder_reorder_pts;
	int				queue_attachments_req;
	int				infinite_buffer;
	int				realtime;
	int64_t			start_time;
	int64_t			duration;
	const char*		wanted_stream_spec[AVMEDIA_TYPE_NB] = { nullptr };
	FrameQueue		pictq_;
	FrameQueue		subpq_;
	FrameQueue		sampq_;
	double			max_frame_duration;      // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
	PacketQueue     videoq_;
	PacketQueue		audioq_;
	PacketQueue     subtitleq_;
	double			audio_diff_cum; /* used for AV difference average computation */
	SDL_cond*       continue_read_thread;
	Clock			audclk;
	Clock			vidclk;
	Clock			extclk;
	int				abort_request;
	int				video_stream_index;
	int             last_video_stream_index;
	int				audio_stream_index;
	int				last_audio_stream_index;
	int             subtitle_stream_index;
	int             last_subtitle_stream_index;
	int             eof;
	AVFormatContext* formatCtx_;
	AVDictionary*    format_opts;
	AVDictionary*    codec_opts;
	bool             video_disable;
	bool             audio_disable;
	bool             subtitle_disable;
	int				last_video_stream;
	int				last_audio_stream;
	int				last_subtitle_stream;
	char*           audio_codec_name;
	char*           subtitle_codec_name;
	char*           video_codec_name;
	int				fast = 0;
	int				genpts = 0;
	int				lowres = 0;
	ShowMode        show_mode;
	Decoder         auddec;
	Decoder         viddec;
	Decoder         subdec;
	int				audio_hw_buf_size;
	bool			muted;
	RDFTContext*	rdft;
	FFTSample*		rdft_data;
	int				rdft_bits;
	int             xleft;//ui draw begin in pixel
	int             ytop;//ui draw begin in pixel
	GUITexture*     vis_texture; //Draw Area for RDTF data
	int				xpos;// current draw pos for wav on ui in pixel
	bool            paused;//ui stream pause ?
	bool            last_paused;
	int				loop;//loop count stream
	bool            force_refresh;

	bool			seek_req_;//Suchen angefordert
	int				seek_flags;
	int64_t			seek_pos;
	int64_t			seek_rel;
	int				seek_by_bytes_ = -1; // seek by bytes 0=off 1=on -1=auto

	int				step;
	int				read_pause_return;
	double			frame_timer;
	double			frame_last_returned_time;
	double			frame_last_filter_delay;
	SDL_mutex*		wait_mutex;
	double			rdftspeed = 0.02; //Opti
	double			last_vis_time;

	std::string		metaTitle_;
	std::string		metaArtist_;
	std::string		metaAlbum_;

	static int      FrameQueueInit(FrameQueue *f, PacketQueue *pktq, int maxSize, const int keepLast);
	static int		PacketQueueInit(PacketQueue* queue);
	int				stream_component_open(int stream_index);
	int				decoder_start(Decoder *d, int(*fn)(void *), const char *name, void *arg);
	int				decoder_decode_frame(Decoder *d, AVFrame *frame, AVSubtitle *sub) const;
	void            QueueStart(PacketQueue *queue);
	int				queue_put_nullpacket(PacketQueue *q, int stream_index);
	int             QueuePutPrivate(PacketQueue *q, AVPacket *pkt);
	int				QueuePut(PacketQueue* queue, AVPacket* pkt);
	int				audio_decode_frame();
	static int      QueueGet(PacketQueue* queue, AVPacket *pkt, int block, int *serial);
	static void		Queueflush(PacketQueue* queue);
	static void     QueueDestroy(PacketQueue *queue);
	void			update_sample_display(short *samples, int samples_size);
	int				get_master_sync_type() const;
	int				synchronize_audio(int nbSamples);
	double			get_master_clock();
	static double	get_clock(Clock *c);
	void			sync_clock_to_slave(Clock *c, Clock *slave);
	void			stream_close();
	void			stream_component_close(int stream_index);
	static void		decoder_abort(Decoder *d, FrameQueue *fq);
	void			video_image_display(GUIRenderer* renderer, GUITexture* screen);
	void            video_audio_display(GUIRenderer* renderer, GUITexture* screen);
	void			stream_seek(int64_t pos, int64_t rel, int seek_by_bytes);
	void			step_to_next_frame();
	void			StreamTogglePause();
	static std::string		GetMetaTag(AVDictionary* meta, const char* tagname);

	//Thread Main Funtions
	int				AudioThread(void *arg);
	int				VideoThread(void *arg);
	int				SubtitleThread(void *arg);
public:
	MediaStream();
	~MediaStream();

	int Init(const std::string& fileName, const AVSampleFormat& outputformat, const int channels, const int hw_buf_size, const int freq);
	StreamStates FillStreams();
	void GetAudioData(Uint8 *stream, int len);
	bool IsReady() const;
	bool UpdateUi(GUIRenderer* renderer, GUITexture* screen);
	void Pause();
	void Resume();
	void GetPlayTimes(int64_t* totalTime, int64_t* currentTime) const;
	void GetMetaData(AudioStreamInfo* streamInfo) const;
};

