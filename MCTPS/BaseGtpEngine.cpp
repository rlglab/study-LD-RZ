#include "BaseGtpEngine.h"
#include <sstream>
#include <cassert>
#include "Logger.h"
using namespace std;

std::map<std::string, BaseGtpEngine::Entry> BaseGtpEngine::m_mapFuncion;
std::string BaseGtpEngine::m_command ;
std::vector<std::string> BaseGtpEngine::m_args ;
std::string BaseGtpEngine::m_commandLine ;
std::string BaseGtpEngine::m_id;
std::string BaseGtpEngine::m_name ;
std::string BaseGtpEngine::m_version ;
bool BaseGtpEngine::m_quit = false;

////// BaseGtpEngine::Entry //////

BaseGtpEngine::Entry::Entry( std::string key, FuncPtr fp, size_t min_argc /*= 0*/, size_t max_argc /*= 0 */ )
	: m_key(key), m_fp(fp), m_iMinArgc(min_argc), m_iMaxArgc(max_argc)
{
}

size_t BaseGtpEngine::Entry::min_argc() const
{
	return m_iMinArgc ;
}

size_t BaseGtpEngine::Entry::max_argc() const
{
	return m_iMaxArgc ;
}

const std::string& BaseGtpEngine::Entry::getKey() const
{
	return m_key ;
}

BaseGtpEngine::FuncPtr BaseGtpEngine::Entry::getFunction() const
{
	return m_fp ;
}

bool BaseGtpEngine::Entry::operator<( const Entry& entry ) const
{
	return this->m_key < entry.m_key ;
}

////// BaseGtpEngine::Entry //////

////// BaseGtpEngine //////

BaseGtpEngine::BaseGtpEngine( std::ostream & os )
	: m_os(os)
{
	RegisterFunction( "list_commands", this, &BaseGtpEngine::cmdListCommands, 0 ) ;
	RegisterFunction( "known_command", this, &BaseGtpEngine::cmdKnownCommand, 1 ) ;
	RegisterFunction( "protocol_version", this, &BaseGtpEngine::cmdProtocolVersion, 0 ) ;
	RegisterFunction( "quit", this, &BaseGtpEngine::cmdQuit, 0 ) ;
}

void BaseGtpEngine::cmdListCommands()
{
	ostringstream oss ;
	map<string, Entry>::iterator it ;
	for ( it=m_mapFuncion.begin() ; it!=m_mapFuncion.end() ; ++it ) {
		oss << it->first << endl;
	}
	reply ( GTP_SUCC, oss.str() ) ;
}

void BaseGtpEngine::cmdKnownCommand()
{
	reply ( GTP_SUCC, ((m_mapFuncion.find(m_args[0])!=m_mapFuncion.end())?"true":"false") );
}

void BaseGtpEngine::cmdQuit()
{
	m_quit = true ;
	reply ( GTP_SUCC, "" ) ;
}

void BaseGtpEngine::cmdProtocolVersion()
{
	reply ( GTP_SUCC, "2" ) ;
}

void BaseGtpEngine::cmdDefault()
{
	reply ( GTP_FAIL, "unknown command" ) ;
}

void BaseGtpEngine::reply( GtpResponse type, const string& sReply )
{
	assert ( type != GTP_NONE ) ;
	int nl_cnt = 0;
	for ( int i=(int)sReply.length()-1;i>=0;--i ) {
		if ( sReply[i]=='\n' ) ++nl_cnt ;
		else break;
	}
	string sMessage = sReply ;

	if ( nl_cnt == 0 ) sMessage += "\n\n";
	else if ( nl_cnt == 1 ) sMessage += "\n" ;

	m_os << char(type) << m_id << ' ' << sMessage ;
	m_os.flush();
}

bool BaseGtpEngine::hasQuit() const
{
	return m_quit ;
}

bool BaseGtpEngine::parseCommand( const std::string & sCommandLine )
{
	CERR() << "IN: " << sCommandLine << endl;
	m_id.clear() ;
	m_command.clear();
	m_args.clear() ;
	m_commandLine = sCommandLine ;

	size_t found = m_commandLine.find('#');
	if ( found != string::npos ) {
		m_commandLine.resize(found) ;
	}

	istringstream iss ( m_commandLine ) ;

	iss >> m_command >> ws ;
	if ( m_command.length()==0 ) {
		return false; // empty line
	}

	if ( isdigit( m_command.at(0) ) ) {
		m_id = m_command ;
		iss >> m_command >> ws ;
	}

	string arg;
	while ( iss >> arg ) {
		m_args.push_back ( arg );
	}

	return true;
}

void BaseGtpEngine::runCommand( const std::string & sCommandLine )
{
	if( !parseCommand(sCommandLine) ) { return ; }

	checkAndReplaceCommand();

	map<string, Entry>::iterator it = m_mapFuncion.find(m_command);

	if ( it == m_mapFuncion.end() ) {
		cmdDefault () ;
	} else {
		Entry* entry = &(it->second) ;
		if ( !checkArgSize( entry ) ) {
			return ;
		} else {
			// (this->*(entry->getFunction())) () ;
			FuncPtr p = entry->getFunction();
			beforeRunCommand();
			(*p)();
			afterRunCommand();
			// (*(*(entry->getFunction())))();
		}
	}
}

bool BaseGtpEngine::checkArgSize( const Entry* pEntry )
{
	size_t sz = m_args.size();
	size_t upper = pEntry->max_argc(), lower = pEntry->min_argc() ;
	if ( sz < lower || sz > upper ) {
		ostringstream oss ;
		oss << "command needs " ;

		if ( lower == upper ) {
			oss << "exactly " << lower << " argument" << (lower==1?"":"s") ;
		} else {
			oss << lower << " to " << upper << " arguments" ;
		}
		reply ( GTP_FAIL, oss.str() ) ;
		return false;
	} else {
		return true;
	}
}

void BaseGtpEngine::cmdName()
{
	reply ( GTP_SUCC, m_name ) ;
}

void BaseGtpEngine::cmdVersion()
{
	reply ( GTP_SUCC, m_version ) ;
}

void BaseGtpEngine::setName( std::string name, std::string version /*= "1.0" */ )
{
	m_name = name ;
	m_version = version ;
} 
