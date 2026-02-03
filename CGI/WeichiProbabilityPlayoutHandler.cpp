#include "WeichiProbabilityPlayoutHandler.h"
#include "WeichiBoard.h"

void WeichiProbabilityPlayoutHandler::reset()
{
	m_probTable.m_black.reset();
	m_probTable.m_white.reset();
	m_modifyIndexTable.clear();
	m_updatePatternTable.clear();
}

void WeichiProbabilityPlayoutHandler::initailizeTable()
{
	reset();

	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		const WeichiGrid& grid = m_board->getGrid(*it);
		if( !grid.isEmpty() ) { continue; }

		setPatternScore(grid,COLOR_BLACK);
		setPatternScore(grid,COLOR_WHITE);
	}

	assertToFile( invariance(), m_board );
}

void WeichiProbabilityPlayoutHandler::setUpdatePattern( int position )
{
	if( m_updatePatternTable.contains(position) ) { return; }
	m_updatePatternTable.addFeature(position);
}

void WeichiProbabilityPlayoutHandler::updatePattern()
{
	for( uint iIndex=0; iIndex<m_updatePatternTable.size(); iIndex++ ) {
		const uint position = m_updatePatternTable[iIndex];
		const WeichiGrid& grid = m_board->getGrid(position);

		if( !grid.isEmpty() ) { continue; }
		setPatternScore(grid,COLOR_BLACK);
		setPatternScore(grid,COLOR_WHITE);
	}
	m_updatePatternTable.clear();
}

void WeichiProbabilityPlayoutHandler::putStone( uint position )
{
	putStone(position,COLOR_BLACK);
	putStone(position,COLOR_WHITE);
	
	assertToFile( invariance(), m_board );
}

void WeichiProbabilityPlayoutHandler::skipPosition( const uint position, Color turnColor )
{
	if( !m_modifyIndexTable.contains(position) ) { m_modifyIndexTable.addFeature(position); }

	WeichiProbabilityTable& table = m_probTable.get(turnColor);
	ProbabilityGrid& probGrid = table.getProbGrid(position);

	const double dOldScore = probGrid.getScore();
	probGrid.setIsUsed(false);

	double dDiffScore = 0 - dOldScore;
	changeTableScore(position,turnColor,dDiffScore);
}

void WeichiProbabilityPlayoutHandler::addAdditionalFeature( const uint position, Color turnColor, const uint iFeatureIndex )
{
	ProbabilityGrid& probGrid = m_probTable.get(turnColor).getProbGrid(position);
	if( probGrid.isSkipGrid() ) { return; }

	bool bIsFirstTimeAddFeature = !m_modifyIndexTable.contains(position);
	if( bIsFirstTimeAddFeature ) { m_modifyIndexTable.addFeature(position); }

	const int iIndex = iFeatureIndex + 2*MAX_3X3PATTERN_SIZE;
	const double dScore = WeichiGammaTable::getPlayoutFeatureValue(iIndex);
	if( probGrid.getPlayoutFeatureBits().BitIsOn(iFeatureIndex) ) { return; }

	const double dOldScore = probGrid.getScore();
	probGrid.addAdditionalFeature(iFeatureIndex,dScore);
	if( bIsFirstTimeAddFeature ) {
		double dLocalPatternScore = WeichiGammaTable::getPlayoutFeatureValue(probGrid.getPatternIndex()+MAX_3X3PATTERN_SIZE);
		probGrid.addAdditionalPatternFeature(dLocalPatternScore);
		double dLocalFeatureScore = WeichiGammaTable::getPlayoutFeatureValue(POLICY_REPLY+2*MAX_3X3PATTERN_SIZE);
		probGrid.addAdditionalFeature(POLICY_REPLY,dLocalFeatureScore);
	}
	const double dDiffScore = probGrid.getScore() - dOldScore;
	changeTableScore(position,turnColor,dDiffScore);

	WeichiProbabilityTable& table = m_probTable.get(turnColor);
	table.replaceMaxScore(probGrid.getScore());

	assertToFile( invariance(), m_board );
}

void WeichiProbabilityPlayoutHandler::recover( Color turnColor )
{
	WeichiProbabilityTable& table = m_probTable.get(turnColor);
	for( uint iIndex=0; iIndex<m_modifyIndexTable.size(); iIndex++ ) {
		const uint position = m_modifyIndexTable[iIndex];
		ProbabilityGrid& probGrid = table.getProbGrid(position);

		const double dOldScore = probGrid.isUsed()? probGrid.getScore(): 0.0f;
		probGrid.recover(turnColor);
		const double dDiffScore = probGrid.getScore() - dOldScore;

		changeTableScore(position,turnColor,dDiffScore);
	}
	m_modifyIndexTable.clear();

	assertToFile( invariance(), m_board );
}

void WeichiProbabilityPlayoutHandler::removeLifeCandidates( WeichiClosedArea* closedArea )
{
	uint pos;
	WeichiBitBoard bmCAStone = closedArea->getStoneMap()-m_board->getBitBoard();
	while( (pos=bmCAStone.bitScanForward())!=-1 ) {
		putStone(pos,COLOR_BLACK);
		putStone(pos,COLOR_WHITE);
		m_probTable.get(COLOR_BLACK).getProbGrid(pos).setIsSkipGird(true);
		m_probTable.get(COLOR_WHITE).getProbGrid(pos).setIsSkipGird(true);
	}

	assertToFile( invariance(), m_board );
}

void WeichiProbabilityPlayoutHandler::setPatternScore( const WeichiGrid& grid, Color turnColor )
{
	assertToFile( grid.isEmpty(), m_board );

	const int position = grid.getPosition();
	ProbabilityGrid& probGrid = m_probTable.get(turnColor).getProbGrid(position);
	if( probGrid.isSkipGrid() ) { return; }

	const int iPatternIndex = (grid.getRadiusPatternRealIndex(3)<<(RADIUS_NBR_LIB_SIZE*RADIUS_NBR_LIB_BITS)) + grid.getNbrLibIndex();
	const int iFeatureIndex = WeichiRadiusPatternTable::radius3SymmetricIndex.get(turnColor)[iPatternIndex];
	/*if( iFeatureIndex==2050 ) {
		m_board->showColorBoard();
		cerr << WeichiMove(grid.getPosition()).toGtpString() << endl;
		cerr << toChar(turnColor) << endl;
		int k;
		cin >> k;
	}*/

	const double dPatternScore = WeichiGammaTable::getPlayoutFeatureValue(iFeatureIndex);
	const double dOldScore = probGrid.getScore();
	probGrid.setPatternScore(iFeatureIndex,dPatternScore);
	const double dNewScore = probGrid.getScore();

	double dDiffScore = dNewScore - dOldScore;
	changeTableScore(position,turnColor,dDiffScore);
}

void WeichiProbabilityPlayoutHandler::putStone( const uint position, Color turnColor )
{
	WeichiProbabilityTable& table = m_probTable.get(turnColor);
	ProbabilityGrid& probGrid = table.getProbGrid(position);

	const double dOldScore = probGrid.getScore();
	probGrid.clear();
	double dDiffScore = probGrid.getScore() - dOldScore;

	changeTableScore(position,turnColor,dDiffScore);
}

void WeichiProbabilityPlayoutHandler::changeTableScore( const uint position, Color turnColor, const double dDiffScore )
{
	WeichiProbabilityTable& table = m_probTable.get(turnColor);
	int row = WeichiMove(position).y();

	table.setRowScore(row,table.getRowScore(row)+dDiffScore);
	table.setTotalScore(table.getTotalScore()+dDiffScore);
}

bool WeichiProbabilityPlayoutHandler::invariance() const
{
	assertToFile( m_probTable.get(COLOR_BLACK).invariance(), m_board );
	assertToFile( m_probTable.get(COLOR_WHITE).invariance(), m_board );
	assertToFile( invariance_checkFullBoardProbilityGrid(COLOR_BLACK), m_board );
	assertToFile( invariance_checkFullBoardProbilityGrid(COLOR_WHITE), m_board );

	return true;
}

bool WeichiProbabilityPlayoutHandler::invariance_checkFullBoardProbilityGrid( Color checkColor ) const
{
	const WeichiProbabilityTable& table = m_probTable.get(checkColor);
	double dTotalScore = 0.0f;
	double dRowScore = 0.0f;
	const double dError = 0.00001;

	for( uint row=0; row<WeichiConfigure::BoardSize; row++ ) {
		dRowScore = 0.0f;
		for( uint x=0; x<WeichiConfigure::BoardSize; x++ ) {
			const uint position = row*MAX_BOARD_SIZE + x;
			const WeichiGrid& grid = m_board->getGrid(position);
			const ProbabilityGrid& probGrid = table.getProbGrid(position);

			if( !probGrid.isUsed() || !grid.isEmpty() ) { continue; }

			Vector<uint,MAX_NUM_PLAYOUT_FEATURE_GROUPS> vFeatureIndex = probGrid.getFeatureIndexs();
			const int iPatternIndex = (grid.getRadiusPatternRealIndex(3)<<(RADIUS_NBR_LIB_SIZE*RADIUS_NBR_LIB_BITS))+grid.getNbrLibIndex();
			const int iPatternMinIndex = WeichiRadiusPatternTable::radius3SymmetricIndex.get(checkColor)[iPatternIndex];
			assertToFile( probGrid.getPatternIndex()==iPatternMinIndex, m_board );

			double dScore = 1.0f;
			for( uint i=0; i<vFeatureIndex.size(); i++ ) {
				const int iIndex = vFeatureIndex[i];
				dScore *= WeichiGammaTable::getPlayoutFeatureValue(iIndex);
			}

			assertToFile( fabs(probGrid.getScore()-dScore)<dError, m_board );
			
			dRowScore += dScore;
			dTotalScore += dScore;
		}

		assertToFile( fabs(dRowScore-table.getRowScore(row))<dError, m_board );
	}

	assertToFile( fabs(dTotalScore-table.getTotalScore())<dError, m_board );

	return true;
}