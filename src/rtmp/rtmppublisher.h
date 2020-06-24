#pragma once

#include "session.h"

class RtmpStream;
class AudioFrame;
class VideoFrame;
class RtmpPublisher : public Publisher {
public:
    RtmpPublisher(RtmpStream* stream);
    ~RtmpPublisher();

public:
    virtual void    on_audio(AudioFrame* frame);
    virtual void    on_video(VideoFrame* frame);

private:
    RtmpStream* m_pStream;
};

