#pragma once

#include <isnet.h>
#include <stdint.h>
#include <string>
#include <map>
#include <vector>

class Stream;
class Consumer;
class Publisher;
class Session;
class Client;
class Filter;

class VideoFrame;
class AudioFrame;
class Publisher {
public:
    virtual uint32_t  id() = 0;

    virtual std::string stream() = 0;
    /**
     * callback when new consumer comes. this is useful when new consumer coming in and then publisher can send some data to init.
     */
    virtual void    on_new_consumer(Consumer* consumer) = 0;


    /**
     * callback when @setDataFrame comes.
     */
    virtual void    on_meta_data(const char* data, int len) = 0;

    virtual void    on_audio(AudioFrame* frame) = 0;

    /**
     * callback for receive video frame from any other protocol. this is useful for common protocol.
     * 
     */
    virtual void    on_video(VideoFrame* frame) = 0;

    /**
     * callback for rtmp video message, this break abstraction, but is quite important because it simplify RTMP message distribution.
     * Consider most clients are RTMP, use this greatly reduce memory copy refer to on_video callback.
     * 
     */
    virtual void    on_video_rtmp(const char* data, int len) = 0;

    /**
     * rtmp sequence header, which hold the rtmp sps&pps data.
     * rtmp_sh data need to be send to all "new consumer"
     */
    virtual void    on_video_rtmp_sh(const char* data, int len) = 0;

    virtual ~Publisher() {}

};

class Consumer {
public:
    virtual uint32_t  id() = 0;
    virtual void    on_meta_data(const char* data, int len) = 0;
    virtual void    on_audio(AudioFrame* frame) = 0;
    virtual void    on_audio_rtmp(const char* data, int len) = 0;
    virtual void    on_video(VideoFrame* frame) = 0;
    virtual void    on_video_rtmp(const char* data, int len) = 0;
    virtual void    on_video_rtmp_sh(const char* data, int len) = 0;
    virtual ~Consumer() {}
};

class Session {
public:
    Session(const std::string& stream);
    virtual ~Session();

public:
    void    set_publisher(Publisher* publisher);
    void    remove_publisher();
    void    add_consumer(Consumer* consumer);
    void    remove_consumer(uint32_t id);
    Consumer*   get_consumer(uint32_t id);
    
    Client* add_forwarder(const std::string& url, Client* client);
    void    remove_forwarder(const std::string& url);
    Client* get_forwarder(const std::string& url);

    void    add_filter(Filter* filter);

    std::string stream() { return m_strStream; }
    Publisher*  publisher() { return m_pPublisher; }

public:
    void    on_meta_data(const char* data, int len);
    void    on_audio();
    void    on_audio_rtmp(const char* data, int len);

    void    on_video(VideoFrame* frame);
    void    on_video_rtmp(const char* data, int len);

private:
    std::string m_strStream;
    Publisher*  m_pPublisher;
    std::map<uint32_t, Consumer*>   m_mapConsumers;
    std::map<std::string, Client*>  m_mapForwarders;
    std::vector<Filter*>    m_arrFilters;
};



