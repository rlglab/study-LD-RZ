#ifndef WEICHIPROBABILITYTABLE_H
#define WEICHIPROBABILITYTABLE_H

#include "BasicType.h"
#include "Color.h"
#include "FeatureIndexList.h"
#include "WeichiMove.h"

// for num 3x3 feature & num playout feature
const int MAX_LEGAL_3X3PATTERN_SIZE = 7533;
const int MAX_3X3PATTERN_SIZE = MAX_LEGAL_3X3PATTERN_SIZE + 1;	// add one for illegal 3x3 pattern
const int MAX_NUM_PLAYOUT_FEATURES = 2*MAX_3X3PATTERN_SIZE + POLICY_SIZE;
const int MAX_NUM_PLAYOUT_FEATURE_GROUPS = 2 + POLICY_SIZE;	// add two for pattern & local_pattern

class ProbabilityGrid {
private:
	bool m_bIsUsed;
	bool m_bIsSkipGrid;
	double m_dScore;
	double m_dPatternScore;
	uint m_patternIndex;
	WeichiPlayoutFeatureBits m_bmFeatures;

public:
	ProbabilityGrid() {}
	inline void clear() {
		m_bIsUsed = true;
		m_bIsSkipGrid = false;
		m_dScore = m_dPatternScore = 0.0f;
		m_patternIndex = 0;
		m_bmFeatures.Reset();
	}
	inline void setPatternScore( const uint iFeatureIndex, const double dScore ) {
		m_bIsUsed = true;
		m_bIsSkipGrid = false;
		m_patternIndex = iFeatureIndex;
		m_dPatternScore = m_dScore = dScore;
	}
	inline void addAdditionalFeature( const uint iFeatureIndex, const double dScore ) {
		m_dScore *= dScore;
		m_bmFeatures.SetBitOn(iFeatureIndex);
	}
	inline void addAdditionalPatternFeature( const double dScore ) {
		if (m_dPatternScore == 0) { m_dScore = dScore; }
		else { m_dScore *= dScore / m_dPatternScore; }
	}
	inline void recover( Color turnColor ) {
		m_bIsUsed = true;
		m_bIsSkipGrid = false;
		m_bmFeatures.Reset();
		m_dScore = m_dPatternScore;
	}

	inline bool isUsed() const { return m_bIsUsed; }
	inline void setIsUsed( bool bIsUsed ) { m_bIsUsed = bIsUsed; }
	inline bool isSkipGrid() const { return m_bIsSkipGrid; }
	inline void setIsSkipGird( bool bIsSkipGird ) { m_bIsSkipGrid = bIsSkipGird; }
	inline double getScore() const { return m_dScore; }
	inline double getPatternScore() const { return m_dPatternScore; }
	inline uint getPatternIndex() const { return m_patternIndex; }
	inline WeichiPlayoutFeatureBits& getPlayoutFeatureBits() { return m_bmFeatures; }
	inline const WeichiPlayoutFeatureBits& getPlayoutFeatureBits() const { return m_bmFeatures; }
	inline Vector<uint,MAX_NUM_PLAYOUT_FEATURE_GROUPS> getFeatureIndexs() const
	{
		uint iFeature;
		WeichiPlayoutFeatureBits bmFeatures = m_bmFeatures;
		Vector<uint,MAX_NUM_PLAYOUT_FEATURE_GROUPS> vFeatures;
		while( (iFeature=bmFeatures.bitScanForward())!=-1 ) {
			vFeatures.push_back(iFeature+2*MAX_3X3PATTERN_SIZE);
		}

		if( vFeatures.size()>0 ) { vFeatures.push_back(m_patternIndex+MAX_3X3PATTERN_SIZE); }
		else { vFeatures.push_back(m_patternIndex); }

		return vFeatures;
	}
};

class WeichiProbabilityTable {
	friend class WeichiProbabilityPlayoutHandler;
private:
	double m_dMaxScore;
	double m_dTotalScore;
	Vector<double,MAX_BOARD_SIZE> m_vRowScore;
	Vector<ProbabilityGrid,MAX_NUM_GRIDS> m_vProbGrids;

public:
	WeichiProbabilityTable() {}

	void reset();
	inline void replaceMaxScore( double dScore ) { m_dMaxScore = (dScore>m_dMaxScore)? dScore: m_dMaxScore; }
	inline void setMaxScore( double dScore ) { m_dMaxScore = dScore; }
	inline double getMaxScore() const { return m_dMaxScore; }
	inline double getRowScore( int row ) const { return m_vRowScore[row]; }
	inline void setRowScore( int row, double dScore ) { m_vRowScore[row] = dScore; }
	inline ProbabilityGrid& getProbGrid( uint position ) { return m_vProbGrids[position]; }
	inline const ProbabilityGrid& getProbGrid( uint position ) const { return m_vProbGrids[position]; }
	inline Vector<ProbabilityGrid,MAX_NUM_GRIDS>& getAllProbGrid() { return m_vProbGrids; }
	inline const Vector<ProbabilityGrid,MAX_NUM_GRIDS>& getAllProbGrid() const { return m_vProbGrids; }
	inline double getTotalScore() const { return m_dTotalScore; }
	inline void setTotalScore( double dScore ) { m_dTotalScore = dScore; }

	int chooseOneMoveBySoftMax(const WeichiBitBoard& bmStone) const;

private:
	bool invariance() const;
};

#endif