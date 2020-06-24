#pragma once

#include "session.h"

class AudioFrame;
class VideoFrame;
class RtmpConsumer : public Consumer {
public:
    RtmpConsumer(uint32_t id);
    ~RtmpConsumer();

public:
    virtual uint32_t  id();
    virtual void    on_audio(AudioFrame* frame);
    virtual void    on_video(VideoFrame* frame);

private:
    uint32_t    m_nId;
};

