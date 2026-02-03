#ifndef WEICHILOWLIBREADER_H
#define WEICHILOWLIBREADER_H

#include "BasicType.h"
#include "WeichiBoard.h"

/*
	calculate can save or kill lower liberty (1~2 liberty)
*/

class WeichiLowLibReader
{
public:
	WeichiLowLibReader();

	static void save1Lib( const WeichiBoard& board, const OpenAddrHashTable& ht, const WeichiBlock* block, WeichiBitBoard& bmSave, WeichiBitBoard& bmUnknown, WeichiBitBoard& bmSkip );
	static bool kill1Lib( const WeichiBoard& board, const OpenAddrHashTable& ht, const WeichiBlock* block );
	static bool kill1Lib( const WeichiBoard& board, const OpenAddrHashTable& ht, const WeichiBlock* block, uint lastLiberty );
	static void kill2Lib( const WeichiBoard& board, const OpenAddrHashTable& ht, const WeichiBlock* block, WeichiBitBoard& bmKill, WeichiBitBoard& bmUnknwon, WeichiBitBoard& bmSkip );
	static bool canKillNbr1Lib( const WeichiBoard& board, const OpenAddrHashTable& ht, const WeichiBlock* block );
	static bool hasNbr1LibCanKill( const WeichiBoard& board, const OpenAddrHashTable& ht, const WeichiBlock* block, WeichiBitBoard& bmSave, WeichiBitBoard& bmEatNbr1Lib, WeichiBitBoard& bmSkip );
	static bool isSemeaiBlock( const WeichiBoard& board, const WeichiBlock* block );

//private:
	static void check1LibRunWillBeKillAgain( const WeichiBoard& board, const uint runPos, const Color myColor, const PredictPlayInfo& moveInfo, WeichiBitBoard& bmSave, WeichiBitBoard& bmUnknown, WeichiBitBoard& bmSkip );
	static bool killNeighbor1Lib( const WeichiBoard& board, const OpenAddrHashTable& ht, const WeichiBlock* block, WeichiBitBoard& bmKill );
	static bool isBitBoardInBlockCA( const WeichiBoard& board, const WeichiBlock* block, const WeichiBitBoard& bmBoard );
	static bool kill2LibDirectly( const WeichiBoard& board, const WeichiBlock* block, uint killPos, uint runPos, WeichiBitBoard& bmKill, WeichiBitBoard& bmUnknown );
	static bool kill2LibByCloseDoor( const WeichiBoard& board, const WeichiBlock* block, uint pos1, uint pos2, WeichiBitBoard& bmKill );
	static uint kill2LibByCloseDoor( const WeichiBoard& board, const WeichiBlock* block, uint pos1, uint pos2 );
	static bool checkCloseDoor( const WeichiBoard& board, const WeichiGrid& grid, Color myColor );
	static bool isDirectionBlockLargeThan2Lib( const WeichiBoard& board, const WeichiGrid& grid, const uint dir );
};

#endif