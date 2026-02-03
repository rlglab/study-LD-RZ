#include "arguments.h"

namespace arguments
{
    /// ProgramOption
    void initProgramOption ( ) ; 

    /// Configure
    void initConfigure ( ) ;
    void loadConfigure(const string& conf_file, const string& conf_str) ;
	bool checkAndInitConfigure() ;
    void printDefaultConfigure ( ostream& os ) ;

    /*!
    @brief  clean temperate variables in namesapce arguments
    @author T.F. Liao
    call after all configures loaded to namespace Configure
    */
    void reset () ;

    ProgramOption po ;
    ConfigureLoader cl ;

    string mode = "gtp" ;
    string gen_file_name ;
    string conf_file ;
    string conf_str ;
}