#include "WeichiLocalSequence.h"

void WeichiLocalSequence::Clear()
{
	m_id = static_cast<uint>(-1);
}

void WeichiLocalSequence::init( WeichiLocalSearchNode* rootNode )
{
	m_result = rootNode->getResult();
	m_success.clear();
	m_failed.clear();
	m_unknown.clear();

	WeichiLocalSearchNode* childNode = rootNode->getChild();
	while( childNode!=NULL ) {
		const uint pos = childNode->getMove().getPosition();
		if( childNode->getResult()==RESULT_SUCCESS ) { m_success.addFeature(pos); }
		else if( childNode->getResult()==RESULT_FAILED ) { m_failed.addFeature(pos); }
		else { m_unknown.addFeature(pos); }
		
		childNode = childNode->getNextSibling();
	}
}

void WeichiLocalSequence::saveLocalSequence( WeichiLocalSearchNode* rootNode )
{
	// we only interest success sequence
	if( rootNode->getResult()!=RESULT_SUCCESS ) { return; }

	WeichiLocalSearchNode* childNode = rootNode->getChild();
	while( childNode!=NULL ) {
		if( childNode->getResult()==RESULT_SUCCESS ) { break; }
		childNode = childNode->getNextSibling();
	}

	traversalLocalSearchTree(childNode,true);
}

void WeichiLocalSequence::traversalLocalSearchTree( WeichiLocalSearchNode* parentNode, bool bFindSuccesss )
{
	assert( parentNode );

	if( (bFindSuccesss && parentNode->getResult()!=RESULT_SUCCESS) ||
		(!bFindSuccesss && parentNode->getResult()!=RESULT_FAILED) )
	{
		return;
	}

	CERR() << ' ' << toChar(parentNode->getMove().getColor()) << ' ' << parentNode->getMove().toGtpString() << endl;

	WeichiLocalSearchNode* childNode = parentNode->getChild();
	while( childNode!=NULL ) {
		traversalLocalSearchTree(childNode,!bFindSuccesss);
		childNode = childNode->getNextSibling();
	}
}