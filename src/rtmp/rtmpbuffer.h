#pragma once

#include <string>
#include <stdint.h>
#include <deque>
#include <vector>

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
    void    init(int fmt, int cid, int basic_header_int, int msg_header_len, int payload_len, int msg_len, int msg_type);
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
};

/**
 * A rtmp msg buffer, just msg payload, not include basic_header & msg_header from RtmpChunkBuffer.
 *  
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
    int     left_len();
    bool    ready();

private:
    std::vector<RtmpChunkBuffer*>   m_arrChunks;
    char*   m_pBuffer;
    int     m_nLen;
};

class RtmpBuffer
{
public:
    RtmpBuffer();
    ~RtmpBuffer();

public:
    void    set_chunk_size(uint32_t chunk_size);
    void    push(const char* data, int len);
    RtmpMsgBuffer*    get_msg_buf();
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
    uint32_t    m_nChunkSize;
    RtmpMsgBuffer*    m_pCurMsg;
    int     m_nLastLen;
    int     m_nLastType;
    bool    m_bFirstMsg;    //first msg maybe SET_CHUNK_SIZE, don't do batch parse on this case.
};