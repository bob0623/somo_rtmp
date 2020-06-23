#include "app.h"
#include "server.h"
#include "session.h"
#include "protocol.h"
#include "rtmpprotocol.h"

#include "common/logger.h"

#define __CLASS__   "App"

App::App() {
    add_protocol(PROTOCOL_RTMP);
}

App::~App() {
    clear_servers();
    clear_sessions();
}

void    App::add_protocol(int proto) {
    auto it = m_mapProtocols.find(proto);
    if( it != m_mapProtocols.end() ) {
        FUNLOG(Error, "app add protocol failed, already exist protocol=%d", proto);
        return;
    }

    if( proto == PROTOCOL_RTMP ) {
        m_mapProtocols[proto] = new RtmpProtocol();
    }
}

Protocol*   App::get_protocol(int proto) {
    auto it = m_mapProtocols.find(proto);
    if( it == m_mapProtocols.end() ) 
        return NULL;
    return it->second;
}

void    App::add_server(short port, int proto) {
    Protocol* protocol = get_protocol(proto);
    if( protocol == NULL ) {
        FUNLOG(Error, "app add server failed, protocol==NULL for port=%d, protocol=%d", port, proto);
        return;
    }

    add_server(port, protocol);
}

void    App::add_server(short port, Protocol* protocol) {
    FUNLOG(Info, "app add protocol, port=%d, protocol=%d, name=%s", port, protocol->protocol(), protocol->name().c_str());
    Server* server = protocol->create_server();
    server->listen(port);

    m_mapServers[port] = server;
}


Server*   App::get_server(short port) {
    auto it = m_mapServers.find(port);
    if( it == m_mapServers.end() ) 
        return NULL;

    return it->second;
}

void     App::clear_servers() {
    for( auto it = m_mapServers.begin(); it!=m_mapServers.end(); it++ ) {
        if( it->second ) {
            delete it->second;
        }
    }
    m_mapServers.clear();
}

Session* App::add_session(const std::string& stream, int proto) {
    FUNLOG(Info, "app add session, stream=%s, proto=%d", stream.c_str(), proto);
    Protocol* protocol = get_protocol(proto);
    if( protocol == NULL ) {
        FUNLOG(Error, "app add session failed, protocol==NULL for stream=%s, proto=%d", stream.c_str(), proto);
        return NULL;
    }

    return add_session(stream, protocol);
}

Session*    App::add_session(const std::string& stream, Protocol* protocol) {
    Session* session = get_session(stream);
    if( session == NULL ) {
        session = protocol->create_session(stream);
        m_mapSessions[stream] = session;
    }

    return session;
}

Session* App::get_session(const std::string& stream) {
    auto it = m_mapSessions.find(stream);
    if( it == m_mapSessions.end() ) {
        return NULL;
    }

    return it->second;
}

void    App::clear_sessions() {
    for( auto it=m_mapSessions.begin(); it!=m_mapSessions.end(); it++ ) {
        if( it->second ) {
            delete it->second;
        }
    }
    m_mapSessions.clear();
}
