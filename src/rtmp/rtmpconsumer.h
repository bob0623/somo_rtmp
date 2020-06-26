#pragma once

#include "session.h"

class RtmpStream;
class AudioFrame;
class VideoFrame;
class RtmpConsumer : public Consumer {
public:
    RtmpConsumer(RtmpStream* stream, uint32_t id);
    ~RtmpConsumer();

public:
    virtual uint32_t  id();
    virtual void    on_audio(AudioFrame* frame);
    virtual void    on_audio_rtmp(const char* data, int len);

    virtual void    on_video(VideoFrame* frame);
    virtual void    on_video_rtmp(const char* data, int len);
    virtual void    on_video_rtmp_sh(const char* data, int len);

private:
    uint32_t    m_nId;
    RtmpStream* m_pStream;
};

