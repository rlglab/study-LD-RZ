#include "DbOpening.h"

int DbOpening::dbOpeningInit() 
{
	string db_sqlite = WeichiConfigure::db_dir + "opening.db";
	if( db_sqlite == "" ){
		CERR() << " No sqlite configuraitons." << endl;
		return -1;
	}
	return dbOpeningInit(db_sqlite);
}

int DbOpening::dbOpeningInit(string sSqliteName) 
{
	m_SqliteDb.Open(sSqliteName);
	dbOpeningClear();
	return 0;
}

void DbOpening::dbOpeningMakeMove(DbMove move)
{
	m_DbMoves[m_iMoveNum] = move;
	m_iMoveNum++;
}

void DbOpening::dbOpeningBackMove() 
{
	if(m_iMoveNum <= 0) return;
	--m_iMoveNum;
}

void DbOpening::dbOpeningClear() 
{
	m_iMoveNum = 0;
}

DbMove DbOpening::getNextOpeningMove() 
{
	return getNextOpeningMove(m_DbMoves, m_iMoveNum);
}

DbMove DbOpening::getNextOpeningMove(DbMove* moves, uint move_num) 
{
	if( !dbConnected() ) {
		CERR() << "Opening book not connected." << endl ;
		return NONEMOVE;
	}

	m_iMoveNum = move_num ;
	for( uint i=0 ; i< move_num ; i++ ) 
		m_DbMoves[i] = moves[i] ;

	vector<DataMove> vCandidateMove = getCandidateMove(m_DbMoves,m_iMoveNum);
	if(vCandidateMove.empty()) {
		CERR() << "Out of opening book." << endl ;
		return NONEMOVE;
	}

	DbMove move;
	if(!m_bRandomMove) 
		move = chooseBestMove(vCandidateMove); 
	else 
		move = chooseRandomMove(vCandidateMove);

	if(move == NONEMOVE) return NONEMOVE;

	return move;
}

vector<DataMove> DbOpening::getCandidateMove(DbMove* moves, int move_num)
{
	vector<DataMove> vGeneralMove;
	queryDatabase(moves,move_num);
	while(fetchRowDatabase()){
		DataMove oDataMove;
		getDataMoveFromDatabase(oDataMove);
		if (oDataMove.dbMove == NONEMOVE) continue;
		if( oDataMove.dScore )
			vGeneralMove.push_back(oDataMove);		
	}
	finalizeDatabase();
	return vGeneralMove ;
}

DbMove DbOpening::chooseBestMove(vector<DataMove> vCandidateMove )
{
	size_t iMoveNum = vCandidateMove.size();
	int iMaxMove = 0;
	double dTotalScores = 0 ;
	for(unsigned int i = 0; i < iMoveNum; i++){
		dTotalScores += vCandidateMove[i].dScore;
		if( policy == COUNT ){
			if(vCandidateMove[i].winCounts >= vCandidateMove[iMaxMove].winCounts)
				iMaxMove = i;
		} else if ( policy == WINRATE ) {
			if(vCandidateMove[i].dScore >= vCandidateMove[iMaxMove].dScore)
				iMaxMove = i;
		}
	}

	// If the positionsNodeCount are greater than the counts, then believe it, or return nonemove.

	double dbSimulationRealCount = positionNodeCountWeight * double(vCandidateMove[iMaxMove].positionsNodeCount) ;

	if( dbSimulationRealCount >= getSimulationCountThreshold() ) {
		CERR() << "dbSimulationRealCount:" << dbSimulationRealCount << " > SimulationCountThreshold:" << getSimulationCountThreshold() << endl ;
		storeResultScore(vCandidateMove[iMaxMove].dScore, dTotalScores) ;
		return getMoveAfterSymetric(vCandidateMove[iMaxMove]);
	}else {
		CERR() << "Predicted simulation counts exceed opening book position counts." << endl ;
		CERR() << "dbSimulationRealCount:" << dbSimulationRealCount << " < SimulationCountThreshold:" << getSimulationCountThreshold() << endl ;
		return NONEMOVE ;
	}

}

DbMove DbOpening::chooseRandomMove(vector<DataMove> vCandidateMove)
{
	size_t iMoveNum = vCandidateMove.size();
	double dTotalScores = 0;
	for(unsigned int i = 0; i < iMoveNum; i++){
		dTotalScores += vCandidateMove[i].dScore;
	}
	if(!dTotalScores) return NONEMOVE;
	vector<int> vRate;
	int total = 0;
	for(unsigned int i = 0; i < iMoveNum; ++i){
		int rate;
		int tmp = (int)((vCandidateMove[i].dScore/dTotalScores)*100.0);
		if(!tmp) rate = 1;
		else rate = tmp;
		total += rate;
		vRate.push_back(rate);
	}
	int random = (rand()%total)+1;
	total = 0;
	int iMove = 0;
	while(random > total){
		total += vRate[iMove];
		iMove++;
	}
	iMove--;
	storeResultScore(vCandidateMove[iMove].dScore, dTotalScores);
	return getMoveAfterSymetric(vCandidateMove[iMove]);
}

void DbOpening::storeResultScore(double score, double total)
{
	now_score =static_cast<int>(score);
	total_score = static_cast<int>(total);
}

unsigned long long DbOpening::getBoardHashKey(DbMove* moves, int move_num)
{
	unsigned long long ullHashKey ;
	ullHashKey = m_DbHashTable->getBoardHashKey(moves,move_num).getHashKey() ;

	return ullHashKey;
}


DbMove DbOpening::getMoveAfterSymetric(DataMove MoveData)
{
	vector<SymmetryType> types = m_DbHashTable->getSymmetricTypes();

	for(int i = 0; i < 8; i++) {
		// Get the 8 symmetric rotations of the points
		DbMove newMove = MoveData.dbMove ;
		MoveLocation location(newMove.getValue()) ;
		MoveLocation changedLocation = IsomorphicMinHashkey::getSymmetricCoordinate(location, types[i]) ;
		newMove.setValue(changedLocation.getMoveValue());
		m_DbMoves[m_iMoveNum] = newMove;
		// If the board play this move, it corresponds the hashkey of the database, it is 
		// getBoardHashKey is the hash key of current board
		if( getBoardHashKey(m_DbMoves, m_iMoveNum+1) == MoveData.ullToHashKey ) {
			m_DbMoves[m_iMoveNum] = NONEMOVE;
			return newMove;
		}
	}	
	m_DbMoves[m_iMoveNum] = NONEMOVE; 
	return NONEMOVE;
}

int DbOpening::queryDatabase(DbMove* moves, int move_num, bool bOrder/*=true*/, bool bNoZero/*=false*/)
{
	string sQuery = "SELECT MoveValue, BestProbabilityWinRate, MoveCount, ToHashkey, PositionsNodeCount "
		"FROM positions JOIN Moves ON positions.Hashkey = moves.ToHashkey "
		"WHERE FromHashkey = " + ToString(getBoardHashKey(moves, move_num)) + " AND positions.WinValue = 0 ;" ;
		
	return m_SqliteDb.Prepare(sQuery);
}

bool DbOpening::fetchRowDatabase()
{
	if( m_SqliteDb.Step() == SQLITE_ROW )
		return true;
	else
		return false;
}

void DbOpening::getDataMoveFromDatabase(DataMove &oDatamove)
{	
	oDatamove.dbMove.setValue(m_SqliteDb.ColumnInt32(0)) ; 
	oDatamove.dScore = m_SqliteDb.ColumnDouble(1) ;        
	oDatamove.winCounts = m_SqliteDb.ColumnInt32(2) ; 	
	string textHash(m_SqliteDb.ColumnText(3)) ;

	std::stringstream sstr(textHash);
	unsigned long long val;
	sstr >> val;	
	oDatamove.ullToHashKey = val ;	

	oDatamove.positionsNodeCount = m_SqliteDb.ColumnInt32(4) ;
}

void DbOpening::finalizeDatabase()
{
	m_SqliteDb.Finalize() ;
}


void DbOpening::changeBoardSize(uint boardsize)
{
	delete m_DbHashTable;
	dbOpeningClear() ;
	now_score = total_score = 0;
	m_DbHashTable = new DBHashTable(boardsize);
	simulationCountThreshold = 0 ;
} 
