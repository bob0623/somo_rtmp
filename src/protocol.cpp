#include "protocol.h"
#include "rtmp/rtmpconnection.h"
#include "rtmpprotocol.h"
#include "rtpprotocol.h"

Protocol::Protocol(int protocol, const std::string& name) 
: m_nProtocol(protocol)
, m_strName(name)
{

}

Protocol::~Protocol() {

}



std::string protocol_get_name(int protocol) {
    switch (protocol)
    {
    case PROTOCOL_SOMO_UDP:
        return "somo";
        break;
    case PROTOCOL_RTMP:
        return "rtmp";
    case PROTOCOL_RTP:
        return "rtp";
    default:
        break;
    }

    return "";
}

int protocol_parse_url(const std::string& url) {
    if( url.find("rtmp://") != -1 )
        return PROTOCOL_RTMP;
    else if( url.find("somo://") != -1 ) 
        return PROTOCOL_SOMO_UDP;
    else if( url.find("rtp://") != -1 )
        return PROTOCOL_RTP;
} 

Protocol*   protocol_get_rtmp() {
    return new RtmpProtocol();
}

Protocol*   protocol_get_rtp() {
    return new RtpProtocol();
}

Protocol*   protocol_get_somo() {
    return NULL;
}