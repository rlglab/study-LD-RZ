#ifndef WEICHIBADMOVEREADER_H
#define WEICHIBADMOVEREADER_H

#include "BasicType.h"
#include "WeichiBoard.h"

/*
	bad move means we will never put stone here
*/

class WeichiBadMoveReader
{
public:
	WeichiBadMoveReader();

	static WeichiBadMoveType getBadMoveType( const WeichiBoard& board, const WeichiMove& move );
	static bool isLifeTerritory( const WeichiBoard& board, const WeichiMove& move );
	static bool isPotentialTrueEye( const WeichiBoard& board, const WeichiMove& move );
	/*!
		@brief  OOOOO
				O!X!O
				OXX.O
				O!.OO
				OOOOO
				if white put its stone in exclamation mark, black can make nakade shape, white block is dead (even if black tenuki)
				therefore we want to avoid this situation
		@author kwchen
		@return true if move is suicide by above situation
	*/
	static bool isFillingOwnEyeShape ( const WeichiBoard& board, const WeichiMove& move );
	/*!
		@brief  check if move is making nakade shape in opponent closed area
		@author kwchen
		@return true if move is suicide by above situation
	*/
	static bool isNotMakingNakadeInOppCA ( const WeichiBoard& board, const WeichiMove& move, const PredictPlayInfo& moveInfo );
	/*!
		@brief  check if move is making nakade shape and kill opponent
		@author kwchen
	*/
	static bool isMakingNakadeShape( const WeichiBoard& board, const WeichiMove& move );
	static bool isTwoLibFalseEye( const WeichiBoard& board, const WeichiMove& move, const PredictPlayInfo& moveInfo );
	/*!
		@brief check if the move is a BadSelfAtari
		@author ccshih
		@return true if it is badSelfAtari
	*/
	static bool isBadSelfAtari( const WeichiBoard& board, const WeichiMove& move, const PredictPlayInfo& moveInfo );
	/*!
		@brief  check if move is atari for both black and white
		@author kwchen
		@return true if the move cannot play because it is seki position
	*/
	static bool isMutualAtari( const WeichiBoard& board, const WeichiMove& move, const PredictPlayInfo& myInfo );
	/*!
		@brief  check if move is snapback or not
		@author 5D
		@return true if it is snapback
	*/
	static bool isSnapback( const WeichiBoard& board, const WeichiMove& move );

private:
	static WeichiBadMoveType getBadMoveType( const WeichiBoard& board, const WeichiMove& move, const PredictPlayInfo& moveInfo );
	static bool isSharedLibertyWithNbrBlock( const WeichiBoard& board, const PredictPlayInfo& info );

	static bool invariance_snapback( const WeichiBoard& board, const WeichiMove& move );
};

#endif