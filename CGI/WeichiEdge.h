#ifndef _WEICHIEDGE_
#define _WEICHIEDGE_

#include "BasicType.h"
#include "Color.h"

class WeichiEdge {
private:
	uint m_id;
	int m_iAnotherEndStonePos;
	WeichiBitBoard m_bmRealStone;
	WeichiBitBoard m_bmEndStone;
	WeichiBitBoard m_bmErodedStone;
	WeichiBitBoard m_bmVirtualCAStone;
	WeichiBitBoard m_bmCAStone;
	WeichiBitBoard m_bmThreatStone;
	WeichiEdgeBits m_edgeBits;

public:
	WeichiEdge() { Clear(); }
	
	void Clear();
	void init();
	inline uint GetID () const { return m_id; }
	inline void SetID(uint id) { m_id = id ; }

	// set edge's property
	inline void setRealStone( WeichiBitBoard bmRealStone ) { m_bmRealStone = bmRealStone; }
	inline void setEndStone( WeichiBitBoard bmEndStone ) { m_bmEndStone = bmEndStone; }
	inline void setErodedStone( WeichiBitBoard bmErodedStone ) { m_bmErodedStone = bmErodedStone; }
	inline void setVirtualCAStone( WeichiBitBoard bmVirtualCAStone ) { m_bmVirtualCAStone = bmVirtualCAStone; }
	inline void setCAStone( WeichiBitBoard bmCAStone ) { m_bmCAStone = bmCAStone; }
	inline void setThreatStone( WeichiBitBoard bmThreatStone ) { m_bmThreatStone = bmThreatStone; }
	inline void setAnotherEndStonePos( uint pos ) { m_iAnotherEndStonePos = pos; }
	inline void setEdgeBits( WeichiEdgeBits edgeBits ) { m_edgeBits |= edgeBits ;}

	// get edge's property
	inline const WeichiBitBoard& getRealStone() const { return m_bmRealStone; }
	inline const WeichiBitBoard& getEndStone() const { return m_bmEndStone; }
	inline const WeichiBitBoard& getErodedStone() const { return m_bmErodedStone; }
	inline const WeichiBitBoard& getVirtualCAStone() const { return m_bmVirtualCAStone; }
	inline const WeichiBitBoard& getCAStone() const { return m_bmCAStone; }
	inline const WeichiBitBoard& getThreatStone() const { return m_bmThreatStone; }
	inline const WeichiEdgeBits& getEdgeBits() const { return m_edgeBits; }
	inline uint getAnotherEndStonePos() const { return m_iAnotherEndStonePos; }

	// add edge's property
	inline void addRealStone( const WeichiBitBoard& bmRealStone ) { m_bmRealStone |= bmRealStone; }
	inline void addEndStone( const WeichiBitBoard& bmEndStone ) { m_bmEndStone |= bmEndStone; }
	inline void addErodedStone( const WeichiBitBoard& bmErodedStone ) { m_bmErodedStone |= bmErodedStone; }
	inline void addVirtualCAStone( const WeichiBitBoard& bmVirtualCAStone ) { m_bmVirtualCAStone |= bmVirtualCAStone; }
	inline void addCAStone( const WeichiBitBoard& bmCAStone ) { m_bmCAStone |= bmCAStone; }
	inline void addThreatStone( const WeichiBitBoard& bmThreatStone ) { m_bmThreatStone |= bmThreatStone; }

	// isEdge Type
	inline bool isEdgeType() { return !m_edgeBits.empty() ; } 

	// information
	std::string getEdgeInformationString() const;
};

#endif