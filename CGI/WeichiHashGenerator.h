#ifndef WEICHIHASHGENERATOR_H
#define WEICHIHASHGENERATOR_H

#include "BasicType.h"
#include "rand64.h"

template<int _BoardSize>
class WeichiHashGenerator
{
private:

    static const int _MAX_GRID_IN_BOARD = _BoardSize*_BoardSize;
    static const int _MAX_PATH_LENGTH = 20 ;

    static HashKey64 m_keys[_MAX_GRID_IN_BOARD][3]; // for  (x,y), color
    static HashKey64 m_pathkeys[_MAX_GRID_IN_BOARD][_MAX_PATH_LENGTH]; // for (x,y) path
    static HashKey64 m_passkeys[_MAX_PATH_LENGTH]; // for (x,y) path	
    static HashKey64 m_turnkey ;
	static HashKey64 m_sequenceGameKeys[MAX_GAME_LENGTH][MAX_NUM_GRIDS][2];
	static HashKey64 m_kokeys[MAX_NUM_GRIDS];

    inline void rotate ( int& x, int& y );

public:

    WeichiHashGenerator ( ) ;
    ~WeichiHashGenerator ( ) ;
    static void initialize ( string superko_rule="situational" ) ;

    inline HashKey64 getZHashKeyOf ( Color c, uint position ) const ;
    inline HashKey64 getZHashPathKeyOf ( uint position, uint path ) const ;
    inline HashKey64 getZHashPassKey ( uint path ) const ;
    inline HashKey64 getTurnKey () const { return m_turnkey; }
	inline HashKey64 getSequenceGameKeys ( uint move_number, uint position, Color c ) const { return m_sequenceGameKeys[move_number][position][(uint)c-1]; }
	inline HashKey64 getKoKeys ( uint position ) const { return m_kokeys[position]; }
};

template<int _BoardSize>
HashKey64 WeichiHashGenerator<_BoardSize>::m_keys[_MAX_GRID_IN_BOARD][3];
template<int _BoardSize>
HashKey64 WeichiHashGenerator<_BoardSize>::m_pathkeys[_MAX_GRID_IN_BOARD][_MAX_PATH_LENGTH]; // for (x,y) path
template<int _BoardSize>
HashKey64 WeichiHashGenerator<_BoardSize>::m_passkeys[_MAX_PATH_LENGTH]; // for (x,y) path
template<int _BoardSize>
HashKey64 WeichiHashGenerator<_BoardSize>::m_turnkey ;
template<int _BoardSize>
HashKey64 WeichiHashGenerator<_BoardSize>::m_sequenceGameKeys[MAX_GAME_LENGTH][MAX_NUM_GRIDS][2] ;
template<int _BoardSize>
HashKey64 WeichiHashGenerator<_BoardSize>::m_kokeys[MAX_NUM_GRIDS];

template<int _BoardSize>
HashKey64 WeichiHashGenerator<_BoardSize>::getZHashKeyOf( Color c, uint position ) const
{
    assert ( ColorNotEmpty(c) && position < _MAX_GRID_IN_BOARD ) ;
    return m_keys[position][(uint)c] ;
}

template<int _BoardSize>
HashKey64 WeichiHashGenerator<_BoardSize>::getZHashPathKeyOf( uint position, uint path ) const
{
    assert ( position < _MAX_GRID_IN_BOARD && path <_MAX_PATH_LENGTH ) ;
    return m_pathkeys[path] ;
}

template<int _BoardSize>
HashKey64 WeichiHashGenerator<_BoardSize>::getZHashPassKey( uint path ) const
{
    assert ( path <_MAX_PATH_LENGTH ) ;
    return m_passkeys[path] ;
}

template<int _BoardSize>
WeichiHashGenerator<_BoardSize>::WeichiHashGenerator ()
{
}

template<int _BoardSize>
WeichiHashGenerator<_BoardSize>::~WeichiHashGenerator ()
{ }

template<int _BoardSize>
void WeichiHashGenerator<_BoardSize>::initialize ( string superko_rule/*="situational"*/ )
{
    init_genrand64(0);

	// board key
	for( int i=0; i<_MAX_GRID_IN_BOARD; i++ ) {
		for( int color=0; color<3; color++ ) { m_keys[i][color] = rand64(); }
		for( int path=0; path<20; path++ ) { m_pathkeys[i][path] = rand64(); }
	}

    for ( int path=0;path<20;++path ) {
        m_passkeys[path] = rand64();
    }

	// positional rule don't care who's turn
	m_turnkey = rand64();
	if( superko_rule=="positional" ) {
		m_turnkey = 0;
	}

	// sequence game keys
	for( int i=0; i<MAX_GAME_LENGTH; i++ ) {
		for( int j=0; j<MAX_NUM_GRIDS; j++ ) {
			for( int color=0; color<2; color++ ) {
				m_sequenceGameKeys[i][j][color] = rand64();
			}
		}
	}

	// ko keys
	for (int i = 0; i < _MAX_GRID_IN_BOARD; i++) {
		m_kokeys[i] = rand64();
	}
}

template<int _BoardSize>
void WeichiHashGenerator<_BoardSize>::rotate ( int& x, int& y )
{
    int tx = x ;
    x = y;
    y = _BoardSize-tx-1 ;
}

#endif 
