#ifndef __TIMESYSTEM__
#define __TIMESYSTEM__

#include <cassert>
#include <string>
#include <ctime>
#include <boost/date_time/posix_time/posix_time.hpp>
#define DO_WINDOWS_TIMER 0
#if DO_WINDOWS_TIMER
#include "windows.h"
#endif
class TimeSystem
{
public:
    /*!
        @brief  get the current time
        @author kwchen
    */
    static boost::posix_time::ptime getLocalTime () ;

    /*!
        @brief  get format string 
        @author T.F. Liao
        @author kwchen
        @param  sFormat [in]    the format of output string
        @param  timeStamp   [in]    the timestamp to be translated
        @return the formated string
    */
    static std::string getTimeString ( std::string sFormat = "Y/m/d H:i:s", boost::posix_time::ptime timeStamp = getLocalTime() ) ;

private:
    /*!
        @breif  translate integer to string with specified minimal width of output
        @author T.F. Liao
        @param  iVal    [in] integer to be translate
        @param  iWidth  [in] minimal width
        @return the translated string
    */
    static std::string translateIntToString ( int iVal, int iWidth = 0) ;
};

class StopWatch
{
    typedef boost::posix_time::time_duration::tick_type tick_type ;
    static const int ticks_per_second = 1000 ;
private:
    boost::posix_time::ptime start ;
    tick_type total_ms ;

#ifndef NDEBUG
    int state ;
    static const int 
        STOPWATCH_NULL = 0, 
        STOPWATCH_RUNNING = 1,
        STOPWATCH_STOP = 2 ;
#endif

public:
    StopWatch () { reset() ; }

    inline void reset ( ) {
        total_ms = (tick_type)0 ;

#ifndef NDEBUG
        state = STOPWATCH_NULL ;
#endif
    }
    inline void run ( ) {
#ifndef NDEBUG
        assert ( state==STOPWATCH_NULL );
        state = STOPWATCH_RUNNING ;
#endif
        start = boost::posix_time::microsec_clock::local_time() ;
    }
    inline void stop ( ) {
#ifndef NDEBUG
        assert ( state==STOPWATCH_RUNNING );
        state = STOPWATCH_STOP ;
#endif
        boost::posix_time::ptime now (boost::posix_time::microsec_clock::local_time());
        total_ms += (now-start).total_milliseconds();
    }
    inline void rerun ( ) {
#ifndef NDEBUG
        assert ( state==STOPWATCH_STOP );
        state = STOPWATCH_RUNNING ;
#endif
        start = boost::posix_time::microsec_clock::local_time() ;
    }

    inline float getRunSeconds ( ) const {
        assert ( state==STOPWATCH_RUNNING ) ;
        boost::posix_time::ptime now (boost::posix_time::microsec_clock::local_time());
        //std::cerr<<total_ms<<" "<<now<<" "<<start<<std::endl;
        return (total_ms+(now-start).total_milliseconds())/(float)ticks_per_second ;
    }
    inline float getTotalSeconds ( ) const {
        assert ( state==STOPWATCH_STOP );
        return total_ms/(float)ticks_per_second;
    }
    inline float getTotalMiniSeconds ( ) const {
        assert ( state==STOPWATCH_STOP);
        return (float)total_ms;
    }
};

class Timer 
{
    typedef boost::posix_time::time_duration::tick_type tick_type ;
    static const int ticks_per_second = 1000 ;
private:
    boost::posix_time::ptime start ;
    tick_type limit_ms ;

#ifndef NDEBUG
    int state ;
    static const int 
        TIMER_NULL = 0, 
        TIMER_RUNNING = 1;
#endif


public:
    Timer() { reset(); }

    inline void reset () {
        limit_ms = (tick_type) 0 ;
#ifndef NDEBUG
        state = TIMER_NULL ;
#endif
    }

    inline void setLimit ( double seconds ) {
#ifndef NDEBUG
        assert ( state == TIMER_NULL ) ;
#endif
        limit_ms = (tick_type) ( seconds*ticks_per_second ) ;
    }
    inline void run () {
#ifndef NDEBUG
        assert ( state == TIMER_NULL ) ;
        state = TIMER_RUNNING ;
#endif
        start = boost::posix_time::microsec_clock::local_time() ;
    }
    inline bool timeUp () {
#ifndef NDEBUG
        assert ( state == TIMER_RUNNING ) ;
#endif
        boost::posix_time::ptime now (boost::posix_time::microsec_clock::local_time());
        return (now-start).total_milliseconds() >= limit_ms ;
    }

};
#if DO_WINDOWS_TIMER
class MicrosecondTimer{
private:
    std::string m_timerName;
    double m_PCFreq;
    __int64 m_counterStart;
    double m_totalMicroSeconds;
    size_t m_counter;
    
public:
    MicrosecondTimer(){ m_timerName=""; m_PCFreq = 0.0; m_counterStart = 0; m_totalMicroSeconds = 0.0; m_counter = 0; }
    inline double getTimerTotalMicroSeconds(){ return m_totalMicroSeconds; }
    inline size_t getTimerTotalCounts(){ return m_counter; }

    void startStopWatch(){
        LARGE_INTEGER li;
        if(!QueryPerformanceFrequency(&li)) return;

        m_PCFreq = (double)li.QuadPart/1000000.0;
        QueryPerformanceCounter(&li);
        m_counterStart = li.QuadPart;
    }
    void stopStopWatch () {
        m_totalMicroSeconds += getStopWatchTime(/*timerName*/);
        m_counter++;
    }
private:
    double getStopWatchTime(){
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);
        return (double)li.QuadPart-m_counterStart;
    }
};
#endif

#endif 
