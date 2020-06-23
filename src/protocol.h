#pragma once

#include <string>

#include "app.h"

class Server;
class Session;
class Connection;
struct ISNLink;
class Protocol {
public:
    Protocol(int protocol, const std::string& name);
    ~Protocol();

public:
    virtual Server*     create_server() = 0;
    virtual Session*    create_session(const std::string& stream) = 0;
    virtual Connection* create_connection(ISNLink* link) = 0;

public:
    int protocol() { return m_nProtocol; }
    std::string name() { return m_strName; }

private:
    int         m_nProtocol;
    std::string m_strName;
};

std::string     protocol_get_name(int protcol);
