#pragma once

#include "isnet.h"
#include <stdint.h>
#include <map>
#include <set>

class Protocol;
class Connection;
class Server : public ISNLinkHandler {
public:
    Server(Protocol* protocol);
    virtual ~Server();

public:
    bool    listen(uint16_t port);
    void    clear();
    void    check_alive();

public:
    virtual void on_connected(ISNLink* pLink);
    virtual void on_close(ISNLink* pLink);
    virtual int  on_data(const char* data, size_t len, ISNLink* pLink);

public:
    Connection* get_connection(uint32_t linkid);

protected:
    Protocol*       m_pProtocol;
    ISNTcpServer*   m_pListener;
    std::map<uint32_t, Connection*> m_mapConnections;
};
