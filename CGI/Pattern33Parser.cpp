#include "Pattern33Parser.h"
#include "Color.h"
#include "strops.h"
#include "Pattern33Regex.h"
#include <boost/regex.hpp>
#include <fstream>
using namespace boost ;
using namespace std;

namespace pattern33 {

	static const int 
		comment_idx			= 0,
		p1grid_idx			= 1,
		p3grid_idx			= 2,
		p5grid_idx			= 3,
		define_group_idx	= 4,
		idEqualVal_idx		= 5,					 
		define_idx			= 6,
		rotation_idx		= 7;


	const regex Pattern33Parser::s_recognizer[8] = 
	{
		RECOGNIZER_COMMENT	,
		RECOGNIZER_1GRID	, 
		RECOGNIZER_3GRIDS	, 
		RECOGNIZER_5GRIDS	, 
		RECOGNIZER_GROUP	,  
		RECOGNIZER_IDEQVAL	, 
		RECOGNIZER_DEFCONST	, 		
		RECOGNIZER_ROTATION	   
	};

	const regex Pattern33Parser::s_extractor[8] =	
	{
		EXTRACTOR_1ST_LINE	,  
		EXTRACTOR_2ND_LINE	,  
		EXTRACTOR_3RD_LINE	,  
		EXTRACTOR_4TH_LINE	,  
		EXTRACTOR_5TH_LINE	,  
		EXTRACTOR_COMMENT	, 
		EXTRACTOR_IDEQVAL	, 
		EXTRACTOR_ROT		 
	};

	const regex Pattern33Parser::s_typeChecker[NumDataType] = {
		TYPECHKER_TRUEFALSE		,
		TYPECHKER_BLACKWHITE	,
		TYPECHKER_4ADJ			,
		TYPECHKER_4DIAG			,
		TYPECHKER_8DIR			,
		TYPECHKER_BIT4ADJ		,
		TYPECHKER_BIT4DIAG		,
		TYPECHKER_BIT8DIR		,
		TYPECHKER_VALUE_NBIT	
	};

	const regex Pattern33Parser::s_idChecker = regex("(Black|White)?(.*)") ;

	const uint  Pattern33Parser::rotationTable[8][6] =
	{
		{0,1,3,2,4,6},
		{1,2,0,3,7,5},
		{2,3,1,0,6,4},
		{3,0,2,1,5,7},
		{4,5,7,6,0,2},
		{5,6,4,7,3,1},
		{6,7,5,4,2,0},
		{7,4,6,5,4,3}
	};

	bool Pattern33Parser::parseFromFile( const std::string& filename, std::vector<PatternFeature>& features )
	{
		string content, line ;
		ifstream fin ( filename.c_str() ) ;
		if ( !fin ) {
			return setError("Can not open file \""+filename+"\"") ;
		}
		while ( getline(fin,line) ) {
			content += line += "\n" ;
		}
		fin.close();

		return parseFromString(content, features) ;
	}

	bool Pattern33Parser::parseFromString( const std::string& str, std::vector<PatternFeature>& features )
	{
		if( !loadConstant(str) )			return setError( getError() + " when loading constant.") ;		
		if( !loadPattern(str,features) )	return setError( getError() + " when loading pattern.") ;
		
		return true;
	}

	bool Pattern33Parser::setError( const std::string& msg )
	{
		m_error = msg ;
		return false ;
	}

	bool Pattern33Parser::loadConstant( const std::string& str )
	{
		m_mapConsts.clear() ;

		regex p = s_recognizer[define_idx] ;
		string::const_iterator begin = str.begin(), end = str.end();
		smatch match ;
		while ( regex_search(begin, end, match, p) ) {
			std::string key = match[1];
			int value = atoi(string(match[2]).c_str()) ;

			if ( m_mapConsts.count(key) != 0 ) {
				if ( m_mapConsts[key] != value ) {
					return setError("Conflict definitions for "+key) ;
				}
			}
			m_mapConsts[key] = value ;
			begin = match[0].second ;
		}
		return true ;
	}

	bool Pattern33Parser::loadPattern( const std::string& str, std::vector<PatternFeature>& features)
	{
		enum ParseStatus {
			STATUS_EXPECT_PATTERN, 
			STATUS_OPTION_ROTATION,
			STATUS_EXPECT_VAL_OR_NEW,
		};

		istringstream iss ( str ) ;
		Pattern33Info patternInfo ;
		string line ;
		int lineNo = 0 ;		
		ParseStatus parse_status = STATUS_EXPECT_PATTERN ;

		while( getline(iss,line) ) 
		{	 							
			lineNo++ ;
			if( matchCommentLine(line) ) continue ; 
			if( matchDefineLine (line) ) continue ;

			smatch match;
			string::const_iterator begin = line.begin(), end = line.end();
			regex patternRegex(s_extractor[patternInfo.getLineNo()]);

			switch( parse_status ) {
				case STATUS_EXPECT_PATTERN:
					regex_search( begin , end , match , patternRegex ) ;
					patternInfo.addLine(match[1]) ;
					if( patternInfo.isFull() ) { 
						patternInfo.resetLineNo();
						parse_status = STATUS_OPTION_ROTATION ;									
					}
					break;
				case STATUS_OPTION_ROTATION:
					if( regex_match( line , s_recognizer[rotation_idx]) ) {
						regex_search( begin , end , match , EXTRACTOR_ROT ) ;
						string rotation = transRotationNums( ToString(match[1]) ) ;
						if( isErrorRoation(rotation) )
							return setError("Pattern rotation error, near \""+match[0]+"\" at line " + ToString(lineNo)) ;
						patternInfo.setRotateInfo(rotation) ;
						break ;
					}					
				case STATUS_EXPECT_VAL_OR_NEW:					
					if( regex_search(begin, end, match, EXTRACTOR_IDEQVAL ) ) {
						parse_status = STATUS_EXPECT_VAL_OR_NEW ;
						string ID = match[1];
						string VALUE = match[2];
						PatternFeature feature;
						if( patternInfo.hasSpecificRotation() )
							feature.setRotations(patternInfo.getRotateInfo());
						else
							feature.setAllRotationsOn();

						if ( !toFeature(feature, ID, VALUE, patternInfo.getPatternStr()) ) 
							return setError("create pattern fail, near \""+match[0]+"\" at line " + ToString(lineNo)) ;							
						features.push_back(feature) ;
					}else if( regex_search( begin , end , match , patternRegex ) ) {
						patternInfo.clear() ;						
						patternInfo.addLine(match[1]) ;
						parse_status = STATUS_EXPECT_PATTERN ;			
					}
					break ;
				default:
					return setError("create pattern fail, near \""+match[0]+"\" at line " + ToString(lineNo) ) ;							
			}		
		}

		return true ;
	}

	bool Pattern33Parser::toFeature( PatternFeature& feature, const std::string& ID, const std::string& value, const std::string& symbols )
	{
		interpretSymbols( feature , symbols ) ;
		setIDandDuality( feature , ID ) ;

		feature.bits = 0;
		//// determine data type, value		
		for ( uint i=0 ; i<NumDataType ; ++i ) {
			smatch m;			
			if ( regex_match(value, m, s_typeChecker[i]) ) {
				feature.dt = DataType(i);
				feature.value = resolveValue(feature.dt, m[1]) ;
				if ( !getError().empty() )  {
					return false ; // resolve value fail
				}
				feature.bits = s_bitsForTypes[i] ;
				if ( feature.dt == NBits ) {
					feature.bits = atoi( string(m[2]).c_str() ) ;
				}
				break;
			}
		}


		// If no data type match, output error message.
		if ( feature.bits == 0 ) {
			COUT() << "ID:" << ID << " Value:" << value << endl;
			COUT() << "No type match for Value \""+value+"\"" << endl;
			return setError("No type match for Value \""+value+"\"") ;
		}

		return true;
	}

	void Pattern33Parser::setIDandDuality( PatternFeature& feature , const std::string& ID )
	{
		smatch m ;
		regex_match(ID, m, s_idChecker) ;
		feature.Id = m[2];
		if ( m[1] == "" )
			feature.duality = COLOR_NONE ;
		else 
			feature.duality = (m[1]=="Black"?COLOR_BLACK:COLOR_WHITE) ;		
	}

	void Pattern33Parser::interpretSymbols( PatternFeature& feature , const std::string& symbols )
	{
		static uint index_mapping[] = {6,5,1,2,3,7,11,10,9,4,0,8,12} ;

		assert ( symbols.length() == 13 ) ;
		for ( uint i=0;i<13;++i ) {
			feature.symbols[i] = interpret(symbols[index_mapping[i]]) ;
		}
	}

	pattern33::Symbol Pattern33Parser::interpret( char c )
	{
		switch ( c ) {
			case '.': return SYM_EMPTY;
			case 'B': return SYM_BLACK;
			case 'W': return SYM_WHITE;
			case ' ': return SYM_BORDER;
			case '*': return SYM_EMPTY_BLACK_WHITE;
			case '+': return SYM_BLACK_WHITE;
			case 'w': return SYM_WHITE_EMPTY;
			case 'b': return SYM_BLACK_EMPTY;
			case 'X': return SYM_ALL;
			default: assert(false) ;
				return SYM_BORDER ;
		}
	}

	uint Pattern33Parser::resolveValue( DataType dt, const std::string& value )
	{
		if ( dt == TrueFalse ) {
			return (value[0]=='1'||tolower(value[0])=='t') ;
		} else if ( dt == BlackWhite ) {
			char c = tolower(value[0]);
			return (uint)(c=='n'?COLOR_NONE:(c=='b'?COLOR_BLACK:COLOR_WHITE)) ;
		} else if ( dt == Dir4Adj || dt == Dir4Diag || dt == Dir8 ) {
			uint index = 999 ;
			if ( value == "L" ) index = 0;
			else if ( value == "U") index = 1;
			else if ( value == "R") index = 2;
			else if ( value == "D") index = 3;
			else if ( value == "UL") index = 4;
			else if ( value == "UR") index = 5;
			else if ( value == "DR") index = 6;
			else if ( value == "DL") index = 7;
			else assert(false);

			if ( dt == Dir4Diag ) index -= 4 ;

			return index ;
		} else if ( dt == DirVec4Adj || dt == DirVec4Diag || dt == DirVec8 ) {
			uint index = 0;
			for (uint i=0;i<value.length();++i) {
				if ( isupper(value[i]) )
					index |= (1<<i) ;
			}
			if ( dt == DirVec4Diag ) {
				index = ((index&0x80)>>4) |
					((index&0x20)>>3) |
					((index&0x8)>>2) |
					((index&0x2)>>1) ;
			}
			return index ;

		} else if ( dt == NBits ) {
			if ( regex_match(value,regex("\\d+")) ) {
				return atoi( value.c_str() ) ;
			}
			if ( m_mapConsts.count(value) == 0 ) {
				setError("Undefined constant " + value );
				COUT() << getError() << endl;
				return -1;
			} else {
				return m_mapConsts[value] ;
			}
		} else {
			assert (false) ;
			return 0;
		}
	}

	void Pattern33Parser::printPattern(string& patternStr) {
		printf(  "  %c \n",patternStr[0]);
		printf(" %c%c%c \n",patternStr[1],patternStr[2],patternStr[3]);
		printf("%c%c%c%c%c\n",patternStr[4],patternStr[5],patternStr[6],patternStr[7],patternStr[8]);
		printf(" %c%c%c \n",patternStr[9],patternStr[10],patternStr[11]);
		printf(  "  %c \n",patternStr[12]);
	}

	string Pattern33Parser::transRotationNums(string rotDef)
	{
		stringstream ss(rotDef);
		string retString;
		retString.clear() ;
		string def ;				
		
		while( getline(ss,def,',') ) {
			uint rotationType = 0 ;
			string::const_iterator begin = def.begin(), end = def.end();
			smatch match ;			
			regex reg_def( "(R\\((\\-?[0-9]+)\\))?(S\\(([ULDR])\\))?") ;
			if( !regex_search(begin, end, match, reg_def) ) { return STR_ERROR_ROTATION ;  ; }
			// ex: R(270)S(D)
			// $1 = R(270)
			// $2 = 270
			// $3 = S(D)
			// $4 = D
			if( match[2].first != match[2].second ) {
				int degree = atoi(ToString(match[2]).c_str()) ; 
				switch( degree )
				{
				case    0:  rotationType = rotationTable[rotationType][0];
							break;					
				case   90:
				case -270:	rotationType = rotationTable[rotationType][1];
							break;					
				case  -90:
				case  270:  rotationType = rotationTable[rotationType][2];
							break;					
				case  180:
				case -180:  rotationType = rotationTable[rotationType][3];
							break;											
				default	 :  return STR_ERROR_ROTATION ;
				}
			}

			if( match[4].first != match[4].second ) {
				char dir = ToString(match[4])[0] ;					
				switch(dir)
				{
				case 'U':
				case 'D': rotationType = rotationTable[rotationType][4];
							break;					
				case 'L':
				case 'R': rotationType = rotationTable[rotationType][5];
							break;					
				default	: return STR_ERROR_ROTATION ;
				}
			}
			retString += ToString(rotationType) ;
		}
		return retString ;
	}

	bool Pattern33Parser::isErrorRoation  ( const string& rotation) const  { return rotation == STR_ERROR_ROTATION ; }
	bool Pattern33Parser::matchCommentLine( const string& line ) const { return regex_match(line, s_recognizer[comment_idx]); }
	bool Pattern33Parser::matchDefineLine ( const string& line ) const { return regex_match(line, s_recognizer[define_idx] ); }

}
 
