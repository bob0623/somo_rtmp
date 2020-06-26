#include "videonaluparser.h"

#include "common/logger.h"
#include <stdlib.h>

#define __CLASS__ "VideoNaluParser"

VideoNaluParser::VideoNaluParser() {

}

void 	VideoNaluParser::parse(const char* data, int size, std::vector<NaluItem>& nalus) {
    int first_nalu_start = 0;
	int first_nalu_end = 0;
	char* nalu_start = const_cast<char*>(data);
    char* nalu_end = nalu_start + size -1;
    char* nalu = nalu_start;
    int type = 0;
    int loops = 0;  //max loop<100; void infinit loop:

    while( nalu < nalu_end -3 ) {
        loops++;
        if( loops >= 100 ) {
            FUNLOG(Error, "rtmp parse H264, meet infinit loop, loops=%d", loops);
            break;
        }

        int nalu_size = find_nal_unit(nalu, data+size-nalu, &first_nalu_start, &first_nalu_end, &type);
        if( nalu_size == 0  || nalu_size>size ) {
            FUNLOG(Error, "rtmp parse H264, invalid nalu_size=%d, size=%d", nalu_size, size);
            break;
        }
        if( first_nalu_start != 0 ) {
            FUNLOG(Error, "rtmp parse H264, first_nalu_start=%d, nalu.size%d, data.size=%d", first_nalu_start, nalu_size, size);
        }

        
        nalu = nalu + first_nalu_start + nalu_size;

        NaluItem nalu_item;
        nalu_item.data = nalu+first_nalu_start;
        nalu_item.len = nalu_size;
        nalus.push_back(nalu_item);
    }
}

int 	VideoNaluParser::find_nal_unit(char* buf, int size, int* nalu_start, int* nalu_end, int* type) {
    int i;
    *nalu_start = 0;
    *nalu_end = 0;
    *type = 0;

    i = 0;
    //dump_8bytes(buf);
    while (   //( next_bits( 24 ) != 0x000001 && next_bits( 32 ) != 0x00000001 )
            (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0x01) &&
            (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0 || buf[i+3] != 0x01)
            ) {
        i++; // skip leading zero
        if (i+4 >= size) {
            FUNLOG(Error, "h264 parser, not find NALU start!!! buf[0]=%d, buf[1]=%d, buf[2]=%d, buf[3]=%d", buf[0], buf[1], buf[2], buf[3]);
            return 0;
        } // did not find nal start
    }

    *nalu_start = i;
    if( buf[i+2] == 0x01 ) {
        *type = get_nal_type(buf + *nalu_start+3);
    } else if( buf[i+3] == 0x01 ) {
        *type = get_nal_type(buf + *nalu_start+4);
    }
    //FUNLOG(Info, "h264 parser, find startcode, i=%d", i);

    i+=3;
    while (   //( next_bits( 24 ) != 0x000000 && next_bits( 24 ) != 0x000001 )
            (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0) &&
            (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0x01)
            ) {
        i++;
        // FIXME the next line fails when reading a nal that ends exactly at the end of the data
        if (i+3 >= size) {
            *nalu_end = size;
            //FUNLOG(Error, "h264 parser, not find NALU end!!!, nalu_start=%d, nalu_end=%d, nalu_size=%d, size=%d",
            //    nalu_start, nalu_end, (*nalu_end-*nalu_start), size);
            return (*nalu_end-*nalu_start);
        } // did not find nal end, stream ended first
    }
    *nalu_end = i;
    return (*nalu_end-*nalu_start);
}

int		VideoNaluParser::get_nal_type(char* buf) {
    int b1 = buf[0] & 31;
    return b1;
}

void    VideoNaluParser::dump_8bytes(char* buf) {
    FUNLOG(Info, "h264 parser dump 8 bytes, %d-%d-%d-%d-%d-%d-%d-%d", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    FUNLOG(Info, "h264 parser dump 8 bytes, %d-%d-%d-%d-%d-%d-%d-%d", buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
}