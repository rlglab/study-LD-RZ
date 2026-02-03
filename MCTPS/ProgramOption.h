#ifndef PROGRAMOPTION_H
#define PROGRAMOPTION_H

#include <string>
#include <sstream>
#include <vector>
#include <map>

#include "Invoker.h"

using std::string ;

class ProgramOption
{
    struct Option 
    {
        string key;
        string desc ;
        bool flag ;
        mutable BaseInvoker* invoker ;

        Option ( const string& key, const string& desc, BaseInvoker* invoker, bool flag )
            : key(key), desc(desc), flag(flag), invoker(invoker) {}
        ~Option () { if (invoker) delete invoker ; }
        Option ( const Option& rhs )
            : key(rhs.key), desc(rhs.desc), flag(rhs.flag), invoker(rhs.invoker)
        { rhs.invoker = NULL ; }

        Option& operator= ( const Option& rhs )
        {
            key = rhs.key ;
            desc = rhs.desc ;
            flag = rhs.flag ;
            invoker = rhs.invoker ;
            rhs.invoker = NULL ;
            return *this;
        }
    };

    std::vector<Option> m_options ;
    string m_error ;
    std::map<string, size_t> m_mapIdx ;

public:
    template<class T, class _Loader>
    void regOption ( const string& key, T& ref, const string& desc, _Loader loader )  {
        _register ( key, ref, desc, loader, false ) ;
    }

    template<class T>
    void regOption ( const string& key, T& ref, const string& desc )  {
        _register ( key, ref, desc, load<T>, false ) ;
    }

    template<class _Loader>
    void regFlagOption ( const string& key, bool& ref, const string& desc, _Loader loader ) {
        _register ( key, ref, desc, loader, true ) ;
    }

    void regFlagOption ( const string& key, bool& ref, const string& desc ) {
        _register ( key, ref, desc, ValueSetter<bool>(true), true ) ;
    }

    void reset () ;
    bool parse ( int argc, char** argv ) ;
    string getErrorMsg() const ;
    string getUsage(const string& progname) const ;

    void exitError ( const string& msg ) ;

private:
    template<class T, class _Loader>
    void _register ( string key, T& ref, const string& desc, _Loader loader, bool flag )  {
        unsigned int len = 0;
        while ( len < key.length() && key[len] == '-' ) ++ len ;
        if ( len != 0 )
            key.replace(0, len, ""); // remove leading '-'

        if ( key.length() == 0 ) 
            exitError ( "no key specified." ) ;

        if ( m_mapIdx.count(key) != 0 ) 
            exitError ( "duplicate key " + key + " registered, ignored it." ) ;

        m_mapIdx[key] = m_options.size(); // next index
        m_options.push_back ( Option ( key, desc, new Invoker<T, _Loader>(ref, loader), flag ) ) ;
    }
    bool setError ( const string& msg ) ;
};

#endif 
