#include "audioframe.h"

#include <logger.h>
#include <stdlib.h>
#include <string.h>

#define __CLASS__ "AudioFrame"

AudioFrame::AudioFrame(int capacity)
: m_nStamp(0)
, m_nSize(0)
, m_nCapacity(capacity)
, m_nCodec(0)
, m_nSampleRate(0)
, m_nSampleFormat(0)
, m_nChannels(0)
{
	m_pData = new char[capacity];
}

AudioFrame::~AudioFrame() {
	delete[] m_pData;
}

bool	AudioFrame::assign(uint64_t stamp, const char* data, int len) {
	if( len >= m_nCapacity ) {
        FUNLOG(Error, "rtmp frame assign, capacity<size, size=%d, capacity=%d", len, m_nCapacity);
		return false;
	}

	m_nStamp = stamp;
	m_nSize = len;
	memcpy(m_pData, data, len);

	return true;
}

bool	AudioFrame::assign(uint64_t stamp, const std::string& payload) {
	if( payload.size() >= m_nCapacity ) {
        FUNLOG(Error, "rtmp frame assign, capacity<size, size=%d, capacity=%d", payload.size(), m_nCapacity);
		return false;
	}

	m_nStamp = stamp;
	m_nSize = payload.size();
	memcpy(m_pData, payload.c_str(), m_nSize);
	return true;
}

void	AudioFrame::clear() {
	m_nStamp = 0;
	m_nSize = 0;
	m_nCodec = 0;
	m_nSampleRate = 0;
	m_nSampleFormat = 0;
	m_nChannels = 0;
}
