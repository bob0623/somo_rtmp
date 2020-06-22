#pragma once

#include "video/videospsparser.h"
#include <stdint.h>
#include <unistd.h>

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


class VideoFrame;
class AudioFrame;
class RtmpParser {
public:
    RtmpParser();
    ~RtmpParser();

public:
    void    parse_video(uint8_t* buf, size_t size, VideoFrame* frame);
    void    parse_video_avc_seq_header(uint8_t* buf, size_t size);
    void    parse_video_avc_packet(uint8_t* buf, size_t size, VideoFrame* frame);
    void    parse_video_decoder_config(uint8_t* buf, size_t size, VideoFrame* frame);
    void    parse_video_nalu(uint8_t* buf, size_t size, VideoFrame* frame);

    void    parse_audio(uint8_t* buf, size_t size, AudioFrame* frame);

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
