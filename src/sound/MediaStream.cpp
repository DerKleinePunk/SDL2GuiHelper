/**
 * @file
 * simple media player based on the FFmpeg libraries and the source of FFplay
 */

#ifndef ELPP_DEFAULT_LOGGER
#define ELPP_DEFAULT_LOGGER "MediaStream"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#ifdef NVWAMEMCHECK
#include "../../../common/nvwa/debug_new.h"
#endif

#include "../../common/easylogging/easylogging++.h"
#include "../../common/utils/commonutils.h"
#include "../gui/GUIRenderer.h"
#include "../gui/GUITexture.h"
#include "MediaStream.h"

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 25
#define SDL_AUDIO_MIN_BUFFER_SIZE 512

/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

// https://sourceforge.net/p/karlyriceditor/code/HEAD/tree/src/audioplayerprivate.cpp
// https://github.com/popcornmix/omxplayer Raspberry Hardware decoder (use ins this Player)
// https://github.com/FFmpeg/FFmpeg/blob/master/fftools/ffplay.c
// F:\Mine\OpenSource\ffmpeg\lastcode\ffplay.c

MediaStream::MediaStream()
    : audio_stream_(nullptr), video_stream_(nullptr), subtitle_stream_(nullptr), audio_diff_avg_coef(0),
      audio_diff_avg_count(0), audio_diff_threshold(0), audio_clock(0), pSwrCtx_(nullptr), av_sync_type(0),
      is_ready(false), streamdown(false), totalTime_(0), currentTime_(0), sample_array_index(0), last_i_start(0),
      audio_callback_time(0), audio_buffer_(nullptr), audio_buf1(nullptr), audio_buf1_size(0), audio_buffer_size_(0),
      audio_buffer_index_(0), audio_write_buf_size(0), audio_clock_serial(0), realtime(0), max_frame_duration(0),
      audio_diff_cum(0), continue_read_thread(nullptr), abort_request(0), video_stream_index(-1),
      last_video_stream_index(-1), audio_stream_index(-1), last_audio_stream_index(-1), subtitle_stream_index(-1),
      last_subtitle_stream_index(-1), eof(0), formatCtx_(nullptr), codec_opts(nullptr), subtitle_disable(false),
      last_video_stream(0), last_audio_stream(0), last_subtitle_stream(0), audio_codec_name(nullptr),
      subtitle_codec_name(nullptr), video_codec_name(nullptr), audio_hw_buf_size(0), rdft(nullptr), rdft_data(nullptr),
      vis_texture(nullptr), seek_flags(0), seek_pos(0), seek_rel(0), step(0), read_pause_return(0), frame_timer(0),
      frame_last_returned_time(0), frame_last_filter_delay(0), wait_mutex(nullptr), last_vis_time(0), audioQueue_()
{
    av_init_packet(&audio_packet_);
    av_init_packet(&flush_pkt);
    flush_pkt.data = reinterpret_cast<unsigned char*>(const_cast<char*>("FLUSH"));
    format_opts = nullptr;
    video_disable = true;
    audio_disable = false;
    // show_mode = SHOW_MODE_RDFT;
    show_mode = SHOW_MODE_WAVES;
    decoder_reorder_pts = -1;
    queue_attachments_req = 0;
    infinite_buffer = -1; // don't limit the input buffer size (useful with realtime streams)
    start_time = AV_NOPTS_VALUE;
    duration = AV_NOPTS_VALUE;
    muted = false;
    paused = false;
    last_paused = false;
    force_refresh = false;
    xleft = 0;
    ytop = 0;
    rdft_bits = 0;
    xpos = 0;
    loop = 1;
    seek_req_ = false;
    el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
}


MediaStream::~MediaStream()
{
    stream_close();
}

static void packet_queue_abort(PacketQueue* q)
{
    SDL_LockMutex(q->mutex);

    q->abort_request = 1;

    SDL_CondSignal(q->cond);

    SDL_UnlockMutex(q->mutex);
}

static void frame_queue_signal(FrameQueue* f)
{
    SDL_LockMutex(f->mutex);
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

void MediaStream::decoder_abort(Decoder* d, FrameQueue* fq)
{
    packet_queue_abort(d->queue);
    frame_queue_signal(fq);
    SDL_WaitThread(d->decoder_tid, nullptr);
    d->decoder_tid = nullptr;
    Queueflush(d->queue);
}

static void frame_queue_unref_item(Frame* vp)
{
    av_frame_unref(vp->frame);
    avsubtitle_free(&vp->sub);
}

static void frame_queue_destory(FrameQueue* f)
{
    for(auto i = 0; i < f->max_size; i++) {
        auto vp = &f->queue[i];
        frame_queue_unref_item(vp);
        av_frame_free(&vp->frame);
    }
    SDL_DestroyMutex(f->mutex);
    SDL_DestroyCond(f->cond);
}

void MediaStream::stream_close()
{
    /* XXX: use a special url_shutdown call to abort parse cleanly */
    abort_request = 1;
    // SDL_WaitThread(read_tid, nullptr);

    /* close each stream */
    if(audio_stream_index >= 0) stream_component_close(audio_stream_index);
    if(video_stream_index >= 0) stream_component_close(video_stream_index);
    if(subtitle_stream_index >= 0) stream_component_close(subtitle_stream_index);

    avformat_close_input(&formatCtx_);

    QueueDestroy(&videoq_);
    QueueDestroy(&audioq_);
    QueueDestroy(&subtitleq_);

    /* free all pictures */
    frame_queue_destory(&pictq_);
    frame_queue_destory(&sampq_);
    frame_queue_destory(&subpq_);
    SDL_DestroyCond(continue_read_thread);
    // sws_freeContext(is->img_convert_ctx);
    // sws_freeContext(is->sub_convert_ctx);

    if(vis_texture != nullptr) delete vis_texture;
    /*if (is->vid_texture)
        SDL_DestroyTexture(is->vid_texture);
    if (is->sub_texture)
        SDL_DestroyTexture(is->sub_texture);*/

    if(wait_mutex) SDL_DestroyMutex(wait_mutex);
}

static void decoder_destroy(Decoder* d)
{
    av_packet_unref(&d->pkt);
    avcodec_free_context(&d->avctx);
}

void MediaStream::stream_component_close(const int streamIndex)
{
    if(streamIndex < 0 || streamIndex >= static_cast<int>(formatCtx_->nb_streams)) return;
    const auto codecpar = formatCtx_->streams[streamIndex]->codecpar;

    switch(codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        decoder_abort(&auddec, &sampq_);
        decoder_destroy(&auddec);
        swr_free(&pSwrCtx_);
        av_freep(&audio_buf1);
        audio_buf1_size = 0;
        audio_buf1 = nullptr;

        if(rdft) {
            av_rdft_end(rdft);
            av_freep(&rdft_data);
            rdft = nullptr;
            rdft_bits = 0;
        }
        break;
    case AVMEDIA_TYPE_VIDEO:
        decoder_abort(&viddec, &pictq_);
        decoder_destroy(&viddec);
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        decoder_abort(&subdec, &subpq_);
        decoder_destroy(&subdec);
        break;
    default:
        break;
    }

    formatCtx_->streams[streamIndex]->discard = AVDISCARD_ALL;
    switch(codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        audio_stream_ = nullptr;
        audio_stream_index = -1;
        break;
    case AVMEDIA_TYPE_VIDEO:
        video_stream_ = nullptr;
        video_stream_index = -1;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        subtitle_stream_ = nullptr;
        subtitle_stream_index = -1;
        break;
    default:
        break;
    }
}
static void set_clock_at(Clock* c, double pts, int serial, double time)
{
    c->pts = pts;
    c->last_updated = time;
    c->pts_drift = c->pts - time;
    c->serial = serial;
}

static void set_clock(Clock* c, double pts, int serial)
{
    double time = av_gettime_relative() / 1000000.0;
    set_clock_at(c, pts, serial, time);
}

static void init_clock(Clock* c, int* queue_serial)
{
    c->speed = 1.0;
    c->paused = 0;
    c->queue_serial = queue_serial;
    set_clock(c, NAN, -1);
}

static int decode_interrupt_cb(void* ctx)
{
    // auto is = static_cast<MediaStream*>(ctx);
    // Return 1 stop player
    return 0;
}

void decoder_init(Decoder* d, AVCodecContext* avctx, PacketQueue* queue, SDL_cond* empty_queue_cond)
{
    memset(d, 0, sizeof(Decoder));
    d->avctx = avctx;
    d->queue = queue;
    d->empty_queue_cond = empty_queue_cond;
    d->start_pts = AV_NOPTS_VALUE;
}

void MediaStream::QueueStart(PacketQueue* queue)
{
    SDL_LockMutex(queue->mutex);
    queue->abort_request = 0;
    QueuePutPrivate(queue, &flush_pkt);
    SDL_UnlockMutex(queue->mutex);
}

int MediaStream::decoder_start(Decoder* d, int (*fn)(void*), const char* name, void* arg)
{
    QueueStart(d->queue);
    d->decoder_tid = SDL_CreateThread(fn, name, arg);
    if(!d->decoder_tid) {
        av_log(nullptr, AV_LOG_ERROR, "SDL_CreateThread(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    return 0;
}

int MediaStream::decoder_decode_frame(Decoder* d, AVFrame* frame, AVSubtitle* sub) const
{
    auto ret = AVERROR(EAGAIN);

    for(;;) {
        AVPacket pkt;

        if(d->queue->serial == d->pkt_serial) {
            do {
                if(d->queue->abort_request) return -1;

                switch(d->avctx->codec_type) {
                case AVMEDIA_TYPE_VIDEO:
                    ret = avcodec_receive_frame(d->avctx, frame);
                    if(ret >= 0) {
                        if(decoder_reorder_pts == -1) {
                            frame->pts = frame->best_effort_timestamp;
                        } else if(!decoder_reorder_pts) {
                            frame->pts = frame->pkt_dts;
                        }
                    }
                    break;
                case AVMEDIA_TYPE_AUDIO:
                    ret = avcodec_receive_frame(d->avctx, frame);
                    if(ret >= 0) {
                        AVRational tb; // = (AVRational) { 1, frame->sample_rate };
                        tb.den = 1;
                        tb.num = frame->sample_rate;
                        if(frame->pts != AV_NOPTS_VALUE)
                            frame->pts = av_rescale_q(frame->pts, d->avctx->pkt_timebase, tb);
                        else if(d->next_pts != AV_NOPTS_VALUE)
                            frame->pts = av_rescale_q(d->next_pts, d->next_pts_tb, tb);
                        if(frame->pts != AV_NOPTS_VALUE) {
                            d->next_pts = frame->pts + frame->nb_samples;
                            d->next_pts_tb = tb;
                        }
                    }
                    break;
                case AVMEDIA_TYPE_UNKNOWN:
                    LOG(ERROR) << "AVMEDIA_TYPE_UNKNOWN";
                    break;
                case AVMEDIA_TYPE_DATA:
                    LOG(WARNING) << "AVMEDIA_TYPE_DATA not handled";
                    break;
                case AVMEDIA_TYPE_SUBTITLE:
                    LOG(WARNING) << "AVMEDIA_TYPE_SUBTITLE not handled";
                    break;
                case AVMEDIA_TYPE_NB:
                    LOG(WARNING) << "AVMEDIA_TYPE_NB not handled";
                    break;
                case AVMEDIA_TYPE_ATTACHMENT:
                    LOG(WARNING) << "AVMEDIA_TYPE_ATTACHMENT not handled";
                    break;
                }
                if(ret == AVERROR_EOF) {
                    d->finished = d->pkt_serial;
                    avcodec_flush_buffers(d->avctx);
                    return 0;
                }
                if(ret >= 0) return 1;
            } while(ret != AVERROR(EAGAIN));
        }

        do {
            if(d->queue->nb_packets == 0) SDL_CondSignal(d->empty_queue_cond);
            if(d->packet_pending) {
                av_packet_move_ref(&pkt, &d->pkt);
                d->packet_pending = 0;
            } else {
                if(QueueGet(d->queue, &pkt, 1, &d->pkt_serial) < 0) return -1;
            }
        } while(d->queue->serial != d->pkt_serial);

        if(pkt.data == flush_pkt.data) {
            avcodec_flush_buffers(d->avctx);
            d->finished = 0;
            d->next_pts = d->start_pts;
            d->next_pts_tb = d->start_pts_tb;
        } else {
            if(d->avctx->codec_type == AVMEDIA_TYPE_SUBTITLE) {
                int got_frame = 0;
                ret = avcodec_decode_subtitle2(d->avctx, sub, &got_frame, &pkt);
                if(ret < 0) {
                    ret = AVERROR(EAGAIN);
                } else {
                    if(got_frame && !pkt.data) {
                        d->packet_pending = 1;
                        av_packet_move_ref(&d->pkt, &pkt);
                    }
                    ret = got_frame ? 0 : (pkt.data ? AVERROR(EAGAIN) : AVERROR_EOF);
                }
            } else {
                if(avcodec_send_packet(d->avctx, &pkt) == AVERROR(EAGAIN)) {
                    av_log(d->avctx, AV_LOG_ERROR, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
                    d->packet_pending = 1;
                    av_packet_move_ref(&d->pkt, &pkt);
                }
            }
            av_packet_unref(&pkt);
        }
    }
}

static Frame* frame_queue_peek_writable(FrameQueue* f)
{
    /* wait until we have space to put a new frame */
    SDL_LockMutex(f->mutex);
    while(f->size >= f->max_size && !f->pktq->abort_request) {
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);

    if(f->pktq->abort_request) return nullptr;

    return &f->queue[f->windex];
}

static void frame_queue_push(FrameQueue* f)
{
    if(++f->windex == f->max_size) f->windex = 0;
    SDL_LockMutex(f->mutex);
    f->size++;
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

int MediaStream::AudioThread(void* arg)
{
    AVFrame* frame = av_frame_alloc();
    int got_frame = 0;
    AVRational tb;
    int ret = 0;

    if(!frame) return AVERROR(ENOMEM);

    do {
        if((got_frame = decoder_decode_frame(&auddec, frame, nullptr)) < 0) goto the_end;

        if(got_frame) {
            tb.den = 1; //(AVRational) { 1, frame->sample_rate };
            tb.num = frame->sample_rate;
            const auto af = frame_queue_peek_writable(&sampq_);
            if(af == nullptr) goto the_end;

            currentTime_ = av_rescale_q(frame->pkt_dts, formatCtx_->streams[audio_stream_index]->time_base, AV_TIME_BASE_Q) / 1000;

            af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
            af->pos = frame->pkt_pos; // av_frame_get_pkt_pos(frame);
            af->serial = auddec.pkt_serial;
            AVRational rational;
            rational.num = frame->nb_samples;
            rational.den = frame->sample_rate;
            af->duration = av_q2d(rational);

            av_frame_move_ref(af->frame, frame);
            frame_queue_push(&sampq_);
        }
    } while(ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF);
the_end:
    av_frame_free(&frame);
    LOG(DEBUG) << "audio thread stoppt";
    return ret;
}

int MediaStream::SubtitleThread(void* arg)
{
    Frame* sp;
    int got_subtitle;
    double pts;

    for(;;) {
        if(!((sp = frame_queue_peek_writable(&subpq_)))) return 0;

        if((got_subtitle = decoder_decode_frame(&subdec, nullptr, &sp->sub)) < 0) break;

        pts = 0;

        if(got_subtitle && sp->sub.format == 0) {
            if(sp->sub.pts != AV_NOPTS_VALUE) pts = sp->sub.pts / (double)AV_TIME_BASE;
            sp->pts = pts;
            sp->serial = subdec.pkt_serial;
            sp->width = subdec.avctx->width;
            sp->height = subdec.avctx->height;
            sp->uploaded = 0;

            /* now we can update the picture count */
            frame_queue_push(&subpq_);
        } else if(got_subtitle) {
            avsubtitle_free(&sp->sub);
        }
    }
    return 0;
}

int MediaStream::VideoThread(void* args)
{
    /*AVFrame *frame = av_frame_alloc();
    double pts;
    double duration;
    int ret;
    AVRational tb = video_stream_->time_base;
    AVRational frame_rate = av_guess_frame_rate(formatCtx_, video_stream_, nullptr);

    if (!frame) {
        return AVERROR(ENOMEM);
    }

    for (;;) {
        ret = get_video_frame(is, frame);
        if (ret < 0)
            goto the_end;
        if (!ret)
            continue;

            duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational) { frame_rate.den, frame_rate.num }) : 0);
            pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
            ret = queue_picture(is, frame, pts, duration, frame->pkt_pos, viddec.pkt_serial);
            av_frame_unref(frame);

        if (ret < 0)
            goto the_end;
    }
the_end:

    av_frame_free(&frame);*/
    return 0;
}

/* open a given stream. Return 0 if OK */
int MediaStream::stream_component_open(const int streamIndex)
{
    const char* forcedCodecName = nullptr;
    AVDictionaryEntry* t = nullptr;
    int ret = 0;
    int stream_lowres = lowres;

    if(streamIndex < 0 || streamIndex >= static_cast<int>(formatCtx_->nb_streams)) return -1;
    auto avctxpar = formatCtx_->streams[streamIndex]->codecpar;

    auto avctx = avcodec_alloc_context3(nullptr);
    if(!avctx) return AVERROR(ENOMEM);
    ret = avcodec_parameters_to_context(avctx, avctxpar);
    if(ret < 0) return ret;
    avctx->pkt_timebase = formatCtx_->streams[streamIndex]->time_base;

    auto codec = avcodec_find_decoder(avctx->codec_id);

    switch(avctxpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        last_audio_stream = streamIndex;
        forcedCodecName = audio_codec_name;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        last_subtitle_stream = streamIndex;
        forcedCodecName = subtitle_codec_name;
        break;
    case AVMEDIA_TYPE_VIDEO:
        last_video_stream = streamIndex;
        forcedCodecName = video_codec_name;
        break;
    case AVMEDIA_TYPE_UNKNOWN:
        break;
    case AVMEDIA_TYPE_NB:
        break;
    case AVMEDIA_TYPE_DATA:
        break;
    case AVMEDIA_TYPE_ATTACHMENT:
        break;
    }

    if(forcedCodecName) codec = avcodec_find_decoder_by_name(forcedCodecName);
    if(!codec) {
        if(forcedCodecName)
            av_log(nullptr, AV_LOG_WARNING, "No codec could be found with name '%s'\n", forcedCodecName);
        else
            av_log(nullptr, AV_LOG_WARNING, "No codec could be found with id %d\n", avctxpar->codec_id);
        return -1;
    }

    avctx->codec_id = codec->id;
    if(stream_lowres > codec->max_lowres) {
        av_log(avctx, AV_LOG_WARNING, "The maximum value for lowres supported by the decoder is %d\n", codec->max_lowres);
        stream_lowres = codec->max_lowres;
    }
    avctx->lowres = stream_lowres;

#if FF_API_EMU_EDGE
    if(stream_lowres) avctx->flags |= CODEC_FLAG_EMU_EDGE;
#endif
    if(fast) avctx->flags2 |= AV_CODEC_FLAG2_FAST;
#if FF_API_EMU_EDGE
    if(codec->capabilities & AV_CODEC_CAP_DR1) avctx->flags |= CODEC_FLAG_EMU_EDGE;
#endif

    AVDictionary* opts =
    nullptr; // filter_codec_opts(codec_opts, avctx->codec_id, formatCtx_, formatCtx_->streams[streamIndex], codec);
    if(!av_dict_get(opts, "threads", nullptr, 0)) av_dict_set(&opts, "threads", "auto", 0);
    if(stream_lowres) av_dict_set_int(&opts, "lowres", stream_lowres, 0);
    if(avctxpar->codec_type == AVMEDIA_TYPE_VIDEO || avctxpar->codec_type == AVMEDIA_TYPE_AUDIO)
        av_dict_set(&opts, "refcounted_frames", "1", 0);
    if((ret = avcodec_open2(avctx, codec, &opts)) < 0) {
        goto fail;
    }
    if((t = av_dict_get(opts, "", nullptr, AV_DICT_IGNORE_SUFFIX))) {
        av_log(nullptr, AV_LOG_ERROR, "Option %s not found.\n", t->key);
        ret = AVERROR_OPTION_NOT_FOUND;
        goto fail;
    }

    eof = 0;
    formatCtx_->streams[streamIndex]->discard = AVDISCARD_DEFAULT;
    switch(avctx->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
        /* prepare audio output */
        audio_buffer_size_ = 0;
        audio_buffer_index_ = 0;

        /* init averaging filter */
        audio_diff_avg_coef = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
        audio_diff_avg_count = 0;
        /* since we do not have a precise anough audio fifo fullness,
        we correct audio sync only if larger than this threshold*/
        audio_diff_threshold = static_cast<double>(audio_hw_buf_size) / audio_tgt.bytes_per_sec;

        audio_stream_index = streamIndex;
        audio_stream_ = formatCtx_->streams[streamIndex];

        decoder_init(&auddec, avctx, &audioq_, continue_read_thread);
        if((formatCtx_->iformat->flags & (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) &&
           !formatCtx_->iformat->read_seek) {
            auddec.start_pts = audio_stream_->start_time;
            auddec.start_pts_tb = audio_stream_->time_base;
        }

        {
            utils::Callback<int(void*)>::func = std::bind(&MediaStream::AudioThread, this, std::placeholders::_1);
            int (*audio_thread)(void*) = static_cast<decltype(audio_thread)>(utils::Callback<int(void*)>::callback);

            if((ret = decoder_start(&auddec, audio_thread, "audio", this)) < 0) goto fail;
        }

        break;
    case AVMEDIA_TYPE_VIDEO:
        video_stream_index = streamIndex;
        video_stream_ = formatCtx_->streams[streamIndex];

        decoder_init(&viddec, avctx, &videoq_, continue_read_thread);

        {
            utils::Callback<int(void*)>::func = std::bind(&MediaStream::VideoThread, this, std::placeholders::_1);
            int (*video_thread)(void*) = static_cast<decltype(video_thread)>(utils::Callback<int(void*)>::callback);

            if((ret = decoder_start(&viddec, video_thread, "video", this)) < 0) goto fail;
        }
        queue_attachments_req = 1;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        subtitle_stream_index = streamIndex;
        subtitle_stream_ = formatCtx_->streams[streamIndex];

        decoder_init(&subdec, avctx, &subtitleq_, continue_read_thread);

        {
            utils::Callback<int(void*)>::func = std::bind(&MediaStream::VideoThread, this, std::placeholders::_1);
            int (*subtitle_thread)(void*) = static_cast<decltype(subtitle_thread)>(utils::Callback<int(void*)>::callback);
            if((ret = decoder_start(&subdec, subtitle_thread, "subtitle", this)) < 0) goto fail;
        }
        break;
    default:
        break;
    }

    goto out;

fail:
    avcodec_free_context(&avctx);
out:
    av_dict_free(&opts);
    return ret;
}

static int is_realtime(AVFormatContext* s)
{
    if(!strcmp(s->iformat->name, "rtp") || !strcmp(s->iformat->name, "rtsp") || !strcmp(s->iformat->name, "sdp"))
        return 1;

    /*if (s->pb && (!strncmp(s->url, "rtp:", 4)
        || !strncmp(s->url, "udp:", 4)
        )
        )
        return 1;*/
    return 0;
}

std::string MediaStream::GetMetaTag(AVDictionary* meta, const char* tagname)
{
    const auto ent = av_dict_get(meta, tagname, nullptr, 0);
    if(ent) {
        return std::string(ent->value);
    }
    LOG(DEBUG) << tagname << " not found";
    return std::string("not found");
}

int MediaStream::Init(const std::string& fileName, const AVSampleFormat& outputformat, const int channels, const int hw_buf_size, const int freq)
{
    audio_hw_buf_size = hw_buf_size;
    audio_tgt.channels = channels;
    audio_tgt.fmt = outputformat;
    audio_tgt.freq = freq;
    audio_tgt.channel_layout = av_get_default_channel_layout(channels);
    audio_tgt.bytes_per_sec = av_samples_get_buffer_size(nullptr, audio_tgt.channels, audio_tgt.freq, audio_tgt.fmt, 1);
    audio_tgt.frame_size = av_samples_get_buffer_size(nullptr, audio_tgt.channels, 1, audio_tgt.fmt, 1);

    audio_src = audio_tgt;

    /* start video display */
    if(FrameQueueInit(&pictq_, &videoq_, VIDEO_PICTURE_QUEUE_SIZE, 1) < 0) return -1;
    if(FrameQueueInit(&subpq_, &subtitleq_, SUBPICTURE_QUEUE_SIZE, 0) < 0) return -1;
    if(FrameQueueInit(&sampq_, &audioq_, SAMPLE_QUEUE_SIZE, 1) < 0) return -1;

    if(PacketQueueInit(&videoq_) < 0 || PacketQueueInit(&audioq_) < 0 || PacketQueueInit(&subtitleq_) < 0) return -1;

    if(!((continue_read_thread = SDL_CreateCond()))) {
        av_log(nullptr, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return -1;
    }

    init_clock(&vidclk, &videoq_.serial);
    init_clock(&audclk, &audioq_.serial);
    init_clock(&extclk, &extclk.serial);

    audio_clock_serial = -1;

    av_sync_type = AV_SYNC_AUDIO_MASTER;

    int st_index[AVMEDIA_TYPE_NB];
    memset(st_index, -1, sizeof st_index);

    last_video_stream_index = video_stream_index = -1;
    last_audio_stream_index = audio_stream_index = -1;
    last_subtitle_stream_index = subtitle_stream_index = -1;
    eof = 0;

    formatCtx_ = avformat_alloc_context();

    formatCtx_->interrupt_callback.callback = decode_interrupt_cb;
    formatCtx_->interrupt_callback.opaque = this;

    int scan_all_pmts_set = 0;

    if(!av_dict_get(format_opts, "scan_all_pmts", nullptr, AV_DICT_MATCH_CASE)) {
        av_dict_set(&format_opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
        scan_all_pmts_set = 1;
    }

    if(scan_all_pmts_set) av_dict_set(&format_opts, "scan_all_pmts", nullptr, AV_DICT_MATCH_CASE);

    const auto fileNameAv = av_strdup(fileName.c_str());

    // auto avformat_open_input(&ic, is->filename, is->iformat, &format_opts); set the 3 paramter not null tel the input format
    auto err = avformat_open_input(&formatCtx_, fileNameAv, nullptr, &format_opts);
    if(err < 0) {
        // print_error(is->filename, err);
        // ret = -1;
        av_free(fileNameAv);
        return -1;
    }

    av_format_inject_global_side_data(formatCtx_);
    // opts = setup_find_stream_info_opts(ic, codec_opts);
    // auto orig_nb_streams = formatCtx_->nb_streams;

    err = avformat_find_stream_info(formatCtx_, /*opts*/ nullptr);

    /*for (auto i = 0; i < orig_nb_streams; i++)
        av_dict_free(&opts[i]);
    av_freep(&opts);*/

    if(formatCtx_->pb)
        formatCtx_->pb->eof_reached = 0; // FIXME hack, ffplay maybe should not use avio_feof() to test for the end

    if(seek_by_bytes_ < 0)
        seek_by_bytes_ = !!(formatCtx_->iformat->flags & AVFMT_TS_DISCONT) && strcmp("ogg", formatCtx_->iformat->name);

    max_frame_duration = (formatCtx_->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;
    // if (!window_title && (t = av_dict_get(ic->metadata, "title", NULL, 0)))
    // window_title = av_asprintf("%s - %s", t->value, input_filename);

    auto ret = -1;

    /* if seeking requested, we execute it */
    if(start_time != AV_NOPTS_VALUE) {
        auto timestamp = start_time;
        /* add the stream start time */
        if(formatCtx_->start_time != AV_NOPTS_VALUE) timestamp += formatCtx_->start_time;
        ret = avformat_seek_file(formatCtx_, -1, INT64_MIN, timestamp, INT64_MAX, 0);
        if(ret < 0) {
            av_log(nullptr, AV_LOG_WARNING, "%s: could not seek to position %0.3f\n", fileName.c_str(),
                   static_cast<double>(timestamp) / AV_TIME_BASE);
        }
    }

    realtime = is_realtime(formatCtx_);

#ifdef _DEBUG
    // Dump information about file onto standard error
    av_dump_format(formatCtx_, 0, fileName.c_str(), 0);
#endif

    for(unsigned i = 0; i < formatCtx_->nb_streams; i++) {
        auto st = formatCtx_->streams[i];
        const auto type = st->codecpar->codec_type;
        st->discard = AVDISCARD_ALL;
        if(wanted_stream_spec[type] && st_index[type] == -1)
            if(avformat_match_stream_specifier(formatCtx_, st, wanted_stream_spec[type]) > 0) st_index[type] = i;
    }

    for(auto i = 0; i < AVMEDIA_TYPE_NB; i++) {
        if(wanted_stream_spec[i] && st_index[i] == -1) {
            av_log(nullptr, AV_LOG_ERROR, "Stream specifier %s does not match any %s stream\n", wanted_stream_spec[i],
                   av_get_media_type_string(static_cast<AVMediaType>(i)));
            st_index[i] = INT_MAX;
        }
    }

    if(!video_disable) {
        st_index[AVMEDIA_TYPE_VIDEO] =
        av_find_best_stream(formatCtx_, AVMEDIA_TYPE_VIDEO, st_index[AVMEDIA_TYPE_VIDEO], -1, nullptr, 0);
        if(st_index[AVMEDIA_TYPE_VIDEO] < 0) {
            if(st_index[AVMEDIA_TYPE_VIDEO] == AVERROR_STREAM_NOT_FOUND) {
                av_log(nullptr, AV_LOG_ERROR, "Video Stream not notfound");
            }
            if(st_index[AVMEDIA_TYPE_VIDEO] == AVERROR_DECODER_NOT_FOUND) {
                av_log(nullptr, AV_LOG_ERROR, "Video Decoder not notfound");
            }
        }
    }

    if(!audio_disable) {
        st_index[AVMEDIA_TYPE_AUDIO] = av_find_best_stream(formatCtx_, AVMEDIA_TYPE_AUDIO, st_index[AVMEDIA_TYPE_AUDIO],
                                                           st_index[AVMEDIA_TYPE_VIDEO], nullptr, 0);
        if(st_index[AVMEDIA_TYPE_AUDIO] < 0) {
            if(st_index[AVMEDIA_TYPE_AUDIO] == AVERROR_STREAM_NOT_FOUND) {
                av_log(nullptr, AV_LOG_ERROR, "Audio Stream not notfound");
            }
            if(st_index[AVMEDIA_TYPE_AUDIO] == AVERROR_DECODER_NOT_FOUND) {
                av_log(nullptr, AV_LOG_ERROR, "Audio Decoder not notfound");
            }
        }
    }

    if(!video_disable && !subtitle_disable)
        st_index[AVMEDIA_TYPE_SUBTITLE] =
        av_find_best_stream(formatCtx_, AVMEDIA_TYPE_SUBTITLE, st_index[AVMEDIA_TYPE_SUBTITLE],
                            (st_index[AVMEDIA_TYPE_AUDIO] >= 0 ? st_index[AVMEDIA_TYPE_AUDIO] : st_index[AVMEDIA_TYPE_VIDEO]),
                            nullptr, 0);

    if(st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
        /*AVStream *st = formatCtx_->streams[st_index[AVMEDIA_TYPE_VIDEO]];
        auto *avctx = st->codecpar;
        AVRational sar = av_guess_sample_aspect_ratio(formatCtx_, st, nullptr);
        if (avctx->width)
            //set_default_window_size(avctx->width, avctx->height, sar);*/
    }

    /* open the streams */
    if(st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
        stream_component_open(st_index[AVMEDIA_TYPE_AUDIO]);
    }

    if(st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
        ret = stream_component_open(st_index[AVMEDIA_TYPE_VIDEO]);
    }
    if(show_mode == SHOW_MODE_NONE) show_mode = ret >= 0 ? SHOW_MODE_VIDEO : SHOW_MODE_RDFT;

    if(st_index[AVMEDIA_TYPE_SUBTITLE] >= 0) {
        stream_component_open(st_index[AVMEDIA_TYPE_SUBTITLE]);
    }

    AVDictionary* metadata;

    if(video_stream_index < 0 && audio_stream_index < 0) {
        av_log(nullptr, AV_LOG_FATAL, "Failed to open file '%s' or configure filtergraph\n", fileName.c_str());
        ret = -1;
        goto initfail;
    }

    if(infinite_buffer < 0 && realtime) infinite_buffer = 1;

    av_free(fileNameAv);

    wait_mutex = SDL_CreateMutex();

    metadata = formatCtx_->metadata;
    if(metadata) {
        metaTitle_ = GetMetaTag(metadata, "title");
        metaArtist_ = GetMetaTag(metadata, "artist");
        metaAlbum_ = GetMetaTag(metadata, "album");
    }

    totalTime_ = av_rescale_q(formatCtx_->streams[audio_stream_index]->duration,
                              formatCtx_->streams[audio_stream_index]->time_base, AV_TIME_BASE_Q) /
                 1000;

    LOG(INFO) << "Stream Init OK with Title " << metaTitle_ << " Artist " << metaArtist_ << " Album " << metaAlbum_;

    return 0;

initfail:
    if(formatCtx_) avformat_close_input(&formatCtx_);

    /*if (ret != 0) {
        SDL_Event event;

        event.type = FF_QUIT_EVENT;
        event.user.data1 = is;
        SDL_PushEvent(&event);
    }
    SDL_DestroyMutex(wait_mutex);*/

    return -1;
}

int MediaStream::FrameQueueInit(FrameQueue* f, PacketQueue* pktq, int maxSize, const int keepLast)
{
    memset(f, 0, sizeof(FrameQueue));
    if(!((f->mutex = SDL_CreateMutex()))) {
        av_log(nullptr, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    if(!((f->cond = SDL_CreateCond()))) {
        av_log(nullptr, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    f->pktq = pktq;
    f->max_size = FFMIN(maxSize, FRAME_QUEUE_SIZE);
    f->keep_last = !!keepLast;
    for(auto i = 0; i < f->max_size; i++)
        if(!((f->queue[i].frame = av_frame_alloc()))) return AVERROR(ENOMEM);
    return 0;
}

int MediaStream::PacketQueueInit(PacketQueue* queue)
{
    memset(queue, 0, sizeof(PacketQueue));
    queue->mutex = SDL_CreateMutex();
    if(!queue->mutex) {
        av_log(nullptr, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    queue->cond = SDL_CreateCond();
    if(!queue->cond) {
        av_log(nullptr, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }

    queue->abort_request = 1;
    return 0;
}

void MediaStream::Queueflush(PacketQueue* queue)
{
    MyAVPacketList* pkt1;

    SDL_LockMutex(queue->mutex);
    for(auto pkt = queue->first_pkt; pkt; pkt = pkt1) {
        pkt1 = pkt->next;
        av_packet_unref(&pkt->pkt);
        av_freep(&pkt);
    }
    queue->last_pkt = nullptr;
    queue->first_pkt = nullptr;
    queue->nb_packets = 0;
    queue->size = 0;
    queue->duration = 0;
    SDL_UnlockMutex(queue->mutex);
}

int MediaStream::QueuePutPrivate(PacketQueue* q, AVPacket* pkt)
{
    if(q->abort_request) return -1;

    auto pkt1 = static_cast<MyAVPacketList*>(av_malloc(sizeof(MyAVPacketList)));
    if(!pkt1) return -1;
    pkt1->pkt = *pkt;
    pkt1->next = nullptr;
    if(pkt == &flush_pkt) q->serial++;
    pkt1->serial = q->serial;

    if(!q->last_pkt)
        q->first_pkt = pkt1;
    else
        q->last_pkt->next = pkt1;
    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size + sizeof(*pkt1);
    q->duration += pkt1->pkt.duration;
    /* XXX: should duplicate packet data in DV case */
    SDL_CondSignal(q->cond);
    return 0;
}

int MediaStream::QueuePut(PacketQueue* queue, AVPacket* pkt)
{
    SDL_LockMutex(queue->mutex);
    const auto ret = QueuePutPrivate(queue, pkt);
    SDL_UnlockMutex(queue->mutex);

    if(pkt != &flush_pkt && ret < 0) av_packet_unref(pkt);

    return ret;
}

void MediaStream::QueueDestroy(PacketQueue* queue)
{
    Queueflush(queue);
    SDL_DestroyMutex(queue->mutex);
    SDL_DestroyCond(queue->cond);
}

int MediaStream::queue_put_nullpacket(PacketQueue* q, const int streamIndex)
{
    AVPacket pkt1, *pkt = &pkt1;
    av_init_packet(pkt);
    pkt->data = nullptr;
    pkt->size = 0;
    pkt->stream_index = streamIndex;
    return QueuePut(q, pkt);
}

static int stream_has_enough_packets(AVStream* st, const int streamId, PacketQueue* queue)
{
    return streamId < 0 || queue->abort_request || (st->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
           (queue->nb_packets > MIN_FRAMES && (!queue->duration || av_q2d(st->time_base) * queue->duration > 1.0));
}

static int frame_queue_nb_remaining(FrameQueue* f)
{
    return f->size - f->rindex_shown;
}

/* pause or resume the video */
void MediaStream::StreamTogglePause()
{
    if(paused) {
        frame_timer += av_gettime_relative() / 1000000.0 - vidclk.last_updated;
        if(read_pause_return != AVERROR(ENOSYS)) {
            vidclk.paused = 0;
        }
        set_clock(&vidclk, get_clock(&vidclk), vidclk.serial);
    }
    set_clock(&extclk, get_clock(&extclk), extclk.serial);
    paused = audclk.paused = vidclk.paused = extclk.paused = !paused;
}

void MediaStream::step_to_next_frame()
{
    /* if the stream is paused unpause it, then step */
    if(paused) StreamTogglePause();
    step = 1;
}

/* seek in the stream */
void MediaStream::stream_seek(int64_t pos, int64_t rel, int seek_by_bytes)
{
    if(!seek_req_) {
        seek_pos = pos;
        seek_rel = rel;
        seek_flags &= ~AVSEEK_FLAG_BYTE;
        if(seek_by_bytes) seek_flags |= AVSEEK_FLAG_BYTE;
        seek_req_ = true;
        SDL_CondSignal(continue_read_thread);
    }
}

StreamStates MediaStream::FillStreams()
{
    int ret;
    if(abort_request) return StreamStates::error;

    if(paused != last_paused) {
        last_paused = paused;
        if(paused)
            read_pause_return = av_read_pause(formatCtx_);
        else
            av_read_play(formatCtx_);
    }

#if CONFIG_RTSP_DEMUXER || CONFIG_MMSH_PROTOCOL
    if(is->paused && (!strcmp(ic->iformat->name, "rtsp") || (ic->pb && !strncmp(input_filename, "mmsh:", 5)))) {
        /* wait 10 ms to avoid trying to get another packet */
        /* XXX: horrible */
        SDL_Delay(10);
        continue;
    }
#endif

    if(seek_req_) {
        int64_t seek_target = seek_pos;
        int64_t seek_min = seek_rel > 0 ? seek_target - seek_rel + 2 : INT64_MIN;
        int64_t seek_max = seek_rel < 0 ? seek_target - seek_rel - 2 : INT64_MAX;
        // FIXME the +-2 is due to rounding being not done in the correct direction in generation
        //      of the seek_pos/seek_rel variables

        ret = avformat_seek_file(formatCtx_, -1, seek_min, seek_target, seek_max, seek_flags);
        if(ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "%s: error while seeking\n", "?" /*formatCtx_->url*/);
        } else {
            if(audio_stream_index >= 0) {
                Queueflush(&audioq_);
                QueuePut(&audioq_, &flush_pkt);
            }
            if(subtitle_stream_index >= 0) {
                Queueflush(&subtitleq_);
                QueuePut(&subtitleq_, &flush_pkt);
            }
            if(video_stream_index >= 0) {
                Queueflush(&videoq_);
                QueuePut(&videoq_, &flush_pkt);
            }
            if(seek_flags & AVSEEK_FLAG_BYTE) {
                set_clock(&extclk, NAN, 0);
            } else {
                set_clock(&extclk, seek_target / (double)AV_TIME_BASE, 0);
            }
        }
        seek_req_ = false;
        queue_attachments_req = 1;
        if(paused) step_to_next_frame();
    }

    if(queue_attachments_req) {
        if(video_stream_ && video_stream_->disposition & AV_DISPOSITION_ATTACHED_PIC) {
            AVPacket copy = { 0 };
            if((ret = av_packet_ref(&copy, &video_stream_->attached_pic)) < 0) {
                LOG(ERROR) << av_err2str(ret);
                return StreamStates::error;
            }
            QueuePut(&videoQueue_, &copy);
            queue_put_nullpacket(&videoq_, video_stream_index);
        }
        queue_attachments_req = 0;
    }

    /* if the queue are full, no need to read more */
    if(infinite_buffer < 1 && (audioq_.size + videoq_.size + subtitleq_.size > MAX_QUEUE_SIZE ||
                               (stream_has_enough_packets(audio_stream_, audio_stream_index, &audioq_) &&
                               stream_has_enough_packets(video_stream_, video_stream_index, &videoq_) &&
                               stream_has_enough_packets(subtitle_stream_, subtitle_stream_index, &subtitleq_)))) {
        /* wait 10 ms */
        SDL_LockMutex(wait_mutex);
        SDL_CondWaitTimeout(continue_read_thread, wait_mutex, 10);
        SDL_UnlockMutex(wait_mutex);
        is_ready = true;
        return StreamStates::bufferfull;
    }

    if(!paused && (!audio_stream_ || (auddec.finished == audioq_.serial && frame_queue_nb_remaining(&sampq_) <= 0)) &&
       (!video_stream_ || (viddec.finished == videoq_.serial && frame_queue_nb_remaining(&pictq_) <= 0))) {
        if(loop != 1 && (!loop || --loop)) {
            stream_seek(start_time != AV_NOPTS_VALUE ? start_time : 0, 0, 0);
        } else {
            is_ready = false;
            queue_put_nullpacket(&audioq_, audio_stream_index);
            return StreamStates::finish;
        }
    }

    AVPacket pkt1, *pkt = &pkt1;
    // eof = 0;
    ret = av_read_frame(formatCtx_, pkt);
    if(ret < 0) {
        if((ret == AVERROR_EOF || avio_feof(formatCtx_->pb)) && !eof) {
            if(video_stream_index >= 0) queue_put_nullpacket(&videoq_, video_stream_index);
            if(audio_stream_index >= 0) queue_put_nullpacket(&audioq_, audio_stream_index);
            if(subtitle_stream_index >= 0) queue_put_nullpacket(&subtitleq_, subtitle_stream_index);
            eof = 1;
        }
        if(formatCtx_->pb && formatCtx_->pb->error) return StreamStates::error;
        SDL_LockMutex(wait_mutex);
        SDL_CondWaitTimeout(continue_read_thread, wait_mutex, 10);
        SDL_UnlockMutex(wait_mutex);
        if(eof == 1) {
            return StreamStates::allloaded;
        }
    }


    /* check if packet is in play range specified by user, then queue, otherwise discard */
    auto stream_start_time = formatCtx_->streams[pkt->stream_index]->start_time;
    auto pkt_ts = pkt->pts == AV_NOPTS_VALUE ? pkt->dts : pkt->pts;
    auto pkt_in_play_range =
    duration == AV_NOPTS_VALUE || (pkt_ts - (stream_start_time != AV_NOPTS_VALUE ? stream_start_time : 0)) *
                                  av_q2d(formatCtx_->streams[pkt->stream_index]->time_base) -
                                  (double)(start_time != AV_NOPTS_VALUE ? start_time : 0) / 1000000 <=
                                  ((double)duration / 1000000);
    if(pkt->stream_index == audio_stream_index && pkt_in_play_range) {
        QueuePut(&audioq_, pkt);
    } else if(pkt->stream_index == video_stream_index && pkt_in_play_range &&
              !(video_stream_->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
        QueuePut(&videoq_, pkt);
    } else if(pkt->stream_index == subtitle_stream_index && pkt_in_play_range) {
        QueuePut(&subtitleq_, pkt);
    } else {
        av_packet_unref(pkt);
    }

    return StreamStates::loading;
}

/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
int MediaStream::QueueGet(PacketQueue* queue, AVPacket* pkt, const int block, int* serial)
{
    int ret;

    SDL_LockMutex(queue->mutex);

    for(;;) {
        if(queue->abort_request) {
            ret = -1;
            break;
        }

        auto pkt1 = queue->first_pkt;
        if(pkt1) {
            queue->first_pkt = pkt1->next;
            if(!queue->first_pkt) queue->last_pkt = nullptr;
            queue->nb_packets--;
            queue->size -= pkt1->pkt.size + sizeof(*pkt1);
            queue->duration -= pkt1->pkt.duration;
            *pkt = pkt1->pkt;
            if(serial) *serial = pkt1->serial;
            av_free(pkt1);
            ret = 1;
            break;
        }

        if(!block) {
            ret = 0;
            break;
        }

        if(SDL_CondWaitTimeout(queue->cond, queue->mutex, 1000) != 0) {
            LOG(DEBUG) << "waiting PacketQueue refill timeout";
            ret = -1;
            break;
        }
    }
    SDL_UnlockMutex(queue->mutex);
    return ret;
}

static Frame* frame_queue_peek_readable(FrameQueue* f)
{
    /* wait until we have a readable a new frame */
    SDL_LockMutex(f->mutex);
    while(f->size - f->rindex_shown <= 0 && !f->pktq->abort_request) {
        if(SDL_CondWaitTimeout(f->cond, f->mutex, 5000) != 0) {
            LOG(DEBUG) << "frame queue timeout";
            break;
        }
    }
    SDL_UnlockMutex(f->mutex);

    if(f->pktq->abort_request) return nullptr;

    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

static void frame_queue_next(FrameQueue* f)
{
    if(f->keep_last && !f->rindex_shown) {
        f->rindex_shown = 1;
        return;
    }
    frame_queue_unref_item(&f->queue[f->rindex]);
    if(++f->rindex == f->max_size) f->rindex = 0;
    SDL_LockMutex(f->mutex);
    f->size--;
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

int MediaStream::get_master_sync_type() const
{
    if(av_sync_type == AV_SYNC_VIDEO_MASTER) {
        if(video_stream_)
            return AV_SYNC_VIDEO_MASTER;
        else
            return AV_SYNC_AUDIO_MASTER;
    } else if(av_sync_type == AV_SYNC_AUDIO_MASTER) {
        if(audio_stream_)
            return AV_SYNC_AUDIO_MASTER;
        else
            return AV_SYNC_EXTERNAL_CLOCK;
    } else {
        return AV_SYNC_EXTERNAL_CLOCK;
    }
}

double MediaStream::get_clock(Clock* c)
{
    if(*c->queue_serial != c->serial) return NAN;
    if(c->paused) {
        return c->pts;
    }

    const auto time = av_gettime_relative() / 1000000.0;
    return c->pts_drift + time - (time - c->last_updated) * (1.0 - c->speed);
}

/* get the current master clock value */
double MediaStream::get_master_clock()
{
    double val;

    switch(get_master_sync_type()) {
    case AV_SYNC_VIDEO_MASTER:
        val = get_clock(&vidclk);
        break;
    case AV_SYNC_AUDIO_MASTER:
        val = get_clock(&audclk);
        break;
    default:
        val = get_clock(&extclk);
        break;
    }
    return val;
}

/* return the wanted number of samples to get better sync if sync_type is video
 * or external master clock */
int MediaStream::synchronize_audio(const int nbSamples)
{
    int wanted_nb_samples = nbSamples;

    /* if not master, then we try to remove or add samples to correct the clock */
    if(get_master_sync_type() != AV_SYNC_AUDIO_MASTER) {
        double diff, avg_diff;
        int min_nb_samples, max_nb_samples;

        diff = get_clock(&audclk) - get_master_clock();

        if(!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD) {
            audio_diff_cum = diff + audio_diff_avg_coef * audio_diff_cum;
            if(audio_diff_avg_count < AUDIO_DIFF_AVG_NB) {
                /* not enough measures to have a correct estimate*/
                audio_diff_avg_count++;
            } else {
                /* estimate the A-V difference*/
                avg_diff = audio_diff_cum * (1.0 - audio_diff_avg_coef);

                if(fabs(avg_diff) >= audio_diff_threshold) {
                    wanted_nb_samples = nbSamples + (int)(diff * audio_src.freq);
                    min_nb_samples = ((nbSamples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    max_nb_samples = ((nbSamples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
                    wanted_nb_samples = av_clip(wanted_nb_samples, min_nb_samples, max_nb_samples);
                }
                av_log(nullptr, AV_LOG_TRACE, "diff=%f adiff=%f sample_diff=%d apts=%0.3f %f\n", diff, avg_diff,
                       wanted_nb_samples - nbSamples, audio_clock, audio_diff_threshold);
            }
        } else {
            /* too big difference : may be initial PTS errors, so
            reset A-V filter*/
            audio_diff_avg_count = 0;
            audio_diff_cum = 0;
        }
    }

    return wanted_nb_samples;
}

/**
 * Decode one audio frame and return its uncompressed size.
 *
 * The processed audio frame is decoded, converted if required, and
 * stored in is->audio_buf, with size in bytes given by the return
 * value.
 */
int MediaStream::audio_decode_frame()
{
    int resampledDataSize;
    Frame* af;

    if(paused || !is_ready) {
        LOG(DEBUG) << "stream pause or not ready";
        return -1;
    }

    if(eof == 1) {
        if(frame_queue_nb_remaining(&sampq_) == 0) {
            LOG(DEBUG) << "eof and queue is empty";
            return -1;
        }
    }

    do {
#if defined(_WIN32)
        while(frame_queue_nb_remaining(&sampq_) == 0) {
            if(av_gettime_relative() - audio_callback_time > 1000000LL * audio_hw_buf_size / audio_tgt.bytes_per_sec / 2) {
                LOG(DEBUG) << "WIN32 spezial exit";
                return -1;
            }
            av_usleep(1000);
        }
#endif
        if(!((af = frame_queue_peek_readable(&sampq_)))) return -1;
        frame_queue_next(&sampq_);
    } while(af->serial != audioq_.serial);

    const auto data_size = av_samples_get_buffer_size(nullptr, af->frame->channels, af->frame->nb_samples,
                                                      static_cast<AVSampleFormat>(af->frame->format), 1);

    const int64_t dec_channel_layout =
    (af->frame->channel_layout && af->frame->channels == av_get_channel_layout_nb_channels(af->frame->channel_layout)) ?
    af->frame->channel_layout :
    av_get_default_channel_layout(af->frame->channels);
    const auto wanted_nb_samples = synchronize_audio(af->frame->nb_samples);


    if(af->frame->format != audio_src.fmt || dec_channel_layout != audio_src.channel_layout ||
       af->frame->sample_rate != audio_src.freq || (wanted_nb_samples != af->frame->nb_samples && !pSwrCtx_)) {
        swr_free(&pSwrCtx_);
        pSwrCtx_ = swr_alloc_set_opts(nullptr, audio_tgt.channel_layout, audio_tgt.fmt, audio_tgt.freq, dec_channel_layout,
                                      static_cast<AVSampleFormat>(af->frame->format), af->frame->sample_rate, 0, nullptr);
        if(!pSwrCtx_ || swr_init(pSwrCtx_) < 0) {
            av_log(nullptr, AV_LOG_ERROR, "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                   af->frame->sample_rate, av_get_sample_fmt_name(static_cast<AVSampleFormat>(af->frame->format)),
                   af->frame->channels, audio_tgt.freq, av_get_sample_fmt_name(audio_tgt.fmt), audio_tgt.channels);
            swr_free(&pSwrCtx_);
            return -1;
        }
        audio_src.channel_layout = dec_channel_layout;
        audio_src.channels = af->frame->channels;
        audio_src.freq = af->frame->sample_rate;
        audio_src.fmt = static_cast<AVSampleFormat>(af->frame->format);
    }

    if(pSwrCtx_) {
        const uint8_t** in = const_cast<const uint8_t**>(af->frame->extended_data);
        uint8_t** out = &audio_buf1;
        int out_count = (int64_t)wanted_nb_samples * audio_tgt.freq / af->frame->sample_rate + 256;
        int out_size = av_samples_get_buffer_size(nullptr, audio_tgt.channels, out_count, audio_tgt.fmt, 0);
        int len2;
        if(out_size < 0) {
            av_log(nullptr, AV_LOG_ERROR, "av_samples_get_buffer_size() failed\n");
            return -1;
        }
        if(wanted_nb_samples != af->frame->nb_samples) {
            if(swr_set_compensation(pSwrCtx_, (wanted_nb_samples - af->frame->nb_samples) * audio_tgt.freq / af->frame->sample_rate,
                                    wanted_nb_samples * audio_tgt.freq / af->frame->sample_rate) < 0) {
                av_log(nullptr, AV_LOG_ERROR, "swr_set_compensation() failed\n");
                return -1;
            }
        }
        av_fast_malloc(&audio_buf1, &audio_buf1_size, out_size);
        if(!audio_buf1) return AVERROR(ENOMEM);
        len2 = swr_convert(pSwrCtx_, out, out_count, in, af->frame->nb_samples);
        if(len2 < 0) {
            av_log(nullptr, AV_LOG_ERROR, "swr_convert() failed\n");
            return -1;
        }
        if(len2 == out_count) {
            av_log(nullptr, AV_LOG_WARNING, "audio buffer is probably too small\n");
            if(swr_init(pSwrCtx_) < 0) swr_free(&pSwrCtx_);
        }
        audio_buffer_ = audio_buf1;
        resampledDataSize = len2 * audio_tgt.channels * av_get_bytes_per_sample(audio_tgt.fmt);
    } else {
        audio_buffer_ = af->frame->data[0];
        resampledDataSize = data_size;
    }

    /* const double audio_clock0 = audio_clock;*/
    /* update the audio clock with the pts */
    if(!isnan(af->pts))
        audio_clock = af->pts + (double)af->frame->nb_samples / af->frame->sample_rate;
    else
        audio_clock = NAN;
    audio_clock_serial = af->serial;

#ifdef DEBUG
    {
        static double last_clock;
        printf("audio: delay=%0.3f clock=%0.3f clock0=%0.3f\n", audio_clock - last_clock, audio_clock, audio_clock0);
        last_clock = audio_clock;
    }
#endif
    return resampledDataSize;
}

/* copy samples for viewing in editor window */
void MediaStream::update_sample_display(short* samples, int samples_size)
{
    int size = samples_size / sizeof(short);
    while(size > 0) {
        int len = SAMPLE_ARRAY_SIZE - sample_array_index;
        if(len > size) len = size;
        memcpy(sample_array + sample_array_index, samples, len * sizeof(short));
        samples += len;
        sample_array_index += len;
        if(sample_array_index >= SAMPLE_ARRAY_SIZE) sample_array_index = 0;
        size -= len;
    }
}

void MediaStream::sync_clock_to_slave(Clock* c, Clock* slave)
{
    double clock = get_clock(c);
    double slave_clock = get_clock(slave);
    if(!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
        set_clock(c, slave_clock, slave->serial);
}

void MediaStream::GetAudioData(Uint8* stream, int len)
{
    if(stream == nullptr) {
        LOG(ERROR) << "stream is an null pointer";
		return;
    }

    audio_callback_time = av_gettime_relative();

    while(len > 0) {
        if(audio_buffer_index_ >= audio_buffer_size_) {
            const auto audioSize = audio_decode_frame();
            if(audioSize < 0) {
                /* if error, just output silence */
                audio_buffer_ = nullptr;
                audio_buffer_size_ = SDL_AUDIO_MIN_BUFFER_SIZE / audio_tgt.frame_size * audio_tgt.frame_size;
            } else {
                if(show_mode != SHOW_MODE_VIDEO)
                    update_sample_display(reinterpret_cast<int16_t*>(audio_buffer_), audioSize);
                audio_buffer_size_ = audioSize;
            }
            audio_buffer_index_ = 0;
        }
        int len1 = audio_buffer_size_ - audio_buffer_index_;
        if(len1 > len) len1 = len;
        // Laustärke im AudioManager
        if(!muted && audio_buffer_) {
            memcpy(stream, static_cast<uint8_t*>(audio_buffer_) + audio_buffer_index_, len1);

        } else {
            memset(stream, 0, len1);
        }
        len -= len1;
        stream += len1;
        audio_buffer_index_ += len1;
    }

    audio_write_buf_size = audio_buffer_size_ - audio_buffer_index_;

    /* Let's assume the audio driver that is used by SDL has two periods. */
    if(!isnan(audio_clock)) {
        set_clock_at(&audclk, audio_clock - (double)(2 * audio_hw_buf_size + audio_write_buf_size) / audio_tgt.bytes_per_sec,
                     audio_clock_serial, audio_callback_time / 1000000.0);
        sync_clock_to_slave(&extclk, &audclk);
    }
}

bool MediaStream::IsReady() const
{
    return is_ready;
}

void MediaStream::video_image_display(GUIRenderer* renderer, GUITexture* screen)
{
    /*
    Frame *vp;
    Frame *sp = nullptr;
    SDL_Rect rect;

    vp = frame_queue_peek_last(&pictq_);
    if (subtitle_stream_) {
        if (frame_queue_nb_remaining(&subpq_) > 0) {
            sp = frame_queue_peek(&subpq_);

            if (vp->pts >= sp->pts + ((float)sp->sub.start_display_time / 1000)) {
                if (!sp->uploaded) {
                    uint8_t* pixels[4];
                    int pitch[4];
                    int i;
                    if (!sp->width || !sp->height) {
                        sp->width = vp->width;
                        sp->height = vp->height;
                    }
                    if (realloc_texture(&sub_texture, SDL_PIXELFORMAT_ARGB8888, sp->width, sp->height,
SDL_BLENDMODE_BLEND, 1) < 0) return;

                    for (i = 0; i < sp->sub.num_rects; i++) {
                        AVSubtitleRect *sub_rect = sp->sub.rects[i];

                        sub_rect->x = av_clip(sub_rect->x, 0, sp->width);
                        sub_rect->y = av_clip(sub_rect->y, 0, sp->height);
                        sub_rect->w = av_clip(sub_rect->w, 0, sp->width - sub_rect->x);
                        sub_rect->h = av_clip(sub_rect->h, 0, sp->height - sub_rect->y);

                        sub_convert_ctx = sws_getCachedContext(sub_convert_ctx,
                            sub_rect->w, sub_rect->h, AV_PIX_FMT_PAL8,
                            sub_rect->w, sub_rect->h, AV_PIX_FMT_BGRA,
                            0, NULL, NULL, NULL);
                        if (!sub_convert_ctx) {
                            av_log(nullptr, AV_LOG_FATAL, "Cannot initialize the conversion context\n");
                            return;
                        }
                        if (!SDL_LockTexture(is->sub_texture, (SDL_Rect *)sub_rect, (void **)pixels, pitch)) {
                            sws_scale(is->sub_convert_ctx, (const uint8_t * const *)sub_rect->data, sub_rect->linesize,
                                0, sub_rect->h, pixels, pitch);
                            SDL_UnlockTexture(is->sub_texture);
                        }
                    }
                    sp->uploaded = 1;
                }
            }
            else
                sp = nullptr;
        }
    }

    calculate_display_rect(&rect, is->xleft, is->ytop, is->width, is->height, vp->width, vp->height, vp->sar);

    if (!vp->uploaded) {
        if (upload_texture(&is->vid_texture, vp->frame, &is->img_convert_ctx) < 0)
            return;
        vp->uploaded = 1;
        vp->flip_v = vp->frame->linesize[0] < 0;
    }

    SDL_RenderCopyEx(renderer, is->vid_texture, NULL, &rect, 0, NULL, vp->flip_v ? SDL_FLIP_VERTICAL : 0);
    if (sp) {
#if USE_ONEPASS_SUBTITLE_RENDER
        SDL_RenderCopy(renderer, is->sub_texture, NULL, &rect);
#else
        int i;
        double xratio = (double)rect.w / (double)sp->width;
        double yratio = (double)rect.h / (double)sp->height;
        for (i = 0; i < sp->sub.num_rects; i++) {
            SDL_Rect *sub_rect = (SDL_Rect*)sp->sub.rects[i];
            SDL_Rect target = { .x = rect.x + sub_rect->x * xratio,
                .y = rect.y + sub_rect->y * yratio,
                .w = sub_rect->w * xratio,
                .h = sub_rect->h * yratio };
            SDL_RenderCopy(renderer, is->sub_texture, sub_rect, &target);
        }
#endif
    }
    */
}

static int compute_mod(const int a, const int b)
{
    return a < 0 ? a % b + b : a % b;
}

static int realloc_texture(GUIRenderer* renderer,
                           GUITexture** texture,
                           const Uint32 newFormat,
                           const int new_width,
                           const int new_height,
                           const SDL_BlendMode blendmode,
                           const bool init_texture)
{
    // Todo make it better this code is trible
    Uint32 format;
    int access, w, h;
    if(*texture == nullptr || SDL_QueryTexture((*texture)->operator SDL_Texture*(), &format, &access, &w, &h) < 0 ||
       new_width != w || new_height != h || newFormat != format) {
        void* pixels;
        int pitch;

        if(*texture != nullptr) SDL_DestroyTexture((*texture)->operator SDL_Texture*());

        if(!((*texture = renderer->CreateTexture(newFormat, SDL_TEXTUREACCESS_STREAMING, GUISize(new_width, new_height)))))
            return -1;
        (*texture)->SetBlendMode(blendmode);
        if(init_texture) {
            if(SDL_LockTexture((*texture)->operator SDL_Texture*(), nullptr, &pixels, &pitch) < 0) {
                LOG(ERROR) << "LockTexture failed";
                return -1;
            }
            memset(pixels, 0x00, pitch * new_height);
            SDL_UnlockTexture((*texture)->operator SDL_Texture*());
        }
        av_log(nullptr, AV_LOG_VERBOSE, "Created %dx%d texture with %s.\n", new_width, new_height, SDL_GetPixelFormatName(newFormat));
    }
    return 0;
}

void MediaStream::video_audio_display(GUIRenderer* renderer, GUITexture* screen)
{
    // This Code Only works with 16 Bit Audio Data at audio_tgt
    int i, i_start, x, y1, y, ys, delay, n, nb_display_channels;
    int ch, h = 0, h2;
    int cur_rdft_bits, nb_freq;
    const auto size = screen->Size();
    for(cur_rdft_bits = 1; (1 << cur_rdft_bits) < 2 * size.width; cur_rdft_bits++)
        ;
    nb_freq = 1 << (cur_rdft_bits - 1);

    /* compute display index : center on currently output samples */
    int channels = audio_tgt.channels;
    nb_display_channels = channels;
    if(!paused) {
        int data_used = show_mode == SHOW_MODE_WAVES ? size.width : (2 * nb_freq);
        n = 2 * channels;
        delay = audio_write_buf_size;
        delay /= n;

        /* to be more precise, we take into account the time spent since
        the last buffer computation */
        if(audio_callback_time) {
            const int64_t time_diff = av_gettime_relative() - audio_callback_time;
            delay -= time_diff * audio_tgt.freq / 1000000;
        }

        delay += 2 * data_used;
        if(delay < data_used) delay = data_used;

        i_start = x = compute_mod(sample_array_index - delay * channels, SAMPLE_ARRAY_SIZE);
        if(show_mode == SHOW_MODE_WAVES) {
            h = INT_MIN;
            for(i = 0; i < 1000; i += channels) {
                int idx = (SAMPLE_ARRAY_SIZE + x - i) % SAMPLE_ARRAY_SIZE;
                int a = sample_array[idx];
                int b = sample_array[(idx + 4 * channels) % SAMPLE_ARRAY_SIZE];
                int c = sample_array[(idx + 5 * channels) % SAMPLE_ARRAY_SIZE];
                int d = sample_array[(idx + 9 * channels) % SAMPLE_ARRAY_SIZE];
                int score = a - d;
                if(h < score && (b ^ c) < 0) {
                    h = score;
                    i_start = idx;
                }
            }
        }

        last_i_start = i_start;
    } else {
        i_start = last_i_start;
    }

    if(show_mode == SHOW_MODE_WAVES) {
        /* total height for one channel */
        h = size.height / nb_display_channels;
        /* graph height / 2 */
        h2 = (h * 9) / 20;
        for(ch = 0; ch < nb_display_channels; ch++) {
            if(ch & 1) {
                renderer->DrawColor(0x00, 0xBC, 0xA9, 255);
            } else {
                renderer->DrawColor(0x00, 0x84, 0x00, 255);
            }
            i = i_start + ch;
            y1 = ytop + ch * h + (h / 2); /* position of center line */
            for(x = 0; x < size.width; x++) {
                y = (sample_array[i] * h2) >> 15;
                if(y < 0) {
                    y = -y;
                    ys = y1 - y;
                } else {
                    ys = y1;
                }
                renderer->DrawFillRect(GUIRect(xleft + x, ys, 1, y));
                i += channels;
                if(i >= SAMPLE_ARRAY_SIZE) i -= SAMPLE_ARRAY_SIZE;
            }
        }

        renderer->DrawColor(0, 0, 255, 255);

        for(ch = 1; ch < nb_display_channels; ch++) {
            y = ytop + ch * h;
            renderer->DrawFillRect(GUIRect(xleft, y, size.width, 1));
        }
    } else {
        // SDL_BLENDMODE_NONE SDL_BLENDMODE_BLEND
        if(realloc_texture(renderer, &vis_texture, SDL_PIXELFORMAT_ARGB8888, size.width, size.height, SDL_BLENDMODE_NONE, true) < 0)
            return;

        nb_display_channels = FFMIN(nb_display_channels, 2);
        if(cur_rdft_bits != rdft_bits) {
            av_rdft_end(rdft);
            av_free(rdft_data);
            rdft = av_rdft_init(cur_rdft_bits, DFT_R2C);
            rdft_bits = cur_rdft_bits;
            rdft_data = static_cast<FFTSample*>(av_malloc_array(nb_freq, 4 * sizeof(*rdft_data)));
        }
        if(!rdft || !rdft_data) {
            av_log(nullptr, AV_LOG_ERROR, "Failed to allocate buffers for RDFT, switching to waves display\n");
            show_mode = SHOW_MODE_WAVES;
        } else {
            FFTSample* data[2];
            SDL_Rect rect; // = { .x = s->xpos,.y = 0,.w = 1,.h = s->height };
            rect.x = xpos;
            rect.y = 0;
            rect.w = 1;
            rect.h = size.height;
            uint32_t* pixels;
            int pitch;
            for(ch = 0; ch < nb_display_channels; ch++) {
                data[ch] = rdft_data + 2 * nb_freq * ch;
                i = i_start + ch;
                for(x = 0; x < 2 * nb_freq; x++) {
                    const auto w = (x - nb_freq) * (1.0 / nb_freq);
                    data[ch][x] = sample_array[i] * (1.0 - w * w);
                    i += channels;
                    if(i >= SAMPLE_ARRAY_SIZE) i -= SAMPLE_ARRAY_SIZE;
                }
                av_rdft_calc(rdft, data[ch]);
            }
            /* Least efficient way to do this, we should of course
             * directly access it but it is more than fast enough.*/
            if(SDL_LockTexture(vis_texture->operator SDL_Texture*(), &rect, reinterpret_cast<void**>(&pixels), &pitch) == 0) {
                pitch >>= 2;
                pixels += pitch * size.height;
                for(y = 0; y < size.height; y++) {
                    const auto w = 1 / sqrt(nb_freq);
                    auto a = static_cast<int>(
                    sqrt(w * sqrt(data[0][2 * y + 0] * data[0][2 * y + 0] + data[0][2 * y + 1] * data[0][2 * y + 1])));
                    auto b =
                    nb_display_channels == 2 ? static_cast<int>(sqrt(w * hypot(data[1][2 * y + 0], data[1][2 * y + 1]))) : a;
                    a = FFMIN(a, 255);
                    b = FFMIN(b, 255);
                    pixels -= pitch;
                    *pixels = (0xff << 24) + (a << 16) + (b << 8) + ((a + b) >> 1);
                }
                SDL_UnlockTexture(vis_texture->operator SDL_Texture*());
            }
            renderer->RenderCopy(vis_texture, GUIPoint(0, 0));
        }
        if(!paused) xpos++;
        if(xpos >= size.width) xpos = xleft;
    }
}

bool MediaStream::UpdateUi(GUIRenderer* renderer, GUITexture* screen)
{
    if(paused) return false;

    auto repaint = false;
    if(audio_stream_ && show_mode != SHOW_MODE_VIDEO) {
        const auto time = av_gettime_relative() / 1000000.0;
        if(force_refresh || last_vis_time + rdftspeed < time) {
            renderer->Clear(transparent_color);
            video_audio_display(renderer, screen);
            repaint = true;
            last_vis_time = time;
        }
        //*remaining_time = FFMIN(*remaining_time, is->last_vis_time + rdftspeed - time);
    } else if(video_stream_) {
        renderer->Clear(transparent_color);
        video_image_display(renderer, screen);
        repaint = true;
    }
    return repaint;
}

void MediaStream::Pause()
{
    if(paused) return;
    StreamTogglePause();
    step = 0;
}

void MediaStream::Resume()
{
    if(!paused) return;
    StreamTogglePause();
}

void MediaStream::GetPlayTimes(int64_t* totalTime, int64_t* currentTime) const
{
    *totalTime = totalTime_;
    *currentTime = currentTime_;
}

void MediaStream::GetMetaData(AudioStreamInfo* streamInfo) const
{
    streamInfo->metaAlbum_ = metaAlbum_;
    streamInfo->metaArtist_ = metaArtist_;
    streamInfo->metaTitle_ = metaTitle_;
}
