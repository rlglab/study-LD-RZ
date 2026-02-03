#ifndef WEICHIPLAYOUTRESULT_H
#define WEICHIPLAYOUTRESULT_H

#include "strops.h"
#include "Color.h"

class WeichiPlayoutResult
{
private:
    float m_score ;
	float m_value ;
public:
    WeichiPlayoutResult ( float score )
        : m_score ( score )
    {   
    }

    inline float getScore ()
    {
        return m_score;
    }
	inline void setValue( float fValue )
	{
		m_value = fValue;
	}
	inline float getValue ()
	{
		return m_value;
	}
    inline Color getWinner () 
    {
        return ( m_score == 0.0f ? COLOR_NONE : 
            (m_score > 0.0f ? COLOR_BLACK : COLOR_WHITE) );
    }
    inline std::string toString() const 
    {
		if (m_score == 0) { return "0"; }
		else if (m_score == MAX_NUM_GRIDS || m_score == -MAX_NUM_GRIDS) {
			if (m_score > 0) { return "B+R"; }
			else { return  "W+R"; }
		} else {
			if (m_score > 0) { return "B+" + ToString<float>(m_score); }
			else { return "W+" + ToString<float>(-m_score); }
		}
    }

};

#endif  
