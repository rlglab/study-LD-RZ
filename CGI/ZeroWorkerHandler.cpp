#include "ZeroWorkerHandler.h"
#include "BaseCNNNet.h"
#include "SgfLoader.h"
#include "TimeSystem.h"

string ZeroWorkerSharedData::getOneSelfPlay()
{
	boost::lock_guard<boost::mutex> lock(m_selfPlayQueueMutex);
	if (m_selfPlayQueue.size() == 0) { return ""; }
	else {
		string sSelfPlay = m_selfPlayQueue.front();
		m_selfPlayQueue.pop_front();
		return sSelfPlay;
	}
}

string ZeroWorkerSharedData::getOptimizationResult()
{
	boost::lock_guard<boost::mutex> lock(m_optimizationQueueMutex);
	if (m_optimizationQueue.size() == 0) { return ""; }
	else {
		string sOptimization = m_optimizationQueue.front();
		m_optimizationQueue.pop_front();
		return sOptimization;
	}
}

// ZeroWorkerStatus
void ZeroWorkerStatus::handle_msg(const std::string msg)
{
	std::vector<std::string> vArgs;
	boost::split(vArgs, msg, boost::is_any_of(" "), boost::token_compress_on);

	if (vArgs[0] == "Info") {
		// Info name num_gpu type
		m_sName = vArgs[1];
		m_sGPUList = vArgs[2];
		if (vArgs[3] == "Self-play") { m_type = ZERO_SELFPLAY; }
		else if (vArgs[3] == "Optimization") { m_type = ZERO_OPTIMIZAITON; }
		else {
			std::cerr << "[error] receive error type \"" << vArgs[3] << "\" from worker, disconnect." << std::endl;
			do_close();
			return;
		}

		boost::lock_guard<boost::mutex> lock(m_sharedData.m_workerMutex);
		m_sharedData.m_fWorkerLog << "[Worker-Connection] "
			<< TimeSystem::getTimeString("Y/m/d_H:i:s.f ")
			<< m_sName << " " << m_sGPUList << " " << getZeroTypeString(m_type) << endl;
	} else if (vArgs[0] == "Train_DIR") {
		m_bIdle = true;
	} else if (vArgs[0] == "Self-play") {
		if (msg.find("Self-play", msg.find("Self-play", 0) + 1) != string::npos) { return; }
		if (m_type == ZERO_SELFPLAY) {
			string sSelfPlay = msg.substr(msg.find(vArgs[0]) + vArgs[0].length() + 1);
			boost::lock_guard<boost::mutex> lock(m_sharedData.m_selfPlayQueueMutex);
			m_sharedData.m_selfPlayQueue.push_back(sSelfPlay);
		}
	} else if (vArgs[0] == "Optimization") {
		boost::lock_guard<boost::mutex> lock(m_sharedData.m_optimizationQueueMutex);
		m_sharedData.m_optimizationQueue.push_back(vArgs[1]);
	} else {
		// unknown client, reject it
		string sMessage = msg;
		std::replace(sMessage.begin(), sMessage.end(), '\r', ' ');
		std::replace(sMessage.begin(), sMessage.end(), '\n', ' ');
		std::cerr << "[error] receive \"" << sMessage << "\" from worker, disconnect." << std::endl;
		do_close();
	}
}

void ZeroWorkerStatus::do_close()
{
	boost::lock_guard<boost::mutex> lock(m_sharedData.m_workerMutex);
	m_sharedData.m_fWorkerLog << "[Worker-Disconnection] " << TimeSystem::getTimeString("Y/m/d_H:i:s.f ") << m_sName << " " << m_sGPUList << endl;
	TBaseWorkerStatus::do_close();
}

// ZeroWorkerHandler (public function)
void ZeroWorkerHandler::run()
{
	initialize();
	start_accept();
	CERR() << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "Server initialize over." << endl;

	for (m_iteration = WeichiConfigure::zero_start_iteration; m_iteration <= WeichiConfigure::zero_end_iteration; ++m_iteration) {
		SelfPlay();
		if (m_iteration >= WeichiConfigure::zero_replay_buffer_size / 2) { Optimization(); }
	}
}

boost::shared_ptr<ZeroWorkerStatus> ZeroWorkerHandler::handle_accept_new_worker(boost::shared_ptr<tcp::socket> socket) {
	boost::shared_ptr<ZeroWorkerStatus> pWorkerStatus = boost::make_shared<ZeroWorkerStatus>(socket, m_sharedData);
	pWorkerStatus->write("Info");
	pWorkerStatus->write("TRAIN_DIR " + WeichiConfigure::zero_train_directory);
	
	return pWorkerStatus;
}

// ZeroWorkerHandler (private function)
void ZeroWorkerHandler::initialize()
{
	uint seed = Configure::UseTimeSeed ? static_cast<uint>(time(NULL)) : Configure::RandomSeed;
	Random::reset(seed);
	m_stage = ZERO_UNKNOWN;
	m_fKomi = WeichiConfigure::komi;

	// create log files
	m_logger.createLogDirectoryAndFiles();

	// latest model
	CNNNetParam blackParam(WeichiConfigure::zero_selfplay_black_net);
	CNNNetParam whiteParam(WeichiConfigure::zero_selfplay_white_net);
	m_sLatestModel = blackParam.m_sCaffeModel.substr(blackParam.m_sCaffeModel.find_last_of("/") + 1);

	// self-play configure
	m_sSelfPlayConfigure = "";
	m_sSelfPlayConfigure += "USE_TIME_SEED=false:BOARD_SIZE=" + ToString(WeichiConfigure::BoardSize);
	m_sSelfPlayConfigure += ":PUCT_BIAS=" + ToString(WeichiConfigure::mcts_puct_bias);
	//m_sSelfPlayConfigure += ":ZERO_NUM_SIMULATION=" + ToString(WeichiConfigure::zero_num_simulation);
	m_sSelfPlayConfigure += ":ZERO_THRESHOLD_RATIO=" + ToString(WeichiConfigure::zero_threshold_ratio);
	m_sSelfPlayConfigure += ":ZERO_ROOT_NODE_NOISE=" + ToString(WeichiConfigure::zero_root_node_noise);
	m_sSelfPlayConfigure += ":ZERO_NOISE_ALPHA=" + ToString(WeichiConfigure::zero_noise_alpha);
	m_sSelfPlayConfigure += ":ZERO_NOISE_EPSILON=" + ToString(WeichiConfigure::zero_noise_epsilon);
	m_sSelfPlayConfigure += ":ZERO_DISABLE_RESIGN_PROB=" + ToString(WeichiConfigure::zero_disable_resign_prob);
	m_sSelfPlayConfigure += ":DCNN_USE_FTL=" + ToString(WeichiConfigure::dcnn_use_ftl);
	m_sSelfPlayConfigure += ":ZERO_TRAINING_7x7_KILLALL=" + ToString(WeichiConfigure::zero_training_7x7_killall);
	m_sSelfPlayConfigure += ":ZERO_SELFPLAY_OPENINGS_FILE=" + ToString(WeichiConfigure::zero_selfplay_openings_file);

	// customized configure
	m_sSelfPlayConfigure += ":RANDOM_SEED=@RANDOMSEED@";
	m_sSelfPlayConfigure += ":KOMI=@KOMI@";
	m_sSelfPlayConfigure += ":DCNN_TRAIN_GPU_LIST=@GPULIST@";
	m_sSelfPlayConfigure += ":ZERO_RESIGN_THRESHOLD=@RESIGN_THRESHOLD@";
	m_sSelfPlayConfigure += ":ZERO_SELFPLAY_MACHINE_NAME=@MACHINENAME@";
	m_sSelfPlayConfigure += ":ZERO_NUM_SIMULATION=@ZERONUMSIMULATION@";
	m_sSelfPlayConfigure += ":ZERO_SELFPLAY_BLACK_NET=NET|" + ToString(blackParam.m_iRandomNum);
	m_sSelfPlayConfigure += "|" + getWeichiCNNFeatureTypeString(blackParam.m_featureType) + "|";
	m_sSelfPlayConfigure += "../../" + WeichiConfigure::zero_train_directory + "/model/";
	m_sSelfPlayConfigure += blackParam.m_sPrototxt.substr(blackParam.m_sPrototxt.find_last_of("/") + 1) + "|@BLACKMODEL@|0";
	m_sSelfPlayConfigure += ":ZERO_SELFPLAY_WHITE_NET=NET|" + ToString(whiteParam.m_iRandomNum);
	m_sSelfPlayConfigure += "|" + getWeichiCNNFeatureTypeString(whiteParam.m_featureType) + "|";
	m_sSelfPlayConfigure += "../../" + WeichiConfigure::zero_train_directory + "/model/";
	m_sSelfPlayConfigure += whiteParam.m_sPrototxt.substr(whiteParam.m_sPrototxt.find_last_of("/") + 1) + "|@WHITEMODEL@|0";

	// optimization configure
	m_sOptimizationConfigure = "";
	m_sOptimizationConfigure += "USE_TIME_SEED=false:BOARD_SIZE=" + ToString(WeichiConfigure::BoardSize);
	m_sOptimizationConfigure += ":RANDOM_SEED=" + ToString(getNextSeed());
	m_sOptimizationConfigure += ":LOG_CONSOLE_FILENAME=console_[Y_m_d_H_i_s].txt";
	m_sOptimizationConfigure += ":ZERO_TRAIN_DIRECTORY=" + ToString(WeichiConfigure::zero_train_directory);
	m_sOptimizationConfigure += ":ZERO_REPLAY_BUFFER_SIZE=" + ToString(WeichiConfigure::zero_replay_buffer_size);
	m_sOptimizationConfigure += ":ZERO_OPTIMIZATION_NUM_WORKER=" + ToString(WeichiConfigure::zero_optimization_num_worker);	
	m_sOptimizationConfigure += ":ZERO_OPTIMIZATION_LEARNING_RATE=" + ToString(WeichiConfigure::zero_optimization_learning_rate);
	CNNNetParam optimizationParam(WeichiConfigure::zero_optimization_net);
	m_sOptimizationConfigure += ":ZERO_OPTIMIZATION_NET=NET|" + ToString(optimizationParam.m_iRandomNum);
	m_sOptimizationConfigure += "|" + getWeichiCNNFeatureTypeString(optimizationParam.m_featureType);
	m_sOptimizationConfigure += "||@SOLVERSTATE@|0";
	m_sOptimizationConfigure += ":ZERO_OPTIMIZATION_SOLVER=" + WeichiConfigure::zero_train_directory + "/model/" + WeichiConfigure::zero_optimization_solver;
	m_sOptimizationConfigure += ":ZERO_OPTIMIZATION_SNAPSHOT_ITER=" + ToString(WeichiConfigure::zero_optimization_snapshot_iter);
	m_sOptimizationConfigure += ":ZERO_OPTIMIZAIOTN_SAVE_NAME_PREFIX=" + WeichiConfigure::zero_optimization_save_name_prefix;
	m_sOptimizationConfigure += ":DCNN_USE_FTL=" + ToString(WeichiConfigure::dcnn_use_ftl);
	
	// customized configure
	m_sOptimizationConfigure += ":KOMI=@KOMI@";
	m_sOptimizationConfigure += ":ZERO_OPTIMIZATION_SGF_MAX_ITERATION=@SGFMAXITERATION@";
}

void ZeroWorkerHandler::SelfPlay()
{
	// setup
	m_total_games = 0;
	m_stage = ZERO_SELFPLAY;
	string sSelfPlayGameFileName = WeichiConfigure::zero_train_directory + "/sgf/" + ToString(m_iteration) + ".sgf";
	m_logger.m_fSelfPlayGame.open(sSelfPlayGameFileName.c_str(), ios::out);
	string sSelfPlayDebugGameFileName = WeichiConfigure::zero_train_directory + "/sgf/debug/" + ToString(m_iteration) + ".sgf";
	m_logger.m_fSelfPlayDebugGame.open(sSelfPlayDebugGameFileName.c_str(), ios::out);
	m_logger.m_fTrainingLog << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "[Iteration] =====" << m_iteration << "=====" << endl;
	m_logger.m_fTrainingLog << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "[SelfPlay] Start " << getModelIteration(m_sLatestModel) << endl;

	// collect selfplay games
	int komi_results[99] = {0};
	StatisticData wr;
	while (m_total_games < WeichiConfigure::zero_num_game) {
		// send selfplay command
		{
			boost::lock_guard<boost::mutex> lock(m_workerMutex);
			for (auto worker : m_vWorkers) {
				if (worker->getType() != ZERO_SELFPLAY || !worker->isIdle()) { continue; }
				worker->setIdle(false);
				worker->write(getSelfPlayConfigure(worker));
			}
		}

		string sSelfPlay = "";
		while ((sSelfPlay = m_sharedData.getOneSelfPlay()) != "") {
			if (sSelfPlay.find(m_sLatestModel) != string::npos) { break; }
		}
		if (sSelfPlay == "") {
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
			continue;
		}

		SgfLoader sgfLoader;
		if (!sgfLoader.parseFromString(sSelfPlay)) { continue; }
		if (sgfLoader.getWinner() == COLOR_BLACK) { wr.add(1); }
		else if (sgfLoader.getWinner() == COLOR_WHITE) { wr.add(0); }
		else { wr.add(0.5); }
		if (WeichiConfigure::dcnn_use_ftl) {
			int move_size = sgfLoader.getPlayMoveLength();
			move_size = (move_size >= 98 ? 98 : move_size);
			++komi_results[move_size];
		} else {
			string sResultInfo = sgfLoader.getSgfTag("RE");
			int BLeadCount = (sResultInfo == "0" ? 0 : static_cast<float>(atof(sResultInfo.substr(2).c_str())));
			BLeadCount = (sgfLoader.getWinner() == COLOR_BLACK) ? BLeadCount + sgfLoader.getKomi() : -BLeadCount + sgfLoader.getKomi();
			++komi_results[BLeadCount + 49];	// special case for 7x7
		}

		string sMoveNumber = sSelfPlay.substr(0, sSelfPlay.find("(") - 1);
		string sSgfString = sSelfPlay.substr(sSelfPlay.find("("));
		string sSimpleSgf = deleteUnusedSgfTag(sSgfString);

		m_logger.m_fSelfPlayGame << m_total_games << " " << sMoveNumber << " " << sSimpleSgf << endl;
		m_logger.m_fSelfPlayDebugGame << m_total_games << " " << sMoveNumber << " " << sSgfString << endl;
		addResignThreshold(sSimpleSgf);
		m_total_games++;

		if (m_total_games % static_cast<int>(WeichiConfigure::zero_num_game * 0.25) == 0) {
			m_logger.m_fTrainingLog << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "[SelfPlay Progress] "
				<< m_total_games << " / " << WeichiConfigure::zero_num_game << endl;
		}
	}

	// change komi
	int newKomi = 0, count = 0;
	for (int i = 0; i < 99; ++i) {
		count += komi_results[i];
		if (count < m_total_games / 2) { continue; }
		newKomi = i;
		break;
	}
	if (!WeichiConfigure::dcnn_use_ftl) { newKomi -= 49; }
	m_logger.m_fTrainingLog << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "[SelfPlay] Black WR: " << wr.toString() << endl;
	m_logger.m_fTrainingLog << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "[SelfPlay] Komi change: " << m_fKomi << " -> " << newKomi << endl;
	m_fKomi = newKomi;

	m_logger.m_fSelfPlayGame.close();
	m_logger.m_fSelfPlayDebugGame.close();
	adjustResignThreshold();

	// notify selfplay worker
	{
		boost::lock_guard<boost::mutex> lock(m_workerMutex);
		for (auto worker : m_vWorkers) {
			if (worker->getType() != ZERO_SELFPLAY) { continue; }
			worker->setIdle(true);
			worker->write("Job_Done");
		}
	}
	m_logger.m_fTrainingLog << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "[SelfPlay] Finished." << endl;
}

void ZeroWorkerHandler::Optimization()
{
	m_logger.m_fTrainingLog << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "[Optimization] Start." << endl;

	string sLatestModel = "";
	while ((sLatestModel = m_sharedData.getOptimizationResult()) == "") {
		// notify optimization worker
		{
			boost::lock_guard<boost::mutex> lock(m_workerMutex);
			for (auto worker : m_vWorkers) {
				if (worker->getType() != ZERO_OPTIMIZAITON || !worker->isIdle()) { continue; }
				worker->setIdle(false);
				worker->write(getOptimizationConfigure());
			}
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}
	m_sLatestModel = sLatestModel;

	// notify optimization worker
	{
		boost::lock_guard<boost::mutex> lock(m_workerMutex);
		for (auto worker : m_vWorkers) {
			if (worker->getType() != ZERO_OPTIMIZAITON) { continue; }
			worker->setIdle(true);
		}
	}

	while (m_sharedData.getOptimizationResult() == "") {
		// change weight
		{
			boost::lock_guard<boost::mutex> lock(m_workerMutex);
			for (auto worker : m_vWorkers) {
				if (worker->getType() != ZERO_OPTIMIZAITON || !worker->isIdle()) { continue; }
				worker->setIdle(false);
				worker->write("Change_weight");
			}
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}

	// notify optimization worker
	{
		boost::lock_guard<boost::mutex> lock(m_workerMutex);
		for (auto worker : m_vWorkers) {
			if (worker->getType() != ZERO_OPTIMIZAITON) { continue; }
			worker->setIdle(true);
		}
	}
	m_logger.m_fTrainingLog << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "[Optimization] Finished." << endl;
}

string ZeroWorkerHandler::getSelfPlayConfigure(boost::shared_ptr<ZeroWorkerStatus> worker)
{
	string sConfigure = "Self-play " + m_sSelfPlayConfigure;
	sConfigure = sConfigure.replace(sConfigure.find("@KOMI@"), sizeof("@KOMI@") - 1, ToString(m_fKomi));
	sConfigure = sConfigure.replace(sConfigure.find("@RANDOMSEED@"), sizeof("@RANDOMSEED@") - 1, ToString(getNextSeed()));
	sConfigure = sConfigure.replace(sConfigure.find("@GPULIST@"), sizeof("@GPULIST@") - 1, worker->getGPUList());
	sConfigure = sConfigure.replace(sConfigure.find("@RESIGN_THRESHOLD@"), sizeof("@RESIGN_THRESHOLD@") - 1, ToString(WeichiConfigure::zero_resign_threshold));

	// num simulation
	int simulation = WeichiConfigure::zero_num_simulation;
	if (m_iteration <= WeichiConfigure::zero_replay_buffer_size / 2) { simulation = 1; }
	else if (m_iteration <= WeichiConfigure::zero_replay_buffer_size) { simulation = 100; }
	sConfigure = sConfigure.replace(sConfigure.find("@ZERONUMSIMULATION@"), sizeof("@ZERONUMSIMULATION@") - 1, ToString(simulation));
	
	// machine name
	string sMachineName = worker->getName().substr(0, worker->getName().find("_"));
	sConfigure = sConfigure.replace(sConfigure.find("@MACHINENAME@"), sizeof("@MACHINENAME@") - 1, sMachineName);

	// model
	string sBlackModel = "../../" + WeichiConfigure::zero_train_directory + "/model/" + m_sLatestModel;
	string sWhiteModel = "../../" + WeichiConfigure::zero_train_directory + "/model/" + m_sLatestModel;
	sConfigure = sConfigure.replace(sConfigure.find("@BLACKMODEL@"), sizeof("@BLACKMODEL@") - 1, sBlackModel);
	sConfigure = sConfigure.replace(sConfigure.find("@WHITEMODEL@"), sizeof("@WHITEMODEL@") - 1, sWhiteModel);

	return sConfigure;
}

string ZeroWorkerHandler::getOptimizationConfigure()
{
	string sConfigure = "Optimization " + m_sOptimizationConfigure;
	//sConfigure = sConfigure.replace(sConfigure.find("@KOMI@"), sizeof("@KOMI@") - 1, ToString(m_fKomi));
	sConfigure = sConfigure.replace(sConfigure.find("@KOMI@"), sizeof("@KOMI@") - 1, ToString(0));
	sConfigure = sConfigure.replace(sConfigure.find("@SGFMAXITERATION@"), sizeof("@SGFMAXITERATION@") - 1, ToString(m_iteration));

	string sSolverstate = "";
	if (m_sLatestModel == "iter_0.pb") { sSolverstate = "../../" + WeichiConfigure::zero_train_directory + "/model/iter_0.caffemodel"; }
	else { sSolverstate = "../../_" + m_sLatestModel.substr(0, m_sLatestModel.find(".")) + ".solverstate"; }
	sConfigure = sConfigure.replace(sConfigure.find("@SOLVERSTATE@"), sizeof("@SOLVERSTATE@") - 1, sSolverstate);

	return sConfigure;
}

void ZeroWorkerHandler::adjustResignThreshold()
{
	std::sort(m_vResignThreshold.begin(), m_vResignThreshold.end());
	
	float fOldResignThreshold = WeichiConfigure::zero_resign_threshold;
	if (m_vResignThreshold.size() > 0) {
		const float FALSE_POSITIVES_RATIO = 0.05f;
		int index = m_vResignThreshold.size() * FALSE_POSITIVES_RATIO;
		WeichiConfigure::zero_resign_threshold = m_vResignThreshold[index];
		m_logger.m_fTrainingLog << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "[SelfPlay Disable Resign Games] "
			<< m_vResignThreshold.size() << endl;
	}

	m_logger.m_fTrainingLog << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "[SelfPlay Resign Threshold] "
		<< fOldResignThreshold << " -> " << WeichiConfigure::zero_resign_threshold << endl;
	if (m_iteration <= 5) { WeichiConfigure::zero_resign_threshold = -1.0f; }
	else if (WeichiConfigure::zero_resign_threshold > -0.6) { WeichiConfigure::zero_resign_threshold = -0.6; }

	m_vResignThreshold.clear();
}

void ZeroWorkerHandler::addResignThreshold(string sSgfString)
{
	if (sSgfString.find("Mini-winrate:") == string::npos) { return; }

	string sResignThreshold = sSgfString.substr(sSgfString.find("Mini-winrate: ") + ToString("Mini-winrate: ").length());
	sResignThreshold = sResignThreshold.substr(0, sResignThreshold.find(";"));

	m_vResignThreshold.push_back(atof(sResignThreshold.c_str()));
}

string ZeroWorkerHandler::deleteUnusedSgfTag(string sSgfString)
{
	int end = 0;
	string sNewSgfString = "";

	while ((end = sSgfString.find("*")) != string::npos) {
		sNewSgfString += sSgfString.substr(0, end);
		sSgfString = sSgfString.substr(end);
		sSgfString = sSgfString.substr(sSgfString.find("]"));
	}
	sNewSgfString += sSgfString;

	return sNewSgfString;
}

void ZeroWorkerHandler::keepAlive()
{
	broadcast("keep_alive");
	keepAliveTimer();
}

void ZeroWorkerHandler::keepAliveTimer()
{
	m_keepAliveTimer.expires_from_now(boost::posix_time::minutes(1));
	m_keepAliveTimer.async_wait(boost::bind(&ZeroWorkerHandler::keepAlive, this));
}