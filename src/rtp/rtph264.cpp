#include "rtph264.h"

#include "logger.h"

#include <string.h>

#define __CLASS__ "RtpH264Frame"
#define MAX_FRAME_SIZE  1024*512

RtpH264Frame::RtpH264Frame(uint32_t ssrc, int nalu_type) 
: m_nFrameId(0)
, m_nSSRC(ssrc)
, m_nNaluType(nalu_type)
{
    m_nCapacity = MAX_FRAME_SIZE;
    m_pData = new char[m_nCapacity];

    //frame start with 0x00000001
    m_pData[0] = 0x0;
    m_pData[1] = 0x0;
    m_pData[2] = 0x0;
    m_pData[3] = 0x1;
    m_nLen = 4;
}

RtpH264Frame::~RtpH264Frame() {
    delete m_pData;
}

void    RtpH264Frame::setFrameId(uint32_t frame) {
    m_nFrameId = frame;
}

void    RtpH264Frame::addPacket(const char* data, int len) {
    memcpy(m_pData+m_nLen, data, len);
    m_nLen += len;
}




RtpH264FrameBuilder::RtpH264FrameBuilder() 
: m_nLastSeq(0)
, m_nLastReadySeq(0)
, m_nLastReadyFrame(0)
, m_pCurrentFrame(NULL)
, m_nBuildFrames(0)
{

}

RtpH264FrameBuilder::~RtpH264FrameBuilder() {

}

void    RtpH264FrameBuilder::addPacket(RtpPacket* packet) {
    if( m_nLastSeq == 0 ) {
        m_nLastSeq = packet->getSeq();
    } else {
        if( packet->getSeq() > m_nLastSeq + 1 ) {
            //FUNLOG(Warn, "rtp h264 add packet, maybe lost, last=%u, arrived=%u", m_nLastSeq, packet->getSeq());
            m_nLastSeq = packet->getSeq();
        } else if( packet->getSeq() == m_nLastSeq + 1 ) {
            //FUNLOG(Warn, "rtp h264 add packet, add disort packet, last=%u, arrived=%u", m_nLastSeq, packet->getSeq());
            m_nLastSeq = packet->getSeq();
        } else if( packet->getSeq() + 5000 <= m_nLastSeq ) {
            //seq reset:
            FUNLOG(Info, "rtp h264 add packet, seq reset!!! last, last=%u, arrived=%u", m_nLastSeq, packet->getSeq());
            clear();
            m_nLastSeq = packet->getSeq();
        }
    }
    m_mapPackets[packet->getSeq()] = packet;

    //update losts:
    if( m_setLosts.find(packet->getSeq()) != m_setLosts.end() ) {
        m_setLosts.erase(packet->getSeq());
    }
    //getRtpH264Type(packet);

    //build frames:
    buildFrames();

    trim();
}

void    RtpH264FrameBuilder::buildFrames() {
    if( m_mapPackets.empty() )
        return;

    uint32_t first_packet = 0;
    RtpPacket* packet = NULL;
    if( m_nLastReadySeq == 0 ) {
        packet = m_mapPackets.begin()->second;
        first_packet = packet->getSeq();
        m_nLastReadySeq = first_packet;
        //FUNLOG(Info, "rtp h264 builder, first packet!!! ssrc=%u, seq=%u", packet->getSSRC(), packet->getSeq());
    } else {
        first_packet = m_nLastReadySeq+1;
        if(first_packet >= m_nLastSeq ) {
            //no more packets!!!!
            return;
        }

        auto it = m_mapPackets.find(first_packet);
        if( it == m_mapPackets.end() ) {
            dropLostPackets();
            return;
        }

        packet = it->second;
        m_nLastReadySeq = first_packet;
    }
    

    //now handle the packets:
    m_nBuildFrames++;
    int type = getRtpH264Type(packet);
    if( m_nBuildFrames%100 == 0 ) {
        FUNLOG(Info, "rtp h264 build, check H264 type=%d, ssrc=%u, seq=%u", type, packet->getSSRC(), packet->getSeq());
    }
    if( type <= 23 ) {
        handleNalu(packet);
    } else if( type == H264_PT_STAPA ) {
        handleSTAPA(packet);
    } else if( type == H264_PT_STAPB ) {
        FUNLOG(Error, "rtp h264 builder, STAPB!!! ssrc=%u, seq=%u, type=%d", packet->getSSRC(), packet->getSeq(), type);
    } else if( type == H264_PT_MTAP16 ) {
        FUNLOG(Error, "rtp h264 builder, MATP16!!! ssrc=%u, seq=%u, type=%d", packet->getSSRC(), packet->getSeq(), type);
    } else if( type == H264_PT_MTAP24 ) {
        FUNLOG(Error, "rtp h264 builder, MATP32!!! ssrc=%u, seq=%u, type=%d", packet->getSSRC(), packet->getSeq(), type);
    } else if( type == H264_PT_FUA ) {
        handleFUA(packet);
    } else if( type == H264_PT_FUB ) {
        FUNLOG(Error, "rtp h264 builder, FUB!!! ssrc=%u, seq=%u, type=%d", packet->getSSRC(), packet->getSeq(), type);
    } else {
        FUNLOG(Error, "rtp h264 builder, invalid H264 type!!! ssrc=%u, seq=%u, type=%d", packet->getSSRC(), packet->getSeq(), type);
    }
}

RtpH264Frame*   RtpH264FrameBuilder::getReadyFrame() {
    if( m_mapReadyFrames.empty() )
        return NULL;

    auto it = m_mapReadyFrames.begin();
    RtpH264Frame* frame = it->second;
    m_mapReadyFrames.erase(it->first);

    return frame;
}

void    RtpH264FrameBuilder::getLostSeqs(std::set<uint32_t>& losts, int max) {
    losts.clear();
    for( auto it = m_setLosts.begin(); it!=m_setLosts.end(); it++ ) {
        losts.insert(*it);
        if( losts.size() >= max )
            break;
    }
}

int     RtpH264FrameBuilder::getRtpH264Type(RtpPacket* packet) {
    const char* payload = packet->getPayload();
    if( payload == NULL ) {
        FUNLOG(Error, "rtp h264 builder, invalid payload==NULL, ssrc=%u, seq=%u", packet->getSSRC(), packet->getSeq());
        return 0;
    }
    uint8_t first_byte = *(uint8_t*)payload;
    int type = first_byte&0x1f;

    //FUNLOG(Info, "rtp h264 build, check H264 type=%d, ssrc=%u, seq=%u", type, packet->getSSRC(), packet->getSeq());
    return type;
}

void    RtpH264FrameBuilder::dropLostPackets() {

}

void    RtpH264FrameBuilder::handleNalu(RtpPacket* packet) {
    char* payload = packet->getPayload();
    uint8_t first_byte = *(uint8_t*)payload;
    int type = first_byte&0x1f;
    if( type == 7 ) {
        //FUNLOG(Info, "rtp h264 builder, found SPS, ssrc=%u, seq=%u, type=%d, len=%d", packet->getSSRC(), packet->getSeq(), type, packet->getPayloadLength() );
    } else if( type == 8 ) {
        //FUNLOG(Info, "rtp h264 builder, found PPS, ssrc=%u, seq=%u, type=%d, len=%d", packet->getSSRC(), packet->getSeq(), type, packet->getPayloadLength() );
    } else if( type == 5 ) {
        FUNLOG(Info, "rtp h264 builder, found I Frame, ssrc=%u, seq=%u, type=%d, len=%d", packet->getSSRC(), packet->getSeq(), type, packet->getPayloadLength() );
    }
    m_pCurrentFrame = new RtpH264Frame(packet->getSSRC(), type);
    m_pCurrentFrame->addPacket(payload, packet->getPayloadLength());
    makeFrameReady();
}

void    RtpH264FrameBuilder::handleSTAPA(RtpPacket* packet) {
    char* payload = packet->getPayload();
    char* pos = payload+1;

    while( pos < payload + packet->getPayloadLength() ) {
        uint16_t nalu_size = ntohs(*(uint16_t*)pos);
        if( nalu_size < 0 || nalu_size > packet->getPayloadLength()-1 ) {
            //FUNLOG(Error, "rtp h264 builder, invalid nalu size for STAP-A, ssrc=%u, seq=%u, nalu_size=%d, payload.len=%d, total.len=%d", 
            //    packet->getSSRC(), packet->getSeq(), nalu_size, packet->getPayloadLength(), packet->getLen());
            return;
        }
        pos = pos + 2;
        uint8_t first_byte = *(uint8_t*)pos;
        int type = first_byte&0x1f;

        m_pCurrentFrame = new RtpH264Frame(packet->getSSRC(), type);
        m_pCurrentFrame->addPacket(pos, nalu_size);
        makeFrameReady();

        //FUNLOG(Info, "rtp h264 builder, get nalu from SPAP-A, ssrc=%u, seq=%d, nalu_size=%d", packet->getSSRC(), packet->getSeq(), nalu_size);

        pos += nalu_size;
    }
}

void    RtpH264FrameBuilder::handleFUA(RtpPacket* packet) {
    char* payload = packet->getPayload();
    fua_indicator* fuai = (fua_indicator*)payload;
    fua_hdr* fuah = (fua_hdr*)(payload+1);

    if( fuai->type != 28 ) {
        FUNLOG(Error, "rtp h264 builder, not valid FU-A packet!!! ssrc=%u, seq=%u, nalu_type=%d, len=%d", packet->getSSRC(), packet->getSeq(), fuah->type, packet->getPayloadLength()-2);
        return;
    }
    if( fuah->s == 1 ) {
        //start:
        //FUNLOG(Info, "rtp h264 builder, first FU-A packet!!! ssrc=%u, seq=%u, nalu_type=%d, len=%d", packet->getSSRC(), packet->getSeq(), fuah->type, packet->getPayloadLength()-2);
        m_pCurrentFrame = new RtpH264Frame(packet->getSSRC(), fuah->type);

        uint8_t nal_fua = (payload[0]&0xe0 )|(payload[1]&0x1f);
        payload[1] = nal_fua;
        m_pCurrentFrame->addPacket(payload+1, packet->getPayloadLength()-1);
    } else if( fuah->e == 1 ) {
        //end:
        //FUNLOG(Info, "rtp h264 builder, last FU-A packet!!! ssrc=%u, seq=%u, nalu_type=%d, len=%d", packet->getSSRC(), packet->getSeq(), fuah->type, packet->getPayloadLength()-2);
        if( m_pCurrentFrame == NULL )
            return;
        m_pCurrentFrame->addPacket(payload+2, packet->getPayloadLength()-2);
        makeFrameReady();
    } else {
        if( m_pCurrentFrame == NULL )
            return;
        m_pCurrentFrame->addPacket(payload+2, packet->getPayloadLength()-2);
    }
}

void    RtpH264FrameBuilder::makeFrameReady() {
    m_nLastReadyFrame++;
    m_pCurrentFrame->setFrameId(m_nLastReadyFrame);
    m_mapReadyFrames[m_nLastReadyFrame] = m_pCurrentFrame;
    m_mapFrames[m_nLastReadyFrame] = m_pCurrentFrame;

    if( m_nLastReadyFrame%100 == 0 || m_pCurrentFrame->getNaluType() == 5 || m_pCurrentFrame->getNaluType() == 7 || m_pCurrentFrame->getNaluType() == 8 ) {
        FUNLOG(Info, "rtp h264 frame ready, ssrc=%u, frame=%d, type=%d, len=%d, start=%x %x %x %x %x %x %x %x", 
            m_pCurrentFrame->getSSRC(), m_pCurrentFrame->getFrameId(), m_pCurrentFrame->getNaluType(), m_pCurrentFrame->getLen(),
            m_pCurrentFrame->getData()[0], m_pCurrentFrame->getData()[1], m_pCurrentFrame->getData()[2], m_pCurrentFrame->getData()[3], 
            m_pCurrentFrame->getData()[4], m_pCurrentFrame->getData()[5], m_pCurrentFrame->getData()[6], m_pCurrentFrame->getData()[7]);
    }
    //clear m_pCurrrentFrame
    m_pCurrentFrame = NULL;
}

void    RtpH264FrameBuilder::trim() {
    while( m_mapPackets.size()>2000 ) {
        RtpPacket* packet = m_mapPackets.begin()->second;
        uint32_t seq = packet->getSeq();
        delete packet;
        m_mapPackets.erase( m_mapPackets.begin());
    }

    while( m_mapFrames.size() > 100 ) {
        RtpH264Frame* frame = m_mapFrames.begin()->second;
        uint32_t frameId = m_mapFrames.begin()->first;
        delete frame;
        m_mapFrames.erase(frameId);
    }

    while( m_mapReadyFrames.size() > 100 ) {
        RtpH264Frame* frame = m_mapReadyFrames.begin()->second;
        uint32_t frameId = m_mapFrames.begin()->first;
        m_mapFrames.erase(frameId);
    }
}

void    RtpH264FrameBuilder::clear() {
    for( auto it=m_mapPackets.begin(); it!=m_mapPackets.end(); it++ ) {
        delete it->second;
    }
    m_mapPackets.clear();

    for( auto it=m_mapFrames.begin(); it!=m_mapFrames.end(); it++ ) {
        delete it->second;
    }
    m_mapFrames.clear();

    m_mapReadyFrames.clear();
    m_setLosts.clear();

    m_nLastReadyFrame = 0;
    m_nLastSeq = 0;
    m_nLastReadySeq = 0;
    m_nBuildFrames = 0;
    m_pCurrentFrame = NULL;
}