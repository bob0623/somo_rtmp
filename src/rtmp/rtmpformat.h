#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#define RTMP_AMF0_COMMAND_CONNECT               "connect"
#define RTMP_AMF0_COMMAND_CREATE_STREAM         "createStream"
#define RTMP_AMF0_COMMAND_CLOSE_STREAM          "closeStream"
#define RTMP_AMF0_COMMAND_PLAY                  "play"
#define RTMP_AMF0_COMMAND_PAUSE                 "pause"
#define RTMP_AMF0_COMMAND_ON_BW_DONE            "onBWDone"
#define RTMP_AMF0_COMMAND_ON_STATUS             "onStatus"
#define RTMP_AMF0_COMMAND_RESULT                "_result"
#define RTMP_AMF0_COMMAND_ERROR                 "_error"
#define RTMP_AMF0_COMMAND_RELEASE_STREAM        "releaseStream"
#define RTMP_AMF0_COMMAND_FC_PUBLISH            "FCPublish"
#define RTMP_AMF0_COMMAND_UNPUBLISH             "FCUnpublish"
#define RTMP_AMF0_COMMAND_PUBLISH               "publish"
#define RTMP_AMF0_DATA_SAMPLE_ACCESS            "|RtmpSampleAccess"


#define RTMP_MSG_SetChunkSize                   0x01
#define RTMP_MSG_AbortMessage                   0x02
#define RTMP_MSG_Acknowledgement                0x03
#define RTMP_MSG_UserControlMessage             0x04
#define RTMP_MSG_WindowAcknowledgementSize      0x05
#define RTMP_MSG_SetPeerBandwidth               0x06
#define RTMP_MSG_EdgeAndOriginServerCommand     0x07
#define RTMP_MSG_AMF3CommandMessage             17 // 0x11
#define RTMP_MSG_AMF0CommandMessage             20 // 0x14
#define RTMP_MSG_AMF0DataMessage                18 // 0x12
#define RTMP_MSG_AMF3DataMessage                15 // 0x0F
#define RTMP_MSG_AMF3SharedObject               16 // 0x10
#define RTMP_MSG_AMF0SharedObject               19 // 0x13
#define RTMP_MSG_AudioMessage                   8 // 0x08
#define RTMP_MSG_VideoMessage                   9 // 0x09
#define RTMP_MSG_AggregateMessage               22 // 0x16

#define RTMP_USER_CTL_STREAM_BEGIN              0
#define RTMP_USER_CTL_STREAM_EOF                1
#define RTMP_USER_CTL_STREAM_DRY                2
#define RTMP_USER_CTL_SET_BUFFER_LENGTH         3
#define RTMP_USER_CTL_STREAM_IS_RECORDED        4
#define RTMP_USER_CTL_PING_REQUEST              6
#define RTMP_USER_CTL_PING_RESPONSE             7
#define RTMP_USER_CTL_FMS                       0x1a

class RtmpAmf0Any;
class RtmpAmf0Object;

struct RtmpChunkHeader {
    uint32_t    fmt;
    uint32_t    cid;
};

struct RtmpMsgHeader {
    uint32_t    type;
    uint32_t    len;
    uint32_t    stamp;
    uint32_t    id;
};

struct RtmpConnectParams {
    std::string     app;
    std::string     type;
    std::string     flashVer;
    std::string     swfUrl;
    std::string     tcUrl;
};

struct RtmpReleaseStreamParams {
    uint32_t        tid;
    std::string     stream;
};

struct RtmpFCPublishParams {
    uint32_t        tid;
    std::string     stream;
};

struct RtmpCreateStreamParams {
    uint32_t        tid;
};

struct RtmpPublishParams {
    uint32_t        tid;
    std::string     stream;
    std::string     mode;
};

struct RtmpPlayParams {
    uint32_t        tid;
    std::string     stream;
    double          start;
    double          duration;
};

class IOBuffer;
class RtmpMessage;
class RtmpCommandMsg;
class RtmpPacket;
class RtmpVideoPacket;
class VideoFrame;
class RtmpChunkStream {
public:
    RtmpChunkStream(int cid);
    ~RtmpChunkStream();

public:
    void    create_msg(uint32_t fmt, uint32_t type, uint32_t len, uint32_t stamp, uint32_t id);
    void    release_msg();
    RtmpMessage*   get_msg() { return m_pMsg; }
    void    add_payload(const char* data, int len);
    //void    add_payload_fmt0(const char* data, int len);
    int     cid() { return m_nCid; }
    int     last_type() { return m_nLastType; }
    int     last_len() { return m_nLastLen; }
    RtmpMessage*   msg() { return m_pMsg; }

private:
    RtmpChunkHeader*m_pChunkHeader;
    RtmpMessage*   m_pMsg;
    int     m_nCid;
    int     m_nLastType;
    int     m_nLastLen;
};

class RtmpPacketFactory {
public:
    static RtmpPacket*    create_packet(uint32_t type);
};

class RtmpMessageFactory {
public:
    static RtmpMessage* create_video_message(RtmpChunkStream* chunk_stream, uint32_t stamp, RtmpVideoPacket* packet);
    static RtmpMessage* create_video_message(RtmpChunkStream* chunk_stream, uint32_t stamp, VideoFrame* frame);
};

bool rtmp_is_valid_msg_type(int type);
bool rtmp_is_av_msg(int type);

class RtmpMessage {
public:
    RtmpMessage(RtmpChunkStream* chunk_stream);
    RtmpMessage(RtmpChunkStream* chunk_stream, uint32_t type);
    RtmpMessage(RtmpChunkStream* chunk_stream, uint32_t type, uint32_t len);
    RtmpMessage(RtmpChunkStream* chunk_stream, uint32_t type, uint32_t len, uint32_t stamp, uint32_t id);
    ~RtmpMessage();

public:
    void    set_fmt(uint32_t fmt);
    void    set_type(uint32_t type);
    void    set_msg_len(uint32_t len);
    void    set_stamp(uint32_t stamp);
    void    set_stream_id(uint32_t stream_id);

    uint32_t fmt() { return m_nFmt; }
    uint32_t type() { return m_header.type; }
    uint32_t msg_len() { return m_header.len; }
    uint32_t stamp() { return m_header.stamp; }
    uint32_t stream_id() { return m_header.id; }

    RtmpChunkStream* chunk_stream() { return m_pChunkStream; }
    RtmpPacket* packet() { return m_pPacket; }

    void    add_payload(const char* data, int len);
    //void    add_payload_fmt0(const char* data, int len);

    /**
     * get payload data, not including headers!
     */
    char*    payload() { return m_pBuf; }
    int      payload_len() { return m_nLen; }

    /**
     * get full data, including headers, this is usually used to send msg on network!
     */
    int     get_full_data(int fmt, int cid, char* data, int len);

    // the message is ready
    int     ready() { return (m_nLen>=m_header.len); }

    int     get_unready_len() { return m_header.len-m_nLen; }

protected:
    void    dump();

protected:
    RtmpChunkStream*m_pChunkStream;
    RtmpMsgHeader   m_header;
    uint32_t        m_nFmt;
    char*           m_pBuf;
    int             m_nLen;
    RtmpPacket*     m_pPacket;
};

/**
 * RtmpPacket is the payload of a RTMP Message, like:
 *  RtmpCommandPacket: refer to the payload of RTMP Message AMF0 Command.
 *  RtmpVideoPacket: refer to the payload of RTMP Message Video Tag.
 * 
 */
class RtmpPacket {
public:
    RtmpPacket();
    virtual ~RtmpPacket();

public:
    virtual void    decode(IOBuffer* buf) = 0;
    virtual int     encode(IOBuffer* buf) = 0;
};

class RtmpUnknownPacket : public RtmpPacket {
public:
    RtmpUnknownPacket();
    virtual ~RtmpUnknownPacket();

public:
    virtual void    decode(IOBuffer* buf);
    virtual int     encode(IOBuffer* buf);
};

class RtmpSetChunkSizePacket : public RtmpPacket {
public:
    RtmpSetChunkSizePacket();
    RtmpSetChunkSizePacket(uint32_t chunk_size);
    ~RtmpSetChunkSizePacket();

public:
    virtual void    decode(IOBuffer* buf);
    virtual int     encode(IOBuffer* buf);

public:
    void        set_chunk_size(uint32_t chunk_size);
    uint32_t    chunk_size() { return m_nChunkSize; }

private:
    uint32_t    m_nChunkSize;
};

//User controller
class RtmpUserCtlPacket : public RtmpPacket {
public:
    RtmpUserCtlPacket();
    RtmpUserCtlPacket(int event, uint32_t data, uint32_t extra);
    ~RtmpUserCtlPacket();

public:
    virtual void    decode(IOBuffer* buf);
    virtual int     encode(IOBuffer* buf);

public:
    void    set_event(int event) { m_nEvent = event; }
    void    set_data(uint32_t data) { m_nData = data; }
    void    set_extra(uint32_t extra) { m_nExtra = extra; }

private:
    int     m_nEvent;
    uint32_t    m_nData;
    uint32_t    m_nExtra;

};

//Window Ack Size
class RtmpWindowAckSizePacket : public RtmpPacket {
public:
    RtmpWindowAckSizePacket();
    RtmpWindowAckSizePacket(uint32_t windows_size);
    ~RtmpWindowAckSizePacket();

public:
    virtual void    decode(IOBuffer* buf);
    virtual int     encode(IOBuffer* buf);
    
public:
    void        set_window_size(uint32_t window_size);
    uint32_t    window_size() { return m_nWindowSize; }

private:
    uint32_t    m_nWindowSize;
};

class RtmpSetPeerBandwidthPacket : public RtmpPacket {
public:
    RtmpSetPeerBandwidthPacket();
    RtmpSetPeerBandwidthPacket(uint32_t bw);
    ~RtmpSetPeerBandwidthPacket();

public:
    virtual void    decode(IOBuffer* buf);
    virtual int     encode(IOBuffer* buf);
    
public:
    void        set_peer_bandwidth(uint32_t bw);
    uint32_t    peer_bandwidth() { return m_nBW; }

private:
    uint32_t    m_nBW;
};

class RtmpDataMessagePacket : public RtmpPacket {
public:
    RtmpDataMessagePacket();
    ~RtmpDataMessagePacket();

public:
    virtual void    decode(IOBuffer* buf);
    virtual int     encode(IOBuffer* buf);

public:


private:
    uint32_t    m_nDuration;
    uint32_t    m_nFileSize;
    uint32_t    m_nVideoWidth;
    uint32_t    m_nVideoHeight;
    uint32_t    m_nVideoCodec;
    uint32_t    m_nVideoDataRate;
    uint32_t    m_nVideoFps;
    uint32_t    m_nAudioCodec;
    uint32_t    m_nAudioDataRate;
    uint32_t    m_nAudioSampleRate;
    uint32_t    m_nAudioSampleSize;
    uint32_t    m_nAudioChannels;
    bool        m_bStereo;
    std::string m_strEncoder;
};

class RtmpCommandPacket : public RtmpPacket {
public:
    RtmpCommandPacket();
    ~RtmpCommandPacket();

public:
    virtual void    decode(IOBuffer* buf);
    virtual int     encode(IOBuffer* buf);

public:
    std::string     name() { return m_strName; }
    void            set_name(const std::string& name);
    void            add_amf0_object(RtmpAmf0Any* object);

    RtmpConnectParams*  connect_packet() { return m_pConnectParams; }
    RtmpReleaseStreamParams* release_stream_packet() { return m_pReleaseStreamParams;}
    RtmpFCPublishParams*fcpublish_packet() { return m_pFCPublishParams; }
    RtmpCreateStreamParams* create_stream_packet() { return m_pCreateStreamParams; }
    RtmpPublishParams*  publish_packet() { return m_pPublishParams; }
    RtmpPlayParams*     play_packet() { return m_pPlayParams; }

private:
    std::string     get_amf_prop(RtmpAmf0Object* obj, const std::string& prop);

private:
    std::string     m_strName;
    RtmpConnectParams*          m_pConnectParams;
    RtmpReleaseStreamParams*    m_pReleaseStreamParams;
    RtmpFCPublishParams*        m_pFCPublishParams;
    RtmpCreateStreamParams*     m_pCreateStreamParams;
    RtmpPublishParams*          m_pPublishParams;
    RtmpPlayParams*             m_pPlayParams;

    std::vector<RtmpAmf0Any*>    m_arrAmfObjs;
};

class RtmpAudioPacket : public RtmpPacket {
public:
    RtmpAudioPacket();
    ~RtmpAudioPacket();

public:
    virtual void    decode(IOBuffer* buf);
    virtual int     encode(IOBuffer* buf);

};

/**
 * RtmpVideoPacket is the payload part of a RTMP message, that is: merge chunks of the same RTMP message.
 * RtmpVideoPacket is indeed a FlvVideoTag object! 
 */
class RtmpVideoPacket : public RtmpPacket {
public:
    RtmpVideoPacket();
    RtmpVideoPacket(const char* data, int len);
    ~RtmpVideoPacket();

public:
    virtual void    decode(IOBuffer* buf);
    virtual int     encode(IOBuffer* buf);

public:
    char*   data() { return m_pBuf; }
    int     len() { return m_nLen; }

private:
    char*   m_pBuf;
    int     m_nLen;
};


