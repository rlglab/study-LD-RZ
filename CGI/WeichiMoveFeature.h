#ifndef WEICHIMOVEFEATURE_H
#define WEICHIMOVEFEATURE_H

#include "BasicType.h"
#include "BitBoard.h"
#include "WeichiGammaTable.h"

class WeichiMoveFeature {
private:
	uint m_numFeatures;
	double m_dMMScore;
	double m_dLFRScore;
	WeichiFixedFeatureBits m_fixedFeatureBits;
	Vector<uint,MAX_RADIUS_SIZE> m_vRadiusPatternIndex;
	Vector<double,WeichiGammaTable::MAX_LFR_DIMENSION> m_vLFRInteraction;

public:
	WeichiMoveFeature() {}
	inline void clear()
	{
		m_numFeatures = 0;
		m_fixedFeatureBits.Reset();
		m_vRadiusPatternIndex.clear();

		if( WeichiConfigure::mcts_use_mm ) { m_dMMScore = 1.0; }
		if( WeichiConfigure::mcts_use_lfr ) {
			m_dLFRScore = WeichiGammaTable::getLFRBias();
			m_vLFRInteraction.setAllAs(0.0,WeichiGammaTable::getDimension());
		}
	}
	inline void addFixedFeature( WeichiMoveFeatureType featureType )
	{
		assert( featureType==MF_NOT_IN_CANDIDATE || !m_fixedFeatureBits.BitIsOn(featureType) );
		++m_numFeatures;
		m_fixedFeatureBits.SetBitOn(featureType);
		addFeatureScore(featureType);
	}
	inline void addRadiusPatternFeature( uint index )
	{
		++m_numFeatures;
		m_vRadiusPatternIndex.push_back(index);
		addFeatureScore(index);
	}
	inline void addFeatureScore( uint index )
	{
		if( WeichiConfigure::mcts_use_mm ) {
			m_dMMScore *= WeichiGammaTable::getMMFeatureValue(index);
		}
		if( WeichiConfigure::mcts_use_lfr ) {
			m_dLFRScore += WeichiGammaTable::getLFRFeatureValue(index);
			for( int k=0; k<WeichiGammaTable::getDimension(); k++ ) {
				m_dLFRScore += m_vLFRInteraction[k]*WeichiGammaTable::getLFRFeautreInteractionValue(index,k);
				m_vLFRInteraction[k] += WeichiGammaTable::getLFRFeautreInteractionValue(index,k);
			}
		}
	}
	inline double getScore() const
	{
		if( WeichiConfigure::mcts_use_mm && WeichiConfigure::mcts_use_lfr ) { return m_dMMScore*m_dLFRScore; }
		else if( WeichiConfigure::mcts_use_mm ) { return m_dMMScore; }
		else if( WeichiConfigure::mcts_use_lfr ) { return m_dLFRScore; }
		else { return 0.0f; }
	}
	inline bool hasFixedFeature( WeichiMoveFeatureType featureType ) const { return m_fixedFeatureBits.BitIsOn(featureType); }
	inline WeichiMoveFeatureType scanForwardFixedFeature() { return static_cast<WeichiMoveFeatureType>(m_fixedFeatureBits.bitScanForward()); }
	inline uint getRadiusPatternFeatureSize() const { return m_vRadiusPatternIndex.size(); }
	inline uint getRadiusPatternFeature( uint index ) const { return m_vRadiusPatternIndex[index]; }
	inline uint getNumFeature() const { return m_numFeatures; }
};

#endif