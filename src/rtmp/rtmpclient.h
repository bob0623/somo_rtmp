#pragma once

#include "isnet.h"
#include <string>

class AudioFrame;
class VideoFrame;

struct ISNLink;
class RtmpShakeHands_Client;
class RtmpConnection;
class RtmpClient : public ISNLinkHandler {
public:
    RtmpClient(const std::string& url);
    ~RtmpClient();

public:
    void    start_publish();
    void    stop_publish();

public:
    virtual void on_connected(ISNLink* pLink);
    virtual void on_close(ISNLink* pLink);
    virtual int  on_data(const char* data, size_t len, ISNLink* pLink);

private:
    void    connect();

public:
    std::string     m_strUrl;
    std::string     m_strIp;
    short           m_nPort;
    std::string     m_strApp;
    std::string     m_strStream;

    RtmpConnection* m_pConnection;
};

