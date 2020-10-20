#include "rtmpstream.h"
#include "rtmpformat.h"
#include "rtmpconnection.h"
#include "rtmppublisher.h"
#include "rtmpconsumer.h"
#include "rtmpsession.h"
#include "rtmpflv.h"
#include "rtmpbuffer.h"
#include "rtmpamf.h"
#include "app.h"
#include "session.h"

#include "video/videoframe.h"
#include "video/videoframepool.h"
#include "audio/audioframe.h"
#include "audio/audioframepool.h"

#include "common/logger.h"
#include <string.h>


#define __CLASS__   "RtmpSession"

RtmpStream::RtmpStream(RtmpConnection* conn) 
: m_pSession(NULL)
, m_pConnection(conn)
, m_nChunkSizeIn(128)
, m_nChunkSizeOut(128*1024)
, m_pPublisher(NULL)
, m_pConsumer(NULL)
, m_nType(RTMP_SESSION_TYPE_UNKNOWN)
, m_nTid(1)
, m_nStatus(RTMP_SESSION_STATUS_INIT)
, m_nPlayTid(0)
, m_bRecvMeta(false)
{
    m_nSendBufCapacity = 1024*128;
    m_pSendBuf = new char[ m_nSendBufCapacity];

    m_pParser = new RtmpFlvParser();

    //for client, parse the app&stream from url
    if( conn->is_client() ) {
        std::string path = conn->path();
        int pos = path.find("/");
        if( pos != -1 ) {
            m_strApp = path.substr(0, pos);
            m_strStream = path.substr(pos+1);
        }
    }
}

RtmpStream::~RtmpStream() {
    FUNLOG(Info,"dealloc rtmpstream, this=%p",this);
    delete m_pSendBuf;
    delete m_pParser;

    if( m_pPublisher ) {
        delete m_pPublisher;
        m_pPublisher = NULL;
    }
    if( m_pConsumer ) {
        delete m_pConsumer;
    }
    FUNLOG(Info,"dealloc rtmpstream complete, this=%p",this);
}

void    RtmpStream::on_msg(RtmpMessage* msg) {
    switch( msg->type() ) {
        case RTMP_MSG_SET_CHUNK_SIZE:
        {
            RtmpSetChunkSizePacket* chunk_msg = (RtmpSetChunkSizePacket*)msg->packet();
            m_nChunkSizeIn = chunk_msg->chunk_size();
            FUNLOG(Info, "rtmp stream set chunk size, chunk_size=%d", m_nChunkSizeIn);
            m_pConnection->buffer()->set_chunk_size(m_nChunkSizeIn);
        }
        break;
        case RTMP_MSG_AMF0DataMessage:
            on_meta_data(msg);
        break;
        case RTMP_MSG_AMF0CommandMessage:   //20
            on_command( msg );
        break;
        case RTMP_MSG_AUDIO:
            on_audio( msg );
            break;
        case RTMP_MSG_VIDEO:
            on_video( msg );
            break;
    }
}

void    RtmpStream::send_msg(RtmpMessage* msg) {
    int total_len = msg->get_full_data(0, 2, m_pSendBuf, m_nSendBufCapacity);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::clear() {
    m_nType = RTMP_SESSION_TYPE_UNKNOWN;
    m_nTid = 1;
    m_nStatus = RTMP_SESSION_STATUS_INIT;
    m_bRecvMeta = false;
    m_nChunkSizeIn = 128;
}

bool    RtmpStream::is_publisher() {
    return m_nType == RTMP_SESSION_TYPE_PUBLISH;
}  
    
bool    RtmpStream::is_consumer() {
    return m_nType == RTMP_SESSION_TYPE_PLAY;
}

void    RtmpStream::on_command(RtmpMessage* msg) {
    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    FUNLOG(Info, "rtmpstream on command: %s", packet->name().c_str());
    if( packet->name() == "connect" ) {

        m_strApp = packet->connect_packet()->app;
        m_strTcUrl = packet->connect_packet()->tcUrl;

        FUNLOG(Info, "rtmp stream command connect, app=%s, tcUrl=%s", m_strApp.c_str(), m_strTcUrl.c_str());
        ack_window_ack_size(msg->chunk_stream(), 2500000);
        ack_set_peer_bandwidth(msg->chunk_stream(), 2500000);
        ack_chunk_size(msg->chunk_stream(), m_nChunkSizeOut);
        ack_connect(msg->chunk_stream());
    } else if( packet->name() == "releaseStream" ) {
        m_strStream = packet->release_stream_packet()->stream;

        uint32_t tid = packet->release_stream_packet()->tid;
        FUNLOG(Info, "rtmp stream command releaseStream, stream=%s, tid=%d", m_strStream.c_str(), tid);
        ack_release_stream(msg->chunk_stream(), tid);

    }  else if( packet->name() == "FCPublish" ) {
        m_strStream = packet->fcpublish_packet()->stream;

        uint32_t tid = packet->fcpublish_packet()->tid;
        FUNLOG(Info, "rtmp stream command FCPublish, stream=%s, tid=%d", m_strStream.c_str(), tid);
        ack_fcpublish(msg->chunk_stream(), tid);

    } else if( packet->name() == "createStream" ) {
        uint32_t tid = packet->create_stream_packet()->tid;
        FUNLOG(Info, "rtmp stream command createStream, tid=%d", tid);
        ack_create_stream(msg->chunk_stream(), tid);

    } else if( packet->name() == "publish" ) {
        uint32_t tid = packet->publish_packet()->tid;
        FUNLOG(Info, "rtmp stream command publish, tid=%d, stream=%s, mode=%s, stream_id=%d", tid, packet->publish_packet()->stream.c_str(), packet->publish_packet()->mode.c_str(), msg->stream_id());
        ack_publish(msg->chunk_stream(), tid, msg->stream_id());
        ack_publish_onstatus(msg->chunk_stream());

        m_nType = RTMP_SESSION_TYPE_PUBLISH;
        m_pPublisher = new RtmpPublisher(this);

        //put this as session publisher:
        //1, verify the stream doesn't exist!
        //2, if stream exist, report fail!
        //3, if stream not exist, create it with App.
        m_pSession = (RtmpSession*)App::Ins()->add_session( packet->publish_packet()->stream, PROTOCOL_RTMP); 
        m_pSession->set_publisher(m_pPublisher);

    } else if( packet->name() == "play" ) {
        uint32_t tid = packet->play_packet()->tid;
        FUNLOG(Info, "rtmp stream command play, stream=%s", packet->play_packet()->stream.c_str());
        ack_stream_begin(msg->chunk_stream());
        ack_play(msg->chunk_stream(), tid);

        m_nType = RTMP_SESSION_TYPE_PLAY;
        m_pConsumer = new RtmpConsumer(this, m_pConnection->linkid());

        //put this as session consumer:
        //1, verify the stream doesn't exist!
        //2, if stream exist, report fail!
        //3, if stream not exist, create it with App.
        m_pSession = (RtmpSession*)App::Ins()->get_session(packet->play_packet()->stream);
        if( m_pSession == NULL ) {
            FUNLOG(Error, "rtmp stream command play, session not exist for stream=%s", packet->play_packet()->stream.c_str());
            return;
        } else {
            m_pSession->add_consumer(m_pConsumer);
        }
    } else if( packet->name() == RTMP_AMF0_COMMAND_RESULT ) {
        //[yunzed] this command only exist when I'm a RTMP client.
        FUNLOG(Info, "rtmp stream command result, status=%d, tid=%d", m_nStatus, packet->result_packet()->tid);
        if( !m_pConnection->is_client()) {
            FUNLOG(Error, "rtmp stream command result only exist as client mode, is_client=%s", m_pConnection->is_client()?"yes":"no");
            return;
        }
        if( m_pConnection->is_player() ) {
            //in player mode, send "PLAY" command.
            if( m_nStatus == RTMP_SESSION_STATUS_PLAYING && m_nPlayTid == packet->result_packet()->tid ) {
                send_play(msg->chunk_stream());
            }
        } else {
            //othersize, send "FCPublish&publish"
            if( m_nStatus == RTMP_SESSION_STATUS_CONNECTING && m_nConnectTid == packet->result_packet()->tid ) {
                send_release_stream(msg->chunk_stream());
            } else if( m_nStatus == RTMP_SESSION_STATUS_RELEASE_STREAM && m_nReleaseStreamTid == packet->result_packet()->tid ) {
                send_fcpublish(msg->chunk_stream());
                send_create_stream(msg->chunk_stream());
            } else if( m_nStatus == RTMP_SESSION_STATUS_CREATE_STREAM && m_nCreateStreamTid == packet->result_packet()->tid ) {
                //send_publish(msg->chunk_stream());
                send_publish(msg->chunk_stream());
            }
        }
    } else if( packet->name() == RTMP_AMF0_COMMAND_ON_STATUS ) {
        FUNLOG(Info, "rtmp stream command onstatus, status=%d, value=%d, level=%s, code=%s, description=%s", 
            m_nStatus, packet->onstatus_packet()->value, packet->onstatus_packet()->level.c_str(), packet->onstatus_packet()->code.c_str(), packet->onstatus_packet()->description.c_str() );
        m_nStatus = RTMP_SESSION_STATUS_READY;
    }
}

void    RtmpStream::on_meta_data(RtmpMessage* msg) {
    RtmpDataMessagePacket* packet = (RtmpDataMessagePacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream on meta data, packet==NULL!!! app=%s, stream=%s", m_strApp.c_str(), m_strStream.c_str());
        return;
    }
    m_meta = *packet->params();
    m_bRecvMeta = true;
    
    FUNLOG(Info, "rtmp stream on meta data, app=%s, stream=%s, video_width=%d, video_height=%d, video_bitrate=%d, video_fps=%d", 
        m_strApp.c_str(), m_strStream.c_str(), m_meta.video_width, m_meta.video_height, m_meta.video_data_rate, m_meta.video_fps);

    int total_len = msg->get_full_data(1, msg->chunk_stream()->cid(), m_pSendBuf, m_nSendBufCapacity);
    m_pPublisher->on_meta_data( m_pSendBuf, total_len );
}

void    RtmpStream::on_audio(RtmpMessage* msg) {
    if( m_nType != RTMP_SESSION_TYPE_PUBLISH ) {
        return;
    }
    int total_len = msg->get_full_data(1, msg->chunk_stream()->cid(), m_pSendBuf, m_nSendBufCapacity);
    FUNLOG(Info, "rtmp stream, on_audio data total_len=%d", total_len);
    m_pPublisher->on_audio_rtmp(m_pSendBuf, total_len);
}

void    RtmpStream::on_video(RtmpMessage* msg) {
    if( m_nType != RTMP_SESSION_TYPE_PUBLISH ) {
        return;
    }

    //call on_video_rtmp for performance, no need to transfer rtmp to video tag and audio tag.
    int total_len = msg->get_full_data(1, msg->chunk_stream()->cid(), m_pSendBuf, m_nSendBufCapacity);
    m_pPublisher->on_video_rtmp( m_pSendBuf, total_len );

    //parse the rtmp payload, which should be VideoTag:
    VideoFrame* frame = VideoFramePool::Ins()->get( msg->payload_len() );
    m_pParser->parse_video_tag( msg->payload(), msg->payload_len(), frame );
    if( m_pParser->is_video_sh() ) {
        m_pPublisher->on_video_rtmp_sh(m_pSendBuf, total_len);
    }
    VideoFramePool::Ins()->free( frame );
}

void    RtmpStream::send_set_chunk_size(RtmpChunkStream* chunk_stream) {
    FUNLOG(Info, "rtmp stream send set chunk size! app=%s, stream=%s, chunk_size=%d", m_strApp.c_str(), m_strStream.c_str(), m_nChunkSizeOut);
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_SET_CHUNK_SIZE, 4);
    RtmpSetChunkSizePacket* packet = (RtmpSetChunkSizePacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream set chunk size, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_chunk_size(m_nChunkSizeOut);

    int total_len = msg->get_full_data(0, 2, m_pSendBuf, m_nSendBufCapacity);
    FUNLOG(Info, "rtmp stream set chunk size, total_len=%d", total_len);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::send_connect(RtmpChunkStream* chunk_stream) {
    FUNLOG(Info, "rtmp stream send connect command! app=%s, stream=%s, tid=%d", m_strApp.c_str(), m_strStream.c_str(), m_nTid);
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_AMF0CommandMessage);
    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream connect, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    m_nConnectTid = m_nTid++;
    m_nStatus = RTMP_SESSION_STATUS_CONNECTING;

    packet->set_name("connect");
    packet->add_amf0_object( RtmpAmf0Any::number(m_nConnectTid) );
    
    RtmpAmf0Object* obj1 = RtmpAmf0Any::object();
    obj1->set("app", RtmpAmf0Any::str(m_strApp.c_str()));
    obj1->set("type", RtmpAmf0Any::str("nonprivate"));
    obj1->set("flashVer", RtmpAmf0Any::str("LUX 9,0,124,2"));

    std::string url = "rtmp://127.0.0.1/" + m_pConnection->path();
    obj1->set("swfUrl", RtmpAmf0Any::str(url.c_str()));
    obj1->set("tcUrl", RtmpAmf0Any::str( url.c_str() ));
    packet->add_amf0_object(obj1);

    //send the data:
    memset(m_pSendBuf, 0, m_nSendBufCapacity);
    int total_len = msg->get_full_data(0, 3, m_pSendBuf, m_nSendBufCapacity);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::send_fcpublish(RtmpChunkStream* chunk_stream) {
    FUNLOG(Info, "rtmp stream send fcpublish command! app=%s, stream=%s, tid=%d", m_strApp.c_str(), m_strStream.c_str(), m_nTid);
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_AMF0CommandMessage);
    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream send fcpublish, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }

    m_nFCPublishTid = m_nTid++;
    m_nStatus = RTMP_SESSION_STATUS_PUBLISHING;

    packet->set_name("FCPublish");
    packet->add_amf0_object( RtmpAmf0Any::number(m_nFCPublishTid) );
    packet->add_amf0_object( RtmpAmf0Any::null());
    packet->add_amf0_object( RtmpAmf0Any::str(m_strStream.c_str()));

    //send the data:
    memset(m_pSendBuf, 0, m_nSendBufCapacity);
    int total_len = msg->get_full_data(1, 3, m_pSendBuf, m_nSendBufCapacity);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::send_publish(RtmpChunkStream* chunk_stream) {
    FUNLOG(Info, "rtmp stream send publish command! app=%s, stream=%s, tid=%d", m_strApp.c_str(), m_strStream.c_str(), m_nTid);
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_AMF0CommandMessage);
    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream send publish, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }

    packet->set_name("publish");
    packet->add_amf0_object( RtmpAmf0Any::number(m_nTid++) );
    packet->add_amf0_object( RtmpAmf0Any::null());
    packet->add_amf0_object( RtmpAmf0Any::str(m_strStream.c_str()));
    packet->add_amf0_object( RtmpAmf0Any::str("live"));

    //send the data:
    memset(m_pSendBuf, 0, m_nSendBufCapacity);
    int total_len = msg->get_full_data(0, 4, m_pSendBuf, m_nSendBufCapacity);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::send_create_stream(RtmpChunkStream* chunk_stream) {
    FUNLOG(Info, "rtmp stream send create_stream command! app=%s, stream=%s, tid=%d", m_strApp.c_str(), m_strStream.c_str(), m_nTid);
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_AMF0CommandMessage);
    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream send create_stream, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }

    m_nCreateStreamTid = m_nTid++;
    m_nStatus = RTMP_SESSION_STATUS_CREATE_STREAM;

    packet->set_name("createStream");
    packet->add_amf0_object( RtmpAmf0Any::number(m_nCreateStreamTid) );
    packet->add_amf0_object( RtmpAmf0Any::null());

    //send the data:
    memset(m_pSendBuf, 0, m_nSendBufCapacity);
    int total_len = msg->get_full_data(1, 3, m_pSendBuf, m_nSendBufCapacity);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::send_release_stream(RtmpChunkStream* chunk_stream) {
    FUNLOG(Info, "rtmp stream send release_stream command! app=%s, stream=%s, tid=%d", m_strApp.c_str(), m_strStream.c_str(), m_nTid);
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_AMF0CommandMessage);
    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream release_stream, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }

    m_nReleaseStreamTid = m_nTid++;
    m_nStatus = RTMP_SESSION_STATUS_RELEASE_STREAM;

    packet->set_name("releaseStream");
    packet->add_amf0_object( RtmpAmf0Any::number(m_nReleaseStreamTid) );
    packet->add_amf0_object( RtmpAmf0Any::null());
    packet->add_amf0_object( RtmpAmf0Any::str(m_strStream.c_str()));

    //send the data:
    memset(m_pSendBuf, 0, m_nSendBufCapacity);
    int total_len = msg->get_full_data(1, 3, m_pSendBuf, m_nSendBufCapacity);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::send_play(RtmpChunkStream* chunk_stream) {
    FUNLOG(Info, "rtmp stream send play command! app=%s, stream=%s, tid=%d", m_strApp.c_str(), m_strStream.c_str(), m_nTid);
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_AMF0CommandMessage);
    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream play, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }

    m_nPlayTid = m_nTid++;
    m_nStatus = RTMP_SESSION_STATUS_PLAYING;

    packet->set_name("play");
    packet->add_amf0_object( RtmpAmf0Any::number(m_nPlayTid) );
    packet->add_amf0_object( RtmpAmf0Any::null());
    packet->add_amf0_object( RtmpAmf0Any::str(m_strStream.c_str()));
    packet->add_amf0_object( RtmpAmf0Any::number(-2000) );

    //send the data:
    memset(m_pSendBuf, 0, m_nSendBufCapacity);
    int total_len = msg->get_full_data(1, 3, m_pSendBuf, m_nSendBufCapacity);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::send_meta_data(RtmpChunkStream* chunk_stream) {
    FUNLOG(Info, "rtmp stream send meta data, app=%s, stream=%s", m_strApp.c_str(), m_strStream.c_str());
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_AMF0DataMessage);
    RtmpDataMessagePacket* packet = (RtmpDataMessagePacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream send meta data, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
}

void    RtmpStream::ack_window_ack_size(RtmpChunkStream* chunk_stream, uint32_t size) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_WindowAcknowledgementSize, 4);
    RtmpWindowAckSizePacket* packet = (RtmpWindowAckSizePacket*)msg->packet();

    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream ack window size, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_window_size(size);

    int total_len = msg->get_full_data(0, 2, m_pSendBuf, m_nSendBufCapacity);
    FUNLOG(Info, "rtmp stream ack window size, total_len=%d", total_len);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_set_peer_bandwidth(RtmpChunkStream* chunk_stream, uint32_t bandwidth) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_SET_PEER_BANDWIDTH, 5);
    RtmpSetPeerBandwidthPacket* packet = (RtmpSetPeerBandwidthPacket*)msg->packet();

    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream ack window size, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_peer_bandwidth(bandwidth);

    int total_len = msg->get_full_data(0, 2, m_pSendBuf, m_nSendBufCapacity);
    FUNLOG(Info, "rtmp stream ack set peer bandwidth, total_len=%d", total_len);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_chunk_size(RtmpChunkStream* chunk_stream, uint32_t chunk_size) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_SET_CHUNK_SIZE, 4);
    RtmpSetChunkSizePacket* packet = (RtmpSetChunkSizePacket*)msg->packet();

    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream set chunk size, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_chunk_size(chunk_size);

    //send the data:
    int total_len = msg->get_full_data(0, 2, m_pSendBuf, m_nSendBufCapacity);
    FUNLOG(Info, "rtmp stream ack chunk size, total_len=%d", total_len);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_connect(RtmpChunkStream* chunk_stream) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_AMF0CommandMessage);
    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream connect result, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_name("_result");
    packet->add_amf0_object( RtmpAmf0Any::number(1) );

    RtmpAmf0Object* obj1 = RtmpAmf0Any::object();
    obj1->set("fmsVer", RtmpAmf0Any::str("FMS/3,0,1,123") );
    obj1->set("capabilities", RtmpAmf0Any::number(31));
    packet->add_amf0_object(obj1);

    RtmpAmf0Object* obj2 = RtmpAmf0Any::object();
    obj2->set("level", RtmpAmf0Any::str("status"));
    obj2->set("code", RtmpAmf0Any::str("NetConnection.Connect.Success"));
    obj2->set("description", RtmpAmf0Any::str("Connection succeeded."));
    obj2->set("objectEncoding", RtmpAmf0Any::number(0));
    packet->add_amf0_object(obj2);

    //send the data:
    int total_len = msg->get_full_data(0, chunk_stream->cid(), m_pSendBuf, m_nSendBufCapacity);
    FUNLOG(Info, "rtmp stream ack connect, total_len=%d", total_len);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_release_stream(RtmpChunkStream* chunk_stream, uint32_t tid) {
    FUNLOG(Info, "rtmp stream ack release stream, tid=%d", tid);
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_AMF0CommandMessage);
    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream ack release stream result, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_name("_result");
    packet->add_amf0_object( RtmpAmf0Any::number(tid) );
    packet->add_amf0_object( RtmpAmf0Any::null());
    packet->add_amf0_object( RtmpAmf0Any::undefined());

    //send the data:
    int total_len = msg->get_full_data(0, chunk_stream->cid(), m_pSendBuf, m_nSendBufCapacity);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_fcpublish(RtmpChunkStream* chunk_stream, uint32_t tid) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_AMF0CommandMessage);
    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream ack fcpublish, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_name("_result");
    packet->add_amf0_object( RtmpAmf0Any::number(tid) );
    packet->add_amf0_object( RtmpAmf0Any::null());
    packet->add_amf0_object( RtmpAmf0Any::undefined());

    //send the data:
    int total_len = msg->get_full_data(0, chunk_stream->cid(), m_pSendBuf, m_nSendBufCapacity);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_create_stream(RtmpChunkStream* chunk_stream, uint32_t tid) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_AMF0CommandMessage);
    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream ack create stream, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_name("_result");
    packet->add_amf0_object( RtmpAmf0Any::number(tid) );
    packet->add_amf0_object( RtmpAmf0Any::null());
    packet->add_amf0_object( RtmpAmf0Any::number(1));

    //send the data:
    int total_len = msg->get_full_data(0, chunk_stream->cid(), m_pSendBuf, m_nSendBufCapacity);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_publish(RtmpChunkStream* chunk_stream, uint32_t tid, uint32_t stream_id) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_AMF0CommandMessage);
    msg->set_stream_id(stream_id);

    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream connect result, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_name("onFCPublish");
    packet->add_amf0_object( RtmpAmf0Any::number(0) );
    packet->add_amf0_object( RtmpAmf0Any::null());
    
    RtmpAmf0Object* obj1 = RtmpAmf0Any::object();
    obj1->set("code", RtmpAmf0Any::str("NetStream.Publish.Start"));
    obj1->set("description", RtmpAmf0Any::str("Started publishing stream."));
    packet->add_amf0_object(obj1);

    //send the data:
    memset(m_pSendBuf, 0, m_nSendBufCapacity);
    int total_len = msg->get_full_data(0, 5, m_pSendBuf, m_nSendBufCapacity);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_publish_onstatus(RtmpChunkStream* chunk_stream) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_AMF0CommandMessage);
    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream connect result, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_name("onStatus");
    packet->add_amf0_object( RtmpAmf0Any::number(0) );
    packet->add_amf0_object( RtmpAmf0Any::null());
    
    RtmpAmf0Object* obj1 = RtmpAmf0Any::object();
    obj1->set("level", RtmpAmf0Any::str("status"));
    obj1->set("code", RtmpAmf0Any::str("NetStream.Publish.Start"));
    obj1->set("description", RtmpAmf0Any::str("Start publishing"));
    packet->add_amf0_object(obj1);

    //send the data:
    memset(m_pSendBuf, 0, m_nSendBufCapacity);
    int total_len = msg->get_full_data(0, chunk_stream->cid(), m_pSendBuf, m_nSendBufCapacity);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_play(RtmpChunkStream* chunk_stream, uint32_t tid) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_AMF0CommandMessage);
    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream play result, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_name("onStatus");
    packet->add_amf0_object( RtmpAmf0Any::number(0) );
    packet->add_amf0_object( RtmpAmf0Any::null());
    
    RtmpAmf0Object* obj1 = RtmpAmf0Any::object();
    obj1->set("level", RtmpAmf0Any::str("status"));
    obj1->set("code", RtmpAmf0Any::str("NetStream.Play.Start"));
    obj1->set("description", RtmpAmf0Any::str("Start live"));
    packet->add_amf0_object(obj1);

    //send the data:
    memset(m_pSendBuf, 0, m_nSendBufCapacity);
    int total_len = msg->get_full_data(0, chunk_stream->cid(), m_pSendBuf, m_nSendBufCapacity);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_stream_begin(RtmpChunkStream* chunk_stream) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_USER_CTL);
    RtmpUserCtlPacket* packet = (RtmpUserCtlPacket*)msg->packet();

    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream ack ping failed, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_event(RTMP_USER_CTL_STREAM_BEGIN);
    packet->set_data(1);

    int total_len = msg->get_full_data(0, 2, m_pSendBuf, m_nSendBufCapacity);
    FUNLOG(Info, "rtmp stream ack stream begin, total_len=%d", total_len);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_ping(RtmpChunkStream* chunk_stream, uint32_t data) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_USER_CTL);
    RtmpUserCtlPacket* packet = (RtmpUserCtlPacket*)msg->packet();

    if( packet == NULL ) {
        FUNLOG(Error, "rtmp stream ack ping failed, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_event(RTMP_USER_CTL_PING_RESPONSE);
    packet->set_data(data);

    int total_len = msg->get_full_data(0, 2, m_pSendBuf, m_nSendBufCapacity);
    FUNLOG(Info, "rtmp stream ack ping, total_len=%d", total_len);
    m_pConnection->send(m_pSendBuf, total_len);
}
