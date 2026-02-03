#include "TimeSystem.h"
#include <cstdio>

using namespace std;

boost::posix_time::ptime TimeSystem::getLocalTime () 
{
	return boost::posix_time::microsec_clock::local_time();
}

std::string TimeSystem::getTimeString ( string sFormat, boost::posix_time::ptime localtime ) 
{
	string sResult ;
	bool bEscape = false ;

	for ( size_t i = 0 ; i < sFormat.length() ; ++i ) {
		if ( bEscape ) {
			sResult += sFormat.at(i);
			bEscape = false;
		} else {
			switch ( sFormat.at(i) ) {
			case 'Y':
				sResult += translateIntToString ( localtime.date().year() );
				break;
			case 'y': 
				sResult += translateIntToString ( localtime.date().year()%100, 2 );
				break;
			case 'm':
				sResult += translateIntToString ( localtime.date().month(), 2 );
				break;
			case 'd':
				sResult += translateIntToString ( localtime.date().day(), 2 );
				break;
			case 'H':
				sResult += translateIntToString ( localtime.time_of_day().hours(), 2 );
				break;
			case 'i':
				sResult += translateIntToString ( localtime.time_of_day().minutes(), 2 );
				break;
			case 's':
				sResult += translateIntToString ( localtime.time_of_day().seconds(), 2 );
				break;
			case 'f':
				sResult += translateIntToString ( localtime.time_of_day().total_milliseconds()%1000, 3 );
				break;
			case 'u':
				sResult += translateIntToString ( localtime.time_of_day().total_microseconds()%1000000, 6 );
				break;
			case '\\':
				bEscape = true ;
				break;
			default:
				sResult += sFormat.at(i) ;
			}
		}
	}
	return sResult ;
}

std::string TimeSystem::translateIntToString ( int iVal, int iZeroFillWidth ) 
{
	char buf[16];
	static char zeroFillFormat[] = "%0*d",
		nonZeroFillFormat[] = "%*d" ;

	if ( iZeroFillWidth > 15 ) iZeroFillWidth = 15;
	if ( iZeroFillWidth < 0 ) iZeroFillWidth = 0;

	char *format = (iZeroFillWidth ? zeroFillFormat : nonZeroFillFormat);


	sprintf ( buf, format, iZeroFillWidth, iVal );
	return buf;
} 
