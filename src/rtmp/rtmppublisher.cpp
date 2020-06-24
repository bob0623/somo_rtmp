#include "rtmppublisher.h"
#include "rtmpstream.h"
#include "rtmpsession.h"

RtmpPublisher::RtmpPublisher(RtmpStream* stream)
: m_pStream(stream) 
{

}

RtmpPublisher::~RtmpPublisher() {

}

void    RtmpPublisher::on_audio(AudioFrame* frame) {

}

void    RtmpPublisher::on_video(VideoFrame* frame) {

}

