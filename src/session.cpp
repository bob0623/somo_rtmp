#include "session.h"
#include "connection.h"

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
}