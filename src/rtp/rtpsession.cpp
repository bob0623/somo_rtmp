#include "rtpsession.h"
#include "rtph264.h"
#include "rtp.h"
#include "rtcp.h"
#include "filewriter.h"
#include "rtmppusher.h"
#include "Config.h"

#include "format.h"
#include "uni.h"
#include "logger.h"

#define __CLASS__ "RtpSession"

RtpSession::RtpSession(uint32_t ssrc) 
: m_nSSRC(ssrc)
, m_nLastStamp(0)
, m_nPackets(0)
, m_pWriter(NULL)
, m_pRtmp(NULL)
, m_bFirstIFrame(false)
{
    m_pBuilder = new RtpH264FrameBuilder();
    if( Config::Ins()->record() != 0 ) {
        std::string filename = Config::Ins()->get_record_folder() + "/" + somo::video::format::uint2str64(ssrc) + ".h264";
        m_pWriter = new FileWriter(ssrc, filename);
    }

    if( Config::Ins()->rtmp() != 0 ) {
        std::string url = Config::Ins()->rtmp_url() + "2000U" + somo::video::format::uint2str64(ssrc);
        m_pRtmp = new RtmpPusher(url);
        m_pRtmp->start();
        FUNLOG(Info, "rtp init rtmp pusher, url=%s", url.c_str());
    }
}
    
RtpSession::~RtpSession() {
    delete m_pBuilder;
    if( m_pWriter ) {
        delete m_pWriter;
    }
    if( m_pRtmp ) {
        delete m_pRtmp;
    }
}

void    RtpSession::addVideoPacket(RtpPacket* packet) {
    m_nPackets++;
    if( m_nPackets%20 == 0 ) {
        m_nLastStamp = uni::sys_time_msec();
    }
    m_pBuilder->addPacket(packet);

    handleFrames();
}

void    RtpSession::addRtcpPacket(RtcpPacket* packet) {
    packet->dump();
}

void    RtpSession::handleFrames() {
    while(true) {
        RtpH264Frame* frame = m_pBuilder->getReadyFrame();
        if( frame == NULL )
            break;

        if( m_bFirstIFrame ) {
            if( m_pWriter != NULL ) {
                m_pWriter->write(frame->getData(), frame->getLen());
            }
            if( m_pRtmp ) {
                m_pRtmp->sendH264(frame->getData(), frame->getLen(), uni::sys_time_msec(), frame->getFrameId());
            }
        } else {
            if( frame->getNaluType() !=7 && frame->getNaluType() != 8 ) {
                return;
            }
            if( m_pWriter != NULL ) {
                m_pWriter->write(frame->getData(), frame->getLen());
            }
            if( m_pRtmp ) {
                m_pRtmp->sendH264(frame->getData(), frame->getLen(), uni::sys_time_msec(), frame->getFrameId());
            }
            m_bFirstIFrame = true;
        }
    } 
}

bool    RtpSession::isExpire(uint64_t stamp) {
    if( m_nLastStamp != 0 && stamp >= m_nLastStamp + 1000*10 ) {
        return true;
    }

    return false;
}

