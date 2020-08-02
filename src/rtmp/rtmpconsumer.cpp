#include "rtmpconsumer.h"
#include "rtmpstream.h"
#include "rtmpconnection.h"
#include "video/videoframe.h"

#include "common/logger.h"

#define __CLASS__ "RtmpConsumer"

RtmpConsumer::RtmpConsumer(RtmpStream* stream, uint32_t id)
: m_pStream(stream)
, m_nId(id) 
, m_nVideoPackets(0)
, m_nAudioPackets(0)
{

}

RtmpConsumer::~RtmpConsumer() {

}

uint32_t    RtmpConsumer::id() {
    return m_nId;
}

void    RtmpConsumer::on_meta_data(const char* data, int len) {
    m_pStream->connection()->send(data, len);
}

void    RtmpConsumer::on_audio(AudioFrame* frame) {

}

void    RtmpConsumer::on_audio_rtmp(const char* data, int len) {
    m_nAudioPackets++;
    if( m_nAudioPackets%1000 == 0 ) {
        FUNLOG(Info, "rtmp consumer on audio rtmp, packets=%llu, len=%d", m_nAudioPackets, len);
    }
    m_pStream->connection()->send(data, len);
}

void    RtmpConsumer::on_video(VideoFrame* frame) {

}

void    RtmpConsumer::on_video_rtmp(const char* data, int len) {
    m_nVideoPackets++;
    if( m_nVideoPackets%1000 == 0 ) {
        FUNLOG(Info, "rtmp consumer on video rtmp, packets=%llu, len=%d", m_nVideoPackets, len);
    }
    m_pStream->connection()->send(data, len);
}

void    RtmpConsumer::on_video_rtmp_sh(const char* data, int len) {
    m_pStream->connection()->send(data, len);
}

