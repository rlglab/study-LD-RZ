#ifndef DUAL_H
#define DUAL_H

#include "Color.h"

template<class _T>
class Dual
{
public:
    _T m_black, m_white ;
    
    inline Dual(){reset();}
	inline Dual(_T black, _T white) { m_black = black; m_white = white; }

    inline _T& get(Color c) { assert(ColorNotEmpty(c)); return (c==COLOR_BLACK?m_black:m_white); }
    inline const _T& get(Color c) const { assert(ColorNotEmpty(c)); return (c==COLOR_BLACK?m_black:m_white); }

    inline void set(Color c, const _T& v) { assert(ColorNotEmpty(c)); (c==COLOR_BLACK?m_black:m_white) = v ; }

    inline void reset() { m_black=_T(); m_white=_T(); }

};

#endif  // DUAL_H