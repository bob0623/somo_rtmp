#include "rtpprotocol.h"
#include "rtp/rtpserver.h"

RtpProtocol::RtpProtocol()
: Protocol(PROTOCOL_RTP, "rtp")
{
}

RtpProtocol::~RtpProtocol() {

}

Server*     RtpProtocol::create_server() {
    return new RtpServer(this);
}

Session*    RtpProtocol::create_session(const std::string& stream) {
    //return new RtmpSession(stream);
    return NULL;
}

Connection* RtpProtocol::create_connection(ISNLink* link) {
    //return new RtmpConnection(link);
    return NULL;
}

Connection* RtpProtocol::create_connection(const std::string& ip, short port, const std::string& path, bool player, ISNLinkHandler* handler) {
    //return new RtmpConnection(ip, port, path, player, handler);
    return NULL;
}

Client*     RtpProtocol::create_client(const std::string& url, bool player) {
    //return new RtmpClient(this, url, player);
    return NULL;
}