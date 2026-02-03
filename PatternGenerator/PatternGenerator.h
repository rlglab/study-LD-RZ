#ifndef PATTERNGENERATOR_H
#define PATTERNGENERATOR_H

#include "Pattern33Parser.h"
#include "PatternUtils.h"
#include <map>

using namespace pattern33 ;
using std::string ;

class PatternGenerator
{
private:
    Pattern33Parser m_parser ;
    std::map<string, uint> m_map ;
    std::vector<PatternFeature> m_features;

    string m_error ;
    static const string s_entry;
    static const string s_table ;
    static const string s_builder;

public:
    
    PatternGenerator() ;

    bool loadFile ( const string& filename ) ;
    void generate ( ) ;
    string getError () const { return m_error; }

    void resetError () ;



private:

    void generateEntryFile ( ) ;
    void generateTableFile (  ) ;
    void generateBuilderFile (  ) ;

    bool setError ( string msg ) { m_error = msg ; return false; }		

};

#endif 
 
