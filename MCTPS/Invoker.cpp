#include "Invoker.h"
#include <algorithm>
#include <cctype>
using namespace std ;

template<>
bool load<string> ( string& ref, const string& key, const string& value )
{
	if( key.find('+') != string::npos ) {
		if( ref.length()>0 ) { ref += ":"; }
		ref += value;
	}
	else { ref = value ; }
    return true ; 
}


template<>
bool load<bool> ( bool& ref, const string& key, const string& value )
{
    string buffer ;
    istringstream iss ( value ) ;
    iss >> buffer ;

    transform ( buffer.begin(), buffer.end(), buffer.begin(), ::toupper ) ;

    ref = ( buffer == "TRUE" || buffer == "1" ) ;
    return true;
} 
