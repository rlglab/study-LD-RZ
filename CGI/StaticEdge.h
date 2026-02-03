#ifndef STATICEDGE_H
#define STATICEDGE_H

#include "BasicType.h"
#include "WeichiEdge.h"

class StaticEdge
{
private:
	enum ROTATION {
		ROTATION_NONE,
		ROTATION_4DIRECTION_CROSS_1_4,
		ROTATION_4DIRECTION_CROSS_2_4,
		ROTATION_4DIRECTION_CROSS_3_4,
		ROTATION_4DIRECTION_X_1_4,
		ROTATION_4DIRECTION_X_2_4,
		ROTATION_4DIRECTION_X_3_4,
		ROTATION_8DIRECTION_1_8,
		ROTATION_8DIRECTION_2_8,
		ROTATION_8DIRECTION_3_8,
		ROTATION_8DIRECTION_4_8,
		ROTATION_8DIRECTION_5_8,
		ROTATION_8DIRECTION_6_8,
		ROTATION_8DIRECTION_7_8,
	};
	class Point {
	private:
		int m_x;
		int m_y;
	public:
		Point() : m_x(INT_MAX), m_y(INT_MAX) {}
		Point( int x, int y ) : m_x(x), m_y(y) {}
		bool isValid() { return (m_x!=INT_MAX && m_y!=INT_MAX); }
		Point roate( ROTATION rotation );
		Point symmetryXAXIS( Point rotatePoint );
		Point symmetryYAXIS( Point rotatePoint );
		Point symmetryXEQUALY( Point rotatePoint );
		Point symmetryXEQUALNEGATIVEY( Point rotatePoint );
		Point symmetryCENTER( Point rotatePoint );
		inline void setX( int x ) { m_x = x; }
		inline void setY( int y ) { m_y = y; }
		inline int getX() { return m_x; }
		inline int getY() { return m_y; }
	};
	struct EdgeProperty {
		const static int StoneNum = 8;
		Point m_realStone[StoneNum];
		Point m_endStone[StoneNum];
		Point m_erodedStone[StoneNum];
		Point m_virtualCAStone[StoneNum];
		Point m_CAStone[StoneNum];
		Point m_ThreatStone[StoneNum];
		void copy( Point *to, Point *from ) {
			for( uint i=0; i<StoneNum; i++ ) { to[i] = from[i]; }
		}
	};
	const static int StoneNum = 8;
	static const ROTATION DIR4_CROSS[4];
	static const ROTATION DIR4_X[4];
	static const ROTATION DIR8[8];
	WeichiEdge m_edges[EDGE_SIZE];
public:
	StaticEdge() {}
	inline const WeichiEdge& getEdge( WeichiEdgeType type ) const { assert(type<EDGE_SIZE); return m_edges[type]; }
	void initialize( uint position );
private:
	bool isLineInBoard( uint line ) { return (line>=0 && line<WeichiConfigure::BoardSize); } 
	bool isEdgeRealPointExist( uint center_pos, Point* realPoint, ROTATION roateType );
	WeichiBitBoard getPointBitBoard( uint center_pos, Point* point, ROTATION rotateType );

	void initializeDiagonal( uint position );
	void initializeJump( uint position );
	void initializeKnight( uint position );
	void initializeBamboo( uint position );
	void initializeLHalfBamboo( uint position );
	void initializeTHalfBamboo( uint position );
	void initializeZHalfBamboo( uint position );
	void initializehBamboo( uint position );
	void initializeWallJump( uint position );
	void initializeWallBamboo( uint position );
	void consturctEdge( uint position, uint size, WeichiEdgeType* type, const ROTATION* rotation, EdgeProperty edgeProperty );
};

#endif