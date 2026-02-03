#include "WeichiClosedArea.h"

void WeichiClosedArea::Clear()
{
	m_id = static_cast<uint>(-1);
}

void WeichiClosedArea::init()
{
	m_color = COLOR_NONE;
	m_numStone = 0;
	m_nakadePos = -1;
	m_bmStone.Reset();
	m_blockIDs.clear();
	m_status = LAD_NOT_EYE;
	m_type = CA_UNKNOWN;
	m_eyeStatus = EYE_UNKNOWN;
	m_bmRZone.Reset() ;	
}