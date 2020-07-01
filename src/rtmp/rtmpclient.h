#pragma once

#include "isnet.h"
#include "client.h"
#include <string>

class AudioFrame;
class VideoFrame;

struct ISNLink;
class RtmpShakeHands_Client;
class RtmpConnection;
class RtmpClient : public Client {
public:
    RtmpClient(Protocol* protocol, const std::string& url, bool player);
    ~RtmpClient();

public:
    virtual void on_connected(ISNLink* pLink);

protected:
    std::string     m_strApp;
    std::string     m_strStream;
};

