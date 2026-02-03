#include "PatternUtils.h"

namespace pattern33 {

const string dt_string[NumDataType] = {
    "TrueFalse",
    "BlackWhite",
    "Dir4_Adjancent",
    "Dir4_Diagonal",
    "Dir8",
    "DirVec4_Adjancent",
    "DirVec4_Diagonal",
    "DirVec8",
    "NBits"
};

void regenSymbols ( uint index, pattern33::Symbol symbols[13] )
{
	int type = 0;
	while ( index >= tblStartAddr[type] ) ++type;
	--type ;
		index -= tblStartAddr[type];
	symbols[0] = SYM_EMPTY;
	for ( int i=1;i<9;++i ) {
		if ( tblPattenType[type][i] ) symbols[i] = SYM_BORDER ;
		else {
			symbols[i] = Symbol(1<<((index)%3));
			index /= 3 ;
		}
	}
}

std::string pat2str (Symbol sym[13])
{
    return (boost::format("%8% %1% %2%\n%7% . %3%\n%6% %5% %4%\n")
        % SymChar[sym[1]] % SymChar[sym[2]] % SymChar[sym[3]] % SymChar[sym[4]] % SymChar[sym[5]] % SymChar[sym[6]] % SymChar[sym[7]] % SymChar[sym[8]]).str() ;
}

std::string pat2str (Symbol sym[13], uint liberty[4], Color c)
{
    return (boost::format(
		"   %10%\n\
		%2% %3% %4%\n\
		%9%%1%(%13%)%5%%11%\n\
		%8% %7% %6%\n\
		%12%\n"
		)
        % SymChar[sym[1]] % SymChar[sym[2]] % SymChar[sym[3]] % SymChar[sym[4]] % SymChar[sym[5]] % SymChar[sym[6]] % SymChar[sym[7]] % SymChar[sym[8]]
        % (char)((sym[1]&SYM_BLACK_WHITE)?'0'+liberty[0]:' ')
        % (char)((sym[3]&SYM_BLACK_WHITE)?'0'+liberty[1]:' ')
        % (char)((sym[5]&SYM_BLACK_WHITE)?'0'+liberty[2]:' ')
        % (char)((sym[7]&SYM_BLACK_WHITE)?'0'+liberty[3]:' ')
        % ( c == COLOR_BLACK?"B":"W") 
        ).str() ;
}

std::string pat2str( uint index )
{
    Symbol sym[13];
    regenSymbols(index, sym) ;
    return pat2str(sym);
}

std::string pat2str( uint index, Color c )
{
    uint pattern_index = index >> 8 ;
    Symbol sym[13];
    regenSymbols(pattern_index, sym) ;
    uint lib[4], liberty = index & 255 ;

    for ( int i=END_ADJANCENT;i>=START_ADJANCENT;--i ) {
        lib[i] = (liberty & 3) +1;
        liberty >>= 2 ;
    }
    return pat2str(sym,lib, c) ;
}

void reverseSymbols( Symbol symbols[13] )
{
    for ( int i=0;i<13;++i ) {
        switch ( symbols[i] ) {
        case SYM_BLACK:         symbols[i] = SYM_WHITE; break;
        case SYM_BLACK_EMPTY:   symbols[i] = SYM_WHITE_EMPTY; break;
        case SYM_WHITE:         symbols[i] = SYM_BLACK; break;
        case SYM_WHITE_EMPTY:   symbols[i] = SYM_BLACK_EMPTY; break;
        default:    symbols[i] = symbols[i]; break;
        }
    }
}

uint getPatternType ( Symbol symbols[13] ) 
{
    for ( uint type=0;type<NUM_PATTERN_TYPE;++type) {
        uint i = 0;
        for ( i=0;i<13;++i ) 
            if ( (symbols[i]==SYM_BORDER) != tblPattenType[type][i] )
                break;
        if ( i == 13 )
            return type ;
    }
    return ILLEGAL_PATTERN_TYPE ;
}


uint getPatternIndex ( Symbol symbols[13] )
{
	uint type = getPatternType(symbols) ;
	if ( type == ILLEGAL_PATTERN_TYPE ) return ILLEGAL_PATTERN_INDEX ;

	uint start_index = tblStartAddr[type];

	uint offset = 0;
	uint base = 1 ;
	for ( uint i=1;i<9;++i ) {
		if ( symbols[i] != SYM_BORDER ) {
			if ( symbols[i] & SYM_BOTH ) {
				offset += base * (symbols[i]==SYM_BLACK?1:2) ;
			}
			base += (base<<1) ; // base *= 3 ;
		}
	}
	return start_index + offset ;
}

int getRadius3PatternIndex ( Symbol symbols[13] )
{
	int index = 0 ;
	static int index_mapping[] = { 3, 4, 5, 6, 7, 8, 1, 2, 10, 11, 12, 9 } ;

	for ( uint i=0 ; i < 8 ; ++i ) {
		int shiftNum = i*2 ;
		switch( symbols[index_mapping[i]] ) {
		case SYM_BLACK: index ^= (1 << shiftNum ) ; break ;
		case SYM_WHITE: index ^= (2 << shiftNum ) ; break ;
		case SYM_BORDER: index ^= ( 3 << shiftNum ) ; break ;
		}
	}
	return index ;
}

uint getDualIndex(uint idx)
{
    pattern33::Symbol symbols[13];    
    regenSymbols(idx, symbols) ;
    reverseSymbols(symbols) ;
    return getPatternIndex(symbols) ;
}

Vector<uint,8> getRotationIndices(uint idx) 
{
	Symbol symbols[13];    
	regenSymbols(idx, symbols) ;
	// generate original symbols
	Vector<uint,8> retIndices ;	
	for ( uint i=0;i<8;++i ) 
	{
		Symbol tmpSymbols[13];
		memcpy(tmpSymbols, symbols, sizeof (tmpSymbols) ) ;
		RotateReflect(tmpSymbols,i) ;
		retIndices.push_back(getPatternIndex(tmpSymbols)) ;
	}
	return retIndices ;
}

void RotateReflect (Symbol symbols[13], uint method) 
{
	Symbol tmp[13];
	memcpy(tmp, symbols, sizeof(tmp) );
	for ( uint i=0;i<13;++i )
		symbols[i] = tmp[tblRotateDir[method][i]];
}

}