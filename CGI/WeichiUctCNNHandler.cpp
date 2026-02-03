#include "WeichiUctCNNHandler.h"
#include "WeichiUctNode.h"
#include "UctChildIterator.h"
#include "NodeAllocator.h"
#include "WeichiJLMctsHandler.h"

void WeichiUctCNNHandler::doUctCNNJob()
{
	if (!m_dcnnNetCollection.hasCNNNet()) { return; }

	if (WeichiConfigure::do_mcts_sequential_search && WeichiGlobalInfo::getDoCNNForward() != 1) { return; }

	getCNNData().clear();
	if (collectJobFromQueue() == 0) {
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		return;
	}

	forwardAndUpdate();	

	if (WeichiConfigure::do_mcts_sequential_search) { WeichiGlobalInfo::getDoCNNForward() = 0; }
}

void WeichiUctCNNHandler::putBackAllToQueue()
{
	if (!m_dcnnNetCollection.hasCNNNet()) { return; }
	putBackToQueue();
}

std::string WeichiUctCNNHandler::runRemoteJob( WeichiCNNNetType type, const vector<CNNJob>& vJobs )
{
	UctNodePtr pParent;
	vector<CNNData>& vCNNData = getCNNData();
	vCNNData.clear();
	for (uint i = 0; i < vJobs.size(); i++) {
		m_state.rollback();
		for (uint j = 0; j < vJobs[i].m_vPaths.size(); ++j) {
			m_state.play(vJobs[i].m_vPaths[j]);
		}

		CNNData cnnData;
		cnnData.m_bIsValid = true;
		cnnData.m_vPaths = vJobs[i].m_vPaths;
		vCNNData.push_back(cnnData);

		WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(m_state, m_dcnnNetCollection.getCNNNet());
	}

	m_dcnnNetCollection.getCNNNet()->forward();

	// set return string of reply_job
	ostringstream oss;
	for (uint i = 0; i < vJobs.size(); i++) {
		if ( m_dcnnNetCollection.getCNNNet()->hasSLOutput() ) {
			const vector<CandidateEntry>& vCandidates = m_dcnnNetCollection.getCNNNet()->getSLCandidates(i);
			int returnCandidateSize = (vCandidates.size()>50 ? 50 : vCandidates.size());
			oss << ' ' << returnCandidateSize;
			for ( uint j = 0; j < returnCandidateSize; ++j ) {
				oss << ' ' << vCandidates[j].getPosition() << ' ' << vCandidates[j].getScore();
			}
		}
		
		if ( m_dcnnNetCollection.getCNNNet()->hasVNOutput() ) {
			const vector<float>& fValue = m_dcnnNetCollection.getCNNNet()->getVNResult(i);
			oss << ' ' << fValue.size() << ' ' << toChar(m_dcnnNetCollection.getCNNNet()->getColor(i));
			for ( uint j = 0; j < fValue.size(); ++j ) {
				oss << ' ' << fValue[j];
			}
		}
	}
	return oss.str();
}

void WeichiUctCNNHandler::updateAll( const WeichiRemoteCNNJob& currentJob )
{
	// TODO: a function for updateNet
	updateNet(currentJob);
	putBackToQueue();
}

int WeichiUctCNNHandler::collectJobFromQueue()
{
	int nCandidates = 0;
	WeichiCNNNet* net = m_dcnnNetCollection.getCNNNet();

	while (!net->isFull()) {
		int newCandidatesCnt = getQueueCandidates(net->getSpaceLeft());
		nCandidates += newCandidatesCnt;
		if (newCandidatesCnt == 0) { break; }
	}
	
	return nCandidates;
}

int WeichiUctCNNHandler::getQueueCandidates(int nSpaceLeft)
{
	int num = 0;
	UctNodePtr pParent;
	vector<CNNData>& vCNNData = getCNNData();
	vector<CNNJob> vJobs = WeichiGlobalInfo::getCNNJobQueue().getJob(nSpaceLeft);

	for (uint i = 0; i < vJobs.size(); i++) {
		UctNodePtr pNode = selectNodeByPaths(vJobs[i].m_vPaths, true, false, pParent);
		if (pNode == UctNodePtr::NULL_PTR) { continue; }

		CNNData cnndata;
		cnndata.m_bIsValid = true;
		cnndata.m_vPaths = vJobs[i].m_vPaths;
		vCNNData.push_back(cnndata);				
		WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(m_state, m_dcnnNetCollection.getCNNNet());
		num++;
	}

	return num;
}

WeichiUctCNNHandler::UctNodePtr WeichiUctCNNHandler::selectNodeByPaths( Vector<WeichiMove,MAX_TREE_DEPTH> vMoves, bool bWithPlay, bool bWithToken, UctNodePtr& pParent )
{
	vector<UctNodePtr> vNodePaths;
	UctNodePtr pNode = selectNodeByPaths(vMoves,bWithPlay,bWithToken,vNodePaths);

	if( vNodePaths.size()<2 ) { pParent = UctNodePtr::NULL_PTR; }
	else { pParent = vNodePaths[vNodePaths.size()-2]; }

	return pNode;
}

WeichiUctCNNHandler::UctNodePtr WeichiUctCNNHandler::selectNodeByPaths(Vector<WeichiMove, MAX_TREE_DEPTH> vMoves, bool bWithPlay, bool bWithToken, vector<UctNodePtr>& vNodePaths)
{
	bWithPlay = true;
	if (bWithPlay) { m_state.rollback(); }

	UctNodePtr pNode = m_pRoot, pNext;
	UctNodePtr pParent = UctNodePtr::NULL_PTR;

	vNodePaths.push_back(pNode);
	if (bWithToken) { getUpdateToken(pNode); }
	for (uint i = 0; i < vMoves.size(); ++i) {
		const WeichiMove& move = vMoves[i];
		pNext = selectChildByMove(pNode, move);
		if (pNext.isValid()) {
			if (bWithToken) { getUpdateToken(pNext); }
			releaseUpdateToken(pParent);
			pParent = pNode;
			pNode = pNext;
			vNodePaths.push_back(pNode);
			if (bWithPlay) { m_state.play(move); }
		} else {
			releaseUpdateToken(pNode);
			releaseUpdateToken(pParent);
			pParent = UctNodePtr::NULL_PTR;
			return UctNodePtr::NULL_PTR;
		}
	}

	return pNode;
}

WeichiUctCNNHandler::UctNodePtr WeichiUctCNNHandler::selectChildByMove( UctNodePtr pNode, WeichiMove move )
{
	if( pNode.isNull() ) { return UctNodePtr::NULL_PTR; }
	for( UctChildIterator<WeichiUctNode> it(pNode); it; ++it ) {
		if( it->getMove()!=move ) { continue; }
		return it;
	}
	return UctNodePtr::NULL_PTR;
}

void WeichiUctCNNHandler::forwardAndUpdate()
{
	m_dcnnNetCollection.getCNNNet()->forward();
	updateAll();
	putBackToQueue();
}

void WeichiUctCNNHandler::updateAll()
{
	vector<CNNData>& vCNNData = getCNNData();
	for (uint batch = 0; batch < vCNNData.size(); ++batch) {
		CNNData& cnnData = vCNNData[batch];
		if (!cnnData.m_bIsValid) { continue; }

		bool bUpdateSuccess = updateNet(batch, cnnData, m_dcnnNetCollection.getCNNNet());
		if (bUpdateSuccess) { cnnData.m_bIsValid = false; }
	}
}

void WeichiUctCNNHandler::putBackToQueue()
{
	int nRemoveUnfinishedJob = 0;
	vector<CNNData>& vCNNData = getCNNData();
	for (uint batch = 0; batch < vCNNData.size(); batch++) {
		CNNData& cnnData = vCNNData[batch];
		if (!cnnData.m_bIsValid) { ++nRemoveUnfinishedJob; continue; }

		WeichiGlobalInfo::getCNNJobQueue().addJobToFront(CNNJob(cnnData.m_vPaths));
		cnnData.m_bIsValid = false;
	}

	WeichiGlobalInfo::getCNNJobQueue().removeUnfinishedJob(nRemoveUnfinishedJob);
}

void WeichiUctCNNHandler::updateNet(const WeichiRemoteCNNJob& currentJob)
{
	const vector<CNNJob>& vJobs = currentJob.getJobContent();
	const vector<vector<CandidateEntry> >& vSLResults = currentJob.getSLResult();
	const vector<Color>& vVNColors = currentJob.getVNColors();
	const vector<vector<float> >& vVNResults = currentJob.getVNValues();
	vector<CNNData>& vCNNData = getCNNData();
	vCNNData.clear();
	for ( uint i = 0; i < vJobs.size(); ++i ) {
		// set cnnData
		CNNData cnnData;
		cnnData.m_bIsValid = true;
		cnnData.m_vPaths = vJobs[i].m_vPaths;
		vCNNData.push_back(cnnData);

		// set netResult (assume hasSLOutput() and hasVNOutput() are true)
		DCNNNetResult netResult;
		netResult.setHasPolicyResult(true);
		netResult.setPolicyResult(vSLResults[i]);

		netResult.setHasValueResult(true);
		netResult.setColor(vVNColors[i]);
		netResult.setValueResult(vVNResults[i][WeichiCNNNet::getVNKomiIndex(WeichiDynamicKomi::Internal_komi, vVNResults[i].size())]);

		bool bUpdateSuccess = updateNet(vCNNData[i], netResult);
		if ( bUpdateSuccess ) { vCNNData[i].m_bIsValid = false; }
	}
}

bool WeichiUctCNNHandler::updateNet(int batch, CNNData& cnnData, WeichiCNNNet* cnnNet)
{
	DCNNNetResult netResult;
	netResult.setColor(cnnNet->getColor(batch));

	if (cnnNet->hasSLOutput()) {
		netResult.setHasPolicyResult(cnnNet->hasSLOutput());
		netResult.setPolicyResult(cnnNet->getSLCandidates(batch));
	}

	if (cnnNet->hasVNOutput()) {
		netResult.setHasValueResult(cnnNet->hasVNOutput());
		float komi = WeichiDynamicKomi::Internal_komi;
		if (WeichiConfigure::dcnn_ftl_remaining_move_num) { komi -= cnnData.m_vPaths.size(); }
		//if (WeichiConfigure::dcnn_ftl_remaining_move_num) { komi -= (komi<cnnNet->getVNLabelSize() - 1) ? cnnData.m_vPaths.size() : 0; }
		netResult.setValueResult(cnnNet->getVNResult(batch, komi));
	}

	return updateNet(cnnData, netResult);
}

bool WeichiUctCNNHandler::updateNet(CNNData& cnnData, const DCNNNetResult& netResult)
{
	vector<UctNodePtr> vNodePaths;
	UctNodePtr pNode = selectNodeByPaths(cnnData.m_vPaths, false, true, vNodePaths);
	UctNodePtr pParent = (vNodePaths.size() >= 2) ? vNodePaths[vNodePaths.size() - 2] : UctNodePtr::NULL_PTR;

	if (pNode.isNull()) {
		// if can't find node, we should not put the job back to queue
		cnnData.m_bIsValid = false;
		releaseUpdateToken(pParent);
		removePathVirtualLoss(vNodePaths);		
		return false;
	}

	// start update node
	if (netResult.hasPolicyResult()) {
		if (!updatePolicy(netResult, pNode)) {
			releaseUpdateToken(pNode);
			releaseUpdateToken(pParent);			
			return false;
		}
	}

	if (netResult.hasValueResult()) {
		if (!updateValue(netResult, pNode, vNodePaths)) {
			releaseUpdateToken(pNode);
			releaseUpdateToken(pParent);
			return false;
		}
	}

	// update success
	WeichiGlobalInfo::getTreeInfo().m_nDCNNJob++;
	pNode->setCNNNode();
	releaseUpdateToken(pNode);
	releaseUpdateToken(pParent);
	removePathVirtualLoss(vNodePaths);

	return true;
}

bool WeichiUctCNNHandler::updatePolicy(const DCNNNetResult& netResult, UctNodePtr pNode)
{
	const vector<CandidateEntry>& vCandidates = netResult.getPolicyResult();

	if (pNode == m_pRoot) { ignoreMoves(vCandidates); }

	WeichiGlobalInfo::getTreeInfo().m_treeWidth.add(static_cast<double>(vCandidates.size()));
	if (vCandidates.size() > WeichiGlobalInfo::getTreeInfo().m_nMaxTreeWidth) {
		WeichiGlobalInfo::getTreeInfo().m_nMaxTreeWidth = static_cast<int>(vCandidates.size());
	}

	uint numChildren = (pNode->getNumChildren() > vCandidates.size()) ? pNode->getNumChildren() : static_cast<uint>(vCandidates.size());
	UctNodePtr cnnChild = m_nodeAllocator.allocateNodes(numChildren);
	if (cnnChild.isNull()) { return false; }	// no memory

	UctNodePtr newNodeTable[MAX_NUM_GRIDS];
	for (uint i = 0; i < MAX_NUM_GRIDS; i++) { newNodeTable[i] = UctNodePtr::NULL_PTR; }

	Color turnColor = m_state.m_board.getToPlay();
	UctChildIterator<WeichiUctNode> childIterator(cnnChild, numChildren);
	for (uint i = 0; i < numChildren; i++) {
		uint position = (i < vCandidates.size()) ? vCandidates[i].getPosition() : PASS_MOVE.getPosition();
		WeichiMove move(turnColor, position);

		childIterator->reset(move);
		childIterator->setProb(vCandidates[i].getScore());
		newNodeTable[move.getPosition()] = childIterator;

		++childIterator;
	}

	// copy old node information
	for (UctChildIterator<WeichiUctNode> it(pNode); it; ++it) {
		copyPolicyNode(it, newNodeTable[it->getMove().getPosition()]);
	}

	pNode->setChildren(cnnChild, numChildren);

	return true;
}

void WeichiUctCNNHandler::copyPolicyNode(UctNodePtr oldNode, UctNodePtr newNode)
{
	if (!oldNode.isValid()) { return; }

	// reuse mm node
	newNode->getUctData() = oldNode->getUctData();
	newNode->getVNValueData() = oldNode->getVNValueData();
	newNode->addVirtualLoss(oldNode->getVirtualLossCount());
	newNode->setCNNVNValue(oldNode->getCNNVNValue());
	newNode->getVNValueData() = oldNode->getVNValueData();
	if (oldNode->isCNNNode()) { newNode->setCNNNode(); }
	if (oldNode->isInCNNQueue()) { newNode->setInCNNQueue(); }
	if (oldNode->hasChildren()) { newNode->setChildren(oldNode->getChild(0), oldNode->getNumChildren()); }
}

bool WeichiUctCNNHandler::updateValue(const DCNNNetResult& netResult, UctNodePtr pNode, vector<UctNodePtr>& vNodePaths)
{
	WeichiGlobalInfo::getGlobalLock().lock();
	Color color = netResult.getColor();
	float fValueResult = netResult.getValueResult();
	if (pNode->getColor() == color) { pNode->setCNNVNValue(fValueResult); }
	else { pNode->setCNNVNValue(-fValueResult); }

	// update node path
	for (uint i = 0; i < vNodePaths.size(); i++) {			
		if (WeichiConfigure::use_playout) {
			if (vNodePaths[i]->getColor() == color) { vNodePaths[i]->getVNValueData().add(fValueResult); }
			else if (vNodePaths[i]->getColor() == AgainstColor(color)) { vNodePaths[i]->getVNValueData().add(-fValueResult); }
		} else {
			if (vNodePaths[i]->getColor() == color) { vNodePaths[i]->getUctData().add(fValueResult); }
			else if (vNodePaths[i]->getColor() == AgainstColor(color)) { vNodePaths[i]->getUctData().add(-fValueResult); }
		}		
	}

	if (m_pRoot->hasChildren()) { 
		// root has child
		UctNodePtr maxCountChild = UctNodePtr::NULL_PTR;	
		double maxCount = -1;
		for (UctChildIterator<WeichiUctNode> it(m_pRoot); it; ++it) {
			if (it->getUctDataWithoutVirtualLoss().getCount() > maxCount) {
				maxCountChild = it;
				maxCount = it->getUctDataWithoutVirtualLoss().getCount();
			}
		}
		WeichiGlobalInfo::getRootSimInfo().push_back(tuple<int,double,short>(m_pRoot->getUctDataWithoutVirtualLoss().getCount(), maxCountChild->getUctDataWithoutVirtualLoss().getMean(), maxCountChild->getPosition()));
	} 
	WeichiGlobalInfo::getGlobalLock().unlock();

	return true;
}

void WeichiUctCNNHandler::removePathVirtualLoss(vector<UctNodePtr>& vNodePaths)
{
	for (uint i = 1; i < vNodePaths.size(); i++) {
		if (!WeichiConfigure::use_virtual_loss) { return; }
		vNodePaths[i]->removeVirtualLoss(WeichiConfigure::virtual_loss_count);
	}
}

void WeichiUctCNNHandler::ignoreMoves(const vector<CandidateEntry>& vUctCandidateList)
{
	if (arguments::mode != "jlmcts") { return; }

	string sIgnores = WeichiConfigure::Ignore;
	string normalizedIgnored = WeichiJLMctsHandler::normalizePathStringFromJLMCTS(sIgnores);
	for (uint pos = 0; normalizedIgnored.find_first_of("BW", pos) != string::npos; pos += 5) {
		Color c = toColor(normalizedIgnored[pos]);
		WeichiMove m(c, normalizedIgnored.substr(pos + 2, 2));
		for (uint i = 0; i < vUctCandidateList.size(); i++) {
			if (m.getPosition() == vUctCandidateList[i].getPosition()) {
				const_cast<vector<CandidateEntry>&>(vUctCandidateList).erase(vUctCandidateList.begin()+i);
				break;
			}
		}
	}

	sort(const_cast<vector<CandidateEntry>&>(vUctCandidateList).begin(), const_cast<vector<CandidateEntry>&>(vUctCandidateList).end(), [](CandidateEntry& lhs, CandidateEntry& rhs) {
		return lhs.getScore() > rhs.getScore();
		});

	// judge it again
	if (vUctCandidateList.size() == 1) {
		WeichiGlobalInfo::get()->m_bIsLastCandidate = true;
	}

	if (vUctCandidateList.size() == 0) {
		const_cast<vector<CandidateEntry>&>(vUctCandidateList).push_back(CandidateEntry(PASS_MOVE.getPosition(), 0.00001));
		WeichiGlobalInfo::get()->m_bIsLastCandidate = true;
	}

	return;
} 
