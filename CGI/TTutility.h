#ifndef H_TT_UTILITY
#define H_TT_UTILITY

#include "TTentry.h"
#include "WeichiBoard.h"

class WeichiThreadState;

class TtUtility
{
	TtUtility() {}
public:
	static TTentry createTtEntry(const WeichiThreadState& state, const TTentry& ttEntry);	

private:
	static int getKoPosition(const WeichiThreadState& state, const TTentry& entry);
	static int getForbiddenKoPosition(const WeichiThreadState& state, const TTentry& entry);
	static int getEatKoPosition(const WeichiThreadState& state, const TTentry& entry);
};

#endif
