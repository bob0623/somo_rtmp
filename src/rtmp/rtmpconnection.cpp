#include "rtmpconnection.h"
#include "rtmpshakehands.h"
#include "rtmpsession.h"
#include "rtmpstream.h"
#include "rtmpformat.h"
#include "rtmpbuffer.h"
#include "common/buffer.h"
#include "common/util.h"
#include "common/logger.h"

#define __CLASS__ "RtmpConnection"

RtmpConnection::RtmpConnection(const std::string& ip, short port, const std::string& path, bool player,ISNLinkHandler* handler)
: Connection(ip, port, handler)
, m_bClient(true)
, m_bPlayer(player)
, m_bShakeHands(false)
, m_pSHClient(NULL)
, m_pSHServer(NULL)
, m_strRtmpPath(path)
, m_nLastDataStamp(0)
{
    FUNLOG(Info, "rtmp connection new, ip=%s, port=%d", ip.c_str(), port);
    if( m_bClient ) {
        m_pSHClient = new RtmpShakeHands_Client(this);
    } else {
        m_pSHServer = new RtmpShakeHands_Server(this);
    }

    m_pStream = new RtmpStream(this);
    m_pBuffer = new RtmpBuffer();

    //chunk stream 2&3:
    m_mapStreams[2] = new RtmpChunkStream(2);
    m_mapStreams[3] = new RtmpChunkStream(3);
}

RtmpConnection::RtmpConnection(ISNLink* link) 
: Connection(link)
, m_bClient(false)
, m_bShakeHands(false)
, m_pSHClient(NULL)
, m_pSHServer(NULL)
{
    m_nConnectStamp = Util::system_time_msec();
    if( m_bClient ) {
        m_pSHClient = new RtmpShakeHands_Client(this);
    } else {
        m_pSHServer = new RtmpShakeHands_Server(this);
    }
    m_pStream = new RtmpStream(this);
    m_pBuffer = new RtmpBuffer();

    //chunk stream 2&3:
    m_mapStreams[2] = new RtmpChunkStream(2);
    m_mapStreams[3] = new RtmpChunkStream(3);
}

RtmpConnection::~RtmpConnection() {
    if( m_pSHClient ) {
        delete m_pSHClient;
    }
    if( m_pSHServer ) {
        delete m_pSHServer;
    }
    delete m_pStream;
    delete m_pBuffer;

    for( auto it=m_mapStreams.begin(); it!=m_mapStreams.end(); it++ ) {
        delete it->second;
    }
    m_mapStreams.clear();
}

int    RtmpConnection::on_data(const char* data, int len) {
    //FUNLOG(Info, "rtmp connection on data, len=%d", len);
    m_nLastDataStamp = Util::system_time_msec();

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

void RtmpConnection::clear() {
    m_pStream->clear();
    m_bShakeHands = false;
    if( m_pSHClient != NULL ) {
        m_pSHClient->clear();
    }
    if( m_pSHServer != NULL ) {
        m_pSHServer->clear();
    }
}

Session* RtmpConnection::session() {
    return m_pStream->session();
}

bool RtmpConnection::is_alive() {
    if( m_bClient ) {
        return true;
    }

    //if connection is in server mode, check if no data for 5 seconds
    if( Util::system_time_msec()-m_nLastDataStamp > 10*1000 && m_nLastDataStamp > 0 ) {
        return false;
    }

    return true;
}

void RtmpConnection::start_shake_hands() {
    FUNLOG(Info, "rtmp connection start client shake hands!", NULL);
    if( m_bClient ) {
        m_pSHClient->start();
    }
}

int     RtmpConnection::shake_hands(const char* data, int len) {
    FUNLOG(Info, "rtmp connect shake hands start, client=%d, len=%d", m_bClient?1:0, len);
    int ret = 0;
    if( m_bClient ) {
        FUNLOG(Info, "rtmp connect client shake hands start, len=%d", len);
        ret = m_pSHClient->on_data(data, len);
        m_bShakeHands = m_pSHClient->done();

        //for client connection, SET_CHUNK_SIZE+CONNECT
        if( m_bShakeHands ) {
            FUNLOG(Info, "rtmp connection client shake hands done! linkid=%d", linkid());
            m_pStream->send_set_chunk_size( get_chunk_stream(2) );
            m_pStream->send_connect( get_chunk_stream(3) );
        }
    } else {
        FUNLOG(Info, "rtmp connect server shake hands start, len=%d", len);
        ret = m_pSHServer->on_data(data, len);
        m_bShakeHands = m_pSHServer->done();
        if( m_bShakeHands ) {
            FUNLOG(Info, "rtmp connection server shake hands done! linkid=%d", linkid());
        }
    }
    if( ret == 0 )
        return 0;

    return ret;
}

void     RtmpConnection::handle_msg(RtmpMsgBuffer* msg_buf) {
    RtmpChunkStream* chunk_stream = get_chunk_stream( msg_buf->cid() );
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
    if( it == m_mapStreams.end() ) {
        FUNLOG(Error, "rtmp connection get chunk stream failed! cid=%d", cid);
        return NULL;
    }

    return it->second;
}