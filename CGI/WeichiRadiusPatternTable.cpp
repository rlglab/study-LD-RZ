#include "WeichiRadiusPatternTable.h"
#include "Pattern33Table.h"
#include "Pattern7533.h"
#include "rand64.h"

int WeichiRadiusPatternTable::m_iEachRadiusNumber[MAX_RADIUS_SIZE];
int WeichiRadiusPatternTable::m_iEachRadiusStartIndex[MAX_RADIUS_SIZE];
int WeichiRadiusPatternTable::m_iRadiusTable[RADIUS_PATTERN_DIAMETER][RADIUS_PATTERN_DIAMETER];
HashKey64 WeichiRadiusPatternTable::m_hRadiusNbrLibertyHashKey[RADIUS_NBR_LIB_SIZE][RADIUS_NBR_MAX_LIB];
HashKey64 WeichiRadiusPatternTable::m_hRadiusTableHashKey[RADIUS_PATTERN_TABLE_SIZE][COLOR_SIZE];
HashKey64 *WeichiRadiusPatternTable::m_hRadiusPattrenHashKey[MAX_RADIUS_SIZE];
int WeichiRadiusPatternTable::m_iPatternIndexOffset[RADIUS_PATTERN_TABLE_SIZE][COLOR_SIZE];
Vector<int,RADIUS_NBR_LIB_SIZE> WeichiRadiusPatternTable::vRadiusNbrLibOrder;
Vector<vector<Point>,MAX_RADIUS_SIZE> WeichiRadiusPatternTable::vRadiusTableOrder;
Vector<vector<Point>,MAX_RADIUS_SIZE> WeichiRadiusPatternTable::vRadiusTableShiftOrder;
Vector<Vector<uint,RADIUS_PATTERN_DIAMETER>,RADIUS_PATTERN_DIAMETER> WeichiRadiusPatternTable::vRadiusShiftIndex;
Dual<int*> WeichiRadiusPatternTable::radius3SymmetricIndex;

void WeichiRadiusPatternTable::initialize()
{
	initializeRadiusNumberAndTable();
	initializeRadiusTableHashKey();
	initializeRadiusTableOrderPoint();
	initializeShiftIndexAndOffset();

	initializeRadiusPatternHashKey();
	if( !loadRadius3SymmetricIndex() ) {
		initailizeRadius3SymmetricIndex();
	}
}

void WeichiRadiusPatternTable::saveToDB( ofstream& fDB )
{
	fDB.write(reinterpret_cast<char*>(radius3SymmetricIndex.m_black), sizeof(int)*(1<<RADIUS3_TOTAL_BIT_SIZE));
	fDB.write(reinterpret_cast<char*>(radius3SymmetricIndex.m_white), sizeof(int)*(1<<RADIUS3_TOTAL_BIT_SIZE));
}

void WeichiRadiusPatternTable::readFromDB( ifstream& fDB )
{
	radius3SymmetricIndex.m_black = new int[1<<RADIUS3_TOTAL_BIT_SIZE];
	radius3SymmetricIndex.m_white = new int[1<<RADIUS3_TOTAL_BIT_SIZE];
	fDB.read(reinterpret_cast<char*>(radius3SymmetricIndex.m_black), sizeof(int)*(1<<RADIUS3_TOTAL_BIT_SIZE));
	fDB.read(reinterpret_cast<char*>(radius3SymmetricIndex.m_white), sizeof(int)*(1<<RADIUS3_TOTAL_BIT_SIZE));
}

void WeichiRadiusPatternTable::VirtualTable::initTable( const Vector<uint,MAX_RADIUS_SIZE>& vIndex )
{
	ull maskBits = 0x3;

	for( int iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {		
		int iIndex = vIndex[iRadius];
		if( iRadius==3 ) { iIndex = (iIndex>>(RADIUS_NBR_LIB_SIZE*RADIUS_NBR_LIB_BITS)); }
		
		const vector<Point>& vOrderPoint = WeichiRadiusPatternTable::vRadiusTableOrder[iRadius];
		for( uint iNum=0; iNum<vOrderPoint.size(); iNum++ ) {
			const uint color = (iIndex>>(iNum*2))&maskBits;
			const Point& point = vOrderPoint[iNum];
			m_table[point.getY()][point.getX()] = color;
		}
	}

	// radius neighbor liberty
	for( int iNbr=0; iNbr<RADIUS_NBR_LIB_SIZE; iNbr++ ) {
		m_nbrLib[iNbr] = ((vIndex[3]>>(iNbr*RADIUS_NBR_LIB_BITS))&maskBits);
	}

	// for debug assert
	Vector<uint,MAX_RADIUS_SIZE> vInvarianceIndex = calculateTableIndex();
	for( int iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		assert( vIndex[iRadius]==vInvarianceIndex[iRadius] );
	}
}

Vector<uint,MAX_RADIUS_SIZE> WeichiRadiusPatternTable::VirtualTable::calculateTableIndex()
{
	Vector<uint,MAX_RADIUS_SIZE> vIndex;
	vIndex.setAllAs(0,MAX_RADIUS_SIZE);

	for( int iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		const vector<Point>& vOrderPoint = WeichiRadiusPatternTable::vRadiusTableOrder[iRadius];
		for( uint iNum=0; iNum<vOrderPoint.size(); iNum++ ) {
			const Point& point = vOrderPoint[iNum];
			int color = m_table[point.getY()][point.getX()];
			assert( color<COLOR_SIZE );

			vIndex[iRadius] += (color << (iNum*2));
		}
	}

	// radius neighbor liberty
	vIndex[3] = (vIndex[3]<<(RADIUS_NBR_LIB_SIZE*RADIUS_NBR_LIB_BITS));
	for( int iNbr=0; iNbr<RADIUS_NBR_LIB_SIZE; iNbr++ ) {
		vIndex[3] += (m_nbrLib[iNbr]<<(iNbr*RADIUS_NBR_LIB_BITS));
	}
	
	return vIndex;
}

WeichiRadiusPatternTable::VirtualTable WeichiRadiusPatternTable::VirtualTable::roate( SymmetryType type )
{
	VirtualTable rotateTable;
	int iCenter = (RADIUS_PATTERN_DIAMETER-1)/2;

	for( int iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		const vector<Point>& vOrderShiftPoint = WeichiRadiusPatternTable::vRadiusTableShiftOrder[iRadius];
		for( uint iNum=0; iNum<vOrderShiftPoint.size(); iNum++ ) {
			const Point& point = vOrderShiftPoint[iNum];
			const Point roatePoint = point.getSymmetryOf(static_cast<SymmetryType>(type));
			rotateTable.m_table[iCenter+point.getY()][iCenter+point.getX()] = m_table[iCenter+roatePoint.getY()][iCenter+roatePoint.getX()];
			assert( rotateTable.m_table[iCenter+point.getY()][iCenter+point.getX()]<COLOR_SIZE );
		}
	}

	// radius neighbor liberty
	int iNbr = 0;
	Point pCenter(iCenter,iCenter);
	for( uint i=0; i<vRadiusTableShiftOrder[3].size(); i++ ) {
		const Point& point = vRadiusTableOrder[3][i];
		if( pCenter.getRadiusDistance(point)==3 ) { continue; }

		const Point& shiftPoint = vRadiusTableShiftOrder[3][i];
		const Point roatePoint = shiftPoint.getSymmetryOf(static_cast<SymmetryType>(type));
		rotateTable.m_nbrLib[iNbr++] = m_nbrLib[getNeighborDirection(roatePoint)];
	}

	return rotateTable;
}

void WeichiRadiusPatternTable::VirtualTable::writeTable( int *table )
{
	// 3x3 table, liberty table (left, up, right, down)
	int iCenter = (RADIUS_PATTERN_DIAMETER-1)/2;
	for( int i=iCenter-1, iCount=0; i<=iCenter+1; i++,iCount++ ) {
		for( int j=iCenter-1, jCount=0; j<=iCenter+1; j++,jCount++ ) {
			table[iCount*3+jCount] = m_table[i][j];
		}
	}
	for( int iNbr=0, iIndex=9; iNbr<RADIUS_NBR_LIB_SIZE; iNbr++,iIndex++ ) {
		table[iIndex] = m_nbrLib[iNbr];
	}
}

void WeichiRadiusPatternTable::VirtualTable::readTable( int *table )
{
	int iCenter = (RADIUS_PATTERN_DIAMETER-1)/2;
	for( int i=iCenter-1, iCount=0; i<=iCenter+1; i++,iCount++ ) {
		for( int j=iCenter-1, jCount=0; j<=iCenter+1; j++,jCount++ ) {
			m_table[i][j] = table[iCount*3+jCount];
		}
	}
	for( int iNbr=0, iIndex=9; iNbr<RADIUS_NBR_LIB_SIZE; iNbr++,iIndex++ ) {
		m_nbrLib[iNbr] = table[iIndex];
	}
}

string WeichiRadiusPatternTable::getRadiusPatternDiagramString( Vector<uint,MAX_RADIUS_SIZE> vIndex )
{
	VirtualTable vt;
	vt.initTable(vIndex);

	ostringstream oss;
	oss << "radius diagram:" << endl;
	for( int y=RADIUS_PATTERN_DIAMETER-1; y>=0; y-- ) {
		for( int x=0; x<RADIUS_PATTERN_DIAMETER; x++ ) {
			if( vt.m_table[y][x]==COLOR_NONE ) { oss << "¡E"; }
			else if( vt.m_table[y][x]==COLOR_BLACK ) { oss << "¡´"; }
			else if( vt.m_table[y][x]==COLOR_WHITE ) { oss << "¡³"; }
			else if( vt.m_table[y][x]==COLOR_BORDER ) { oss << "¢æ"; }
			else { oss << "  "; }
		}
		oss << endl;
	}

	// this direction is defined in basicType
	oss << "Neighbor liberty: " << endl;
	const string sDirName[RADIUS_NBR_LIB_SIZE] = {"left", "up", "right", "down"};
	for( int i=0; i<RADIUS_NBR_LIB_SIZE; i++ ) {
		oss << sDirName[i] << ": " << vt.m_nbrLib[i] << endl;
	}

	return oss.str();
}

Vector<HashKey64,MAX_RADIUS_SIZE> WeichiRadiusPatternTable::calculateMinRadiusPatternHashKey( const Vector<uint,MAX_RADIUS_SIZE>& vIndex )
{
	const Vector<Vector<HashKey64,SYMMETRY_SIZE>,MAX_RADIUS_SIZE>& vRotateHashKey = calculateRadiusPatternRotationHashKey(vIndex);

	Vector<HashKey64,MAX_RADIUS_SIZE> vMinHashKey;
	vMinHashKey.setAllAs(0,MAX_RADIUS_SIZE);
	for( int iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		vMinHashKey[iRadius] = vRotateHashKey[iRadius][0];
		for( int symmetric=1; symmetric<SYMMETRY_SIZE; symmetric++ ) {
			if( vRotateHashKey[iRadius][symmetric]<vMinHashKey[iRadius] ) { vMinHashKey[iRadius] = vRotateHashKey[iRadius][symmetric]; }
		}
	}

	return vMinHashKey;
}

Vector<Vector<HashKey64,SYMMETRY_SIZE>,MAX_RADIUS_SIZE> WeichiRadiusPatternTable::calculateRadiusPatternRotationHashKey( const Vector<uint,MAX_RADIUS_SIZE>& vIndex )
{
	VirtualTable vt;
	vt.initTable(vIndex);
	Vector<Vector<HashKey64,SYMMETRY_SIZE>,MAX_RADIUS_SIZE> vRadiusPatternRotationHashKey;

	vRadiusPatternRotationHashKey.resize(MAX_RADIUS_SIZE);
	for( int iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		vRadiusPatternRotationHashKey[iRadius].setAllAs(0,SYMMETRY_SIZE);
	}

	HashKey64 accumalateKey;
	for( int symmetric=0; symmetric<SYMMETRY_SIZE; symmetric++ ) {
		accumalateKey = 0;
		VirtualTable rotateTable = vt.roate(static_cast<SymmetryType>(symmetric));
		Vector<uint,MAX_RADIUS_SIZE> vRotateIndex = rotateTable.calculateTableIndex();

		for( int iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
			uint index = vRotateIndex[iRadius];
			vRadiusPatternRotationHashKey[iRadius][symmetric] = m_hRadiusPattrenHashKey[iRadius][index] ^ accumalateKey;
			accumalateKey = vRadiusPatternRotationHashKey[iRadius][symmetric];
		}
	}

	return vRadiusPatternRotationHashKey;
}

void WeichiRadiusPatternTable::initializeRadiusNumberAndTable()
{
	int iCenter = (RADIUS_PATTERN_DIAMETER-1)/2;
	Point pCenter(iCenter,iCenter);

	for( int iRadius=0; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		m_iEachRadiusNumber[iRadius] = m_iEachRadiusStartIndex[iRadius] = 0;
	}

	for( int x=0; x<RADIUS_PATTERN_DIAMETER; x++ ) {
		for( int y=0; y<RADIUS_PATTERN_DIAMETER; y++ ) {
			int distance = pCenter.getRadiusDistance(Point(x,y));
			if( distance==2 ) { distance++; }	// we take 2 as 3

			m_iRadiusTable[x][y] = distance;
			if( distance>=MIN_RADIUS_SIZE && distance<MAX_RADIUS_SIZE ) { m_iEachRadiusNumber[distance]++; }
		}
	}

	for( int iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		m_iEachRadiusStartIndex[iRadius] += m_iEachRadiusStartIndex[iRadius-1] + m_iEachRadiusNumber[iRadius-1];
	}
}

void WeichiRadiusPatternTable::initializeRadiusTableHashKey()
{
	//init_genrand64(0);
	for( int iNbr=0; iNbr<RADIUS_NBR_LIB_SIZE; iNbr++ ) {
		for( int lib=0; lib<RADIUS_NBR_MAX_LIB; lib++ ) {
			m_hRadiusNbrLibertyHashKey[iNbr][lib] = rand64();
		}
	}
	for( uint iIndex=0; iIndex<RADIUS_PATTERN_TABLE_SIZE; iIndex++ ) {
		for( uint color=0; color<COLOR_SIZE; color++ ) {
			m_hRadiusTableHashKey[iIndex][color] = rand64();
		}
	}
}

void WeichiRadiusPatternTable::initializeRadiusTableOrderPoint()
{
	vRadiusTableOrder.resize(MAX_RADIUS_SIZE);
	vRadiusTableShiftOrder.resize(MAX_RADIUS_SIZE);

	int iCenter = (RADIUS_PATTERN_DIAMETER-1)/2;
	for( int x=0; x<RADIUS_PATTERN_DIAMETER; x++ ) {
		for( int y=0; y<RADIUS_PATTERN_DIAMETER; y++ ) {
			int iRadius = m_iRadiusTable[x][y];
			if( iRadius<MIN_RADIUS_SIZE || iRadius>=MAX_RADIUS_SIZE ) { continue; }
			
			vRadiusTableOrder[iRadius].push_back(Point(x,y));
			vRadiusTableShiftOrder[iRadius].push_back(Point(x-iCenter,y-iCenter));
		}
	}

	for( int iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		assert( vRadiusTableOrder[iRadius].size()==m_iEachRadiusNumber[iRadius] );
		sort(vRadiusTableOrder[iRadius].begin(),vRadiusTableOrder[iRadius].end());
		sort(vRadiusTableShiftOrder[iRadius].begin(),vRadiusTableShiftOrder[iRadius].end());
	}

	// radius neighbor order
	Point pCenter(iCenter,iCenter);
	for( uint i=0; i<vRadiusTableShiftOrder[3].size(); i++ ) {
		const Point& point = vRadiusTableOrder[3][i];
		if( pCenter.getRadiusDistance(point)==3 ) { continue; }

		const Point& shiftPoint = vRadiusTableShiftOrder[3][i];
		vRadiusNbrLibOrder.push_back(getNeighborDirection(shiftPoint));
	}
}

void WeichiRadiusPatternTable::initializeShiftIndexAndOffset()
{
	vRadiusShiftIndex.resize(RADIUS_PATTERN_DIAMETER);
	for( int i=0; i<RADIUS_PATTERN_DIAMETER; i++ ) { vRadiusShiftIndex[i].resize(RADIUS_PATTERN_DIAMETER); }
	
	int counter = 0;
	for( int iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {		
		const vector<Point>& vOrderPoint = WeichiRadiusPatternTable::vRadiusTableOrder[iRadius];
		for( uint iNum=0; iNum<vOrderPoint.size(); iNum++ ) {
			const Point& point = vOrderPoint[iNum];

			vRadiusShiftIndex[point.getY()][point.getX()] = counter;
			for( int color=0; color<COLOR_SIZE; color++ ) {
				m_iPatternIndexOffset[counter][color] = (color << (iNum*2));
			}
			counter++;
		}
	}	
}

void WeichiRadiusPatternTable::initializeRadiusPatternHashKey()
{
	/*
	2-bit representation:
		Empty:  00
		Black:  01
		White:  10
		Border: 11
	*/

	CERR() << "Generate radius pattern hash key ... ";

	// malloc each radius pattern table size
	for( int iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		int iPatternTableSize = static_cast<int>(pow(2.0,m_iEachRadiusNumber[iRadius]*2));
		// special case for radius 3 (with neighbor liberty)
		if( iRadius==3 ) { iPatternTableSize *= static_cast<int>(pow(2.0f,RADIUS_NBR_LIB_SIZE*RADIUS_NBR_LIB_BITS)); }
	
		// calculate each index hash key
		m_hRadiusPattrenHashKey[iRadius] = new HashKey64[iPatternTableSize];
		for( int iIndex=0; iIndex<iPatternTableSize; iIndex++ ) {
			m_hRadiusPattrenHashKey[iRadius][iIndex] = calculateRadiusPatternIndexHashKey(iRadius,iIndex);
		}
	}

	CERR() << " ==> succeed!" << endl;
}

bool WeichiRadiusPatternTable::loadRadius3SymmetricIndex()
{
	CERR() << "Load radius3 symmetric index ... ";

	string sDirectory = WeichiConfigure::db_dir + "radius3_symmetric_index.db";
	ifstream fDB(sDirectory.c_str(),ios::in|ios::binary);
	if( !fDB ) {
		CERR() << " ==> failed! Cannot find \"" << sDirectory << "\" (generate later)"<< endl;
		return false;
	}

	readFromDB(fDB);
	fDB.close();

	CERR() << " ==> succeed!" << endl;

	return true;
}

void WeichiRadiusPatternTable::initailizeRadius3SymmetricIndex()
{
	CERR() << "Generate radius3 symmetric index ... ";

	int table[13];
	VirtualTable vt;
	Vector<uint,MAX_RADIUS_SIZE> vIndex;
	vIndex.setAllAs(0,MAX_RADIUS_SIZE);

	map<HashKey64,uint> mapIndex;
	for( int iCount=0; iCount<MAX_LEGAL_3X3PATTERN_SIZE; iCount++ ) {
		for( int i=0; i<13; i++ ) { table[i] = g_sPattern7533[iCount][i]-'0'; }

		vt.readTable(table);
		vIndex = vt.calculateTableIndex();
		Vector<HashKey64,MAX_RADIUS_SIZE> vMinHash = calculateMinRadiusPatternHashKey(vIndex);
		mapIndex[vMinHash[3]] = iCount;
	}
	assert( mapIndex.size()==MAX_LEGAL_3X3PATTERN_SIZE );

	const int radius3Size = (1<<RADIUS3_TOTAL_BIT_SIZE);
	radius3SymmetricIndex.m_black = new int[radius3Size];
	radius3SymmetricIndex.m_white = new int[radius3Size];
	for( int iIndex=0; iIndex<radius3Size; iIndex++ ) {
		// black index
		vIndex[3] = iIndex;
		Vector<HashKey64,MAX_RADIUS_SIZE> vMinHash = calculateMinRadiusPatternHashKey(vIndex);
		if( mapIndex.find(vMinHash[3])!=mapIndex.end() ) { radius3SymmetricIndex.m_black[iIndex] = mapIndex[vMinHash[3]]; }
		else { radius3SymmetricIndex.m_black[iIndex] = MAX_LEGAL_3X3PATTERN_SIZE; }

		// white index (last 8-bits should not change)
		vIndex[3] = ((iIndex&0xaaaaaaaa)>>1) | ((iIndex&0x55555555)<<1);
		vIndex[3] = ((vIndex[3]&0xffffff00) | (iIndex&0xff));
		vMinHash = calculateMinRadiusPatternHashKey(vIndex);
		if( mapIndex.find(vMinHash[3])!=mapIndex.end() ) { radius3SymmetricIndex.m_white[iIndex] = mapIndex[vMinHash[3]]; }
		else { radius3SymmetricIndex.m_white[iIndex] = MAX_LEGAL_3X3PATTERN_SIZE; }
	}

	CERR() << " ==> succeed!" << endl;
}

HashKey64 WeichiRadiusPatternTable::calculateRadiusNbrLibertyHashKey( int iLibIndex )
{
	ull maskBits = 0x3;
	HashKey64 key = 0;
	
	for( int iNbr=0; iNbr<RADIUS_NBR_LIB_SIZE; iNbr++ ) {
		int liberty = ((iLibIndex>>(iNbr*RADIUS_NBR_LIB_BITS))&maskBits);
		
		if( liberty>3 ) { liberty = 0; }
		key ^= m_hRadiusNbrLibertyHashKey[iNbr][liberty];
	}

	return key;
}

HashKey64 WeichiRadiusPatternTable::calculateRadiusPatternIndexHashKey( int iRadius, int iIndex )
{
	assert( iRadius<MAX_RADIUS_SIZE );

	ull maskBits = 0x3;
	HashKey64 key = 0;

	// radius 3 neighbor liberty index
	if( iRadius==3 ) {
		key = calculateRadiusNbrLibertyHashKey(iIndex);
		iIndex = (iIndex>>(RADIUS_NBR_LIB_SIZE*RADIUS_NBR_LIB_BITS));
	}

	for( int iNum=0; iNum<m_iEachRadiusNumber[iRadius]; iNum++ ) {
		int color = (iIndex>>(iNum*2)&maskBits);
		key ^= m_hRadiusTableHashKey[m_iEachRadiusStartIndex[iRadius]+iNum][color];
	}

	return key;
}

int WeichiRadiusPatternTable::getNeighborDirection( Point shifPoint )
{
	assert( abs(shifPoint.getX())==0 || abs(shifPoint.getY())==0 );
	assert( abs(shifPoint.getX())==1 || abs(shifPoint.getY())==1 );

	if( shifPoint.getX()==0 ) {
		if( shifPoint.getY()==1 ) { return UPPER_IDX; }
		else if( shifPoint.getY()==-1 ) { return DOWN_IDX; }
	} else if( shifPoint.getY()==0 ) {
		if( shifPoint.getX()==1 ) { return RIGHT_IDX; }
		else if( shifPoint.getX()==-1 ) { return LEFT_IDX; }
	}

	assert( false );
	return -1;
}