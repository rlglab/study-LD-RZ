#ifndef _DBOPENING_H_
#define _DBOPENING_H_

#include "DbMove.h"
#include "DbHashTable.h"
#include "WeichiConfigure.h"
#include "BasicType.h"
#include "rand64.h"
#include "SQLite.h"
#include <time.h>

#define MAX_MOVE 361

#define WIN "Win"
#define GENERAL "General"

class DataMove
{
public:
	DbMove dbMove;
	double dScore; // winRate
	uint winCounts;
	uint positionsNodeCount;
	unsigned long long ullToHashKey;
};

enum selection_policy{
	COUNT,
	WINRATE
};

class DbOpening {
public:
	DbOpening() {
		dbOpeningClear();
		now_score = total_score = 0;
		m_DbHashTable = new DBHashTable(WeichiConfigure::BoardSize);
		if( WeichiConfigure::opening_book_control == "COUNT" ) 
			policy = COUNT ;
		else if( WeichiConfigure::opening_book_control == "WINRATE" ) 
			policy = WINRATE ;
		m_bRandomMove = false;
		simulationCountThreshold = 0 ;
		positionNodeCountWeight = 100000 ;
	}

	~DbOpening() { 
		delete m_DbHashTable;
	}

	bool dbConnected() {
		return m_SqliteDb.m_bIsOpen;
	}
	int dbOpeningInit();
	int dbOpeningInit(string sFilename);
	void dbOpeningMakeMove(DbMove move);
	void dbOpeningBackMove();
	void dbOpeningClear();
	DbMove getNextOpeningMove();
	DbMove getNextOpeningMove(DbMove* moves, uint move_num);
	vector<DataMove> getCandidateMove(DbMove* moves, int move_num);//if have win moves return them, else return general moves
	DbMove chooseBestMove(vector<DataMove> vCandidateMove);
	DbMove chooseRandomMove(vector<DataMove> vCandidateMove);//choose a move by score with random
	unsigned long long getBoardHashKey(DbMove* moves, int move_num);
	DbMove getMoveAfterSymetric(DataMove MoveData);//get correct move by add new piece and compare hashkey
	void storeResultScore(double score, double total);
	int queryDatabase(DbMove* moves, int move_num, bool bOrder=true, bool bNoZero=false);
	bool fetchRowDatabase();
	void getDataMoveFromDatabase(DataMove &oDatamove);
	void finalizeDatabase();

	inline void setSimulationCountThreshold (double counts) { simulationCountThreshold = counts ; }
	inline double getSimulationCountThreshold () const { return simulationCountThreshold ; }

public:
	int now_score;
	int total_score;
	void changeBoardSize(uint boardSize) ;
private:
	DBHashTable *m_DbHashTable;
	SQLite m_SqliteDb;
	DbMove m_DbMoves[MAX_MOVE];
	int m_iMoveNum;
	selection_policy policy ;
	bool m_bRandomMove;
	double simulationCountThreshold ;
	double positionNodeCountWeight ;
};

extern DbOpening dbOpening;

#endif 
