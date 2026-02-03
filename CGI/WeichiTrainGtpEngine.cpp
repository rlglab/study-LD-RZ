#include "WeichiTrainGtpEngine.h"

void WeichiTrainGtpEngine::Register()
{
	RegisterFunction("open_file", this, &WeichiTrainGtpEngine::cmdOpenFile, 1);
	RegisterFunction("clear_file", this, &WeichiTrainGtpEngine::cmdClearFile, 1);
	RegisterFunction("write_file", this, &WeichiTrainGtpEngine::cmdWriteFile, 1, 10);
	RegisterFunction("T_SL_features", this, &WeichiTrainGtpEngine::TSLFeatures, 0);
	RegisterFunction("T_dcnn_bv_territory", this, &WeichiTrainGtpEngine::TDCNNBVTerritory, 0);
	RegisterFunction("T_pure_playout_win_rate", this, &WeichiTrainGtpEngine::TPurePlayoutWinRate, 0);
	RegisterFunction("T_bv_playout_win_rate", this, &WeichiTrainGtpEngine::TBVPlayoutWinRate, 0);
	RegisterFunction("T_MCTS_win_rate", this, &WeichiTrainGtpEngine::TMCTSWinRate, 0);
	RegisterFunction("T_CNN_playout_win_rate", this, &WeichiTrainGtpEngine::TCNNPlayoutWinRate, 0);
}

string WeichiTrainGtpEngine::getGoguiAnalyzeCommandsString()
{
	return "";
}

void WeichiTrainGtpEngine::cmdOpenFile()
{
	string sFileName = m_args[0];

	if( m_sFileName!=sFileName ) {
		if( m_fout.is_open() ) { m_fout.close(); }
		m_sFileName = sFileName;
		m_fout.open(m_sFileName.c_str(),ios::out|ios::app);
	}

	reply(GTP_SUCC,"");
}

void WeichiTrainGtpEngine::cmdWriteFile()
{
	for( uint i=0; i<m_args.size(); i++ ) {
		m_fout << m_args[i] << ' ';
	}

	reply(GTP_SUCC,"");
}

void WeichiTrainGtpEngine::cmdClearFile()
{
	string sFileName = m_args[0];
	fstream fout(sFileName.c_str(),ios::out);
	fout.close();

	reply(GTP_SUCC,"");
}

void WeichiTrainGtpEngine::TSLFeatures()
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
	
	WeichiBitBoard bmLegal;
	float features[MAX_CNN_CHANNEL_SIZE*MAX_NUM_GRIDS];
	state.m_rootFilter.startFilter();
	WeichiCNNFeatureGenerator::calculateDCNNFeatures(state,CNN_FEATURE_F16,features,bmLegal);

	int channel_size = CNN_CHANNEL_SIZE[/*WeichiConfigure::dcnn_sl_feature*/CNN_FEATURE_F16];
	for( int i=0; i<channel_size; i++ ) {
		for( uint j=0; j<WeichiConfigure::TotalGrids; j++ ) {
			m_fout << features[i*WeichiConfigure::TotalGrids+j];
		}
	}

	m_fout << endl;

	reply(GTP_SUCC,"");
}

void WeichiTrainGtpEngine::TDCNNBVTerritory()
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
	WeichiCNNBVVNNet* bvvnNet = state.getBVVNNet();

	state.m_rootFilter.startFilter();
	WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(state,bvvnNet);
	bvvnNet->forward();

	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		m_fout << bvvnNet->getCNNTerritory(0,*it) << " ";
	}
	
	m_fout << endl;

	reply(GTP_SUCC,"");
}

void WeichiTrainGtpEngine::TPurePlayoutWinRate()
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
	double dBlackWinRate = state.startPreSimulation(Configure::SimulationCountLimit);

	m_fout << dBlackWinRate << endl;

	reply(GTP_SUCC,"");
}

void WeichiTrainGtpEngine::TBVPlayoutWinRate()
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
	WeichiCNNBVVNNet* bvvnNet = state.getBVVNNet();

	state.m_rootFilter.startFilter();
	WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(state,bvvnNet);
	bvvnNet->forward();

	StatisticData blackWinRate;
	for( int i=0; i<100; i++ ) {		
		Territory territory;
		state.startPreSimulation(1,territory);

		double dThreshold = Random::nextReal(1);
		double dResult = -WeichiDynamicKomi::Internal_komi;
		for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
			double dBVResult = (bvvnNet->getCNNTerritory(0,*it)+1)/2;
			Color owner = (dBVResult>0.5)? COLOR_BLACK: COLOR_WHITE;
			double dProb = (owner==COLOR_BLACK)? dBVResult: (1-dBVResult);

			if( dThreshold>0.1 && dProb>=0.9 && dThreshold<0.85 ) { dResult += (owner==COLOR_BLACK)? 1.0f: -1.0f; }
			else if( dThreshold>0.1 && dProb>=0.8 && dThreshold<0.75 ) { dResult += (owner==COLOR_BLACK)? 1.0f: -1.0f; }
			else if( dThreshold>0.1 && dProb>=0.7 && dThreshold<0.65 ) { dResult += (owner==COLOR_BLACK)? 1.0f: -1.0f; }
			else { dResult += territory.getTerritory(*it); }
		}

		if( dResult>0 ) { blackWinRate.add(1); }
		else { blackWinRate.add(0); }
	}

	m_fout << blackWinRate.getMean() << endl;

	reply(GTP_SUCC,"");
}

void WeichiTrainGtpEngine::TMCTSWinRate()
{
	WeichiGlobalInfo::get()->cleanSummaryInfo();	
	WeichiMove move = m_mcts.genmove(false);
	WeichiGlobalInfo::getSearchInfo().m_moveCache.clear();
	
	double dPoWinRate = WeichiGlobalInfo::getTreeInfo().m_bestSD.getMean();
	double dVNWinRate = WeichiGlobalInfo::getTreeInfo().m_bestValueSD.getMean();
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());

	m_fout << dPoWinRate << " " << dVNWinRate << " " << (dPoWinRate + dVNWinRate) / 2 << endl;

	reply(GTP_SUCC,"");
}

void WeichiTrainGtpEngine::TCNNPlayoutWinRate()
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
	m_cnnPlayoutGenerator.initialize(state.getSLNet()->getBatchSize());
	
	StatisticData blackWinRate = m_cnnPlayoutGenerator.run(100);

	m_fout << blackWinRate.getMean() << endl;

	reply(GTP_SUCC,"");
}