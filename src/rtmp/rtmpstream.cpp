#include "rtmpstream.h"
#include "app.h"
#include "session.h"
#include "rtmpformat.h"
#include "rtmpconnection.h"
#include "rtmppublisher.h"
#include "rtmpconsumer.h"
#include "rtmpsession.h"
#include "rtmpparser.h"
#include "rtmpbuffer.h"
#include "rtmpamf.h"

#include "video/videoframe.h"
#include "video/videoframepool.h"
#include "audio/audioframe.h"
#include "audio/audioframepool.h"

#include "common/logger.h"
#include <string.h>


#define __CLASS__   "RtmpSession"

RtmpStream::RtmpStream(RtmpConnection* conn) 
: m_nChunkSizeIn(0)
, m_pConnection(conn)
, m_pPublisher(NULL)
, m_pConsumer(NULL)
, m_nType(RTMP_SESSION_TYPE_UNKNOWN)
, m_nAudioFrames(0)
, m_nVideoFrames(0)
{
    m_nSendBufCapacity = 1024*128;
    m_pSendBuf = new char[ m_nSendBufCapacity];

    m_pParser = new RtmpParser();
}

RtmpStream::~RtmpStream() {
    delete m_pSendBuf;
    delete m_pParser;

    if( m_pPublisher ) {
        delete m_pPublisher;
    }
    if( m_pConsumer ) {
        delete m_pConsumer;
    }
}

void    RtmpStream::on_msg(RtmpMessage* msg) {
    switch( msg->type() ) {
        case RTMP_MSG_SetChunkSize:
        {
            RtmpSetChunkSizePacket* chunk_msg = (RtmpSetChunkSizePacket*)msg->packet();
            m_nChunkSizeIn = chunk_msg->chunk_size();
            FUNLOG(Info, "rtmp session set chunk size, chunk_size=%d", m_nChunkSizeIn);
            m_pConnection->buffer()->set_chunk_size(m_nChunkSizeIn);
        }
        break;
        case RTMP_MSG_AMF0CommandMessage:   //20
            on_command( msg );
        break;
        case RTMP_MSG_AudioMessage:
            on_audio( msg );
            break;
        case RTMP_MSG_VideoMessage:
            on_video( msg );
            break;
    }
}

void    RtmpStream::on_command(RtmpMessage* msg) {
    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    if( packet->name() == "connect" ) {

        m_strApp = packet->connect_packet()->app;
        m_strTcUrl = packet->connect_packet()->tcUrl;

        FUNLOG(Info, "rtmp session command connect, app=%s, tcUrl=%s", m_strApp.c_str(), m_strTcUrl.c_str());
        ack_window_ack_size(msg->chunk_stream(), 2500000);
        ack_set_peer_bandwidth(msg->chunk_stream(), 2500000);
        ack_chunk_size(msg->chunk_stream(), 60000);
        ack_connect(msg->chunk_stream());
    } else if( packet->name() == "releaseStream" ) {
        m_strStream = packet->release_stream_packet()->stream;

        uint32_t tid = packet->release_stream_packet()->tid;
        FUNLOG(Info, "rtmp session command releaseStream, stream=%s, tid=%d", m_strStream.c_str(), tid);
        ack_release_stream(msg->chunk_stream(), tid);

    }  else if( packet->name() == "FCPublish" ) {
        m_strStream = packet->fcpublish_packet()->stream;

        uint32_t tid = packet->fcpublish_packet()->tid;
        FUNLOG(Info, "rtmp session command FCPublish, stream=%s, tid=%d", m_strStream.c_str(), tid);
        ack_fcpublish(msg->chunk_stream(), tid);

    } else if( packet->name() == "createStream" ) {
        uint32_t tid = packet->create_stream_packet()->tid;
        FUNLOG(Info, "rtmp session command createStream, tid=%d", tid);
        ack_create_stream(msg->chunk_stream(), tid);

    } else if( packet->name() == "publish" ) {
        uint32_t tid = packet->publish_packet()->tid;
        FUNLOG(Info, "rtmp session command publish, tid=%d, stream=%s, mode=%s, stream_id=%d", tid, packet->publish_packet()->stream.c_str(), packet->publish_packet()->mode.c_str(), msg->stream_id());
        ack_publish(msg->chunk_stream(), tid, msg->stream_id());
        ack_publish_onstatus(msg->chunk_stream());

        m_nType = RTMP_SESSION_TYPE_PUBLISH;
        m_pPublisher = new RtmpPublisher();

        //put this as session publisher:
        //1, verify the stream doesn't exist!
        //2, if stream exist, report fail!
        //3, if stream not exist, create it with App.
        App::Ins()->add_session( packet->publish_packet()->stream, PROTOCOL_RTMP); 
    } else if( packet->name() == "play" ) {
        uint32_t tid = packet->play_packet()->tid;
        FUNLOG(Info, "rtmp session command play, stream=%s", packet->play_packet()->stream.c_str());
        ack_stream_begin(msg->chunk_stream());
        ack_play(msg->chunk_stream(), tid);

        m_nType = RTMP_SESSION_TYPE_PLAY;
        m_pConsumer = new RtmpConsumer();

        //put this as session consumer:
        //1, verify the stream doesn't exist!
        //2, if stream exist, report fail!
        //3, if stream not exist, create it with App.
        Session* session = App::Ins()->get_session(packet->play_packet()->stream);
        if( session == NULL ) {
            FUNLOG(Error, "rtmp session command play, session not exist for stream=%s", packet->play_packet()->stream.c_str());
            return;
        } else {
            RtmpConsumer* consumer = new RtmpConsumer();
            session->add_consumer(consumer);
        }
    }
}


void    RtmpStream::on_audio(RtmpMessage* msg) {
    m_nAudioFrames++;

    if( m_nAudioFrames%100 == 0 || m_nAudioFrames <= 5 ) {
        FUNLOG(Info, "rtmp session on audio frame, frames=%d, size=%d", m_nAudioFrames, msg->msg_len());
    }

    //AudioFrame* frame = AudioFramePool::Ins()->get( msg->payload_len() );
    //m_pParser->parse_audio( (uint8_t*)msg->payload(), (size_t)msg->payload_len(), frame );
    //AudioFramePool::Ins()->free(frame);
}

void    RtmpStream::on_video(RtmpMessage* msg) {
    m_nVideoFrames++;

    if( m_nVideoFrames%30 == 0 || m_nVideoFrames <= 5) {
        FUNLOG(Info, "rtmp session on video frame, frames=%d, size=%d", m_nVideoFrames, msg->msg_len());
    }

    //VideoFrame* frame = VideoFramePool::Ins()->get( msg->payload_len() );
    //m_pParser->parse_video((uint8_t*)msg->payload(), (size_t)msg->payload_len(), frame );
    //VideoFramePool::Ins()->free(frame);
}

void    RtmpStream::ack_window_ack_size(RtmpChunkStream* chunk_stream, uint32_t size) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_WindowAcknowledgementSize, 4);
    RtmpWindowAckSizePacket* packet = (RtmpWindowAckSizePacket*)msg->packet();

    if( packet == NULL ) {
        FUNLOG(Error, "rtmp session ack window size, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_window_size(size);

    int total_len = msg->get_full_data(0, 2, m_pSendBuf, m_nSendBufCapacity);
    FUNLOG(Info, "rtmp session ack window size, total_len=%d", total_len);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_set_peer_bandwidth(RtmpChunkStream* chunk_stream, uint32_t bandwidth) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_SetPeerBandwidth, 5);
    RtmpSetPeerBandwidthPacket* packet = (RtmpSetPeerBandwidthPacket*)msg->packet();

    if( packet == NULL ) {
        FUNLOG(Error, "rtmp session ack window size, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_peer_bandwidth(bandwidth);

    int total_len = msg->get_full_data(0, 2, m_pSendBuf, m_nSendBufCapacity);
    FUNLOG(Info, "rtmp session ack set peer bandwidth, total_len=%d", total_len);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_chunk_size(RtmpChunkStream* chunk_stream, uint32_t chunk_size) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_SetChunkSize, 4);
    RtmpSetChunkSizePacket* packet = (RtmpSetChunkSizePacket*)msg->packet();

    if( packet == NULL ) {
        FUNLOG(Error, "rtmp session set chunk size, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_chunk_size(chunk_size);

    //send the data:
    int total_len = msg->get_full_data(0, 2, m_pSendBuf, m_nSendBufCapacity);
    FUNLOG(Info, "rtmp session ack chunk size, total_len=%d", total_len);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_connect(RtmpChunkStream* chunk_stream) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_AMF0CommandMessage);
    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp session connect result, packet==NULL! cid=%d", chunk_stream->cid());
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
    FUNLOG(Info, "rtmp session ack connect, total_len=%d", total_len);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_release_stream(RtmpChunkStream* chunk_stream, uint32_t tid) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_AMF0CommandMessage);
    RtmpCommandPacket* packet = (RtmpCommandPacket*)msg->packet();
    if( packet == NULL ) {
        FUNLOG(Error, "rtmp session release stream result, packet==NULL! cid=%d", chunk_stream->cid());
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
        FUNLOG(Error, "rtmp session connect result, packet==NULL! cid=%d", chunk_stream->cid());
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
        FUNLOG(Error, "rtmp session connect result, packet==NULL! cid=%d", chunk_stream->cid());
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
        FUNLOG(Error, "rtmp session connect result, packet==NULL! cid=%d", chunk_stream->cid());
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
        FUNLOG(Error, "rtmp session connect result, packet==NULL! cid=%d", chunk_stream->cid());
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
        FUNLOG(Error, "rtmp session play result, packet==NULL! cid=%d", chunk_stream->cid());
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

    packet->add_amf0_object( RtmpAmf0Any::object_eof() );

    //send the data:
    memset(m_pSendBuf, 0, m_nSendBufCapacity);
    int total_len = msg->get_full_data(0, chunk_stream->cid(), m_pSendBuf, m_nSendBufCapacity);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_stream_begin(RtmpChunkStream* chunk_stream) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_UserControlMessage);
    RtmpUserCtlPacket* packet = (RtmpUserCtlPacket*)msg->packet();

    if( packet == NULL ) {
        FUNLOG(Error, "rtmp session ack ping failed, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_event(RTMP_USER_CTL_STREAM_BEGIN);
    packet->set_data(1);

    int total_len = msg->get_full_data(0, 2, m_pSendBuf, m_nSendBufCapacity);
    FUNLOG(Info, "rtmp session ack stream begin, total_len=%d", total_len);
    m_pConnection->send(m_pSendBuf, total_len);
}

void    RtmpStream::ack_ping(RtmpChunkStream* chunk_stream, uint32_t data) {
    RtmpMessage* msg = new RtmpMessage(chunk_stream, RTMP_MSG_UserControlMessage);
    RtmpUserCtlPacket* packet = (RtmpUserCtlPacket*)msg->packet();

    if( packet == NULL ) {
        FUNLOG(Error, "rtmp session ack ping failed, packet==NULL! cid=%d", chunk_stream->cid());
        return;
    }
    packet->set_event(RTMP_USER_CTL_PING_RESPONSE);
    packet->set_data(data);

    int total_len = msg->get_full_data(0, 2, m_pSendBuf, m_nSendBufCapacity);
    FUNLOG(Info, "rtmp session ack ping, total_len=%d", total_len);
    m_pConnection->send(m_pSendBuf, total_len);
}
