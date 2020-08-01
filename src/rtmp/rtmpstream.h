#pragma once

#include <string>

#define RTMP_SESSION_TYPE_UNKNOWN       0
#define RTMP_SESSION_TYPE_PUBLISH       1
#define RTMP_SESSION_TYPE_PLAY          2

#define RTMP_SESSION_STATUS_INIT        0
#define RTMP_SESSION_STATUS_CONNECTING  1
#define RTMP_SESSION_STATUS_PUBLISHING  2
#define RTMP_SESSION_STATUS_PLAYING     3
#define RTMP_SESSION_STATUS_CREATE_STREAM  4
#define RTMP_SESSION_STATUS_RELEASE_STREAM  5
#define RTMP_SESSION_STATUS_READY       10


class RtmpSession;
class RtmpFlvParser;
class RtmpChunkStream;
class RtmpConnection;
class RtmpPublisher;
class RtmpConsumer;
class RtmpMessage;
class RtmpCommandPacket;
class RtmpAudioPacket;
class RtmpVideoPacket;
class RtmpStream {
    friend class RtmpClient;
public:
    RtmpStream(RtmpConnection* conn);
    ~RtmpStream();

public:
    void    on_msg(RtmpMessage* msg);
    void    send_msg(RtmpMessage* msg);
    void    clear();
    bool    is_publisher();
    bool    is_consumer();
    RtmpSession*    session() { return m_pSession; }
    RtmpConnection* connection() { return m_pConnection; }
    RtmpPublisher*  publisher() { return m_pPublisher; }
    RtmpConsumer*   consumer() { return m_pConsumer; }
    uint32_t    in_chunk_size() { return m_nChunkSizeIn; }
    std::string stream() { return m_strStream; }

public:
    void    send_set_chunk_size(RtmpChunkStream* chunk_stream);
    void    send_connect(RtmpChunkStream* chunk_stream);
    void    send_fcpublish(RtmpChunkStream* chunk_stream);
    void    send_publish(RtmpChunkStream* chunk_stream);
    void    send_create_stream(RtmpChunkStream* chunk_stream);
    void    send_release_stream(RtmpChunkStream* chunk_stream);
    void    send_play(RtmpChunkStream* chunk_stream);
    void    send_meta_data(RtmpChunkStream* chunk_stream);
    
    void    ack_window_ack_size(RtmpChunkStream* chunk_stream, uint32_t size);
    void    ack_set_peer_bandwidth(RtmpChunkStream* chunk_stream, uint32_t bandwidth);
    void    ack_chunk_size(RtmpChunkStream* chunk_stream, uint32_t chunk_size);
    void    ack_connect(RtmpChunkStream* chunk_stream);
    void    ack_release_stream(RtmpChunkStream* chunk_stream, uint32_t tid);
    void    ack_fcpublish(RtmpChunkStream* chunk_stream, uint32_t tid);
    void    ack_create_stream(RtmpChunkStream* chunk_stream, uint32_t tid);
    void    ack_publish(RtmpChunkStream* chunk_stream, uint32_t tid, uint32_t stream_id);
    void    ack_publish_onstatus(RtmpChunkStream* chunk_stream);

    void    ack_play(RtmpChunkStream* chunk_stream, uint32_t tid);
    void    ack_stream_begin(RtmpChunkStream* chunk_stream);
    void    ack_ping(RtmpChunkStream* chunk_stream, uint32_t data);

private:
    void    on_command(RtmpMessage* msg);
    void    on_meta_data(RtmpMessage* msg);
    void    on_audio(RtmpMessage* msg);
    void    on_video(RtmpMessage* msg);

private:
    RtmpSession*    m_pSession;
    RtmpConnection* m_pConnection;
    RtmpPublisher*  m_pPublisher;
    RtmpConsumer*   m_pConsumer;
    RtmpFlvParser*  m_pParser;

    int             m_nType;
    std::string     m_strApp;
    std::string     m_strTcUrl;
    std::string     m_strStream;
    uint32_t        m_nChunkSizeIn;
    uint32_t        m_nChunkSizeOut;

    //runtime:
    uint32_t        m_nStatus;
    uint32_t        m_nTid;
    uint32_t        m_nConnectTid;
    uint32_t        m_nFCPublishTid;
    uint32_t        m_nCreateStreamTid;
    uint32_t        m_nReleaseStreamTid;
    uint32_t        m_nPlayTid;

    //for send buf
    char*           m_pSendBuf;
    int             m_nSendBufCapacity;
};