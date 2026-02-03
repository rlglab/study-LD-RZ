#ifndef CONFIGURELOADER_H
#define CONFIGURELOADER_H

#include "Invoker.h"
#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include "strops.h"
using std::string ;

class ConfigureLoader
{
    struct Entry {
        string key;
        string value ;
        string desc ;
        mutable BaseInvoker* invoker ; 

        Entry ( const string& key, const string& value, const string& desc, BaseInvoker* invoker )
            : key(key), value(value), desc(desc), invoker(invoker) {}
        ~Entry () { if (invoker) delete invoker ; }
        Entry ( const Entry& rhs )
            : key(rhs.key), value(rhs.value), desc(rhs.desc), invoker(rhs.invoker)
        { rhs.invoker = NULL ; }

        Entry& operator= ( const Entry& rhs )
        {
            key = rhs.key ;
            desc = rhs.desc ;
            value = rhs.value ;
            invoker = rhs.invoker ;
            rhs.invoker = NULL ;
            return *this;
        }
    };
    std::map<string, std::vector<size_t> > m_groups ;
    std::vector<Entry> m_entries ;
    string m_error ;
    std::map<string, size_t> m_mapIdx ;

public:

    ConfigureLoader ( ) ;
    bool loadConfigureFromFile ( const string& conf_file ) ;
    bool loadConfigureFromString ( const string& conf_str ) ;

    template<class T, class _Loader>
    void addOption ( string key, T& ref, const string& value,  
                     const string& comment, const string& group, _Loader loader ) 
    {
        trim(key) ;
        if ( key.length() == 0 ) 
            exitError ( "no key specified." ) ;
        
        strToUpper(key) ;

        if ( m_mapIdx.count(key) != 0 && key.find('+') == string::npos ) {
            std::cerr << "duplicate key " << key << " registered, ignored it." << std::endl;
        }

        m_groups[group].push_back(m_entries.size()) ;
        m_mapIdx[key] = m_entries.size();
        m_entries.push_back ( Entry(key, value, comment, new Invoker<T, _Loader>(ref, loader)) ) ;
    }

    template<class T>
    void addOption ( const string& key, T& ref, const string& value,  
        const string& comment = "", const string& group = "" ) 
    {
        addOption(key, ref, value, comment, group, load<T>) ;
    }

    string getDefaultConfigure () ;

    const string& getErrorMsg () const ;

    void reset () ;

private:

    bool loadConfigure ( std::istream& is ) ; 
    bool parseLine ( string& line, string& key, string& value ) ;
    bool setValue ( const string& key, const string& value ) ;

    bool setWarning ( const string& msg );
    bool setError ( const string& msg ) ;

    void exitError ( const string& msg ) ;
};

#endif 
