#pragma once

#include "isnet.h"
#include "packet.h"
#include "singleton.h"

class RtcpLinkHandler : public ISNLinkHandler, public Singleton<RtcpLinkHandler>
{
public:
    RtcpLinkHandler();
    virtual ~RtcpLinkHandler();

public:
    virtual int  on_data(const char* data, size_t len, ISNLink* pLink);

};

