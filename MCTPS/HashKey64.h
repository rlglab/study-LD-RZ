#ifndef HASHKEY64_H
#define HASHKEY64_H

#include "types.h"
#include <iostream>

class HashKey64
{
public:
    HashKey64 () 
		: m_key ( 0 ) {}

    HashKey64 ( const HashKey64& key ) 
        : m_key ( key.m_key ) {}

    HashKey64 ( ull key )
        : m_key ( key ) {} 

    inline bool operator== (const HashKey64& key ) const ;
    inline bool operator!= (const HashKey64& key ) const ;

    inline bool operator< ( const HashKey64& key ) const ; 

    inline HashKey64 operator^ ( const HashKey64& key ) const ;
    inline HashKey64 operator& ( const HashKey64& key ) const ;
    inline ull operator& ( ull mask ) const ;

    inline HashKey64& operator^= ( const HashKey64& key ) ;
    operator long long int () const { return m_key ; }

	inline std::ostream& operator<< (std::ostream& stream) const ;
	friend std::istream& operator>> (std::istream& stream, HashKey64& rhs)
	{
		stream >> rhs.m_key;
		return stream;
	}

private:
    ull m_key ;
};


inline bool HashKey64::operator== (const HashKey64& key ) const 
{
    return this->m_key == key.m_key ;
}

inline bool HashKey64::operator!= (const HashKey64& key ) const 
{
    return this->m_key != key.m_key ;
}

inline bool HashKey64::operator< ( const HashKey64& key ) const 
{
    return this->m_key < key.m_key ;
}


inline class HashKey64 HashKey64::operator^ ( const HashKey64& key ) const 
{
    return HashKey64( this->m_key ^ key.m_key ) ;
}
inline class HashKey64 HashKey64::operator& ( const HashKey64& key ) const 
{
    return HashKey64( this->m_key & key.m_key ) ;
}

ull HashKey64::operator&( ull mask ) const
{
    return (this->m_key & mask) ;
}

inline class HashKey64& HashKey64::operator^= ( const HashKey64& key ) 
{
    this->m_key ^= (key.m_key) ;
    return *this;
}

inline std::ostream& HashKey64::operator<< (std::ostream& stream) const 
{
	stream << m_key;
	return stream;
}

#endif // HASHKEY64_H
 
