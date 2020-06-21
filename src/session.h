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
    virtual void    on_audio(AudioFrame* frame) = 0;
    virtual void    on_video(VideoFrame* frame) = 0;
};

class Session {
public:
    Session(uint32_t sid);
    ~Session();

public:
    void    set_publisher(Publisher* publisher);
    void    add_consumer(Consumer* consumer);
    void    remove_consumer(uint32_t linkid);

public:
    uint64_t    sid() { return m_nSid; }

private:
    uint32_t    m_nSid;
    std::string m_strApp;
    std::string m_strStream;
    Publisher*  m_pPublisher;
    std::map<uint32_t, Consumer*>   m_mapConsumers;
};


