#include "Train.h"
#include "SgfLoader.h"
#include "WeichiPlayoutAgent.h"
#include "WeichiBadMoveReader.h"
#include "WeichiRadiusPatternTable.h"

void Train::start()
{
	if( WeichiConfigure::BoardSize!=19 ) {
		COUT() << "Attention: Current board size is " << WeichiConfigure::BoardSize << "!!!" << endl;
	}
	
	// create threads
	COUT() << "using " << Configure::NumThread << " thread(s) to train." << endl;
	int total_gpu_size = static_cast<int>(WeichiConfigure::dcnn_train_gpu_list.size());
	for( uint i=0; i<Configure::NumThread; i++ ) {
		uint gpuID = (total_gpu_size>0)? (WeichiConfigure::dcnn_train_gpu_list[i%total_gpu_size] - '0'): -1;
		TrainGenerator* generator = new TrainGenerator(m_shareData,i+1,gpuID);
		m_threads.create_thread(boost::bind(&TrainGenerator::run,generator));
		m_vTrainGenerator.push_back(generator);
	}

	uint type;
	COUT() << "Choose train mode:" << endl;
	for( uint i=0; i<TRAINMODE_SIZE; i++ ) {
		COUT() << "\t" << i << ". " << getTrainModeString(static_cast<TrainMode>(i)) << endl;
	}

	cin >> type;
	m_shareData.m_mode = static_cast<TrainMode>(type);
	if( m_shareData.m_mode==TRAINMODE_EXIT ) { return; }

	if( isNeedParallelMode() ) { doParallelMode(); }
	else { doNonParallelMode(); }
}

bool Train::isNeedParallelMode()
{
	// since the number of non-parallel mode is less, we only pick non-parallel mode
	switch( m_shareData.m_mode ) {
	case TRAINMODE_GENERATE_RADIUS3_SYMMETRIC_INDEX_DB:
		return false;
	default:
		return true;
	}
}

bool Train::doParallelMode()
{
	if( !initialThreadShareData() ) { return false; }
	for( uint i=0; i<Configure::NumThread; i++ ) { m_vTrainGenerator[i]->startRun(); }
	for( uint i=0; i<Configure::NumThread; i++ ) { m_vTrainGenerator[i]->finishRun(); }
	summarizeThreadTrainResult();

	return true;
}

bool Train::initialThreadShareData()
{
	if( !initializeDirectory() ) { return false; }
	Random::reset(Configure::RandomSeed);

	switch( m_shareData.m_mode ) {
	case TRAINMODE_COLLECT_RADIUS_PATTERN:
		m_shareData.m_fTrainResult.open("RadiusPatternWinLose.txt",ios::out);
		break;
	case TRAINMODE_GENERATE_UCT_FEATURES:
		generateUctFeatureHeader();
		m_shareData.m_fTrainResult.open("UCTFeatureWinLose.txt",ios::out);
		break;
	case TRAINMODE_GENERATE_PLAYOUT_FEATURES:
		generatePlayoutFeatureHeader();
		m_shareData.m_fTrainResult.open("PlayoutFeatureWinLose.txt",ios::out);
		break;
	case TRAINMODE_FIND_SGF_ATTRIBUTE:
		m_shareData.m_fTrainResult.open("sgf_attribute.txt",ios::out);
		break;
	case TRAINMODE_FIND_REPETITION_SGF:
#if defined __GNUC__
		m_shareData.m_fTrainResult.open("removeRepeatedSgf.sh",ios::out);
#else
		m_shareData.m_fTrainResult.open("removeRepeatedSgf.bat",ios::out);
#endif
		break;
	case TRAINMODE_LOG_NET_RECORD:
		cerr << "input num Sample: ";
		cin >> m_shareData.m_numSample;
		m_shareData.m_fTrainResult.open("net_loss.txt", ios::out);
		break;
	case TRAINMODE_WRITE_DEVELOPMENT_COMMAND:
		m_shareData.m_fTrainResult.open("development_command.txt", ios::out);
		m_shareData.m_fTrainResult << "clear_file development_command_result.txt" << endl;
		m_shareData.m_fTrainResult << "open_file development_command_result.txt" << endl;
		break;
	case TRAINMODE_FTL_POLICY:		
		m_shareData.m_fTrainResult.open("FTL_policy.txt", ios::out);
		break;
	default:
		if( !isNeedParallelMode() ) { CERR() << "error parallel mode" << endl; return false; }
		break;
	}

	return true;
}

bool Train::initializeDirectory()
{
	COUT() << "input directory name: ";
	string sDirName;
	cin >> sDirName;

	if( !boost::filesystem::exists(sDirName) ) {
		CERR() << boost::format("directory \"%1%\" not exists.") % sDirName << endl;
		return false;
	}

	m_shareData.m_fileExplorer.reset();
	m_shareData.m_fileExplorer.setRootDirectory(sDirName);

	return true;
}

void Train::summarizeThreadTrainResult()
{
	// In summarize, we will give all result to thread 0
	for( uint i=1; i<Configure::NumThread; i++ ) { m_vTrainGenerator[0]->summarizeNumberMoves(*m_vTrainGenerator[i]); }
	string sFileName;
	fstream fNumLines;
	switch( m_shareData.m_mode ) {
	case TRAINMODE_COLLECT_RADIUS_PATTERN:
		// write remain radius pattern
		for( uint i=0; i<Configure::NumThread; i++ ) { m_vTrainGenerator[i]->writeRadiusPatternCollection(); }
		break;
	case TRAINMODE_CALCULATE_UCT_MOVE_PREDICTION:
	case TRAINMODE_CALCULATE_PLAYOUT_MOVE_PREDICTION:
		for( uint i=1; i<Configure::NumThread; i++ ) { m_vTrainGenerator[0]->summarizeMovePrediction(*m_vTrainGenerator[i]); }
		if( m_shareData.m_mode==TRAINMODE_CALCULATE_UCT_MOVE_PREDICTION ) { m_vTrainGenerator[0]->outputPredictionRateFile("UCT_prediction_rate.txt"); }
		else { m_vTrainGenerator[0]->outputPredictionRateFile("Playout_prediction_rate.txt"); }
		break;
	case TRAINMODE_FIND_REPETITION_SGF:
		for( uint i=1; i<Configure::NumThread; i++ ) { m_vTrainGenerator[0]->summarizeRepeatedSgfData(*m_vTrainGenerator[i]); }
		m_vTrainGenerator[0]->outputRepeatedSgfFile();
		break;
	default:
		break;
	}
	fNumLines.close();
}

bool Train::doNonParallelMode()
{
	switch( m_shareData.m_mode ) {
	case TRAINMODE_GENERATE_RADIUS3_SYMMETRIC_INDEX_DB:
		generateRadius3SymmetricIndexDB(); break;
	default:
		if( isNeedParallelMode() ) { CERR() << "error non-parallel mode" << endl; return false; }
		break;
	}

	return true;
}

void Train::generateRadius3SymmetricIndexDB()
{
	COUT() << "Generate radius3_symmetric_index.db ..." << endl;

	// save radius pattern table database
	ofstream fDB("radius3_symmetric_index.db",ios::out|ios::binary);
	WeichiRadiusPatternTable::saveToDB(fDB);
	fDB.close();
}

void Train::generateUctFeatureHeader()
{
	uint numFeature = MOVE_FEATURE_SIZE;
	uint numFeatureSet = (MOVEFEATURE_TYPE_NUM+1) + (MAX_RADIUS_SIZE-MIN_RADIUS_SIZE);	// +1 for MF_NOT_IN_CANDIDATE feature

	for( uint iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		numFeature += WeichiGammaTable::getRadiusPatternCount(iRadius);
	}

	m_fTrainResult.open("UCTFeatureWinLose_header.txt",ios::out);
	m_fTrainResult << "! " << numFeature << endl;
	m_fTrainResult << numFeatureSet << endl;

	// for fixed feature
	m_fTrainResult << "1 NOT_IN_CANDIDATE\n";
	for( uint i=0; i<MOVEFEATURE_TYPE_NUM; i++ ) {
		uint currentIndex = MOVEFEATURE_START_INDEX[i];
		uint nextIndex = (i==MOVEFEATURE_TYPE_NUM-1) ? MOVE_FEATURE_SIZE: MOVEFEATURE_START_INDEX[i+1];

		m_fTrainResult << (nextIndex-currentIndex) << ' '
					   << getWeichiMoveFeatureTypeName(MOVEFEATURE_START_INDEX[i])
					   << endl;
	}

	// for radius pattern feature
	for( uint iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		m_fTrainResult << WeichiGammaTable::getRadiusPatternCount(iRadius) << ' '
					   << "Radius" << iRadius << endl;
	}

	m_fTrainResult << "!\n";
	m_fTrainResult.close();
}

void Train::generatePlayoutFeatureHeader()
{
	m_fTrainResult.open("PlayoutFeatureWinLose_header.txt",ios::out);
	m_fTrainResult << "! " << MAX_NUM_PLAYOUT_FEATURES << endl;
	m_fTrainResult << MAX_NUM_PLAYOUT_FEATURE_GROUPS << endl;

	m_fTrainResult << MAX_3X3PATTERN_SIZE << " pattern3x3" << endl;
	m_fTrainResult << MAX_3X3PATTERN_SIZE << " reply_pattern3x3" << endl;
	for( uint i=0; i<POLICY_SIZE; i++ ) {
		m_fTrainResult << 1 << " " << getWeichiPlayoutPolicyString(static_cast<WeichiPlayoutPolicy>(i)) << endl;
	}

	m_fTrainResult << "!\n";
	m_fTrainResult.close();	
} 
