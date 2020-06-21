#pragma once

#include "common/singleton.h"
#include <deque>

class VideoFrame;
class VideoFramePool : public Singleton<VideoFramePool>
{   
public:
    VideoFramePool();
    ~VideoFramePool();

public:
    VideoFrame*	get(int size);
    void		free(VideoFrame* pFrame);
    void        reset();

private:
    typedef std::deque<VideoFrame*>	frame_pool_t;
	frame_pool_t	m_arr256KFrames;
	frame_pool_t	m_arr16KFrames;
	int			m_nGetCount;
	int			m_nFreeCount;
};
