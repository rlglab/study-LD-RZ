#ifndef UCTNODEORDER_H
#define UCTNODEORDER_H

#include "NodePtr.h"
#include "WeichiUctNode.h"

class UctNodeOrder {
	typedef NodePtr<WeichiUctNode> UctNodePtr;
public:
	int m_order;
	UctNodePtr m_pNode;
	UctNodeOrder() {}
	UctNodeOrder( int order, UctNodePtr pNode )
	{
		m_order = order;
		m_pNode = pNode;
	}
	inline bool operator<( const UctNodeOrder& rhs ) const
		{ return ((m_pNode->getUctData().getCount()>rhs.m_pNode->getUctData().getCount())
			|| (m_pNode->getUctData().getCount()==rhs.m_pNode->getUctData().getCount() && m_pNode->getProb()>rhs.m_pNode->getProb())); }
};

#endif 
