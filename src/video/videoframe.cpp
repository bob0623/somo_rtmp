#include "videoframe.h"

#include "common/logger.h"
#include <stdlib.h>
#include <string.h>

#define __CLASS__ "VideoFrame"

VideoFrame::VideoFrame(int capacity)
: m_nStamp(0)
, m_nSize(0)
, m_nCapacity(capacity)
, m_nType(0)
, m_nWidth(0)
, m_nHeight(0)
, m_nFps(0)
{
	m_pData = new char[capacity];
}

VideoFrame::~VideoFrame() {
	delete[] m_pData;
}

bool	VideoFrame::assign(uint64_t stamp, const char* data, int len) {
	if( len > m_nCapacity ) {
        FUNLOG(Error, "rtmp frame assign, capacity<size, size=%d, capacity=%d", len, m_nCapacity);
		return false;
	}
	
	m_nStamp = stamp;
	m_nSize = len;
	memcpy(m_pData, data, len);
	return true;
}

bool	VideoFrame::assign(uint64_t stamp, const std::string& payload) {
	if( payload.size() >= m_nCapacity ) {
        FUNLOG(Error, "rtmp frame assign, capacity<size, size=%d, capacity=%d", payload.size(), m_nCapacity);
		return false;
	}
	
	m_nStamp = stamp;
	m_nSize = payload.size();
	memcpy(m_pData, payload.c_str(), m_nSize);
	return true;
}

void    VideoFrame::append(const char* data, int len) {
	memcpy(m_pData+m_nSize, data, len);
	m_nSize+=len;
}

void 	VideoFrame::setType(int type) {
	m_nType = type;
}

void    VideoFrame::setWidth(int width)
{
    m_nWidth = width;
}

void    VideoFrame::setHeight(int height)
{
    m_nHeight = height;
}

void	VideoFrame::setFps(int fps) {
	m_nFps = fps;
}

void	VideoFrame::clear() {
//    memset(m_pData, 0, m_nSize);
	m_nStamp = 0;
	m_nSize = 0;
	m_nType = 0;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nFps = 0;
}
