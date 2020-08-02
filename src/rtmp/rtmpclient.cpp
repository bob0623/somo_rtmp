#include "rtmpclient.h"
#include "rtmpconnection.h"
#include "rtmpshakehands.h"
#include "rtmpstream.h"
#include "rtmpformat.h"
#include "protocol.h"

#include "common/util.h"
#include "common/logger.h"

#define __CLASS__   "RtmpClient"

RtmpClient::RtmpClient(Protocol* protocol, const std::string& url, bool player)
: Client(protocol, url, player)
, m_nVideoFrames(0)
{ 
    FUNLOG(Info, "rtmp client new, url=%s, is_player=%s", url.c_str(), player?"yes":"no");
}

RtmpClient::~RtmpClient() {

}

void RtmpClient::on_connected(ISNLink* pLink) {
    FUNLOG(Info, "rtmp client on connected! linkid=%d", pLink->linkid());
    RtmpConnection* conn = (RtmpConnection*)connection();
    if( conn == NULL ) {
        FUNLOG(Error, "rtmp client on connected, conn==NULL for linkid=%d", pLink->linkid());
        return;
    }
    conn->clear();
    conn->start_shake_hands();
}

void RtmpClient::on_close(ISNLink* pLink) {
    FUNLOG(Info, "rtmp client on close! linkid=%d", pLink->linkid());
}

void    RtmpClient::on_meta_data(const char* data, int len) {
    connection()->send(data, len);
}

void    RtmpClient::on_video_rtmp(const char* data, int len) {\
    m_nVideoFrames++;
    if( m_nVideoFrames%1000 == 0 || m_nVideoFrames<=5 ) {
        FUNLOG(Info, "rtmp client on video rtmp, len=%d, frames=%d", len, m_nVideoFrames);
    }
    connection()->send(data, len);
}

void    RtmpClient::on_video_rtmp_sh(const char* data, int len) {
    connection()->send(data, len);
}

RtmpForwarder::RtmpForwarder(const std::string& url)
: RtmpClient( protocol_get_rtmp() , url, false)
{

}
    
RtmpForwarder::~RtmpForwarder()
{
    
}