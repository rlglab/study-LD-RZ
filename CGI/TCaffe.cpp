#include "TCaffe.h"

void TCaffeSlave::initialize()
{
	TBaseSlave::initialize();

	m_label = new float[m_sharedData.m_cnnTrainNet->getLabelSize()];
}

bool TCaffeSlave::isOver()
{
	return false;
}

void TCaffeSlave::reset()
{
}

void TCaffeSlave::doSlaveJob()
{
	while( !m_sharedData.m_cnnTrainNet->isFull() ) {
		calculateFeatures();
	}
}

SgfInformation TCaffeSlave::getNextSgf()
{
	boost::lock_guard<boost::mutex> lock(m_sharedData.m_mutex);

	SgfInformation sgfInformation;
	while( sgfInformation.m_sFilename=="" ) {
		FileDirectoryExplorer& dirExplorer = m_sharedData.m_bIsTraining? m_sharedData.m_trainDirExplorer: m_sharedData.m_testDirExplorer;
		if( dirExplorer.isEndSgf() ) { resetFileDirectory(); }
		sgfInformation = dirExplorer.getNextSgfInformation();
	}

	return sgfInformation;
}

void TCaffeSlave::calculateFeatures()
{
	SgfInformation sgfInformation = getNextSgf();
	string sSgfString = sgfInformation.m_sSgfString;

	SgfLoader sgfLoader;
	if (!sgfLoader.parseFromString(sSgfString)) { return; }

	const vector<WeichiMove>& vPreset = sgfLoader.getPreset();
	const vector<WeichiMove>& vMoves = sgfLoader.getPlayMove();
	const Color winner = sgfLoader.getWinner();
	if (vPreset.size() > 0) { CERR() << "[Warning] Sgf with preset\n" << sSgfString << endl; return; }
	if (vMoves.size() <= 0) { CERR() << "[Warning] Sgf without any move\n" << sSgfString << endl; return; }
	//if( winner==COLOR_NONE ) { CERR() << "[Warning] Sgf with unknown winner\n" << sSgfString << endl; return; }

	// random a position and play
	WeichiCNNTrainNet* cnnTrainNet = m_sharedData.m_cnnTrainNet;
	int rand_moves = pickRandomMove(sgfLoader);
	if (rand_moves == -1) { CERR() << "[Warning] Cannot pick a random move\n" << sSgfString << endl; return; }

	m_state.resetThreadState();
	for (int i = 0; i < rand_moves; i++) {
		if (m_state.m_board.isIllegalMove(vMoves[i], m_state.m_ht)) {
			CERR() << "[Warning] Sgf with illegal move " << i + 1 << '\n' << sSgfString << endl;
			return;
		}
		m_state.play(vMoves[i]);
	}

	// features
	WeichiBitBoard bmLegal;
	m_state.m_rootFilter.startFilter();
	float features[MAX_CNN_CHANNEL_SIZE*MAX_NUM_GRIDS];
	WeichiCNNFeatureGenerator::calculateDCNNFeatures(m_state, cnnTrainNet->getCNNFeatureType(), features, bmLegal);

	//---------------------------label---------------------------//
	Color turnColor = m_state.m_board.getToPlay();
	float* fLabel = m_label;

	// SL
	if (cnnTrainNet->hasSLOutput()) {
		const WeichiMove& move = vMoves[rand_moves];
		fLabel[0] = static_cast<float>(WeichiMove::toCompactPosition(move.getPosition()));
		fLabel++;
	}

	// BV
	if (cnnTrainNet->hasBVOutput()) {
		for (uint i = rand_moves; i < vMoves.size(); i++) { m_state.play(vMoves[i]); }
		m_state.eval();
		Territory territory;
		territory.addTerritory(m_state.m_board.getTerritory(), winner);
		calculateBVLabel(fLabel, sgfLoader, turnColor, territory);
		fLabel += m_sharedData.m_cnnTrainNet->getBVLabelSize();
	}

	// VN
	if (cnnTrainNet->hasVNOutput()) {
		calculateVNLabel(fLabel, sgfLoader, turnColor);
	}
	//---------------------------label---------------------------//

	// store features & log info
	boost::lock_guard<boost::mutex> lock(m_sharedData.m_mutex);
	if (m_sharedData.m_cnnTrainNet->isFull()) { return; }
	m_sharedData.m_cnnTrainNet->push_back(m_sharedData.m_bUpdateFirstBatch, features, turnColor, m_label, static_cast<float>(rand_moves));
	int real_batch_size = m_sharedData.m_cnnTrainNet->getCurrentBatchSize();
	real_batch_size += (m_sharedData.m_bUpdateFirstBatch ? m_sharedData.m_cnnTrainNet->getBatchSize() * WeichiConfigure::dcnn_train_gpu_list.length() : 0);
	m_sharedData.m_logFile << TimeSystem::getTimeString("[Y/m/d_H:i:s] ")
		<< "Thread " << setw(2) << m_id
		<< ": file = \"" << sgfInformation.m_sFilename
		<< "\", line = " << setw(8) << sgfInformation.m_iFileLine
		<< ", move = " << setw(3) << rand_moves
		<< ", color = " << toChar(turnColor)
		<< ", rotate = " << m_sharedData.m_cnnTrainNet->getRotateType(real_batch_size - 1)
		<< ", batch = " << setw(3) << real_batch_size - 1
		<< ", label = ";
	float* fTrainNetLabel = m_sharedData.m_cnnTrainNet->getLabel(real_batch_size - 1);
	for (int i = 0; i < m_sharedData.m_cnnTrainNet->getLabelSize(); i++) { m_sharedData.m_logFile << fTrainNetLabel[i] << " "; }
	m_sharedData.m_logFile << endl;
}

void TCaffeSlave::calculateVNLabel(float *label, const SgfLoader& sgfLoader, Color turnColor)
{
	string sResultInfo = sgfLoader.getSgfTag("RE");
	const Color winner = sgfLoader.getWinner();
	float fBWinCount = static_cast<float>(atof(sResultInfo.substr(2).c_str()));
	
	fBWinCount = (winner == COLOR_BLACK) ? fBWinCount + sgfLoader.getKomi() : -fBWinCount + sgfLoader.getKomi();
	int vnLabel_size = m_sharedData.m_cnnTrainNet->getVNLabelSize();
	float komi = WeichiConfigure::komi - (vnLabel_size / 2)*WeichiConfigure::caffe_komi_vn_step;
	for (int i = 0; i<vnLabel_size; i++) {
		float bWinRate = (fBWinCount - komi>0) ? 1.0f : -1.0f;
		if (turnColor == COLOR_WHITE) { bWinRate = -bWinRate; }
		label[i] = bWinRate;
		komi += WeichiConfigure::caffe_komi_vn_step;
	}
}

void TCaffeSlave::calculateBVLabel( float *label, const SgfLoader& sgfLoader, Color turnColor, const Territory& territory )
{
	if (m_sharedData.m_cnnTrainNet->getBVLabelSize() != WeichiConfigure::TotalGrids){
		CERR() << "[Warning] BV label size != " << WeichiConfigure::TotalGrids << endl;
	}
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		float dTerritory = static_cast<float>(territory.getTerritory(*it));
		if (turnColor == COLOR_WHITE) { dTerritory = -dTerritory; }
		label[WeichiMove(*it).getCompactPosition()] = (dTerritory + 1) / 2;
	}
}

int TCaffeSlave::pickOpeningMove(const SgfLoader &sgfLoader)
{
	if( m_sharedData.m_cnnTrainNet->getCNNNetType()!=CNN_NET_BV_VN ) {
		CERR() << "[ERROR] Opening only support in BV_VN network" << endl;
		exit(1);
	}

	const string &event = sgfLoader.getSgfTag("EV");
	string sEvent = event.substr(0, event.find(";"));
	int max = atoi(sEvent.c_str());

	int randomMoveIndex = Random::nextInt(max);
	randomMoveIndex = min(randomMoveIndex, static_cast<int>(sgfLoader.getPlayMove().size()));
	return randomMoveIndex;
}

int TCaffeSlave::pickRandomMove(const SgfLoader &sgfLoader)
{
	WeichiCNNTrainNet *cnnTrainNet = m_sharedData.m_cnnTrainNet;
	const vector<WeichiMove>& vMoves = sgfLoader.getPlayMove();
	int max_moves = (static_cast<int>(vMoves.size()) > MAX_MOVES) ? MAX_MOVES : static_cast<int>(vMoves.size());
	if( sgfLoader.getSgfTag("EV")!="" ) {
		const string &event = sgfLoader.getSgfTag("EV");
		string sEvent = event.substr(0, event.find(";"));
		max_moves = atoi(sEvent.c_str());
	}

	return Random::nextInt(max_moves);

	/*int openingMove = 0;
	if( WeichiConfigure::caffe_use_opening ) {
		// format: Open:42 ...
		bool isLegal = true;
		string sSgfString = sgfLoader.getSgfString();
		size_t start_index, end_index;
		if( (start_index=sSgfString.find("Open:"))==string::npos ) { isLegal = false; }
		start_index += 5;
		if( isLegal == false || (end_index=sSgfString.find(" ",start_index))==string::npos ) { isLegal = false; }
		if ( isLegal ){
			string number = sSgfString.substr(start_index, end_index-start_index);
			openingMove = atoi(number.c_str());
			if ( openingMove < 0 || (openingMove == 0 &&  number[0] != '0') ){ isLegal = false; }
		}
		if (isLegal == false){ CERR() << "[Error] Parse opening move failed, get \"" << openingMove << "\"\n" << sSgfString << endl; exit(1); }
		if (openingMove >= vMoves.size()) { CERR() << "[Error] Opening move is larger than game length, get \"" << openingMove << "\"\n" << sSgfString << endl; exit(1); }
	}

	if (cnnTrainNet->getCNNNetType() == CNN_NET_SL && cnnTrainNet->getLabelSize() > 1){
		if (max_moves < openingMove + cnnTrainNet->getLabelSize()){ return -1; }
		max_moves -= (cnnTrainNet->getLabelSize() - 1);
	}
	int rand_moves = Random::nextInt(openingMove,max_moves);
	if (cnnTrainNet->getCNNNetType() == CNN_NET_SL && WeichiConfigure::caffe_sl_only_win_move){
		if (sgfLoader.getWinner() != vMoves[rand_moves].getColor()){
			++rand_moves;
			if (rand_moves >= max_moves){
				if (rand_moves >= 2 + openingMove) { rand_moves -= 2; }
				else{ return -1; }
			}
		}
	}
	return rand_moves;*/
}

SgfInformation TCaffeSlave::pickRandomSgf()
{
	SgfInformation sgfInformation;
	FileDirectoryExplorer& dirExplorer = m_sharedData.m_bIsTraining? m_sharedData.m_trainDirExplorer: m_sharedData.m_testDirExplorer;
	if( dirExplorer.isEndSgf() ) { resetFileDirectory(); }
	return dirExplorer.getNextSgfInformation();
}

bool TCaffeSlave::resetFileDirectory()
{
	string sDirectory = m_sharedData.m_bIsTraining? WeichiConfigure::caffe_train_dir: WeichiConfigure::caffe_test_dir;
	FileDirectoryExplorer& dirExplorer = m_sharedData.m_bIsTraining? m_sharedData.m_trainDirExplorer: m_sharedData.m_testDirExplorer;

	CERR() << "Total " << dirExplorer.getSgfCount() << " files" << endl;
	if( WeichiConfigure::caffe_train_recursively ) {
		dirExplorer.reset();
		dirExplorer.setRootDirectory(sDirectory);
		CERR() << "Traverse all " << (m_sharedData.m_bIsTraining? "training": "testing") << " set, start again." << endl;
		return true;
	}

	return false;
}

void TCaffeMaster::train()
{
	if( !initialize() ) { return; }

	// collect data at the first time
	m_sharedData.m_bUpdateFirstBatch = false;
	for( int i=0; i<m_nThread; i++ ) { m_vSlaves[i]->startRun(); }
	for( int i=0; i<m_nThread; i++ ) { m_vSlaves[i]->finishRun(); }

	while( true ) {
		int iteration = m_sharedData.m_cnnTrainNet->getIteration()+1;
		m_sharedData.m_logFile << TimeSystem::getTimeString("[Y/m/d_H:i:s] ") << "Iteration: " << iteration << endl;
		
		m_sharedData.m_bUpdateFirstBatch = !m_sharedData.m_bUpdateFirstBatch;
		m_sharedData.m_cnnTrainNet->resetCurrentBatchSize();
		for( int i=0; i<m_nThread; i++ ) { m_vSlaves[i]->startRun(); }
		updateCNNNet();
		for( int i=0; i<m_nThread; i++ ) { m_vSlaves[i]->finishRun(); }
		summarizeSlavesData();
	}
}

bool TCaffeMaster::initialize()
{
	// set directory
	if( WeichiConfigure::caffe_train_dir!="" ) {
		m_sharedData.m_trainDirExplorer.reset();
		m_sharedData.m_trainDirExplorer.setRootDirectory(WeichiConfigure::caffe_train_dir);
		CERR() << "Open training directory " << WeichiConfigure::caffe_train_dir << endl;
	} else {
		CERR() << "Couldn't find training directory " << WeichiConfigure::caffe_train_dir << endl;
		return false;
	}
	if( WeichiConfigure::caffe_test_dir!="" ) {
		m_sharedData.m_testDirExplorer.reset();
		m_sharedData.m_testDirExplorer.setRootDirectory(WeichiConfigure::caffe_test_dir);
		CERR() << "Open testing directory " << WeichiConfigure::caffe_test_dir << endl;
	} else {
		CERR() << "Couldn't find testing directory " << WeichiConfigure::caffe_test_dir << endl;
		return false;
	}

	// log file
	m_sharedData.m_logFile.open(TimeSystem::getTimeString("Y_m_d_H_i_s")+".log",ios::out);

	// set train network
	string sSolver = WeichiConfigure::dcnn_dir + WeichiConfigure::caffe_dcnn_solver;
	m_sharedData.m_cnnTrainNet = new WeichiCNNTrainNet(CNNNetParam(WeichiConfigure::caffe_dcnn_net), sSolver);
	m_sharedData.m_cnnTrainNet->init(false);
	m_sharedData.m_bIsTraining = true;

	// check dcnn net type
	if (m_sharedData.m_cnnTrainNet->getCNNNetType() == CNN_NET_UNKNOWN) { return false; }
	if (m_sharedData.m_cnnTrainNet->getCNNFeatureType() == CNN_FEATURE_UNKNOWN) { return false; }

	// display message
	CERR() << "Total create " << m_nThread << " thread(s)." << endl;
	CERR() << "Training data path: " << WeichiConfigure::caffe_train_dir << endl;
	CERR() << "Train recursively: " << WeichiConfigure::caffe_train_recursively << endl;
	CERR() << "VNML komi step: " << WeichiConfigure::caffe_komi_vn_step << endl;
	CERR() << "SL only train win moves: " << WeichiConfigure::caffe_sl_only_win_move << endl;
	CERR() << "Use sgf containing opening: " << WeichiConfigure::caffe_use_opening << endl;
	CERR() << m_sharedData.m_cnnTrainNet->getNetInfomation() << endl;

	return TBaseMaster::initialize();
}

TCaffeSlave* TCaffeMaster::newSlave( int id )
{
	return new TCaffeSlave(id,m_sharedData);
}

void TCaffeMaster::summarizeSlavesData()
{
}

void TCaffeMaster::updateCNNNet()
{
	m_sharedData.m_cnnTrainNet->update(m_sharedData.m_bUpdateFirstBatch);
	m_sharedData.m_logFile << TimeSystem::getTimeString("[Y/m/d_H:i:s] ") << "Update Info: " << endl;
	m_sharedData.m_logFile << m_sharedData.m_cnnTrainNet->getCaffeLossInfo() << endl;
}
