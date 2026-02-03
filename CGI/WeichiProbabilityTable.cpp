#include "WeichiProbabilityTable.h"
#include "StaticBoard.h"

void WeichiProbabilityTable::reset()
{
	m_dMaxScore = 1.0f;
	m_dTotalScore = 0.0f;
	m_vRowScore.setAllAs(0.0f,MAX_BOARD_SIZE);
	m_vProbGrids.resize(MAX_NUM_GRIDS);
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		m_vProbGrids[*it].clear();
	}
}

int WeichiProbabilityTable::chooseOneMoveBySoftMax(const WeichiBitBoard& bmStone) const
{
	double dTotalScore = getTotalScore() / getMaxScore();
	double dRandom = Random::nextReal(dTotalScore);

	// find row
	int y;
	for( y=WeichiConfigure::BoardSize-1; y>=0; y-- ) {
		const double dRowScore = m_vRowScore[y] / getMaxScore();
		if( dRandom-dRowScore<0 ) { break; }
		dRandom -= dRowScore;
	}
	if( y<0 ) { y = 0; }

	// find column
	uint x;
	for( x=0; x<WeichiConfigure::BoardSize; x++ ) {
		const uint position = WeichiMove::toPosition(x,y);
		const ProbabilityGrid& probGrid = m_vProbGrids[position];
		if (!probGrid.isUsed() || probGrid.isSkipGrid()) { continue; }
		if (bmStone.BitIsOn(position) || probGrid.getPatternIndex() == MAX_LEGAL_3X3PATTERN_SIZE) { continue; }
		
		const double dScore = probGrid.getScore() / getMaxScore();
		if( dRandom-dScore<0 ) { return position; }
		dRandom -= dScore;
	}

	return PASS_MOVE.getPosition();
}

bool WeichiProbabilityTable::invariance() const
{
	/*int maxScale = 0;
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		const ProbabilityGrid& probGrid = m_vProbGrids[*it];
		if( !probGrid.isUsed() ) { continue; }

		const double dTheta = probGrid.getTheta();
		const int scale = getThetaScale(dTheta);
		if( scale>maxScale ) { maxScale = scale; }
	}

	int numCandidate = 0;
	assert( m_maxSacle==maxScale );
	for( uint iScale=0; iScale<PROBABILITY_THETA_SCALE; iScale++ ) {
		const ProbabilityScale& scale = m_vScale[iScale];
		numCandidate += scale.getNumCandidate();
		assert( m_vScale[iScale].invariance(m_vProbGrids) );
	}
	assert( numCandidate==m_bmCandidate.bitCount() );*/

	return true;
}