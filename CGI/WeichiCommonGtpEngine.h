#ifndef WEICHICOMMONGTPENGINE_H
#define WEICHICOMMONGTPENGINE_H

#include "WeichiBaseGtpEngine.h"
#include "concepts.h"
#include "BasicType.h"
#include "SgfLoader.h"
#include "DbOpening.h"

class WeichiCommonGtpEngine : public WeichiBaseGtpEngine
{
	static const int UCT_GENMOVE = 0;
	static const int PLAYOUT_GENMOVE = 1;
	static const int DCNN_GENBESTMOVE = 2;
	static const int DCNN_GENSOFTMAXMOVE = 3;
	static const int DCNN_GENVNMOVE = 4;

private:
	class HandicapSetter {
		std::vector<WeichiMove> m_preset ;
	public:
		bool operator()(WeichiThreadState& state) const ;
		void placeHandicap(vector<WeichiMove>& handicaps );
	private:
		void clear() { m_preset.clear(); }
		void addPreset(WeichiMove m) { m_preset.push_back(m) ; }
	};
	
	SgfLoader m_sgfLoader;
	DbOpening m_dbOpening;
	HandicapSetter m_handicap_setter;	

protected:
	bool m_resign;

public:
	WeichiCommonGtpEngine( std::ostream& os, MCTS& instance )
		: WeichiBaseGtpEngine(os,instance), m_resign(false)
	{
	}
	void Register();
	string getGoguiAnalyzeCommandsString();

protected:
	void setTime( float seconds );
	void adjustTimeLimit( Color c );

	void cmdName();
	void cmdVersion();
	void cmdBoardsize();
	void cmdShowboard();
	void cmdClearBoard();
	void cmdSleep() ;
	void cmdLoadsgf();
	void cmdLoadjson();
	void cmdKomi();
    void cmdSetInternalKomi();
	void cmdFixedHandicap();
	void cmdPlaceFreeHandicap();
	void cmdSetFreeHandicap();
	void cmdTimeSettings();
	void cmdTimeLeft();
	void cmdPlay();
	void cmdGoguiPlaySequence();
	void cmdGenmove();
	void cmdRegGenmove();
    void cmdKgsGenmoveCleanup();
	void cmdPeek();
	void cmdWinRate();
	void cmdFinalScore();
	void cmdFinalStatusList();
	void cmdMctsBV();
	void cmdShowKo();

	string genOneMCTSMove( Color turnColor, bool bWithPlay );
	Move genmoveFromDB( Color c );
	bool checkImitate();
	bool checkCloseGame();
	bool checkMoveToResign();
	bool checkMoveToPass( const WeichiBoard& board );
	void changeBoardSize( uint boardsize );
	void changeKomi( float komi );
	void clearBoard();
	void saveWinRateDropSgf();

	float getMovesToGo() ;
	float getNextThinkTime( Color c , float moves_to_go);
};

#endif