#ifndef _DBHASHTABLE_H_
#define _DBHASHTABLE_H_

#include "rand64.h"
#include <algorithm>
#include <vector>
#include "IsomorphicMinHashKey.h"
#include "DbMove.h"
#include "MoveLocation.h"
#include "WeichiBoard.h"
#include "WeichiMove.h"

using std::vector;

typedef int PlayerColor;

const int PLAYERCOLOR_BLACK = 0;
const int PLAYERCOLOR_WHITE = 1;
const int PLAYERCOLOR_NUM = 2;


class DBHashTable
{
protected:
	static const int DEFAULT_BOARD_SIZE = 9;
	static const int MAX_BOARD_SIZE = 19;
	static const int GO_MAX_LOOP = 20;
	static const int HASH_KEY_VARIANCE = 2 + GO_MAX_LOOP;
	static const int SYMMETRIC_TYPE_SIZE = 8;

public:
	DBHashTable(int boardSize = DEFAULT_BOARD_SIZE);
	void initializeZHashKey();
	void creatSymmetricHashTable();
	IsomorphicMinHashkey getBoardHashKey(DbMove* moves, uint moveNum, int iLoopDepth = 0);	
	IsomorphicMinHashkey generateBoardHashKey(PlayerColor color, bool bLastMoveIsPassMove, const vector<Point>& vLoopPositions);
	vector<SymmetryType>& getSymmetricTypes();
protected:
	int getHashTableIndex(const Point& position);
	int getHashTableIndex(const int& x, const int& y);
	bool checkNodeIsPassMove(Point position);
	bool isLegalPosition(Point position);
	bool isPassMove(const Point& position);
	vector<Point> getLoopPositions( /*BaseTree::NodePointer pNode,*/ const int& iLoopDepth);
	virtual unsigned long long getAllPieceHashValue(int symmetricType);
	virtual void setDefualtSymmetricTypes();

private:
	vector<Point> m_vBlackPosition;
	vector<Point> m_vWhitePosition;
	int m_boardSize;
	WeichiBoard m_board;
	OpenAddrHashTable m_ht;
	unsigned long long m_whiteTurnHashValue;
	unsigned long long m_passMoveHashValue[HASH_KEY_VARIANCE];
	unsigned long long m_hashTable[MAX_BOARD_SIZE * MAX_BOARD_SIZE][HASH_KEY_VARIANCE];

protected:
	vector<SymmetryType> m_vSymmetryTypes;
	unsigned long long m_symmetricHashTable[MAX_BOARD_SIZE * MAX_BOARD_SIZE][SYMMETRIC_TYPE_SIZE][HASH_KEY_VARIANCE];
};

#endif