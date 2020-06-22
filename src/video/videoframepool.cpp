#include "videoframepool.h"
#include "videoframe.h"
#include "common/logger.h"

#include <stdlib.h>

#define VIDEO_FRAME_SIZE_256K       256*1024
#define VIDEO_FRAME_SIZE_16K      	16*1024
#define VIDEO_PACKET_POOL_MAX_16K   	10
#define VIDEO_PACKET_POOL_MAX_256K   	5

#define __CLASS__ "VideoFramePool"

VideoFramePool::VideoFramePool()
: m_nGetCount(0)
, m_nFreeCount(0)
{
}

VideoFramePool::~VideoFramePool()
{
    reset();
}


VideoFrame* VideoFramePool::get(int size)
{
    m_nGetCount++;
    if( m_nGetCount%1000 == 0 ) {
        FUNLOG(Info, "video frame pool get, m_nGetCount=%d, m_nFreeCount=%d", m_nGetCount, m_nFreeCount);
    }

    VideoFrame* pFrame = NULL;
	if( size <= VIDEO_FRAME_SIZE_16K ) {
		if ( !m_arr16KFrames.empty() )
		{

			pFrame = m_arr16KFrames.front();
			m_arr16KFrames.pop_front();
		}
	    else {
            pFrame = new VideoFrame(VIDEO_FRAME_SIZE_16K);
	    }
	} else if( size <= VIDEO_FRAME_SIZE_256K ) {
		if ( !m_arr256KFrames.empty() )
		{
			pFrame = m_arr256KFrames.front();
			m_arr256KFrames.pop_front();
		}
	    else {
            pFrame = new VideoFrame(VIDEO_FRAME_SIZE_256K);
	    }
	} else {//too large and need to new size
        pFrame = new VideoFrame(size);
	}

	return pFrame;
}


void VideoFramePool::free(VideoFrame* pFrame)
{
    m_nFreeCount++;
	pFrame->clear();

	if( pFrame->capacity() == VIDEO_FRAME_SIZE_16K ) {
		if ( m_arr16KFrames.size() > VIDEO_PACKET_POOL_MAX_16K ) {
			delete pFrame;
			pFrame = NULL;
		} else {
			m_arr16KFrames.push_back(pFrame);
		}
	} else if( pFrame->capacity() == VIDEO_FRAME_SIZE_256K ) {
		if ( m_arr256KFrames.size() > VIDEO_PACKET_POOL_MAX_256K ) {
			delete pFrame;
			pFrame = NULL;
		} else {
			m_arr256KFrames.push_back(pFrame);
		}
	} else {
            //video frame pool free, too large ,and need to delete
            delete pFrame;
            pFrame = NULL;
	}
}

void VideoFramePool::reset() {
    for ( frame_pool_t::iterator io = m_arr16KFrames.begin(); io != m_arr16KFrames.end(); ++io ) {
        delete (*io);
        *io = NULL;
    }
    m_arr16KFrames.clear();

    for ( frame_pool_t::iterator io = m_arr256KFrames.begin(); io != m_arr256KFrames.end(); ++io ) {
        delete (*io);
        *io = NULL;
    }
    m_arr256KFrames.clear();

    m_nGetCount = 0;
    m_nFreeCount = 0;
}
