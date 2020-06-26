#include "rtmppublisher.h"
#include "rtmpstream.h"
#include "rtmpsession.h"
#include "rtmpformat.h"

#include "video/videoframe.h"
#include "video/videoframepool.h"
#include "audio/audioframe.h"

#include "common/logger.h"

#include <string.h>

#define __CLASS__   "RtmpPublisher"

RtmpPublisher::RtmpPublisher(RtmpStream* stream)
: m_pStream(stream) 
, m_nAudioFrames(0)
, m_nVideoFrames(0)
{
    m_pSeqHeader = new char[1024];
}

RtmpPublisher::~RtmpPublisher() {
    delete m_pSeqHeader;
}

void    RtmpPublisher::on_new_consumer(Consumer* consumer) {
    FUNLOG(Info, "rtmp publisher on new consumer, consumer_id=%d", consumer->id());
    consumer->on_video_rtmp_sh( m_pSeqHeader, m_nSeqHeaderLen );
}

void    RtmpPublisher::on_audio(AudioFrame* frame) {
    m_nAudioFrames++;

    if( m_nAudioFrames%100 == 0 || m_nAudioFrames <= 5 ) {
        FUNLOG(Info, "rtmp publisher on audio frame, frames=%d, size=%d", m_nAudioFrames, frame->size());
    }
    
}

void    RtmpPublisher::on_audio_rtmp(const char* data, int len) {
    m_nAudioFrames++;

    if( m_nAudioFrames%100 == 0 || m_nAudioFrames <= 5) {
        FUNLOG(Info, "rtmp publisher on audio frame, frames=%d, size=%d,", m_nAudioFrames, len);
    }
    m_pStream->session()->on_audio_rtmp(data, len);
}

void    RtmpPublisher::on_video(VideoFrame* frame) {
    m_nVideoFrames++;

    if( m_nVideoFrames%30 == 0 || m_nVideoFrames <= 5) {
        FUNLOG(Info, "rtmp publisher on video frame, frames=%d, size=%d,", m_nVideoFrames, frame->size());
    }
}

void    RtmpPublisher::on_video_rtmp(const char* data, int len) {
    m_nVideoFrames++;

    if( m_nVideoFrames%30 == 0 || m_nVideoFrames <= 5) {
        FUNLOG(Info, "rtmp publisher on video frame, frames=%d, size=%d,", m_nVideoFrames, len);
    }
    m_pStream->session()->on_video_rtmp(data, len);
}

void    RtmpPublisher::on_video_rtmp_sh(const char* data, int len) {
    memcpy(m_pSeqHeader, data, len);
    m_nSeqHeaderLen = len;
}

