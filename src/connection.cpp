#include "connection.h"

#include "common/logger.h"

#define __CLASS__   "Connection"

Connection::Connection(const std::string& ip, short port, ISNLinkHandler* handler) {
    FUNLOG(Info, "connection new, ip=%s, port=%d", ip.c_str(), port);
    m_pLink = SNFactory::createTcpLink();
    m_pLink->set_handler(handler);
    m_pLink->connect(ip, port);
}


Connection::Connection(ISNLink* link) 
: m_pLink(link)
{
    //[yunzd] [TODO] if m_pLink is created, how to release?
}

Connection::~Connection() {
    close();
}

void    Connection::send(const char* data, int len) {
    if( m_pLink == NULL ) {
        return;
    }
    m_pLink->send(data, len);
}

void    Connection::close() {
    if( m_pLink == NULL )
        return;
        
    m_pLink->close();
    m_pLink = NULL;
}