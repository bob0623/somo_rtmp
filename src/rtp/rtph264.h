#pragma once

#include "rtp.h"
#include <map>
#include <set>

/* h264 payload type */
#define H264_PT_RSV0        0
#define H264_PT_NAT_UNIT_1  1
#define H264_PT_NAT_UNIT_2  2
#define H264_PT_NAT_UNIT_3  3
#define H264_PT_NAT_UNIT_4  4
#define H264_PT_NAT_UNIT_5  5
#define H264_PT_NAT_UNIT_6  6
#define H264_PT_NAT_UNIT_7  7
#define H264_PT_NAT_UNIT_8  8
#define H264_PT_NAT_UNIT_9  9
#define H264_PT_NAT_UNIT_10 10
#define H264_PT_NAT_UNIT_11 11
#define H264_PT_NAT_UNIT_12 12
#define H264_PT_NAT_UNIT_13 13
#define H264_PT_NAT_UNIT_14 14
#define H264_PT_NAT_UNIT_15 15
#define H264_PT_NAT_UNIT_16 16
#define H264_PT_NAT_UNIT_17 17
#define H264_PT_NAT_UNIT_18 18
#define H264_PT_NAT_UNIT_19 19
#define H264_PT_NAT_UNIT_20 20
#define H264_PT_NAT_UNIT_21 21
#define H264_PT_NAT_UNIT_22 22
#define H264_PT_NAT_UNIT_23 23
#define H264_PT_STAPA       24
#define H264_PT_STAPB       25
#define H264_PT_MTAP16      26
#define H264_PT_MTAP24      27
#define H264_PT_FUA         28
#define H264_PT_FUB         29
#define H264_PT_RSV1        30
#define H264_PT_RSV2        31

#pragma pack(push, 1)
typedef struct fua_indicator fua_indicator_t;
struct fua_indicator {
#if __BYTE_ORDER == __BIG_ENDIAN
   uint8_t f:1;
   uint8_t nir:2;
   uint8_t type:5;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
   uint8_t type:5;
   uint8_t nir:2;
   uint8_t f:1;
#endif
};

typedef struct fua_hdr fua_hdr_t;
struct fua_hdr {
#if __BYTE_ORDER == __BIG_ENDIAN
   uint8_t s:1;
   uint8_t e:1;
   uint8_t r:1;
   uint8_t type:5;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
   uint8_t type:5;
   uint8_t r:1;
   uint8_t e:1;
   uint8_t s:1;
#endif
};
#pragma pack(pop)

class RtpH264Frame {
public:
    RtpH264Frame(uint32_t ssrc, int nalu_type);
    ~RtpH264Frame();

public:
    void    setFrameId(uint32_t frame);
    void    addPacket(const char* data, int len);
    char*   getData() { return m_pData; }
    int     getLen() { return m_nLen; }
    uint32_t getFrameId() { return m_nFrameId; }
    uint32_t getSSRC() { return m_nSSRC; }
    int     getNaluType() { return m_nNaluType; }

private:
    uint32_t m_nSSRC;
    int     m_nNaluType;
    uint32_t m_nFrameId;
    char*   m_pData;
    int     m_nCapacity;
    int     m_nLen;
};

class RtpH264FrameBuilder {
public:
    RtpH264FrameBuilder();
    ~RtpH264FrameBuilder();

public:
    void    addPacket(RtpPacket* packet);
    void    buildFrames();
    RtpH264Frame*   getReadyFrame();
    void    getLostSeqs(std::set<uint32_t>& losts, int max);

private:
    int     getRtpH264Type(RtpPacket* packet);
    void    dropLostPackets();
    void    handleNalu(RtpPacket* packet);
    void    handleSTAPA(RtpPacket* packet);
    void    handleFUA(RtpPacket* packet);
    void    makeFrameReady();
    void    trim();
    void    clear();

private:
    uint32_t        m_nLastReadyFrame;
    uint32_t        m_nLastSeq;
    uint32_t        m_nLastReadySeq;
    uint32_t        m_nBuildFrames;
    RtpH264Frame*   m_pCurrentFrame;
    std::map<uint32_t, RtpPacket*>    m_mapPackets;
    std::map<uint32_t, RtpH264Frame*> m_mapReadyFrames;
    std::map<uint32_t, RtpH264Frame*> m_mapFrames;
    std::set<uint32_t>      m_setLosts;
};


