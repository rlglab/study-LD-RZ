#ifndef __TEESTREAM_H__
#define __TEESTREAM_H__

#include <iostream>

// create tee stream buffer and stream
// reference: http://wordaligned.org/articles/cpp-streambufs
class TeeBuffer: public std::streambuf
{
public:
	// Construct a streambuf which tees output to both input
	// streambufs.
	TeeBuffer(std::streambuf * sb1, std::streambuf * sb2)
		: m_sb1(sb1)
		, m_sb2(sb2)
	{
	}

	void setStreambuf(std::streambuf * sb1, std::streambuf * sb2)
	{
		m_sb1 = sb1;
		m_sb2 = sb2;
	}
private:
	// This tee buffer has no buffer. So every character "overflows"
	// and can be put directly into the teed buffers.
	virtual int overflow(int c)
	{
		if ( c == EOF ) { return !EOF; }
		else {
			int r1 = c;
			int r2 = c;
			if ( m_sb1 ) { r1 = m_sb1->sputc(c); }
			if ( m_sb2 ) { r2 = m_sb2->sputc(c); }
			return r1 == c || r2 == c ? c : EOF;
		}
	}

	// Sync both teed buffers.
	virtual int sync()
	{
		int r1 = 0;
		int r2 = 0;
		if ( m_sb1 ) { r1 = m_sb1->pubsync(); }
		if ( m_sb2 ) { r2 = m_sb2->pubsync(); }
		return r1 == 0 || r2 == 0 ? 0 : -1;
	}   
private:
	std::streambuf * m_sb1;
	std::streambuf * m_sb2;
};

class TeeStream : public std::ostream
{
public:
	// Construct an ostream which tees output to the supplied
	// ostreams.
	TeeStream(std::streambuf * sb1, std::streambuf * sb2)
		: std::ostream(&m_tbuf)
		, m_tbuf(sb1, sb2)
	{
	}

	void setStreambuf(std::streambuf * sb1, std::streambuf * sb2)
	{
		m_tbuf.setStreambuf(sb1, sb2);
	}
private:
	TeeBuffer m_tbuf;
};

#endif