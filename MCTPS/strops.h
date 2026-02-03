#ifndef STROPS_H
#define STROPS_H

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

using std::string ;

void trimLeft ( string& str ) ;
void trimRight ( string& str ) ;
void trim ( string& str ) ;
void strToUpper ( string& str ) ;
void strToLower ( string& str ) ;

std::vector<string> splitToVector ( const string& str, const char& cToken ) ;

bool startsWith ( const string& str, const string& prefix ) ;
bool endsWith ( const string& str, const string& suffix ) ;

template<class T>
string ToString ( const T& value )
{
    std::ostringstream oss ;
    oss << value;
    return oss.str();
}

template<>
string ToString<bool> (const bool& value) ;

template<class T>
string ToString ( const T& value, int precision ) 
{
    std::ostringstream oss ;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

#endif 
