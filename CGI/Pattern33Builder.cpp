#include "Pattern33Builder.h"
#include "Pattern33Parser.h"
#include <boost/filesystem.hpp>
#include <cstring>

#include "WeichiConfigure.h"

namespace pattern33 {

	Pattern33Table s_pattern_table ;  // instance of pattern table

	static const int tblRotate[8][13] = 
	{
		{ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12},
		{ 0,  7,  8,  1,  2,  3,  4,  5,  6, 12,  9, 10, 11},
		{ 0,  5,  6,  7,  8,  1,  2,  3,  4, 11, 12,  9, 10},
		{ 0,  3,  4,  5,  6,  7,  8,  1,  2, 10, 11, 12,  9},
		{ 0,  1,  8,  7,  6,  5,  4,  3,  2,  9, 12, 11, 10},
		{ 0,  3,  2,  1,  8,  7,  6,  5,  4, 10,  9, 12, 11},
		{ 0,  5,  4,  3,  2,  1,  8,  7,  6, 11, 10,  9, 12},
		{ 0,  7,  6,  5,  4,  3,  2,  1,  8, 12, 11, 10,  9},
	};

	static const int tblDir4Rotate[8][4] =
	{
		{ 0,  1,  2,  3}, { 1,  2,  3,  0},
		{ 2,  3,  0,  1}, { 3,  0,  1,  2},
		{ 0,  3,  2,  1}, { 1,  0,  3,  2},
		{ 2,  1,  0,  3}, { 3,  2,  1,  0},
	};

	static const int tblDir4dRotate[8][4] =
	{
		{ 0,  1,  2,  3}, { 1,  2,  3,  0},
		{ 2,  3,  0,  1}, { 3,  0,  1,  2},
		{ 3,  2,  1,  0}, { 0,  3,  2,  1}, 
		{ 1,  0,  3,  2}, { 2,  1,  0,  3}, 
	};

	static const int tblDirv8Rotate[8][8] =
	{
		{ 0,  1,  2,  3,  4,  5,  6,  7},
		{ 2,  3,  4,  5,  6,  7,  0,  1},
		{ 4,  5,  6,  7,  0,  1,  2,  3},
		{ 6,  7,  0,  1,  2,  3,  4,  5},
		{ 0,  7,  6,  5,  4,  3,  2,  1},
		{ 2,  1,  0,  7,  6,  5,  4,  3},
		{ 4,  3,  2,  1,  0,  7,  6,  5},
		{ 6,  5,  4,  3,  2,  1,  0,  7},
	};

	static const int tblDir8Rotate[8][8] =
	{
		{ 0,  1,  2,  3,  4,  5,  6,  7},
		{ 1,  2,  3,  0,  5,  6,  7,  4},
		{ 2,  3,  0,  1,  6,  7,  4,  5},
		{ 3,  0,  1,  2,  7,  4,  5,  6},

		{ 0,  3,  2,  1,  7,  6,  5,  4},
		{ 1,  0,  3,  2,  4,  7,  6,  5},
		{ 2,  1,  0,  3,  5,  4,  7,  6},
		{ 3,  2,  1,  0,  6,  5,  4,  7},
	};

	Pattern33Builder::Pattern33Builder()
		: m_entries(s_pattern_table.m_table)
	{
		initByGenerator();
	}

	bool Pattern33Builder::build( const string& dirname )
	{
		if( TEST_PATTERN_BUILD ) {
			self_test() ;
		}

		if ( loadPreBuilt() ) 
			return true;

		memset(m_entries, 0, sizeof(Pattern33Entry)*TABLE_SIZE) ;
		buildByProcedure() ;
		
		using namespace boost::filesystem ;
		if ( !exists(dirname) || !is_directory(dirname) ) {
			return setError("directory not exists") ;
		}

		Pattern33Parser parser;

		directory_iterator end ;
		for ( directory_iterator it(dirname); it!=end; ++it) {

			// Run pasrser and push into features
			vector<PatternFeature> features;
			string file = it->path().string();
			if ( is_directory(file) ) continue;
			if ( !parser.parseFromFile(file, features) ) {
				return setError("When parsing \""+file+"\", "+parser.getError()) ;
			}

			for ( uint i=0;i<features.size();++i ) {
				PatternFeature feature = features[i];
				bool success = true;

				// Build duality pattern
				if ( feature.duality == COLOR_NONE ) {
					if ( !buildPattern(feature) ) success = false;
				} else {
					feature.Id = (feature.duality==COLOR_BLACK?"Black":"White") + features[i].Id ;
					if ( !buildPattern(feature) ) success = false;
					feature = getDual(feature) ;
					feature.Id = (feature.duality==COLOR_BLACK?"Black":"White") + features[i].Id ;
					if ( !buildPattern(feature) ) success = false;
				}
				if ( !success ) {
					return setError(getError() + "When build \"" + features[i].toString() + "\" in file \"" + file + "\"") ;
				}
			}
		}

		storePreBuilt();
		return true;
	}

	bool Pattern33Builder::loadPreBuilt()
	{
		string file = WeichiConfigure::db_dir + "pattern_table.db" ;
		ifstream fdb (file.c_str(), ios::binary|ios::in);
		if ( !fdb ) return false ;

#ifdef PATTERN_VERSION
		uint version_size = PATTERN_VERSION_SZ; 
#else
		uint version_size = 0 ;
#endif
		uint table_size = sizeof(Pattern33Entry)*TABLE_SIZE ;
		uint expectedSize = version_size + table_size + 8*8*128 + 8*8;

		fdb.seekg(0, ios::end) ;
		std::streamoff sz = fdb.tellg();
		if ( (uint)sz != expectedSize ) return false ;

		fdb.seekg(0, ios::beg);
#ifdef PATTERN_VERSION
		char version[PATTERN_VERSION_SZ+1] ;
		fdb.read(version,version_size);
		version[version_size] = 0 ;
		if ( fdb.gcount() != version_size || string(version) != string(PATTERN_VERSION) ) 
			return false; // incorrect pattern version
#endif
		fdb.read((char*)m_entries,table_size);
		if ( fdb.gcount() != table_size ) return false ;

		uint m_Pattern_ActionSize = 8*8*128 ;
		uint m_Pattern_RemoveSize = 8*8 ;

 		fdb.read((char*)PatternActionTable::m_Pattern_Action, m_Pattern_ActionSize );
		fdb.read((char*)PatternActionTable::m_Pattern_Remove, m_Pattern_RemoveSize );

		return true;
	}

	void Pattern33Builder::storePreBuilt()
	{
		string file = WeichiConfigure::db_dir + "pattern_table.db";
		ofstream fdb (file.c_str(), ios::binary|ios::out);
		if ( !fdb ) return ; 
#ifdef PATTERN_VERSION
		uint version_size = PATTERN_VERSION_SZ; 
		fdb.write((const char*)PATTERN_VERSION, version_size);
#endif
		uint table_size = sizeof(Pattern33Entry)*TABLE_SIZE ;
		fdb.write((const char*)m_entries, table_size);

		ull Pattern_Action[8][128];
		ull Pattern_Remove[8];

		for( uint i = 0 ; i < 8 ; i++ )
		{
			for( uint j = 0 ; j<128 ; j++ )
			{
				Pattern_Action[i][j] = PatternActionTable::m_Pattern_Action[i][j] ;								
			}
			Pattern_Remove[i] = PatternActionTable::m_Pattern_Remove[i] ;				
		}

		uint m_Pattern_ActionSize = 8*8*128 ;
		uint m_Pattern_RemoveSize = 8*8 ;

		fdb.write( (const char*)Pattern_Action, m_Pattern_ActionSize );
		fdb.write( (const char*)Pattern_Remove, m_Pattern_RemoveSize );
	}

	bool Pattern33Builder::buildPattern( const PatternFeature& feature )
	{
		uint fnCount = static_cast<uint>(m_mapFn.count(feature.Id));

		if( fnCount == 0 ) {
			CERR() << "No PatternSetter for ID=" << feature.Id << ", skip it" << endl;
			return true;
		}			

		uint value = 0;
		for ( uint i=0;i<8;++i ) 
		{
			if( !feature.rotations[i] ) continue;
			Symbol symbols[13];
			memcpy(symbols, feature.symbols, sizeof (symbols) ) ;
			RotateReflect(symbols,i);
			value = RotateReflect(feature.dt, feature.value, i);
			buildPattern_r(symbols,value,m_mapFn[feature.Id]);
		}
		return true;
	}

	void Pattern33Builder::buildPattern_r( Symbol symbols[13], uint value, PatternSetter fn, int depth /*= 0*/ )
	{

		if ( depth == 9 ) {		
			/*
			if( symbols[0] == SYM_EMPTY && symbols[1] == SYM_EMPTY && symbols[2] == SYM_EMPTY &&
				symbols[3] == SYM_BLACK && symbols[4] == SYM_BLACK && symbols[5] == SYM_EMPTY &&
				symbols[6] == SYM_BORDER && symbols[7] == SYM_BORDER && symbols[8] == SYM_BORDER ) 
			{
				printPattern(symbols) ;
			}
			*/
			int pattern_index = getPatternIndex(symbols) ;			
			if( pattern_index == ILLEGAL_PATTERN_INDEX ) return ;			
			(*fn)(m_entries[pattern_index], value,symbols);
			return ;
		}

		Symbol sym = symbols[depth] ;
		for ( uint b=1/*SYM_EMPTY*/;b<=8/*SYM_BORDER*/;b<<=1 ) {
			if ( sym & b ) {
				symbols[depth] = (Symbol)b;
				buildPattern_r(symbols, value, fn, depth+1);
			}
		}
		symbols[depth] = sym ;
	}


	int Pattern33Builder::getRadius3PatternIndex(Symbol symbols[13])
	{
		if( getPatternType(symbols) == -1 ) {
			return -1;
		}
		static int index_mapping[] = { 3, 5, 7, 1, 4, 6, 8, 2, 10, 11, 12, 9 } ;

		int index = 0 ;
		for ( uint i=0 ; i < 8 ; ++i ) {
			int shiftNum = i*2 ;
			switch( symbols[index_mapping[i]] ) {
				case SYM_BLACK: index |= (1 << shiftNum ) ; break ;
				case SYM_WHITE: index |= (2 << shiftNum ) ; break ;
				case SYM_BORDER: index |= ( 3 << shiftNum ) ; break ;
				case SYM_EMPTY: break;
				default: assert(false);
			}
		}
		return index ;
	}


	void Pattern33Builder::RegSetter( const string& ID, PatternSetter fn )
	{
		assert ( m_mapFn.count(ID) == 0 ) ;
		m_mapFn[ID] = fn ;
	}



	void Pattern33Builder::RotateReflect( Symbol symbols[13], uint method )
	{
		Symbol tmp[13];
		memcpy(tmp, symbols, sizeof(tmp) );
		for ( uint i=0;i<13;++i )
			symbols[i] = tmp[tblRotate[method][i]] ;
	}

	uint Pattern33Builder::RotateReflect( DataType dt, uint value, uint method )
	{
		if ( dt == TrueFalse || dt == BlackWhite || dt == NBits )
			return value ;

		const int * tbl = NULL ;
		switch ( dt ) {
		case Dir4Adj: case DirVec4Adj: tbl = tblDir4Rotate[method] ; break;
		case Dir4Diag: case DirVec4Diag: tbl = tblDir4dRotate[method] ; break;
		case Dir8: tbl = tblDir8Rotate[method] ; break;
		case DirVec8: tbl = tblDirv8Rotate[method] ; break;
		default: assert(false);
		}

		uint result = 0;
		switch ( dt ) {
		case Dir4Adj: case Dir4Diag: case Dir8:
			return tbl[value] | (1<<(s_bitsForTypes[dt]-1)) ;
			break;
		case DirVec4Adj: case DirVec4Diag: case DirVec8:
			for ( int i=0;value;++i ) {
				if ( value & (1<<i) ) {
					result |= (1<<tbl[i]) ;
					value ^= (1<<i) ;
				}
			}
			result |= (1<<(s_bitsForTypes[dt]-1)) ; // set bit indicate has such feature
			return result;
			break;
			default: assert(false);
		}
		assert(false);
		return -1 ;
	}

	void printb(int x, int n)
	{
		for ( int b=(1<<(n-1));b;b>>=1 ) {
			printf("%d", ((x&b)?1:0)) ;
		} printf ( "  " ) ;
	}

	void Pattern33Builder::self_test()
	{
		Symbol sym[13] = { SYM_ALL, 
			SYM_BORDER, SYM_BORDER, SYM_BLACK, SYM_WHITE, 
			SYM_BLACK, SYM_WHITE, SYM_BLACK, SYM_BORDER, 
			SYM_EMPTY, SYM_EMPTY, SYM_EMPTY, SYM_EMPTY };

		int index = getPatternIndex(sym) ;

		assert( index == 64921 ) ;
	}

	void Pattern33Builder::printPattern( Symbol sym[13] )
	{
		printf ( "@  %c\n@ %c%c%c\n@%c%c%c%c%c\n@ %c%c%c\n@  %c\n",
												SymChar[sym[10]],
								SymChar[sym[2]], SymChar[sym[3]], SymChar[sym[4]],
			SymChar[sym[9]], SymChar[sym[1]], SymChar[sym[0]], SymChar[sym[5]], SymChar[sym[11]], 
								SymChar[sym[8]], SymChar[sym[7]], SymChar[sym[6]],
												SymChar[sym[12]]
		) ;
	}

	void Pattern33Builder::buildByProcedure ( )
	{

		Symbol symbols[] = {SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, 
			SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL} ;
		
		Symbol symbols1[] = {SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, 
			SYM_ALL, SYM_ALL, SYM_BORDER, SYM_ALL, SYM_ALL, SYM_ALL} ;

		Symbol symbols2[] = {SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, 
			SYM_ALL, SYM_ALL, SYM_ALL, SYM_BORDER, SYM_ALL, SYM_ALL} ;

		Symbol symbols3[] = {SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, 
			SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_BORDER, SYM_ALL} ;

		Symbol symbols4[] = {SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, 
			SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_BORDER} ;
		
		Symbol symbols5[] = {SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, 
			SYM_ALL, SYM_ALL, SYM_BORDER, SYM_BORDER, SYM_ALL, SYM_ALL} ;

		Symbol symbols6[] = {SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, 
			SYM_ALL, SYM_ALL, SYM_ALL, SYM_BORDER, SYM_BORDER, SYM_ALL} ;

		Symbol symbols7[] = {SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, 
			SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_BORDER, SYM_BORDER} ;

		Symbol symbols8[] = {SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, SYM_ALL, 
			SYM_ALL, SYM_ALL, SYM_BORDER, SYM_ALL, SYM_ALL, SYM_BORDER} ;


		buildPattern_r( symbols,  0 , &Pattern33Builder::buildProcedure ) ;
		
		buildPattern_r( symbols1, 0 , &Pattern33Builder::buildProcedure ) ;
		buildPattern_r( symbols2, 0 , &Pattern33Builder::buildProcedure ) ;
		buildPattern_r( symbols3, 0 , &Pattern33Builder::buildProcedure ) ;
		buildPattern_r( symbols4, 0 , &Pattern33Builder::buildProcedure ) ;

		buildPattern_r( symbols5, 0 , &Pattern33Builder::buildProcedure ) ;
		buildPattern_r( symbols6, 0 , &Pattern33Builder::buildProcedure ) ;
		buildPattern_r( symbols7, 0 , &Pattern33Builder::buildProcedure ) ;
		buildPattern_r( symbols8, 0 , &Pattern33Builder::buildProcedure ) ;

	}


	void Pattern33Builder::buildProcedure( Pattern33Entry& entry, uint v, Symbol symbols[13]  )
	{
		uint nEmpty = 0, nBlack = 0, nWhite = 0 ;
		uint nAdjEmpty = 0, nAdjBlack = 0, nAdjWhite = 0 ;
		uint nDiagEmpty = 0, nDiagBlack = 0, nDiagWhite = 0 ;
		
		for ( uint i=0;i<9;++i ) {
			switch(symbols[i]) {
			case SYM_EMPTY: ++nEmpty; break;
			case SYM_BLACK: ++nBlack; break;
			case SYM_WHITE: ++nWhite; break;
			case SYM_BORDER: break;
			default: assert(false);
			}
		}
		for ( uint i=1;i<9;++i ) {
			if ( i & 1 ) {
				switch(symbols[i]) {
				case SYM_EMPTY: ++nAdjEmpty; break;
				case SYM_BLACK: ++nAdjBlack; break;
				case SYM_WHITE: ++nAdjWhite; break;
				case SYM_BORDER: break;
				default: assert(false);
				}
			} else {
				switch(symbols[i]) {
				case SYM_EMPTY: ++nDiagEmpty; break;
				case SYM_BLACK: ++nDiagBlack; break;
				case SYM_WHITE: ++nDiagWhite; break;
				case SYM_BORDER: break;
				default: assert(false);
				}
			}
		}
		if ( nEmpty == 9 ) {
			// set only for center is also empty
			entry.inn[0].CompleteEmpty = true;
		}
		Color center = Sym2Color(symbols[0]) ;
		entry.inn[center].BlackGridCount = nBlack ;
		entry.inn[center].WhiteGridCount = nWhite ;
		entry.inn[center].EmptyGridCount = nEmpty ;

		entry.inn[center].BlackAdjGridCount = nAdjBlack ;
		entry.inn[center].WhiteAdjGridCount = nAdjWhite ;
		entry.inn[center].EmptyAdjGridCount = nAdjEmpty ;

		entry.inn[center].BlackDiagGridCount = nDiagBlack ;
		entry.inn[center].WhiteDiagGridCount = nDiagWhite ;
		entry.inn[center].EmptyDiagGridCount = nDiagEmpty ;

        
		entry.inn[center].BlackCutArea = buildCutArea ( entry, v, symbols, SYM_BLACK) ;
		entry.inn[center].WhiteCutArea = buildCutArea ( entry, v, symbols, SYM_WHITE) ;
	}
    

	/// 1. 如果有切出新區域
	///    => 回傳新區域的代表點 (上下左右) & 被包圍的斜角
	/// 2. 若沒有切出新區域
	///    => 回傳被包圍的斜角

	static int Find ( int x, int group[9] ) {
		if ( x == group[x] ) return x ;
		else return group[x] = Find(group[x], group);
	}

	static bool Union ( int a, int b, Symbol symbols[13], Symbol color, int group[9] ) {
		Symbol symA = symbols[a], symB = symbols[b];
		if ( ((symA|symB)&(color|SYM_BORDER)) == 0 ) { // both symA and symB are not 'color' or border
			int ga = Find(a,group), gb = Find(b,group);
			if ( ga != gb ) {
				// select 4 adjacent grid first (odd if here)
				if ( a&1 ) group[gb] = ga ;
				else group[ga] = gb;
				return true;
			}
		}
		return false ;
	}

	int Pattern33Builder::buildCutArea( Pattern33Entry& entry, uint v, Symbol symbols[13], Symbol color )
	{
		if ( symbols[0] != SYM_EMPTY ) return 0;
		// 2 3 4
		// 1 0 5
		// 8 7 6

		int group[9] = {0,1,2,3,4,5,6,7,8} ;

		Union(1, 2, symbols, color, group);
		Union(2, 3, symbols, color, group);
		Union(3, 4, symbols, color, group);
		Union(4, 5, symbols, color, group);
		Union(5, 6, symbols, color, group);
		Union(6, 7, symbols, color, group);
		Union(7, 8, symbols, color, group);
		Union(8, 1, symbols, color, group);

		int ret_var = 0;
		for ( uint i=1;i<9;++i ) {
			// if opp or empty, set group on
			if ( symbols[i]&(SYM_EMPTY_BLACK_WHITE^color) ) 
				ret_var |= (1<<(Find(i,group)-1));
		}

		// count how many disconnected parts connect to center grid
		int cnt = 0;
		cnt = (Union(0,1, symbols, color, group) ? 1 : 0 ) +
				(Union(0,3, symbols, color, group) ? 1 : 0 ) +
				(Union(0,5, symbols, color, group) ? 1 : 0 ) +
				(Union(0,7, symbols, color, group) ? 1 : 0 ) ;

		if ( cnt >= 2 ) return ret_var;
		else return ret_var &= 0xAA; 
		// in case no new close create, check only diagonal grids
	}

	PatternFeature Pattern33Builder::getDual( const PatternFeature& feature )
	{
		assert ( feature.duality != COLOR_NONE ) ;
		PatternFeature result = feature ;
		result.duality = AgainstColor(result.duality) ;
		reverseSymbols(result.symbols);
		return result;
	}

} 
