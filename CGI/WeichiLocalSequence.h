#ifndef WEICHILOCALSEQUENCE_H
#define WEICHILOCALSEQUENCE_H

#include "WeichiLocalSearchNode.h"
#include "FeatureIndexList.h"

class WeichiLocalSequence
{
private:
	uint m_id;
	mutable uint m_checkIdx;
	WeichiBlockSearchResult m_result;
	FeatureIndexList<short, MAX_NUM_GRIDS> m_failed;
	FeatureIndexList<short, MAX_NUM_GRIDS> m_success;
	FeatureIndexList<short, MAX_NUM_GRIDS> m_unknown;

public:
	WeichiLocalSequence() { Clear(); }

	void Clear();
	void init( WeichiLocalSearchNode* rootNode );

	inline uint GetID() const { return m_id; }
	inline void SetID( uint id ) { m_id = id ; }

	inline uint getCheckIdx() const { return m_checkIdx; }
	inline void setCheckIdx( uint idx ) const { m_checkIdx = idx ; }

	inline WeichiBlockSearchResult getResult() const { return m_result; }

	inline uint getNumFailed() const { return m_failed.size(); }
	inline uint getNumSuccess() const { return m_success.size(); }
	inline uint getNumUnknown() const { return m_unknown.size(); }

	inline uint getFailedPos( uint iIndex ) const { return m_failed[iIndex]; }
	inline uint getSuccessPos( uint iIndex ) const { return m_success[iIndex]; }
	inline uint getUnknownPos( uint iIndex ) const { return m_unknown[iIndex]; }

	void saveLocalSequence( WeichiLocalSearchNode* rootNode );

private:
	void traversalLocalSearchTree( WeichiLocalSearchNode* parentNode, bool bFindSuccesss );
};

#endif