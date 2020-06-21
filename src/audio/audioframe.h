#pragma once

#include <stdint.h>
#include <string>

class AudioFrame {
public:
    AudioFrame(int capacity);
    ~AudioFrame();

public:
	bool	assign(uint64_t stamp, const char* data, int len);
    bool	assign(uint64_t stamp, const std::string& payload);
	void	clear();
	void	setCodec(int codec) {m_nCodec = codec;}
	void	setSampleRate(int sample_rate) { m_nSampleRate = sample_rate; }
	void	setSampleFormat(int sample_format) { m_nSampleFormat = sample_format; }
	void	setChannels(int channels) { m_nChannels=channels;}

public:
	inline uint64_t		stamp() { return m_nStamp; }
	inline char*		data() { return m_pData; }
	inline int			size() { return m_nSize; }
	inline int			capacity() { return m_nCapacity; }
	inline int			codec() { return m_nCodec; }
	inline int			sample_rate() { return m_nSampleRate; }
	inline int			sample_format() { return m_nSampleFormat; }
	inline int 			channels() { return m_nChannels;}

private:
	uint64_t	m_nStamp;
	char*		m_pData;
	int			m_nSize;
	int			m_nCapacity;
	int			m_nCodec;
	int			m_nSampleRate;
	int			m_nSampleFormat;
	int			m_nChannels;
};
