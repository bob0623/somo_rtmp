#pragma once

#include <stdint.h>
#include <netinet/in.h>

#define RTP_HEADER_SIZE	12
#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)
struct rtp_header
{
#if __BYTE_ORDER == __BIG_ENDIAN
	uint16_t version:2;
	uint16_t padding:1;
	uint16_t extension:1;
	uint16_t csrccount:4;
	uint16_t markerbit:1;
	uint16_t type:7;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	uint16_t csrccount:4;
	uint16_t extension:1;
	uint16_t padding:1;
	uint16_t version:2;
	uint16_t type:7;
	uint16_t markerbit:1;
#endif
	uint16_t seq_number;
	uint32_t timestamp;
	uint32_t ssrc;
	uint32_t csrc[1];
};

typedef struct rtp_header_extension {
	uint16_t type;
	uint16_t length;
} rtp_header_extension;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

class RtpPacket {
public:
    RtpPacket();
    RtpPacket(bool video, const char* data, int len);
    ~RtpPacket();

public:
    uint32_t    getSSRC();
    void        setSSRC(uint32_t ssrc);

    uint16_t    getCSRCCount();
    void        setCSRCCount(int count);

    uint16_t    getPayloadType();
    void        setPayloadType(int type);

    uint16_t    getSeq();
    void        setSeq(uint16_t seq);

    uint32_t    getStamp();
    void        setStamp(uint32_t stamp);

    bool        isVideoFrameEnd();
    void        setVideoFrameEnd(bool end);

    bool        hasExtHeader();
    void        setExtHeader(bool ext);

    int         getExtLength();
    void        setExtLength(int len);

    id_t        getLen() { return m_nLen; }

    char*       getPayload();
    void        setPayload(const char* data, int len);

    int         getPayloadLength();
    int         getPayloadOffset();
    void        dump();

public:
    static bool is_rtp(char *buf, uint32_t len);

private:
    rtp_header* m_pHeader;
    rtp_header_extension*   m_pExtension;
    char*       m_pData;
    int         m_nCapacity;
    int         m_nLen;
    bool        m_bVideo;
};


