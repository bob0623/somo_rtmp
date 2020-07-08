#include "client.h"
#include "connection.h"
#include "protocol.h"

#include "common/util.h"
#include "common/logger.h"

#define __CLASS__   "Client"

Client::Client(Protocol* protocol, const std::string& url, bool player)
: m_pConnection(NULL)
, m_pProtocol(protocol)
, m_strUrl(url)
, m_bPlayer(player)
{
    connect();    
}

Client::~Client() {
    if( m_pConnection != NULL ) {
        delete m_pConnection;
    }
}

void    Client::on_connected(ISNLink* pLink) {
    FUNLOG(Info, "client on connected!, linkid=%d", pLink->linkid());
}

void    Client::on_close(ISNLink* pLink) {
    FUNLOG(Info, "client on close! linkid=%d", pLink->linkid());
}

int     Client::on_data(const char* data, size_t len, ISNLink* pLink) {
    return m_pConnection->on_data(data, len);
}

void    Client::connect() {
    std::string server;
    std::string ip;
    short port = 1935;
    std::string domain = Util::get_url_domain(m_strUrl);
    if( domain.size() == 0 ) {
        FUNLOG(Error, "client connect failed! url=%s, domain=%s", m_strUrl.c_str(), domain.c_str());
        return;
    }
    FUNLOG(Info, "client connect, url=%s, domain=%s", m_strUrl.c_str(), domain.c_str());

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
    ip = Util::get_host_ip(server.c_str());
    FUNLOG(Info, "client connect, url=%s, domain=%s, ip=%s", m_strUrl.c_str(), domain.c_str(), ip.c_str());

    m_strPath = Util::get_url_path(m_strUrl);

    m_strIp = ip;
    m_nPort = port;
    m_pConnection = m_pProtocol->create_connection(ip, port, m_strPath, m_bPlayer, this);
}


