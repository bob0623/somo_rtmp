#include "protocol.h"
#include "rtmp/rtmpconnection.h"

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
    default:
        break;
    }

    return "";
}
