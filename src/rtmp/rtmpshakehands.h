#pragma once

#include "isnet.h"
#include <stdint.h>

class RtmpConnection;
class RtmpShakeHands
{
public:
    // For RTMP proxy, the real IP.
    uint32_t proxy_real_ip;
    // [1+1536]
    char* c0c1;
    // [1+1536]
    char* s0s1s2;
    // [1536]
    char* c2;
public:
    RtmpShakeHands(RtmpConnection* conn);
    ~RtmpShakeHands();

public:
    int     on_data(const char* data, int len);
    bool    done();

private:
    bool read_c0c1(const char* data, int len);
    bool read_c2(const char* data, int len);
    bool create_s0s1s2();

private:
    RtmpConnection* m_pRtmpConn;
    bool    m_bReadC0C1;
    bool    m_bReadC2;
};