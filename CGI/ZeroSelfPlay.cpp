#include "ZeroSelfPlay.h"
#include "SgfLoader.h"
#include <boost/algorithm/string.hpp>

// ZeroMSSharedData (public function)
uint ZeroSelfPlayMSSharedData::getNextMiniMCTSIndex()
{
	boost::lock_guard<boost::mutex> lock(m_mutex);

	int iIndex = m_miniMCTSIndex++;
	int max_batch_size = m_gpu_size * m_vCNNNet.m_black[0]->getBatchSize();
	if (iIndex >= max_batch_size) { return -1; }

	return iIndex;
}

// ZeroSlave (public function)
void ZeroSelfPlaySlave::doSlaveJob()
{
	if (m_sharedData.m_bForwardGPU && isGPUThread()) {
		m_cnnNet.get(m_sharedData.m_turnColor)->forward();
	} else if (!m_sharedData.m_bForwardGPU && isCPUThread()) {
		uint gameIndex;
		while ((gameIndex = m_sharedData.getNextMiniMCTSIndex()) != -1) { doCPUSelfPlay(gameIndex); }
	}
}

// ZeroSlave (private function)
void ZeroSelfPlaySlave::initialize()
{
	TBaseSlave::initialize();
	if (!isGPUThread()) {
		m_bIsInitialize = true;
		return;
	}

	m_cnnNet.m_black = m_sharedData.m_vCNNNet.m_black[m_id];
	m_cnnNet.m_black->setGpuID(WeichiConfigure::dcnn_train_gpu_list[m_id] - '0');
	m_cnnNet.m_black->loadNetWork();
	m_cnnNet.m_black->forward();

	m_cnnNet.m_white = m_sharedData.m_vCNNNet.m_white[m_id];
	m_cnnNet.m_white->setGpuID(WeichiConfigure::dcnn_train_gpu_list[m_id] - '0');
	m_cnnNet.m_white->loadNetWork();
	m_cnnNet.m_white->forward();

	m_bIsInitialize = true;
}

void ZeroSelfPlaySlave::doCPUSelfPlay(uint gameIndex)
{
	TMiniMCTS* miniMCTS = m_sharedData.m_vMiniMCTS[gameIndex];

	if (miniMCTS->getState().getRootTurn() != m_sharedData.m_turnColor) { return; }
	miniMCTS->runMCTSSimulationAfterForward();
	miniMCTS->runMCTSSimulationBeforeForward();
}

// ZeroMaster (public function)
void ZeroSelfPlayMaster::run()
{
	if (!initialize()) { return; }

	m_sharedData.m_bForwardGPU = false;
	m_sharedData.m_turnColor = COLOR_BLACK;
	while (true) {
		if (!m_bHasCommand) {
			m_sharedData.m_miniMCTSIndex = 0;
			for (int i = 0; i < m_nThread; i++) { m_vSlaves[i]->startRun(); }
			for (int i = 0; i < m_nThread; i++) { m_vSlaves[i]->finishRun(); }

			m_sharedData.m_bForwardGPU = !m_sharedData.m_bForwardGPU;
			int numBlack = 0;
			for (uint i = 0; i < m_sharedData.m_vMiniMCTS.size(); i++) {
				if (m_sharedData.m_vMiniMCTS[i]->getState().getRootTurn() == COLOR_BLACK) { numBlack++; }
				else { numBlack--; }
			}
			m_sharedData.m_turnColor = (numBlack >= 0 ? COLOR_BLACK : COLOR_WHITE);
		} else {
			replaceNetworkModel();
			CERR() << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << m_command << endl;
			m_bHasCommand = false;
		}
	}
	m_input_thread.join();
}

bool ZeroSelfPlayMaster::initialize()
{
	m_sharedData.m_gpu_size = static_cast<int>(WeichiConfigure::dcnn_train_gpu_list.length());

	for (int net_id = 0; net_id < m_sharedData.m_gpu_size; net_id++) {
		m_sharedData.m_vCNNNet.m_black.push_back(new WeichiCNNNet(CNNNetParam(WeichiConfigure::zero_selfplay_black_net)));
		m_sharedData.m_vCNNNet.m_white.push_back(new WeichiCNNNet(CNNNetParam(WeichiConfigure::zero_selfplay_white_net)));
	}
	if (!TBaseMaster::initialize()) { return false; }
	
	for (int i = 0; i < m_nThread; i++) {
		while (!m_vSlaves[i]->isInitialize()) { boost::this_thread::sleep(boost::posix_time::milliseconds(100)); }
	}
	if (m_sharedData.m_vCNNNet.m_black[0]->getBatchSize() != m_sharedData.m_vCNNNet.m_white[0]->getBatchSize()) {
		CERR() << "Batch size not match!" << endl;
		exit(0);
	}

	int batch_size = m_sharedData.m_vCNNNet.m_black[0]->getBatchSize();
	int nMCTSGameSize = m_sharedData.m_gpu_size * batch_size;
	for (int i = 0; i < nMCTSGameSize; i++) {
		m_sharedData.m_vMiniMCTS.push_back(new TMiniMCTS(WeichiConfigure::zero_num_simulation, m_sharedData.m_mutex));
		
		TMiniMCTS* miniMCTS = m_sharedData.m_vMiniMCTS.back();
		int net_index = (i % (m_sharedData.m_gpu_size * batch_size)) / batch_size;
		miniMCTS->setDisplayBoard(i == 0);
		miniMCTS->setBatchID(i % batch_size);
		miniMCTS->setCNNNet(m_sharedData.m_vCNNNet.m_black[net_index], COLOR_BLACK);
		miniMCTS->setCNNNet(m_sharedData.m_vCNNNet.m_white[net_index], COLOR_WHITE);
		miniMCTS->setNoise(WeichiConfigure::zero_root_node_noise);
		miniMCTS->newGame();
	}

	// run another thread to read standard input
	m_input_thread = boost::thread(boost::bind(&ZeroSelfPlayMaster::readInput, this));

	return true;
}

void ZeroSelfPlayMaster::readInput()
{
	while (true) {
		if (!m_bHasCommand) {
			if (!getline(cin, m_command)) { break; }
			m_bHasCommand = true;
		}
	}
}

void ZeroSelfPlayMaster::replaceNetworkModel()
{
	std::vector<std::string> vArgs;
	boost::split(vArgs, m_command, boost::is_any_of(" "), boost::token_compress_on);

	if (vArgs[0] == "Black_model" || vArgs[0] == "White_model") {
		string sModelPath = WeichiConfigure::dcnn_dir + vArgs[1];
		CERR() << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "Replace network model: " << sModelPath << endl;
		for (int net_id = 0; net_id < m_sharedData.m_gpu_size; net_id++) {
			if (vArgs[0] == "Black_model") { m_sharedData.m_vCNNNet.m_black[net_id]->reloadNetWork(sModelPath); }
			else if (vArgs[0] == "White_model") { m_sharedData.m_vCNNNet.m_white[net_id]->reloadNetWork(sModelPath); }
		}
	} else if (vArgs[0] == "Resign_threshold") {
		CERR() << TimeSystem::getTimeString("[Y/m/d H:i:s.f] ") << "Resign threshold: "
			<< WeichiConfigure::zero_resign_threshold << " -> " << vArgs[1] << endl;
		WeichiConfigure::zero_resign_threshold = atof(vArgs[1].c_str());
	}
} 
