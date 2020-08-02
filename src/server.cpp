#include "server.h"
#include "protocol.h"
#include "connection.h"
#include "session.h"
#include "common/logger.h"

#define __CLASS__ "Server"

Server::Server(Protocol* protocol) 
: m_pListener(NULL)
, m_pProtocol(protocol)
{

}

Server::~Server() {
    clear();
}

bool Server::listen(uint16_t port) {
    m_pListener = SNFactory::createTcpServer();
    m_pListener->set_handler( this );
    m_pListener->listen(port);

    return true;
}

void    Server::clear() {
    for( auto it=m_mapConnections.begin(); it!=m_mapConnections.end(); it++ ) {
        delete it->second;
    }
    m_mapConnections.clear();
}

void    Server::check_alive() {
    std::set<uint32_t> expire_links;
    for( auto it=m_mapConnections.begin(); it!=m_mapConnections.end(); it++ ) {
        Connection* conn = it->second;
        if( !conn ) {
            continue;
        }

        if( !conn->is_alive() ) {
            FUNLOG(Warn, "server check alive, expire connid=%d", conn->linkid());
            expire_links.insert(it->first);
        }
    }

    for( auto it=expire_links.begin(); it!=expire_links.end(); it++ ) {
        m_mapConnections.erase(*it);
    }
}

void    Server::on_connected(ISNLink* link) {
    FUNLOG(Info, "connection open, id=%d", link->linkid());
    Connection* conn = get_connection(link->linkid());
    if( conn != NULL ) {
        FUNLOG(Error, "rtmp connection open, same linkid=%d", link->linkid());
        return;
    }

    conn = m_pProtocol->create_connection(link);
    m_mapConnections[link->linkid()] = conn; 
}

void    Server::on_close(ISNLink* link) {
    FUNLOG(Info, "rtmp connection close, id=%d", link->linkid());
    //remove from Session:
    Connection* connection;
    Session* session = NULL;
    Publisher* publisher = NULL;

    //remove the connection:
    auto it = m_mapConnections.find(link->linkid());
    if( it == m_mapConnections.end() ) {
        return;
    }
    connection = it->second;
    // FUNLOG(Info, "session = %p", connection->session());
    session = connection->session();
    if (session == NULL) {
        FUNLOG(Warn, "session == NULL, exit.", NULL);
        return;
    }
    publisher = session->publisher();
    if (publisher == NULL) {
        FUNLOG(Warn, "publisher == NULL, exit.", NULL);
        return;
    }
    if( publisher->id() == link->linkid() ) {
        session->remove_publisher();
    } else {
        session->remove_consumer(link->linkid());
    }
    // FUNLOG(Info, "delete connection, connection=%p", connection);
    //delete the Connection:
    delete connection;
    m_mapConnections.erase(it);
    FUNLOG(Info, "rtmp connection close complete, id=%d", link->linkid());
}

int    Server::on_data(const char* data, size_t len, ISNLink* link) {
    //FUNLOG(Info, "rtmp connection data, id=%d, len=%d", link->linkid(), len);
    Connection* conn = get_connection(link->linkid());
    if( conn == NULL ) {
        FUNLOG(Error, "rtmp connection data, not found linkid=%d", link->linkid());
        return len;
    }

    return conn->on_data(data, len);
}

Connection* Server::get_connection(uint32_t linkid) {
    auto it = m_mapConnections.find(linkid);
    if( it == m_mapConnections.end() ) {
        return NULL;
    }

    return it->second;
}

