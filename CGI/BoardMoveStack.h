#ifndef BOARDMOVESTACK_H
#define BOARDMOVESTACK_H

#include "BasicType.h"
#include "BoardStatus.h"
#include "WeichiBlock.h"

struct BoardMoveStack {
	BoardStatus m_status;
	WeichiBlock* m_mainBlock;
	WeichiBitBoard m_bmMainBlockStoneNbrMap;
	Vector<WeichiBlock*,4> m_vCombineBlocks;
	Vector<WeichiBlock*,4> m_vDeadBlocks;
};

#endif