#include "session.h"
#include "connection.h"
#include "client.h"

#include "common/logger.h"

#define     __CLASS__   "Session"

Session::Session(const std::string& stream) 
: m_strStream(stream)
{

}

Session::~Session() {

}

void    Session::set_publisher(Publisher* publisher) {
    m_pPublisher = publisher;
}

void    Session::remove_publisher() {
    FUNLOG(Info, "session remove publisher, stream=%s", m_pPublisher->stream().c_str() );
    if( m_pPublisher ) {
        delete m_pPublisher;
        m_pPublisher = NULL;
    }
}

void    Session::add_consumer(Consumer* consumer) {
    Consumer* old = get_consumer(consumer->id());
    if( old != NULL ) {
        FUNLOG(Info, "session add consumer failed! consumer exist for id=%d", old->id());
        delete consumer;
    }

    m_mapConsumers[consumer->id()] = consumer;

    //maybe need to send some config data to new consumer:
    //Ex: RTMP Sequence header need to be sent to new consumer.
    //
    m_pPublisher->on_new_consumer(consumer);
}

void    Session::remove_consumer(uint32_t id) {
    FUNLOG(Info, "session remove consumer, id=%d", id);
    Consumer* consumer = get_consumer(id);
    if( consumer == NULL ) {
        FUNLOG(Error, "session remove consumer failed! not exist for id=%d", id);
        return;
    }

    delete consumer;
    m_mapConsumers.erase(id);
}

Consumer*   Session::get_consumer(uint32_t id) {
    auto it = m_mapConsumers.find(id);
    if( it == m_mapConsumers.end() ) {
        return NULL;
    }

    return it->second;
}

Client* Session::add_forwarder(const std::string& url, Client* client) {
    Client* old = get_forwarder(url);
    if( old != NULL ) {
        FUNLOG(Error, "session add forwarder failed, already exist for url=%s", url.c_str());
        return old;
    }

    m_mapForwarders[url] = client;
}

void    Session::remove_forwarder(const std::string& url) {
    auto it = m_mapForwarders.find(url);
    if( it == m_mapForwarders.end() ) {
        FUNLOG(Error, "session get forwarder failed, not exist for url=%s", url.c_str());
        return;
    }
    delete it->second;
    m_mapForwarders.erase(it);
}

Client* Session::get_forwarder(const std::string& url) {
    auto it = m_mapForwarders.find(url);
    if( it == m_mapForwarders.end() ) {
        FUNLOG(Error, "session get forwarder failed, not exist for url=%s", url.c_str());
        return NULL;
    }

    return it->second;
}

void    Session::on_meta_data(const char* data, int len) {
    FUNLOG(Info, "session on meta data, len=%d", len);
    for( auto it=m_mapConsumers.begin(); it!=m_mapConsumers.end(); it++ ) {
        it->second->on_meta_data(data, len);
    }

    for( auto it=m_mapForwarders.begin(); it!=m_mapForwarders.end(); it++ ) {
        it->second->on_meta_data(data, len);
    }
}

void    Session::on_audio_rtmp(const char* data, int len) {
    for( auto it=m_mapConsumers.begin(); it!=m_mapConsumers.end(); it++ ) {
        it->second->on_audio_rtmp(data, len);
    }
}

void    Session::on_video(VideoFrame* frame) {
    for( auto it=m_mapConsumers.begin(); it!=m_mapConsumers.end(); it++ ) {
        it->second->on_video(frame);
    }
}

void    Session::on_video_rtmp(const char* data, int len) {
    for( auto it=m_mapConsumers.begin(); it!=m_mapConsumers.end(); it++ ) {
        it->second->on_video_rtmp(data, len);
    }

    for( auto it=m_mapForwarders.begin(); it!=m_mapForwarders.end(); it++ ) {
        it->second->on_video_rtmp(data, len);
    }
}