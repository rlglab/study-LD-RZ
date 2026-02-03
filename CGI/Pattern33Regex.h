#ifndef PATTERN33REGEX_H
#define PATTERN33REGEX_H

#include <boost/regex.hpp>
using namespace boost ;


namespace pattern33 {
	regex RECOGNIZER_COMMENT = regex ( "\\s*(#[^\\n]*)?") ;
	regex RECOGNIZER_1GRID = regex ( "\\s*@  [.BW *+bwX1-9]{1}\\s*") ;
	regex RECOGNIZER_3GRIDS = regex ( "\\s*@ [.BW *+bwX1-9]{3}\\s*") ;
	regex RECOGNIZER_5GRIDS = regex ( "\\s*@[.BW *+bwX1-9]{5}\\s*") ;
	regex RECOGNIZER_GROUP = regex ( "\\s*(:[1-9]=[bwlh][1-9])+\\s*") ;
	regex RECOGNIZER_IDEQVAL = regex ( "\\s*\\w+\\s*=\\s*[\\w \t()]+\\s*") ;
	regex RECOGNIZER_DEFCONST = regex ( "^define\\s+(\\w+)\\s+(\\d+)\\s*$") ;
	regex RECOGNIZER_ROTATION = regex ( "\\s*ROT=(.)+\\s*") ;

	regex EXTRACTOR_1ST_LINE = regex ("@  ([.BW *+bwX1-9]{1})\\s*") ;
	regex EXTRACTOR_2ND_LINE = regex ("@ ([.BW *+bwX1-9]{3})\\s*") ;
	regex EXTRACTOR_3RD_LINE = regex ("@([.BW *+bwX1-9]{5})\\s*") ;
	regex EXTRACTOR_4TH_LINE = regex ("@ ([.BW *+bwX1-9]{3})\\s*") ;
	regex EXTRACTOR_5TH_LINE = regex ("@  ([.BW *+bwX1-9]{1})\\s*") ;
	regex EXTRACTOR_COMMENT  = regex ("(#[^\\n]*\n\\s*)*") ;
	regex EXTRACTOR_IDEQVAL  = regex ("\\s*(\\w+)\\s*=\\s*([\\w \t()]+)\\s*") ;
	regex EXTRACTOR_ROT      = regex ("\\s*ROT=((.)+)\\s*") ;

	regex TYPECHKER_TRUEFALSE		= regex ( "(1|0|true|false|True|False|TRUE|FALSE)" ) ;
	regex TYPECHKER_BLACKWHITE		= regex ( "(None|none|NONE|Black|White|black|white|BLACK|WHITE)" ) ;
	regex TYPECHKER_4ADJ			= regex ( "DIR4_(L|U|R|D)" ) ;
	regex TYPECHKER_4DIAG			= regex ( "DIR4D?_(UL|UR|DR|DL)" ) ;
	regex TYPECHKER_8DIR			= regex ( "DIR8_(L|UL|U|UR|R|DR|D|DL)" ) ;
	regex TYPECHKER_BIT4ADJ			= regex ( "DIRV4_([Ll][Uu][Rr][Dd])" ) ;
	regex TYPECHKER_BIT4DIAG		= regex ( "DIRV4D?_(l[Xx]u[Xx]r[Xx]d[Xx])" ) ;
	regex TYPECHKER_BIT8DIR			= regex ( "DIRV8_([Ll][Xx][Uu][Xx][Rr][Xx][Dd][Xx])" ) ;
	regex TYPECHKER_VALUE_NBIT		= regex ( "(\\w+)\\s*\\((\\d+)\\)" ) ;

	const string STR_ERROR_ROTATION = "ErrorRotation" ;
}

#endif