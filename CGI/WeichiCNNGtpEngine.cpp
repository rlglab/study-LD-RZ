#include "WeichiCNNGtpEngine.h"
#include "GtpColorGradient.h"

void WeichiCNNGtpEngine::Register()
{
	RegisterFunction("dcnn_move_predictor", this, &WeichiCNNGtpEngine::cmdDCNNMovePredictor, 0);
	RegisterFunction("dcnn_move_order", this, &WeichiCNNGtpEngine::cmdDCNNMoveOrder, 0);
	RegisterFunction("dcnn_move_score", this, &WeichiCNNGtpEngine::cmdDCNNMoveScore, 0);
	RegisterFunction("dcnn_sim_territory", this, &WeichiCNNGtpEngine::cmdDCNNSimTerritory, 0);
	RegisterFunction("dcnn_bv_vn", this, &WeichiCNNGtpEngine::cmdDCNNBVVN, 0);
	RegisterFunction("dcnn_eye", this, &WeichiCNNGtpEngine::cmdDCNNEye, 0);
	RegisterFunction("dcnn_connect", this, &WeichiCNNGtpEngine::cmdDCNNConnect, 0);
	RegisterFunction("dcnn_connect_value", this, &WeichiCNNGtpEngine::cmdDCNNConnectValue, 0);
	RegisterFunction("dcnn_param_setting", this, &WeichiCNNGtpEngine::cmdDCNNParamSetting, 0, 2);
	RegisterFunction("dcnn_net", this, &WeichiCNNGtpEngine::cmdDCNNNet, 0);
	RegisterFunction("dcnn_reload_net", this, &WeichiCNNGtpEngine::cmdDCNNReloadNet, 1);
}

string WeichiCNNGtpEngine::getGoguiAnalyzeCommandsString()
{
	ostringstream oss;
	oss << "string/=====CNN Information=====/empty\n"
		<< "cboard/DCNN Move Predictor/dcnn_move_predictor\n"
		<< "gfx/DCNN Move Order/dcnn_move_order\n"
		<< "gfx/DCNN Move Score/dcnn_move_score\n"
		<< "dboard/DCNN Simulation Territory/dcnn_sim_territory\n"
		<< "dboard/DCNN BV VN/dcnn_bv_vn\n"
		<< "dboard/DCNN Eye/dcnn_eye\n"	
		<< "cboard/DCNN Connect/dcnn_connect\n"
		<< "sboard/DCNN Connect Value/dcnn_connect_value\n"
		<< "param/DCNN Param Setting/dcnn_param_setting\n"
		<< "gfx/DCNN Net/dcnn_net\n"
		;

	return oss.str();
}

void WeichiCNNGtpEngine::cmdDCNNMovePredictor()
{
	double dMinScore = DBL_MAX;
	double dMaxScore = -DBL_MAX;
	Vector<double,MAX_NUM_GRIDS> vScore;
	vScore.setAllAs(-DBL_MAX,MAX_NUM_GRIDS);

	vector<CandidateEntry> vCandidate = calculateSLCandidate(m_cnnRotateType);
	for( uint i=0; i<vCandidate.size(); i++ ) {
		if( vCandidate[i].getPosition()==PASS_MOVE.getPosition() || vCandidate[i].getPosition()==-1 ) { continue; }
		vScore[vCandidate[i].getPosition()] = vCandidate[i].getScore();
		if( vScore[vCandidate[i].getPosition()]>dMaxScore ) { dMaxScore = vScore[vCandidate[i].getPosition()]; }
		if( vScore[vCandidate[i].getPosition()]<dMinScore ) { dMinScore = vScore[vCandidate[i].getPosition()]; }
	}

	ostringstream oss;
	GtpColorGradient colorGradient(RGB_GREEN,RGB_BLUE,dMinScore,dMaxScore);

	oss << endl;
	CERR() << "Min value=" << dMinScore << "; Max value=" << dMaxScore << endl;
	for( StaticBoard::iterator it=StaticBoard::getGoguiIterator(); it; ++it ) {
		if( vScore[*it]==-DBL_MAX ) { oss << "\"\" "; }
		else { oss << colorGradient.colorOf(vScore[*it]).toString() << " "; }
		if( *it%MAX_BOARD_SIZE==WeichiConfigure::BoardSize-1 ) { oss << endl; }
	}

	reply(GTP_SUCC, oss.str());
}

void WeichiCNNGtpEngine::cmdDCNNMoveOrder()
{
	vector<CandidateEntry> vCandidate = calculateSLCandidate(m_cnnRotateType);
	sort(vCandidate.begin(),vCandidate.end());
	
	ostringstream oss;
	oss << "LABEL ";
	uint preOrder = 1;
	double dPreMoveScore = vCandidate[0].getScore();
	for( uint i=0; i<vCandidate.size(); i++ ) {
		if( vCandidate[i].getPosition()==PASS_MOVE.getPosition() || vCandidate[i].getPosition()==-1 ) { continue; }
		if( vCandidate[i].getScore()==dPreMoveScore ) {
			oss << WeichiMove(vCandidate[i].getPosition()).toGtpString() << " " << preOrder << " ";
		} else {
			oss << WeichiMove(vCandidate[i].getPosition()).toGtpString() << " " << (i+1) << " ";
			dPreMoveScore = vCandidate[i].getScore();
			preOrder = i+1;
		}
		if( preOrder>=m_maxOrder ) { break; }
	}
	oss << endl;

	reply(GTP_SUCC, oss.str());
}

void WeichiCNNGtpEngine::cmdDCNNMoveScore()
{
	vector<CandidateEntry> vCandidate = calculateSLCandidate(m_cnnRotateType);
	sort(vCandidate.begin(),vCandidate.end());

	ostringstream oss;
	double dMinScore = DBL_MAX;
	double dMaxScore = -DBL_MAX;
	oss << "LABEL ";
	for( uint i=0; i<vCandidate.size(); i++ ) {
		if( vCandidate[i].getPosition()==PASS_MOVE.getPosition() || vCandidate[i].getPosition()==-1 ) { continue; }
		oss << WeichiMove(vCandidate[i].getPosition()).toGtpString() << ' ' << vCandidate[i].getScore() << ' ';
		if( vCandidate[i].getScore()>dMaxScore ) { dMaxScore = vCandidate[i].getScore(); }
		if( vCandidate[i].getScore()<dMinScore ) { dMinScore = vCandidate[i].getScore(); }
		if( i>m_maxOrder ) { break; }
	}

	CERR() << "Min value=" << dMinScore << "; Max value=" << dMaxScore << endl;
	reply(GTP_SUCC, oss.str());
}

void WeichiCNNGtpEngine::cmdDCNNSimTerritory()
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState(0));
	BaseCNNPlayoutGenerator cnnGenerator(state);
	cnnGenerator.initialize(state.getSLNet()->getBatchSize());
	StatisticData blackWinRate = cnnGenerator.run(static_cast<int>(WeichiConfigure::NumSimToEnsureTerritory));

	const Territory& territory = cnnGenerator.getTerritory();
	ostringstream oss ;
	oss << endl;
	for( StaticBoard::iterator it=StaticBoard::getGoguiIterator(); it; ++it ) {
		double prob = territory.getTerritory(*it);
		oss << setw(6) << std::fixed << setprecision(3) << prob << " " ;
		if( *it%MAX_BOARD_SIZE==WeichiConfigure::BoardSize-1 ) { oss << endl; }
	}
	CERR() << "win rate: " << blackWinRate.getMean() << " (" << blackWinRate.getCount() << ")" << endl;
	reply(GTP_SUCC, oss.str());
}

void WeichiCNNGtpEngine::cmdDCNNBVVN()
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
	
	state.m_rootFilter.startFilter();
	WeichiCNNNet* cnnNet = state.m_dcnnNetCollection.getCNNNet();
	WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(state, cnnNet);
	cnnNet->forward();

	vector<float> vTerritory;
	vTerritory.resize(MAX_NUM_GRIDS, -1.0f);
	if (cnnNet->hasBVOutput()) {
		for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
			float fProb = cnnNet->getBVResult(0, *it);
			if (vTerritory[*it] == -1.0f) { vTerritory[*it] = fProb; } else { vTerritory[*it] += fProb; }
		}
	}

	ostringstream oss;
	oss << endl;

	if(cnnNet->hasBVOutput() ) {
		double dProbSum = 0;
		std::streamsize default_precision = std::cout.precision();
		for( StaticBoard::iterator it=StaticBoard::getGoguiIterator(); it; ++it ) {
			oss << setw(6) << std::fixed << setprecision(3) << vTerritory[*it] << " " ;
			if( *it%MAX_BOARD_SIZE==WeichiConfigure::BoardSize-1 ) { oss << endl; }
			dProbSum += vTerritory[*it];
		}
		oss.unsetf(ios::fixed);
		oss << setprecision(default_precision);
		CERR() << getSymmetryTypeString(cnnNet->getSymmetryType(0)) << endl;
		CERR() << "sum of probability = " << dProbSum << endl;
	}

	reply(GTP_SUCC, oss.str());
}

void WeichiCNNGtpEngine::cmdDCNNEye()
{	
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
	WeichiCNNNet* cnnNet = state.m_dcnnNetCollection.getCNNNet();

	vector<float> vTerritory;
	vTerritory.resize(MAX_NUM_GRIDS,-1.0f);	
	for( int symmetric=0; symmetric<SYMMETRY_SIZE; ++symmetric ) {
		if( m_cnnRotateType!=SYMMETRY_SIZE && m_cnnRotateType!=symmetric ) { continue; }

		state.m_rootFilter.startFilter();
		WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(state,cnnNet,static_cast<SymmetryType>(symmetric));
		cnnNet->forward();
		
		for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
			float fProb = cnnNet->getEyeResult(0,*it);
			if( vTerritory[*it]==-1.0f ) { vTerritory[*it] = fProb; }
			else { vTerritory[*it] += fProb; }
		}
	}
	if( m_cnnRotateType==SYMMETRY_SIZE ) {
		for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
			if( vTerritory[*it]==-1 ) { continue; }
			vTerritory[*it] = vTerritory[*it]/SYMMETRY_SIZE;
		}
	}

	ostringstream oss;
	oss << endl;

	double dProbSum = 0;
	std::streamsize default_precision = std::cout.precision();
	for( StaticBoard::iterator it=StaticBoard::getGoguiIterator(); it; ++it ) {
		oss << setw(6) << std::fixed << setprecision(3) << vTerritory[*it] << " " ;
		if( *it%MAX_BOARD_SIZE==WeichiConfigure::BoardSize-1 ) { oss << endl; }
		dProbSum += vTerritory[*it];
	}
	oss.unsetf(ios::fixed);
	oss << setprecision(default_precision);
	cerr << "sum of probability = " << dProbSum << endl;

	reply(GTP_SUCC, oss.str());
}

void WeichiCNNGtpEngine::cmdDCNNConnect()
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());

	vector<CandidateEntry> vCandidate;
	vCandidate.resize(MAX_NUM_GRIDS, CandidateEntry(-1, -1));
	state.m_rootFilter.startFilter();
	WeichiCNNNet* cnnNet = state.m_dcnnNetCollection.getCNNNet();
	WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(state, cnnNet);
	cnnNet->forward();

	ostringstream oss;
	GtpColorGradient colorGradientB(RGB_GREEN, RGB_BLUE, 0, 1);
	GtpColorGradient colorGradientR(RGB_RED, RGB_BLUE, 0, 1);

	oss << endl;
	for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
		float score = cnnNet->getConnectResult(0, *it);
		if (score != score) score = 0; //nan
		if (score == -DBL_MAX) { oss << "\"\" "; }
		else if (score > 0) { oss << colorGradientB.colorOf(score).toString() << " "; }
		else { oss << colorGradientR.colorOf(-score).toString() << " "; }
		if (*it%MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
	}

	reply(GTP_SUCC, oss.str());
}

void WeichiCNNGtpEngine::cmdDCNNConnectValue()
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());

	vector<CandidateEntry> vCandidate;
	vCandidate.resize(MAX_NUM_GRIDS, CandidateEntry(-1, -1));
	state.m_rootFilter.startFilter();
	WeichiCNNNet* cnnNet = state.m_dcnnNetCollection.getCNNNet();
	WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(state, cnnNet);
	cnnNet->forward();

	ostringstream oss;

	oss << endl;

	for ( StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it ) {
		float score = cnnNet->getConnectResult(0, *it);
		if (score != score) score = 0; //nan
		if (score == -DBL_MAX) { oss << "\"\" "; }
		else { oss << setw(6) << std::fixed << setprecision(2) << score << " " ; }
		if (*it%MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
	}

	reply(GTP_SUCC, oss.str());
}

void WeichiCNNGtpEngine::cmdDCNNParamSetting()
{
	ostringstream oss;
	if( m_args.size()==0 ) {
		WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());

		// SL net
		oss << "[list";
		//for( uint id=0; id<state.m_dcnnNetCollection.getNumSLNet(); id++ ) {
		for( uint id=0; id<state.m_dcnnNetCollection.getNumCNNNet(); id++ ) {
			//WeichiCNNSLNet* slNet = state.m_dcnnNetCollection.getSLNet(id);
			WeichiCNNNet* slNet = state.m_dcnnNetCollection.getCNNNet(id);
			if( slNet==nullptr ) { continue; }
			string sPrototxt = slNet->getNetParam().m_sPrototxt;
			sPrototxt = sPrototxt.substr(sPrototxt.find_last_of("/")+1);
			oss << "/" << id << "." << sPrototxt;
		}
		//WeichiCNNSLNet* slNet = state.m_dcnnNetCollection.getSLNet(m_slNetID);
		WeichiCNNNet* slNet = state.m_dcnnNetCollection.getCNNNet(m_slNetID);
		if( slNet!=nullptr ) {
			string sPrototxt = slNet->getNetParam().m_sPrototxt;
			sPrototxt = sPrototxt.substr(sPrototxt.find_last_of("/")+1);
			oss << "] dcnn_sl_net " << m_slNetID << "." << sPrototxt << "\n";
		} else { oss << "] dcnn_sl_net --\n"; }
		
		// BV VN net
		oss << "[list";
		for( uint id=0; id<state.m_dcnnNetCollection.getNumBVVNNet(); id++ ) {
			WeichiCNNBVVNNet* bvvnNet = state.m_dcnnNetCollection.getBVVNNet(id);
			if( bvvnNet==nullptr ) { continue; }
			string sPrototxt = bvvnNet->getNetParam().m_sPrototxt;
			sPrototxt = sPrototxt.substr(sPrototxt.find_last_of("/")+1);
			oss << "/" << id << "." << sPrototxt;
		}
		WeichiCNNBVVNNet* bvvnNet = state.m_dcnnNetCollection.getBVVNNet(m_bvvnNetID);
		if( bvvnNet!=nullptr ) {
			string sPrototxt = bvvnNet->getNetParam().m_sPrototxt;
			sPrototxt = sPrototxt.substr(sPrototxt.find_last_of("/")+1);
			oss << "] dcnn_bvvn_net " << m_bvvnNetID << "." << sPrototxt << "\n";
		} else { oss << "] dcnn_bvvn_net --\n"; }

		// symmetry type
		oss << "[list";
		for( int symmetric=0; symmetric<SYMMETRY_SIZE; symmetric++ ) {
			oss << "/" << getSymmetryTypeString(static_cast<SymmetryType>(symmetric));
		}
		oss << "/average] dcnn_rotate_direction ";
		if( m_cnnRotateType==SYMMETRY_SIZE ) { oss << "average\n"; }
		else { oss << getSymmetryTypeString(m_cnnRotateType) << "\n"; }

		oss	<< "[string] max_order " << m_maxOrder << "\n";

		oss << "[string] simulation_time_for_tree_search " << m_iTreeSearchSimulationNum << "\n";

	} else if( m_args.size()==2 ) {
		if( m_args[0]=="dcnn_sl_net" ) {
			m_slNetID = m_args[1][0] - '0';
		} else if( m_args[0]=="dcnn_bvvn_net" ) {
			m_bvvnNetID = m_args[1][0] - '0';
		} else if( m_args[0]=="dcnn_rotate_direction" ) {
			m_cnnRotateType = getSymmetryType(m_args[1]);
		} else if( m_args[0]=="max_order" ) {
			m_maxOrder = atoi(m_args[1].c_str());
		} else if(m_args[0]=="simulation_time_for_tree_search"){
			m_iTreeSearchSimulationNum = atoi(m_args[1].c_str());
		} else {
			reply(GTP_FAIL, "unknown parameter " + m_args[0]);
		}
	} else {
		reply(GTP_FAIL, "need 0 or 2 arguments");
	}

	reply(GTP_SUCC, oss.str());
}

void WeichiCNNGtpEngine::cmdDCNNNet()
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());

	state.m_rootFilter.startFilter();
	WeichiCNNNet* cnnNet = state.m_dcnnNetCollection.getCNNNet(m_slNetID);
	WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(state, cnnNet);
	cnnNet->forward();
	vector<CandidateEntry> vCandidate = cnnNet->getSLCandidates(0);
	sort(vCandidate.begin(), vCandidate.end());

	ostringstream oss;
	oss << "LABEL ";
	double dPreMoveScore = vCandidate[0].getScore();
	for (uint i = 0; i < vCandidate.size(); i++) {
		if (vCandidate[i].getPosition() == PASS_MOVE.getPosition() || vCandidate[i].getPosition() == -1) { continue; }
		oss << WeichiMove(vCandidate[i].getPosition()).toGtpString() << " " << vCandidate[i].getScore() << " ";
	}
	oss << endl;
	
	CERR() << "Other probability: ";
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		if (!cnnNet->isLegal(0, *it)) { CERR() << WeichiMove(*it).toGtpString() << " " << cnnNet->getSLResult(0,*it) << " "; }
	}
	if (!cnnNet->isLegal(0, PASS_MOVE.getPosition())) {
		CERR() << PASS_MOVE.toGtpString() << " " << cnnNet->getSLResult(0, PASS_MOVE.getPosition());
	}
	CERR() << endl;

	CERR() << "To Play: " << toChar(state.m_board.getToPlay()) << endl;
	oss << "To Play: " << toChar(state.m_board.getToPlay()) << endl;
	if (WeichiConfigure::dcnn_use_ftl) {
		for (int i = 0; i < cnnNet->getVNLabelSize(); ++i) {
			CERR() << (i + 1) << ": " << cnnNet->getVNResult(0, i) << "\t";
			if ((i + 1) % 10 == 0) { CERR() << endl; }
		}
	} else {
		float fCenterKomi = (WeichiConfigure::BoardSize == 7 ? 0 : 7.5);
		double dMinKomi = fCenterKomi - (cnnNet->getVNLabelSize() - 1) / 2;
		double dMaxKomi = fCenterKomi + (cnnNet->getVNLabelSize() - 1) / 2;
		for (double dKomi = dMinKomi; dKomi <= dMaxKomi; dKomi++) {
			CERR() << dKomi << ": " << cnnNet->getVNResult(0, dKomi) << "\t";
			oss << dKomi << ": " << cnnNet->getVNResult(0, dKomi) << "\t";
			if ((static_cast<int>(dKomi - dMaxKomi) + 1) % 5 == 0) { CERR() << endl; }
		}
	}	
	CERR() << endl;

	if (cnnNet->hasGLOutput()) {
		CERR() << "GL output: " << endl;
		int middle_index = -1;
		float fAverage = 0.0f, fSum = 0.0f;
		vector<float> vGLResults = cnnNet->getGLResult(0);
		for (int i = 0; i < vGLResults.size(); ++i) {
			fSum += vGLResults[i];
			fAverage += vGLResults[i] * (i + 1);
			if (fSum >= 0.5 && middle_index == -1) { middle_index = i + 1; }
			CERR() << (i + 1) << ": " << vGLResults[i] << "\t";
			if ((i + 1) % 10 == 0) { CERR() << endl; }
		}
		CERR() << endl;
		int max_index = distance(vGLResults.begin(), max_element(vGLResults.begin(), vGLResults.end()) - *vGLResults.begin()) + 1;
		CERR() << "max index: " << max_index << ", value = " << cnnNet->getVNResult(0, max_index) << endl;
		CERR() << "avg: " << fAverage << ", value = " << cnnNet->getVNResult(0, fAverage) << endl;
		CERR() << "middle index: " << middle_index << ", value = " << cnnNet->getVNResult(0, middle_index - 1) << ", diff = " << fabs(cnnNet->getVNResult(0, middle_index - 1) - cnnNet->getVNResult(0, cnnNet->getVNLabelSize() - 1)) << endl;

		fSum = 0.0f;
		for (int i = 0; i < vGLResults.size(); ++i) {
			fSum += vGLResults[i];

			float Va = (cnnNet->getVNResult(0, i) + 1) / 2;
			//float ans = R*(Va + fEpsilion) + 2 * Va + fEpsilion;
			float ans = Va + (1 - fSum) / 2;
			ans = ans * 2 - 1;

			CERR() << (i + 1) << ": " << ans << "\t";
			if ((i + 1) % 10 == 0) { CERR() << endl; }
		}
		CERR() << endl;
	}

	reply(GTP_SUCC, oss.str());
}

void WeichiCNNGtpEngine::cmdDCNNReloadNet()
{
	for (uint i = 0; i < m_mcts.getNumThreads(); i++) {
		if (!m_mcts.getState(i).m_dcnnNetCollection.hasCNNNet()) { continue; }
		const_cast<WeichiThreadState&>(m_mcts.getState(i)).m_dcnnNetCollection.getCNNNet()->reloadNetWork(m_args[0]);
	}

	reply(GTP_SUCC, "");
}

vector<CandidateEntry> WeichiCNNGtpEngine::calculateSLCandidate( SymmetryType type/*=SYM_NORMAL*/ )
{
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());

	vector<CandidateEntry> vCandidate;
	vCandidate.resize(MAX_NUM_GRIDS,CandidateEntry(-1,-1));
	for( int symmetric=0; symmetric<SYMMETRY_SIZE; symmetric++ ) {
		if( type!=SYMMETRY_SIZE && type!=symmetric ) { continue; }

		state.m_rootFilter.startFilter();
		WeichiCNNNet* cnnNet = state.m_dcnnNetCollection.getCNNNet(m_slNetID);
		WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(state,cnnNet,static_cast<SymmetryType>(symmetric));
		cnnNet->forward();

		for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
			if( !cnnNet->isLegal(0,*it) ) { continue; }
			double dProb = cnnNet->getSLResult(0,*it);
			if( vCandidate[*it].getPosition()==-1 ) { vCandidate[*it] = CandidateEntry(*it,dProb); }
			else { vCandidate[*it].setScore(vCandidate[*it].getScore()+dProb); }
		}
	}

	if( type==SYMMETRY_SIZE ) {
		for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
			if( vCandidate[*it].getPosition()==-1 ) { continue; }
			vCandidate[*it].setScore(vCandidate[*it].getScore()/SYMMETRY_SIZE);
		}
	}

	return vCandidate;
} 
