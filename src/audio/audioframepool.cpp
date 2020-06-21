#include "audioframepool.h"
#include "audioframe.h"

#include <logger.h>
#include <stdlib.h>

#define __CLASS__ "AudioFramePool"

#define AUDIO_PACKET_POOL_MAX   	100


AudioFramePool::AudioFramePool()
: m_nGetCount(0)
, m_nFreeCount(0)
{
}

AudioFramePool::~AudioFramePool()
{
    reset();
}


AudioFrame* AudioFramePool::get(int size)
{
    m_nGetCount++;
    if( m_nGetCount%300000 == 0 ) {
        FUNLOG(Info, "video frame pool get, m_nGetCount=%d, m_nFreeCount=%d", m_nGetCount, m_nFreeCount);
    }

    AudioFrame* pFrame = NULL;
	if ( !m_arrFrames.empty() )
	{

		pFrame = m_arrFrames.front();
		m_arrFrames.pop_front();
	}
    else {
        pFrame = new AudioFrame(1200);
    }

	return pFrame;
}


void AudioFramePool::free(RtmpAudioFrame* pFrame)
{
    m_nFreeCount++;
	pFrame->clear();

	m_mutex.lock();	
	if ( m_arrFrames.size() > AUDIO_PACKET_POOL_MAX ) {
		delete pFrame;
		pFrame = NULL;
	} else {
		m_arrFrames.push_back(pFrame);
	}
	m_mutex.unlock();
}

void AudioFramePool::reset() {
	m_mutex.lock();
    for ( frame_pool_t::iterator io = m_arrFrames.begin(); io != m_arrFrames.end(); ++io ) {
        delete (*io);
    }
    m_arrFrames.clear();
    m_mutex.unlock();
}
