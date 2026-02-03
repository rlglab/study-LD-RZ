#ifndef WEICHIEDGEHANDLER_H
#define WEICHIEDGEHANDLER_H

#include "BasicType.h"
#include "FeatureList.h"
#include "WeichiEdge.h"
#include "WeichiGrid.h"

class WeichiBoard;

class WeichiEdgeHandler
{
	friend class WeichiBoard;
private:
	WeichiBoard *m_board;
	FeatureList<WeichiEdge, MAX_NUM_GRIDS> m_edges;
public:
	WeichiEdgeHandler() { clear(); }
	inline void initialize( WeichiBoard* board ) { m_board = board; }
	inline void clear() { m_edges.reset(); }

	void findFullBoardEdges();
	void removeAllEdges();

private:
	void setEachEdgeBits( WeichiGrid& grid, WeichiEdgeBits edgeBits );
};

#endif