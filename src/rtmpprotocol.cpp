#include "rtmpprotocol.h"
#include "rtmp/rtmpserver.h"
#include "rtmp/rtmpsession.h"
#include "rtmp/rtmpconnection.h"
#include "rtmp/rtmpclient.h"

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

Connection* RtmpProtocol::create_connection(const std::string& ip, short port, const std::string& path, bool player, ISNLinkHandler* handler) {
    return new RtmpConnection(ip, port, path, player, handler);
}

Client*     RtmpProtocol::create_client(const std::string& url, bool player) {
    return new RtmpClient(this, url, player);
}