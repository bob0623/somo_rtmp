#include "rtmpconsumer.h"

RtmpConsumer::RtmpConsumer(uint32_t id)
: m_nId(id) 
{

}

RtmpConsumer::~RtmpConsumer() {

}

uint32_t    RtmpConsumer::id() {
    return m_nId;
}

void    RtmpConsumer::on_audio(AudioFrame* frame) {

}

void    RtmpConsumer::on_video(VideoFrame* frame) {

}