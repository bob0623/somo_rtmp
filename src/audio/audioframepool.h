#pragma once

#include "common/singleton.h"
#include <deque>

class AudioFrame;
class AudioFramePool : public Singleton<AudioFramePool>
{   
public:
    AudioFramePool();
    ~AudioFramePool();

public:
    AudioFrame*	get(int size);
    void		free(AudioFrame* pFrame);
    void        reset();

private:
    typedef std::deque<AudioFrame*>	frame_pool_t;
	frame_pool_t m_arrFrames;
	int			m_nGetCount;
	int			m_nFreeCount;
};
