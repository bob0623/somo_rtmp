#include "rtmpshakehands.h"
#include "rtmpconnection.h"
#include "common/buffer.h"
#include "common/util.h"
#include "common/logger.h"

#include <string.h>

#define __CLASS__   "RtmpShakeHands"

RtmpShakeHands_Server::RtmpShakeHands_Server(RtmpConnection* conn)
: m_pRtmpConn(conn)
, m_bReadC0C1(false)
, m_bReadC2(false)
{
   m_pC0C1 = m_pS0S1S2 = m_pC2 = NULL;
    proxy_real_ip = 0;
}

RtmpShakeHands_Server::~RtmpShakeHands_Server()
{
    if( m_pC0C1 )
        delete m_pC0C1;
    if( m_pS0S1S2 ) 
        delete m_pS0S1S2;
    if( m_pC2 )
        delete m_pC2;
}

int  RtmpShakeHands_Server::on_data(const char* data, int len) {
    if( !m_bReadC0C1 ) {
        if( len >= 1537 ) {
            FUNLOG(Info, "rtmp shake hands server, get C0&C1, len=%d, b[0]=%d", len, data[0]);
            m_bReadC0C1 = true;
            read_c0c1(data, len);
            create_s0s1s2();
            m_pRtmpConn->send(m_pS0S1S2, 3073);

            return 1537;
        } else {
            FUNLOG(Error, "rtmp shake hands server, C0&C1 lens!=1537, len=%d", len);
            return 0;
        }
    } else if( !m_bReadC2 ) {
        FUNLOG(Info, "rtmp shake hands server, get C2, len=%d", len);
        if( len >= 1536 ) {
            m_bReadC2 = true;
            return 1536;
        } else {
            FUNLOG(Error, "rtmp shake hands server, C2 lens!=1536, len=%d", len);
            return 0;
        }
    } else {
        FUNLOG(Error, "rtmp shake hands server failed, already read C0&C1&C2, len=%d", len);
        return 0;
    }
}

bool RtmpShakeHands_Server::done() {
    return m_bReadC0C1&&m_bReadC2;
}

bool RtmpShakeHands_Server::read_c0c1(const char* data, int len) {
    if (m_pC0C1) {
        return false;
    }
    if( len != 1537 ) {
        FUNLOG(Error, "rtmp shake hands failed, c0c1.len!=1537, len=%d", len);
        return false;
    }
    
    m_pC0C1 = new char[1537];
    if (uint8_t(data[0]) != 0x3) {
        FUNLOG(Error, "rtmp shake hands failed, byte[0]!=0x3, len=%d", len);
        return false;
    }
    memcpy(m_pC0C1, data, len);
    
    return true;
}

bool RtmpShakeHands_Server::read_c2(const char* data, int len){    
    if (m_pC2) {
        return false;
    }
    
    ssize_t nsize;
    
    m_pC2 = new char[1536];
    memcpy(m_pC2, data, len);
    
    return true;
}

bool RtmpShakeHands_Server::create_s0s1s2()
{
    if (m_pS0S1S2) {
        return true;
    }
    
    m_pS0S1S2 = new char[3073];
    Util::generate_random(m_pS0S1S2, 3073);
    
    // plain text required.
    IOBuffer stream(m_pS0S1S2, 9);
    
    stream.write_1bytes(0x03);
    stream.write_4bytes((int32_t)::time(NULL));
    //// s1 time2 copy from c1
    if (m_pC0C1) {
        stream.write_bytes(m_pC0C1 + 1, 4);
    }
    
    // if c1 specified, copy c1 to s2.
    // @see: https://github.com/ossrs/srs/issues/46
    if (m_pC0C1) {
        memcpy(m_pS0S1S2 + 1537, m_pC0C1+1, 1536);
    }
    
    return true;
}

//client:
RtmpShakeHands_Client::RtmpShakeHands_Client(RtmpConnection* conn)
: m_pRtmpConn(conn)
, m_bReadS0S1S2(false)
{
   m_pC0C1 = m_pS0S1S2 = m_pC2 = NULL;
}

RtmpShakeHands_Client::~RtmpShakeHands_Client()
{
    if( m_pC0C1 )
        delete m_pC0C1;
    if( m_pS0S1S2 ) 
        delete m_pS0S1S2;
    if( m_pC2 )
        delete m_pC2;
}

void    RtmpShakeHands_Client::start() {
    create_c0c1();
    m_pRtmpConn->send(m_pC0C1, 1537);
}

int  RtmpShakeHands_Client::on_data(const char* data, int len) {
    if( len >= 3073 ) {
        FUNLOG(Info, "rtmp shake hands client, get S0S1S2, len=%d, b[0]=%d", len, data[0]);
        m_bReadS0S1S2 = true;
        create_c2();
        m_pRtmpConn->send(m_pC2, 1537);
        return 3073;
    } else {
        FUNLOG(Error, "rtmp shake hands client, C0&C1 lens!=1537, len=%d", len);
        return 0;
    }
}

bool RtmpShakeHands_Client::done() {
    return m_bReadS0S1S2;
}

bool RtmpShakeHands_Client::create_c0c1() {
    if( m_pC0C1 ) {
        return true;
    }

    m_pC0C1 = new char[1537];
    Util::generate_random(m_pC0C1, 1537);
    m_pC0C1[0] = 3;
    m_pC0C1[5] = 0;
    m_pC0C1[6] = 0;
    m_pC0C1[7] = 0;
    m_pC0C1[8] = 0;

    return true;
}

bool RtmpShakeHands_Client::create_c2() {
    if (m_pC2) {
        return false;
    }
    
    ssize_t nsize;
    m_pC2 = new char[1536];
    Util::generate_random(m_pC2, 1536);
    
    return true;
}