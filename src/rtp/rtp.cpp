#include "rtp.h"

#include <logger.h>
#include <memory.h>
#include <stdlib.h>

#define __CLASS__ "RtpPacket"

RtpPacket::RtpPacket() {
    m_pData = new char[1600];
    m_nCapacity = 1600;
    m_nLen = 0;
    memset(m_pData, 0, m_nCapacity);

    m_pHeader = (rtp_header*)m_pData;
    m_pExtension = NULL;
}

RtpPacket::RtpPacket(bool video, const char* data, int len) {
    if( data == NULL || len > 1500 ) {
        FUNLOG(Error, "rtp packet new with invalid paramater, len=%d", len);
        return;
    }
    m_pData = new char[1600];
    m_nCapacity = 1600;
    m_nLen = len;
    m_bVideo = video;
    memset(m_pData, 0, m_nCapacity);
    memcpy(m_pData, data, len);
    m_pHeader = (rtp_header*)m_pData;
    if( m_pHeader->extension ) {
        m_pExtension = (rtp_header_extension*)(m_pData+12+m_pHeader->csrccount*4);
    }
}

RtpPacket::~RtpPacket() {
    delete m_pData;
}

void        RtpPacket::setSSRC(uint32_t ssrc) {
    m_pHeader->ssrc = htonl(ssrc);
}



uint32_t    RtpPacket::getSSRC() {
    return ntohl(m_pHeader->ssrc);
}

uint16_t    RtpPacket::getCSRCCount() {
    return m_pHeader->csrccount;
}

void        RtpPacket::setCSRCCount(int count) {
    m_pHeader->csrccount = count;
}

uint16_t    RtpPacket::getPayloadType() {
    return m_pHeader->type;
}

void        RtpPacket::setPayloadType(int type) {
    m_pHeader->type = type;
}

uint16_t    RtpPacket::getSeq() {
    return ntohs(m_pHeader->seq_number);
}

void        RtpPacket::setSeq(uint16_t seq) {
    m_pHeader->seq_number = htons(seq);
}

uint32_t    RtpPacket::getStamp() {
    return ntohl(m_pHeader->timestamp);
}

void        RtpPacket::setStamp(uint32_t stamp) {
    m_pHeader->timestamp = htonl(stamp);
}

bool        RtpPacket::isVideoFrameEnd() {
    return (m_pHeader->markerbit==1);
}

void        RtpPacket::setVideoFrameEnd(bool end) {
    m_pHeader->markerbit = end?1:0;
}

bool        RtpPacket::hasExtHeader() {
    return m_pHeader->extension==1;
}

void        RtpPacket::setExtHeader(bool ext) {
    m_pHeader->extension = ext?1:0;
}

int         RtpPacket::getExtLength() {
    if( m_pExtension == NULL )
        return 0;

    return ntohs(m_pExtension->length)*4;
}

void        RtpPacket::setExtLength(int len) {
    m_pExtension->length = htons(len/4);
}

char*       RtpPacket::getPayload() {
    int hlen = 12;
    if( m_pHeader->csrccount > 0 ) {
        hlen += m_pHeader->csrccount*4;
    }

    if( m_pHeader->extension ) {
        rtp_header_extension *ext = (rtp_header_extension*)(m_pData+hlen);
		int extlen = ntohs(ext->length)*4;
		hlen += 4;
		if(m_nLen > (hlen + extlen)) {
			hlen += extlen;
        }
    }

    if( m_nLen <hlen ) {
        return NULL;
    }

    return m_pData+hlen;
}

void        RtpPacket::setPayload(const char* data, int len) {
    int pos = getPayloadOffset();
    memcpy(m_pData+pos, data, len);
    m_nLen = pos+len;
}

int         RtpPacket::getPayloadLength() {
    int hlen = 12;
    if( m_pHeader->csrccount > 0 ) {
        hlen += m_pHeader->csrccount*4;
    }

    if( m_pHeader->extension ) {
        rtp_header_extension *ext = (rtp_header_extension*)(m_pData+hlen);
		int extlen = ntohs(ext->length)*4;
		hlen += 4;
		if(m_nLen > (hlen + extlen)) {
			hlen += extlen;
        }
    }

    if( m_nLen <hlen ) {
        return 0;
    }

    return m_nLen-hlen;
}

int         RtpPacket::getPayloadOffset() {
    int hlen = 12;
    if( m_pHeader->csrccount > 0 ) {
        hlen += m_pHeader->csrccount*4;
    }

    if( m_pHeader->extension ) {
        rtp_header_extension *ext = (rtp_header_extension*)(m_pData+hlen);
		int extlen = ntohs(ext->length)*4;
		hlen += 4;
		if(m_nLen > (hlen + extlen)) {
			hlen += extlen;
        }
    }

    if( m_nLen <hlen ) {
        return 0;
    }

    return hlen;
}

void    RtpPacket::dump() {
    FUNLOG(Info, "rtp %s packet, ssrc=%u, ts=%u, seq=%u, type=%u, len=%d, end=%d, ext=%d, csrcs=%d, version=%d", 
        m_bVideo?"video":"audio", getSSRC(), getStamp(), getSeq(), m_pHeader->type, m_nLen, m_pHeader->markerbit, m_pHeader->extension, m_pHeader->csrccount, m_pHeader->version);
    //FUNLOG(Info, "rtp packet, %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x", m_pData[0], m_pData[1], m_pData[2], m_pData[3], m_pData[4], m_pData[5], m_pData[6], m_pData[7], 
    //    m_pData[8], m_pData[9], m_pData[10], m_pData[11]);
}

bool    RtpPacket::is_rtp(char *buf, uint32_t len) {
	if (len < 12)
		return false;
	rtp_header *header = (rtp_header *)buf;
	return ((header->type < 64) || (header->type >= 96));
}

