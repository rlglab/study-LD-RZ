#include "TrainGenerator.h"
#include "WeichiPlayoutAgent.h"
#include "BaseCNNPlayoutGenerator.h"

void TrainGenerator::run()
{
	m_barStart.wait();

	initialize();

	while(true) {
		m_sgfInformation = m_shareData.m_fileExplorer.getNextSgfInformation(true);
		if (m_sgfInformation.m_sSgfString == ""){
			break;
		}
		++m_iSgfCounter;
		playOneGame(m_sgfInformation.m_sSgfString);
	}

	if (m_shareData.m_mode == TRAINMODE_LOG_NET_RECORD) { forwardNet(); }	

	m_barFinish.wait();
}

void TrainGenerator::summarizeNumberMoves( const TrainGenerator& rhs )
{
	m_numMoves += rhs.m_numMoves;
}

void TrainGenerator::writeRadiusPatternCollection()
{
	// since this is shared file, need to lock when one thread writing
	// format: radius hash_key total win index,index... (use tab to split)
	ostringstream oss;
	for( uint iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		for( map< HashKey64, RadiusPatternEntry >::iterator it=m_vRadiusPatternCollection[iRadius].begin(); it!=m_vRadiusPatternCollection[iRadius].end(); it++ ) {
			RadiusPatternEntry& entry = it->second;
			oss << iRadius << "\t"
				<< hex << it->first << "\t"
				<< dec << entry.total << "\t"
				<< entry.win << "\t";
			oss << entry.vIndex[MIN_RADIUS_SIZE];
			for( uint i=MIN_RADIUS_SIZE+1; i<=iRadius; i++ ) { oss << "," << entry.vIndex[i]; }
			oss << endl;
		}
		m_vRadiusPatternCollection[iRadius].clear();
	}
	
	writeLock();
	m_shareData.m_fTrainResult << oss.str();
	writeUnlock();
}

void TrainGenerator::summarizeMovePrediction( const TrainGenerator& rhs )
{
	m_movePredictorStatistic.summarizeStatisticData(rhs.m_movePredictorStatistic);
}

void TrainGenerator::outputPredictionRateFile( string sFileName )
{
	m_movePredictorStatistic.outputMovePredictorStatistic(sFileName);
}

void TrainGenerator::outputEvaluationError( string sFileName )
{
	fstream fout(sFileName.c_str(),ios::out);
	for( uint i=0; i<MAX_GAME_LENGTH; i++ ) {
		if( m_vNumEvaluation[i]==0 ) { break; }
		fout << i << '\t' << m_vSquareError[i]/(2*m_vNumEvaluation[i]) << endl;
	}
	fout.close();
}

void TrainGenerator::summarizeEvaluationError( const TrainGenerator& rhs )
{
	for( uint i=0; i<MAX_GAME_LENGTH; i++ ) {
		m_vSquareError[i] += rhs.m_vSquareError[i];
		m_vNumEvaluation[i] += rhs.m_vNumEvaluation[i];
	}
}

void TrainGenerator::summarizeRepeatedSgfData( const TrainGenerator& rhs )
{
	for( map< string,vector<HashKey64> >::const_iterator it=rhs.m_mMoveThresholdHashkeys.begin(); it!=rhs.m_mMoveThresholdHashkeys.end(); ++it ) {
		if( m_mMoveThresholdHashkeys.find(it->first)!=m_mMoveThresholdHashkeys.end() ) { CERR() << "error for same file name " << it->first << endl; }
		m_mMoveThresholdHashkeys[it->first] = it->second;
	}
}

void TrainGenerator::outputRepeatedSgfFile()
{
	vector<bool> vChecked;
	vChecked.resize(m_mMoveThresholdHashkeys.size(),false);

	uint counter1, counter2;
	counter1 = counter2 = 0;
	for( map< string,vector<HashKey64> >::iterator it1=m_mMoveThresholdHashkeys.begin(); it1!=m_mMoveThresholdHashkeys.end(); ++it1,++counter1 ) {
		bool bIsFirst = true;
		const vector<HashKey64>& vTargetHashKey = it1->second;
		if( vChecked[counter1] ) { continue; }

		counter2 = counter1;
		for( map< string,vector<HashKey64> >::iterator it2=it1; it2!=m_mMoveThresholdHashkeys.end(); ++it2,++counter2 ) {
			if( it2==it1 ) { continue; }

			const vector<HashKey64>& vHashKey = it2->second;
			if( vTargetHashKey.size()!=vHashKey.size() ) { continue; }

			bool bIsSame = true;
			for( uint i=0; i<vTargetHashKey.size(); i++ ) {
				if( vTargetHashKey[i]!=vHashKey[i] ) { bIsSame = false; break; }
			}
			if( !bIsSame ) { continue; }

			vChecked[counter2] = true;
			if( bIsFirst ) {
				bIsFirst = false;
#if defined __GNUC__
				m_shareData.m_fTrainResult << endl << "rm -rf \"" << it2->first << "\"";
#else
				m_shareData.m_fTrainResult << endl << "del \"" << it2->first << "\"";
#endif
			} else {
				m_shareData.m_fTrainResult << " \"" << it2->first << "\"";
			}
		}
	}
}

void TrainGenerator::outputDCNNRotationResult( string sFileName )
{
	fstream fout( (sFileName.substr(0,sFileName.size()-4) + "_result.txt").c_str(),ios::out);
	fout << sFileName  << endl;

	// move number in one game should equal to size of vCandidates
	for(int i=0;i< m_vRotateCandidates.size(); ++i){
		fout << m_vRotateAnswer[i] << " " << endl; // write answer first

		// output candidates of eight direction
		for(int j=0;j<m_vRotateCandidates[i].size();++j){
			fout << m_vRotateCandidates[i][j].getScore() << " ";		
			if( (j+1)%361 ==0){ fout << endl ;}
		}
	}
	m_vRotateCandidates.resize(0) ; // clear the content
	m_vRotateAnswer.resize(0);
}

void TrainGenerator::initialize()
{
	uint seed = Configure::RandomSeed;
	if( Configure::UseTimeSeed ) { seed = static_cast<uint>(time(NULL)); }
	Random::reset(seed);
	vector<string> vParamString = splitToVector(WeichiConfigure::dcnn_net,':');
	m_state->m_dcnnNetCollection.initialize(1,vParamString,m_gpu_device);
	m_state->resetThreadState();	

	FileDirectoryExplorer fExplorer;
	string rawSgfDir,line;
	fstream mask_data;
	switch( m_shareData.m_mode ) {
	case TRAINMODE_COLLECT_RADIUS_PATTERN:
		m_vRadiusPatternCollection.clear();
		m_vRadiusPatternCollection.resize(MAX_RADIUS_SIZE);
		break;
	case TRAINMODE_GENERATE_CNN_ROTATATION_DB:
		m_vRotateAnswer.resize(0);
		m_vRotateCandidates.resize(0);
		break;
	case TRAINMODE_CALCULATE_UCT_MOVE_PREDICTION:
	case TRAINMODE_CALCULATE_PLAYOUT_MOVE_PREDICTION:
		m_movePredictorStatistic.initialize();
		break;
	case TRAINMODE_LOG_NET_RECORD:
		m_state->getCNNNet()->setGpuID(m_gpu_device);
		m_state->getCNNNet()->loadNetWork();
		break;
	case TRAINMODE_MERGE_TO_RAW_SGF:
		CERR() << "input Mask sgf Dir: ";
		cin >> rawSgfDir;
		fExplorer.setRootDirectory(rawSgfDir);

		while (true) {
			m_sgfInformation = fExplorer.getNextSgfInformation(true);
			if (m_sgfInformation.m_sSgfString == "") {
				break;
			}
			if (!m_sgfLoader.parseFromString(m_sgfInformation.m_sSgfString)) {
				CERR() << "open file \"" << m_sgfInformation.m_sSgfString << "\" fail" << endl;
				return;
			}
			string filename = m_sgfInformation.m_sFilename;
			filename = filename.substr(filename.find_last_of("/") + 1, filename.length() - filename.find_last_of("/"));
			m_LookupSgfs.insert(pair<string, string>(filename, m_sgfInformation.m_sSgfString));
		}

		// add on
		mask_data.open("data.mask", ios::in);	
		while (getline(mask_data, line)) {
			istringstream iss(line);
			string sgf, mask_sgf;
			iss >> sgf >> mask_sgf;
			m_maskSgfs.insert(pair<string, string>(sgf, mask_sgf));
			CERR() << sgf << mask_sgf << endl;
		}
		break;
	default:
		break;
	}
}

void TrainGenerator::playOneGame( string sSgfString )
{
	if( !m_sgfLoader.parseFromString(sSgfString) ) {
		CERR() << "open file \"" << sSgfString << "\" fail" << endl;
		return ;
	}

	const vector<WeichiMove>& vPreset = m_sgfLoader.getPreset();
	const vector<WeichiMove>& vMoves = m_sgfLoader.getPlayMove();
	//m_moves += vMoves.size();	// count total moves

	m_numMoves += static_cast<uint>(vMoves.size());
	if( m_shareData.m_mode==TRAINMODE_FIND_SGF_ATTRIBUTE ) { findSgfAttribute(m_sgfLoader); }
	else if (m_shareData.m_mode == TRAINMODE_LOG_NET_RECORD) { logNetRecord(); }
	else if (m_shareData.m_mode == TRAINMODE_COLLECT_OPENING) { collectHumanGameOpening(m_sgfLoader); }
	else if (m_shareData.m_mode == TRAINMODE_WRITE_DEVELOPMENT_COMMAND ) { writeDevelopmentCommand(m_sgfLoader); }	
	else if (m_shareData.m_mode == TRAINMODE_FTL_POLICY) { logFTLPolicy(); }		
	else if (m_shareData.m_mode == TRAINMODE_ADD_REGION) { addRegionFromSplitBoard(); }			
	else if (m_shareData.m_mode == TRAINMODE_MERGE_TO_RAW_SGF) { mergeToRawSgf();  }
	else if (m_shareData.m_mode == TRAINMODE_CHECK_SGF) { checkSgf();  }
	else if (m_shareData.m_mode == TRAINMODE_GENERATE_JSON) { generateJson();  }
	else {
		m_state->resetThreadState();	
		if( vPreset.size()>0 ) {
			for( uint iMoveNumber=0; iMoveNumber<vPreset.size(); iMoveNumber++ ) {
				m_state->m_board.preset(vPreset[iMoveNumber]);
			}
		}
		for( uint iMoveNumber=0; iMoveNumber<vMoves.size(); iMoveNumber++ ) {
			bool bIsSuccess = generateOneMove(iMoveNumber,m_sgfLoader);
			if ( !bIsSuccess ) break;
			m_state->play(vMoves[iMoveNumber]);
		}
	}

	// write pattern collection
	if( m_shareData.m_mode==TRAINMODE_COLLECT_RADIUS_PATTERN && m_iSgfCounter%RADIUS_PATTERN_COLLECTION_INTERVAL==0 ) { writeRadiusPatternCollection(); }
	// write DCNN Candidate DB
	else if( m_shareData.m_mode==TRAINMODE_GENERATE_CNN_ROTATATION_DB ) outputDCNNRotationResult(sSgfString);
}

void TrainGenerator::collectHumanGameOpening(SgfLoader& sgfLoader)
{
	const vector<WeichiMove>& vPreset = m_sgfLoader.getPreset();
	const vector<WeichiMove>& vMoves = m_sgfLoader.getPlayMove();
	if (vPreset.size() > 0) { return; }
	if (vMoves.size() == 0) { return; }

	Color winnerColor = sgfLoader.getWinner();
	if (winnerColor == COLOR_NONE) { return; }
	if (sgfLoader.getSgfTag("RE")[2] == 'T') { return; }
	if (sgfLoader.getSgfTag("RE")[2] != 'R') {
		double dResult = atof(sgfLoader.getSgfTag("RE").substr(sgfLoader.getSgfTag("RE").find("+") + 1).c_str());
		if (winnerColor == COLOR_BLACK && (dResult + sgfLoader.getKomi() - 7.5) < 0) { return; }
		else if (winnerColor == COLOR_WHITE && (dResult - sgfLoader.getKomi() + 7.5)<0) { return; }
	}
	m_state->resetThreadState();
	
	//if(!(m_sgfLoader.getRank(COLOR_BLACK) == "9d" || m_sgfLoader.getRank(COLOR_WHITE) == "9d")) return;
	
	size_t endIndex = (vMoves.size() >= 30) ? 30 : vMoves.size();
	string GTPstring = "";
	
	for (uint moveNumber = 0 ; moveNumber < endIndex; ++moveNumber){
		if (m_state->m_board.isIllegalMove(vMoves[moveNumber], m_state->m_ht)) { break; }
		m_state->play(vMoves[moveNumber]);
		GTPstring += ";" + vMoves[moveNumber].toSgfString(true);
		unsigned long long hashValue = m_state->m_board.getMinimumHash();//str_hash(GTPstring);
		writeLock();
		if(m_shareData.m_uniqueMap.find(hashValue) != m_shareData.m_uniqueMap.end()){
			writeUnlock();
			continue;
		}
		m_shareData.m_uniqueMap[hashValue] = true;
		m_shareData.m_fTrainResult << "(;GM[1]SZ[19]PW[?]PB[?]" << GTPstring << ")" << endl;
		
		writeUnlock();
	}

}

void TrainGenerator::addRegionFromSplitBoard()
{
	const vector<WeichiMove>& vPreset = m_sgfLoader.getPreset();
	const vector<WeichiMove>& vMoves = m_sgfLoader.getPlayMove();

	m_state->resetThreadState();
	for (int moveNumber = 0; moveNumber < vPreset.size(); ++moveNumber) { m_state->m_board.preset(vPreset[moveNumber]); }
	for (int moveNumber = 0; moveNumber < vMoves.size(); ++moveNumber) { m_state->play(vMoves[moveNumber]); }

	vector<WeichiMove> vTriangle = m_sgfLoader.getSgfNode()[0].m_vTriangle;
	Color crucialColor = COLOR_NONE;
	for (int i = 0; i < vTriangle.size(); ++i) {
		int crucialPos = vTriangle[i].getPosition(); 
		crucialColor = m_state->m_board.getGrid(crucialPos).getColor();
		break;
	}

	Color wallColor = AgainstColor(crucialColor);
	WeichiBlock* biggestBlock = NULL;
	int numStoneOfBiggestBlock = 0;
	for (int iBlock = 0; iBlock < m_state->m_board.getBlockList().getCapacity() ; ++iBlock) {
		if (!m_state->m_board.getBlockList().isValidIdx(iBlock)) { continue; }
		WeichiBlock* block = m_state->m_board.getBlockList().getAt(iBlock);
		if (!block->isUsed()) { continue; }
		if (block->getColor() != wallColor) { continue; }
		if (block->getNumStone() > numStoneOfBiggestBlock) {
			numStoneOfBiggestBlock = block->getNumStone();
			biggestBlock = block;
		}
	}

	WeichiBitBoard bmCandidate = StaticBoard::getMaskBorder() - biggestBlock->getStoneMap();	
	WeichiBitBoard bmBlockNbr = biggestBlock->getNbrMap();
	WeichiBitBoard bmSearchRoom;
	int pos = 0;
	while ((pos = bmBlockNbr.bitScanForward()) != -1) {
		WeichiBitBoard bmResult;
		bmCandidate.floodfill(pos,bmResult);
		bool bFindRoom = true;
		for (int i = 0; i < vTriangle.size(); ++i) {
			if (!bmResult.BitIsOn(vTriangle[i].getPosition())) {
				bFindRoom = false;
				break;
			}
		}

		bmCandidate -= bmResult;
		bmBlockNbr -= bmResult;

		if (bFindRoom) { 
			bmSearchRoom = bmResult;
		}
	}	

	if (bmSearchRoom.empty()) {
		CERR() << "[WARNING] no room contains all the crucial stones." << endl;
		exit(-1);
	}
	
	string sMarks = "MA";
	while ((pos = bmSearchRoom.bitScanForward()) != -1) {
		sMarks += ("[" + WeichiMove(COLOR_NONE, pos).toSgfString(false) + "]");
	}

	string filename = m_sgfInformation.m_sFilename;
	filename = filename.substr(filename.find_last_of("/")+1, filename.length()- filename.find_last_of("/"));
	fstream fs;
	fs.open(filename.c_str(), ios::out);
	string sgfString = m_sgfInformation.m_sSgfString;
	string result = sgfString.substr(0, sgfString.find_last_of(")")) ;
	result += sMarks;
	result += ")";
	fs << result;

	return;
}

void TrainGenerator::mergeToRawSgf()
{
	// Later dir if raw sgf
	string filename = m_sgfInformation.m_sFilename;
	filename = filename.substr(filename.find_last_of("/") + 1, filename.length() - filename.find_last_of("/"));

	std::map<string, string>::iterator it = m_maskSgfs.find(filename);
	if (it == m_maskSgfs.end()) { return; }

	// get mask file
	std::map<string, string>::iterator it2 = m_LookupSgfs.find(it->second);
	if (it2 == m_LookupSgfs.end()) { return; }
	SgfLoader sgfLoader;
	sgfLoader.parseFromString(it2->second);
	WeichiBitBoard bmBlack, bmWhite;
	vector<WeichiMove> vPreset = sgfLoader.getSgfNode()[0].m_preset;
	vector<WeichiMove> vTriangle = sgfLoader.getSgfNode()[0].m_vTriangle;
	for (int iPreset = 0; iPreset < vPreset.size(); ++iPreset) {
		if (vPreset[iPreset].getColor() == COLOR_BLACK) { bmBlack.SetBitOn(vPreset[iPreset].getPosition()); }
		else if (vPreset[iPreset].getColor() == COLOR_WHITE) { bmWhite.SetBitOn(vPreset[iPreset].getPosition()); }
	}

	// masked sgf
	WeichiBitBoard bmMaskedBlack, bmMaskedWhite;
	WeichiBitBoard bmMaskedTriangle;
	vPreset = m_sgfLoader.getSgfNode()[0].m_preset;
	vTriangle = m_sgfLoader.getSgfNode()[0].m_vTriangle;
	for (int iPreset = 0; iPreset < vPreset.size(); ++iPreset) {
		if (vPreset[iPreset].getColor() == COLOR_BLACK) { bmMaskedBlack.SetBitOn(vPreset[iPreset].getPosition()); }
		else if (vPreset[iPreset].getColor() == COLOR_WHITE) { bmMaskedWhite.SetBitOn(vPreset[iPreset].getPosition()); }
	}
	for (int iTriangle = 0; iTriangle < vTriangle.size(); ++iTriangle) { bmMaskedTriangle.SetBitOn(vTriangle[iTriangle].getPosition()); }

	WeichiBitBoard bmMergeBlackMask = bmMaskedBlack | bmBlack;
	WeichiBitBoard bmMergeWhiteMask = bmMaskedWhite | bmWhite;
	ostringstream oss;
	oss << "(;FF[4]CA[UTF-8]SZ[19]KM[7.5]";

	if (!bmMergeBlackMask.empty()) { oss << "AB"; }
	int pos = 0;
	while ((pos = bmMergeBlackMask.bitScanForward()) != -1) {
		oss << "[" << WeichiMove(COLOR_NONE, pos).toSgfString(false) << "]";
	}
	if (!bmMergeWhiteMask.empty()) { oss << "AW"; }
	pos = 0;
	while ((pos = bmMergeWhiteMask.bitScanForward()) != -1) {
		oss << "[" << WeichiMove(COLOR_NONE, pos).toSgfString(false) << "]";
	}

	string sPL = m_sgfLoader.getSgfTag("PL");
	oss << "PL[" << sPL << "]";

	pos = 0;
	if (!bmMaskedTriangle.empty()) { oss << "TR"; }
	while ((pos = bmMaskedTriangle.bitScanForward()) != -1) {
		oss << "[" << WeichiMove(COLOR_NONE, pos).toSgfString(false) << "]";
	}

	oss << ")";

	fstream fs;
	fs.open(filename, ios::out);
	fs << oss.str();
	fs.close();

	return;
}

void TrainGenerator::checkSgf()
{
	string filename = m_sgfInformation.m_sFilename;	

	const vector<WeichiMove>& vPreset = m_sgfLoader.getPreset();
	const vector<WeichiMove>& vMoves = m_sgfLoader.getPlayMove();

	m_state->resetThreadState();
	for (int moveNumber = 0; moveNumber < vPreset.size(); ++moveNumber) { m_state->m_board.preset(vPreset[moveNumber]); }	
	for (int moveNumber = 0; moveNumber < vMoves.size(); ++moveNumber) { m_state->play(vMoves[moveNumber]); }

	if (vMoves.size() != 0) CERR() << "Error, has moves in sgf." << endl;

	vector<WeichiMove> vTriangle = m_sgfLoader.getSgfNode()[0].m_vTriangle;

	WeichiBitBoard bmCrucial;
	for (int i = 0; i < vTriangle.size(); ++i) {
		const WeichiGrid& grid = m_state->m_board.getGrid(vTriangle[i].getPosition());
		if (grid.isEmpty()) { CERR() << "Error, crucial stone is empty." << endl; }

		const WeichiBlock* block = grid.getBlock();
		if (bmCrucial.hasIntersection(block->getStoneMap())) { CERR() << "Error, crucial stone overlapped." << endl; }

		bmCrucial |= block->getStoneMap();		
	}

	return;
}

void TrainGenerator::generateJson()
{
	string filename = m_sgfInformation.m_sFilename;
	string sgfString = m_sgfInformation.m_sSgfString;
	filename = filename.substr(filename.find_last_of("/") + 1, filename.length() - filename.find_last_of("/"));
	string filenametag = filename.substr(0, filename.find_last_of(".") );
	CERR() << "[" << filenametag << "]" << endl;

	const vector<WeichiMove>& vPreset = m_sgfLoader.getPreset();
	const vector<WeichiMove>& vMoves = m_sgfLoader.getPlayMove();

	m_state->resetThreadState();
	for (int moveNumber = 0; moveNumber < vPreset.size(); ++moveNumber) { m_state->m_board.preset(vPreset[moveNumber]); }
	for (int moveNumber = 0; moveNumber < vMoves.size(); ++moveNumber) { m_state->play(vMoves[moveNumber]); }

	if (vMoves.size() != 0) CERR() << "Error, has moves in sgf." << endl;

	Color winColor = COLOR_NONE;
	WeichiBitBoard bmCrucialStone;
	vector<WeichiMove> vTriangle = m_sgfLoader.getSgfNode()[0].m_vTriangle;
	vector<WeichiMove> vMarks = m_sgfLoader.getSgfNode()[0].m_vMark;
	string sCrucialStone = "";
	
	bool bFirst = true;
	for (int iTriangle = 0; iTriangle < vTriangle.size(); ++iTriangle) { 
		const WeichiGrid& grid = m_state->m_board.getGrid(vTriangle[iTriangle].getPosition());
		if (grid.getColor() == COLOR_NONE) { CERR() << "Error, no stone at crucial position"; exit(0); }
		if (winColor == COLOR_NONE) { winColor = grid.getColor(); }
		else {
			Color oppColor = AgainstColor(winColor);
			if (grid.getColor() == oppColor) { CERR() << "Error, crucial stone color not consistent"; exit(0); }
		}		
		bmCrucialStone |= grid.getBlock()->getStoneMap();
		if (!bFirst) { sCrucialStone += ","; }
		else { bFirst = false; }
		sCrucialStone += WeichiMove(grid.getPosition()).toSgfString(false);
	}

	string sBlackCruciaStone="", sWhiteCruciaStone="";
	string sBlackGoal = "", sWhiteGoal = "";
	string sBlackKoRule = "", sWhiteKoRule = "";
	string sRegion = "";
	bFirst = true;
	for (int iMark = 0; iMark < vMarks.size(); ++iMark) {
		if (!bFirst) { sRegion += ","; }
		else { bFirst = false; }
		sRegion += WeichiMove(vMarks[iMark]).toSgfString(false);
	}

	if (winColor == COLOR_BLACK) {
		sBlackCruciaStone = sCrucialStone; 
		sBlackGoal = "TOLIVE";
		sWhiteGoal = "TOKILL";
		sBlackKoRule = "allow_ko";
		sWhiteKoRule = "disallow_ko";
	} else if (winColor == COLOR_WHITE) { 
		sWhiteCruciaStone = sCrucialStone; 
		sBlackGoal = "TOKILL";
		sWhiteGoal = "TOLIVE";
		sBlackKoRule = "disallow_ko";
		sWhiteKoRule = "allow_ko";
	}

	string sPL = m_sgfLoader.getSgfTag("PL");
	sPL = (sPL == "B" ? "b" : "w");
	string sWinColor = winColor == COLOR_BLACK ? "b" : "w";
	ostringstream oss;
	oss << "{" << endl;
	oss << "\t\"filename\": \"" << filename << "\"," << endl;
	oss << "\t\"category\": \"" << "TOKILL" << "\"," << endl;
	oss << "\t\"winning_color\": \"" << sWinColor << "\"," << endl;
	oss << "\t\"turn_color\": \"" << sPL << "\"," << endl;
	oss << "\t\"black_crucial_stone\": \"" << sBlackCruciaStone << "\"," << endl;
	oss << "\t\"white_crucial_stone\": \"" << sWhiteCruciaStone << "\"," << endl;
	oss << "\t\"black_search_goal\": \"" << sBlackGoal << "\"," << endl;
	oss << "\t\"white_search_goal\": \"" << sWhiteGoal << "\"," << endl;
	oss << "\t\"black_ko_rule\": \"" << sBlackKoRule << "\"," << endl;
	oss << "\t\"white_ko_rule\": \"" << sWhiteKoRule << "\"," << endl;
	oss << "\t\"masked_sgf_str\": \"" << sgfString << "\"," << endl;
	oss << "\t\"region\": \"" << sRegion << "\"" << endl;
	oss << "}" << endl;

	//CERR() << oss.str() << endl;
	fstream fs;
	fs.open(filenametag+".json", ios::out);
	fs << oss.str();
	fs.close();

	return;
}

/*
void TrainGenerator::findHighCriticalitySgf( SgfLoader& sgfLoader)
{
	const vector<WeichiMove>& vPreset = m_sgfLoader.getPreset();
	const vector<WeichiMove>& vMoves = m_sgfLoader.getPlayMove();
	if (vPreset.size() > 0) { return; }
	if (vMoves.size() == 0) { return; }

	Color winnerColor = sgfLoader.getWinner();
	if (winnerColor == COLOR_NONE) { return; }
	if (sgfLoader.getResultInfo()[2] == 'T') { return; }
	if (sgfLoader.getResultInfo()[2] != 'R') {
		double dResult = atof(sgfLoader.getResultInfo().substr(sgfLoader.getResultInfo().find("+") + 1).c_str());
		if (winnerColor == COLOR_BLACK && (dResult + sgfLoader.getKomi() - 7.5) < 0) { return; }
		else if (winnerColor == COLOR_WHITE && (dResult - sgfLoader.getKomi() + 7.5)<0) { return; }
	}
	m_state->resetThreadState();

	int simulationTime = 100; //m_shareData.m_numSimulation;

	vector<double> vCriticality;
	double maxCriticality = -999;
	for (uint iMoveNumber = 0; iMoveNumber < vMoves.size(); iMoveNumber++) {
		if (m_state->m_board.isIllegalMove(vMoves[iMoveNumber], m_state->m_ht)) { return; }
		m_state->play(vMoves[iMoveNumber]);
		Territory territory;
		m_state->startPreSimulation(simulationTime, territory);
		double fAccumulateCriticality = 0;
		for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
			double cri = territory.getCriticality(*it);
			fAccumulateCriticality += cri;
			//if(cri >= WeichiConfigure::CriticalityThreshold) { fAccumulateCriticality += cri; }
		}
		maxCriticality = (fAccumulateCriticality >= maxCriticality) ? fAccumulateCriticality : maxCriticality;
		vCriticality.push_back(fAccumulateCriticality);	
	}
	ostringstream oss;
	oss << m_sgfInformation.m_sFilename << "\t" << maxCriticality << "\t";
	for(int i=0;i<vCriticality.size();++i) { oss << vCriticality[i] << "\t";}
	writeLock();
	m_shareData.m_fTrainResult << oss.str() << endl;
	writeUnlock();

}
*/
/*
void TrainGenerator::generateCGIMSECommand(SgfLoader& sgfLoader)
{
	const vector<WeichiMove>& vPreset = m_sgfLoader.getPreset();
	const vector<WeichiMove>& vMoves = m_sgfLoader.getPlayMove();
	if (vPreset.size() > 0) { return; }
	if (vMoves.size() == 0) { return; }

	Color winnerColor = sgfLoader.getWinner();
	if (winnerColor == COLOR_NONE) { return; }
	if (sgfLoader.getResultInfo()[2] == 'T') { return; }
	if (sgfLoader.getResultInfo()[2] != 'R') {
		double dResult = atof(sgfLoader.getResultInfo().substr(sgfLoader.getResultInfo().find("+") + 1).c_str());
		if (winnerColor == COLOR_BLACK && (dResult + sgfLoader.getKomi() - 7.5) < 0) { return; }
		else if (winnerColor == COLOR_WHITE && (dResult - sgfLoader.getKomi() + 7.5)<0) { return; }
	}
	
	
	m_state->resetThreadState();
	vector<uint> vMoveNumber = randomMoveNumber(sgfLoader);
	ostringstream oss;
	for (uint moveNumber = 0, count = 0; moveNumber < vMoves.size(); ++moveNumber){
		oss << "play "<< toChar(m_state->m_board.getToPlay()) << " " << vMoves[moveNumber].toGtpString() << endl;
		m_state->play(vMoves[moveNumber]);
		if( moveNumber!=vMoveNumber[count] ) { continue; }
		oss << "development_command" << endl; 
		count++;
		if( count>vMoveNumber.size() ) { break; }
	}
	oss << "clear_board" << endl;
	writeLock();
	m_shareData.m_fTrainResult << oss.str() ;
	writeUnlock();




}
*/
bool TrainGenerator::generateOneMove( uint moveNumber, SgfLoader& sgfLoader )
{
	const vector<WeichiMove>& vMoves = sgfLoader.getPlayMove();

	switch( m_shareData.m_mode ) {
	case TRAINMODE_COLLECT_RADIUS_PATTERN:
		collectRadiusPattern(vMoves[moveNumber]); break;
	case TRAINMODE_GENERATE_UCT_FEATURES:
		collectUCTFeatures(vMoves[moveNumber]); break;
	case TRAINMODE_GENERATE_PLAYOUT_FEATURES:
		collectPlayoutFeatures(vMoves[moveNumber]); break;
	case TRAINMODE_GENERATE_CNN_ROTATATION_DB:
		collectDCNNRotateCandidate(vMoves,moveNumber); break;
	case TRAINMODE_CALCULATE_UCT_MOVE_PREDICTION:
		addUpUctMovePredictorStatistic(vMoves[moveNumber],moveNumber); break;
	case TRAINMODE_CALCULATE_PLAYOUT_MOVE_PREDICTION:
		addUpPlayoutMovePredictorStatistic(vMoves[moveNumber],moveNumber); break;
	case TRAINMODE_FIND_REPETITION_SGF:
		if( !collectRepeatedSgf() ) { return false; }
		break;
	default:
		return false;
	}

	return true;
}

void TrainGenerator::setLifeDeathConfig(string config) 
{	
	m_testSubject = "";
	m_testPos = -1;

	istringstream iss(config);
	string token ;
	std::getline(iss, token, ';');
	string goal = token ;
	// 1. Search Goal
	if (goal == "TOKILL") { WeichiConfigure::search_goal = 1 ; }
	else if (goal == "TOLIVE") { WeichiConfigure::search_goal = 2 ; }
	else if (goal == "TERRITORY") { WeichiConfigure::search_goal = 3 ; }
	 
	// 2. Win Color
	std::getline(iss, token, ';');
	string toPlay = token ;	
	if (toPlay == "BLACK") { WeichiGlobalInfo::getTreeInfo().m_winColor = COLOR_BLACK; }
	else if (toPlay == "WHITE") { WeichiGlobalInfo::getTreeInfo().m_winColor = COLOR_WHITE; }

	// 3. test subject
	std::getline(iss, token, ';');
		
	if (token.find(",") == string::npos) {
		m_testSubject = token;	
		return;
	}

	// 4. test if there's additional target
	string nextToken;
	istringstream iss2(token);
	std::getline(iss2, nextToken, ',');
	m_testSubject = nextToken;	

	std::getline(iss2, nextToken, ',');
	m_testPos = WeichiMove(COLOR_NONE, nextToken).getPosition();

	return;
}

void TrainGenerator::collectRadiusPattern( const WeichiMove& winMove )
{
	Color turnColor = winMove.getColor();
	if( m_sgfLoader.getWinner()!=winMove.getColor() ) { return; }

	const Vector<WeichiMoveFeature,MAX_NUM_GRIDS>& vCandidateList = m_state->m_moveFeatureHandler.calculateFullBoardCandidates(turnColor);
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		const WeichiMoveFeature& moveFeature = vCandidateList[*it];
		if( moveFeature.hasFixedFeature(MF_NOT_IN_CANDIDATE) ) { continue; }

		Vector<uint,MAX_RADIUS_SIZE> vIndex;
		vIndex.resize(MAX_RADIUS_SIZE);

		const WeichiMove move(turnColor,*it);
		const WeichiGrid& grid = m_state->m_board.getGrid(*it);
		for( int iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
			vIndex[iRadius] = grid.getRadiusPatternRealIndex(iRadius,move.getColor());
			if( iRadius==3 ) { vIndex[iRadius] = (vIndex[iRadius]<<(RADIUS_NBR_LIB_SIZE*RADIUS_NBR_LIB_BITS)) + grid.getNbrLibIndex(); }
		}

		Vector<HashKey64,MAX_RADIUS_SIZE> vKeys = WeichiRadiusPatternTable::calculateMinRadiusPatternHashKey(vIndex);
		for( uint iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
			if( m_vRadiusPatternCollection[iRadius].find(vKeys[iRadius])!=m_vRadiusPatternCollection[iRadius].end() ) {
				if( move==winMove ) { ++m_vRadiusPatternCollection[iRadius][vKeys[iRadius]].win; }
				++m_vRadiusPatternCollection[iRadius][vKeys[iRadius]].total;
			} else {
				// first time finding this pattern, need to save index
				m_vRadiusPatternCollection[iRadius][vKeys[iRadius]].total = 1;
				if( move==winMove ) { m_vRadiusPatternCollection[iRadius][vKeys[iRadius]].win = 1; }
				else { m_vRadiusPatternCollection[iRadius][vKeys[iRadius]].win = 0; }

				Vector<uint,MAX_RADIUS_SIZE>& vEntryIndex = m_vRadiusPatternCollection[iRadius][vKeys[iRadius]].vIndex;
				for( uint i=MIN_RADIUS_SIZE; i<=iRadius; i++ ) {
					vEntryIndex[i] = vIndex[i];
				}
			}
		}
	}
}

void TrainGenerator::collectUCTFeatures( const WeichiMove& winMove )
{
	Color turnColor = winMove.getColor();
	if( m_sgfLoader.getWinner()!=winMove.getColor() ) { return; }

	const Vector<WeichiMoveFeature,MAX_NUM_GRIDS>& vCandidateList = m_state->m_moveFeatureHandler.calculateFullBoardCandidates(turnColor);

	WeichiMoveFeature passMoveFeature = vCandidateList[PASS_MOVE.getPosition()];

	// get win move feature
	WeichiMoveFeature winMoveFeature = winMove.isPass() ? passMoveFeature : vCandidateList[winMove.getPosition()];
	vector<uint> vFeatureIndex = translateUCTFeatureToFeatureTeam(winMoveFeature,true);

	// write win/lose team
	ostringstream oss;
	if( vFeatureIndex.size()>0 ) {
		writeFeatureTeam(oss,vFeatureIndex,winMove.getPosition(),true);
		// write lose team
		for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
			WeichiMove move(winMove.getColor(),*it);
			if( move==winMove ) { continue; }
			if( vCandidateList[*it].hasFixedFeature(MF_NOT_IN_CANDIDATE) ) { continue; }

			writeFeatureTeam(oss,translateUCTFeatureToFeatureTeam(vCandidateList[*it],false),*it,false);
		}
		// write pass (if win move is not pass)
		if( !winMove.isPass() ) { writeFeatureTeam(oss,translateUCTFeatureToFeatureTeam(passMoveFeature,false),PASS_MOVE.getPosition(),false); }
	} else {
		CERR() << "found no feature this move!!!" << endl;
		CERR() << "At file " << m_sgfInformation.m_sSgfString << " " << endl;
		m_state->m_board.showColorBoard();
		CERR() << "Move is " << winMove.toGtpString() << ", move number = " << m_state->m_board.getMoveList().size()+1 << endl;
		CERR() << "============================================" << endl;
	}

	writeLock();
	m_shareData.m_fTrainResult << oss.str();
	writeUnlock();
}

void TrainGenerator::collectDCNNRotateCandidate( const vector<WeichiMove>& vMoves, uint moveNumber)
{
	/* 
		generate the set of candidates of 8 direction vector
		vCandidate: format: SYM1(361) + SYM2(361) ...  SYM8(361)
		the size of vector is 361* 8 = 2888
		
		P.S: after generate rotation DB, the result will be saved in the directory where we put .sgf.
		Note that some result may be mis-processed and generate the file like xxxx_result_result.txt, delete it.
		The result file can be processed by analysis tool by ./analysis [dirName].
		the dirName should be saved in the directory under "candidateDB" in the directory of analyzed tool.

	*/
	/*m_vRotateAnswer.push_back(vMoves[moveNumber].getCompactPosition());
	vector<CandidateEntry> vCandidate;

	m_state->m_rootFilter.startFilter();
	WeichiCNNSLNet& slNet = m_state->getCNNHandler().getSLHandler().getNet();
	WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(m_state,&slNet);	
	
	int batchSize = slNet.getBatchSize();
	for( int i=0; i<SYMMERTY_SIZE; i+=batchSize ) {
		// how many batch has been pushed to GPU in this round
		for(int symmetry=i; symmetry<i+batchSize && symmetry < SYMMERTY_SIZE;  ++symmetry ) {
			slNet.push_back(input,m_state->m_board,m_state->m_ht,static_cast<SymmetryType>(symmetry));
		}
		slNet.forward();
		for(int symmetry=i; symmetry<i+batchSize && symmetry < SYMMERTY_SIZE;  ++symmetry ) {
			for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
				if( !slNet.isLegal((symmetry-i),*it) ) { 
					vCandidate.push_back(CandidateEntry(*it,-1));
				}else{
					double dProb = slNet.getCNNresult((symmetry-i),*it);
					vCandidate.push_back(CandidateEntry(*it,dProb));
				}
			}
		}
	}
	m_vRotateCandidates.push_back(vCandidate);*/
}

void TrainGenerator::collectPlayoutFeatures( const WeichiMove& winMove )
{
	if( winMove.isPass() ) { return; }

	Color turnColor = winMove.getColor();
	WeichiMove lastMove = m_state->m_board.hasPrevMove() ? m_state->m_board.getPrevMove() : PASS_MOVE;

	WeichiPlayoutAgent agent(*m_state);
	agent.calcualteLocalPlayoutFeature(lastMove);
	agent.m_bmSkipMove.Reset();
	
	ostringstream oss;
	vector<uint> vIndex;
	WeichiProbabilityPlayoutHandler& probabilityHandler = m_state->m_board.m_probabilityHandler;
	const Vector<ProbabilityGrid,MAX_NUM_GRIDS>& vProbGrid = probabilityHandler.getProbGrid(turnColor);

	// write win team
	Vector<uint,MAX_NUM_PLAYOUT_FEATURE_GROUPS> vFeatureIndex = vProbGrid[winMove.getPosition()].getFeatureIndexs();
	for( uint i=0; i<vFeatureIndex.size(); i++ ) { vIndex.push_back(vFeatureIndex[i]); }
	writeFeatureTeam(oss,vIndex,winMove.getPosition(),true);

	WeichiPlayoutPolicy policy = POLICY_UNKNOWN;
	bool bIsWin = vProbGrid[winMove.getPosition()].getPlayoutFeatureBits().BitIsOn(policy);
	bool bIsLose = false;

	// write lose team
	CandidateList& candidates = m_state->m_board.m_candidates;
	for( uint i=0; i<candidates.size(); i++ ) {
		if( candidates[i]==winMove.getPosition() ) { continue; }

		WeichiMove move(m_state->m_board.getToPlay(), candidates[i]);
		if( m_state->m_board.isIllegalMove(move,m_state->m_ht) ) { continue; }

		vIndex.clear();
		Vector<uint,MAX_NUM_PLAYOUT_FEATURE_GROUPS> vFeatureIndex = vProbGrid[move.getPosition()].getFeatureIndexs();
		for( uint i=0; i<vFeatureIndex.size(); i++ ) { vIndex.push_back(vFeatureIndex[i]); }
		writeFeatureTeam(oss,vIndex,winMove.getPosition(),false);

		bool bLose = vProbGrid[move.getPosition()].getPlayoutFeatureBits().BitIsOn(policy);
		bIsLose = bIsLose | bLose;
		if( bLose ) { cerr << "lose move: " << move.toGtpString() << " " << toChar(move.getColor()) << "      " << endl; }
	}

	static int win = 0;
	static int lose = 0;
	if( bIsWin ) { cerr << "win! " << endl; win++; }
	else if( bIsLose ) { cerr << "lose!" << endl; lose++; }
	
	if( bIsWin || bIsLose ) {
		cerr << "win move: " << winMove.toGtpString() << " " << toChar(winMove.getColor()) << endl;
		cerr << "previous move:" << m_state->m_board.getPrevMove().toGtpString() << " " << toChar(m_state->m_board.getPrevMove().getColor()) << endl;
		cerr << "win/lose: " << win << "/" << lose << ", " << (double)win/(win+lose) << endl;
		m_state->m_board.showColorBoard();
		int k;
		cin >> k;
	}

	probabilityHandler.recover(turnColor);

	writeLock();
	m_shareData.m_fTrainResult << oss.str();
	writeUnlock();
}

vector<uint> TrainGenerator::translateUCTFeatureToFeatureTeam( WeichiMoveFeature moveFeature, bool bIsWinMove )
{
	WeichiMoveFeatureType type;
	vector<uint> vFeatureIndex;

	if( moveFeature.hasFixedFeature(MF_NOT_IN_CANDIDATE) ) { return vFeatureIndex; }

	// write fixed feature
	while( (type=moveFeature.scanForwardFixedFeature())!=-1 ) {
		vFeatureIndex.push_back(type);
	}

	// for radius pattern
	for( uint iIndex=0; iIndex<moveFeature.getRadiusPatternFeatureSize(); iIndex++ ) {
		vFeatureIndex.push_back(moveFeature.getRadiusPatternFeature(iIndex));
	}

	return vFeatureIndex;
}

void TrainGenerator::writeFeatureTeam( ostringstream& oss, vector<uint> vFeatureIndex, uint pos, bool bIsWinMove)
{
	if( bIsWinMove ) { oss << "#\n"; }
	oss << vFeatureIndex[0];
	for( uint i=1; i<vFeatureIndex.size(); i++ ) {
		oss << ' ' << vFeatureIndex[i];
	}
	oss << '\n';
}

void TrainGenerator::addUpUctMovePredictorStatistic( const WeichiMove& winMove, uint moveNumber )
{
	Color turnColor = winMove.getColor();

	m_state->m_moveFeatureHandler.calculateFullBoardCandidates(turnColor);
	const Vector<CandidateEntry,MAX_NUM_GRIDS>& vUctCandidateList = m_state->m_moveFeatureHandler.getUCTCandidateList();

	//uint winRank = MAX_NUM_GRIDS;
	double dWinScore,dTotalScore;
	Vector<double,MAX_NUM_GRIDS> vScore;
	vScore.setAllAs(0,MAX_NUM_GRIDS);

	dTotalScore = 0.0f;
	dWinScore = vScore[PASS_MOVE.getPosition()];
	for( uint i=0; i<vUctCandidateList.size(); i++ ) {
		if( vUctCandidateList[i].getPosition()==winMove.getPosition() ) { dWinScore = vUctCandidateList[i].getScore(); /*winRank = i;*/ }
		vScore[vUctCandidateList[i].getPosition()] = vUctCandidateList[i].getScore();
		dTotalScore += vUctCandidateList[i].getScore();
	}

	m_movePredictorStatistic.addStageCounter(moveNumber);
	m_movePredictorStatistic.addMovePredictionRate(dWinScore,vScore,moveNumber);
	/*for( uint i=0; i<MovePredictorStatistic::CONTINUOUS_LIMIT; i++ ) {
		m_movePredictorStatistic.addContinuousMovePredictionRate(i,moveNumber,winRank<=i,moveNumber==0);
	}
	m_movePredictorStatistic.addProbabilityPrediction(dWinScore,dTotalScore,vScore,moveNumber);
	m_movePredictorStatistic.addWinMoveProbability(dWinScore,dTotalScore,vScore,moveNumber);*/
}

void TrainGenerator::addUpPlayoutMovePredictorStatistic( const WeichiMove& winMove, uint moveNumber )
{
	Color turnColor = winMove.getColor();
	WeichiMove lastMove = m_state->m_board.hasPrevMove() ? m_state->m_board.getPrevMove() : PASS_MOVE;

	WeichiPlayoutAgent agent(*m_state);
	agent.calcualteLocalPlayoutFeature(lastMove);
	agent.m_bmSkipMove.Reset();

	WeichiProbabilityPlayoutHandler& probabilityHandler = m_state->m_board.m_probabilityHandler;

	// skip bad move
	if( agent.checkRandomMove(winMove)!=WeichiPlayoutAgent::FINE ) {
		probabilityHandler.recover(turnColor);
		return ;
	}

	uint winRank = MAX_NUM_GRIDS;
	double dWinScore,dTotalScore;
	Vector<double,MAX_NUM_GRIDS> vScore;
	vScore.setAllAs(0,MAX_NUM_GRIDS);

	dTotalScore = 0.0f;
	dWinScore = vScore[PASS_MOVE.getPosition()];
	const Vector<ProbabilityGrid,MAX_NUM_GRIDS>& vProbGrid = probabilityHandler.getProbGrid(turnColor);
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		const WeichiGrid& grid = m_state->m_board.getGrid(*it);
		if( !grid.isEmpty() ) { continue; }

		if( *it==winMove.getPosition() ) { dWinScore = vProbGrid[*it].getScore(); }
		vScore[*it] = vProbGrid[*it].getScore();
		dTotalScore += vProbGrid[*it].getScore();
	}
	m_movePredictorStatistic.addStageCounter(moveNumber);
	m_movePredictorStatistic.addMovePredictionRate(dWinScore,vScore,moveNumber);
	for( uint i=0; i<MovePredictorStatistic::CONTINUOUS_LIMIT; i++ ) {
		m_movePredictorStatistic.addContinuousMovePredictionRate(i,moveNumber,winRank<=i,moveNumber==0);
	}
	m_movePredictorStatistic.addProbabilityPrediction(dWinScore,dTotalScore,vScore,moveNumber);
	m_movePredictorStatistic.addWinMoveProbability(dWinScore,dTotalScore,vScore,moveNumber);

	probabilityHandler.recover(turnColor);
}

void TrainGenerator::logNetRecord()
{
	const vector<WeichiMove>& vPreset = m_sgfLoader.getPreset();
	const vector<WeichiMove>& vMoves = m_sgfLoader.getPlayMove();
	if (vPreset.size() > 0) { return; }
	if (vMoves.size() == 0) { return; }

	Color winnerColor = m_sgfLoader.getWinner();
	if (winnerColor == COLOR_NONE) { return; }
	if (!isdigit(m_sgfLoader.getSgfTag("RE")[2]) && m_sgfLoader.getSgfTag("RE")[2] != 'R') { return; }

	LogData gamedata;
	gamedata.m_sFilename = m_sgfInformation.m_sFilename;
	gamedata.m_iFileLine = m_sgfInformation.m_iFileLine;

	m_state->resetThreadState();
	vector<uint> vMoveNumber = randomMoveNumber();
	for (uint moveNumber = 0, count = 0; moveNumber < vMoves.size(); ++moveNumber) {
		if (moveNumber != vMoveNumber[count]) {
			m_state->play(vMoves[moveNumber]);
			continue;
		}

		Color turnColor = m_state->m_board.getToPlay();
		gamedata.m_movePosition = vMoves[moveNumber].getPosition();
		gamedata.m_moveNumber = moveNumber;
		gamedata.m_turnColor = turnColor;
		gamedata.m_winnerColor = winnerColor;
		gamedata.m_komi = m_sgfLoader.getKomi();
		gamedata.m_sResultInfo = m_sgfLoader.getSgfTag("RE");

		m_state->m_rootFilter.startFilter();
		WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(*m_state, m_state->getCNNNet());
		m_vLogData.push_back(gamedata);

		if (m_state->getCNNNet()->isFull()) { forwardNet(); }

		count++;
		if (count >= vMoveNumber.size()) { break; }

		m_state->play(vMoves[moveNumber]);
	}
}

void TrainGenerator::logFTLPolicy()
{
	const vector<WeichiMove>& vPreset = m_sgfLoader.getPreset();
	const vector<WeichiMove>& vMoves = m_sgfLoader.getPlayMove();
	
	m_state->resetThreadState();
	if (WeichiConfigure::zero_training_7x7_killall) {
		for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
			m_state->m_board.getLADToKillStones().get(COLOR_BLACK).SetBitOn(*it);
			m_state->m_board.getLADToLifeStones().get(COLOR_WHITE).SetBitOn(*it);
		}
	} else {
		string sEvent = m_sgfLoader.getSgfTag("EV");
		vector<string> vSplit;
		vSplit.push_back("");
		for (auto c : sEvent) {
			if (c == ';') { vSplit.push_back(""); }
			else { vSplit.back().append(1, c); }
		}

		// format: BL ; BK ; WL ; WK ; type
		if (vSplit.size() != 5) { CERR() << "LAD format error!! " << sEvent << endl; }
		setLADBitBoard(vSplit[0], m_state->m_board.getLADToLifeStones().get(COLOR_BLACK));
		setLADBitBoard(vSplit[1], m_state->m_board.getLADToKillStones().get(COLOR_BLACK));
		setLADBitBoard(vSplit[2], m_state->m_board.getLADToLifeStones().get(COLOR_WHITE));
		setLADBitBoard(vSplit[3], m_state->m_board.getLADToKillStones().get(COLOR_WHITE));
		if (vSplit[4] == "UCL" || vSplit[4] == "KbK") {
			if (vSplit[0] == "") {
				// B to kill, W to live
				m_state->m_board.getLADKoAvailable().m_black = true;
				m_state->m_board.getLADKoAvailable().m_white = false;
			} else if (vSplit[1] == "") {
				// B to live, W to kill
				m_state->m_board.getLADKoAvailable().m_black = false;
				m_state->m_board.getLADKoAvailable().m_white = true;
			}
		} else if (vSplit[4] == "UCD" || vSplit[4] == "LbK") {
			if (vSplit[0] == "") {
				// B to kill, W to live
				m_state->m_board.getLADKoAvailable().m_black = false;
				m_state->m_board.getLADKoAvailable().m_white = true;
			} else if (vSplit[1] == "") {
				// B to live, W to kill
				m_state->m_board.getLADKoAvailable().m_black = true;
				m_state->m_board.getLADKoAvailable().m_white = false;
			}
		}
	}
	for (int moveNumber = 0; moveNumber < vPreset.size(); ++moveNumber) { m_state->m_board.preset(vPreset[moveNumber]); }
	for (int moveNumber = 0; moveNumber < vMoves.size(); ++moveNumber) { m_state->play(vMoves[moveNumber]); }
	if (m_sgfLoader.hasSgfTag("PL")) {
		Color turnColor = toColor(m_sgfLoader.getSgfTag("PL")[0]);
		m_state->setRootTurn(turnColor);
		m_state->m_board.setToPlay(turnColor);
	}

	writeLock();
	m_shareData.m_fTrainResult << m_sgfInformation.m_sSgfString << "\t";
	bool bFirst = true;
	const vector<WeichiMove>& currentMarks = m_sgfLoader.getSgfNode()[0].m_vMark;
	if (currentMarks.size() == 0) { CERR() << "Error, no marks at " + m_sgfInformation.m_sSgfString << endl; exit(0); }
	for (int iMark = 0; iMark < currentMarks.size(); ++iMark) {
		if (bFirst) { bFirst = false; }
		else { m_shareData.m_fTrainResult << "|"; }
		m_shareData.m_fTrainResult << currentMarks[iMark].toSgfString();
	}

	float ftlPolicy = 0.0f;
	int ftlRank = MAX_NUM_GRIDS;
	m_state->m_rootFilter.startFilter();
	WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(*m_state, m_state->getCNNNet());
	m_state->getCNNNet()->forward();
	vector<CandidateEntry> cadidates = m_state->getCNNNet()->getSLCandidates(0);
	for (int index = 0; index < cadidates.size(); index++) {
		for (int iMark = 0; iMark < currentMarks.size(); ++iMark) {
			WeichiMove mark = currentMarks[iMark];
			if (cadidates[index].getPosition() == mark.getPosition()) {
				ftlPolicy += cadidates[index].getScore();
				ftlRank = min(ftlRank, index);
			}
		}
	}
	m_shareData.m_fTrainResult << "\t" << ftlPolicy << "\t" << ftlRank << endl;
	writeUnlock();

	return;
}

void TrainGenerator::setLADBitBoard(string sSgfPos, WeichiBitBoard& bmStone)
{
	vector<string> vPoints = splitToVector(sSgfPos, ',');
	for (auto s : vPoints) {
		if (s.empty()) { continue; }
		WeichiMove m(COLOR_BLACK, s);
		bmStone.SetBitOn(m.getPosition());
	}
}

void TrainGenerator::forwardNet()
{
	const int currentBatchSize = m_state->getCNNNet()->getCurrentBatchSize();
	if (currentBatchSize == 0) { return; }

	m_state->getCNNNet()->forward();
	for (int batch = 0; batch < currentBatchSize; batch++) {
		ostringstream oss;
		LogData& gameData = m_vLogData[batch];

		oss << gameData.m_sFilename << "\t" << gameData.m_iFileLine << "\t" << gameData.m_moveNumber << "\t"
			<< gameData.m_komi << "\t" << toChar(gameData.m_turnColor) << "\t" << toChar(gameData.m_winnerColor) << "\t";

		int iCorrectIndex = 0;
		if (m_state->getCNNNet()->hasSLOutput()) {
			vector<CandidateEntry> cadidates = m_state->getCNNNet()->getSLCandidates(batch);
			for (int index = 0; index < cadidates.size(); index++) {
				if (cadidates[index].getPosition() == gameData.m_movePosition) { iCorrectIndex = index; }
			}
		}

		float fValue = 0.0f;
		if (m_state->getCNNNet()->hasVNOutput()) {
			fValue = m_state->getCNNNet()->getVNResult(batch, m_sgfLoader.getKomi());
		}

		oss << iCorrectIndex << "\t" << fValue << "\t" << endl;
		
		writeLock();
		m_shareData.m_fTrainResult << oss.str();
		writeUnlock();
	}

	m_vLogData.clear();
}

vector<uint> TrainGenerator::randomMoveNumber()
{
	vector<uint> vMoveNumber;
	const vector<WeichiMove>& vMoves = m_sgfLoader.getPlayMove();
	size_t max_move = vMoves.size();
	int numErase = static_cast<int>(max_move) - m_shareData.m_numSample;
	if (numErase < 0) { numErase = 0; }
	for (uint i = 0; i < max_move; i++) { vMoveNumber.push_back(i); }
	for (int i = 0; i < numErase; i++) {
		int index = Random::nextInt(static_cast<int>(vMoveNumber.size()));
		vMoveNumber.erase(vMoveNumber.begin() + index);
	}

	return vMoveNumber;
}

void TrainGenerator::writeDevelopmentCommand(SgfLoader& sgfLoader)
{
	const int NUM_SIM = 100;
	const int RECORD_STEP = 15;

	const vector<WeichiMove>& vPreset = m_sgfLoader.getPreset();
	const vector<WeichiMove>& vMoves = m_sgfLoader.getPlayMove();
	if (vPreset.size() > 0) { return; }
	if (vMoves.size() == 0) { return; }

	Color winnerColor = sgfLoader.getWinner();
	if (winnerColor == COLOR_NONE) { return; }
	if (!isdigit(sgfLoader.getSgfTag("RE")[2]) && sgfLoader.getSgfTag("RE")[2] != 'R') { return; }

	ostringstream oss;
	m_state->resetThreadState();
	oss << "clear_board" << endl;
	for (uint moveNumber = 0; moveNumber < vMoves.size(); moveNumber++) {
		if (moveNumber > 0 && moveNumber%RECORD_STEP == 0) {
			oss << "write_file " << m_sgfInformation.m_sFilename
				<< " " << moveNumber
				<< " " << toChar(m_state->m_board.getToPlay())
				<< " " << toChar(winnerColor) << endl
				<< "development_command" << endl;
		}

		m_state->play(vMoves[moveNumber]);
		oss << "play " << toChar(vMoves[moveNumber].getColor()) << " " << vMoves[moveNumber].toGtpString() << endl;
	}

	writeLock();
	m_shareData.m_fTrainResult << oss.str();
	writeUnlock();
}

void TrainGenerator::findSgfAttribute( SgfLoader& sgfLoader )
{
	bool bIsHandicap = isHandicapSgf(sgfLoader);
	bool bIsLowRank = isLowRankSgf(sgfLoader);
	bool bIsCorrectSgf = true;
	bool bIsBadSgf = false;

	const vector<WeichiMove>& vPreset = m_sgfLoader.getPreset();
	const vector<WeichiMove>& vMoves = m_sgfLoader.getPlayMove();
	m_state->resetThreadState();
	if( vPreset.size()>0 ) {
		for( uint iMoveNumber=0; iMoveNumber<vPreset.size(); iMoveNumber++ ) {
			m_state->m_board.preset(vPreset[iMoveNumber]);
		}
	}

	bool bGoodLengthSgf = ( (vMoves.size() >= 30) && (vMoves.size() < 400) ) ? true : false;

	for( uint iMoveNumber=0; iMoveNumber<vMoves.size(); iMoveNumber++ ) {
		bIsCorrectSgf = !m_state->m_board.isIllegalMove(vMoves[iMoveNumber],m_state->m_ht);
		if( !bIsCorrectSgf ) { break; }
		if( !bIsBadSgf ) { bIsBadSgf = (WeichiBadMoveReader::getBadMoveType(m_state->m_board,vMoves[iMoveNumber])!=BADMOVE_NOT_BADMOVE); }

		m_state->play(vMoves[iMoveNumber]);
	}

	writeLock();
	if( bIsHandicap ) { m_shareData.m_fTrainResult << "handicap: " << m_sgfInformation.getFullyName() << endl; }
	if( bIsLowRank ) { m_shareData.m_fTrainResult << "low_rank: " << m_sgfInformation.getFullyName() << endl; }
	if( !bIsCorrectSgf ) { m_shareData.m_fTrainResult << "incorrect_sgf: " << m_sgfInformation.getFullyName() << endl; }
	if( bIsBadSgf ) { m_shareData.m_fTrainResult << "bad_sgf: " << m_sgfInformation.getFullyName() << endl; }
	if (! bGoodLengthSgf)  { m_shareData.m_fTrainResult << "bad_length: " << m_sgfInformation.getFullyName() << " " << vMoves.size() <<  endl; }
	writeUnlock();
}

bool TrainGenerator::isLowRankSgf( SgfLoader& sgfLoader )
{
	bool bBLowRnak = false;
	bool bWLowRank = false;
	string sBlackRank = sgfLoader.getPlayerRank(COLOR_BLACK);
	string sWhiteRank = sgfLoader.getPlayerRank(COLOR_WHITE);

	if( sBlackRank[1]=='k' || (sBlackRank[1]=='d' && sBlackRank[0]<='6') ) { bBLowRnak = true; }
	if( sWhiteRank[1]=='k' || (sWhiteRank[1]=='d' && sWhiteRank[0]<='6') ) { bWLowRank = true; }
	
	return (bBLowRnak || bWLowRank);	
}

bool TrainGenerator::isHandicapSgf( SgfLoader& sgfLoader )
{
	return (sgfLoader.getHandicap()>0 || sgfLoader.getPreset().size()>0);
}

bool TrainGenerator::collectRepeatedSgf()
{
	//cout << m_state->m_board.getMoveList().size() > REPETITION_POSITION_THRESHOLD << std::endl;

	if( m_state->m_board.getMoveList().size()>REPETITION_POSITION_THRESHOLD ) { return false; }

	HashKey64 hash = m_state->m_board.getMinimumHash() ; // minimum hash
	if( m_state->m_board.getMoveList().size()==0 ) {
		vector<HashKey64> vec;
		vec.push_back(hash);
		m_mMoveThresholdHashkeys.insert(pair< string,vector<HashKey64> >(m_sgfInformation.m_sSgfString,vec));
	} else {
		map< string, vector<HashKey64> >::iterator it = m_mMoveThresholdHashkeys.find(m_sgfInformation.m_sSgfString);
		(*it).second.push_back(hash); // add new hash
	}
	return true;
} 
