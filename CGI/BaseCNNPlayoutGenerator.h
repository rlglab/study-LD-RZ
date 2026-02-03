#ifndef BASECNNPLAYOUTGENERATOR_H
#define BASECNNPLAYOUTGENERATOR_H

#include "GameInfo.h"

class BaseCNNPlayoutGenerator
{
protected:
	uint m_nDoing;
	uint m_nFinish;
	uint m_nTotals;
	uint m_nParallelGames;
	bool m_bIsInitailize;
	Color m_turnColor;
	Color m_startColor;
	string m_sEventName;
	Territory m_territory;
	Dual<string> m_sPlayerName;
	WeichiThreadState& m_state;
	StatisticData m_blackWinRate;
	vector<string> m_vSgfGameInfo;
	Dual<WeichiCNNSLNet*> m_slNet;
	vector<GameInfo*> m_vPlayoutGames;

public:
	BaseCNNPlayoutGenerator( WeichiThreadState& state )
		: m_state(state), m_bIsInitailize(false), m_startColor(COLOR_BLACK)
	{
	}

	~BaseCNNPlayoutGenerator() {
		for( uint gameID=0; gameID<m_vPlayoutGames.size(); gameID++ ) {
			delete m_vPlayoutGames[gameID];
		}
	}

	void reset();
	void initialize( uint nParallelGames );
	StatisticData run( int nGames=1, Dual<string> sPlayerName=Dual<string>(), string sEventName="" );
	inline const Territory& getTerritory() const { return m_territory; }
	inline void setStartColor( Color startColor ) { m_startColor = startColor; }
	inline GameInfo* getGameInfo( uint gameID ) { return m_vPlayoutGames[gameID]; }
	inline const vector<string>& getSgfGameInfo() const { return m_vSgfGameInfo; }
	inline void setSLNet(Color color, WeichiCNNSLNet* slNet) { m_slNet.set(color, slNet); }

private:
	void prepareOneMoveData();
	//void forward();
	void playOneMove();
	void checkEndGame();

protected:
	virtual void prepareOneMoveData( uint gameID );
	virtual void forward();
	virtual void playOneMove( uint gameID, uint net_index );
	virtual bool checkEndGame( uint gameID );
	virtual WeichiCNNSLNet& getNet( Color turnColor );
};

#endif