#include "rtmpprotocol.h"
#include "rtmp/rtmpserver.h"
#include "rtmp/rtmpsession.h"
#include "rtmp/rtmpconnection.h"

RtmpProtocol::RtmpProtocol()
: Protocol(PROTOCOL_RTMP, "rtmp")
{
}

RtmpProtocol::~RtmpProtocol() {

}

Server*     RtmpProtocol::create_server() {
    return new RtmpServer(this);
}

Session*    RtmpProtocol::create_session(const std::string& stream) {
    return new RtmpSession(stream);
}

Connection* RtmpProtocol::create_connection(ISNLink* link) {
    return new RtmpConnection(link);
}