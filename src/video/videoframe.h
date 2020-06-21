#pragma once

#include <stdint.h>
#include <string>

class VideoFrame {
public:
    VideoFrame(int capacity);
    ~VideoFrame();

public:
    bool	assign(uint64_t stamp, const char* data, int len);
    bool	assign(uint64_t stamp, const std::string& payload);
    void    append(const char* data, int len);
	void	clear();
    void    setType(int type);
    void    setWidth(int width);
    void    setHeight(int height);
    void    setFps(int fps);

public:
	inline uint64_t		stamp() { return m_nStamp; }
	inline char*		data() { return m_pData; }
	inline int			size() { return m_nSize; }
	inline int			capacity() { return m_nCapacity; }
    inline int          type() { return m_nType; }
    inline int          width() {return m_nWidth;}
    inline int          height() {return m_nHeight;}
    inline int          fps() {return m_nFps; }

private:
	uint64_t	m_nStamp;
	char*		m_pData;
	int			m_nSize;
	int			m_nCapacity;
    int         m_nType;
    int         m_nWidth;
    int         m_nHeight;
    int         m_nFps;
};
