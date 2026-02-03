#ifndef __BASEGTPENGINE__
#define __BASEGTPENGINE__

#include <vector>
#include <map>
#include <string>
#include <iostream>

#include <boost/shared_ptr.hpp>

class BaseGtpEngine
{
protected:
	enum GtpResponse {
		GTP_NONE = 0,
		GTP_FAIL = '?',
		GTP_SUCC = '='
	};
private:
	// typedef void (BaseGtpEngine::*Func)();
	struct Func 
	{
		virtual void operator()() = 0 ;
	};

	typedef boost::shared_ptr<Func> FuncPtr ;

	template<typename _T, typename _F>
	struct FuncInherit : public Func 
	{
		_T* m_instance ;
		_F m_fp ;
		FuncInherit(_T* instance, _F fp) : m_instance(instance), m_fp(fp) {}
		void operator()() { (*m_instance.*m_fp)(); }
	};

	class Entry
	{
	public:
		Entry () {}
		Entry ( std::string key, FuncPtr fp, size_t min_argc = 0, size_t max_argc = 0 ) ;
		size_t min_argc () const ;
		size_t max_argc () const ;
		const std::string& getKey () const ;
		// Func getFunction () const ;
		FuncPtr getFunction () const ;
		bool operator< ( const Entry& entry ) const;
	private:
		std::string m_key ;
		FuncPtr m_fp ;
		size_t m_iMinArgc;
		size_t m_iMaxArgc ;
	};

	// typedef pair<std::string, Function> Entry;

public:
	BaseGtpEngine( std::ostream & os ) ;
	bool parseCommand( const std::string & sCommandLine ) ;
	void runCommand ( const std::string & sCommandLine ) ;
	virtual void checkAndReplaceCommand() {}
	bool hasQuit ( ) const ;

protected:
	template<typename _T, typename _Function>
	void RegisterFunction ( const std::string& sCommand, _T* instance, _Function fp, int min_argc, int max_argc = -1 ) ;

	void Unregister ( const std::string& sCommand ) { m_mapFuncion.erase(sCommand); }

	virtual void cmdName ( ) ; 
	virtual void cmdVersion ( ) ; 
	virtual void cmdListCommands ( ) ; 
	virtual void cmdKnownCommand ( ) ;
	virtual void cmdProtocolVersion ( ) ;
	virtual void cmdDefault ( ) ;
	virtual void cmdQuit ( ) ;

	void reply ( GtpResponse type, const std::string& sReply ) ;

	void setName ( std::string name, std::string version = "1.0" ) ;

	virtual void beforeRunCommand() {}
	virtual void afterRunCommand() {}

private:
	/// avoid copy construct or copy assignment
	BaseGtpEngine ( const BaseGtpEngine& ) ;
	BaseGtpEngine& operator= ( const BaseGtpEngine& ) ;

	// Function getFunctionToCall ( const std::string & sCommand ) ;
	const Entry* getEntry ( const std::string& sCommand ) ;
	bool checkArgSize ( const Entry* pEntry ) ;

private:
	// one program must only has one gtp engine
	static std::map<std::string, Entry> m_mapFuncion;
	std::ostream& m_os ;

protected:
	static std::string m_command ;
	static std::vector<std::string> m_args ;
	static std::string m_commandLine ;
	static std::string m_id;

	static std::string m_name ;
	static std::string m_version ;

	static bool m_quit ;
};

template<typename _T, typename _Function>
void BaseGtpEngine::RegisterFunction( const std::string& sCommand, _T* instance, _Function fp, int min_argc, int max_argc /*= -1 */ )
{
	if ( max_argc < min_argc ) max_argc = min_argc ;
	m_mapFuncion[sCommand] = Entry (sCommand, FuncPtr(new FuncInherit<_T, _Function>(instance, fp)), min_argc, max_argc) ;
}

#endif
 
