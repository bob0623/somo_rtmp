#pragma once

#include <stdint.h>

class FileWriter;
class RtmpPusher;
class RtpPacket;
class RtpH264FrameBuilder;
class RtcpPacket;
class RtpSession {
public:
    RtpSession(uint32_t ssrc);
    ~RtpSession();

public:
    void    addVideoPacket(RtpPacket* packet);
    void    addRtcpPacket(RtcpPacket* packet);
    void    handleFrames();
    bool    isExpire(uint64_t stamp);

public:
    uint32_t    getSSRC() { return m_nSSRC; }

private:
    uint32_t    m_nSSRC;
    RtpH264FrameBuilder*    m_pBuilder;
    FileWriter* m_pWriter;
    RtmpPusher* m_pRtmp;
    bool        m_bFirstIFrame;
    uint64_t    m_nLastStamp;
    uint32_t    m_nPackets;
};

