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
    virtual ~RtmpClient();

public:
    virtual void on_connected(ISNLink* pLink);
    virtual void on_close(ISNLink* pLink);

public:
    virtual void    on_meta_data(const char* data, int len);
    virtual void    on_video_rtmp(const char* data, int len);
    virtual void    on_video_rtmp_sh(const char* data, int len);

protected:
    std::string     m_strApp;
    std::string     m_strStream;
    uint32_t        m_nVideoFrames;
};

class RtmpForwarder : public RtmpClient {
public:
    RtmpForwarder(const std::string& url);
    ~RtmpForwarder();
};



