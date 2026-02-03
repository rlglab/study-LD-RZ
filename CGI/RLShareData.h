#ifndef RLSHAREDATA_H
#define RLSHAREDATA_H

#include "BasicType.h"

class RLShareData {
public:
	uint m_iteration;
	uint m_totalGames;
	string m_sOpponent;
	string m_sSgfDirectory;
	string m_sModelDirectory;
	string m_sDirectoryPrefix;
};

#endif