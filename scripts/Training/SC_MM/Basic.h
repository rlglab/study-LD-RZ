#ifndef BASIC_H
#define BASIC_H

#include <vector>
#include <string>
#include <sstream>

using namespace std;

string receieveColor( string sMeg )
{
#if defined __GNUC__
	return "\33[1;32m" + sMeg + "\33[0m";
#else
	return sMeg;
#endif
}

string sendColor( string sMeg )
{
#if defined __GNUC__
	return "\33[1;34m" + sMeg + "\33[0m";
#else
	return sMeg;
#endif
}

string progressColor( string sMeg )
{
#if defined __GNUC__
	return "\33[1;37m" + sMeg + "\33[0m";
#else
	return sMeg;
#endif
}

string finishColor( string sMeg )
{
#if defined __GNUC__
	return "\33[5;37m" + sMeg + "\33[0m";
#else
	return sMeg;
#endif
}

string errorColor( string sMeg )
{
#if defined __GNUC__
	return "\33[1;31m" + sMeg + "\33[0m";
#else
	return sMeg;
#endif
}

string showTime( int num )
{
	int hour,min,second;

	hour = num / 3600;	num -= hour * 3600;
	min = num / 60;		num -= min * 60;
	second = num;

	ostringstream oss;
	if( hour>0 ) { oss << hour << " h "; }
	if( min>0 ) { oss << min << " m "; }
	if( second>0 ) { oss << second << " s "; }
	return oss.str();
}

#endif
