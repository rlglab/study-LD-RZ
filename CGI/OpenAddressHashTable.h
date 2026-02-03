#ifndef OPENADDRESSHASHTABLE_H
#define OPENADDRESSHASHTABLE_H

#include "HashKey64.h"

template<class _data> class OpenAddressHashTable;
template<class _data> class OpenAddressHashTableEntry {
	friend class OpenAddressHashTable<_data>;
private:
	bool m_bIsFree;
	HashKey64 m_key;

public:
	_data m_data;
	OpenAddressHashTableEntry() : m_bIsFree(true) {}
	inline void setEntry(HashKey64 key) { m_key = key; }
	inline void clear()
	{
		m_data.clear();
		m_bIsFree = true;
	}
	inline HashKey64 getHashKey() const { return m_key; }
};

template<class _data> class OpenAddressHashTable {
	typedef unsigned int IndexType;
private:
	IndexType m_count;
	const IndexType m_mask;
	const size_t m_size;

public:
	OpenAddressHashTableEntry<_data>* m_entry;

	OpenAddressHashTable(int bitSize = 12)
		: m_mask((1 << bitSize) - 1)
		, m_count(0)
		, m_size(1 << bitSize)
		, m_entry(new OpenAddressHashTableEntry<_data>[1ULL << bitSize])
	{}

	~OpenAddressHashTable() { delete[] m_entry; }

	IndexType getCount() const { return m_count; }

	IndexType lookup(const HashKey64& key) const
	{
		IndexType index = static_cast<IndexType>(key)&m_mask;

		while (true) {
			const OpenAddressHashTableEntry<_data>& entry = m_entry[index];

			if (!entry.m_bIsFree) {
				if (entry.m_key == key) { return index; }
				else { index = (index + 1)&m_mask; }
			} else {
				return -1;
			}
		}
		return -1;
	}

	void store(const HashKey64& key, const _data& data)
	{
		IndexType index = static_cast<IndexType>(key)&m_mask;
		while (true) {
			OpenAddressHashTableEntry<_data>& entry = m_entry[index];

			if (entry.m_bIsFree) {
				entry.m_key = key;
				entry.m_data = data;
				entry.m_bIsFree = false;
				m_count++;
				break;
			} else {
				index = (index + 1)&m_mask;
			}
		}
	}

	void clear()
	{
		m_count = 0;
		for (uint i = 0; i < m_size; ++i) { m_entry[i].clear(); }
	}
};

#endif  // OPENADDRESSHASHTABLE_H 
