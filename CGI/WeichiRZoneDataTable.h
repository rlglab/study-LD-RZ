#ifndef WEICHI_RZONEDATATABLE_H
#define WEICHI_RZONEDATATABLE_H
#include "boost/atomic.hpp"
#include "WeichiMove.h"
#include "WeichiConfigure.h"
#include "WeichiUctNode.h"
#include "NodePage.h"
#include "WeichiCNNNetType.h"
#include "WeichiRZoneDataTableEntry.h"
#include <bitset>
class WeichiRZoneDataTable;
class WeichiRZoneDataTable
{
	typedef int DTIndex;
	typedef NodePtr<WeichiUctNode> UctNodePtr;

private:
	std::vector<WeichiRZoneDataTableEntry> m_table;	
	int m_baseSize;
	boost::atomic<int> m_end;
public:
	WeichiRZoneDataTable() { reset(); }

	void reset() {
		//m_baseSize = static_cast<int>(node_manage::PageSize * Configure::MaxPageNum)/8;
		m_baseSize = 1000000;
		m_table.resize(m_baseSize);
		m_end = 0;
	}

	DTIndex createRZoneData(WeichiBitBoard bmRZone, const WeichiBoard& board)
	{
		m_table[m_end].setRZone(bmRZone);
		m_table[m_end].setRZoneStone(board.getStoneBitBoard(COLOR_BLACK)&bmRZone, COLOR_BLACK);
		m_table[m_end].setRZoneStone(board.getStoneBitBoard(COLOR_WHITE)&bmRZone, COLOR_WHITE);
		DTIndex newEnd = m_end.load() + 1;
		if (newEnd == m_baseSize) {
			CERR() << "[Full RZoneData Table!]" << endl;
			exit(0);			
		}
		m_end.store(newEnd);
		return (m_end.load() - 1);
	}

	void setRZoneData(DTIndex id, WeichiBitBoard bmRZone)
	{
		m_table[id].setRZone(bmRZone);
		return;
	}

	void appendRZoneData(DTIndex id, WeichiBitBoard bmRZone, const WeichiBoard& board)
	{
		m_table[id].addRZone(bmRZone);
		m_table[id].addRZoneStone(board.getStoneBitBoard(COLOR_BLACK)&bmRZone, COLOR_BLACK);
		m_table[id].addRZoneStone(board.getStoneBitBoard(COLOR_WHITE)&bmRZone, COLOR_WHITE);
		return;
	}

	inline WeichiBitBoard getRZone(DTIndex id) {
		if (id != -1) { return m_table[id].getRZone(); }
		return WeichiBitBoard();
	}

	inline WeichiBitBoard getRZoneStone(DTIndex id, Color c) {
		if (id != -1) { return m_table[id].getRZoneStone(c); }
		return WeichiBitBoard();
	}

	inline Dual<WeichiBitBoard> getRZoneStone(DTIndex id) {
		if (id != -1) { return m_table[id].getRZoneStone(); }
		return Dual<WeichiBitBoard>();
	}

	inline int getSize() const { return m_end; }
	inline int getMaximum() const { return m_baseSize; }
};


#endif  //WEICHI_RZONEDATATABLE_H
 
