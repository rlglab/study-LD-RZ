#ifndef __SYMMETRICMINHASHKEY_H__
#define __SYMMETRICMINHASHKEY_H__

#include <algorithm>
#include <vector>
#include "MoveLocation.h"

using std::vector;
using std::find;


class IsomorphicMinHashkey
{
	/////////////////////////
	// static function
public:
	static void setStaticCentralPosition(const int& boardSize);
	static MoveLocation getSymmetricCoordinate(const MoveLocation& position, const SymmetryType& rotateAndReflectType);
	static MoveLocation getRestoredSymmetricCoordinate(const MoveLocation& position, const SymmetryType& rotateAndReflectType);
	static void getAllSymmetricCoordinate(const MoveLocation& position, vector<MoveLocation>& vSymmetricLocation);
private:
	static bool shouldReflect(const SymmetryType& rotateAndReflectType);
private:
	static MoveLocation staticCentralPosition;
	/////////////////////////
	// member function
public:
	IsomorphicMinHashkey()
		:m_ullHashKey(0), m_position(MoveLocation(-1, -1)), m_symmetryType(SYM_NORMAL), m_iLoopDepth(0) {}
	IsomorphicMinHashkey(const unsigned long long& ullHashKey, const MoveLocation& position, const SymmetryType& symmetryType, int iLoopDepth = 0)
		:m_ullHashKey(ullHashKey), m_position(position), m_symmetryType(symmetryType), m_iLoopDepth(iLoopDepth) {}
	IsomorphicMinHashkey(const unsigned long long& ullHashKey, const MoveLocation& position, const int& iSymmetryType, int iLoopDepth = 0)
		:m_ullHashKey(ullHashKey), m_position(position), m_symmetryType(static_cast<SymmetryType>(iSymmetryType)), m_iLoopDepth(iLoopDepth) {}
	~IsomorphicMinHashkey(void) {}
	inline void setHashKey(const unsigned long long& ullHashKey) { m_ullHashKey = ullHashKey; }
	inline void setPosition(const MoveLocation& minPosition) { m_position = minPosition; }
	inline void setSymmetryType(const SymmetryType& symmetryType) { m_symmetryType = symmetryType; }
	inline void setLoopDepth(const int& iLoopDepth) { m_iLoopDepth = iLoopDepth; };
	inline unsigned long long getHashKey() const { return m_ullHashKey; }
	inline MoveLocation getMoveLocation() const { return m_position; }
	inline SymmetryType getSymmetryType() const { return m_symmetryType; }
	inline int getLoopDepth() const { return m_iLoopDepth; }
	inline bool isNull() const { return m_ullHashKey == 0 && m_position.isNullMove() && m_symmetryType == SYM_NORMAL && m_iLoopDepth == 0; }
private:
	unsigned long long m_ullHashKey;
	MoveLocation m_position;
	SymmetryType m_symmetryType;
	int m_iLoopDepth;
};

#endif 
