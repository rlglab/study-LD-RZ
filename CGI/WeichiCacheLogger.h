#ifndef WEICHICACHELOGGER_H
#define WEICHICACHELOGGER_H

#include "BasicType.h"
#include "Dual.h"
#include "Color.h"
#include "WeichiBoard.h"

class WeichiCacheLogger
{
private:
	int simulation;
	fstream m_fLogger[3];	// for black, white, both
	string m_sCacheString;
	string m_sBoardString;
	Dual<WeichiBitBoard> m_bmBoard;
	WeichiBitBoard m_bmCachePlayRegion;
	WeichiBitBoard m_bmCacheResultRegion;

	static const int SIMULATION_INTERVAL = 1000;

public:
	WeichiCacheLogger() {}

	void reset( const WeichiBoard& board ) {
		simulation = 0;
		m_sBoardString = board.toMoveString();
	}
	void openCacheLogFile( const WeichiBoard& board, int minSimulation, int maxSimulation );
	void closeCacheLogFile();
	void writeTreeMove( const WeichiBoard& board, const Vector<WeichiMove,MAX_GAME_LENGTH>& vMoves );
	void writeOneMove( const WeichiBoard& board, const WeichiMove& move, const string& sComment );
	void writeCacheLogFile( const WeichiBoard& board, const Vector<Color,MAX_NUM_GRIDS>& territory );

	inline void setCachePlayRegion( WeichiBitBoard bmCachePlayRegion ) { m_bmCachePlayRegion = bmCachePlayRegion; }
	inline void setCacheResultRegion( WeichiBitBoard bmCacheResultRegion ) { m_bmCacheResultRegion = bmCacheResultRegion; }

	inline WeichiBitBoard getCachePlayRegion() const { return m_bmCachePlayRegion; }
	inline WeichiBitBoard getCacheResultRegion() const { return m_bmCacheResultRegion; }

private:
	void writePresetBitBoard( string sHeader, WeichiBitBoard bmBoard );
	Color checkCacheResult( const WeichiBoard& board, const Vector<Color,MAX_NUM_GRIDS>& territory );
};

#endif