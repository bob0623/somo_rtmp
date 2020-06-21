#pragma once

#include <isnet.h>

class Connection {
public:
    Connection(ISNLink* link);
    ~Connection();

public:
    virtual int    on_data(const char* data, int len) = 0;

public:
    void    send(const char* data, int len);
    void    close();
    int     linkid() { return m_pLink->linkid(); }

private:
    ISNLink*    m_pLink;

};
