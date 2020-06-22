#pragma once

#include <string>

#define RTMP_SESSION_TYPE_UNKNOWN   0
#define RTMP_SESSION_TYPE_PUBLISH   1
#define RTMP_SESSION_TYPE_PLAY      2

class RtmpChunkStream;
class RtmpConnection;
class RtmpBasicMsg;
class RtmpCommandPacket;
class RtmpAudioPacket;
class RtmpVideoPacket;
class RtmpParser;
class RtmpStream {
public:
    RtmpStream(RtmpConnection* conn);
    ~RtmpStream();

public:
    void    on_msg(RtmpBasicMsg* msg);

public:
    uint32_t    in_chunk_size() { return m_nChunkSizeIn; }

private:
    void    on_command(RtmpBasicMsg* msg);
    void    on_audio(RtmpBasicMsg* msg);
    void    on_video(RtmpBasicMsg* msg);

private:
    void    ack_window_ack_size(RtmpChunkStream* chunk_stream, uint32_t size);
    void    ack_set_peer_bandwidth(RtmpChunkStream* chunk_stream, uint32_t bandwidth);
    void    ack_chunk_size(RtmpChunkStream* chunk_stream, uint32_t chunk_size);
    void    ack_connect(RtmpChunkStream* chunk_stream);
    void    ack_release_stream(RtmpChunkStream* chunk_stream, uint32_t tid);
    void    ack_fcpublish(RtmpChunkStream* chunk_stream, uint32_t tid);
    void    ack_create_stream(RtmpChunkStream* chunk_stream, uint32_t tid);
    void    ack_publish(RtmpChunkStream* chunk_stream, uint32_t tid, uint32_t stream_id);
    void    ack_publish_onstatus(RtmpChunkStream* chunk_stream);

private:
    RtmpConnection* m_pConnection;
    RtmpParser*     m_pParser;

    int             m_nType;
    std::string     m_strApp;
    std::string     m_strTcUrl;
    std::string     m_strStream;
    uint32_t        m_nChunkSizeIn;

    //for send buf
    char*           m_pSendBuf;
    int             m_nSendBufCapacity;

    uint64_t        m_nAudioFrames;
    uint64_t        m_nVideoFrames;


};