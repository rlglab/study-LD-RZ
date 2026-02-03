#include "GenSgf.h"
#include "ColorMessage.h"

// GenSgfSession
void GenSgfSession::handle_read( std::string sResult )
{
	if( sResult.back()=='\n' ) { sResult.pop_back(); }

	string sReturnResult = "";
	if( sResult.find("get_command")!=string::npos ) {
		sReturnResult = WeichiConfigure::gensgf_command;
	} else if( sResult.find("get_opening")!=string::npos ) {
		SgfInformation sgfInfo = m_sharedData.m_explorer.getNextSgfInformation();
		if( sgfInfo.m_sSgfString=="" ) { sReturnResult = "QUIT"; }
		else { sReturnResult = sgfInfo.m_sFilename + ":" + ToString(sgfInfo.m_iFileLine) + " " + sgfInfo.m_sSgfString; }
	} else if( sResult.find("sgf_result")!=string::npos ) {
		saveSgf(sResult.substr(sResult.find(" ")+1),m_sharedData.m_foutSgf);	// remove header
		sReturnResult = "OK";
	} else if( sResult.find("abort_sgf")!=string::npos ) {
		saveSgf(sResult.substr(sResult.find(" ")+1),m_sharedData.m_abortSgf,"_abort");	// remove header
		sReturnResult = "OK";
	} else if( sResult.find("name")!=string::npos ) {
		m_sName = make_pair(getIP(),sResult.substr(sResult.find(" ")+1));	// remove header
		m_sharedData.m_lock.lock();
		m_sharedData.m_machineMap[m_sName] = "Connected Time:" + TimeSystem::getTimeString("Y/m/d_H:i:s");
		if( m_sharedData.m_machineProgress.find(m_sName)==m_sharedData.m_machineProgress.end() ) {
			m_sharedData.m_machineProgress[m_sName] = 0;
		}
		displayProgress();
		m_sharedData.m_lock.unlock();
		sReturnResult = "OK";
	} else {
		sReturnResult = "QUIT";
		CERR() << getColorMessage("[Error] Unknown command.",ANSITYPE_BOLD,ANSICOLOR_RED,ANSICOLOR_BLACK) << endl;
	}
	
	write(sReturnResult);
	if( m_sharedData.m_total_progress%1000==0 ) { displayProgress(); }
}

void GenSgfSession::handle_disconnect()
{
	m_sharedData.m_lock.lock();
	m_sharedData.m_numWorker--;
	m_sharedData.m_machineMap[m_sName] = "DisConnected Time:" + TimeSystem::getTimeString("Y/m/d_H:i:s");
	displayProgress();
	m_sharedData.m_lock.unlock();
}

void GenSgfSession::displayProgress()
{
	CERR() << "\033[2J\033[1;1H";
	CERR() << endl;
	CERR() << getColorMessage("Server start from " + m_sharedData.m_startTime,ANSITYPE_BOLD,ANSICOLOR_WHITE,ANSICOLOR_BLACK) << endl;
	CERR() << getColorMessage("Current mode: " + WeichiConfigure::gensgf_command,ANSITYPE_BOLD,ANSICOLOR_WHITE,ANSICOLOR_BLACK) << endl;
	CERR() << endl;
	CERR() << getColorMessage("Total has " + ToString(m_sharedData.m_machineMap.size()) + " workers",ANSITYPE_BOLD,ANSICOLOR_WHITE,ANSICOLOR_BLACK) << endl;
	CERR() << getColorMessage("Current has " + ToString(m_sharedData.m_numWorker) + " workers",ANSITYPE_BOLD,ANSICOLOR_WHITE,ANSICOLOR_BLACK) << endl;
	CERR() << getColorMessage("Total has " + ToString(m_sharedData.m_total_progress) + " sgfs",ANSITYPE_BOLD,ANSICOLOR_WHITE,ANSICOLOR_BLACK) << endl;

	string sSplitLine = "=============================================================================================";
	CERR() << getColorMessage(sSplitLine,ANSITYPE_BOLD,ANSICOLOR_WHITE,ANSICOLOR_BLACK) << endl;

	for( map<pair<string,string>,string>::iterator it=m_sharedData.m_machineMap.begin(); it!=m_sharedData.m_machineMap.end(); ++it ) {
		pair<string,string> sName = it->first;
		int nProgress = m_sharedData.m_machineProgress[sName];
		double dProgressRate = nProgress*100.0/m_sharedData.m_total_progress;

		ostringstream oss;
		oss << setw(20) << sName.first << " " << setw(10) << sName.second
			<< setw(12) << nProgress << " (" << setw(7) << fixed << setprecision(2) << dProgressRate << "%)\t" << it->second;
		if( it->second.find("Dis")==string::npos ) { CERR() << getColorMessage(oss.str(),ANSITYPE_BOLD,ANSICOLOR_WHITE,ANSICOLOR_BLACK) << endl; }
		else { CERR() << getColorMessage(oss.str(),ANSITYPE_NORMAL,ANSICOLOR_WHITE,ANSICOLOR_BLACK) << endl; }
	}

	CERR() << getColorMessage(sSplitLine,ANSITYPE_BOLD,ANSICOLOR_WHITE,ANSICOLOR_BLACK) << endl << endl;
}

void GenSgfSession::saveSgf( string sResult, fstream& fout, string sPostfix/*=""*/ )
{
	m_sharedData.m_lock.lock();

	if( !fout.is_open() || m_sharedData.m_total_progress%m_sharedData.MAX_GAMES_IN_ONE_FILE==0 ) {
		string sFileName = WeichiConfigure::gensgf_filename_prefix + TimeSystem::getTimeString("Y_m_d_H_i_s") + sPostfix + ".sgf";

		fout.close();
		fout.open(sFileName.c_str(),ios::out);
		CERR() << "Open a new file \"" << sFileName << "\"" << endl;
	}
	fout << sResult << endl;
	m_sharedData.m_total_progress++;
	m_sharedData.m_machineProgress[m_sName]++;

	m_sharedData.m_lock.unlock();
}

// GenSgfServer
bool GenSgfServer::checkConfigure()
{
	// check command
	if( WeichiConfigure::gensgf_command!="SL" && WeichiConfigure::gensgf_command!="RL" && WeichiConfigure::gensgf_command!="RL_END" ) {
		CERR() << "Unknown Command: \"" << WeichiConfigure::gensgf_command << "\""
			   << ", should be \"SL\" or \"RL\" or \"RL_END\"" << endl;
		return false;
	}

	return true;
}

void GenSgfServer::initialize()
{
	if( !checkConfigure() ) { exit(0); }

	m_shareData.m_numWorker = 0;
	m_shareData.m_total_progress = 0;
	m_shareData.m_startTime = TimeSystem::getTimeString("Y/m/d_H:i:s");
	if( WeichiConfigure::gensgf_command.find("RL")!=string::npos ) {
		m_shareData.m_explorer.setRootDirectory(WeichiConfigure::gensgf_opening_directory);
	}

	uint seed = Configure::UseTimeSeed? static_cast<uint>(time(NULL)): Configure::RandomSeed;
	Random::reset(seed);

	CERR() << "Initialize over, wait for client to connect." << endl;
}

GenSgfSession* GenSgfServer::getNewSession()
{
	return new GenSgfSession(m_io_service,m_shareData);
}

// GenSgfServer
void GenSgfServer::handle_accept( GenSgfSession* new_session )
{
	m_shareData.m_numWorker++;
	string sNewClinetMessage = "Accept a new client: " + new_session->getIP() + "/" + ToString(new_session->getPort());
	CERR() << getColorMessage(sNewClinetMessage,ANSITYPE_BOLD,ANSICOLOR_WHITE,ANSICOLOR_BLACK) << endl;
}

// GenSgfClient
void GenSgfClient::run()
{
	if( !connectToServer() ) {
		CERR() << getColorMessage("Connect failed.",ANSITYPE_BOLD,ANSICOLOR_RED,ANSICOLOR_BLACK) << endl;
		return;
	}

	CERR() << getColorMessage("Connect to server.",ANSITYPE_BOLD,ANSICOLOR_WHITE,ANSICOLOR_BLACK) << endl;
}

// GenSgfSlave
void GenSgfSlave::initialize()
{
	TBaseSlave::initialize();

	// load SL & RL net collection
	int gpu_id = WeichiConfigure::dcnn_train_gpu_list[m_id%WeichiConfigure::dcnn_train_gpu_list.length()] - '0';
	vector<string> vParamString = splitToVector(WeichiConfigure::gensgf_dcnn_sl_net,':');
	m_dcnnSLNetCollection.initialize(1,vParamString,gpu_id,(m_id==0));
	vParamString = splitToVector(WeichiConfigure::gensgf_dcnn_rl_net,':');
	m_dcnnRLNetCollection.initialize(1,vParamString,gpu_id,(m_id==0));
	
	int total_game = 0;
	if( m_sharedData.m_sCommand=="SL" ) {
		if( m_dcnnSLNetCollection.getNumSLNet()==0 ) {
			CERR() << "[Error] Found no SL net." << endl;
			exit(0);
		}
		total_game = m_dcnnSLNetCollection.getSLNet(0)->getBatchSize();
		for( uint id=1; id<m_dcnnSLNetCollection.getNumSLNet(); id++ ) {
			if( m_dcnnSLNetCollection.getSLNet(id)->getBatchSize()==total_game ) { continue; }
			CERR() << "[Error] Batch size didn't match" << endl;
			exit(0);
		}
		randomChangeSLNet();
	} else if( m_sharedData.m_sCommand.find("RL")!=string::npos ) {
		if( m_dcnnRLNetCollection.getNumSLNet()>1 ) {
			CERR() << "[Error] Should only has 1 RL net." << endl;
			exit(0);
		}
		total_game = m_dcnnRLNetCollection.getSLNet(0)->getBatchSize();
		for( uint id=1; id<m_dcnnRLNetCollection.getNumSLNet(); id++ ) {
			if( m_dcnnRLNetCollection.getSLNet(id)->getBatchSize()==total_game ) { continue; }
			CERR() << "[Error] Batch size didn't match" << endl;
			exit(0);
		}
		m_net = m_dcnnRLNetCollection.getSLNet();
	} else {
		CERR() << "[Error] Unknown command." << endl;
	}

	m_nFinish = 0;
	m_vGames.resize(total_game);
	for( int i=0; i<total_game; i++ ) { m_vGames[i] = new SlaveGame(); }

	// display message
	if( m_id==0 ) {
		CERR() << TimeSystem::getTimeString("[Y/m/d_H:i:s] ")
			<< "Initialize Over, total load " << m_dcnnSLNetCollection.getNumSLNet() << " SL net(s)"
			<< ", " << m_dcnnRLNetCollection.getNumSLNet() << " RL net(s)"
			<< ", total games = " << total_game << endl;
	}
}

bool GenSgfSlave::isOver()
{
	return false;
}

void GenSgfSlave::reset()
{
}

void GenSgfSlave::doSlaveJob()
{
	if( m_sharedData.m_sCommand.find("RL")!=string::npos ) {
		// since RL should ask opening first, set valid as false
		for( uint index=0; index<m_vGames.size(); index++ ) { m_vGames[index]->m_bIsValid = false; }
	}

	// never stop for running
	while( true ) {
		for( uint index=0; index<m_vGames.size(); index++ ) { handle_endGame(index); }
		for( uint index=0; index<m_vGames.size(); index++ ) { handle_loadNewGame(index); }
		for( uint index=0; index<m_vGames.size(); index++ ) { handle_addFeature(index); }
		handle_forward();

		if( m_nFinish==m_vGames.size() ) { break; }
	}
}

void GenSgfSlave::randomChangeSLNet()
{
	int numSLNet = m_dcnnSLNetCollection.getNumSLNet();
	m_net = m_dcnnSLNetCollection.getSLNet(Random::nextInt(numSLNet));
	m_net->resetCurrentBatchSize();

	m_sharedData.m_lock.lock();
	CERR() << TimeSystem::getTimeString("[Y/m/d_H:i:s] ")
		   << "Thread " << m_id << " choose new SL net, "
		   << m_net->getNetParam().m_sPrototxt << endl;
	m_sharedData.m_lock.unlock();
}

void GenSgfSlave::handle_endGame( int gameID )
{
	SlaveGame* game = m_vGames[gameID];
	if( !game->m_bIsValid ) { return; }

	const int MAX_PLAY_MOVES = 500;
	const WeichiBoard& board = game->m_state.m_board;
	if( m_sharedData.m_sCommand=="SL" ) {
		if( board.getMoveList().size()<game->m_random_move_number ) { return; }
	} else if( m_sharedData.m_sCommand.find("RL")!=string::npos ) {
		if( !board.isEarlyEndGame() && !board.isTwoPassEndGame() && board.getMoveList().size()<MAX_PLAY_MOVES ) { return; }
	}

	// save the game
	string sEvent = "";
	Dual<string> sPlayer;
	WeichiPlayoutResult result(game->m_state.eval());

	if( m_sharedData.m_sCommand=="SL" ) {
		sEvent = ToString(game->m_random_move_number);
		sPlayer.m_black = sPlayer.m_white = m_net->getNetParam().m_sPrototxt;
		sPlayer.m_black = sPlayer.m_white = sPlayer.m_black.substr(sPlayer.m_black.find_last_of("/")+1);
	} else if( m_sharedData.m_sCommand.find("RL")!=string::npos ) {
		sEvent = ToString(game->m_random_move_number);
		sEvent += ";" + game->m_sgfLoader.getSgfTag("RE");
		sEvent += ";" + game->m_sSgfLinkInfo;
	}
	SgfTag sgfTag;
	sgfTag.setSgfTag("RE", result.toString());
	sgfTag.setSgfTag("PB", sPlayer.m_black);
	sgfTag.setSgfTag("PW", sPlayer.m_white);
	sgfTag.setSgfTag("EV", sEvent);
	string sSgfResult = board.toSgfFileString(sgfTag);
	string sSendHeader = (board.getMoveList().size()<MAX_PLAY_MOVES)? "sgf_result": "abort_sgf";

	m_sharedData.m_lock.lock();
	m_sharedData.m_client->writeToServer(sSendHeader+" "+sSgfResult);
	CERR() << getColorMessage(">> sent \""+sSendHeader+"\" to server.",ANSITYPE_BOLD,ANSICOLOR_GREEN,ANSICOLOR_BLACK) << endl;
	m_sharedData.m_client->readFromServer();
	m_sharedData.m_lock.unlock();
	
	game->m_bIsValid = false;
}

void GenSgfSlave::handle_loadNewGame( int gameID )
{
	SlaveGame* game = m_vGames[gameID];
	if( game->m_bIsFinish || game->m_bIsValid ) { return; }

	if( m_sharedData.m_sCommand=="SL" ) {
		game->reset();
	} else if( m_sharedData.m_sCommand.find("RL")!=string::npos ) {
		m_sharedData.m_lock.lock();
		m_sharedData.m_client->writeToServer("get_opening",true);
		string sSgfInfo = m_sharedData.m_client->readFromServer();
		m_sharedData.m_lock.unlock();

		if( sSgfInfo=="QUIT" ) {
			m_nFinish++;
			game->m_bIsFinish = true;
			game->m_bIsValid = false;
		} else {
			game->m_sSgfLinkInfo = sSgfInfo.substr(0,sSgfInfo.find(" "));
			string sSgfString = sSgfInfo.substr(sSgfInfo.find(" ")+1); 
			game->loadGame(sSgfString,(m_sharedData.m_sCommand=="RL_END"));
			game->m_state.m_rootFilter.startFilter();
		}
	}
}

void GenSgfSlave::handle_addFeature( int gameID )
{
	SlaveGame* game = m_vGames[gameID];
	if( !game->m_bIsValid ) { return; }
	
	WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(game->m_state,m_net);
}

void GenSgfSlave::handle_forward()
{
	m_net->forward();
	int batch = 0;
	for( uint index=0; index<m_vGames.size(); index++ ) {
		SlaveGame* forwardGame = m_vGames[index];
		if( !forwardGame->m_bIsValid ) { continue; }
		if( m_sharedData.m_sCommand=="SL" && forwardGame->m_random_move_number==0 ) {
			batch++;
			continue;
		}

		forwardGame->m_state.play(m_net->getSoftMaxMove(batch++));
	}
}

// GenSgfMaster
void GenSgfMaster::run()
{
	if( !initialize() ) { return; }
	m_sStartTime = TimeSystem::getTimeString("Y/m/d_H:i:s");
	for( int i=0; i<m_nThread; i++ ) { m_vSlaves[i]->startRun(); }
	for( int i=0; i<m_nThread; i++ ) { m_vSlaves[i]->finishRun(); }
	m_sEndTime = TimeSystem::getTimeString("Y/m/d_H:i:s");
	summarizeSlavesData();
}

bool GenSgfMaster::initialize()
{
	m_sharedData.m_client->run();
	m_sharedData.m_client->writeToServer("name "+WeichiConfigure::gensgf_filename_prefix);
	m_sharedData.m_client->readFromServer(true);
	m_sharedData.m_client->writeToServer("get_command",true);
	m_sharedData.m_sCommand = m_sharedData.m_client->readFromServer(true);
	
	return TBaseMaster::initialize();
} 
