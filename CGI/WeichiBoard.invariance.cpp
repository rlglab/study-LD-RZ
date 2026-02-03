#include "WeichiBoard.h"

bool WeichiBoard::invariance() const
{
	assertToFile( invariance_checkFullBoard(), this);
	assertToFile( invariance_checkGrid(), this);
	assertToFile( invariance_checkBlock(), this);
	assertToFile( invariance_checkFullBoardRadiusPattern(), this);
	if( WeichiConfigure::use_closed_area ) { assertToFile( m_closedAreaHandler.invariance(), this); }
	if( WeichiConfigure::use_probability_playout && !isPlayLightStatus() ) { assertToFile( m_probabilityHandler.invariance(), this); }
	//assertToFile( m_regionHandler.invariance(), this );

	return true;
}

bool WeichiBoard::invariance_checkFullBoard() const
{
	assertToFile( ColorNotEmpty(m_status.m_colorToPlay), this );
	assertToFile( m_moves.size()+m_preset.size()==m_moveStacks.size(), this );

	// check candidate
	if( isPlayLightStatus() ) { return true; }
	for( uint pos=0; pos<m_candidates.size(); pos++ ) {
		assertToFile( getGrid(m_candidates[pos]).isEmpty(), this );
	}

	return true;
}

bool WeichiBoard::invariance_checkGrid() const
{
	// all non-empty grid must have pointer to block which contains this grid
	HashKey64 boardHash;
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		const WeichiGrid& grid = getGrid(*it);

		// check grid & board bit-board
		if( grid.isEmpty() ) {
			assertToFile( !m_bmBoard.BitIsOn(*it), this );
			assertToFile( !m_bmStone.get(COLOR_BLACK).BitIsOn(*it), this );
			assertToFile( !m_bmStone.get(COLOR_WHITE).BitIsOn(*it), this );
			continue;
		}
		
		Color myColor = grid.getColor();
		Color oppColor = AgainstColor(myColor);
		assertToFile( m_bmBoard.BitIsOn(*it), this );
		assertToFile( m_bmStone.get(myColor).BitIsOn(*it), this );
		assertToFile( !m_bmStone.get(oppColor).BitIsOn(*it), this );

		// check pattern index 
		WeichiMove m(COLOR_NONE, grid.getPosition() ) ;
		uint m_x = m.x() ;
		uint m_y = m.y();

		// Check if the pattern index of every grid is correct
		int gridPatternIndex = 0 ;				

		if( m_x==0 && m_y==0 ) { gridPatternIndex = pattern33::START_ADDR_THREE_LD; }
		else if( m_x==WeichiConfigure::BoardSize-1 && m_y==0 ) { gridPatternIndex = pattern33::START_ADDR_THREE_RD; }
		else if( m_x==0 && m_y==WeichiConfigure::BoardSize-1 ) { gridPatternIndex = pattern33::START_ADDR_THREE_LU; }
		else if( m_x==WeichiConfigure::BoardSize-1 && m_y==WeichiConfigure::BoardSize-1 ) { gridPatternIndex = pattern33::START_ADDR_THREE_RU; }
		else if( m_x==0 ) { gridPatternIndex = pattern33::START_ADDR_FIVE_LEFT; }
		else if( m_x==WeichiConfigure::BoardSize-1 ) { gridPatternIndex = pattern33::START_ADDR_FIVE_RIGHT; }
		else if( m_y==0 ) { gridPatternIndex = pattern33::START_ADDR_FIVE_DOWN; }
		else if( m_y==WeichiConfigure::BoardSize-1 ) { gridPatternIndex = pattern33::START_ADDR_FIVE_UPPER; }
		else { gridPatternIndex = pattern33::START_ADDR_EIGHT ; }
		
		const int* nbr33 = grid.getStaticGrid().get3x3Neighbors();
		const int* rel33 = grid.getStaticGrid().get3x3Relations();

		int reverse_rel;

		for( ; *nbr33!=-1 ; nbr33++, rel33++ ) {
			const WeichiGrid& neighborGrid = getGrid(*nbr33) ;
			reverse_rel = StaticBoard::getRevDirect(*rel33);
			if( !neighborGrid.isEmpty() ) {
				const int* indexDiff = neighborGrid.getStaticGrid().getPatternIndexDifference(neighborGrid.getColor());
				gridPatternIndex += ( indexDiff[reverse_rel] ) ; 
			}
		}
		assertToFile( gridPatternIndex==grid.getPatternIndex(), this );

		// check grid liberty & grid pointer to block & block bit-board
		const WeichiBlock* block = grid.getBlock();
		assertToFile( block!=NULL, this );
		assertToFile( block->getStoneMap().BitIsOn(*it), this );

		uint liberty = 0;
		const int *iNbr = grid.getStaticGrid().getAdjacentNeighbors();
		for( ; *iNbr!=-1; iNbr++ ) {
			assertToFile( block->getStonenNbrMap().BitIsOn(*it), this );
			const WeichiGrid& nbrGrid = getGrid(*iNbr);
			if( nbrGrid.isEmpty() ) { ++liberty; }
		}
		
		assertToFile( liberty==grid.getLiberty(), this );
		
		// check board hash key
		boardHash ^= grid.getStaticGrid().getHash(grid.getColor());
	}

	// odd number move should XOR turn key
	if( m_moves.size()%2!=0 ) {
		boardHash ^= StaticBoard::getHashGenerator().getTurnKey();
	}

	assertToFile( boardHash==m_hash, this );

	return true;
}

bool WeichiBoard::invariance_checkBlock() const
{
	HashKey64 boardHash;
	Dual<WeichiBitBoard> bmBoardStone;
	Dual<WeichiBitBoard> bmOneLibBlocks;
	Dual<WeichiBitBoard> bmTwoLibBlocks;

	for( uint i=0; i<m_blockList.getCapacity(); i++ ) {
		if( !m_blockList.isValidIdx(i) ) { continue; }
		const WeichiBlock* block = m_blockList.getAt(i);

		assertToFile( block->valid(), this );

		if( !block->isUsed() ) { continue; }

		// check board bit-board
		Color blkColor = block->getColor();
		bmBoardStone.get(blkColor) |= block->getStoneMap();

		// check all grid in it
		uint pos;
		uint numStone = 0;
		HashKey64 blockHash;
		WeichiBitBoard bmStone = block->getStoneMap();
		WeichiBitBoard bmStoneNbr;
		BoundingBox box;
		box.setLowestPosition(block->getiFirstGrid());
		while( (pos=bmStone.bitScanForward())!=-1 ) {
			const WeichiGrid& grid = getGrid(pos);
			
			assertToFile( grid.getBlock()==block, this );
			assertToFile( grid.getColor()==blkColor, this );

			box.combine(pos);
			blockHash ^= grid.getStaticGrid().getHash(grid.getColor());
			bmStoneNbr |= grid.getStaticGrid().getStoneNbrsMap();
			numStone++;
		}

		assertToFile( block->getStoneMap().BitIsOn(block->getiFirstGrid()), this );
		assertToFile( numStone==block->getNumStone(), this );
		assertToFile( bmStoneNbr==block->getStonenNbrMap(), this );
		assertToFile( blockHash==block->getHash(), this );
		if( !isPlayLightStatus() ) {
			assertToFile( box.getUpBound()==block->getBoundingBox().getUpBound(), this );
			assertToFile( box.getDownBound()==block->getBoundingBox().getDownBound(), this );
			assertToFile( box.getLeftBound()==block->getBoundingBox().getLeftBound(), this );
			assertToFile( box.getRightBound()==block->getBoundingBox().getRightBound(), this );
			assertToFile( box.getLowestPosition()==block->getBoundingBox().getLowestPosition(), this );
		}

		assertToFile( block->getLiberty()==block->getLibertyExclude(getBitBoard()), this );
		if( block->getLiberty()==1 ) { bmOneLibBlocks.get(blkColor) |= block->getStoneMap(); }
		else if( block->getLiberty()==2 ) { bmTwoLibBlocks.get(blkColor) |= block->getStoneMap(); }

		// check one liberty block liberty pos
		if( block->getLiberty()==1 ) {
			if (block->getLastLibertyPos() != block->getLastLiberty(m_bmBoard)) {
				CERR() << "Error" << endl;
				CERR() << block->getLastLibertyPos() << endl;
				CERR() << block->getLastLiberty(m_bmBoard) << endl;
			}
			assertToFile( block->getLastLibertyPos()==block->getLastLiberty(m_bmBoard), this );
		}

		// check board hash key
		boardHash ^= block->getHash();
	}

	// odd number move should XOR turn key
	if( m_moves.size()%2!=0 ) {
		boardHash ^= StaticBoard::getHashGenerator().getTurnKey();
	}

	assertToFile( boardHash==m_hash, this );
	assertToFile( bmBoardStone.get(COLOR_BLACK)==m_bmStone.get(COLOR_BLACK), this );
	assertToFile( bmBoardStone.get(COLOR_WHITE)==m_bmStone.get(COLOR_WHITE), this );

	// check liberty block list
	assertToFile( bmOneLibBlocks.get(COLOR_BLACK)==m_bmOneLibBlocks.get(COLOR_BLACK), this );
	assertToFile( bmOneLibBlocks.get(COLOR_WHITE)==m_bmOneLibBlocks.get(COLOR_WHITE), this );
	assertToFile( bmTwoLibBlocks.get(COLOR_BLACK)==m_bmTwoLibBlocks.get(COLOR_BLACK), this );
	assertToFile( bmTwoLibBlocks.get(COLOR_WHITE)==m_bmTwoLibBlocks.get(COLOR_WHITE), this );

	return true;
}

bool WeichiBoard::invariance_checkLibertyList( const FeaturePtrList<WeichiBlock*, MAX_GAME_LENGTH>& libBlockList, Color color, uint liberty ) const
{
	for( uint i=0; i<libBlockList.size(); i++ ) {
		WeichiBlock* block = libBlockList[i];
		assertToFile( block->valid(), this );
		assertToFile( block->isUsed(), this );
		assertToFile( block->getColor()==color, this );
		assertToFile( block->getLiberty()==liberty, this );
	}

	return true;
}

bool WeichiBoard::invariance_checkFullBoardRadiusPattern() const
{
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		const WeichiGrid& grid = getGrid(*it);
		assertToFile( invariance_checkEachGridLibertyPattern(grid), this );
		assertToFile( invariance_checkEachGridRadiusPattern(grid), this );
	}

	return true;
}

bool WeichiBoard::invariance_checkEachGridRadiusPattern( const WeichiGrid& grid ) const
{
	if( isPlayLightStatus() ) { return true; }

	int iPatternIndex[MAX_RADIUS_SIZE];
	const StaticGrid::RadiusGridRelation* radiusGridRelation = grid.getStaticGrid().getRadiusGridRelations();

	for( uint iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		iPatternIndex[iRadius] = grid.getStaticGrid().getInitialRadiusPattern(iRadius);
	}

	for( ; radiusGridRelation->m_iRadius!=-1; radiusGridRelation++ ) {
		const WeichiGrid& radiusGrid = getGrid(radiusGridRelation->m_neighborPos);
		if( radiusGrid.isEmpty() ) { continue; }

		Color color = radiusGrid.getColor();
		uint iOffsetIndex = radiusGridRelation->m_iOffsetIndex;
		uint iRadius = radiusGridRelation->m_iRadius;
		iPatternIndex[iRadius] += WeichiRadiusPatternTable::getRadiusPatternIndexOffset(iOffsetIndex,color);
	}

	for( uint iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		if( isPlayoutStatus() && iRadius>3 ) { break; }

		int iIndex = grid.getRadiusPatternRealIndex(iRadius);
		assertToFile( iIndex==iPatternIndex[iRadius], this );
	}

	return true;
}

bool WeichiBoard::invariance_checkEachGridLibertyPattern( const WeichiGrid& grid ) const
{
	if( isPlayLightStatus() ) { return true; }

	if( !grid.isEmpty() ) { return true; }

	uint iNbrLibIndex = 0;
	const StaticGrid& sgrid = grid.getStaticGrid();

	for( uint dir=0; dir<4; dir++ ) {
		if( sgrid.getNeighbor(dir)==-1 ) { continue; }
		if( getGrid(grid,dir).getBlock()==NULL ) { continue; }
		
		if( getGrid(grid,dir).getBlock()->getLiberty()>2 ) { continue; }
		else if( getGrid(grid,dir).getBlock()->getLiberty()==2 ) { iNbrLibIndex |= (2<<(dir*RADIUS_NBR_LIB_BITS)); }
		else if( getGrid(grid,dir).getBlock()->getLiberty()==1 ) { iNbrLibIndex |= (1<<(dir*RADIUS_NBR_LIB_BITS)); }
	}

	assertToFile( grid.getNbrLibIndex()==iNbrLibIndex, this );

	return true;
}