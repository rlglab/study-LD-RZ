#include "ProgramOption.h"
#include <iostream>
#include <cstdlib>
using namespace std;

void ProgramOption::reset ()
{
    m_mapIdx.clear();
    m_error.clear();
    m_options.clear();
}

bool ProgramOption::parse ( int argc, char** argv ) 
{
    for ( int i=1;i<argc;++i ) {
        const char* p = argv[i];
        while ( *p=='-' ) ++p ;
        string key = p ;
        
        if ( m_mapIdx.count(key) == 0 ) 
            return setError ( "Unknown option \""+key+"\"" ); 
        
        Option& option = m_options[m_mapIdx[key]] ;
        string value = "" ;
        if ( !option.flag ) {
            if ( ++i >= argc ) 
                return setError ( "missing argument for \""+key+"\"" ) ;
            value = argv[i] ;
        }
        if ( ! (*(option.invoker))(key, value) ) {
            return setError ( "Unsatisfiable value \""+value+"\" for option \""+key+"\"") ;
        }
    }
    return true;
}

bool ProgramOption::setError ( const std::string& msg ) 
{
    m_error = msg ;
    return false ;
}

string ProgramOption::getErrorMsg() const 
{
    return m_error ;
}

void ProgramOption::exitError ( const std::string& msg ) 
{
    cerr << msg << endl;
    exit(1);
}

string ProgramOption::getUsage ( const string& progname ) const 
{
    ostringstream oss ;
    oss << "Usage: " << progname << " [options].\n";
    for ( unsigned int i=0 ; i<m_options.size(); ++i ) {
        const Option& option = m_options[i];
        oss << "\t-" << option.key << "\t" << (option.flag?"[flag]\t":"\t") << option.desc << endl;
    }
    return oss.str();
} 
