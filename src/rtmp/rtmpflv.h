#pragma once

#include "video/videospsparser.h"

#include <unistd.h>

class IOBuffer;
class VideoFrame;
class AudioFrame;

struct VideoTagHeader {
    int     frame_type;
    int     codec;
    int     avc_packet_type;
    int     composition_time;
};
    
struct AVCDecoderConfigurationRecord {
    int     version;
    int     avc_profile;
    int     profile_compatibility;
    int     avc_level;
    int     length_size_minus_one;
    int     num_sps;
    int     num_pps;
};
    
struct VideoNalu {
    int     type;
};

struct AudioTagHeader {
    int     channels;
    int     sample_format;
    int     sample_rate;
    int     codec;
};

class RtmpFlvFormat
{
public:
    RtmpFlvFormat();
    ~RtmpFlvFormat();

public:
    void format_video_sh(IOBuffer* buf, const char* sps, int sps_len, const char* pps, int pps_len);
    void format_video_frame(IOBuffer* buf, VideoFrame* frame);
    void format_audio(IOBuffer* buf, AudioFrame* frame);

private:

};

class RtmpFlvParser {
public:
    RtmpFlvParser();
    ~RtmpFlvParser();

public:
    void    parse_video_tag(const char* buf, size_t size, VideoFrame* frame);
    void    parse_video_avc_packet(const char* buf, size_t size, VideoFrame* frame);
    void    parse_video_decoder_config(const char* buf, size_t size, VideoFrame* frame);
    void    parse_video_nalu(const char* buf, size_t size, VideoFrame* frame);
    void    parse_audio_tag(const char* buf, size_t size, AudioFrame* frame);

public:
    bool    is_video_sh();

private:
    int     flv_sample_rate_2_somo(int flv_sample_rate);

private:
    VideoTagHeader  header;
    AVCDecoderConfigurationRecord   avc_config;
    VideoNalu       nalu;
    AudioTagHeader  m_audio_header;

    char*           m_pSpsPpsBuffer;
    int             m_iSpsPpsLen;
    int             m_iIDRFrameCount;

    char*           m_pFrame;
    int             m_nFrameLen;
    int             m_nFrameType;
    sps_info        m_sps;
};
