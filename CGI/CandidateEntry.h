#ifndef CANDIDATEENTRY_H
#define CANDIDATEENTRY_H

#include "BasicType.h"

class CandidateEntry
{
private:
	uint m_position;
	double m_dScore;
public:
	CandidateEntry() {}
	CandidateEntry( uint position, double dScore ) : m_position(position), m_dScore(dScore) {}
	inline void clear( uint position )
	{
		m_position = position;
		m_dScore = 1.0f;
	}
	inline uint getPosition() const { return m_position; }
	inline void setPosition( uint position ) { m_position = position; }
	inline double getScore() const { return m_dScore; }
	inline void setScore( double dScore ) { m_dScore = dScore; }
	inline bool operator< ( const CandidateEntry& rhs ) const { return m_dScore>rhs.m_dScore; }
};


#endif 