#ifndef RZONE_DATATABLEENTRY_H
#define RZONE_DATATABLEENTRY_H
#include "boost/atomic.hpp"
#include "WeichiMove.h"
#include "WeichiConfigure.h"
#include "WeichiUctNode.h"
#include "BasicType.h"
class WeichiRZoneDataTableEntry
{
public:
	WeichiRZoneDataTableEntry() { reset(); }
	WeichiRZoneDataTableEntry(WeichiBitBoard bm) { m_bmRZone = bm; }
	inline void reset() { m_bmRZone.Reset(); }
	inline void setRZone(WeichiBitBoard bmRZone) { m_bmRZone = bmRZone; }
	inline void setRZoneStone(WeichiBitBoard bmRZone, Color c) { m_bmRZoneStone.get(c) = bmRZone; }
	inline void addRZone(WeichiBitBoard bmRZone) { m_bmRZone |= bmRZone; }
	inline void addRZoneStone(WeichiBitBoard bmRZone, Color c) { m_bmRZoneStone.get(c) |= bmRZone; }
	inline WeichiBitBoard getRZone() const { return m_bmRZone; }
	inline Dual<WeichiBitBoard> getRZoneStone() const { return m_bmRZoneStone; }
	inline WeichiBitBoard getRZoneStone(Color c) const { return m_bmRZoneStone.get(c); }
	inline void setRZoneBitOn(int pos) { m_bmRZone.SetBitOn(pos); }
	inline void setRZoneBitOff(int pos) { m_bmRZone.SetBitOff(pos); }

private:
	WeichiBitBoard m_bmRZone;
	Dual<WeichiBitBoard> m_bmRZoneStone;

};
#endif