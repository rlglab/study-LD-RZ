#ifndef WEICHIPROBABILITYPLAYOUTHANDLER_H
#define WEICHIPROBABILITYPLAYOUTHANDLER_H

#include "BasicType.h"
#include "Dual.h"
#include "WeichiGrid.h"
#include "FeatureIndexList.h"
#include "WeichiProbabilityTable.h"

class WeichiProbabilityPlayoutHandler
{
	friend class WeichiBoard;
private:
	WeichiBoard* m_board;
	Dual<WeichiProbabilityTable> m_probTable;
	FeatureIndexList<uint,MAX_NUM_GRIDS> m_modifyIndexTable;
	FeatureIndexList<uint,MAX_NUM_GRIDS> m_updatePatternTable;

public:
	WeichiProbabilityPlayoutHandler() {}

	void reset();
	uint chooseBySoftMax(Color turnColor, const WeichiBitBoard& bmStone) const { return m_probTable.get(turnColor).chooseOneMoveBySoftMax(bmStone); }
	inline void initialize( WeichiBoard* board ) { m_board = board; }
	inline WeichiProbabilityTable& getProbabilityTable( Color turnColor ) { return m_probTable.get(turnColor); }
	inline const WeichiProbabilityTable& getProbabilityTable( Color turnColor ) const { return m_probTable.get(turnColor); }
	inline const Vector<ProbabilityGrid,MAX_NUM_GRIDS>& getProbGrid( Color turnColor ) const { return m_probTable.get(turnColor).getAllProbGrid(); }

	void initailizeTable();
	void setUpdatePattern( int position );
	void updatePattern();
	void putStone( const uint position );
	void skipPosition( const uint position, Color turnColor );
	void addAdditionalFeature( const uint position, Color turnColor, const uint iFeatureIndex );
	void recover( Color turnColor );
	void removeLifeCandidates( WeichiClosedArea* closedArea );

private:
	void setPatternScore( const WeichiGrid& grid, Color turnColor );
	void putStone( const uint position, Color turnColor );
	void changeTableScore( const uint position, Color turnColor, const double dDiffScore );

	bool invariance() const;
	bool invariance_checkFullBoardProbilityGrid( Color checkColor ) const;
};

#endif