#include "RLDCNN.h"
#include "WeichiPlayoutResult.h"
#include <boost/filesystem.hpp>
#include <vector>

void RLDCNN::start( uint max_iteration )
{
	// create threads
	uint seed = Configure::UseTimeSeed? static_cast<uint>(time(NULL)): Configure::RandomSeed;

	COUT() << "using " << Configure::NumThread << " thread(s) to do rldcnn training." << endl;
	int total_gpu_size = static_cast<int>(WeichiConfigure::dcnn_train_gpu_list.size());
	for( uint id=0; id<Configure::NumThread; id++ ) {
		uint gpuID = (total_gpu_size>0)? (WeichiConfigure::dcnn_train_gpu_list[id%total_gpu_size] - '0'): -1;
		RLPlayoutGenerator* generator = new RLPlayoutGenerator(id,gpuID,m_shareData); // current we use gpuID here instead of m_gpuID in net
		generator->setTimeSeed(seed+id);
		m_threads.create_thread(boost::bind(&RLPlayoutGenerator::run,generator));
		m_vRLPlayoutGenerator.push_back(generator);
	}

	if( !initialize() ) { return; }
	m_shareData.m_iteration = WeichiConfigure::num_rl_start_iteration;
	for( ; m_shareData.m_iteration<=max_iteration; m_shareData.m_iteration++ ) {

		if(WeichiConfigure::rl_isUseOpenning){
			RLOpenning openning = getRLOpenning();
			for(uint j=0;j<Configure::NumThread ; ++j ) { m_vRLPlayoutGenerator[j]->setOpenning(openning.m_vMoves, openning.m_startMove); }
		}
		
		runOneIteration(m_shareData.m_iteration,true);
		
		/*// fight with original
		if( m_shareData.m_iteration%WeichiConfigure::num_add_rl_to_opp_pool==0 ) {
			const int NUM_FIGHT_GAMES = 1000;
			m_shareData.m_totalGames = NUM_FIGHT_GAMES/Configure::NumThread;
			runOneIteration(m_shareData.m_iteration,false);
			m_shareData.m_totalGames = WeichiConfigure::num_rl_game/Configure::NumThread;
		}*/
	}
}

bool RLDCNN::initialize()
{
	if( !readOpponentPoolFile() ) { return false; }

	// set network
	m_rlCNNNet->init();

	// set total games
	m_shareData.m_totalGames = WeichiConfigure::num_rl_game/Configure::NumThread;

	// reset random seed
	uint seed = Configure::RandomSeed;
	if( Configure::UseTimeSeed ) { seed = static_cast<uint>(time(NULL)); }
	Random::reset(seed);

	// create directory
	m_shareData.m_sDirectoryPrefix = TimeSystem::getTimeString("Y_m_d_H_i_s") + "_" + WeichiConfigure::rldcnn_description + "/";
	m_shareData.m_sSgfDirectory = m_shareData.m_sDirectoryPrefix + "sgf/";
	m_shareData.m_sModelDirectory = m_shareData.m_sDirectoryPrefix + "model/";
	boost::filesystem::create_directories(m_shareData.m_sDirectoryPrefix);
	boost::filesystem::create_directories(m_shareData.m_sSgfDirectory);
	boost::filesystem::create_directories(m_shareData.m_sModelDirectory);

	// openning 
	if(WeichiConfigure::rl_isUseOpenning){
		CERR() << "Using openning for training: " << WeichiConfigure::rl_openning_dir << endl;
		if( WeichiConfigure::rl_openning_dir =="") { 
			CERR() << "Directory path is empty." <<	 endl; 
			return false;
		}
		m_trainDirExplorer.reset();
		m_trainDirExplorer.setRootDirectory(WeichiConfigure::rl_openning_dir);
	}

	return true;
}

void RLDCNN::runOneIteration( int iteration, bool bWithUpdate )
{
	boost::posix_time::ptime pStart = TimeSystem::getLocalTime();

	COUT() << "=====Iteration: " << iteration << " " << endl;
	CERR() << "=====Iteration: " << iteration << " " << endl;

	// random new opponent
	uint opponentID = bWithUpdate? randomNewOpponent(): 0;
	m_shareData.m_sOpponent = m_vOppNetworkPool[opponentID].m_sOpponentName;

	// do one iteration
	for( uint i=0; i<Configure::NumThread; i++ ) { m_vRLPlayoutGenerator[i]->startRun(); }
	for( uint i=0; i<Configure::NumThread; i++ ) { m_vRLPlayoutGenerator[i]->finishRun(); }
	summarizeThreadData(opponentID,iteration);
	if( bWithUpdate ) {
		update(true);
		snapshotTrainingResult(iteration);
	}

	boost::posix_time::ptime pEnd = TimeSystem::getLocalTime();
	boost::posix_time::time_duration duration = pEnd - pStart;
	COUT() << "Total use " << duration.total_seconds() << "(s)" << endl;
}

RLOpenning RLDCNN::getRLOpenning()
{
	SgfLoader sgfLoader;
	RLOpenning rlOpenning;
	bool isValidSgf = false;
	while(!isValidSgf){
		string openningString = m_trainDirExplorer.getNextSgfString();
		if( m_trainDirExplorer.isEndSgf() ) {
			m_trainDirExplorer.reset();
			m_trainDirExplorer.setRootDirectory(WeichiConfigure::rl_openning_dir);
			CERR() << "Traverse all training set, start again." << endl;
		}

		if( !sgfLoader.parseFromString(openningString, (1<<10)) ) { continue;  }
		if(sgfLoader.getHandicap() != 0) { continue;}
		if(sgfLoader.getPreset().size() != 0) {continue;}
		const vector<WeichiMove>& vMoves = sgfLoader.getPlayMove();
		if(vMoves.size() < WeichiConfigure::rl_openningMove) { continue;}
		isValidSgf = true;
		
		rlOpenning.m_vMoves = vMoves;
		rlOpenning.m_startMove = Random::nextInt(WeichiConfigure::rl_openningMove / 2)*2;
		//CERR() << "get move openning: " ;
		for(int j=0;j<rlOpenning.m_startMove;++j) CERR() << vMoves[j].toSgfString(true);
		//CERR() << endl;
		//CERR() << "Start move: " << rlOpenning.m_startMove << endl;
		//CERR() << "get move openning: " << sgfInformation.m_sSgfString << endl;
	}
	return rlOpenning;
}


bool RLDCNN::readOpponentPoolFile()
{
	COUT() << "=====Start reading opponent from file \"" << WeichiConfigure::rldcnn_opp_pool << "\"";
	char cLine[1024];
	fstream fOppPool(WeichiConfigure::rldcnn_opp_pool.c_str(),ios::in);
	if( !fOppPool ) {
		COUT() << "Can't find opponent pool file!" << endl;
		return false;
	}

	m_vOppNetworkPool.push_back(RLOpponent(0,CNNNetParam(WeichiConfigure::dcnn_RL_ourNet).m_sCaffeModel));
	while( fOppPool.getline(cLine,1024) ) {
		// check network file is exist
		fstream fExist(cLine,ios::in);
		if( !fExist ) {
			COUT() << "can't not find opponent file \"" << cLine << "\"" << endl;
			return false;
		}

		fExist.close();
		//COUT() << "add \"" << cLine << "\" to opponent pool" << endl;
		m_vOppNetworkPool.push_back(RLOpponent((uint)m_vOppNetworkPool.size(),cLine));
	}
	COUT() << "=====Finish reading opponent, total read " << m_vOppNetworkPool.size()-1 << " opponent(s)" << endl;

	return true;
}

uint RLDCNN::randomNewOpponent()
{
	int rand = WeichiConfigure::rldcnn_softmax_opp ? softmaxWithWinrateRandomNewOpponent() : uniformRandomNewOpponent();
	const RLOpponent& opponent = m_vOppNetworkPool[rand];
	string sOpponentName = opponent.m_sOpponentName;
	
	// message
	COUT() << setprecision(2) << "\tRandom select opponent \"" << sOpponentName << "\", ";
	if( opponent.m_vMeetIteration.size()==0 ) { COUT() << "meet first time." << endl; }
	else {
		int startIndex = static_cast<int>(opponent.m_vMeetIteration.size())-5;
		startIndex = (startIndex<0)? 0: startIndex;
		COUT() << "already meet at iteration " << opponent.m_vMeetIteration[startIndex] << "(" << opponent.m_vOpponentWinRate[startIndex] << "%)";
		for( uint i=startIndex+1; i<opponent.m_vMeetIteration.size(); i++ ) {
			COUT() << ", " << opponent.m_vMeetIteration[i] << "(" << opponent.m_vOpponentWinRate[i] << "%)";
		}
		COUT() << fixed;
		COUT() << ". last 5(" << opponent.m_last5TurnWinRateData.getMean() << "%)"
			   << ", total(" << opponent.m_winRateData.getMean() << "%)";
		COUT() << endl;
	}

	return opponent.m_id;
}

uint RLDCNN::uniformRandomNewOpponent()
{
	return Random::nextInt(static_cast<int>(m_vOppNetworkPool.size()-1)) + 1;
}

uint RLDCNN::softmaxWithWinrateRandomNewOpponent()
{
	double dSum = 0.0f;
	uint targetOpp=1;
	for( int i=1;i<m_vOppNetworkPool.size(); ++i ) {
		StatisticData& fWinrateData = m_vOppNetworkPool[i].m_last5TurnWinRateData;

		double dProb = (100-fWinrateData.getMean()) + 5;  // 5 is the baseline as the basic chance to be chosen
		dSum += dProb;
		double dRand = Random::nextReal(dSum);
		if( dRand<dProb ) { targetOpp = i; }
	}
	return targetOpp;
}

void RLDCNN::summarizeThreadData( uint oppID, uint iteration )
{
	for( uint i=1; i<Configure::NumThread; i++ ) {
		m_vRLPlayoutGenerator[0]->summarizeThreadData(m_vRLPlayoutGenerator[i]);
		
		m_vRLPlayoutGenerator[0]->m_illegal.add(m_vRLPlayoutGenerator[i]->m_illegal);
		m_vRLPlayoutGenerator[i]->m_illegal.reset();
	}
	cout << "illegal: " << m_vRLPlayoutGenerator[0]->m_illegal.getMean() << endl;
	m_vRLPlayoutGenerator[0]->m_illegal.reset();

	Dual<double> dWinRate;
	dWinRate.m_black = m_vRLPlayoutGenerator[0]->getWinRate(COLOR_BLACK).getMean() * 100;
	dWinRate.m_white = (1.0f-m_vRLPlayoutGenerator[0]->getWinRate(COLOR_WHITE).getMean()) * 100;
	double dTotalWinRate = (dWinRate.m_black+dWinRate.m_white)/2;
	COUT() << "black win rate = " << dWinRate.m_black << " %"
		   << ", white win rate = " << dWinRate.m_white << " %"
		   << ", total win rate = " << dTotalWinRate << " %" << endl;

	RLOpponent& opponent = m_vOppNetworkPool[oppID];
	opponent.m_vMeetIteration.push_back(iteration);
	opponent.m_vOpponentWinRate.push_back(dTotalWinRate);
	opponent.m_winRateData.add(dTotalWinRate);
	opponent.m_last5TurnWinRateData.add(dTotalWinRate);
	uint size = static_cast<uint>(opponent.m_vOpponentWinRate.size());
	if( size>5 ) { opponent.m_last5TurnWinRateData.remove(opponent.m_vOpponentWinRate[size-6]); }

	// log result: iteration opp wr_black wr_white wr_total wr_all_total
	string sFileName;
	if( oppID==0 ) { sFileName = m_shareData.m_sDirectoryPrefix + "self_fight_result.txt"; }
	else { sFileName = m_shareData.m_sDirectoryPrefix + "iteration_result.txt"; }

	// write header if first
	bool bIsFirstOpen = false;
	ifstream fin(sFileName.c_str());
	if( !fin.good() ) { bIsFirstOpen = true; }
	fstream fout(sFileName.c_str(),ios::out|ios::app);
	if( bIsFirstOpen ) { fout << "Iteration\tOpponent_name\tBlack_win_rate\tWhite_win_rate\tTotal_win_rate\tMean_win_rate" << endl; }
	fout << iteration << "\t" << opponent.m_sOpponentName << "\t"
		 << setprecision(2) << dWinRate.m_black << "%\t" << dWinRate.m_white << "%\t" << dTotalWinRate << "%\t"
		 << opponent.m_winRateData.getMean() << "%" << endl;
	fout.close();
}

void RLDCNN::update( bool bWithShuffle/*=true*/ )
{
	int win_idx = 0, lose_idx = 0;
	vector< pair<int,int> >& vWinIndex = m_vRLPlayoutGenerator[0]->getWinIndex();
	vector< pair<int,int> >& vLoseIndex = m_vRLPlayoutGenerator[0]->getLoseIndex();

	if( bWithShuffle ) {
		std::random_shuffle(vWinIndex.begin(), vWinIndex.end());
		std::random_shuffle(vLoseIndex.begin(), vLoseIndex.end());
	}

	COUT() << "Start to update, total has " << vWinIndex.size() << " win data(s), "
											<< vLoseIndex.size() << " lose data(s). " << endl;

#ifdef USE_CAFFE
	caffe::NetParameter net_param;
#endif
	int nPositiveUpdate = 0, nNegativeUpdate = 0;
	int dataSize = static_cast<int>(m_rlCNNNet->getBatchSize() * WeichiConfigure::dcnn_train_gpu_list.length());
	while (true) {
		// win
		if( win_idx<vWinIndex.size() ) {
			for( int i=0; i<dataSize && win_idx<vWinIndex.size(); i++ ) {
				const pair<int,int>& indexPair = vWinIndex[win_idx++];
				const RLInputData& input = m_vRLPlayoutGenerator[indexPair.first]->getInputData()[indexPair.second];
				const RLGameData& game = m_vRLPlayoutGenerator[indexPair.first]->getGameData()[input.m_gameID];
				const float fLossWeight = 1.0f-game.m_vBaselineValues[input.m_moveNumber]; 
				uint pos = game.m_vMoves[input.m_moveNumber].getCompactPosition();
				SymmetryType symmetryType = input.type;
				Color turnColor = game.m_vMoves[input.m_moveNumber].getColor();
				m_rlCNNNet->push_back(input.m_input,fLossWeight,turnColor,pos, symmetryType);
			}
#ifdef USE_CAFFE
			m_rlCNNNet->update(true);
#endif
			++nPositiveUpdate;
		}

		// lose
		if( lose_idx<vLoseIndex.size() ) {
			for( int i=0; i<dataSize && lose_idx<vLoseIndex.size(); i++ ) {
				const pair<int,int>& indexPair = vLoseIndex[lose_idx++];
				const RLInputData& input = m_vRLPlayoutGenerator[indexPair.first]->getInputData()[indexPair.second];
				const RLGameData& game = m_vRLPlayoutGenerator[indexPair.first]->getGameData()[input.m_gameID];
				const float fLossWeight = static_cast<float>(fabs(-1.0 - game.m_vBaselineValues[input.m_moveNumber])); // sign is within Learning rate
				uint pos = game.m_vMoves[input.m_moveNumber].getCompactPosition();
				SymmetryType symmetryType = input.type;
				Color turnColor = game.m_vMoves[input.m_moveNumber].getColor();
				 m_rlCNNNet->push_back(input.m_input,fLossWeight,turnColor,pos, symmetryType);
			}
#ifdef USE_CAFFE
			m_rlCNNNet->update(false);
#endif
			++nNegativeUpdate;
		}

		if (win_idx >= vWinIndex.size() && lose_idx >= vLoseIndex.size())
			break;
	}

	COUT() << "Total do " << nPositiveUpdate << " positive update(s); " << nNegativeUpdate << " negative update(s)." << endl;
	
#ifdef USE_CAFFE
	m_rlCNNNet->netParamToProto(net_param);
	for( uint i=0; i<Configure::NumThread; i++ ) { m_vRLPlayoutGenerator[i]->reloadOurNetWorkByParam(net_param); }
#endif
}

void RLDCNN::snapshotTrainingResult( int iteration )
{
	// snapshot network
	string sFile = m_shareData.m_sModelDirectory + "rl_iter_" + ToString(iteration) + ".caffemodel";
	m_vRLPlayoutGenerator[0]->saveOurNetwork(sFile);

	if( iteration%WeichiConfigure::num_add_rl_to_opp_pool==0 ) {
		m_vOppNetworkPool.push_back(RLOpponent((uint)m_vOppNetworkPool.size(),sFile));
		COUT() << "add current opponent \"" << sFile << "\"to opponent pool, total has " << m_vOppNetworkPool.size()-1 << " opponent(s)." << endl;
	}
}