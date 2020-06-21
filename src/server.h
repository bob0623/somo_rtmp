#pragma once

#include "isnet.h"
#include <stdint.h>

class Protocol;
class Connection;
class Server : public ISNLinkHandler {
public:
    Server(Protocol* protocol);
    ~Server();

public:
    bool    listen(uint16_t port);
    void    clear();

public:
    virtual void on_connected(ISNLink* pLink);
    virtual void on_close(ISNLink* pLink);
    virtual int  on_data(const char* data, size_t len, ISNLink* pLink);

public:
    Connection* get_connection(uint32_t linkid);

private:
    Protocol*       m_pProtocol;
    ISNTcpServer*   m_pListener;
    std::map<uint32_t, Connection*> m_mapConnections;
};
