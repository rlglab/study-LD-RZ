#ifndef WEICHIBLOCK_H
#define WEICHIBLOCK_H

#include "BasicType.h"
#include "WeichiGrid.h" 
#include "FeaturePtrList.h"
#include "FeatureList.h"
#include "BoundingBox.h"
#include "WeichiClosedArea.h"
#include "WeichiLocalSequence.h"

class WeichiBlock
{
	friend class WeichiBoard ;
public:
	enum BlockStatus {
		BLK_UNKNOWN,
		BLK_NOT_LIFE,
		BLK_SENTE_LIFE,
		BLK_GOTE_LIFE,
	};
private:
	bool	m_bIsUsed;
	Color	m_color ;
	uint	m_id ;
	uint	m_liberty ;
	uint	m_numStone ;
	uint	m_iFirstGrid ;
	uint	m_modifyMoveNumber ;
	uint	m_lastLibertyPosition ;
	WeichiLifeAndDeathStatus m_LADStatus;
	WeichiLocalSequence* m_saveSequence;
	WeichiLocalSequence* m_killSequence;
	WeichiBitBoard m_bmStone ;
	WeichiBitBoard m_bmStonenNbrs ;
	HashKey64   m_hash ;
	BoundingBox m_boundingBox;
	FeatureIndexList<short, MAX_NUM_CLOSEDAREA> m_closedAreas ;
	short m_dragonID, m_snakeID;

public:
	void    Clear() ;
	void    init ( WeichiGrid & grid ) ;

	inline uint    GetID () const { return m_id; }
	inline void    SetID(uint id) { m_id = id ; }

	inline Color   getColor () const { return m_color ; }
	inline void    setColor (Color c) { m_color = c ; }

	inline uint    getNumStone () const { return m_numStone; }
	inline void    setNumStone (uint n) { m_numStone = n ; }
	inline void    incNumStone () { ++m_numStone ; }

	inline WeichiLifeAndDeathStatus getStatus() const { return m_LADStatus; }
	inline void    setStatus(WeichiLifeAndDeathStatus status) { m_LADStatus = status; }

	inline uint    getLiberty() const { return m_liberty; }
	inline void    incLiberty() { ++m_liberty ; }
	inline void    decLiberty() { --m_liberty ; }

	inline uint    getModifyMoveNumber () const { return m_modifyMoveNumber; }
	inline void    setModifyMoveNumber (uint modifyMoveNumber) { m_modifyMoveNumber = modifyMoveNumber; }

	inline WeichiLocalSequence* getSaveLocalSequence() const { return m_saveSequence; }
	inline void setSaveLocalSequence( WeichiLocalSequence* localSequence ) { m_saveSequence = localSequence; }

	inline WeichiLocalSequence* getKillLocalSequence() const { return m_killSequence; }
	inline void setKillLocalSequence( WeichiLocalSequence* localSequence ) { m_killSequence = localSequence; }

	inline void    setIsUsed( bool bIsUsed ) { m_bIsUsed = bIsUsed; }
	inline bool    isUsed() const { return m_bIsUsed; }

	inline WeichiBitBoard& getStoneMap() { return m_bmStone; }
    
	inline const WeichiBitBoard& getStoneMap() const { return m_bmStone; }
	inline const WeichiBitBoard& getStonenNbrMap() const { return m_bmStonenNbrs; }
	inline const WeichiBitBoard getNbrMap() const { return (m_bmStonenNbrs-m_bmStone) ;}

	inline HashKey64 getHash () const { return m_hash; }
	inline uint getiFirstGrid () const { return m_iFirstGrid ; }

	inline void setDragonId( short id ) { m_dragonID = id ; }
	inline short getDragonId() const { return m_dragonID ; }

	inline void setSnakeId( short id ) { m_snakeID = id ; }
	inline short getSnakeId() const { return m_snakeID ; }

	//ClosedArea
	inline uint getNumClosedArea() const { return m_closedAreas.size(); } 
	inline WeichiClosedArea* getClosedArea(uint idx, FeatureList<WeichiClosedArea, MAX_NUM_CLOSEDAREA>& closedAreas) { return closedAreas.getAt(m_closedAreas[idx]); }
	inline const WeichiClosedArea* getClosedArea(uint idx, const FeatureList<WeichiClosedArea, MAX_NUM_CLOSEDAREA>& closedAreas) const { return closedAreas.getAt(m_closedAreas[idx]); }
	inline uint getClosedAreaID( uint idx ) const { return m_closedAreas[idx]; }
	inline void addClosedArea(WeichiClosedArea* ca) ;
	inline void deleteClosedArea(WeichiClosedArea* ca) ;
	inline void deleteClosedArea(uint idx) ;
	inline bool hasClosedArea ( const WeichiClosedArea* ca ) const ;
	inline bool isInnerBlockAtClosedArea( const WeichiClosedArea* ca ) const
	{
		assert( ca && getNumClosedArea()==1 );
		return ((m_bmStonenNbrs&~ca->getStoneMap())==m_bmStone);
	}

	inline uint getLibertyExclude ( const WeichiBitBoard& bmExclude ) const 
	{
		WeichiBitBoard bm = m_bmStonenNbrs;
		bm -= bmExclude ;
		// bm -= m_bmStone ;
		return bm.bitCount();
	}

	// bounding box
	inline BoundingBox& getBoundingBox() { return m_boundingBox; }
	inline const BoundingBox& getBoundingBox() const { return m_boundingBox; }
	inline void setUpBound(uint upBound)        { m_boundingBox.setUpBound(upBound); }
	inline void setDownBound(uint downBound)    { m_boundingBox.setDownBound(downBound); }
	inline void setLeftBound(uint leftBound)    { m_boundingBox.setLeftBound(leftBound); }
	inline void setRightBound(uint rightBound)  { m_boundingBox.setRightBound(rightBound); }
	inline void setLowest(uint lowestPosition)  { m_boundingBox.setLowestPosition(lowestPosition); }
	inline uint getUpBound()    const { return m_boundingBox.getUpBound(); }
	inline uint getDownBound()  const { return m_boundingBox.getDownBound(); }
	inline uint getLeftBound()  const { return m_boundingBox.getLeftBound(); }
	inline uint getRightBound() const { return m_boundingBox.getRightBound(); }
	inline uint getLowest()     const { return m_boundingBox.getLowestPosition(); }
	inline uint getLineNo()     const { return m_boundingBox.getLineNo(); }

	inline void updateBlockBoundary( const WeichiBlock* fromBlock )
	{
		m_boundingBox.combine( fromBlock->getBoundingBox() ) ;
	}
	inline void updateBlockBoundary( const WeichiGrid& grid )
	{
		m_boundingBox.combine( grid.getPosition() ) ;
	}
	inline bool valid() const { return m_id != static_cast<uint>(-1); }

	std::string getBlockStatusString() const ;

	bool isNeighbor(const WeichiBlock* block ) const ;

	template<int _BufSize>
	inline void getLibertyPositions(const WeichiBitBoard& bmBoard, Vector<uint, _BufSize>& vLibPos) const ;

	inline void setLastLibertyPos( uint lastLibPos ) { assert( m_liberty==1 ); m_lastLibertyPosition = lastLibPos; }
	inline uint getLastLibertyPos() const { assert( m_liberty==1 ); return m_lastLibertyPosition; }
	uint getLastLiberty(const WeichiBitBoard& bmBoard) const ;

	inline void printMemberSize(){
		COUT() << "block size:\t\t"            << sizeof *this << endl;
		COUT() << endl;
	}
	WeichiBitBoard getLibertyBitBoard(const WeichiBitBoard& bmBoard) const;
private:
	/// merge block to this, called by WeichiBlockList only
	void    merge ( const WeichiBlock* block, const WeichiBitBoard& bmBoard ) ;
	void combineBlockWithoutUpdateLib( const WeichiBlock* block );
	void removeBlockWithoutUpdateLib( const WeichiBlock* block );
	inline void setStoneNbrMap( const WeichiBitBoard& bmStoneNbrMap ) { m_bmStonenNbrs = bmStoneNbrMap; }
	inline void updateLiberty( const WeichiBitBoard& bmBoard ) { m_liberty = getLibertyExclude(bmBoard); }
	void    addGrid ( const WeichiGrid& grid, const WeichiBitBoard& bmBoard ) ;
	void removeGrid( const WeichiGrid& grid, const WeichiBitBoard& bmBoard );
    
};

template<int _BufSize>
void WeichiBlock::getLibertyPositions( const WeichiBitBoard& bmBoard, Vector<uint, _BufSize>& vLibPos ) const
{
	assert(m_liberty <= _BufSize) ;
	WeichiBitBoard bmLiberty = m_bmStonenNbrs ;
	bmLiberty -= bmBoard;
	bmLiberty.bitScanAll(vLibPos) ;
}

inline void WeichiBlock::addClosedArea(WeichiClosedArea* ca) { m_closedAreas.addFeature(ca->GetID()); } 
inline void WeichiBlock::deleteClosedArea(WeichiClosedArea* ca) { m_closedAreas.removeFeature(ca->GetID()); }
inline void WeichiBlock::deleteClosedArea(uint idx) { m_closedAreas.removeFeature(idx); }
inline bool WeichiBlock::hasClosedArea( const WeichiClosedArea* ca ) const { return m_closedAreas.contains(ca->GetID()); }

#endif 
