#pragma once

#include <string>

#include "app.h"

class Server;
class Client;
class Session;
class Connection;
struct ISNLink;
struct ISNLinkHandler;
class Protocol {
public:
    Protocol(int protocol, const std::string& name);
    ~Protocol();

public:
    virtual Server*     create_server() = 0;
    virtual Session*    create_session(const std::string& stream) = 0;
    virtual Connection* create_connection(ISNLink* link) = 0;
    virtual Connection* create_connection(const std::string& ip, short port, const std::string& path, bool player, ISNLinkHandler* handler) = 0;
    virtual Client*     create_client(const std::string& url, bool player) = 0;

public:
    int protocol() { return m_nProtocol; }
    std::string name() { return m_strName; }

private:
    int         m_nProtocol;
    std::string m_strName;
};

std::string     protocol_get_name(int protcol);
int protocol_parse_url(const std::string& url);
Protocol*   protocol_get_rtmp();
Protocol*   protocol_get_rtp();
Protocol*   protocol_get_somo();
