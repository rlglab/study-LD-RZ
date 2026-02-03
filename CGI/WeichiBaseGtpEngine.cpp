#include "arguments.h"
#include "WeichiBaseGtpEngine.h"
#include <boost/filesystem.hpp>

void WeichiBaseGtpEngine::cmdEmpty()
{
	reply(GTP_SUCC, "");
}

void WeichiBaseGtpEngine::ensureTerritory()
{
	WeichiGlobalInfo::getSearchInfo().m_calTerritoryStat = true;
	WeichiGlobalInfo::getSearchInfo().m_territory.clear();
	/// backup
	Configure::SimulationControl sim_ctrl = Configure::SimCtrl;
	uint count_limit = Configure::SimulationCountLimit ;
	bool EarlyPassBackup = WeichiConfigure::EarlyPass;
	WeichiConfigure::EarlyPass = false;
	m_mcts.newTree();
	/// simulate some games
	Configure::SimCtrl = Configure::SIMCTRL_COUNT ;
	Configure::SimulationCountLimit = WeichiConfigure::NumSimToEnsureTerritory;
	m_mcts.genmove ( false ) ;
	/// roll-back
	Configure::SimCtrl = sim_ctrl ;
	Configure::SimulationCountLimit = count_limit;
	WeichiConfigure::EarlyPass = EarlyPassBackup;
	WeichiGlobalInfo::getSearchInfo().m_calTerritoryStat = false;
}

void WeichiBaseGtpEngine::createDirectories()
{
	WeichiGlobalInfo::getSearchInfo().m_territory.clear();
}

Move WeichiBaseGtpEngine::genPlayoutMove()
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
	WeichiPlayoutAgent playoutAgent(state);
	WeichiMove lastmove = PASS_MOVE;
	WeichiMove m;

	if( state.m_board.hasPrevMove() ) { lastmove = state.m_board.getPrevMove(); }
	state.m_board.setPlayoutStatus(true);
	m = playoutAgent.genPlayoutMove(lastmove);
	state.m_board.setPlayoutStatus(false);

	m_usedPolicy = playoutAgent.m_usedPolicy;
	CERR() << "Policy = " << getWeichiPlayoutPolicyString(m_usedPolicy) << endl;
	if( m_usedPolicy==POLICY_KILL_SUICIDE ) { m_vCandidateMoves.push_back(m.getPosition()); }
	else { m_vCandidateMoves = playoutAgent.m_vCandidateMoves; }

	return m;
}

Move WeichiBaseGtpEngine::genDCNNBestMove()
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());

	state.m_rootFilter.startFilter();
	WeichiCNNNet* cnnNet = state.getCNNNet();
	WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(state, cnnNet);
	cnnNet->forward();

	return cnnNet->getBestCNNMove(0);

	/*state.m_rootFilter.startFilter();
	WeichiCNNSLNet* slNet = state.getSLNet();
	WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(state,slNet);
	slNet->forward();

	return slNet->getBestCNNMove(0);*/
}

Move WeichiBaseGtpEngine::genDCNNSoftmaxMove()
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());

	state.m_rootFilter.startFilter();
	WeichiCNNNet* cnnNet = state.getCNNNet();
	WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(state, cnnNet);
	cnnNet->forward();

	return cnnNet->getSoftMaxMove(0);
}

Move WeichiBaseGtpEngine::genDCNNVNMove()
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
	Color turnColor = state.m_board.getToPlay();

	state.m_rootFilter.startFilter();
	WeichiCNNSLNet* slNet = state.getSLNet();
	WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(state,slNet);
	slNet->forward();

	//vector<CandidateEntry> vCandidates = slNet->getCandidates(0,slNet->getBatchSize()+1);
	vector<CandidateEntry> vCandidates = slNet->getCandidates(0,slNet->getBatchSize()+1);

	state.backup();
	double dMinScore = 2.0f;
	WeichiCNNBVVNNet* bvvnNet = state.getBVVNNet();
	for( uint i=1; i<vCandidates.size(); i++ ) {
		WeichiMove move(turnColor,vCandidates[i].getPosition());
		state.play(move);
		
		state.m_rootFilter.startFilter();
		WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(state,bvvnNet);

		state.rollback();
	}

	bvvnNet->forward();
	WeichiMove bestMove = WeichiMove(turnColor,PASS_MOVE.getPosition());
	for( uint i=1; i<vCandidates.size(); i++ ) {
		WeichiMove move(turnColor,vCandidates[i].getPosition());
		double dValue = bvvnNet->getVNResult(i-1,WeichiDynamicKomi::Internal_komi);

		cerr << move.toGtpString() << " " << dValue << endl;
		if( dValue<dMinScore ) {
			dMinScore = dValue;
			bestMove = move;
		}
	}

	return bestMove;
}

void WeichiBaseGtpEngine::checkAndReplaceCommand()
{
	if( m_command=="development_command" ) { m_command = WeichiConfigure::DevelopmentCommand; }
}

void WeichiBaseGtpEngine::beforeRunCommand()
{
	if ( Configure::Pondering ) {
		m_mcts.ponder_stop();
	}
}

void WeichiBaseGtpEngine::afterRunCommand()
{
	if ( Configure::Pondering && !hasQuit() ) {
		m_mcts.ponder_start();
	}
} 
