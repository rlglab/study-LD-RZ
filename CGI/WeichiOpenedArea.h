#ifndef H_WEICHI_OPENED_AREA
#define H_WEICHI_OPENED_AREA

#include "BasicType.h"
#include "Color.h"
#include "FeatureIndexList.h"
#include "WeichiEyeStatus.h"

class WeichiOpenedArea
{
public:
	WeichiOpenedArea() { Clear() ; } 

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
	
	inline void setEyeStatus( WeichiEyeStatus eyeStatus ) { m_eyeStatus=eyeStatus ; }
	inline WeichiEyeStatus getEyeStatus() const { return m_eyeStatus ; }

	inline WeichiBitBoard& getStoneMap() { return m_bmStone; }
	inline const WeichiBitBoard& getStoneMap() const { return m_bmStone; }
	inline void setStoneMap( const WeichiBitBoard& bmStone ) { m_bmStone=bmStone; }

	inline void setRZone( WeichiBitBoard bmRZone ) { m_bmRZone = bmRZone ; }
	inline void addRZone( WeichiBitBoard bmRZone ) { m_bmRZone |= bmRZone ; } 
	inline WeichiBitBoard getRZone() const { return m_bmRZone ; }	


private:
	uint m_id;
	Color m_color;
	uint m_numStone;
	WeichiBitBoard m_bmStone;	
	FeatureIndexList<short, MAX_GAME_LENGTH> m_blockIDs;
	WeichiEyeStatus m_eyeStatus ;	
	WeichiBitBoard m_bmRZone ;
};


#endif