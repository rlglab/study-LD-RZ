#include "DbHashTable.h"
#include "string.h"


DBHashTable::DBHashTable(int boardSize)
{
	m_boardSize = boardSize;
	IsomorphicMinHashkey::setStaticCentralPosition(m_boardSize);
	initializeZHashKey();
	creatSymmetricHashTable();
	setDefualtSymmetricTypes();
}

void DBHashTable::initializeZHashKey()
{
	init_genrand64(0);
	for (int x = 0; x < m_boardSize; x++) {
		for (int y = 0; y < m_boardSize; y++) {
			for (int i = 0 ; i < HASH_KEY_VARIANCE ; i++) {
				m_hashTable[getHashTableIndex(x, y)][i] = rand64();
			}				
		}
	}
	for(int i = 0;i < HASH_KEY_VARIANCE; i++)
		m_passMoveHashValue[i] = rand64() ;
	m_whiteTurnHashValue = rand64() ;
}

void DBHashTable::creatSymmetricHashTable()
{
	for (int x = 0; x < m_boardSize; x++) {
		for (int y = 0; y < m_boardSize; y++) {
			for(int symmetricType = SYM_NORMAL; symmetricType <= SYM_HORIZONTAL_REFLECTION_ROTATE_270; symmetricType++) {
				for(int i = 0; i < HASH_KEY_VARIANCE; i++) {
					MoveLocation ml = IsomorphicMinHashkey::getSymmetricCoordinate( Point(x, y), static_cast<SymmetryType>(symmetricType) ) ;
					Point symmetricPosition(ml.x,ml.y) ;
					m_symmetricHashTable[getHashTableIndex(symmetricPosition)][symmetricType][i] = m_hashTable[getHashTableIndex(x, y)][i] ;
				}
			}
		}
	}
}

IsomorphicMinHashkey DBHashTable::getBoardHashKey(DbMove* moves, uint moveNum, int iLoopDepth )
{
	if( moveNum == 0 ) return IsomorphicMinHashkey();

	m_board.reset() ;	
	m_ht.clear() ;
	m_vBlackPosition.clear() ; 
	m_vWhitePosition.clear() ;
	bool bIsLastMovePass = false ;
	for( uint i=0 ; i< moveNum ; i++ ) {
		Color color = i % 2 == 0 ? COLOR_BLACK : COLOR_WHITE ;
		int moveValue = moves[i].getValue() ;
		MoveLocation location(moveValue) ;		
		int x = location.x ;
		int y = location.y ;
		WeichiMove wm(color, WeichiMove::toPosition(x,y) ) ;
		if( wm == PASS_MOVE && i == moveNum -1 )
			bIsLastMovePass = true ;
		if( !m_board.isIllegalMove(wm,m_ht) ){
			m_board.play(wm) ;
			m_ht.store(m_board.getHash()) ;
		}
	}

	//CERR() << m_board.toString() << endl ;
	WeichiBitBoard bm = m_board.getBitBoard() ;
	int pos = 0;
	while( (pos=bm.bitScanForward())!=-1 ) {
		WeichiGrid& grid = m_board.getGrid(pos) ;
		Color c = grid.getColor() ;
		uint posX=0, posY=0 ;
		WeichiMove::toCoordinate(pos, posX, posY) ;
		Point p(posX, posY) ;
		if( c == COLOR_BLACK )
			m_vBlackPosition.push_back(p) ;
		else 
			m_vWhitePosition.push_back(p) ;
	}
		
	PlayerColor lastColor = moveNum % 2 == 1 ? PLAYERCOLOR_BLACK : PLAYERCOLOR_WHITE ;
	return generateBoardHashKey(lastColor, bIsLastMovePass, getLoopPositions(iLoopDepth)) ;
}

IsomorphicMinHashkey DBHashTable::generateBoardHashKey( PlayerColor color, bool bLastMoveIsPassMove, const vector<Point>& vLoopPositions  )
{
	IsomorphicMinHashkey symmetricMinHashKey;
	vector<SymmetryType>& vSymmetryTypes = getSymmetricTypes();
	for(size_t iSymmetryIndex = 0; iSymmetryIndex < vSymmetryTypes.size(); iSymmetryIndex++) {
		SymmetryType symmetryType = vSymmetryTypes[iSymmetryIndex];
		unsigned long long ullHashKey = getAllPieceHashValue(symmetryType);
		for(size_t loopPosIndex = 0; loopPosIndex < vLoopPositions.size(); loopPosIndex++) {
			if(vLoopPositions[loopPosIndex] != Point(-1, -1))
				ullHashKey ^= m_symmetricHashTable[getHashTableIndex(vLoopPositions[loopPosIndex])][symmetryType][3 + loopPosIndex];
			else
				ullHashKey ^= m_passMoveHashValue[2 + loopPosIndex];
		}
		if(color == PLAYERCOLOR_WHITE)
			ullHashKey ^= m_whiteTurnHashValue;
		if(bLastMoveIsPassMove) 
			ullHashKey ^= m_passMoveHashValue[color];

		if( iSymmetryIndex == 0 || ullHashKey < symmetricMinHashKey.getHashKey()) {
			symmetricMinHashKey.setHashKey(ullHashKey);
			symmetricMinHashKey.setSymmetryType(symmetryType);
		}
	}
	return symmetricMinHashKey ;
}

int DBHashTable::getHashTableIndex(const Point& position)
{
	return getHashTableIndex(position.getX(), position.getY() );
}

int DBHashTable::getHashTableIndex(const int& x, const int& y)
{
	return x + y * m_boardSize;
}


bool DBHashTable::checkNodeIsPassMove(Point position)
{
	return position.getX() == -1 && position.getY() == -1;
}

bool DBHashTable::isLegalPosition(Point position)
{
	return position.getX() >= 0 && position.getX() < m_boardSize && position.getY() >= 0 && position.getY() < m_boardSize;
}

bool DBHashTable::isPassMove(const Point& position) 
{
	return position.getX() == -1 && position.getY() == -1;
}

vector<Point> DBHashTable::getLoopPositions( /*BaseTree::NodePointer pNode,*/ const int& iLoopDepth)
{
	vector<Point> vLoopPositions;
	if(iLoopDepth > 0) {
		/*
		int depth = iLoopDepth - 1;
		BaseTree::NodePointer loopNode = pNode;
		while(depth--) {
			vLoopPositions.insert(vLoopPositions.begin(), loopNode.getLocation());
			loopNode.jumpToLastPieceOfParentMove();
		}
		*/
	}
	return vLoopPositions;
}

unsigned long long DBHashTable::getAllPieceHashValue(int symmetricType)
{
	unsigned long long ullHashKey = 0;
	for(vector<Point>::iterator blackPosIt = m_vBlackPosition.begin(); blackPosIt != m_vBlackPosition.end(); ++blackPosIt)
		ullHashKey ^= m_symmetricHashTable[getHashTableIndex(*blackPosIt)][symmetricType][PLAYERCOLOR_BLACK];
	for(vector<Point>::iterator whitePosIt = m_vWhitePosition.begin(); whitePosIt != m_vWhitePosition.end(); ++whitePosIt)
		ullHashKey ^= m_symmetricHashTable[getHashTableIndex(*whitePosIt)][symmetricType][PLAYERCOLOR_WHITE];
	return ullHashKey;
}

void DBHashTable::setDefualtSymmetricTypes()
{
	m_vSymmetryTypes.clear();
	m_vSymmetryTypes.push_back(SYM_NORMAL);
	m_vSymmetryTypes.push_back(SYM_ROTATE_90);
	m_vSymmetryTypes.push_back(SYM_ROTATE_180);
	m_vSymmetryTypes.push_back(SYM_ROTATE_270);
	m_vSymmetryTypes.push_back(SYM_HORIZONTAL_REFLECTION);
	m_vSymmetryTypes.push_back(SYM_HORIZONTAL_REFLECTION_ROTATE_90);
	m_vSymmetryTypes.push_back(SYM_HORIZONTAL_REFLECTION_ROTATE_180);
	m_vSymmetryTypes.push_back(SYM_HORIZONTAL_REFLECTION_ROTATE_270);
}

vector<SymmetryType>& DBHashTable::getSymmetricTypes()
{
	return m_vSymmetryTypes;
} 
