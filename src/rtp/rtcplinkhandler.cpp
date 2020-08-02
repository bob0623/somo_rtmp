#include "rtcplinkhandler.h"
#include "rtcp.h"
#include "rtpsessionmgr.h"
#include "rtpsession.h"

#include "uni.h"
#include "logger.h"

#define __CLASS__ "RtcpLinkHandler"

RtcpLinkHandler::RtcpLinkHandler() {
}

RtcpLinkHandler::~RtcpLinkHandler() {
}

int RtcpLinkHandler::on_data(const char* data, size_t len, ISNLink* pLink)
{
    if( len < 2 ) {
        FUNLOG(Error, "rtcp link handler, invalid len=%d", len);
        return 0;
    }
    int type = RtcpPacket::getType(data, len);
    //FUNLOG(Info, "rtcp packet recevied, type=%d", type);
    if( type == RTCP_SR ) {
        RtcpSRPacket* packet = new RtcpSRPacket(data, len);
        RtpSessionMgr::Ins()->getSession(packet->getSSRC())->addRtcpPacket(packet);
    } else if( type == RTCP_RR ) {

    } else if( type == RTCP_RTPFB ) {
        
    }
    return 0;
}

