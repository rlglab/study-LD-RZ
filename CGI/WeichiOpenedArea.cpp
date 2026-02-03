#include "WeichiOpenedArea.h"

void WeichiOpenedArea::Clear()
{
	m_id = static_cast<uint>(-1);
}

void WeichiOpenedArea::init()
{
	//m_checkIdx = 0;
	m_color = COLOR_NONE;
	m_numStone = 0;	
	m_bmStone.Reset();	
	//m_bmVirtualStone.Reset() ;	
	m_blockIDs.clear();	
	//m_type = AREA_UNKNOWN ;
	m_eyeStatus = EYE_UNKNOWN ;	
	//m_bmRelevantArea.Reset() ;
	m_bmRZone.Reset() ;
}