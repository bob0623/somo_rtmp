#pragma once

#include <string>

#define PROTOCOL_SOMO_UDP       1
#define PROTOCOL_RTMP           2

class Server;
class Connection;
struct ISNLink;
class Protocol {
public:
    Protocol(int protocol, const std::string& name);
    ~Protocol();

public:
    virtual Server*     create_server() = 0;
    virtual Connection* create_connection(ISNLink* link) = 0;

public:
    int protocol() { return m_nProtocol; }
    std::string name() { return m_strName; }

private:
    int         m_nProtocol;
    std::string m_strName;
};

std::string     protocol_get_name(int protcol);
