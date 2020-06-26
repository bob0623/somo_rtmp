#include "connection.h"

#include "common/logger.h"

#define __CLASS__   "Connection"

Connection::Connection(ISNLink* link) 
: m_pLink(link)
{

}

Connection::~Connection() {
    close();
}

void    Connection::send(const char* data, int len) {
    m_pLink->send(data, len);
}

void    Connection::close() {
    if( m_pLink == NULL )
        return;
        
    m_pLink->close();
    m_pLink = NULL;
}