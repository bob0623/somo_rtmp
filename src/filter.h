#pragma once

class VideoFrame;
class AudioFrame;

/**
 * Filters will be called when Producer receive audio&video frames!
 * A Producer can have multiple filters, and the audio&video frames will be handled in sequence!
 */
class Filter {
public:
    virtual void    on_audio(AudioFrame* frame) = 0;
    virtual void    on_video(VideoFrame* frame) = 0;
    virtual ~Filter() {}
};

class NoAudioFilter : public Filter {
    virtual void    on_audio(AudioFrame* frame) {}
    virtual void    on_video(VideoFrame* frame) {}
};

class NoVideoFilter : public Filter {
    virtual void    on_audio(AudioFrame* frame) {}
    virtual void    on_video(VideoFrame* frame) {}
};

class VideoResizeFilter : public Filter {
public:
    virtual void    on_audio(AudioFrame* frame) {}
    virtual void    on_video(VideoFrame* frame) {}
};


class HlsSegmentFilter : public Filter {
public:
    virtual void    on_audio(AudioFrame* frame) {}
    virtual void    on_video(VideoFrame* frame) {}
};





