#pragma once

#include "isnet.h"
#include "packet.h"
#include "singleton.h"

class RtpH264FrameBuilder;
class RtpVideoHandler : public ISNLinkHandler, public Singleton<RtpVideoHandler>
{
public:
    RtpVideoHandler();
    virtual ~RtpVideoHandler();

public:
    virtual int  on_data(const char* data, size_t len, ISNLink* pLink);

};

