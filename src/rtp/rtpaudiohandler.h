#pragma once

#include "isnet.h"
#include "packet.h"
#include "singleton.h"

class RtpAudioHandler : public ISNLinkHandler, public Singleton<RtpAudioHandler>
{
public:
    RtpAudioHandler();
    virtual ~RtpAudioHandler();

public:
    virtual int  on_data(const char* data, size_t len, ISNLink* pLink);

};

