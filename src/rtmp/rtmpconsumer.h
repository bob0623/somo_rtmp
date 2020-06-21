#pragma once

#include "session.h"

class RtmpConsumer : public Consumer {
public:
    RtmpConsumer();
    ~RtmpConsumer();

public:
    virtual void    on_audio(AudioFrame* frame);
    virtual void    on_video(VideoFrame* frame);

private:

};

