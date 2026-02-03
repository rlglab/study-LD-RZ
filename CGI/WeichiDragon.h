#ifndef H_WEICHI_DRAGON
#define H_WEICHI_DRAGON

#include "BasicType.h"
#include "WeichiBlock.h"
#include "WeichiClosedArea.h"
#include "WeichiOpenedArea.h"
#include "Color.h"
#include "HashKey64.h"

class WeichiDragon
{

public:
	enum LifeStatus {
		LIFESTATUS_ALIVE,
		LIFESTATUS_DEAD,
		LIFESTATUS_UNKNOWN,
		
		LIFESTATUS_SIZE
	};

public:
	WeichiDragon() { reset(); }

	inline void reset() 
	{
		m_color = COLOR_NONE;
		m_status = LIFESTATUS_UNKNOWN;
		m_vBlocks.clear();
		m_vClosedAreas.clear();
		m_vOpenedAreas.clear();
		m_bmBlocks.Reset();
		m_bmClosedAreas.Reset();
		m_bmOpenedAreas.Reset();		
		m_bmConnector.Reset();
		m_bmRZone.Reset();
		m_numTrueEyes = 0;	
	}

	// setter
	inline void setColor(Color color) { m_color = color; } 
	inline void setRZone(WeichiBitBoard bmRZone) { m_bmRZone = bmRZone; }
	inline void setNumEyes(int numEyes) { m_numTrueEyes = numEyes; }
	inline void setStatus(LifeStatus status) { m_status = status; }

	// operations
	inline void addBlock( WeichiBlock* block ) { 
		m_vBlocks.push_back(block) ; 
		m_bmBlocks |= block->getStoneMap() ; 
	}
	inline void addClosedArea( WeichiClosedArea* closedarea ) { 
		m_vClosedAreas.push_back(closedarea) ; 
		m_bmClosedAreas |= closedarea->getStoneMap() ;
	}

	inline void addOpenedArea( WeichiOpenedArea* openedArea ) { 
		m_vOpenedAreas.push_back(openedArea) ;
		m_bmOpenedAreas |= openedArea->getStoneMap() ;		
	}
	inline void addConnector( WeichiBitBoard bmConn ) { m_bmConnector |= bmConn ; } 
	inline void addRZone( WeichiBitBoard bmRZone ) { m_bmRZone |= bmRZone ; }
	inline void increaseNumTrueEyes() { ++m_numTrueEyes ; }

	// query	
	inline Color getColor() const { return m_color ; }
	inline bool hasBlock( WeichiBlock* block ) const { return m_bmBlocks.hasIntersection(block->getStoneMap()) ; }
	inline bool hasClosedArea( WeichiClosedArea* closedarea ) const { return m_bmClosedAreas.hasIntersection(closedarea->getStoneMap()) ; }
	inline bool hasOpenedArea( WeichiOpenedArea* openedarea ) const { return m_bmOpenedAreas.hasIntersection(openedarea->getStoneMap()) ; }
	
	inline const Vector<WeichiBlock*, MAX_NUM_BLOCKS>& getBlocks() const { return m_vBlocks ; }	
	inline const Vector<WeichiClosedArea*, MAX_NUM_CLOSEDAREA>& getClosedAreas() const { return m_vClosedAreas ; }
	inline const WeichiClosedArea* getClosedArea(int index) const { return m_vClosedAreas[index] ; }
	inline const Vector<WeichiOpenedArea*, MAX_NUM_OPENEDAREA>& getOpenedAreas() const { return m_vOpenedAreas ; }

	inline int getNumBlocks() const { return m_vBlocks.size(); }
	inline int getNumClosedAreas() const { return m_vClosedAreas.size(); }
	inline int getNumOpenedAreas() const { return m_vOpenedAreas.size() ; }

	inline WeichiBitBoard getStoneMap() const { return m_bmBlocks ; }
	inline WeichiBitBoard getClosedAreasMap() const { return m_bmClosedAreas ; }
	inline WeichiBitBoard getOpenedAreasMap() const { return m_bmOpenedAreas ; }	
	inline WeichiBitBoard getConnectorMap() const { return m_bmConnector ; }
	inline WeichiBitBoard getRZone() const { return m_bmRZone ; }
	inline LifeStatus getStatus() const { return m_status; }
	
	inline int getNumTrueEye() const { return m_numTrueEyes ; }

	HashKey64 getSkeletonHashKey() const
	{
		if (getNumClosedAreas() < 2) { return HashKey64(); }
		
		WeichiClosedArea* ca1 = m_vClosedAreas[0];
		WeichiClosedArea* ca2 = m_vClosedAreas[1];

		WeichiBitBoard bmMinNbr = ca1->getMinimumSurroundedBitBoard() | ca2->getMinimumSurroundedBitBoard();

		HashKey64 hashkey;
		int pos = 0;
		while ((pos=bmMinNbr.bitScanForward()) != -1) {
			hashkey ^= StaticBoard::getHashGenerator().getZHashKeyOf(m_color, pos);
		}

		return hashkey;
	}

	WeichiBitBoard getSkeletonBitBoard() const
	{
		if (getNumClosedAreas() < 2) { return WeichiBitBoard(); }
		
		WeichiClosedArea* ca1 = m_vClosedAreas[0];
		WeichiClosedArea* ca2 = m_vClosedAreas[1];

		WeichiBitBoard bmMinNbr = ca1->getMinimumSurroundedBitBoard() | ca2->getMinimumSurroundedBitBoard();
		return bmMinNbr;
	}


private:
	Color m_color;
	LifeStatus m_status;
	Vector<WeichiBlock*, MAX_NUM_BLOCKS> m_vBlocks ;
	Vector<WeichiClosedArea*, MAX_NUM_CLOSEDAREA> m_vClosedAreas ;
	Vector<WeichiOpenedArea*, MAX_NUM_OPENEDAREA> m_vOpenedAreas ;

	WeichiBitBoard m_bmBlocks ;
	WeichiBitBoard m_bmClosedAreas ;
	WeichiBitBoard m_bmOpenedAreas ;	
	WeichiBitBoard m_bmConnector ;
	WeichiBitBoard m_bmRZone ;
	int m_numTrueEyes ;
};

#endif