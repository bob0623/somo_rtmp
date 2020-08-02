#pragma once

#include "protocol.h"

struct ISNLink;
class RtpProtocol : public Protocol {
public:
    RtpProtocol();
    ~RtpProtocol();

public:
    virtual Server*     create_server();
    virtual Session*    create_session(const std::string& stream);
    virtual Connection* create_connection(ISNLink* link);
    virtual Connection* create_connection(const std::string& ip, short port, const std::string& path, bool player, ISNLinkHandler* handler);
    virtual Client*     create_client(const std::string& url, bool player);
};
