#include "app.h"
#include "server.h"
#include "session.h"
#include "protocol.h"
#include "rtmpprotocol.h"
#include "rtpprotocol.h"

#include "common/logger.h"
#include "common/util.h"

#define __CLASS__   "App"

App::App() {
    add_protocol(PROTOCOL_RTMP);
    add_protocol(PROTOCOL_RTP);
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
    } else if( proto == PROTOCOL_RTP ) {
        m_mapProtocols[proto] = new RtpProtocol();
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
    Server* server = get_server(port);
    if (server == NULL) {
        server = protocol->create_server();
        server->listen(port);
    }
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
        FUNLOG(Info,"app add session=%p, stream=%s", session, stream.c_str());
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

Client*    App::add_client(const std::string& url, bool player) {
    FUNLOG(Info, "app add client, url=%s, is_player=%s", url.c_str(), player?"yes":"no");
    int proto = protocol_parse_url(url);
    Protocol* protocol = get_protocol(proto);
    if( protocol == NULL ) {
        FUNLOG(Error, "app add client failed, protocol==NULL for url=%s", url.c_str());
        return NULL;
    }

    Client* client = protocol->create_client(url, player);
    m_mapClients[url] = client;

    return client;
}

Client* App::add_forwarder(const std::string& url) {
    FUNLOG(Info, "app add forwarder, url=%s", url.c_str());
    int proto = protocol_parse_url(url);
    Protocol* protocol = get_protocol(proto);
    if( protocol == NULL ) {
        FUNLOG(Error, "app add forwarder failed, protocol==NULL for url=%s", url.c_str());
        return NULL;
    }

    std::string stream = Util::get_url_rtmp_stream(url);
    if( stream.empty() ) {
        FUNLOG(Error, "app add forwarder failed, stream==empty, url=%s", url.c_str());
        return NULL;
    }

    //create the client:
    Client* client = protocol->create_client(url, false);
    m_mapClients[url] = client;

    //get the session:
    Session* session = get_session(stream);
    if( session == NULL ) {
        session = add_session(stream, proto);
    }
    session->add_forwarder(url, client);

    return client;
}

void    App::clear_forwarders() {
    
}