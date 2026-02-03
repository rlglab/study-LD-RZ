#ifndef WEICHICLOSEDAREA_H
#define WEICHICLOSEDAREA_H

#include "BasicType.h"
#include "Color.h"
#include "FeatureIndexList.h"
#include "StaticBoard.h"
#include "WeichiEyeStatus.h"

class WeichiClosedArea
{
public:
	enum ClosedAreaType {
		CA_UNKNOWN,
		CA_CORNER,
		CA_EDGE,
		CA_CENTRAL
	};
private:
	uint m_id;
	Color m_color;
	uint m_numStone;
	uint m_nakadePos;
	WeichiBitBoard m_bmStone;	
	FeatureIndexList<short, MAX_GAME_LENGTH> m_blockIDs;
	WeichiLifeAndDeathStatus m_status;
	ClosedAreaType m_type;
	WeichiEyeStatus m_eyeStatus;	
	WeichiBitBoard m_bmRZone;

public:
	WeichiClosedArea() { Clear(); }

	void    Clear();
	void	init();
	inline uint    GetID () const { return m_id; }
	inline void    SetID(uint id) { m_id = id ; }

	inline Color getColor () const { return m_color; }
	inline void setColor(Color c) { m_color = c;}

	inline uint getNumStone () const { return m_numStone; }
	inline void setNumStone (uint numStone) { m_numStone=numStone ; }
	inline void decNumStone () { --m_numStone; }
	inline void incNumStone () { ++m_numStone; }

	inline void addBlockID( uint blockID ) { m_blockIDs.addFeature(blockID); }
	inline void deleteBlockID( uint blockID ) { m_blockIDs.removeFeature(blockID); }
	inline bool hasBlockID( uint blockID ) const { return m_blockIDs.contains(blockID); }
	inline uint getNumBlock() const { return m_blockIDs.getNumFeature(); }
	inline uint getBlockID( uint idx ) const { return m_blockIDs[idx]; }

	inline void setEyeStatus( WeichiEyeStatus eyeStatus ) { m_eyeStatus=eyeStatus; }
	inline WeichiEyeStatus getEyeStatus() const { return m_eyeStatus; }

	inline void clearNakade() { m_nakadePos = -1; }
	inline void setNakade( uint nakade ) { m_nakadePos = nakade; }
	inline bool hasNakade() const { return (m_nakadePos!=-1); }
	inline uint getNakade() const { return m_nakadePos; }

	inline WeichiBitBoard& getStoneMap() { return m_bmStone; }
	inline const WeichiBitBoard& getStoneMap() const { return m_bmStone; }
	inline void setStoneMap( const WeichiBitBoard& bmStone ) { m_bmStone=bmStone; }
	inline void setStoneMapOff( uint pos ) { m_bmStone.SetBitOff(pos); }
	inline WeichiBitBoard getSurroundBitBoard() const { return m_bmStone.dilate() & ~m_bmStone & StaticBoard::getMaskBorder(); }
	inline WeichiBitBoard getMinimumSurroundedBitBoard() const {return (m_bmStone.dilate() - m_bmStone) & StaticBoard::getMaskBorder() ; }

	inline void setStatus( WeichiLifeAndDeathStatus status ) { m_status=status; }
	inline WeichiLifeAndDeathStatus getStatus() const { return m_status; }

	inline void setRZone( WeichiBitBoard bmRZone ) { m_bmRZone = bmRZone ; }
	inline void addRZone( WeichiBitBoard bmRZone ) { m_bmRZone |= bmRZone ; } 
	inline WeichiBitBoard getRZone() const { return m_bmRZone ; }	

	inline void calculateCAType()
	{
		if( !(m_bmStone&StaticBoard::getCornerBitBoard()).empty() ) { m_type = CA_CORNER; }
		else if( !(m_bmStone&StaticBoard::getEdgeBitBoard()).empty() ) { m_type = CA_EDGE; }
		else { m_type = CA_CENTRAL; }
	}
	inline ClosedAreaType getType() const { return m_type; }

	inline void merge( const WeichiClosedArea* mergeCA )
	{
		assert( mergeCA->getColor()==m_color );

		m_numStone += mergeCA->getNumStone();
		m_bmStone |= mergeCA->getStoneMap();
		for( uint iNum=0; iNum<mergeCA->getNumBlock(); iNum++ ) {
			const uint iID = mergeCA->getBlockID(iNum);
			if( !hasBlockID(iID) ) { addBlockID(iID); }
		}
	}

	inline void removePoint( uint pos )
	{
		assert( m_bmStone.BitIsOn(pos) );

		m_bmStone.SetBitOff(pos);
		--m_numStone;
		calculateCAType();
	}
	inline void addPoint( uint pos )
	{
		assert( !m_bmStone.BitIsOn(pos) );

		m_bmStone.SetBitOn(pos);
		++m_numStone;
		calculateCAType();
	}
};

#endif