#include "rtmpbuffer.h"
#include "rtmpformat.h"

#include "common/buffer.h"
#include "common/logger.h"
#include <string.h>


#define     __CLASS__   "RtmpBuffer"
#define     RTMP_BUFFER_MAX     1024*1024

RtmpChunkBuffer::RtmpChunkBuffer(const char* data, int len)
: m_nCid(0)
, m_nFmt(0)
, m_nBasicHeaderLen(0)
, m_nMsgHeaderLen(0)
, m_nPayloadLen(0)
, m_nMsgLen(0)
, m_nMsgType(0)
, m_nStamp(0)
{
    m_pBuffer = new char[len];
    memcpy(m_pBuffer, data, len);
    m_nLen = len;
}

RtmpChunkBuffer::~RtmpChunkBuffer() {
    delete m_pBuffer;
}

void    RtmpChunkBuffer::init(int fmt, int cid, int basic_header_int, int msg_header_len, int payload_len, int msg_len, int msg_type, uint32_t stamp) {
    m_nFmt = fmt;
    m_nCid = cid;
    m_nBasicHeaderLen = basic_header_int;
    m_nMsgHeaderLen = msg_header_len;
    m_nPayloadLen = payload_len;
    m_nMsgLen = msg_len;
    m_nMsgType = msg_type;
    m_nStamp = stamp;
}

RtmpMsgBuffer::RtmpMsgBuffer() 
: m_pBuffer(NULL)
, m_nLen(0)
{
}

RtmpMsgBuffer::~RtmpMsgBuffer() {
    if( m_pBuffer ) {
        delete m_pBuffer;
        m_pBuffer = NULL;
    }

    for( auto it = m_arrChunks.begin(); it != m_arrChunks.end(); it++ ) {
        delete (*it);
    }
}

void    RtmpMsgBuffer::add_chunk(RtmpChunkBuffer* chunk_buf) {
    m_arrChunks.push_back(chunk_buf);

    if( m_pBuffer == NULL ) {
        m_pBuffer = new char[ msg_len() ];
    }
    memcpy(m_pBuffer+m_nLen, chunk_buf->payload(), chunk_buf->payload_len());
    m_nLen += chunk_buf->payload_len();

    //FUNLOG(Info, "rtmp msg buffer add chunk, msg_len=%d, m_nLen=%d, payload_len=%d, left=%d", msg_len(), m_nLen, chunk_buf->payload_len(), left_len());
}

void    RtmpMsgBuffer::dump_ready() {
    FUNLOG(Info, "rtmp msg buffer ready! cid=%d, msg_len=%d, msg_type=%d", cid(), msg_len(), msg_type());
}

int     RtmpMsgBuffer::fmt() {
    if( m_arrChunks.size() == 0 ) {
        return 0;
    }

    return m_arrChunks[0]->fmt();

}

int     RtmpMsgBuffer::cid() {
    if( m_arrChunks.size() == 0 ) {
        return 0;
    }

    return m_arrChunks[0]->cid();
}

int     RtmpMsgBuffer::msg_len() {
    if( m_arrChunks.size() == 0 ) {
        return 0;
    }

    return m_arrChunks[0]->msg_len();
}

int     RtmpMsgBuffer::msg_type() {
    if( m_arrChunks.size() == 0 ) {
        return 0;
    }

    return m_arrChunks[0]->msg_type();
}

uint32_t RtmpMsgBuffer::stamp() {
    if( m_arrChunks.size() == 0 ) {
        return 0;
    }

    return m_arrChunks[0]->stamp();
}

int     RtmpMsgBuffer::left_len() {
    int ready_len = 0;
    for( auto it = m_arrChunks.begin(); it != m_arrChunks.end(); it++ ) {
        ready_len += (*it)->payload_len();
    }

    if( ready_len > msg_len() ) {
        FUNLOG(Error, "rtmp msg buffer left len<0, msg_len=%d, ready_len=%d", msg_len(), ready_len);
        return 0;
    }

    return msg_len()-ready_len;
}

bool     RtmpMsgBuffer::ready() {
    int ready_len = 0;
    for( auto it = m_arrChunks.begin(); it != m_arrChunks.end(); it++ ) {
        ready_len += (*it)->payload_len();
    }

    if( ready_len==msg_len() ) {
        return true;
    }

    return false;
}

RtmpBuffer::RtmpBuffer() 
: m_nChunkSize(128)
, m_pCurMsg(NULL)
, m_nLastLen(0)
, m_nLastType(0)
{
    m_nCapacity = RTMP_BUFFER_MAX;
    m_pBuffer = new char[m_nCapacity];
    m_nLen = 0;
    m_nPos = 0;
}

RtmpBuffer::~RtmpBuffer() {
    delete m_pBuffer;
}

void    RtmpBuffer::set_chunk_size(uint32_t chunk_size) {
    FUNLOG(Info, "rtmp buffer set chunk_size=%d", chunk_size);
    m_nChunkSize = chunk_size;
}

void    RtmpBuffer::push(const char* data, int len) {
    //FUNLOG(Info, "rtmp buffer push, len=%d", len);
    if( m_nPos + m_nLen + len > m_nCapacity ) {
        //buffe not large enough, extend it
        if( m_nCapacity >= RTMP_BUFFER_MAX ) {
            FUNLOG(Error, "rtmp buffer push failed! m_nCapacity>=RTMP_BUFFER_MAX, m_nLen=%d, m_nPos=%d, len=%d", m_nLen, m_nPos, len);
            clear();
            return;
        }

        m_nCapacity = m_nPos + m_nLen + len;
        char* new_buf = new char[m_nCapacity];
        memcpy(new_buf, m_pBuffer+m_nPos, m_nLen);
        delete m_pBuffer;
        m_pBuffer = new_buf;
    }

    memcpy(m_pBuffer+m_nPos+m_nLen, data, len);
    m_nLen += len;

    int count = 0;
    while( parse() ) {
        count++;
        //if we got SET_CHUNK_SIZE, break here:
        //if we got SET_CHUNK_SIZE, break here:
            if( has_set_chunk_size_msg() ) {
                FUNLOG(Info, "rtmp buffer push, break parse for SET_CHUNK_SIZE, msg.count=%d", m_arrMsgs.size());
                break;
            }

        //in case infinit loop:
        if( count >= 100 ) 
            break;
    }

    shuffer();
}

RtmpMsgBuffer*    RtmpBuffer::get_msg_buf() {
    if( m_arrMsgs.empty() ) {
        //parse again, maybe more data:
        int count = 0;
        while( parse() ) {
            count++;

            //if we got SET_CHUNK_SIZE, break here:
            if( has_set_chunk_size_msg() ) {
                FUNLOG(Info, "rtmp buffer get msg buf, break parse for SET_CHUNK_SIZE, msg.count=%d", m_arrMsgs.size());
                break;
            }

            //in case infinit loop:
            if( count >= 100 ) 
                break;
        }
    }
    
    if( m_arrMsgs.empty() )
        return NULL;

    auto it = m_arrMsgs.front();
    if( !it->ready() )
        return NULL;
    m_arrMsgs.pop_front();

    return it;
}

void    RtmpBuffer::clear() {
    m_nPos = 0;
    m_nLen = 0;
}

bool    RtmpBuffer::parse() {
    if( m_nLen <= 7 ) {
        //at least 7 bytes:
        return false;
    }

    char fmt = 0;
    int cid = 0;
    int basic_header_len = 1;
    int chunk_header_len = 11;
    uint8_t b1 = 0;
    uint8_t b2 = 0;
    uint8_t b3 = 0;

    const char* temp = m_pBuffer+m_nPos;
    int len = m_nLen;

    IOBuffer buf(const_cast<char*>(temp), len);
    b1 = buf.read_1bytes();
    cid = b1&0x3f;
    fmt = (b1>>6)&0x03;

    if( cid > 1 ) {
        //FUNLOG(Info, "cid>1, cid=%d", cid);
        //1 byte header
        //cid = b1;
        
    } else if( cid == 0 ) {
        //FUNLOG(Info, "cid==0, cid=%d", cid);
        b2 = buf.read_1bytes();
        cid = b2+64;
        basic_header_len = 2;
    } else if( cid == 1) {
        //FUNLOG(Info, "cid==1, cid=%d", cid);
        b2 = buf.read_1bytes();
        b3 = buf.read_1bytes();
        cid = b3*256+b2+64;
        basic_header_len = 3;
    } else {
        FUNLOG(Error, "rtmp buffer parse failed! fmt=%d, cid<0, cid=%d, total_len=%d", fmt, cid, len);
        clear();
        return false;
    }

    uint32_t stamp = 0;
    uint32_t msg_len = 0;
    uint32_t msg_type = 0;
    uint32_t msg_stream = 0;
    uint32_t stamp_ext = 0;

    if( fmt == 0 ) {
        stamp = buf.read_3bytes();
        msg_len = buf.read_3bytes();
        msg_type = buf.read_1bytes();
        msg_stream = buf.read_4bytes();
        chunk_header_len = 11;

        //save last chunk_len&type:
        m_nLastLen = msg_len;
        m_nLastType = msg_type;
    } else if( fmt == 1 ) {
        stamp = buf.read_3bytes();
        msg_len = buf.read_3bytes();
        msg_type = buf.read_1bytes();
        chunk_header_len = 7;

        //save last chunk_len&type:
        m_nLastLen = msg_len;
        m_nLastType = msg_type;

    } else if( fmt == 2 ) {
        stamp = buf.read_3bytes();
        chunk_header_len = 3;
    } else if( fmt == 3 ) {
        chunk_header_len = 0;
    }
    if( stamp == 0xFFFFFF ) {
        stamp_ext = buf.read_4bytes();
        chunk_header_len += 4;
    }

    //FUNLOG(Info, "rtmp buffer parse, fmt=%d, cid=%d, header.size=%d, b1=0x%x, chunk.header.len=%d, msg_len=%d, msg_type=%d, total_len=%d", fmt, cid, basic_header_len, b1, chunk_header_len, msg_len, msg_type, len);
    if( fmt == 0 || fmt == 1 ) {
        if( m_pCurMsg != NULL ) {
            FUNLOG(Info, "rtmp buffer parse, fmt==0||fmt==1, m_pCurMsg!=NULL, fmt=%d, cid=%d, header.size=%d, b1=0x%x, chunk.header.len=%d, msg_len=%d, total_len=%d", fmt, cid, basic_header_len, b1, chunk_header_len, msg_len, len);
            delete m_pCurMsg;
        }

        //calc this chunk size:
        int payload_len = msg_len;
        int chunk_total_len = basic_header_len+chunk_header_len+msg_len;
        if( m_nChunkSize != 0 && msg_len>m_nChunkSize ) {
            payload_len = m_nChunkSize;
            chunk_total_len = basic_header_len+chunk_header_len+m_nChunkSize;
            //FUNLOG(Info, "rtmp buffer parse, msg_len>chunk_size, msg_len=%d, chunk_total_len=%d, m_nChunkSize=%d", msg_len, chunk_total_len, m_nChunkSize);
        }

        if( m_nLen >= chunk_total_len ) {
            m_pCurMsg = new RtmpMsgBuffer();

            RtmpChunkBuffer* chunk_buf = new RtmpChunkBuffer(m_pBuffer+m_nPos, chunk_total_len);
            chunk_buf->init(fmt, cid, basic_header_len, chunk_header_len, payload_len, msg_len, msg_type, stamp);
            m_pCurMsg->add_chunk(chunk_buf);

            if( m_pCurMsg->ready() ) {
                //m_pCurMsg->dump_ready();
                m_arrMsgs.push_back(m_pCurMsg);
                m_pCurMsg = NULL;
            } else {
                FUNLOG(Warn, "rtmp buffer parse, not ready for cid=%d, msg_len=%d, len=%d", m_pCurMsg->cid(), m_pCurMsg->msg_len(), m_pCurMsg->len());
            }

            m_nLen -= chunk_total_len;
            m_nPos += chunk_total_len;

            return true;
        }
    } else if( fmt == 2 ) {
        if( m_pCurMsg != NULL ) {
            FUNLOG(Info, "rtmp buffer parse, fmt==2, m_pCurMsg!=NULL, fmt=%d, cid=%d, header.size=%d, b1=0x%x, chunk.header.len=%d, msg_len=%d, total_len=%d", fmt, cid, basic_header_len, b1, chunk_header_len, msg_len, len);
            //delete m_pCurMsg;
        }

        msg_len = m_nLastLen;
        msg_type = m_nLastType;

        //calc the chunk size
        int payload_len = msg_len;
        int chunk_total_len = basic_header_len+chunk_header_len+msg_len;
        if( m_nLen >= chunk_total_len ) {
            m_pCurMsg = new RtmpMsgBuffer();
            
            RtmpChunkBuffer* chunk_buf = new RtmpChunkBuffer(m_pBuffer+m_nPos, chunk_total_len);
            chunk_buf->init(fmt, cid, basic_header_len, chunk_header_len, payload_len, msg_len, msg_type, stamp);
            m_pCurMsg->add_chunk(chunk_buf);

            if( m_pCurMsg->ready() ) {
                //m_pCurMsg->dump_ready();
                m_arrMsgs.push_back(m_pCurMsg);
                m_pCurMsg = NULL;
            }

            m_nLen -= chunk_total_len;
            m_nPos += chunk_total_len;

            return true;
        }
    } else if ( fmt == 3 ) {
        if( m_pCurMsg == NULL ) {
            FUNLOG(Error, "rtmp buffer parse, fmt==3 but m_pCurChunk==NULL! fmt=%d, cid=%d, header.size=%d, b1=0x%x, chunk.header.len=%d, msg_len=%d, total_len=%d", fmt, cid, basic_header_len, b1, chunk_header_len, msg_len, len);
            clear();
            return true;
        }
        int left = m_pCurMsg->left_len();
        int payload_len = left;
        int chunk_total_len = basic_header_len+left;
        if( left >= m_nChunkSize ) {
            payload_len = m_nChunkSize;
            chunk_total_len = basic_header_len + payload_len;
            //FUNLOG(Info, "rtmp buffer parse, fmt==3, left>chunk_size, chunk_total_len=%d, payload_len=%d, left=%d", chunk_total_len, payload_len, left);
        } else {
            //FUNLOG(Info, "rtmp buffer parse, fmt==3, left<chunk_size, chunk_total_len=%d, payload_len=%d, left=%d", chunk_total_len, payload_len, left);
        }

        if( m_nLen >= chunk_total_len ) {
            RtmpChunkBuffer* chunk_buf = new RtmpChunkBuffer(m_pBuffer+m_nPos, chunk_total_len);
            chunk_buf->init(fmt, cid, basic_header_len, chunk_header_len, payload_len, 0, 0, stamp);
            m_pCurMsg->add_chunk(chunk_buf);

            if( m_pCurMsg->ready() ) {
                m_pCurMsg->dump_ready();
                m_arrMsgs.push_back(m_pCurMsg);
                m_pCurMsg = NULL;
            }

            m_nLen -= chunk_total_len;
            m_nPos += chunk_total_len;

            return true;
        }
    }

    return false;
}

void    RtmpBuffer::shuffer() {
    if( m_nPos >= m_nLen ) {
        memcpy(m_pBuffer, m_pBuffer+m_nPos, m_nLen);
        m_nPos = 0;
    }
}

bool    RtmpBuffer::has_set_chunk_size_msg() {
    for( auto it=m_arrMsgs.begin(); it!=m_arrMsgs.end(); it++ ) {
        RtmpMsgBuffer* msg = *it;
        if( msg->msg_type() == RTMP_MSG_SET_CHUNK_SIZE ) 
            return true;
    }

    return false;
}
