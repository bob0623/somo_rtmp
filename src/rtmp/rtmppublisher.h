#pragma once

#include "session.h"

class RtmpStream;
class RtmpParser;
class AudioFrame;
class VideoFrame;
class RtmpPublisher : public Publisher {
public:
    RtmpPublisher(RtmpStream* stream);
    virtual ~RtmpPublisher();

public:
    virtual uint32_t  id();
    virtual void    on_new_consumer(Consumer* consumer);

    virtual void    on_meta_data(const char* data, int len);

    virtual void    on_audio(AudioFrame* frame);
    virtual void    on_audio_rtmp(const char* data, int len);
    
    virtual void    on_video(VideoFrame* frame);
    virtual void    on_video_rtmp(const char* data, int len);
    virtual void    on_video_rtmp_sh(const char* data, int len);

private:
    RtmpStream* m_pStream;
    uint64_t    m_nAudioFrames;
    uint64_t    m_nVideoFrames;
    char*       m_pSeqHeader;
    int         m_nSeqHeaderLen;

    char*       m_pMeta;
    int         m_nMetaLen;
};

