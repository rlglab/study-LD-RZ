#ifndef WEICHIGRID_H
#define WEICHIGRID_H

#include "BasicType.h"
#include "StaticBoard.h"
#include "WeichiMove.h"
#include "BitBoard.h"
#include "Dual.h"
#include "FeatureList.h"
#include "FeaturePtrList.h"
#include "FeatureIndexList.h"
#include "WeichiEdge.h"
#include "WeichiGammaTable.h"

class WeichiBlock ;
class WeichiBoard ;
class WeichiClosedArea ;
//class WeichiEdge ;

class WeichiGrid
{
	friend class WeichiBoard ;
private:
	Color   m_color ;
	uint    m_liberty   BIT_N(4) ;
	uint    m_position ;

	Color   m_played ; // color that put here before	
	uint m_newPatternIndex;
	Vector<uint,MAX_RADIUS_SIZE> m_vPatternIndex;
	uint m_nbrLibIndex;
	WeichiBlock* m_block ;

	Dual<WeichiClosedArea*> m_closedArea ;
	WeichiEdge* m_edge;

	const StaticGrid* m_sgrid ;
	const StaticEdge* m_sedge ;

public:
	WeichiGrid () ;

	inline uint GetID() const { return m_position; }

	inline Color getColor () const { return m_color; }
	inline void setColor ( Color c ) { m_color = c ; }
	inline bool isEmpty () const { return m_color == COLOR_NONE; }
	inline bool isBlack () const { return m_color == COLOR_BLACK; }
	inline bool isWhite () const { return m_color == COLOR_WHITE; }

	inline void setPlayedColor ( Color c ) { m_played = (Color)(m_played|c); }
	inline bool isPlayed ( Color c ) const { return (m_played&c)!=0; }

	inline uint getLiberty() const { return m_liberty; }
	inline void incLiberty() { ++m_liberty; }
	inline void decLiberty() { --m_liberty; }

	inline uint getPosition () const { return m_position; }

	inline void setBlock ( WeichiBlock* block ) { m_block = block ; }
	inline WeichiBlock* getBlock () { return m_block ; }
	inline const WeichiBlock* getBlock () const { return m_block ; }

	inline const StaticGrid& getStaticGrid() const { return *m_sgrid ; }
	inline const StaticEdge& getStaticEdge() const { return *m_sedge ; }

	inline const WeichiBitBoard& getStonenNbrMap () const { return getStaticGrid().getStoneNbrsMap(); }

	//inline const pattern33::Pattern33 getPattern() const { return m_pattern; }
	inline const pattern33::Pattern33& getPattern() const { return StaticBoard::getPattern33(m_newPatternIndex); }
	inline void setPatternIndex(uint index)   { m_newPatternIndex = index ; }
	inline const uint getPatternIndex() const { return m_newPatternIndex ; }
	inline void addPatternOffset(uint offset) { m_newPatternIndex += offset ; }
	inline void subPatternOffset(uint offset) { m_newPatternIndex -= offset ; }
	//inline const pattern33::Pattern33& getPattern() const { return StaticBoard::getPattern33(getRadiusPatternRealIndex(0)); }

	inline WeichiEdge* getEdge() { return m_edge; }
	inline void setEdge( WeichiEdge* edge ) { m_edge = edge; }
	inline bool hasEdge() const { return (m_edge!=NULL); }

	inline uint getNbrLibIndex() const { return m_nbrLibIndex; }
	inline void setNbrLibIndex( uint newIndex ) { m_nbrLibIndex = newIndex; }

	inline void addPatternOffset( uint iRadius, uint iOffset ) { m_vPatternIndex[iRadius] += iOffset; }
	inline void subPatternOffset( uint iRadius, uint iOffset ) { m_vPatternIndex[iRadius] -= iOffset; }
	inline uint getRadiusPatternRealIndex( int iRadius, Color findColor=COLOR_BLACK ) const { return (findColor==COLOR_BLACK)?m_vPatternIndex[iRadius]:switch_players_in_patternIndex(m_vPatternIndex[iRadius]); }
	inline uint switch_players_in_patternIndex( uint patternIndex ) const { return ((patternIndex&0xaaaaaaaa)>>1) | ((patternIndex&0x55555555)<<1); }

	//Closed area
	inline Dual<WeichiClosedArea*>& getClosedArea() { return m_closedArea; }
	inline const Dual<WeichiClosedArea*>& getClosedArea() const { return m_closedArea; }
	inline WeichiClosedArea* getClosedArea(Color c) { return m_closedArea.get(c); }
	inline const WeichiClosedArea* getClosedArea(Color c) const { return m_closedArea.get(c); }
	inline void setClosedArea(WeichiClosedArea* closedArea, Color c) { m_closedArea.set(c,closedArea); }

	std::string getEdgeLinesString() const;

	inline void printMemberSize(){
		CERR() << "grid size:\t\t"            << sizeof *this << endl;
		CERR() << endl;
	}
private:
	void initialize ( uint pos ) ;
};

#endif