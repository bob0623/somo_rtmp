#pragma once

#include <isnet.h>
#include <string>

class Session;
class Connection {
public:
    Connection(const std::string& ip, short port, ISNLinkHandler* handler);
    Connection(ISNLink* link);
    ~Connection();

public:
    virtual int    on_data(const char* data, int len) = 0;
    virtual Session*    session() = 0;

public:
    void    send(const char* data, int len);
    void    close();
    int     linkid() { return m_pLink->linkid(); }

private:
    ISNLink*    m_pLink;

};
