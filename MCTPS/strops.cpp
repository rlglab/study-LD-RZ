#include "strops.h"
#include <algorithm>

void trimLeft ( string& str )
{
    size_t pos = 0 ;
    while ( pos < str.length() && isspace( str[pos] )  ) ++ pos;
    if ( pos != 0 ) {
        str = str.substr ( pos ) ;
    }
}

void trimRight ( string& str )
{
    int pos = (int)str.length()-1 ;
    while ( pos >= 0 && isspace ( str[pos] ) ) --pos ;
    if ( pos != str.length() ) {
        str.resize ( pos+1 );
    }
}

void trim ( string& str )
{
    trimLeft ( str ) ;
    trimRight ( str ) ;
}

void strToUpper ( string& str )  
{
    transform ( str.begin(), str.end(), str.begin(), toupper ) ;
}

void strToLower( string& str )  
{
    transform ( str.begin(), str.end(), str.begin(), tolower ) ;
}

#include <iostream>
std::vector<string> splitToVector ( const string& sParam, const char& cToken )
{
	string s;
	std::vector<string> vArgument;
	std::istringstream iss(sParam);
	while( std::getline(iss,s,cToken) ) { vArgument.push_back(s); }

	return vArgument;
}

bool startsWith( const string& str, const string& prefix )
{
    if ( str.length() < prefix.length() )
        return false ;
    for ( size_t i=0;i<prefix.length();++i ) 
        if ( str[i] != prefix[i] )
            return false;
    return true;
}

bool endsWith( const string& str, const string& suffix )
{
    if ( str.length() < suffix.length() )
        return false ;
    
    size_t offset = str.length() - suffix.length() ;
    for ( size_t i=0;i<suffix.length();++i ) 
        if ( str[offset+i] != suffix[i] )
            return false;
    return true;
}

template<>
string ToString<bool> (const bool& value) 
{
    return (value?"true":"false") ;
} 
