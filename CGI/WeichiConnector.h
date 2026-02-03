#ifndef H_WEICHI_CONNECTOR
#define H_WEICHI_CONNECTOR

#include "BasicType.h"
#include "Color.h"
#include "WeichiBlock.h"

enum WeichiConnectorType {
	TYPE_DIAGONAL,
	TYPE_BAMBOO,
	TYPE_HALFBAMBOO,
	TYPE_WALLBAMBOO,
	TYPE_TIGERMOUTH,
	TYPE_FALSE_EYE,
	TYPE_JUMP,
	TYPE_WALLJUMP,
	TYPE_BAN,
	TYPE_TIGERMOUTH_1LIB_DEADBLOCK,	
	TYPE_MUTUAL_BLOCK_LIB,

	TYPE_UNKNOWN
};

class WeichiConnector
{
public:
	inline uint    GetID () const { return m_id; }
	inline void    SetID ( uint id ) { m_id = id ; }
	inline void	   Clear () { m_id = static_cast<uint>(-1); }

	void init()
	{
		m_type = TYPE_UNKNOWN ;
		m_color = COLOR_NONE ;
		m_blockIds.clear() ;
		m_bmConnect.Reset() ;
		m_bmThreat.Reset() ;					
	}

	// Setter
	inline void setType( WeichiConnectorType type ) { m_type = type ; }
	inline void setColor( Color color ) { m_color = color ; }	
	inline void setConnector( WeichiBitBoard bmConn ) { m_bmConnect = bmConn ; }
	inline void setThreatBitmap( WeichiBitBoard bmThreat ) { m_bmThreat = bmThreat ; }

	// Getter
	inline WeichiConnectorType getType() const { return m_type ; }
	inline Color getColor () const { return m_color ; }	
	inline WeichiBitBoard getConnectorBitmap() const { return m_bmConnect ; }
	inline WeichiBitBoard getThreatBitmap() const { return m_bmThreat ; }
	inline const Vector<short, 4>& getBlockIDs() const { return m_blockIds ; }	

	// Operations
	inline void addBlockId( short blockId ) { m_blockIds.push_back(blockId) ; }
	inline void addConnecPoint( uint pos ) { m_bmConnect.SetBitOn(pos) ; }
	inline void addThreatLocation( uint pos ) { m_bmThreat.SetBitOn(pos) ;}
	inline void addThreatLocation( uint firstPos, uint secondPos ) 
	{
		addThreatLocation(firstPos) ;
		addThreatLocation(secondPos) ;
	}
	inline void addThreatBitmap(WeichiBitBoard bmThreat) {
		m_bmThreat |= bmThreat;
	}

	inline string toTypeString() const
	{
		switch( m_type ) {
			case TYPE_DIAGONAL:						return "TYPE_DIAGONAL" ;
			case TYPE_BAMBOO:						return "TYPE_BAMBOO" ;
			case TYPE_HALFBAMBOO:					return "TYPE_HALFBAMBOO" ;
			case TYPE_WALLBAMBOO:					return "TYPE_WALLBAMBOO" ;
			case TYPE_TIGERMOUTH:					return "TYPE_TIGERMOUTH" ;			
			case TYPE_FALSE_EYE:					return "TYPE_FALSE_EYE" ;
			case TYPE_JUMP:							return "TYPE_JUMP" ;
			case TYPE_WALLJUMP:						return "TYPE_WALL_JUMP" ;
			case TYPE_BAN:							return "TYPE_BAN" ;
			case TYPE_TIGERMOUTH_1LIB_DEADBLOCK:	return "TYPE_TIGERMOUTH_1LIB_DEADBLOCK" ;
		}

		return "TYPE_UNKNOWN" ;
	}

	inline bool isStrong() const {
		switch( m_type ) {
			case TYPE_DIAGONAL:						return true ;
			case TYPE_BAMBOO:						return true ;
			case TYPE_WALLBAMBOO:					return true ;
			case TYPE_TIGERMOUTH:					return true ;			
			case TYPE_FALSE_EYE:					return true ;			
			case TYPE_TIGERMOUTH_1LIB_DEADBLOCK:	return true ;
		}

		// Weak:
		// *TYPE_HALFBAMBOO
		// *TYPE_JUMP
		// *TYPE_WALLJUMP
		// *TYPE_BAN
		
		return false ;
	}

	inline bool isWeak() const { return !isStrong() ; }
	inline bool isConnectedToWall() const { return m_type==TYPE_WALLBAMBOO || m_type==TYPE_WALLJUMP; }	

private:
	uint m_id;
	WeichiConnectorType m_type;	
	Color m_color;
	Vector<short, 4> m_blockIds;			
	WeichiBitBoard m_bmConnect;	
	WeichiBitBoard m_bmThreat;		
};


#endif