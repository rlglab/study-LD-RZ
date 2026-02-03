#include "WeichiDynamicKomi.h"
#include "WeichiConfigure.h"
#include "WeichiGlobalInfo.h"

namespace WeichiDynamicKomi 
{
	int Handicap;
	int Internal_Handicap_komiInit;
	float Internal_komi;
	float Old_Internal_komi;
	float BlackWinStoneNum;
	float WhiteWinStoneNum;

	void adjustDykomi( int numStone, unsigned int numMove )
	{
		int komiDiff = calculateDynamicKomiDiff(numStone,numMove);
		setInternalKomi(komiDiff);
	}

	void setInternalKomi( int komiDiff )
	{
		Old_Internal_komi = Internal_komi;
		Internal_komi = WeichiConfigure::komi + Handicap + komiDiff;
		BlackWinStoneNum = (WeichiConfigure::TotalGrids+Internal_komi)/2 + 1;
		WhiteWinStoneNum = (WeichiConfigure::TotalGrids-Internal_komi)/2 + 1;
	}

	void setHandicapConfigure( int handicap )
	{
		Handicap = handicap;
		Internal_Handicap_komiInit = Handicap;
		if( WeichiConfigure::use_dynamic_komi && Internal_Handicap_komiInit>0 ) {
			Internal_Handicap_komiInit += static_cast<int>(Handicap*7.5-WeichiConfigure::komi);
		}

		setInternalKomi(0);
	}

	int calculateDynamicKomiDiff( int numStone, unsigned int numMove )
	{
		if( WeichiConfigure::BoardSize!=19 ) { return Internal_Handicap_komiInit; }

		//double winrate = WeichiGlobalInfo::getTreeInfo().m_bestSD.getMean();
		const Territory& territory = WeichiGlobalInfo::getSearchInfo().m_territory;
		double bCount = 0;
		double wCount = WeichiConfigure::komi + Handicap;
		for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
			double dTerritory = territory.getTerritory(*it);
			if(dTerritory > 0) { bCount += dTerritory; }
			else if(dTerritory < 0) { wCount -= dTerritory; }
		}

		if( numStone>260 ) { return 0; }
		double diff = bCount - wCount;
		if( numMove<WeichiConfigure::dkomi_start_step ) { diff = Internal_Handicap_komiInit; }
		double stones_per = numStone / (19.0*19.0-WeichiConfigure::dkomi_minus);     // stones / (boardsize*boardsize)
		double t   = (1.0 - stones_per)-WeichiConfigure::dkomi_shift; // - winrate*WeichiConfigure::dkomi_winrate_weight; // change big around 240 moves
		double per = 1.0 / ( 1.0 + exp(-WeichiConfigure::dkomi_slop*t)); 
		if ( per > 0.93 ) { per = 0.93; }
		else if(per < 0.1) { per = 0; }

		return (int)(per * diff);
	}
}