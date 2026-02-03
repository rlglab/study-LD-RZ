#ifndef COLORMESSAGE_H
#define COLORMESSAGE_H

#include "BasicType.h"

enum ANSITYPE {
	ANSITYPE_NORMAL,
	ANSITYPE_BOLD,
	ANSITYPE_UNDERLINE,

	ANSITYPE_SIZE
};

enum ANSICOLOR {
	ANSICOLOR_BLACK,
	ANSICOLOR_RED,
	ANSICOLOR_GREEN,
	ANSICOLOR_YELLOW,
	ANSICOLOR_BLUE,
	ANSICOLOR_PURPLE,
	ANSICOLOR_CYAN,
	ANSICOLOR_WHITE,

	ANSICOLOR_SIZE
};

inline string getColorMessage( string sMessage, ANSITYPE type, ANSICOLOR textColor, ANSICOLOR backgroundColor )
{
#if defined(_WIN32) || defined(_WIN64) || defined(_WIN32_WINNT)
	return sMessage;
#else
	const int ansitype[ANSITYPE_SIZE] = {0,1,4};
	return "\33[" + ToString(ansitype[type]) + ";3" + ToString(textColor) + ";4" + ToString(backgroundColor) + "m" + sMessage + "\33[0m";
#endif
}

#endif 
