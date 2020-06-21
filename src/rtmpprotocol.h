#pragma once

#include "protocol.h"

struct ISNLink;
class RtmpProtocol : public Protocol {
public:
    RtmpProtocol();
    ~RtmpProtocol();

public:
    virtual Server*     create_server();
    virtual Connection* create_connection(ISNLink* link);

};
