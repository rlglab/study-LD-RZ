#include "WeichiLocalSearchHandler.h"

void WeichiLocalSearchHandler::reset()
{
	m_localSequence.reset();
}

void WeichiLocalSearchHandler::findFullBoardBlockLocalSequence()
{
	reset();

	FeatureList<WeichiBlock,MAX_GAME_LENGTH>& blockLists = m_board.getBlockList();
	for( uint iIndex=0; iIndex<blockLists.getCapacity(); iIndex++ ) {
		if( !blockLists.isValidIdx(iIndex) ) { continue; }

		WeichiBlock* block = blockLists.getAt(iIndex);
		if( !block->isUsed() ) { continue; }
		if( block->getStatus()==LAD_LIFE ) { continue; }

		if( block->getLiberty()<=2 ) { findLowLibertyBlockLocalSequence(block); }
		//else { findSemeaiBlockLocalSequence(block); }
	}
}

void WeichiLocalSearchHandler::findSemeaiBlockLocalSequence( WeichiBlock* block )
{
	const Color ownColor = block->getColor();
	const Color oppColor = AgainstColor(ownColor);
	if( (block->getStonenNbrMap()&m_board.getStoneBitBoard(oppColor)).empty() ) { return; }

	WeichiBitBoard bmLiberty1 = block->getLibertyBitBoard(m_board.getBitBoard());
	WeichiBitBoard bmLiberty1Dilate = (bmLiberty1.dilate()-block->getStonenNbrMap()-m_board.getStoneBitBoard(oppColor)) & StaticBoard::getMaskBorder();

	if( !bmLiberty1Dilate.empty() ) { return; }

	/*m_board.showColorBoard();
	cerr << toChar(ownColor) << ' ' << WeichiMove(block->getiFirstGrid()).toGtpString() << endl;

	int k;
	cin >> k;*/
}

void WeichiLocalSearchHandler::findLowLibertyBlockLocalSequence( WeichiBlock* block )
{
	m_localSearch.startLocalSearch();

	WeichiLocalSearchNode* rootNode = NULL;
	WeichiLocalSequence* localSequence = NULL;

	// kill sequence
	m_localSearch.reset();
	m_localSearch.setRootFindAllSolution(true);
	rootNode = m_localSearch.getSearchTreeRootNode();
	rootNode->setResult(m_localSearch.canKill(block,rootNode));
	localSequence = m_localSequence.NewOne();
	localSequence->init(rootNode);
	block->setKillLocalSequence(localSequence);
	/*//uint killPos = m_localSequence.findSequenceFirstMovePos(rootNode);
	if( block->getLiberty()==1 && result==RESULT_SUCCESS && killPos==-1 ) {
		killPos = block->getLastLiberty(m_board.getBitBoard());
	}
	block->setKillPosition(killPos);
	//m_localSequence.saveLocalSequence(rootNode);*/

	// save sequence
	m_localSearch.reset();
	m_localSearch.setRootFindAllSolution(true);
	rootNode = m_localSearch.getSearchTreeRootNode();
	rootNode->setResult(m_localSearch.canSave(block,rootNode));
	localSequence = m_localSequence.NewOne();
	localSequence->init(rootNode);
	block->setSaveLocalSequence(localSequence);

	m_localSearch.endLocalSearch();
}