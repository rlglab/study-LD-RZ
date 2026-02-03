#include "RLPlayoutGenerator.h"

void RLPlayoutGenerator::run()
{
	while( true ) {
		m_barStart.wait();

		initialize();

		// play game
		reset();
		setStartColor(COLOR_BLACK);

		m_winRate.m_black = runOneTurn(COLOR_BLACK);
		m_winRate.m_white = runOneTurn(COLOR_WHITE);

		summarizeData();

		m_barFinish.wait();
	}
}

void RLPlayoutGenerator::initialize()
{
	if( m_bIsInitialize ) { return; }

	// reset random seed
	Random::reset(m_seed);

	// set network
	m_ourCNNNet.setGpuID(m_gpuID);
	m_ourCNNNet.loadNetWork(true);
	m_oppCNNNet.setGpuID(m_gpuID);
	m_oppCNNNet.loadNetWork(true);
	m_bvvnNet.setGpuID(m_gpuID);
	m_bvvnNet.loadNetWork(true);

	// sgf file
	string sFileName = m_shareData.m_sSgfDirectory + "thread_" + ToString(m_id) + ".sgf";
	m_fSgfInfo.open(sFileName.c_str(),ios::out);

	BaseCNNPlayoutGenerator::initialize(m_ourCNNNet.getBatchSize());

	m_bIsInitialize = true;
}

StatisticData RLPlayoutGenerator::runOneTurn( Color ourColor )
{
	Dual<string> sPlayerName;
	string sOpponentName = m_shareData.m_sOpponent.substr(0,m_shareData.m_sOpponent.find_last_of("."));
	string sEventName = "rl_iter_" + ToString(m_shareData.m_iteration);
		
	m_ourColor = ourColor;
	sPlayerName.get(ourColor) = "rl_iter_" + ToString(m_shareData.m_iteration);
	sPlayerName.get(AgainstColor(ourColor)) = sOpponentName;
	StatisticData blackWinRate = BaseCNNPlayoutGenerator::run(m_shareData.m_totalGames/2,sPlayerName,sEventName);

	// save sgf
	for( uint i=0; i<m_vSgfGameInfo.size(); i++ ) {
		m_fSgfInfo << m_vSgfGameInfo[i] << endl;
	}

	return blackWinRate;
}

void RLPlayoutGenerator::summarizeData()
{
	for( uint index=0; index<m_vRLInputData.size(); index++ ) {
		RLInputData& rlInput = m_vRLInputData[index];
		RLGameData& rlGame = m_vRLGameData[rlInput.m_gameID];
		WeichiMove& move = rlGame.m_vMoves[rlInput.m_moveNumber];
		bool bIsLegalMove = rlGame.m_vLegal[rlInput.m_moveNumber];

		// skip draw game & win game's illegal moves
		pair<int,int> indexPair(m_id,index);
		if( rlGame.m_winColor==move.getColor() && bIsLegalMove ) { m_vWinIndex.push_back(indexPair); }
		else if( rlGame.m_winColor==AgainstColor(move.getColor()) ) { m_vLoseIndex.push_back(indexPair); }
	}
}

void RLPlayoutGenerator::summarizeThreadData( RLPlayoutGenerator* generator )
{
	// merge win rate
	m_winRate.m_black.add(generator->m_winRate.m_black);
	m_winRate.m_white.add(generator->m_winRate.m_white);

	// merge win/lose index
	m_vWinIndex.insert(m_vWinIndex.end(),generator->m_vWinIndex.begin(),generator->m_vWinIndex.end());
	m_vLoseIndex.insert(m_vLoseIndex.end(),generator->m_vLoseIndex.begin(),generator->m_vLoseIndex.end());
}

void RLPlayoutGenerator::reset()
{
	// reset game record
	m_vWinIndex.clear();
	m_vLoseIndex.clear();
	m_vRLInputData.clear();
	m_vRLGameData.clear();
	m_vRLGameData.resize(m_shareData.m_totalGames);
	for( uint i=0; i<m_shareData.m_totalGames; i++ ) { m_vRLGameData[i].reset(); }
	reloadOppNetwork(m_shareData.m_sOpponent);
}

void RLPlayoutGenerator::forward()
{
	BaseCNNPlayoutGenerator::forward();
	if( m_turnColor==m_ourColor && WeichiConfigure::rl_isUseVNBaseline ) { m_bvvnNet.forward(); }
}

void RLPlayoutGenerator::prepareOneMoveData( uint gameID )
{
	GameInfo* gameInfo = getGameInfo(gameID);
	WeichiThreadState* gameState = gameInfo->getState();
	BaseCNNPlayoutGenerator::prepareOneMoveData(gameID);
	
	// we're only interested at the move we played
	if( gameState->m_board.getToPlay()!=m_ourColor ) { return; }


	int realID = getGameRealID(gameInfo);
	WeichiCNNSLNet& net = getNet(m_turnColor);
	
	RLGameData& rlGameData = m_vRLGameData[realID];
	RLInputData rlInput(realID,rlGameData.m_vMoves.size());
	float* features = net.getFeatures(net.getCurrentBatchSize()-1);
	SymmetryType symmetryType = net.getSymmetryType(net.getCurrentBatchSize()-1);
	memcpy(rlInput.m_input,features,sizeof(float)*net.getChannelSize()*WeichiConfigure::TotalGrids);
	rlInput.type = ReverseSymmetricType[symmetryType];
	
	m_vRLInputData.push_back(rlInput);

	// bvvn value
	if(WeichiConfigure::rl_isUseVNBaseline){ WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(*gameState,&m_bvvnNet); }

}

void RLPlayoutGenerator::playOneMove( uint gameID, uint net_index ) 
{
	GameInfo* gameInfo = getGameInfo(gameID);
	WeichiThreadState* gameState = gameInfo->getState();

	WeichiCNNSLNet& net = getNet(m_turnColor);
	bool bSoftMaxWithIllegal = (m_turnColor==m_ourColor);
	WeichiMove move = net.getSoftMaxMove(net_index,bSoftMaxWithIllegal);
	double dIllegal = 0.0f;
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		if( !net.isLegal(net_index,*it) ) { dIllegal += net.getCNNResult(net_index,*it); }
	}
	m_illegal.add(dIllegal);

	// store the move we played (take illegal move as PASS move)
	bool bIsLegal = true;
	int realID = getGameRealID(gameInfo);
	float fBaselineValue = 0.5;
	if(WeichiConfigure::rl_isUseVNBaseline){ fBaselineValue = m_bvvnNet.getVNResult(net_index,WeichiConfigure::komi); }
	fBaselineValue = (fBaselineValue *2)-1 ; 

	RLGameData& rlGameData = m_vRLGameData[realID];
	if( m_turnColor==m_ourColor ) { 
		rlGameData.m_vMoves.push_back(move); 
		rlGameData.m_vBaselineValues.push_back(fBaselineValue);
	}

	if( !net.isLegal(net_index,move.getPosition()) ) {
		bIsLegal = false;
		move = WeichiMove(move.getColor(),PASS_MOVE.getPosition());
	}
	if( m_turnColor==m_ourColor ) { rlGameData.m_vLegal.push_back(bIsLegal); }

	gameState->play(move,true);
}

bool RLPlayoutGenerator::checkEndGame( uint gameID )
{
	/*bool bIsEndGame = BaseCNNPlayoutGenerator::checkEndGame(gameID);
	if( !bIsEndGame ) { return false; }*/

	GameInfo* gameInfo = getGameInfo(gameID);
	WeichiThreadState* gameState = gameInfo->getState();

	const int MAX_RL_GAME_LENGTH = 500;
	const WeichiBoard& board = gameState->m_board;
	if( !board.isEarlyEndGame() && !board.isTwoPassEndGame() && board.getMoveList().size()<MAX_RL_GAME_LENGTH ) { return false; }

	Vector<WeichiMove,MAX_GAME_LENGTH> vMoveLists = gameState->m_board.getMoveList();
	gameState->resetThreadState();

	int max_moves = (board.isTwoPassEndGame())? vMoveLists.size()-2: vMoveLists.size();
	for( int i=0; i<max_moves; i++ ) {
		gameState->play(vMoveLists[i]);
	}

	Territory territory;
	gameState->startPreSimulation(32,territory);
	float dResult = -WeichiDynamicKomi::Internal_komi;
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		if( territory.getTerritory(*it)>0.8 ) { dResult++; }
		else if( territory.getTerritory(*it)<-0.8 ) { dResult--; }
	}

	WeichiPlayoutResult result(dResult);
	if( result.getWinner()==COLOR_BLACK ) { m_blackWinRate.add(1.0f); }
	else if( result.getWinner()==COLOR_WHITE ) { m_blackWinRate.add(0.0f); }
	else { m_blackWinRate.add(0.5f); }
	gameInfo->setValid(false);

	SgfTag sgfTag;
	sgfTag.setSgfTag("RE", result.toString());
	sgfTag.setSgfTag("PB", m_sPlayerName.m_black);
	sgfTag.setSgfTag("PW", m_sPlayerName.m_white);
	sgfTag.setSgfTag("EV", m_sEventName);
	string sSgfInfo = gameState->m_board.toSgfFileString(sgfTag);
	m_vSgfGameInfo.push_back(sSgfInfo);

	++m_nFinish;

	saveGameResultToInputData(gameID,result);
	return true;
}

void RLPlayoutGenerator::saveGameResultToInputData( uint gameID, WeichiPlayoutResult result )
{
	GameInfo* gameInfo = getGameInfo(gameID);
	Color winner = result.getWinner();

	int realID = getGameRealID(gameInfo);
	RLGameData& rlGameData = m_vRLGameData[realID];
	rlGameData.m_winColor = winner;
}

WeichiCNNSLNet& RLPlayoutGenerator::getNet( Color turnColor )
{
	return (turnColor==m_ourColor)? m_ourCNNNet: m_oppCNNNet;
}

void RLPlayoutGenerator::setOpenning(vector<WeichiMove>& vMoves, int startMove)
{
	m_state.resetThreadState();
	for(int i=0;i<startMove; ++i){
		m_state.play(vMoves[i]);
	}
}