#include "rtpaudiohandler.h"
#include "rtp.h"
#include "rtph264.h"
#include "rtpsessionmgr.h"
#include "rtpsession.h"

#include "uni.h"
#include "logger.h"

#define __CLASS__ "RtpAudioHandler"

RtpAudioHandler::RtpAudioHandler() {
}

RtpAudioHandler::~RtpAudioHandler() {
}

int RtpAudioHandler::on_data(const char* data, size_t len, ISNLink* pLink)
{
    if( len < 12 ) {
        FUNLOG(Error, "rtp audio handler, invalid len=%d", len);
        return 0;
    }

    RtpPacket* packet = new RtpPacket(false, data, len);
    if( packet->getSeq()%1000 == 0 ) {
        packet->dump();
    }
    //RtpSessionMgr::Ins()->getSession(packet->getSSRC())->addPacket(packet);

    return len;
}

