#include "rtmpclient.h"
#include "rtmpconnection.h"
#include "rtmpshakehands.h"
#include "rtmpstream.h"
#include "rtmpformat.h"

#include "common/util.h"
#include "common/logger.h"

#define __CLASS__   "RtmpClient"

RtmpClient::RtmpClient(const std::string& url)
: m_strUrl(url)
{
    connect();    
}

RtmpClient::~RtmpClient() {
    delete m_pConnection;
}

void    RtmpClient::start_publish() {

}

void    RtmpClient::stop_publish() {

}

void    RtmpClient::on_connected(ISNLink* pLink) {
    FUNLOG(Info, "rtmp client on connected, linkid=%d", pLink->linkid());
    m_pConnection->stream()->send_connect( m_pConnection->get_chunk_stream(2) );
}

void    RtmpClient::on_close(ISNLink* pLink) {
    FUNLOG(Info, "rtmp client on close! linkid=%d", pLink->linkid());
}

int     RtmpClient::on_data(const char* data, size_t len, ISNLink* pLink) {
    return m_pConnection->on_data(data, len);
}

void    RtmpClient::connect() {
    std::string server;
    std::string ip;
    short port = 1935;
    std::string domain = Util::get_url_domain(m_strUrl);
    if( domain.size() == 0 ) {
        FUNLOG(Error, "rtmp forward connect, url=%s, domain=%s", m_strUrl.c_str(), domain.c_str());
        return;
    }

    int pos = domain.find(":");
    if( pos == -1 ) {
        server = domain;
    } else {
        server = domain.substr(0, pos);
        port = atoi( domain.substr(pos+1).c_str() );

        if( port == 0 ) {
            port = 1935;
        }
    }

    m_strIp = ip;
    m_nPort = port;
    m_pConnection = new RtmpConnection(ip, port, this);
}


