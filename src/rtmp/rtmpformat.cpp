#include "rtmpformat.h"
#include "rtmpamf.h"

#include "video/videoframe.h"

#include "common/buffer.h"
#include "common/logger.h"

#include <string.h>
#include <unistd.h>

#define __CLASS__   "RtmpFormat"

RtmpChunkStream::RtmpChunkStream(int cid) 
: m_nCid(cid)
, m_pMsg(NULL)
{
}

RtmpChunkStream::~RtmpChunkStream() {

}

void    RtmpChunkStream::create_msg(uint32_t fmt, uint32_t type, uint32_t len, uint32_t stamp, uint32_t id) {
    //FUNLOG(Info, "rtmp chunk stream create msg, fmt=%d, type=%d, len=%d, stamp=%d, id=%d", fmt, type, len, stamp, id);
    if( m_pMsg != NULL ) {
        //msg still valid, something wrong!
        FUNLOG(Error, "rtmp chunk stream create msg, msg!=NULL, fmt=%d, type=%d, len=%d", fmt, type, len);
        delete m_pMsg;
    }

    m_pMsg = new RtmpMessage(this, type, len, stamp, id);
    m_pMsg->set_fmt(fmt);
    m_nLastType = m_pMsg->type();
    m_nLastLen = m_pMsg->msg_len();
}

void    RtmpChunkStream::release_msg() {
    if( m_pMsg ) {
        delete m_pMsg; 
        m_pMsg = NULL;
    }
}

void    RtmpChunkStream::add_payload(const char* data, int len) {
    if( m_pMsg == NULL ) {
        //something wrong, no vaid msg:
        FUNLOG(Error, "rtmp chunk stream add payload failed, no valid msg!!!", NULL);
        return;
    }
    m_pMsg->add_payload(data, len);
    //FUNLOG(Info, "rtmp chunk steam add payload, msg_len=%d, readed=%d, len=%d", m_pMsg->msg_len(), m_pMsg->len(), len);
}

/*
void    RtmpChunkStream::add_payload_fmt0(const char* data, int len) {
    if( m_pMsg == NULL ) {
        //something wrong, no vaid msg:
        FUNLOG(Error, "rtmp chunk stream add payload failed, no valid msg!!!", NULL);
        return;
    }
    m_pMsg->add_payload_fmt0(data, len);
}
*/

RtmpPacket*    RtmpPacketFactory::create_packet(uint32_t type) {
    if( type == 0 ) {
        return new RtmpUnknownPacket();
    } else if( type == RTMP_MSG_SET_CHUNK_SIZE ) {
        return new RtmpSetChunkSizePacket();
    } else if( type == RTMP_MSG_USER_CTL ) {
        return new RtmpUserCtlPacket();
    } else if( type == RTMP_MSG_WindowAcknowledgementSize ) {
        return new RtmpWindowAckSizePacket();
    } else if( type == RTMP_MSG_SET_PEER_BANDWIDTH ) {
        return new RtmpSetPeerBandwidthPacket();
    }  else if( type == RTMP_MSG_AUDIO ) {
        return new RtmpAudioPacket();
    }  else if( type == RTMP_MSG_VIDEO ) {
        return new RtmpVideoPacket();
    } else if( type == RTMP_MSG_AMF0DataMessage ) {
        return new RtmpDataMessagePacket();
    } else if( type == RTMP_MSG_AMF0CommandMessage ) {
        return new RtmpCommandPacket();
    } else {
        FUNLOG(Error, "rtmp packet factory create, INVALID type=%d", type);
    }

    return NULL;
}

RtmpMessage* RtmpMessageFactory::create_video_message(RtmpChunkStream* chunk_stream, uint32_t stamp, RtmpVideoPacket* packet) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_VIDEO, packet->len(), stamp, 0);
    msg->add_payload(packet->data(), packet->len());
    return msg;
}

//[TODO] not ready yet!
RtmpMessage* RtmpMessageFactory::create_video_message(RtmpChunkStream* chunk_stream, uint32_t stamp, VideoFrame* frame) {
    RtmpVideoPacket* packet = new RtmpVideoPacket( frame->data(), frame->size() );
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_VIDEO, packet->len(), frame->stamp(), 0);

    return msg;
}


bool rtmp_is_valid_msg_type(int type) {
    switch(type) {
        case RTMP_MSG_SET_CHUNK_SIZE:
        case RTMP_MSG_USER_CTL:
        case RTMP_MSG_WindowAcknowledgementSize:
        case RTMP_MSG_SET_PEER_BANDWIDTH:
        case RTMP_MSG_AUDIO:
        case RTMP_MSG_VIDEO:
        case RTMP_MSG_AMF0DataMessage:
        case RTMP_MSG_AMF0CommandMessage:
            return true;
        default:
            return false;
    }

    return false;
}

bool rtmp_is_av_msg(int type) {
    if( type == RTMP_MSG_AUDIO || type == RTMP_MSG_VIDEO )
        return true;
    
    return false;
}

RtmpMessage::RtmpMessage(RtmpChunkStream* chunk_stream) 
: m_pChunkStream(chunk_stream)
{
    m_header.type = 0;
    m_header.len = 0;
    m_header.stamp = 0;
    m_header.id = 0;

    m_pBuf = NULL;
    m_nLen = 0;
    
    m_pPacket = NULL;
}

RtmpMessage::RtmpMessage(RtmpChunkStream* chunk_stream, uint32_t type)
: m_pChunkStream(chunk_stream)
{
    m_header.type = type;
    m_header.len = 0;
    m_header.stamp = 0;
    m_header.id = 0;

    m_pBuf = NULL;
    m_nLen = 0;
    
    m_pPacket = RtmpPacketFactory::create_packet(type);
}

RtmpMessage::RtmpMessage(RtmpChunkStream* chunk_stream, uint32_t type, uint32_t len) 
: m_pChunkStream(chunk_stream)
{
    m_header.type = type;
    m_header.len = len;
    m_header.stamp = 0;
    m_header.id = 0;

    m_pPacket = RtmpPacketFactory::create_packet(type);
}

RtmpMessage::RtmpMessage(RtmpChunkStream* chunk_stream, uint32_t type, uint32_t len, uint32_t stamp, uint32_t id) 
: m_pChunkStream(chunk_stream)
{
    //FUNLOG(Info, "rtmp basic msg new, type=%d, len=%d", type, len);
    m_header.type = type;
    m_header.len = len;
    m_header.stamp = stamp;
    m_header.id = id;

    m_nFmt = 0;

    m_pBuf = new char[len];
    m_nLen = 0;

    m_pPacket = RtmpPacketFactory::create_packet(type);
}

RtmpMessage::~RtmpMessage() {
    delete m_pBuf;
    if( m_pPacket ) {
        delete m_pPacket;
    }
}

void    RtmpMessage::set_fmt(uint32_t fmt) {
    m_nFmt = fmt;
}

void    RtmpMessage::set_type(uint32_t type) {
    m_header.type = type;
}

void    RtmpMessage::set_msg_len(uint32_t len) {
    m_header.len = len;
}

void    RtmpMessage::set_stamp(uint32_t stamp) {
    m_header.stamp = stamp;
}

void    RtmpMessage::set_stream_id(uint32_t stream_id) {
    m_header.id = stream_id;
}

void    RtmpMessage::add_payload(const char* data, int len) {
    if( m_nLen + len > m_header.len ) {
        //wrong msg length:
        FUNLOG(Error, "rtmp basic msg add payload, msg size invalid! m_header.len=%d, m_nLen=%d, len=%d", m_header.len, m_nLen, len);
        return;
    }

    memcpy(m_pBuf+m_nLen, data, len);
    m_nLen += len;

    if( m_nLen == m_header.len ) {
        //it's ready, decode the packet:
        IOBuffer buf(m_pBuf, m_nLen);
        m_pPacket->decode(&buf);
    }
}

/*
void    RtmpMessage::add_payload_fmt0(const char* data, int len) {
    memcpy(m_pBuf+m_nLen, data, len);
    m_nLen += len;

    if( m_nLen == m_header.len ) {
        //it's ready, decode the packet:
        IOBuffer buf(m_pBuf, m_nLen);
        m_pPacket->decode(&buf);
    }
}
*/

int     RtmpMessage::get_full_data(int fmt, int cid, char* data, int len) {
    int total_len = 0;
    int header_len_pos = 0;
    IOBuffer buf(data, len);
    char b0 = fmt;
    b0 = b0<<6;

    //one byte for basic header:
    if( cid >=2 && cid <= 63 ) {
        b0 |= cid;
        buf.write_1bytes(b0);
        //FUNLOG(Info, "rtmp basic msg get full data, fmt=%d, cid=%d, b0=%d", fmt, cid, b0);

        total_len += 1;
        header_len_pos += 1;
    } else if( cid <= 319 ) {
        b0 |= 0;
        buf.write_1bytes(b0);

        char b1 = cid-64;
        buf.write_1bytes(b1);

        total_len += 2;
        header_len_pos += 2;
    } else {
        b0 |= 1;
        buf.write_1bytes(b0);

        char b1 = (cid)%256-64;
        char b2 = cid/256;
        buf.write_1bytes(b1);
        buf.write_1bytes(b2);

        total_len += 3;
        header_len_pos += 3;
    }

    //msg header:
    if( m_header.type == RTMP_MSG_SET_CHUNK_SIZE ) {
        buf.write_3bytes(m_header.stamp);
        buf.write_3bytes(m_header.len);
        buf.write_1bytes(m_header.type);
        buf.write_4bytes(m_header.id);

        total_len += 11;
        header_len_pos += 3;
    }  else if( m_header.type == RTMP_MSG_USER_CTL ) {
        buf.write_3bytes(m_header.stamp);
        buf.write_3bytes(m_header.len);
        buf.write_1bytes(m_header.type);
        buf.write_4bytes(m_header.id);

        total_len += 11;
        header_len_pos += 3;

    } else if( m_header.type == RTMP_MSG_WindowAcknowledgementSize ) {
        buf.write_3bytes(m_header.stamp);
        buf.write_3bytes(m_header.len);
        buf.write_1bytes(m_header.type);
        buf.write_4bytes(m_header.id);

        total_len += 11;
        header_len_pos += 3;

    } else if( m_header.type == RTMP_MSG_SET_PEER_BANDWIDTH ) {
        buf.write_3bytes(m_header.stamp);
        buf.write_3bytes(m_header.len);
        buf.write_1bytes(m_header.type);
        buf.write_4bytes(m_header.id);

        total_len += 11;
        header_len_pos += 3;

    }  else if( m_header.type == RTMP_MSG_AUDIO ) {
        buf.write_3bytes(m_header.stamp);
        buf.write_3bytes(m_header.len);
        buf.write_1bytes(m_header.type);
        buf.write_4bytes(m_header.id);

        total_len += 11;
        header_len_pos += 3;

    }  else if( m_header.type == RTMP_MSG_VIDEO ) {
        buf.write_3bytes(m_header.stamp);
        buf.write_3bytes(m_header.len);
        buf.write_1bytes(m_header.type);
        //buf.write_4bytes(m_header.id);

        total_len += 7;
        header_len_pos += 3;

    } else if( m_header.type == RTMP_MSG_AMF0DataMessage ) {
        buf.write_3bytes(m_header.stamp);
        buf.write_3bytes(m_header.len);
        buf.write_1bytes(m_header.type);
        buf.write_4bytes(m_header.id);

        total_len += 11;
        header_len_pos += 3;
    }  else if( m_header.type == RTMP_MSG_AMF0CommandMessage ) {
        buf.write_3bytes(m_header.stamp);
        buf.write_3bytes(m_header.len);
        buf.write_1bytes(m_header.type);

        if( fmt == 0 ) {
            buf.write_4bytes(m_header.id);
            total_len += 11;
        } else {
            total_len += 7;
        }

        
        header_len_pos += 3;
    } else {
        FUNLOG(Error, "rtmp basic msg get full data, invalid msg type=%d", m_header.type);
    }

    //payload first, because header need payload.len.
    int payload_len = m_pPacket->encode(&buf);
    total_len += payload_len;

    //fix the RtmpMsgHeader.len
    int pos = buf.pos();
    buf.repos(header_len_pos);
    buf.write_3bytes(payload_len);
    buf.repos(pos);

    //FUNLOG(Info, "rtmp basic msg get full data, payload_len=%d, buf.len=%d-%d-%d", payload_len, data[header_len_pos], data[header_len_pos+1], data[header_len_pos+2]);

    return total_len;
}

void    RtmpMessage::dump() {
    FUNLOG(Info, "rtmp basic msg dump, b0-b1-b2-b3-b4-b5-b6-b7-b8=%x %x %x %x %x %x %x %x", m_pBuf[0], m_pBuf[1], m_pBuf[2], m_pBuf[3], m_pBuf[4], m_pBuf[5], m_pBuf[6], m_pBuf[7]);
}


RtmpPacket::RtmpPacket() {
}

RtmpPacket::~RtmpPacket() {
}

RtmpUnknownPacket::RtmpUnknownPacket() {

}

RtmpUnknownPacket::~RtmpUnknownPacket() {

}

void    RtmpUnknownPacket::decode(IOBuffer* buf) {

}

int     RtmpUnknownPacket::encode(IOBuffer* buf) {
    return 0;
}


RtmpSetChunkSizePacket::RtmpSetChunkSizePacket() 
: m_nChunkSize(0)
{

}

RtmpSetChunkSizePacket::RtmpSetChunkSizePacket(uint32_t chunk_size) 
: m_nChunkSize(chunk_size)
{

}

RtmpSetChunkSizePacket::~RtmpSetChunkSizePacket() {

}

void    RtmpSetChunkSizePacket::decode(IOBuffer* buf) {
    m_nChunkSize = buf->read_4bytes();

    FUNLOG(Info, "rtmp set chunk size msg, chunk_size=%d", m_nChunkSize);
}

int     RtmpSetChunkSizePacket::encode(IOBuffer* buf) {
    buf->write_4bytes(m_nChunkSize);

    return 4;
}

void    RtmpSetChunkSizePacket::set_chunk_size(uint32_t chunk_size) {
    m_nChunkSize = chunk_size;
}

//user controller msg:
RtmpUserCtlPacket::RtmpUserCtlPacket()
: m_nEvent(0)
, m_nData(0)
, m_nExtra(0)
{

}

RtmpUserCtlPacket::RtmpUserCtlPacket(int event, uint32_t data, uint32_t extra)
: m_nEvent(event)
, m_nData(data)
, m_nExtra(extra)
{

}

RtmpUserCtlPacket::~RtmpUserCtlPacket() {

}

void    RtmpUserCtlPacket::decode(IOBuffer* buf) {
    m_nEvent = buf->read_2bytes();
    if( m_nEvent == RTMP_USER_CTL_FMS ) {
        m_nData = buf->read_1bytes();
    } else {
        m_nData = buf->read_4bytes();
    }

    if( m_nEvent == RTMP_USER_CTL_SET_BUFFER_LENGTH ) {
        m_nExtra = buf->read_4bytes();
    }
}

int     RtmpUserCtlPacket::encode(IOBuffer* buf) {
    int len = 0;

    buf->write_2bytes(m_nEvent);
    len += 2;

    if( m_nEvent == RTMP_USER_CTL_FMS ) {
        buf->write_1bytes(m_nData);
        len += 1;
    } else {
        buf->write_4bytes(m_nData);
        len += 4;
    }

    if( m_nEvent == RTMP_USER_CTL_SET_BUFFER_LENGTH ) {
        buf->write_4bytes(m_nExtra);
        len += 4;
    }

    return len;
}

RtmpWindowAckSizePacket::RtmpWindowAckSizePacket() 
{

}

RtmpWindowAckSizePacket::RtmpWindowAckSizePacket(uint32_t window_size) 
: m_nWindowSize(window_size)
{

}

RtmpWindowAckSizePacket::~RtmpWindowAckSizePacket() {

}

void    RtmpWindowAckSizePacket::decode(IOBuffer* buf) {
    m_nWindowSize = buf->read_4bytes();

    //FUNLOG(Info, "rtmp window ack size msg, window_size=%d", m_nWindowSize);
}

int     RtmpWindowAckSizePacket::encode(IOBuffer* buf) {
    buf->write_4bytes(m_nWindowSize);

    return 4;
}

void    RtmpWindowAckSizePacket::set_window_size(uint32_t window_size) {
    m_nWindowSize = window_size;
}

//peer bandwidth
RtmpSetPeerBandwidthPacket::RtmpSetPeerBandwidthPacket() 
: m_nBW(25000000)
{

}

RtmpSetPeerBandwidthPacket::RtmpSetPeerBandwidthPacket(uint32_t bw) 
: m_nBW(bw)
{

}

RtmpSetPeerBandwidthPacket::~RtmpSetPeerBandwidthPacket() {

}

void    RtmpSetPeerBandwidthPacket::decode(IOBuffer* buf) {
    m_nBW = buf->read_4bytes();

    //FUNLOG(Info, "rtmp set peer bandwidth msg, window_size=%d", m_nBW);
}

int     RtmpSetPeerBandwidthPacket::encode(IOBuffer* buf) {
    buf->write_4bytes(m_nBW);
    buf->write_1bytes(2);

    return 5;
}

void    RtmpSetPeerBandwidthPacket::set_peer_bandwidth(uint32_t bw) {
    m_nBW = bw;
}


//onMetaData
RtmpDataMessagePacket::RtmpDataMessagePacket() 
: m_pBuf(NULL)
, m_nLen(0)
{
    m_pMetaData = new RtmpMetaParams();
}

RtmpDataMessagePacket::RtmpDataMessagePacket(const char* data, int len) {
    FUNLOG(Info, "rtmp data message, meta len=%d", len);
    memcpy(m_pBuf, data, len);
    m_nLen = len;

    m_pMetaData = new RtmpMetaParams();
}

RtmpDataMessagePacket::~RtmpDataMessagePacket() {
    delete m_pMetaData;
}

void    RtmpDataMessagePacket::decode(IOBuffer* buf) {
    //backup, because new consumer need this:
    int pos = buf->pos();
    int size = buf->left();
    if( size == 0 )
        return;

    if( m_pBuf == NULL ) {
        m_pBuf = new char[size];
    }
    buf->read_bytes(m_pBuf, size);
    m_nLen = size;
    buf->repos(pos);

    //decode the content:
    std::string strAtName;
    std::string strName;

    rtmp_amf0_read_string(buf, strAtName);
    rtmp_amf0_read_string(buf, strName);

    RtmpAmf0EcmaArray* array = RtmpAmf0Any::ecma_array();
    array->read(buf);

    m_pMetaData->duration = (uint32_t)array->ensure_property_number("duration")->to_number();
    m_pMetaData->filesize = (uint32_t)array->ensure_property_number("fileSize")->to_number();
    m_pMetaData->video_width = (uint32_t)array->ensure_property_number("width")->to_number();
    m_pMetaData->video_height = (uint32_t)array->ensure_property_number("height")->to_number();
    m_pMetaData->video_data_rate = (uint32_t)array->ensure_property_number("videodatarate")->to_number();
    m_pMetaData->video_fps = (uint32_t)array->ensure_property_number("framerate")->to_number();

    m_pMetaData->audio_data_rate = (uint32_t)array->ensure_property_number("audiodatarate")->to_number();
    m_pMetaData->audio_sample_rate = (uint32_t)array->ensure_property_number("audiosamplerate")->to_number();
    m_pMetaData->audio_sample_size = (uint32_t)array->ensure_property_number("audiosamplesize")->to_number();
    m_pMetaData->audio_channels = (uint32_t)array->ensure_property_number("audiochannels")->to_number();

    FUNLOG(Info, "rtmp data packet, width=%d, height=%d", m_pMetaData->video_width, m_pMetaData->video_height);
}
   
int     RtmpDataMessagePacket::encode(IOBuffer* buf) {   
    if( m_pBuf != NULL && m_nLen > 0) {
        buf->write_bytes(m_pBuf, m_nLen);
        return m_nLen;
    } 

    int size = buf->pos();
    rtmp_amf0_write_string(buf, "@setDataFrame");
    rtmp_amf0_write_string(buf, "onMetaData");
    
    RtmpAmf0EcmaArray* array = RtmpAmf0Any::ecma_array();
    array->set("duration", RtmpAmf0Any::number(0) );
    array->set("fileSize", RtmpAmf0Any::number(0) );

    //video
    array->set("width", RtmpAmf0Any::number(m_pMetaData->video_width) );
    array->set("height", RtmpAmf0Any::number(m_pMetaData->video_height) );
    array->set("videocodecid", RtmpAmf0Any::str("avc1") );
    array->set("videodatarate", RtmpAmf0Any::number(m_pMetaData->video_data_rate) );
    array->set("framerate", RtmpAmf0Any::number(m_pMetaData->video_fps) );

    //audio
    array->set("audiocodecid", RtmpAmf0Any::str("mp4a") );
    array->set("audiodatarate", RtmpAmf0Any::number(m_pMetaData->audio_data_rate) );
    array->set("audiosamplerate", RtmpAmf0Any::number(m_pMetaData->audio_sample_rate) );
    array->set("audiosamplesize", RtmpAmf0Any::number(m_pMetaData->audio_sample_size) );
    array->set("audiochannels", RtmpAmf0Any::number(m_pMetaData->audio_channels) );
    array->set("stereo", RtmpAmf0Any::boolean( (m_pMetaData->audio_channels==2) ) );

    //profiles:
    array->set("2.1", RtmpAmf0Any::boolean(false) );
    array->set("3.1", RtmpAmf0Any::boolean(false) );
    array->set("4.0", RtmpAmf0Any::boolean(false) );
    array->set("4.1", RtmpAmf0Any::boolean(false) );
    array->set("5.1", RtmpAmf0Any::boolean(false) );
    array->set("7.1", RtmpAmf0Any::boolean(false) );

    //encoder:
    array->set("encoder", RtmpAmf0Any::str("somo_server_mcu(0.0.1") );

    array->write(buf);

    return buf->pos()-size;
}


//command like connect:
RtmpCommandPacket::RtmpCommandPacket() 
: m_pConnectParams(NULL)
, m_pFCPublishParams(NULL)
, m_pCreateStreamParams(NULL)
, m_pPublishParams(NULL)
{

}

RtmpCommandPacket::~RtmpCommandPacket() {

}

void    RtmpCommandPacket::decode(IOBuffer* buf) {
    rtmp_amf0_read_string(buf, m_strName);
    FUNLOG(Info, "rtmp command msg parse body, name=%s", m_strName.c_str());

    if( m_strName == "connect" ) {
        double value = 0;
        rtmp_amf0_read_number(buf, value);

        RtmpAmf0Object* props = RtmpAmf0Any::object();
        props->read(buf);

        m_pConnectParams = new RtmpConnectParams();
        m_pConnectParams->app = get_amf_prop(props, "app");
        m_pConnectParams->flashVer = get_amf_prop(props, "flashVer");
        m_pConnectParams->type = get_amf_prop(props, "type");
        m_pConnectParams->swfUrl = get_amf_prop(props, "swfUrl");
        m_pConnectParams->tcUrl = get_amf_prop(props, "tcUrl");

        FUNLOG(Info, "rtmp connect msg, app=%s, url=%s", m_pConnectParams->app.c_str(), m_pConnectParams->tcUrl.c_str());
    } else if( m_strName == "releaseStream" ) {
        double value = 0;
        std::string stream;

        rtmp_amf0_read_number(buf, value);
        rtmp_amf0_read_null(buf);
        rtmp_amf0_read_string(buf, stream);

        m_pReleaseStreamParams = new RtmpReleaseStreamParams();
        m_pReleaseStreamParams->tid = (uint32_t)value;
        m_pReleaseStreamParams->stream = stream;

    } else if( m_strName == "FCPublish" ) {
        double value = 0;
        std::string stream;

        rtmp_amf0_read_number(buf, value);
        rtmp_amf0_read_null(buf);
        rtmp_amf0_read_string(buf, stream);

        m_pFCPublishParams = new RtmpFCPublishParams();
        m_pFCPublishParams->tid = (uint32_t)value;
        m_pFCPublishParams->stream = stream;

    } else if( m_strName == "createStream" ) {
        double value = 0;
        std::string stream;

        rtmp_amf0_read_number(buf, value);
        rtmp_amf0_read_null(buf);

        m_pCreateStreamParams = new RtmpCreateStreamParams();
        m_pCreateStreamParams->tid = (uint32_t)value;
    } else if( m_strName == "publish" ) {
        double value = 0;
        std::string stream;
        std::string mode;

        rtmp_amf0_read_number(buf, value);
        rtmp_amf0_read_null(buf);
        rtmp_amf0_read_string(buf, stream);
        rtmp_amf0_read_string(buf, mode);

        m_pPublishParams = new RtmpPublishParams();
        m_pPublishParams->tid = (uint32_t)value;
        m_pPublishParams->stream = stream;
        m_pPublishParams->mode = mode;
    }  else if( m_strName == "play" ) {
        double tid = 0;
        double start = 0;
        double duration = 0;
        std::string stream;

        rtmp_amf0_read_number(buf, tid);
        rtmp_amf0_read_null(buf);
        rtmp_amf0_read_string(buf, stream);
        rtmp_amf0_read_number(buf, start);
        rtmp_amf0_read_number(buf, duration);

        m_pPlayParams = new RtmpPlayParams();
        m_pPlayParams->tid = (uint32_t)tid;
        m_pPlayParams->stream = stream;
        m_pPlayParams->start = start;
        m_pPlayParams->duration = duration;
    }  else if( m_strName == RTMP_AMF0_COMMAND_RESULT ) {
        double tid = 0;

        rtmp_amf0_read_number(buf, tid);
        rtmp_amf0_read_null(buf);
        rtmp_amf0_read_undefined(buf);

        m_pResultParams = new RtmpResultParams();
        m_pResultParams->tid = (uint32_t)tid;
    } else if( m_strName == RTMP_AMF0_COMMAND_ON_STATUS ) {
        double code = 0;

        rtmp_amf0_read_number(buf, code);
        rtmp_amf0_read_null(buf);

        m_pOnStatusParams = new RtmpOnStatusParams();
        m_pOnStatusParams->code = (uint32_t)code;
    } else {
        FUNLOG(Info, "rtmp unknown msg, m_strName=%s", m_strName.c_str());
    }
}

int     RtmpCommandPacket::encode(IOBuffer* buf) {
    int size = buf->pos();
    rtmp_amf0_write_string(buf, m_strName);
    for( auto it=m_arrAmfObjs.begin(); it!=m_arrAmfObjs.end(); it++ ) {
        RtmpAmf0Any* obj = *it;
        obj->write(buf);
    }

    FUNLOG(Info, "rtmp command encode, len=%d", buf->pos()-size);
    return buf->pos()-size;
}

void    RtmpCommandPacket::set_name(const std::string& name) {
    m_strName = name;
}

void    RtmpCommandPacket::add_amf0_object(RtmpAmf0Any* object)
{
    m_arrAmfObjs.push_back(object);
}

std::string     RtmpCommandPacket::get_amf_prop(RtmpAmf0Object* obj, const std::string& prop) {
    RtmpAmf0Any* value= obj->get_property(prop);
    if( value == NULL ) 
        return "";

    return value->to_str();
}

RtmpAudioPacket::RtmpAudioPacket() {

}

RtmpAudioPacket::~RtmpAudioPacket() {

}

void    RtmpAudioPacket::decode(IOBuffer* buf) {
    //FUNLOG(Info, "rtmp audio packet, len=%d", buf->left());
}

int     RtmpAudioPacket::encode(IOBuffer* buf) {
}

//video:
RtmpVideoPacket::RtmpVideoPacket() 
: m_pBuf(NULL)
, m_nLen(0)
{

}

RtmpVideoPacket::RtmpVideoPacket(const char* data, int len) {
    m_nLen = len;
    m_pBuf = new char[m_nLen];
    memcpy(m_pBuf, data, len);
}

RtmpVideoPacket::~RtmpVideoPacket() {
    delete m_pBuf;
}

void    RtmpVideoPacket::decode(IOBuffer* buf) {
    int size = buf->left();
    if( size == 0 )
        return;

    if( m_pBuf == NULL ) {
        m_pBuf = new char[size];
    }
    buf->read_bytes(m_pBuf, size);
    m_nLen = size;

    //FUNLOG(Info, "rtmp video packet decode, size=%d", size);
}

int     RtmpVideoPacket::encode(IOBuffer* buf) {
    buf->write_bytes(m_pBuf, m_nLen);
    //FUNLOG(Info, "rtmp video packet encode, size=%d", m_nLen);

    return m_nLen;
}