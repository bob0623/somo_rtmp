#include "rtmpshakehands.h"
#include "rtmpconnection.h"
#include "common/buffer.h"
#include "common/util.h"
#include "common/logger.h"

#include <string.h>

#define __CLASS__   "RtmpShakeHands"

RtmpShakeHands::RtmpShakeHands(RtmpConnection* conn)
: m_pRtmpConn(conn)
, m_bReadC0C1(false)
, m_bReadC2(false)
{
   c0c1 = s0s1s2 = c2 = NULL;
    proxy_real_ip = 0;
}

RtmpShakeHands::~RtmpShakeHands()
{
}

int  RtmpShakeHands::on_data(const char* data, int len) {
    if( !m_bReadC0C1 ) {
        if( len >= 1537 ) {
            FUNLOG(Info, "rtmp shake hands, get C0&C1, len=%d, b[0]=%d", len, data[0]);
            m_bReadC0C1 = true;
            read_c0c1(data, len);
            create_s0s1s2();
            m_pRtmpConn->send(s0s1s2, 3073);

            return 1537;
        } else {
            FUNLOG(Error, "rtmp shake hands, C0&C1 lens!=1537, len=%d", len);
            return 0;
        }
    } else if( !m_bReadC2 ) {
        FUNLOG(Info, "rtmp shake hands, get C2, len=%d", len);
        if( len >= 1536 ) {
            m_bReadC2 = true;
            return 1536;
        } else {
            FUNLOG(Error, "rtmp shake hands, C2 lens!=1536, len=%d", len);
            return 0;
        }
    } else {
        FUNLOG(Error, "rtmp shake hands failed, already read C0&C1&C2, len=%d", len);
        return 0;
    }
}

bool RtmpShakeHands::done() {
    return m_bReadC0C1&&m_bReadC2;
}

bool RtmpShakeHands::read_c0c1(const char* data, int len) {
    if (c0c1) {
        return false;
    }
    if( len != 1537 ) {
        FUNLOG(Error, "rtmp shake hands failed, c0c1.len!=1537, len=%d", len);
        return false;
    }
    
    c0c1 = new char[1537];
    if (uint8_t(data[0]) != 0x3) {
        FUNLOG(Error, "rtmp shake hands failed, byte[0]!=0x3, len=%d", len);
        return false;
    }
    memcpy(c0c1, data, len);
    
    return true;
}

bool RtmpShakeHands::read_c2(const char* data, int len){    
    if (c2) {
        return false;
    }
    
    ssize_t nsize;
    
    c2 = new char[1536];
    memcpy(c2, data, len);
    
    return true;
}

bool RtmpShakeHands::create_s0s1s2()
{
    if (s0s1s2) {
        return true;
    }
    
    s0s1s2 = new char[3073];
    Util::generate_random(s0s1s2, 3073);
    
    // plain text required.
    IOBuffer stream(s0s1s2, 9);
    
    stream.write_1bytes(0x03);
    stream.write_4bytes((int32_t)::time(NULL));
    //// s1 time2 copy from c1
    if (c0c1) {
        stream.write_bytes(c0c1 + 1, 4);
    }
    
    // if c1 specified, copy c1 to s2.
    // @see: https://github.com/ossrs/srs/issues/46
    if (c0c1) {
        memcpy(s0s1s2 + 1537, c0c1+1, 1536);
    }
    
    return true;
}
