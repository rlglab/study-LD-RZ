#include "ConfigureLoader.h"
#include <fstream>
#include <cstdlib>
using namespace std;

std::string ConfigureLoader::getDefaultConfigure ()
{
    ostringstream oss ;
    map<string, vector<size_t> >::iterator it ;
    for ( it = m_groups.begin() ; it != m_groups.end() ; ++it ) {
        const string& group = it->first ;
        const vector<size_t>& options = it->second;

        if ( group.length() != 0 )
            oss << endl << "# " << group << endl;
        for ( unsigned int i=0;i<options.size();++i ) {
            const Entry& entry = m_entries[options[i]] ;
            oss << entry.key << "=" << entry.value ;
            if ( entry.desc.length() > 0 ) oss << " # " << entry.desc ;
            oss << endl;
        }
    }
    return oss.str();
}

ConfigureLoader::ConfigureLoader( )
{
    m_groups[""] ; // insert default group
}

void ConfigureLoader::exitError ( const std::string& msg )
{
    cerr << msg << endl;
    exit(1);
}

bool ConfigureLoader::loadConfigureFromFile ( const std::string& conf_file )
{
    ifstream fin ( conf_file.c_str() ) ;

    if ( !fin ) 
        return setError ( "open file '" + conf_file + "' fail.\n" ) ;

    return loadConfigure ( fin ) ;
}

bool ConfigureLoader::loadConfigureFromString( const std::string& conf_str )
{
    string nl_str ;
    bool quote = false ;
    for ( size_t i=0 ; i<conf_str.length() ; ++i ) {
        switch (conf_str[i]) {
        case '\"': quote = !quote ; break;
        case '\\': 
            if ( i+1 < conf_str.length() )
                nl_str += conf_str[i+1];
            break;
        case ':':
            if ( quote ) nl_str += ':' ;
            else nl_str += '\n';
            break;
        default:
            nl_str += conf_str[i];
            break;

        }
    }
    istringstream iss ( nl_str );

    return loadConfigure ( iss ) ;
}

bool ConfigureLoader::loadConfigure ( std::istream& is )
{
    string line ;
    while ( getline ( is, line ) ) {
        string key, value ;
        if ( !parseLine ( line, key, value ) ) {
            if ( m_error.length() != 0 ) // real error
                return false;
            continue;
        }

        if ( !setValue(key, value) ) {
            if ( m_error.length() != 0 ) // real error
                return false;
            continue;
        }
    }
    return true ;
}

bool ConfigureLoader::parseLine( string& line, string& key, string& value )
{
    size_t pos = line.find_first_of ( '#' ) ; // comment
    if ( pos != string::npos ) 
        line.resize(pos) ;

    trim(line);
    if ( line.length() == 0 )
        return false; // empty line

    // KEY = VALUE
    pos = line.find_first_of ( '=' ) ;
    if ( pos == string::npos ) 
        return setError ( "bad format near \"" + line + "\"" ) ;

    key = line.substr ( 0, pos ) ;
    value = line.substr ( pos+1 ) ;
    trim(key) ;
    strToUpper(key) ;
    trim(value) ;
    return true;
}

const std::string& ConfigureLoader::getErrorMsg() const
{
    return m_error ;
}

bool ConfigureLoader::setValue( const string& key, const string& value )
{
    if ( m_mapIdx.count(key) == 0 )
        return setWarning ( "Invalid key \""+key+"\"" ) ;

	const Entry& entry = m_entries[m_mapIdx[key]];

    if ( ! (*(entry.invoker))(key, value) )
        return setError ( "Unsatisfiable value \""+value+"\" for option \""+key+"\"") ;

    return true ;
}

bool ConfigureLoader::setError( const string& msg )
{
    m_error = msg ;
    return false ;
}

void ConfigureLoader::reset()
{
    m_entries.clear();
    m_error.clear();
    m_groups.clear();
    m_mapIdx.clear();
}

bool ConfigureLoader::setWarning( const string& msg )
{
    cerr << msg << endl;
    return false;
} 
