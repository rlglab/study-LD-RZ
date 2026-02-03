#ifndef PREDICTPLAYINFO_H
#define PREDICTPLAYINFO_H

#include "BasicType.h"

class PredictPlayInfo {
public:
	bool m_bHasDeadBlocks;
	bool m_bHasCombineBlock;
	uint m_liberty;
	uint m_blockNumStone;
	WeichiBitBoard m_bmDeadStone;
	WeichiBitBoard m_bmNewLiberty;
	WeichiBitBoard m_bmPreNbrStoneMap;
	WeichiBitBoard m_bmNewNbrStoneMap;
	BoundingBox m_blockBoundingBox;

	void reset()
	{
		m_bHasDeadBlocks = false;
		m_bHasCombineBlock = false;
		m_liberty = -1;
		m_blockNumStone = 0;
		m_bmDeadStone.Reset();
		m_bmNewLiberty.Reset();
		m_bmPreNbrStoneMap.Reset();
		m_bmNewNbrStoneMap.Reset();
		m_blockBoundingBox = BoundingBox();
	}
};
#endif