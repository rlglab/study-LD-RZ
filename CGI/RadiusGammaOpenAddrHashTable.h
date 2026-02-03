#ifndef RADIUSGAMMAOPENADDRHASHTABLE_H
#define RADIUSGAMMAOPENADDRHASHTABLE_H

#include "HashKey64.h"

class RadiusGammaEntry 
{
public:
	RadiusGammaEntry(): m_index(-1) {}
	HashKey64 m_key;
	uint m_index;
};

class RadiusGammaOpenAddrHashTable
{
	typedef unsigned int IndexType ;
private:
	const IndexType m_mask ;
	const size_t m_size ;
	uint m_count;
	RadiusGammaEntry* m_entry ;
	uint m_maxjump ;
	uint m_totalJump;

public:
	RadiusGammaOpenAddrHashTable ( int bitSize = 22 ) 
		: m_mask ( (1<<bitSize)-1 )
		, m_size ( 1<<bitSize )
		, m_count ( 0 )
		, m_entry ( new RadiusGammaEntry[1ULL<<bitSize] )
		, m_maxjump ( 0 )
		, m_totalJump ( 0 )
	{
	}

	inline size_t getSize() const { return m_size; }
	inline uint getCount() const { return m_count; }
	inline uint getMaxJump() const { return m_maxjump; }
	inline double getAvgJump() const { return 1.0*m_totalJump/m_count; }
	uint getIndex( HashKey64 key ) const
	{
		IndexType index = static_cast<IndexType>(key)&m_mask ;

		while ( true ) {
			const RadiusGammaEntry& entry = m_entry[index];

			if ( entry.m_index!=-1 ) {
				if ( entry.m_key == key ) return entry.m_index;
				else index = (index+1)&m_mask;
			} else {
				return -1;
			}
		}
		return -1;
	}
	void store ( HashKey64 key, uint iIndex ) 
	{
		IndexType index = static_cast<IndexType>(key)&m_mask ;
		uint jump_count = 0 ; 
		assert ( m_count < m_size ) ;
		++m_count;
		while ( true ) {
			RadiusGammaEntry& entry = m_entry[index];

			if ( entry.m_index==-1 ) {
				entry.m_key = key ;
				entry.m_index = iIndex;
				break;
			} else {
				++ jump_count ;
				index = (index+1)&m_mask;
			}
		}
		m_totalJump += jump_count;
		m_maxjump = max ( m_maxjump, jump_count ) ;
	}
	void clear() 
	{
		for (uint i=0;i<m_size;++i) {
			m_entry[i].m_index = -1;
		}
		m_count = 0 ;
		m_maxjump = 0 ;
	}
	void writeToDB( ofstream& fDB )
	{
		fDB.write(reinterpret_cast<char*>(&m_count), sizeof(uint));
		fDB.write(reinterpret_cast<char*>(&m_maxjump), sizeof(uint));
		fDB.write(reinterpret_cast<char*>(m_entry), sizeof(RadiusGammaEntry)*m_size);
	}
	void readFromDB( ifstream& fDB )
	{
		fDB.read(reinterpret_cast<char*>(&m_count), sizeof(uint));
		fDB.read(reinterpret_cast<char*>(&m_maxjump), sizeof(uint));
		fDB.read(reinterpret_cast<char*>(m_entry), sizeof(RadiusGammaEntry)*m_size);
	}
};

#endif 
