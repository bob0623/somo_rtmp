#include "rtmpparser.h"
#include "video/videospsparser.h"
#include "video/videoframe.h"
#include "video/videoh264.h"
#include "audio/audioframe.h"
#include "audio/audiocodec.h"

#include "common/logger.h"

#include <string.h>
#include <stdlib.h>


#define __CLASS__ "RtmpParser"

int bytesToInt(uint8_t* src) {
    int value = (int) (((src[0] & 0xFF) << 24)
                   | ((src[1] & 0xFF) << 16)
                   | ((src[2] & 0xFF) << 8)
                   | (src[3] & 0xFF));
    return value;
}
    
int bytesToInt16(uint8_t* src) {
    int value = (int) (((src[0] & 0xFF) << 8)
                       | (src[1] & 0xFF));
    return value;
}

#define VIDEO_CODEC_AVC     7
#define MAX_SPS_PPS_LEN     1024
#define MAX_FRAME_SIZE      1024*512

RtmpParser::RtmpParser()
    : m_pSpsPpsBuffer(NULL)
    , m_iSpsPpsLen(0)
    , m_iIDRFrameCount(0)
{
    m_pSpsPpsBuffer = new char[MAX_SPS_PPS_LEN];
    m_pFrame = new char[MAX_FRAME_SIZE];
    m_nFrameLen = 0;
    m_nFrameType = 0;
}

RtmpParser::~RtmpParser()
{
    FUNLOG(Info, "dealloc.", NULL);
    delete [] m_pSpsPpsBuffer;
    m_pSpsPpsBuffer = NULL;
    delete[] m_pFrame;
    m_pFrame = NULL;
}

void    RtmpParser::parse_video(uint8_t* buf, size_t size, VideoFrame* frame) {
    auto *p = buf;
    int readed = 0;
    header.frame_type = (*p&0xf0)>>4;
    header.codec = (*p&0x0f);
    readed += 1;
    
    if( header.codec != VIDEO_CODEC_AVC ) {   //NOT AVC
        FUNLOG(Warn, "rtmp parse video tag, not avc, ignore it.", NULL);
        return;
    }
    
    header.avc_packet_type = *(p+readed);
    readed += 1;
    
    if( header.avc_packet_type == 0 ) {
        FUNLOG(Warn, "rtmp parse video tag, avc_packet_type == 0", NULL);
    } else if( header.avc_packet_type == 1 ) {
        header.composition_time = *(p+readed);
    }
    readed+=3;
    
    if( header.frame_type == 5 ) {
        FUNLOG(Warn, "rtmp parse video tag, frame_type == 5", NULL);
    } else{
        if( header.codec == VIDEO_CODEC_AVC ) {
            parse_video_avc_packet(buf+readed, size-readed, frame);
        } else {
            FUNLOG(Warn, "rtmp parse video tag, header.codec != 7, is:%d", header.codec);
        }
    }
}
    
void    RtmpParser::parse_video_avc_seq_header(uint8_t* buf, size_t size) {
    
}
    
void    RtmpParser::parse_video_avc_packet(uint8_t* buf, size_t size, VideoFrame* frame) {
    FUNLOG(Info, "rtmp parse video tag, header.avc_packet_type = %d, size=%d", header.avc_packet_type, size);
    if( header.avc_packet_type == 0 ) {
        //AVCDecoderConfigurationRecord
        parse_video_decoder_config(buf, size, frame);
    } else if( header.avc_packet_type == 1 ) {
        //NALU
        parse_video_nalu(buf, size, frame);
    }
}
    
void    RtmpParser::parse_video_decoder_config(uint8_t* buf, size_t size, VideoFrame* frame) {
    uint8_t* temp = buf;
    avc_config.version = buf[0];
    avc_config.avc_profile = buf[1];
    avc_config.profile_compatibility = buf[2];
    avc_config.avc_level = buf[3];
    avc_config.length_size_minus_one = buf[4]&0x3+1;
    avc_config.num_sps = buf[5]&0x1F;

    memset(m_pSpsPpsBuffer, 0, MAX_SPS_PPS_LEN);
    m_iSpsPpsLen=0;
    int sps_len = 0;
    int pps_len = 0;
    temp += 6;
    uint8_t seperator[4] = {0,0,0,1};
    for( int i=0; i<avc_config.num_sps; i++ ) {
        sps_len = bytesToInt16(temp);
        temp += 2;

        memcpy(m_pSpsPpsBuffer+m_iSpsPpsLen, (char*)seperator, 4);
        m_iSpsPpsLen+=4;
        memcpy(m_pSpsPpsBuffer+m_iSpsPpsLen, (char*)temp, sps_len);
        m_iSpsPpsLen+=sps_len;

        h264_parse_sps(temp, sps_len, &m_sps);
        frame->setWidth( m_sps.width);
        frame->setHeight( m_sps.height );
        frame->setFps( m_sps.fps);

        temp += sps_len;
        FUNLOG(Info, "rtmp parse video config, found sps, sps_num=%d, sps_length=%d", avc_config.num_sps, sps_len);
    }

    avc_config.num_pps = temp[0]&31; //should always equal to 1;
    temp += 1;
    for( int i=0; i<avc_config.num_pps; i++ ) {
        pps_len = bytesToInt16(temp);
        temp += 2;

        memcpy(m_pSpsPpsBuffer+m_iSpsPpsLen, (char*)seperator, 4);
        m_iSpsPpsLen+=4;

        memcpy(m_pSpsPpsBuffer+m_iSpsPpsLen, (char*)temp, pps_len);
        m_iSpsPpsLen+=pps_len;
        
        temp += pps_len;
        FUNLOG(Info, "rtmp parse video config, found pps, pps_num=%d, pps_length=%d",avc_config.num_pps, pps_len);
    }
    
    FUNLOG(Info, "rtmp parse video config, found sps&pps, sps_num={%d}, sps_len={%d}, pps_num={%d}, pps_len={%d}, width=%d,height=%d, fps=%d", avc_config.num_sps, sps_len,avc_config.num_pps, pps_len, frame->width(), frame->height(), frame->fps());
}
    
void    RtmpParser::parse_video_nalu(uint8_t* buf, size_t size, VideoFrame* frame) {
    uint8_t seperator[4] = {0,0,0,1};
    uint8_t* temp = buf;
    int total = 0;
    int len;
    int b1;

    m_nFrameLen = 0;
    frame->setWidth(m_sps.width);
    frame->setHeight(m_sps.height);
    frame->setFps(m_sps.fps);
    while( total < size-4 ) {
        len = bytesToInt(temp);
        b1 = temp[4] & 31; //31=0x00011111
        temp += 4;
        //FUNLOG(Info, "video nalu, frameType=%d, m_iSpsPpsLen=%d", b1, m_iSpsPpsLen);
        if (b1 == 5 && m_iSpsPpsLen > 0) { //insert sps&pps before the first IDR frame.
            frame->append(m_pSpsPpsBuffer, m_iSpsPpsLen);
        }

        frame->append((const char*)seperator, 4);
        frame->append((const char*)temp, len);

        temp+=len;
        m_nFrameType = (b1 == 5 ? VIDEO_FRAME_I : VIDEO_FRAME_P);
        frame->setType(m_nFrameType);
        switch(b1) {
            case 1:
//                FUNLOGI("video nalu, BP frame, len={%d}", len);
                break;
            case 5:
                m_iIDRFrameCount++;
//                FUNLOGI("video nalu, I frame, len={%d}", len);
                break;
            case 6:
                break;
            case 7:
                break;
            case 8:
                break;
            case 9:
                break;
        }
        total += len+4;
    }
}

void    RtmpParser::parse_audio(uint8_t* buf, size_t size, AudioFrame* frame) {
    char* temp = (char*)buf;
    int len = size;
    int aac_packet_type = 0;

    uint8_t b1 = buf[0];
    m_audio_header.codec = (b1&0xf0)>>4;
    m_audio_header.sample_rate = (b1&0xF)>>2;
    m_audio_header.sample_format = (b1&0x3)>>1;
    m_audio_header.channels = b1&0x1;
    if( m_audio_header.codec == 0x0A ) {
        //AAC, another byte:
        aac_packet_type = temp[1];
        temp = temp+2;
        len = len-2;
    } else {
        temp = temp+1;
        len = len-1;
    }

    FUNLOG(Info, "parse audio, buf[0]=%x, buf[1]=%x codec=%d, sample_rate=%d, format=%d, channels=%d", buf[0], buf[1], m_audio_header.codec, m_audio_header.sample_rate, m_audio_header.sample_format, m_audio_header.channels);
    //E4.2.2 AACAUDIODATA
    //if AACPacketType==0, AudioSpecificConfig
    //else if AACPacketType==1, Raw AAC frame data in UI8[]
    if( aac_packet_type == 0 ) {

    } else {
        frame->setCodec(AUDIO_CODEC_AAC);
        frame->setSampleRate( flv_sample_rate_2_somo(m_audio_header.sample_rate) );
        frame->setSampleFormat( m_audio_header.sample_format==0?AUDIO_SAMPLE_FORMAT_8BIT:AUDIO_SAMPLE_FORMAT_16BIT );
        frame->setChannels( m_audio_header.channels==0?1:2 );
        frame->assign(0, temp, len);
    }
}

int     RtmpParser::flv_sample_rate_2_somo(int flv_sample_rate) {
    int somo_sample_rate = 0;
    switch(flv_sample_rate) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            somo_sample_rate = AUDIO_SAMPLE_RATE_44K;
            break;
    }

    return somo_sample_rate;
}
