#ifndef PATTERNUTILS_H
#define PATTERNUTILS_H

#include "Color.h"

namespace pattern33
{
	const uint TABLE_SIZE	= 7641 ;
    const uint EIGHT_SIZE	= 6561 ; 
    const uint FIVE_SIZE	=  243 ; 
    const uint THREE_SIZE	=   27 ; 
     
	const uint START_ADDR_EIGHT = 0 ; // 0 ~ 6560
	
	const uint START_ADDR_FIVE_UPPER	= START_ADDR_EIGHT + EIGHT_SIZE ; // 6561 ~ 6803
	const uint START_ADDR_FIVE_LEFT		= START_ADDR_FIVE_UPPER + FIVE_SIZE ; // 6804 ~ 7046
	const uint START_ADDR_FIVE_RIGHT	= START_ADDR_FIVE_LEFT + FIVE_SIZE ; // 7047 ~ 7289
	const uint START_ADDR_FIVE_DOWN		= START_ADDR_FIVE_RIGHT + FIVE_SIZE ; // 7290 ~ 7532

	const uint START_ADDR_THREE_LU		= START_ADDR_FIVE_DOWN + FIVE_SIZE ; // 7533 ~ 7559
	const uint START_ADDR_THREE_RU		= START_ADDR_THREE_LU + THREE_SIZE ; // 7560 ~ 7586
	const uint START_ADDR_THREE_LD		= START_ADDR_THREE_RU + THREE_SIZE ; // 7587 ~ 7613
	const uint START_ADDR_THREE_RD		= START_ADDR_THREE_LD + THREE_SIZE ; // 7614 ~ 7640

	const uint END_ADDR					= START_ADDR_THREE_RD + THREE_SIZE ;

	const uint ILLEGAL_PATTERN_INDEX	= 99999  ;
	const uint ILLEGAL_PATTERN_TYPE		= 99 ;

	static const uint NUM_PATTERN_TYPE = 9;
	static const uint PATTERN_DEPTH = 9 ;	

    enum DataType
    {
        TrueFalse,
        BlackWhite,
        Dir4Adj,
        Dir4Diag,
        Dir8,
        DirVec4Adj,
        DirVec4Diag,
        DirVec8,
        NBits,
    };

	const uint NumDataType = 9 ;
	const uint s_bitsForTypes[NumDataType] = { 1, 2, 3, 3, 4, 5, 5, 9, 0 } ; 
	static const bool tblPattenType[NUM_PATTERN_TYPE][13] =
	{	//   0      1      2      3      4      5      6      7      8      9      10     11     12
		{false, false, false, false, false, false, false, false, false, false, false, false, false},

		{false, false,  true,  true,  true, false, false, false, false, false,  true, false, false},
		{false,  true,  true, false, false, false, false, false,  true,  true, false, false, false},
		{false, false, false, false,  true,  true,  true, false, false, false, false,  true, false},
		{false, false, false, false, false, false,  true,  true,  true, false, false, false,  true},

		{false,  true,  true,  true,  true, false, false, false,  true,  true,  true, false, false},
		{false, false,  true,  true,  true,  true,  true, false, false, false,  true,  true, false},
		{false,  true,  true, false, false, false,  true,  true,  true,  true, false, false,  true},
		{false, false, false, false,  true,  true,  true,  true,  true, false, false,  true,  true},
	};

	static const uint tblStartAddr[NUM_PATTERN_TYPE+1] =
	{
		START_ADDR_EIGHT, 
		START_ADDR_FIVE_UPPER, START_ADDR_FIVE_LEFT, START_ADDR_FIVE_RIGHT, START_ADDR_FIVE_DOWN,
		START_ADDR_THREE_LU, START_ADDR_THREE_RU, START_ADDR_THREE_LD, START_ADDR_THREE_RD,
		END_ADDR
	};

	static const int tblRotateDir[8][13] = 
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

	enum Symbol {
		/// define symbol type
		SYM_NONE = 0x00,

		SYM_EMPTY = 0x01,  // [.]
		SYM_BLACK = 0x02,  // [B]
		SYM_WHITE = 0x04,  // [W]
		SYM_BORDER = 0x08, // [ ]

		SYM_BOTH = SYM_BLACK | SYM_WHITE,         // [+] 
		SYM_BLACK_WHITE = SYM_BLACK | SYM_WHITE,  // [+]

		SYM_BLACK_EMPTY = SYM_EMPTY | SYM_BLACK,  // [b]
		SYM_WHITE_EMPTY = SYM_EMPTY | SYM_WHITE,  // [w]
		SYM_EMPTY_BLACK_WHITE = SYM_EMPTY | SYM_BLACK | SYM_WHITE,  // [*]
		SYM_EMPTY_BLACK_WHITE_BORDER = SYM_EMPTY | SYM_BLACK | SYM_WHITE | SYM_BORDER,  // [X]
		SYM_ALL = SYM_EMPTY | SYM_BLACK | SYM_WHITE | SYM_BORDER  // [X]
	};
	const char SymChar[17] = {"?.BbWw+* ??????X"};
	extern const std::string dt_string[NumDataType] ;
	inline int ctoi( char c ) { return  c - '0' ; }


	/*
		@       10      
		@     2  3  4   
		@  9  1  0  5 11
		@     8  7  6   
		@       12      
	*/
	struct PatternFeature
	{
		std::string Id;
		Color duality ; ///< none: unique, Black/White for that color

		DataType dt ;
		uint bits ;
		uint value ;

		Symbol symbols[13] ;
		bool rotations[8] ;	

		PatternFeature()
		{
			for(uint i=0;i<8;i++) 
				rotations[i] = false;
			dt = TrueFalse;
			bits = 0;
			value = 0;
		}

		std::string toString() const 
		{
			std::ostringstream oss ;
			oss << "ID: " << (duality==COLOR_NONE?"":(duality==COLOR_BLACK?"(Black)":"(White)")) << Id 
				<< ", type: " << dt_string[(int)dt] << "(" << bits << "), value: " << value ;
			oss << ", symbols: \"" ;		
			for ( uint i=0;i<13;++i ) oss << SymChar[symbols[i]] ;		
			oss << "\"" << std::endl;
			return oss.str();
		}
	

		void setAllRotationsOn()
		{
			for( uint i=0 ; i < 8 ; i++) 
				rotations[i] = true;
		}

		void setRotations( string rotationsDefinition )
		{
			for( uint i=0 ; i < rotationsDefinition.length() ; i++ )
			{
				uint enableIndex = ctoi(rotationsDefinition[i]) ;
				rotations[enableIndex] = true ;
			}
		}
	};



	inline Color Sym2Color ( Symbol sym )
	{
		assert ( sym == SYM_EMPTY || sym == SYM_BLACK || sym == SYM_WHITE ) ;
		return (Color) ( sym/2 ) ;
	}

	std::string pat2str ( uint index ) ;
	std::string pat2str ( uint index, Color c ) ;
	void regenSymbols ( uint index, pattern33::Symbol symbols[13] );
	void reverseSymbols ( pattern33::Symbol symbols[13] ) ;
	uint getPatternType (Symbol symbols[13]) ;
	uint getPatternIndex (Symbol symbols[13]) ;
	uint getDualIndex(uint idx) ;	
	Vector<uint,8> getRotationIndices(uint idx) ;

	void RotateReflect (Symbol symbols[13], uint method) ;

}

#endif