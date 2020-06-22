#pragma once


typedef struct
{
    unsigned int profile_idc;
    unsigned int level_idc;
    
    unsigned int width;
    unsigned int height;
    unsigned int fps;       //SPS中可能不包含FPS信息
} sps_info;

int h264_parse_sps(const unsigned char *data, unsigned int dataSize, sps_info *info);


