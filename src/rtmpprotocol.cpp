#include "rtmpprotocol.h"
#include "rtmp/rtmpserver.h"
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

Connection* RtmpProtocol::create_connection(ISNLink* link) {
    return new RtmpConnection(link);
}