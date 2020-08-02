#include "rtmppublisher.h"
#include "rtmpstream.h"
#include "rtmpsession.h"
#include "rtmpconnection.h"
#include "rtmpformat.h"

#include "video/videoframe.h"
#include "video/videoframepool.h"
#include "audio/audioframe.h"

#include "common/logger.h"

#include <string.h>

#define __CLASS__   "RtmpPublisher"

RtmpPublisher::RtmpPublisher(RtmpStream* stream)
: m_pStream(stream) 
, m_nSeqHeaderLen(0)
, m_nMetaLen(0)
, m_nAudioFrames(0)
, m_nVideoFrames(0)
{
    FUNLOG(Info, "construct Rtmppublisher, this=%p", this);
    m_pSeqHeader = new char[1024];
    m_pMeta = new char[1024];
}

RtmpPublisher::~RtmpPublisher() {
    FUNLOG(Info, "dealloc rtmppublisher, this=%p", this);
    delete m_pSeqHeader;
    delete m_pMeta;
    FUNLOG(Info, "dealloc rtmppublisher complete.", NULL);
}

uint32_t  RtmpPublisher::id() {
    if( m_pStream ) {
        return m_pStream->connection()->linkid();
    }

    return 0;
}

std::string RtmpPublisher::stream() {
    if( m_pStream ) {
        return m_pStream->connection()->stream()->stream();
    }
}

/**
 * callback when new consumer subscribe this stream.
 * 1, send video sequence header to this new consumer!
 * 2, send sps&pps to this new consumer;
 * 3, send GOP to this new consumer [TBD];
 */
void    RtmpPublisher::on_new_consumer(Consumer* consumer) {
    FUNLOG(Info, "rtmp publisher on new consumer, consumer_id=%d", consumer->id());

    //1, send video sequence header to this new consumer!
    consumer->on_video_rtmp_sh( m_pSeqHeader, m_nSeqHeaderLen );

    //2, send sps&pps to this new consumer;
    consumer->on_meta_data( m_pSeqHeader, m_nSeqHeaderLen );

    //3, send GOP to this new consumer [TBD];
    
}

void    RtmpPublisher::on_meta_data(const char* data, int len) {
    memcpy(m_pMeta, data, len);
    m_nMetaLen = len;

    m_pStream->session()->on_meta_data(data, len);
}

void    RtmpPublisher::on_audio(AudioFrame* frame) {
    m_nAudioFrames++;

    if( m_nAudioFrames%1000 == 0 || m_nAudioFrames <= 5 ) {
        FUNLOG(Info, "rtmp publisher on audio frame, frames=%d, size=%d", m_nAudioFrames, frame->size());
    }
    
}

void    RtmpPublisher::on_audio_rtmp(const char* data, int len) {
    m_nAudioFrames++;

    if( m_nAudioFrames%1000 == 0 || m_nAudioFrames <= 5) {
        FUNLOG(Info, "rtmp publisher on audio rtmp, frames=%d, size=%d,", m_nAudioFrames, len);
    }
    m_pStream->session()->on_audio_rtmp(data, len);
}

void    RtmpPublisher::on_video(VideoFrame* frame) {
    m_nVideoFrames++;

    if( m_nVideoFrames%300 == 0 || m_nVideoFrames <= 5) {
        FUNLOG(Info, "rtmp publisher on video frame, frames=%d, size=%d,", m_nVideoFrames, frame->size());
    }
}

void    RtmpPublisher::on_video_rtmp(const char* data, int len) {
    m_nVideoFrames++;

    if( m_nVideoFrames%300 == 0 || m_nVideoFrames <= 5) {
        FUNLOG(Info, "rtmp publisher on video rtmp, frames=%d, size=%d,", m_nVideoFrames, len);
    }
    m_pStream->session()->on_video_rtmp(data, len);
}

void    RtmpPublisher::on_video_rtmp_sh(const char* data, int len) {
    memcpy(m_pSeqHeader, data, len);
    m_nSeqHeaderLen = len;
}

