#ifndef WEICHIGAMMATABLE_H
#define WEICHIGAMMATABLE_H

#include "BasicType.h"
#include "RadiusGammaOpenAddrHashTable.h"
#include "WeichiRadiusPatternTable.h"

class WeichiGammaTable {
public:
	static uint MAX_NUM_FEATURES;
	static const int MAX_LFR_DIMENSION = 10;
private:
	static int m_dimension;
	static double *m_dMMFeatureValue;
	static double *m_dLFRFeatureValue;
	static double *m_dPlayoutFeatureValue;
	static double **m_dLFRFeatureInteractionValue;
	static RadiusGammaOpenAddrHashTable m_htRadiusPatternData;
	static int m_vNumRadiusPattern[MAX_RADIUS_SIZE];
	
public:
	WeichiGammaTable() {}
	static inline int getDimension() { return m_dimension; }
	static inline double getLFRBias() { return m_dLFRFeatureValue[0]; }
	static inline double getMMFeatureValue( uint index ) { assert( index<MAX_NUM_FEATURES ); return m_dMMFeatureValue[index]; }
	static inline double getLFRFeatureValue( uint index ) { assert( index<MAX_NUM_FEATURES ); return m_dLFRFeatureValue[index]; }
	static inline double getLFRFeautreInteractionValue( uint index, int dimension ) { assert( index<MAX_NUM_FEATURES && dimension<m_dimension ); return m_dLFRFeatureInteractionValue[index][dimension]; }
	static inline double getPlayoutFeatureValue( uint index ) { return m_dPlayoutFeatureValue[index]; }
	static inline void setPlayoutFeatureValue( uint index, double dValue ) { m_dPlayoutFeatureValue[index] = dValue; }
	static inline uint getRadiusPatternIndex( HashKey64 key ) { return m_htRadiusPatternData.getIndex(key); }
	static inline uint getRadiusPatternCount( int iRadius ) { return m_vNumRadiusPattern[iRadius]; }
	static inline void setRadiusPatternCount( int iRadius, int iCount ) { m_vNumRadiusPattern[iRadius] = iCount; }
	static void initailize();

private:
	static void loadRadiusPatternTable();
	static void initializeFeatureValue();
	static void loadMMDataBase();
	static void loadLFRDataBase();
	static void loadPlayoutFeatureDataBase();
};

#endif