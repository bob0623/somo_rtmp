#include "rtpvideohandler.h"
#include "rtp.h"
#include "rtph264.h"
#include "rtpsession.h"

#include "common/logger.h"

#define __CLASS__ "RtpVideoHandler"

RtpVideoHandler::RtpVideoHandler() {
}

RtpVideoHandler::~RtpVideoHandler() {
}

int RtpVideoHandler::on_data(const char* data, size_t len, ISNLink* pLink)
{
    if( len < 12 ) {
        FUNLOG(Error, "rtp link handler, invalid len=%d", len);
        return 0;
    }
    RtpPacket* packet = new RtpPacket(true, data, len);
    if( packet->getSeq()%1000 == 0 ) {
        packet->dump();
    }
    //RtpSessionMgr::Ins()->getSession(packet->getSSRC())->addVideoPacket(packet);

    return len;
}

