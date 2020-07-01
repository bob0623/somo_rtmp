#pragma once

#include "isnet.h"
#include <string>

struct ISNLink;
class Protocol;
class Connection;
class Client : public ISNLinkHandler {
public:
    Client(Protocol* protocol, const std::string& url);
    ~Client();

public:
    virtual void on_connected(ISNLink* pLink);
    virtual void on_close(ISNLink* pLink);
    virtual int  on_data(const char* data, size_t len, ISNLink* pLink);

public:
    Connection*     connection() { return m_pConnection; }
    std::string     url() { return m_strUrl; }
    std::string     ip() { return m_strIp; }
    short   port() { return m_nPort; }
    std::string     path() { return m_strPath; }

private:
    void    connect();

protected:
    Protocol*       m_pProtocol;
    std::string     m_strUrl;
    std::string     m_strIp;
    short           m_nPort;
    std::string     m_strPath;
    Connection*     m_pConnection;
};

