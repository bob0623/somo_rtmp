#include "session.h"
#include "connection.h"
#include "client.h"

#include "common/logger.h"

#define     __CLASS__   "Session"

Session::Session(const std::string& stream) 
: m_strStream(stream)
, m_pPublisher(NULL)
, m_nFrameTick(0)
{
    FUNLOG(Info, "session construct, this=%p", this);
}

Session::~Session() {

}

void    Session::set_publisher(Publisher* publisher) {
    m_pPublisher = publisher;
}

void    Session::remove_publisher() {
    FUNLOG(Info, "session remove publisher, stream=%s", m_pPublisher->stream().c_str() );
    // if( m_pPublisher ) {
    //     delete m_pPublisher;
    //     m_pPublisher = NULL;
    // }
    m_pPublisher = NULL;
    FUNLOG(Info, "session remove publisher over.", NULL);
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
    if (m_pPublisher) {
        m_pPublisher->on_new_consumer(consumer);
    } else {
        FUNLOG(Error, "m_pPublisher == NULL", NULL);
    }
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
    FUNLOG(Info, "session add_forwarder, url =%s, client=%p", url.c_str(), client);
    Client* old = get_forwarder(url);
    if( old != NULL ) {
        FUNLOG(Warn, "session add forwarder failed, already exist for url=%s", url.c_str());
        return old;
    }
    FUNLOG(Info, "session add_forwarder complete, url =%s, client=%p", url.c_str(), client);
    m_mapForwarders[url] = client;
}

void    Session::remove_forwarder(const std::string& url) {
    auto it = m_mapForwarders.find(url);
    if( it == m_mapForwarders.end() ) {
        FUNLOG(Warn, "session remove forwarder failed, not exist for url=%s", url.c_str());
        return;
    }
    FUNLOG(Info, "session remove_forwarder, url=%s", url.c_str());
    delete it->second;
    m_mapForwarders.erase(it);
}

Client* Session::get_forwarder(const std::string& url) {
    auto it = m_mapForwarders.find(url);
    if( it == m_mapForwarders.end() ) {
        FUNLOG(Warn, "session get forwarder failed, not exist for url=%s", url.c_str());
        return NULL;
    }

    return it->second;
}

void    Session::add_filter(Filter* filter) {
    m_arrFilters.push_back(filter);
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
    if (m_nFrameTick++ < 3 || m_nFrameTick % 300 == 0) {
        FUNLOG(Info, "session on_video_rtmp, m_mapConsumers.size=%d, m_mapForwarders.size=%d， this=%p", m_mapConsumers.size(), m_mapForwarders.size(),this);
    }
    for( auto it=m_mapConsumers.begin(); it!=m_mapConsumers.end(); it++ ) {
        it->second->on_video_rtmp(data, len);
    }

    for( auto it=m_mapForwarders.begin(); it!=m_mapForwarders.end(); it++ ) {
        it->second->on_video_rtmp(data, len);
    }
}