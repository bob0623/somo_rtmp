#pragma once

#include <isnet.h>
#include <stdint.h>
#include <string>
#include <map>

class VideoFrame;
class AudioFrame;
class Publisher {
public:
    virtual void    on_audio(AudioFrame* frame) = 0;
    virtual void    on_video(VideoFrame* frame) = 0;
     
};

class Consumer {
public:
    virtual uint32_t  id() = 0;
    virtual void    on_audio(AudioFrame* frame) = 0;
    virtual void    on_video(VideoFrame* frame) = 0;
    virtual void    on_video_rtmp(const char* data, int len) = 0;
};

class Session {
public:
    Session(const std::string& stream);
    virtual ~Session();

public:
    void    set_publisher(Publisher* publisher);
    void    add_consumer(Consumer* consumer);
    void    remove_consumer(uint32_t id);
    Consumer*   get_consumer(uint32_t id);

public:
    void    on_audio();
    void    on_video(VideoFrame* frame);
    void    on_video_rtmp(const char* data, int len);

public:
    std::string    stream() { return m_strStream; }

private:
    std::string m_strStream;
    Publisher*  m_pPublisher;
    std::map<uint32_t, Consumer*>   m_mapConsumers;
};


