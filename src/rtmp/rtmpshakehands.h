#pragma once

#include "isnet.h"
#include <stdint.h>

class RtmpConnection;
class RtmpShakeHands_Server
{
public:
    RtmpShakeHands_Server(RtmpConnection* conn);
    ~RtmpShakeHands_Server();

public:
    int     on_data(const char* data, int len);
    bool    done();

private:
    bool    read_c0c1(const char* data, int len);
    bool    read_c2(const char* data, int len);
    bool    create_s0s1s2();

private:
    RtmpConnection* m_pRtmpConn;
    bool    m_bReadC0C1;
    bool    m_bReadC2;
    uint32_t proxy_real_ip;
    char*   m_pC0C1;
    char*   m_pS0S1S2;
    char*   m_pC2;
};

class RtmpShakeHands_Client
{
public:
    RtmpShakeHands_Client(RtmpConnection* conn);
    ~RtmpShakeHands_Client();

public:
    void    start();
    int     on_data(const char* data, int len);
    bool    done();

private:
    bool    read_c0c1(const char* data, int len);
    bool    read_c2(const char* data, int len);
    bool    create_s0s1s2();

    bool    create_c0c1();
    bool    create_c2();

private:
    RtmpConnection* m_pRtmpConn;
    char*   m_pC0C1;
    char*   m_pS0S1S2;
    char*   m_pC2;
    bool    m_bReadS0S1S2;
};

