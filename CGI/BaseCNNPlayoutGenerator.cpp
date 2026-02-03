#include "BaseCNNPlayoutGenerator.h"

void BaseCNNPlayoutGenerator::reset()
{
	m_territory.clear();
	m_vSgfGameInfo.clear();
	m_blackWinRate.reset();
	m_nDoing = m_nFinish = 0;
}

void BaseCNNPlayoutGenerator::initialize( uint nParallelGames )
{
	if( m_bIsInitailize ) { return; }

	m_bIsInitailize = true;
	m_nParallelGames = nParallelGames;
	for( uint gameID=0; gameID<m_nParallelGames; gameID++ ) {
		m_vPlayoutGames.push_back(new GameInfo(gameID));
	}
	setSLNet(COLOR_BLACK, m_state.getSLNet());
	setSLNet(COLOR_WHITE, m_state.getSLNet());
}

StatisticData BaseCNNPlayoutGenerator::run( int nGames/*=1*/, Dual<string> sPlayerName/*=Dual<string>()*/, string sEventName/*=""*/ )
{
	reset();

	m_nTotals = nGames;
	m_turnColor = m_startColor;
	m_sEventName = sEventName;
	m_sPlayerName = sPlayerName;
	while( m_nFinish<m_nTotals ) {
		prepareOneMoveData();
		forward();
		playOneMove();
		checkEndGame();

		m_turnColor = AgainstColor(m_turnColor);
	}

	return m_blackWinRate;
}

void BaseCNNPlayoutGenerator::prepareOneMoveData()
{
	for( uint gameID=0; gameID<m_nParallelGames; gameID++ ) {
		GameInfo* gameInfo = m_vPlayoutGames[gameID];
		if( !gameInfo->isValid() && m_nDoing<m_nTotals && m_startColor==m_turnColor ) {
			gameInfo->copyState(m_state);
			gameInfo->setValid(true);
			gameInfo->setID(m_nDoing);
			m_nDoing++;
		}
		if( !gameInfo->isValid() ) { continue; }

		prepareOneMoveData(gameID);
	}
}

void BaseCNNPlayoutGenerator::forward()
{
	WeichiCNNSLNet& net = getNet(m_turnColor);
	net.forward();
}

void BaseCNNPlayoutGenerator::playOneMove()
{
	int net_index = 0;
	for( uint gameID=0; gameID<m_nParallelGames; gameID++ ) {
		GameInfo* gameInfo = getGameInfo(gameID);
		if( !gameInfo->isValid() ) { continue; }

		playOneMove(gameID,net_index++);
	}
}

void BaseCNNPlayoutGenerator::checkEndGame()
{
	for( uint gameID=0; gameID<m_nParallelGames; gameID++ ) {
		GameInfo* gameInfo = getGameInfo(gameID);
		if( !gameInfo->isValid() ) { continue; }

		checkEndGame(gameID);
	}
}

void BaseCNNPlayoutGenerator::prepareOneMoveData( uint gameID )
{
	GameInfo* gameInfo = getGameInfo(gameID);
	WeichiThreadState* gameState = gameInfo->getState();

	WeichiCNNSLNet& net = getNet(m_turnColor);
	WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(*gameState,&net);
}

void BaseCNNPlayoutGenerator::playOneMove( uint gameID, uint net_index )
{
	GameInfo* gameInfo = getGameInfo(gameID);
	WeichiThreadState* gameState = gameInfo->getState();

	WeichiCNNSLNet& net = getNet(m_turnColor);
	WeichiMove move = net.getSoftMaxMove(net_index);
	gameState->play(move,true);
}

bool BaseCNNPlayoutGenerator::checkEndGame( uint gameID )
{
	GameInfo* gameInfo = getGameInfo(gameID);
	WeichiThreadState* gameState = gameInfo->getState();

	const WeichiBoard& board = gameState->m_board;
	if( !board.isEarlyEndGame() && !board.isTwoPassEndGame() ) { return false; }
	
	WeichiPlayoutResult result(gameState->eval());
	m_territory.addTerritory(board.getTerritory(),result.getWinner());

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

	return true;
}

WeichiCNNSLNet& BaseCNNPlayoutGenerator::getNet( Color turnColor )
{
	return *m_slNet.get(turnColor);
}