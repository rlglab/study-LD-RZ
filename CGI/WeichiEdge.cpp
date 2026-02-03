#include "WeichiEdge.h"
#include "WeichiBoard.h"

void WeichiEdge::Clear()
{
	m_id = static_cast<uint>(-1);
}

void WeichiEdge::init()
{
	m_iAnotherEndStonePos = -1;
	m_bmRealStone.Reset();
	m_bmEndStone.Reset();
	m_bmErodedStone.Reset();
	m_bmVirtualCAStone.Reset();
	m_bmCAStone.Reset();
	m_bmThreatStone.Reset();
	m_edgeBits.Reset();
}

std::string WeichiEdge::getEdgeInformationString() const
{
	ostringstream oss;
	oss << "another end stone position: " << m_iAnotherEndStonePos << endl;
	oss << "Real stone map: " << endl << m_bmRealStone.toString() << endl;
	oss << "End stone map: " << endl << m_bmEndStone.toString() << endl;
	oss << "Eroded stone map: " << endl << m_bmErodedStone.toString() << endl;
	oss << "VirtualCA stone map: " << endl << m_bmVirtualCAStone.toString() << endl;
	oss << "CA stone map: " << endl << m_bmCAStone.toString() << endl;
	oss << "Threat stone map: " << endl << m_bmThreatStone.toString() << endl;
	oss << "Edge bits map: " << endl << m_edgeBits.toString() << endl;	

	return oss.str();
}