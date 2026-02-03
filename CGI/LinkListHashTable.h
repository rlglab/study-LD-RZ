#ifndef LINKLISTHASHTABLE_H
#define LINKLISTHASHTABLE_H

#include "HashKey64.h"
#include "boost/atomic.hpp"

template<class _data> class LinkListHashTable;
template<class _data> class LinkListHashTableEntry
{
	friend class LinkListHashTable<_data>;
private:
	typedef unsigned int IndexType; //IndexType -1 = NULL
	HashKey64 m_key;
	IndexType m_preID, m_nextID;
	int m_validID;
public:
	_data  m_data;
	LinkListHashTableEntry()
		:m_preID(-1),m_nextID(-1),m_validID(-1)
	{
	}
	void setEntry(HashKey64 key,IndexType preID = -1,IndexType nextID = -1)
	{
		m_key = key;
		m_preID = preID;
		m_nextID = nextID;
	}
	inline void clear()
	{
		m_data.clear();
		m_preID = m_nextID = m_validID = -1;
	}
	inline HashKey64 getHashKey() const { return m_key; }
	inline IndexType getNextID() const { return m_nextID; }
};

template<class _data> class LinkListHashTable
{
	typedef unsigned int IndexType;
private:
	class LinkListHashTableHeader
	{
		friend class LinkListHashTable<_data>;
	private:
		typedef int IndexType;
		IndexType m_front;
		boost::atomic<bool> m_writeToken;
	public:
		LinkListHashTableHeader()
			: m_front(-1)
			, m_writeToken(false)
		{}
		void clear()
		{
			m_front = -1;
			m_writeToken = false;
		}
	};

	const IndexType m_mask;
	const size_t m_tableSize;
	const size_t m_entrySize;
	LinkListHashTableHeader* m_hashTableHeader;
	boost::atomic<bool> m_poolToken;
	IndexType* m_entryPool;
	IndexType m_entryPoolIndex;
	IndexType* m_validEntry;
	IndexType m_validEntryCount;
public:
	LinkListHashTableEntry<_data>* m_entry;
	LinkListHashTable ( int tableBitSize=21, int entryBitSize=21+2 )
		: m_mask ( (1<<tableBitSize)-1 )
		, m_tableSize (1<<tableBitSize)
		, m_entrySize (1ULL<<entryBitSize)
		, m_hashTableHeader (new LinkListHashTableHeader[1ULL<<tableBitSize])
		, m_poolToken (false)
		, m_entry (new LinkListHashTableEntry<_data>[1ULL<<entryBitSize])
		, m_entryPool (new IndexType[1ULL<<entryBitSize])
		, m_entryPoolIndex (1<<entryBitSize)
		, m_validEntry (new IndexType[1ULL<<entryBitSize])
		, m_validEntryCount (0)
	{
		for( IndexType i=0; i<m_entryPoolIndex; i++ ) { m_entryPool[i] = i; }
	}

	~LinkListHashTable()
	{
		delete[] m_hashTableHeader;
		delete[] m_entry;
		delete[] m_entryPool;
	}

	void clear()
	{
		while( m_poolToken.exchange(true)==true ) {}

		m_validEntryCount = 0;
		m_entryPoolIndex = static_cast<IndexType>(m_entrySize);
		for( IndexType i=0; i<m_entrySize; i++ ) { m_entryPool[i] = i; }
		for( IndexType i=0; i<m_tableSize; i++ ) { m_hashTableHeader[i].clear(); }

		m_poolToken.store(false);
	}

	IndexType lookup( HashKey64 key ) const 
	{
		IndexType index = static_cast<IndexType>(key)&m_mask ;
		IndexType nowEntryID = m_hashTableHeader[index].m_front;
		while( nowEntryID!=-1 ) {
			LinkListHashTableEntry<_data>& entry = m_entry[nowEntryID];
			if(entry.m_key == key) {
				return nowEntryID;
			} else {
				nowEntryID = entry.m_nextID;
			}
		}
		return -1;
	}

	IndexType lookupWithInsert( HashKey64 key, bool& bHasInsert ) 
	{
		if( isTableFull() ) { return -1; }

		bHasInsert = false;
		IndexType index = static_cast<IndexType>(key)&m_mask;
		getWriteToken(index);

		IndexType nowEntryID = m_hashTableHeader[index].m_front;
		if( nowEntryID==-1 )	 {
			IndexType newEntryID = getNewEntry();
			if( newEntryID==-1 ) { return newEntryID; }
			m_entry[newEntryID].setEntry(key);
			m_hashTableHeader[index].m_front = newEntryID;
			releaseWriteToken(index);
			bHasInsert = true;
			return newEntryID;
		}
		while( true ) {
			LinkListHashTableEntry<_data>& entry = m_entry[nowEntryID];
			if( entry.m_key==key ) {
				releaseWriteToken(index);
				return nowEntryID;
			} else if( entry.m_nextID!=-1 ) {
				nowEntryID = entry.m_nextID;
			} else {
				// case for entry.m_nextID==-1
				IndexType newEntryID = getNewEntry();
				if( newEntryID==-1 ) { return newEntryID; }
				entry.m_nextID = newEntryID;
				m_entry[newEntryID].setEntry(key,nowEntryID);
				releaseWriteToken(index);
				bHasInsert = true;
				return newEntryID;
			}
		}
	}

	bool erase( HashKey64 key ) 
	{
		IndexType entryID = lookup(key);
		if( entryID==-1 ) { return false; }
		erase(entryID);
		return true;
	}

	void erase( IndexType entryID )
	{
		if( entryID==-1 ) { return; }
		HashKey64 key = m_entry[entryID].m_key;
		IndexType index = static_cast<IndexType>(key)&m_mask ;
		getWriteToken(index);
		if( m_hashTableHeader[index].m_front==entryID ) {
			m_hashTableHeader[index].m_front = m_entry[entryID].m_nextID;
			if( m_hashTableHeader[index].m_front!=-1 ) {
				m_entry[m_hashTableHeader[index].m_front].m_preID=-1;
			}
		} else {
			// m_entry[entryID].preID != -1
			IndexType preID = m_entry[entryID].m_preID;
			IndexType nextID = m_entry[entryID].m_nextID;
			m_entry[preID].m_nextID = nextID;
			if( nextID!=-1 ) { m_entry[nextID].m_preID = preID; }
		}
		putBackEntry(entryID);
		releaseWriteToken(index);
	}

	// return num of del
	int cleanHashTable( bool (*checkNeedDel)(_data) )
	{
		int cnt = 0;
		for( IndexType i=0; i<m_tableSize; i++ ) {
			getWriteToken(i);
			IndexType* pEntryID = &m_hashTableHeader[i].m_front;
			while( (*pEntryID)!=-1 ) {
				LinkListHashTableEntry<_data>& entry = m_entry[*pEntryID];
				if( (*checkNeedDel)(entry.m_data)==true ) {
					erase(*pEntryID);
					cnt++;
				} else {
					*pEntryID = entry.m_nextID;
				}
			}
			releaseWriteToken(i);
		}
		return cnt;
	}

	inline IndexType getValidEntryCount() const { return m_validEntryCount; }
	inline IndexType getValidEntryID( int idx ) const { return m_validEntry[idx]; }
	inline bool isTableFull() const { return (m_entryPoolIndex==0); }
	inline uint getEntrySize() const { return static_cast<uint>(m_entrySize); }
	inline double getUseRate() const { return (static_cast<double>(m_validEntryCount)*100.0/static_cast<double>(m_entrySize)); }

private:
	void getWriteToken(IndexType index)
	{
		while( m_hashTableHeader[index].m_writeToken.exchange(true)==true ) {
			// busy waiting
		}
	}
	void releaseWriteToken(IndexType index)
	{
		m_hashTableHeader[index].m_writeToken.store(false);
	}
	IndexType getNewEntry()
	{
		while( m_poolToken.exchange(true)==true ) {}

		if( isTableFull() ) {
			m_poolToken.store(false);
			return -1; 
		}

		IndexType entryID = m_entryPool[m_entryPoolIndex-1];
		m_entry[entryID].clear();
		m_entryPoolIndex--;
		m_validEntry[m_validEntryCount] = entryID;
		m_entry[entryID].m_validID = m_validEntryCount;
		m_validEntryCount++;

		m_poolToken.store(false);
		return entryID;
	}
	void putBackEntry( IndexType entryID )
	{
		while( m_poolToken.exchange(true)==true ) {}

		m_entryPool[m_entryPoolIndex] = entryID;
		m_entryPoolIndex++;
		int validID = m_entry[entryID].m_validID;
		m_validEntry[validID] = m_validEntry[m_validEntryCount-1];
		m_entry[m_validEntry[validID]].m_validID = validID;
		m_validEntryCount--;
		m_entry[entryID].clear();

		m_poolToken.store(false);
	}
};

#endif  //LINKLISTHASHTABLE_H