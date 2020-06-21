#include "app.h"
#include "server.h"
#include "protocol.h"

#include "common/logger.h"

#define __CLASS__   "App"

App::App() {

}

App::~App() {

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