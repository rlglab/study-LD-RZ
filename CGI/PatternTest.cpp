#include "types.h"
#include "WeichiConfigure.h"

#include "Pattern33Builder.h"
#include "Pattern33Table.h"

// #include "StaticBoard.h"
// uncomment this only when debugging

#include <boost/format.hpp>
#include <iostream>
using namespace pattern33;

void pattern_test()
{
	int idx ;
	while ( true ) {
		cin >> idx ;
		if ( idx < 0 || idx > pattern33::TABLE_SIZE )  {
			CERR() << "illegal pattern index" ; 
			continue ;        
		}
		CERR() << pat2str(idx) << endl ;
		CERR() << "Rotations 8 indices:" << endl ;
		Vector<uint, 8> rotationsIndices = getRotationIndices(idx) ;
		for( uint i=0 ; i<8 ; i++ ) {
			CERR() << rotationsIndices[i] << endl ;
		}
	}

} 
