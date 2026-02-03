#ifndef __TERRITORY_H__
#define __TERRITORY_H__

#include "types.h"
#include "Dual.h"
#include "Color.h"
#include "Vector.h"
#include "StaticBoard.h"

using namespace std;

/*
	each grid will save following variables:

		  | BW| WW|tie
		--|---|---|---
		BO| a | b | c
		--|---|---|---
		WO| d | e | f

		BW/WW - means Black/White wins this simulation
		BO/WO - means Black/White owns this point in the end of game

	Territory formula:
		T(x) = (Pb(x)-Pw(x))/N, where
			N = a + b + c + d + e + f (means total valid simulation in x)
			Pb(x) = a + b + c (means total num black own this grid)
			Pw(x) = d + e + f (means total num white own this grid)

	Criticality formula:
		C(x) = Pwin(x) - (2*Pb(x)*Pb-Pb(x)+1), where
			N = a + b + c + d + e + f (means total valid simulation in x)
			Pwin(x) = (a + d) / N (means the probability owns this grid color match winner color)
			Pb(x) = (a + b + c) / N (means the probability black own this grid)
			Pb = (a + d) / N (means the probability black win simulation)
*/

class Territory
{
private:
	class PointStatistic {
	private:
		Vector<Dual<int>,3> m_vSimOwner;
	public:
		PointStatistic() { m_vSimOwner.resize(3); }
		inline void reset()
		{
			m_vSimOwner[COLOR_NONE].reset();
			m_vSimOwner[COLOR_BLACK].reset();
			m_vSimOwner[COLOR_WHITE].reset();
		}
		inline void addOneSimWinAndOwner( Color winColor, Color ownColor ) { ++m_vSimOwner[winColor].get(ownColor); }
		inline int getTotal() const { return (getSimWin(COLOR_NONE) + getSimWin(COLOR_BLACK) + getSimWin(COLOR_WHITE)); }
		inline int getSimWin( Color color ) const { return (m_vSimOwner[color].get(COLOR_BLACK) + m_vSimOwner[color].get(COLOR_WHITE)); }
		inline int getSimWinAndOwn() const { return m_vSimOwner[COLOR_BLACK].get(COLOR_BLACK) + m_vSimOwner[COLOR_WHITE].get(COLOR_WHITE); }
		inline int getSimOwn( Color color ) const { return (m_vSimOwner[COLOR_NONE].get(color) + m_vSimOwner[COLOR_BLACK].get(color) + m_vSimOwner[COLOR_WHITE].get(color)); }
	};
	int m_totalSimulation;
	Vector<PointStatistic,MAX_NUM_GRIDS> m_vStatistic;

public:
	Territory()
	{
		m_vStatistic.resize(MAX_NUM_GRIDS);
	}
	inline void clear()
	{
		m_totalSimulation = 0;
		for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
			m_vStatistic[*it].reset();
		}
	}
	inline double getTerritory( uint pos ) const
	{
		// Territory formula: T(x) = (Pb(x)-Pw(x))/N
		const PointStatistic& statistic = m_vStatistic[pos];
		uint N = statistic.getTotal();
		return (N==0) ? 0 : (statistic.getSimOwn(COLOR_BLACK)-statistic.getSimOwn(COLOR_WHITE))/(double)N;
	}
	inline double getCriticality( uint pos ) const
	{
		// Criticality formula: C(x) = Pwin(x) - (2*Pb(x)*Pb-Pb(x)+1)
		// Criticality formula: C(x) = Pwin(x) - (Pb(x)*Pb+Pw(x)*Pw)
		const PointStatistic& statistic = m_vStatistic[pos];
		double N = statistic.getTotal();
		double Pwinx = statistic.getSimWinAndOwn();
		double Pbx = statistic.getSimOwn(COLOR_BLACK);
		double Pb = statistic.getSimWin(COLOR_BLACK);
		double Pwx = statistic.getSimOwn(COLOR_WHITE);
		double Pw = statistic.getSimWin(COLOR_WHITE);

		return (N==0) ? 0 : (Pwinx-(Pbx*Pb/N+Pwx*Pw/N))/N;
		//return (N==0) ? 0 : (Pwinx-(2.0*Pbx*Pb/N-Pbx-Pb+N))/(double)N;
	}
	inline void ShowCriticalityInfo( uint pos ) const
	{
		// Criticality formula: C(x) = Pwin(x) - (2*Pb(x)*Pb-Pb(x)+1)
		// Criticality formula: C(x) = Pwin(x) - (Pb(x)*Pb+Pw(x)*Pw)
		const PointStatistic& statistic = m_vStatistic[pos];
		double N = statistic.getTotal();
		double Pwinx = statistic.getSimWinAndOwn();
		double Pbx = statistic.getSimOwn(COLOR_BLACK);
		double Pb = statistic.getSimWin(COLOR_BLACK);
		double Pwx = statistic.getSimOwn(COLOR_WHITE);
		double Pw = statistic.getSimWin(COLOR_WHITE);
		double dCriticality = (N==0) ? 0 : (Pwinx-(Pbx*Pb/N+Pwx*Pw/N))/N;

		CERR() << "N=" << statistic.getTotal() << endl;
		CERR() << "Pwinx=" << statistic.getSimWinAndOwn() << endl;
		CERR() << "Pbx=" << statistic.getSimOwn(COLOR_BLACK) << endl;
		CERR() << "Pb=" << statistic.getSimWin(COLOR_BLACK) << endl;
		CERR() << "Pwx=" << statistic.getSimOwn(COLOR_WHITE) << endl;
		CERR() << "Pw=" << statistic.getSimWin(COLOR_WHITE) << endl;
		CERR() << "Criticality=" << dCriticality << endl;
	}
	inline void addTerritory( const Vector<Color, MAX_NUM_GRIDS>& territory, Color winColor )
	{
		++m_totalSimulation;
		for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
			if( territory[*it]==COLOR_NONE ) { continue; }
			assert( territory[*it]==COLOR_BLACK || territory[*it]==COLOR_WHITE );
			PointStatistic& statistic = m_vStatistic[*it];
			statistic.addOneSimWinAndOwner(winColor,territory[*it]);
		}
	}
	inline uint getTerritoryCount( uint pos ) const { return m_vStatistic[pos].getTotal(); }
	inline uint getTerritoryOwnerCount( uint pos, Color color ) const { return m_vStatistic[pos].getSimOwn(color); }
	inline bool hasTerritory() const { return (m_totalSimulation>=10000); }
};

#endif 
