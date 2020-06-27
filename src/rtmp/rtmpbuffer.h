#pragma once

#include <string>
#include <stdint.h>
#include <deque>
#include <vector>

/**
 * RtmpBuffer: parse data from Connection.on_data, and split into rtmp chunks and messages.
 * Input: network data
 * Output: rtmp message buffer (RtmpMsgBuffer).
 * 
 * RtmpChunkBuffer: buffer for a RTMP Chunk.
 * RtmpMsgBuffer: buffer for a RTMP Message.
 * 
 * Real scenario:
 * int    RtmpConnection::on_data(const char* data, int len) {
    if( !m_bShakeHands ) {
        int ret = shake_hands(data, len);
        if( ret == 0 ) {
            FUNLOG(Error, "rtmp connect shake hands failed! linkid=%d, len=%d", linkid(), len);
            return 0;
        } else {
            return ret;
        }
    }
    m_pBuffer->push(data, len);

    while( true ) {
        RtmpMsgBuffer* msg_buf = m_pBuffer->get_msg_buf();
        if( msg_buf != NULL ) {
            handle_msg( msg_buf );
        } else {
            break;
        }
    }

    return len;
}
 */

/**
 * A chunk buffer represent a rtmp chunk.
 * RtmpBuffer receive data from network, and split the network data into RtmpChunkBuffer.
 * RtmpChunkBuffer will be transfer to RtmpBasicMsg, and then do rtmp logic.
 * 
 */
class RtmpChunkBuffer {
public:
    RtmpChunkBuffer(const char* data, int len);
    ~RtmpChunkBuffer();

public:
    void    init(int fmt, int cid, int basic_header_int, int msg_header_len, int payload_len, int msg_len, int msg_type, uint32_t stamp);
    char*   data() { return m_pBuffer; }
    char*   payload() { return m_pBuffer+m_nBasicHeaderLen+m_nMsgHeaderLen; }
    int     len() { return m_nLen; }

    int     fmt() { return m_nFmt; }
    int     cid() { return m_nCid; }
    int     basic_header_len() { return m_nBasicHeaderLen; }
    int     msg_header_len() { return m_nMsgHeaderLen; }
    int     payload_len() { return m_nPayloadLen; }
    int     total_len() { return m_nBasicHeaderLen + m_nMsgHeaderLen + m_nPayloadLen; }
    int     msg_len() { return m_nMsgLen; }
    int     msg_type() { return m_nMsgType; }
    uint32_t stamp() { return m_nStamp; }

private:
    char*   m_pBuffer;
    int     m_nLen;
    int     m_nCapacity;

    int     m_nFmt;
    int     m_nCid;
    int     m_nBasicHeaderLen;
    int     m_nMsgHeaderLen;
    int     m_nPayloadLen;
    int     m_nMsgLen;
    int     m_nMsgType;
    uint32_t m_nStamp;
};

/**
 * A rtmp msg buffer, just msg payload, not include basic_header & msg_header from RtmpChunkBuffer.
 * 
 * The msg buffer refer to VideoTag or AudioTag in FLV format spec.
 */
class RtmpMsgBuffer {
public:
    RtmpMsgBuffer();
    ~RtmpMsgBuffer();

public:
    void    add_chunk(RtmpChunkBuffer* chunk_buf);
    char*   data() { return m_pBuffer; }
    int     len() { return m_nLen; }
    void    dump_ready();

public:
    int     fmt();
    int     cid();
    int     msg_len();
    int     msg_type();
    uint32_t stamp();
    int     left_len();
    bool    ready();

private:
    std::vector<RtmpChunkBuffer*>   m_arrChunks;
    char*   m_pBuffer;
    int     m_nLen;
};

/**
 * RtmpBuffer receive data from Connection.on_data, and convert the data into RtmpMsgBuffer.
 * 
 */
class RtmpBuffer
{
public:
    RtmpBuffer();
    ~RtmpBuffer();

public:
    /**
     * chunk_size is used to composite big messages from chunks, each chunk payload=chunk_size except the last chunk.
     */
    void    set_chunk_size(uint32_t chunk_size);

    /**
     * receive data from Connection, usually called from Connection.on_data.
     */
    void    push(const char* data, int len);

    /**
     * fetch RtmpMsgBuffer, each RtmpMsgBuffer refer to a RtmpMessage.
     */

    RtmpMsgBuffer*    get_msg_buf();
    
    /**
     * clear everything.
     */
    void    clear();

private:
    bool    parse();
    void    shuffer();

public:
    char*   m_pBuffer;
    int     m_nCapacity;
    int     m_nPos;
    int     m_nLen;
    
    std::deque<RtmpMsgBuffer*>    m_arrChunks;
    RtmpMsgBuffer*    m_pCurMsg;
    uint32_t    m_nChunkSize;
    int     m_nLastLen;
    int     m_nLastType;
};