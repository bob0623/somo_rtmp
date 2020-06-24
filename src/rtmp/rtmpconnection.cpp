#include "rtmpconnection.h"
#include "rtmpshakehands.h"
#include "rtmpstream.h"
#include "rtmpformat.h"
#include "rtmpbuffer.h"
#include "common/buffer.h"
#include "common/util.h"
#include "common/logger.h"

#define __CLASS__ "RtmpConnection"

RtmpConnection::RtmpConnection(ISNLink* link) 
: Connection(link)
, m_bShakeHands(false)
{
    m_nConnectStamp = Util::system_time_msec();
    m_pShakeHands = new RtmpShakeHands(this);
    m_pStream = new RtmpStream(this);
    m_pBuffer = new RtmpBuffer();
}

RtmpConnection::~RtmpConnection() {
    delete m_pShakeHands;
    delete m_pStream;
    delete m_pBuffer;
}

int    RtmpConnection::on_data(const char* data, int len) {
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

int     RtmpConnection::shake_hands(const char* data, int len) {
    int ret = m_pShakeHands->on_data(data, len);
    if( ret == 0 )
        return 0;
    m_bShakeHands = m_pShakeHands->done();
    if( m_bShakeHands ) {
        FUNLOG(Info, "rtmp connection shake hands done! linkid=%d", linkid());
    }

    return ret;
}

void     RtmpConnection::handle_msg(RtmpMsgBuffer* msg_buf) {
    RtmpChunkStream* chunk_stream = get_chunk_stream( msg_buf->cid() );
    //new stream first msg:
    //if( chunk_stream != NULL ) {
    //    FUNLOG(Error, "rtmp connection handle chunk, fmt==0 but chunk_stream exist! cid=%d, total_len=%d", msg_buf->cid(), msg_buf->msg_len());
        //return basic_header_len+chunk_header_len+msg_len;
    //}
    if( chunk_stream == NULL ) {
        chunk_stream = new RtmpChunkStream(msg_buf->cid());
        m_mapStreams[msg_buf->cid()] = chunk_stream;
    }
    //verify the msg type:
    if( !rtmp_is_valid_msg_type(msg_buf->msg_type()) ) {
        FUNLOG(Error, "rtmp connect handle chunk, invalid msg type=%d, fmt=%d, cid=%d", msg_buf->msg_type(), msg_buf->fmt(), msg_buf->cid());
        chunk_stream->release_msg();
        return;
    }

    //FUNLOG(Info, "rtmp connection handle chunk, create msg! fmt==0, type=%d, len=%d", msg_type, msg_len);
    chunk_stream->create_msg(msg_buf->fmt(),  msg_buf->msg_type(),  msg_buf->msg_len(), msg_buf->stamp(), 0);
    chunk_stream->add_payload(msg_buf->data(), msg_buf->msg_len());
    m_pStream->on_msg(chunk_stream->msg());
    chunk_stream->release_msg();
}

RtmpChunkStream*    RtmpConnection::get_chunk_stream(int cid) {
    auto it = m_mapStreams.find(cid);
    if( it == m_mapStreams.end() ) 
        return NULL;

    return it->second;
}