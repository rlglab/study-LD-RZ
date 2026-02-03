#ifndef WEICHICNNFEATUREGENERATOR_H
#define WEICHICNNFEATUREGENERATOR_H

#include "BasicType.h"
#include "BaseCNNNet.h"
#include "WeichiThreadState.h"

class WeichiCNNFeatureGenerator
{
	friend class TMiniMCTS;

public:
	WeichiCNNFeatureGenerator() {}

	static void calDCNNFeatureAndAddToNet( WeichiThreadState& state, BaseCNNNet* net, SymmetryType type=SYM_NORMAL );
	static void calculateDCNNFeatures( WeichiThreadState& state, WeichiCNNFeatureType featureType, float* inputFeatures, WeichiBitBoard& bmLegal );
	static void addToDCNNNet( BaseCNNNet* net, float* features, Color turnColor, WeichiBitBoard& bmLegal, SymmetryType type=SYM_NORMAL );

private:
	static void calculateG49Features(WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal);
	static void calculateG50Features(WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal);
	static void calculateG51Features(WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal);
	static void calculateF16Features(WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal);
	static void calculateG17Features(WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal);
	static void calculateC49Features(WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal);
	static void calculateC51Features(WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal);
	static void calculateC52Features(WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal);
	static void calculateF18Features(WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal);
	static void calculateC148Features(WeichiThreadState& state, float features[][MAX_NUM_GRIDS], WeichiBitBoard& bmLegal);

	static void calculateTurnColorFeature(WeichiThreadState& state, float features[]);
	static void calculateBorderFeature(WeichiThreadState& state, float features[], WeichiBitBoard& bmLegal);
};

#endif