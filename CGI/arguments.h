#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <string>
#include "ProgramOption.h"
#include "ConfigureLoader.h"
using namespace std;

namespace arguments
{
    /// ProgramOption
    extern void initProgramOption ( ) ; 

    /// Configure
    extern void initConfigure ( ) ;
    extern void loadConfigure(const string& conf_file, const string& conf_str) ;
	extern bool checkAndInitConfigure() ;
    extern void printDefaultConfigure ( ostream& os ) ;

    /*!
    @brief  clean temperate variables in namesapce arguments
    @author T.F. Liao
    call after all configures loaded to namespace Configure
    */
    extern void reset () ;

    extern ProgramOption po ;
    extern ConfigureLoader cl ;

    extern string mode  ;
    extern string gen_file_name ;
    extern string conf_file ;
    extern string conf_str ;
}

#endif