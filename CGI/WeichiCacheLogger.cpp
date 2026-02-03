#include "WeichiCacheLogger.h"
#include "TimeSystem.h"

void WeichiCacheLogger::openCacheLogFile( const WeichiBoard& board, int minSimulation, int maxSimulation )
{
	if( !WeichiConfigure::DoPlayoutCacheLog ) { return; }

	closeCacheLogFile();

	string sPrefix = TimeSystem::getTimeString("Ymd_His");
	for( uint color=0; color<3; color++ ) {
		string sFileName = sPrefix + "_" + toChar(static_cast<Color>(color)) + "(" + ToString(minSimulation) + "-" + ToString(maxSimulation) + ")" + ".sgf";
		m_fLogger[color].open(sFileName.c_str(),ios::out);
		m_fLogger[color] << "(;FF[4]CA[UTF-8]AP[GoGui:1.4-unknown]SZ["
						 << WeichiConfigure::BoardSize << "]KM[" << WeichiConfigure::komi << "];" << m_sBoardString;
	}
}

void WeichiCacheLogger::closeCacheLogFile()
{
	if( !WeichiConfigure::DoPlayoutCacheLog ) { return; }
	for( uint color=0; color<3; color++ ) {
		if( !m_fLogger[color].is_open() ) { continue; }
		m_fLogger[color] << ")";
		m_fLogger[color].close();
	}
}

void WeichiCacheLogger::writeTreeMove( const WeichiBoard& board, const Vector<WeichiMove,MAX_GAME_LENGTH>& vMoves )
{
	for( uint i=0; i<vMoves.size(); i++ ) {
		m_sCacheString += ";" + vMoves[i].toSgfString() + "C[uct_move]";
	}
}

void WeichiCacheLogger::writeOneMove( const WeichiBoard& board, const WeichiMove& move, const string& sComment )
{
	if( board.isPreSimulationStatus() ) { return; }

	if( !m_bmCachePlayRegion.BitIsOn(move.getPosition()) ) { m_sCacheString += move.toSgfString(true); return; }

	// write cache region move
	m_sCacheString += move.toSgfString();
	m_sCacheString += "C[" + sComment + "]";
	if( board.hasPrevOwnMove() ) { m_sCacheString += "MA[" + board.getPrevOwnMove().toSgfString(false) + "]"; }
	m_sCacheString += ";";
}

void WeichiCacheLogger::writeCacheLogFile( const WeichiBoard& board, const Vector<Color,MAX_NUM_GRIDS>& territory )
{
	if( simulation%SIMULATION_INTERVAL==0 ) { openCacheLogFile(board,simulation,simulation+SIMULATION_INTERVAL); }

	Color result = checkCacheResult(board,territory);
	m_fLogger[result] << endl << "(" << m_sCacheString << ")";
	m_sCacheString = "";
	simulation++;
}

void WeichiCacheLogger::writePresetBitBoard( string sHeader, WeichiBitBoard bmBoard )
{
	if( bmBoard.empty() ) { return; }

	uint pos;
	m_sCacheString += sHeader;
	while( (pos=bmBoard.bitScanForward())!=-1 ) {
		m_sCacheString += "[" + WeichiMove(pos).toSgfString(false) + "]";
	}
}

Color WeichiCacheLogger::checkCacheResult( const WeichiBoard& board, const Vector<Color,MAX_NUM_GRIDS>& territory )
{
	uint pos,black,white,none;
	WeichiBitBoard bmResultRegion = m_bmCacheResultRegion;

	black = white = none = 0;
	while( (pos=bmResultRegion.bitScanForward())!=-1 ) {
		if( territory[pos]==COLOR_BLACK ) { black++; }
		else if( territory[pos]==COLOR_WHITE ) { white++; }
		else { none++; }
	}

	if( black>0 && white==0 && none==0 ) { return COLOR_BLACK; }
	else if( white>0 && black==0 && none==0 ) { return COLOR_WHITE; }
	else { return COLOR_NONE; }
}