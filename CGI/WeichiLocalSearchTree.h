#ifndef WEICHILOCALSEARCHTREE_H
#define WEICHILOCALSEARCHTREE_H

#include "BasicType.h"
#include "WeichiLocalSearchNode.h"

class WeichiLocalSearchTree
{
private:
	static const int MAX_TREE_NODE = 300;	// limit tree search node, will stop if out of limit

	uint m_numUseNodes;
	Vector<WeichiLocalSearchNode,MAX_TREE_NODE> m_vNodes;

public:
	WeichiLocalSearchTree()
	{
		m_vNodes.resize(MAX_TREE_NODE);
		reset();
	}

	inline void reset()
	{
		m_numUseNodes = 1;
		m_vNodes[0].reset();
	}

	inline WeichiLocalSearchNode* getRootNode() { return &m_vNodes[0]; }
	inline WeichiLocalSearchNode* newNode()
	{
		if( m_numUseNodes==MAX_TREE_NODE ) { return NULL; }
		WeichiLocalSearchNode* node = &m_vNodes[m_numUseNodes++];
		node->reset();

		return node;
	}
};

#endif