#ifndef WEICHIRADIUSPATTERNTABLE_H
#define WEICHIRADIUSPATTERNTABLE_H

#include "types.h"
#include "Point.h"
#include "Color.h"
#include "HashKey64.h"
#include "Dual.h"
#include <string>

/*
We collect radius pattern from 3 to 10 like following:
	(formula: d = x + y + max(x,y))
	.  .  .  .  . 10  .  .  .  .  .
	.  .  . 10  9  8  9 10  .  .  .
	.  .  9  8  7  6  7  8  9  .  .
	. 10  8  6  5  4  5  6  8 10  .
	.  9  7  5  3  3  3  5  7  9  .
   10  8  6  4  3  X  3  4  6  8 10
	.  9  7  5  3  3  3  5  7  9  .
	. 10  8  6  5  4  5  6  8 10  .
	.  .  9  8  7  6  7  8  9  .  .
	.  .  . 10  9  8  9 10  .  .  .
	.  .  .  .  . 10  .  .  .  .  .
*/
const int MIN_RADIUS_SIZE = 3;
const int MAX_RADIUS_SIZE = 11;
const int RADIUS_PATTERN_DIAMETER = 11;	// this variable is define by MAX_RADIUS_SIZE
const int RADIUS_PATTERN_TABLE_SIZE = RADIUS_PATTERN_DIAMETER*RADIUS_PATTERN_DIAMETER;
const int RADIUS_NBR_LIB_SIZE = 4;
const int RADIUS_NBR_LIB_BITS = 2;
const int RADIUS_NBR_MAX_LIB = 4;	// pow(2,RADIUS_NBR_LIB_BITS)
const int RADIUS3_TOTAL_BIT_SIZE = 16 + RADIUS_NBR_LIB_SIZE*RADIUS_NBR_LIB_BITS;

class WeichiRadiusPatternTable
{
public:
	class VirtualTable {
	public:
		int m_nbrLib[RADIUS_NBR_LIB_SIZE];	// direction: left, up, right, down
		int m_table[RADIUS_PATTERN_DIAMETER][RADIUS_PATTERN_DIAMETER];

		VirtualTable() { reset(); }
		void reset() {
			for( int i=0; i<RADIUS_NBR_LIB_SIZE; i++ ) { m_nbrLib[i] = 0; }
			for( int i=0; i<RADIUS_PATTERN_DIAMETER; i++ ) {
				for( int j=0; j<RADIUS_PATTERN_DIAMETER; j++ ) {
					m_table[i][j] = COLOR_SIZE;
				}
			}
		}
		void initTable( const Vector<uint,MAX_RADIUS_SIZE>& vIndex );
		Vector<uint,MAX_RADIUS_SIZE> calculateTableIndex();
		VirtualTable roate( SymmetryType type );
		void writeTable( int *table );
		void readTable( int *table );
	};

private:
	static int m_iEachRadiusNumber[MAX_RADIUS_SIZE];
	static int m_iEachRadiusStartIndex[MAX_RADIUS_SIZE];
	static int m_iRadiusTable[RADIUS_PATTERN_DIAMETER][RADIUS_PATTERN_DIAMETER];
	static HashKey64 m_hRadiusNbrLibertyHashKey[RADIUS_NBR_LIB_SIZE][RADIUS_NBR_MAX_LIB];
	static HashKey64 m_hRadiusTableHashKey[RADIUS_PATTERN_TABLE_SIZE][COLOR_SIZE];
	static HashKey64 *m_hRadiusPattrenHashKey[MAX_RADIUS_SIZE];
	static int m_iPatternIndexOffset[RADIUS_PATTERN_TABLE_SIZE][COLOR_SIZE];

public:
	static Vector<int,RADIUS_NBR_LIB_SIZE> vRadiusNbrLibOrder;
	static Vector<vector<Point>,MAX_RADIUS_SIZE> vRadiusTableOrder;
	static Vector<vector<Point>,MAX_RADIUS_SIZE> vRadiusTableShiftOrder;
	static Vector<Vector<uint,RADIUS_PATTERN_DIAMETER>,RADIUS_PATTERN_DIAMETER> vRadiusShiftIndex;
	static Dual<int*> radius3SymmetricIndex;

public:
	WeichiRadiusPatternTable() {}
	
	static void initialize();
	static void saveToDB( ofstream& fDB );
	static void readFromDB( ifstream& fDB );

	static inline uint getRadiusPatternIndexOffset( int iIndex, int color ) {
		assert( iIndex<RADIUS_PATTERN_TABLE_SIZE && color<COLOR_SIZE );
		return m_iPatternIndexOffset[iIndex][color];
	}
	static inline HashKey64 getRadiusPatternHashKey( int iRadius, int iIndex ) {
		assert( iRadius>=MIN_RADIUS_SIZE && iRadius<MAX_RADIUS_SIZE );
		return m_hRadiusPattrenHashKey[iRadius][iIndex];
	}
	static string getRadiusPatternDiagramString( Vector<uint,MAX_RADIUS_SIZE> vIndex );
	static Vector<HashKey64,MAX_RADIUS_SIZE> calculateMinRadiusPatternHashKey( const Vector<uint,MAX_RADIUS_SIZE>& vIndex );
	static Vector<Vector<HashKey64,SYMMETRY_SIZE>,MAX_RADIUS_SIZE> calculateRadiusPatternRotationHashKey( const Vector<uint,MAX_RADIUS_SIZE>& vIndex );

private:
	static void initializeRadiusNumberAndTable();
	static void initializeRadiusTableHashKey();
	static void initializeRadiusTableOrderPoint();
	static void initializeShiftIndexAndOffset();
	static void initializeRadiusPatternHashKey();
	static bool loadRadius3SymmetricIndex();
	static void initailizeRadius3SymmetricIndex();

	static HashKey64 calculateRadiusNbrLibertyHashKey( int iLibIndex );
	static HashKey64 calculateRadiusPatternIndexHashKey( int iRadius, int iIndex );
	static int getNeighborDirection( Point shifPoint );
};

#endif 
