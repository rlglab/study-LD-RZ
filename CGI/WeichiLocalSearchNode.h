#ifndef WEICHILOCALSEARCHNODE_H
#define WEICHILOCALSEARCHNODE_H

#include "BasicType.h"
#include "WeichiMove.h"

class WeichiLocalSearchNode
{
private:
	WeichiMove m_move;
	WeichiBlockSearchResult m_result;
	WeichiLocalSearchNode* m_child;
	WeichiLocalSearchNode* m_nextSibling;

public:
	WeichiLocalSearchNode() { reset(); }

	inline void reset()
	{
		m_move = PASS_MOVE;
		m_result = RESULT_UNKNOWN;
		m_child = m_nextSibling = NULL;
	}

	inline void setMove( const WeichiMove& move ) { m_move = move; }
	inline void setResult( WeichiBlockSearchResult result ) { m_result = result; }
	inline void setChild( WeichiLocalSearchNode* child ) { m_child = child; }
	inline void setNextSibling( WeichiLocalSearchNode* nextSibling ) { m_nextSibling = nextSibling; }

	inline WeichiMove getMove() const { return m_move; }
	inline WeichiBlockSearchResult getResult() const { return m_result; }
	inline WeichiLocalSearchNode* getChild() const { return m_child; }
	inline WeichiLocalSearchNode* getNextSibling() const { return m_nextSibling; }
};

#endif