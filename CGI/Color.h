#ifndef COLOR_H
#define COLOR_H

#include "BasicType.h"

enum Color
{
    COLOR_NONE = 0u,
    COLOR_BLACK = 1u,
    COLOR_WHITE = 2u,
    COLOR_BORDER = 3u,
	COLOR_SIZE = 4u
};

inline Color AgainstColor ( Color c ) {
    assert ( c == COLOR_BLACK || c == COLOR_WHITE ) ;
    // return ( c == COLOR_BLACK ? COLOR_WHITE : COLOR_BLACK ) ;
    return (Color)(c^COLOR_BORDER) ;
}

inline Color toColor ( char c )
{
    switch ( c ) {
    case 'B': case 'b': return COLOR_BLACK ; break;
    case 'W': case 'w': return COLOR_WHITE ; break;
    default: return COLOR_NONE ; break;
    }
}

inline char toChar ( Color c )
{
    switch ( c ) {
	case COLOR_NONE : return 'N' ; break;
    case COLOR_BLACK : return 'B' ; break;
    case COLOR_WHITE : return 'W' ; break;
    default: return '?'; break;
    }
}

inline string toString ( Color c )
{
    switch ( c ) {
	case COLOR_NONE : return "None" ; break;
    case COLOR_BLACK : return "Black" ; break;
    case COLOR_WHITE : return "White" ; break;
    default: return "?"; break;
    }
}

inline string toGuiString ( Color c )
{
#if defined(_WIN32) || defined(_WIN64) || defined(_WIN32_WINNT)
	switch(c) {
	case COLOR_BLACK:	return "¡´";
	case COLOR_WHITE:	return "¡³";
	default:			return " .";
	}
#else
	switch(c) {
	case COLOR_BLACK:	return " #";
	case COLOR_WHITE:	return " O";
	default:			return " .";
	}
#endif
}

inline bool ColorIsEmpty ( Color c )
{
    return c == COLOR_NONE ;
}

inline bool ColorNotEmpty ( Color c )
{
    return c != COLOR_NONE ;
}

#endif 
