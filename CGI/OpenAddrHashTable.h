#ifndef OPENADDRHASHTABLE_H
#define OPENADDRHASHTABLE_H

#include "HashKey64.h"
/// this file is specialized implement for super-ko rule in GO

class OAEntry 
{
public:
	OAEntry ( ) : m_isFree ( true ) {}
	HashKey64 m_key;
	bool m_isFree ;
};

class OpenAddrHashTable
{
	typedef unsigned int IndexType ;
private:
	const IndexType m_mask ;
	const size_t m_size ;
	OAEntry* m_entry ;
#ifndef NDEBUG
	uint m_count ;
	uint m_maxjump ;
#endif
public:
	OpenAddrHashTable ( int bitSize = 12 ) 
		: m_mask ( (1<<bitSize)-1 )
		, m_size ( 1<<bitSize ) 
		, m_entry ( new OAEntry[1ULL<<bitSize] )
#ifndef NDEBUG
		, m_count ( 0 )
		, m_maxjump ( 0 )
#endif 
	{
	}

	~OpenAddrHashTable()
	{
		delete[] m_entry;
	}

	bool lookup ( HashKey64 key ) const 
	{
		assert(invariance()) ;
		IndexType index = static_cast<IndexType>(key)&m_mask ;

		while ( true ) {
			const OAEntry& entry = m_entry[index];

			if ( !entry.m_isFree ) {
				if ( entry.m_key == key ) return true;
				else index = (index+1)&m_mask;
			} else {
				return false ;
			}
		}
		return false;
	}
	void store ( HashKey64 key ) 
	{
		assert(invariance()) ;
		IndexType index = static_cast<IndexType>(key)&m_mask ;
#ifndef NDEBUG
		uint jump_count = 0 ;
		assert ( !lookup(key) ) ; 
		assert ( m_count < m_size ) ;
		++ m_count ;
#endif 
		while ( true ) {
			OAEntry& entry = m_entry[index];

			if ( entry.m_isFree ) {
				entry.m_key = key ;
				entry.m_isFree = false ;
				break;
			} else {
#ifndef NDEBUG
				++ jump_count ;
#endif 
				index = (index+1)&m_mask;
			}
		}
#ifndef NDEBUG
		m_maxjump = max ( m_maxjump, jump_count ) ;
#endif 
		assert(invariance()) ;
	}
	void erase ( HashKey64 key ) 
	{
		assert ( lookup(key) ) ;
		assert(invariance()) ;
#ifndef NDEBUG
		-- m_count ; 
#endif 
		IndexType index = static_cast<IndexType>(key)&m_mask ;
		while ( true ) {
			OAEntry& entry = m_entry[index];

			if ( entry.m_key == key ) {
				entry.m_isFree = true ;
				assert(invariance()) ;
				return ;
			}
			index = (index+1)&m_mask;
		}
		assert ( false ) ; // never visit here
	}
#ifndef NDEBUG
	uint getMaxJump () const { return m_maxjump ; }
#endif
	void clear() 
	{
		for (uint i=0;i<m_size;++i) {
			m_entry[i].m_isFree = true;
		}
#ifndef NDEBUG
		m_count = 0 ;
		m_maxjump = 0 ;
#endif
	}

	bool invariance () const
	{
#ifndef NDEBUG
		uint cnt = 0;
		for ( uint i=0;i<m_size;++i ) {
			if ( !m_entry[i].m_isFree ) ++ cnt ;
		}
		assert ( cnt == m_count ) ;
#endif
		return true;
	}
};

#endif  // OPENADDRHASHTABLE_H 
