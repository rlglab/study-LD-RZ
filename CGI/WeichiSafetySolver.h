#ifndef H_WEICHI_SAFETY_SOLVER
#define H_WEICHI_SAFETY_SOLVER

#include "BasicType.h"
#include "Color.h"
#include "Dual.h"
#include "WeichiBoard.h"

class WeichiSafetySolver 
{
public:
	WeichiSafetySolver() {}
	void reset();
	bool isRegionSurroundedBySafeBlocks(const WeichiClosedArea* region, WeichiThreadState& state);
	bool isRegionSafe(const WeichiClosedArea* region, WeichiThreadState& state);
	void computeFullBoardSafeBlocks(WeichiThreadState& state);
	void computeRegionSafeBlocks(const WeichiClosedArea* region, WeichiThreadState& state);
	void computeEyeSpace(const WeichiClosedArea* region, WeichiThreadState& state);
	void checkNbr(const WeichiGrid& grid, WeichiThreadState& state);

	inline WeichiBitBoard getEyeSpace() const { return m_bmEyeSpace; }
private:
	Color m_regionColor;
	WeichiBitBoard m_bmSafeBlock;
	WeichiBitBoard m_bmEyeSpace;
	WeichiBitBoard m_bmEyePoint;
};

#endif 