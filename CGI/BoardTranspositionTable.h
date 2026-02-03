#ifndef H_BOARD_TT
#define H_BOARD_TT

#include "Timer.h"
#include "TTentry.h"
#include "OpenAddressHashTable.h"

class BoardTranspositionTable {

public:
	BoardTranspositionTable() : m_table(23) { clear(); }

public:
	inline void clear() {
		m_timer.reset();
		m_table.clear();
	}
	inline uint lookup(HashKey64 hashkey) { 
		m_timer.start();
		uint index = m_table.lookup(hashkey); 
		m_timer.stopAndAddAccumulatedTime();
		return index;
	}
	inline void store(HashKey64 hashkey, TTentry entry) { 
		m_timer.start();
		m_table.store(hashkey, entry); 
		m_timer.stopAndAddAccumulatedTime();
	}
	inline TTentry getEntry(uint index){ return m_table.m_entry[index].m_data; }
	inline uint getSize() { return m_table.getCount(); }
	StopTimer& getTimer() { return m_timer; }

private:
	StopTimer m_timer;
	OpenAddressHashTable<TTentry> m_table;
};

#endif