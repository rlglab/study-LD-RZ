#ifndef WEICHISEMEAIHANDLER_H
#define WEICHISEMEAIHANDLER_H

#include "BasicType.h"
#include "WeichiBoard.h"
#include "WeichiSemeaiResult.h"

class WeichiSemeaiHandler
{
private:
	static const int MIN_SEMEAI_LIBERTY = 2;
	static const int MAX_SEMEAI_LIBERTY = 5;

public:
	WeichiSemeaiHandler() {}

	static WeichiSemeaiResult getSemeaiType( const WeichiBoard& board, const WeichiBlock* block, const WeichiBlock* semeaiBlock, Vector<uint,MAX_NUM_GRIDS>& vSemeaiPoints );

private:
	static void findBasicSemeaiPoint( const WeichiBoard& board, const WeichiBlock* block, WeichiBitBoard bmLiberty, const WeichiBlock* semeaiBlock, Vector<uint,MAX_NUM_GRIDS>& vSemeaiPoints );
	static uint adjustSemeaiPoint( const WeichiBoard& board, const WeichiMove move, const PredictPlayInfo& moveInfo );
	static bool isBasicSemeai( const WeichiBoard& board, const WeichiBlock* block, WeichiBitBoard bmLiberty );
};

#endif