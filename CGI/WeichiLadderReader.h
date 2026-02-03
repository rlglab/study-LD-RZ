#ifndef WEICHILADDERREADER_H
#define WEICHILADDERREADER_H

#include "BasicType.h"
#include "WeichiBoard.h"

/*
	calculate simple fast non-search ladder and slow search ladder
*/

class WeichiLadderReader
{
public:
	WeichiLadderReader();

	static bool isInSimpleFastLadder( const WeichiBoard& board, Color runColor, const WeichiGrid& runGrid, WeichiLadderType type );
	static WeichiLadderType getLadderType( const WeichiBoard& board, const WeichiBlock* block );
	static WeichiLadderType getLadderType( const WeichiBoard& board, const WeichiBlock* block, const uint lastLiberty );

	static bool isInComplicateLadder( WeichiBoard& board, OpenAddrHashTable& ht, const WeichiBlock* block );

	static WeichiBlockSearchResult save_ladder( WeichiBoard& board, OpenAddrHashTable& ht, const uint blockPos, const uint savePos );
	static WeichiBlockSearchResult kill_ladder( WeichiBoard& board, OpenAddrHashTable& ht, const uint blockPos, const Vector<uint,2> vKillPos );
};

#endif