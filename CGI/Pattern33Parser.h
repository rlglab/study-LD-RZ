#ifndef PATTERN33PARSER_H
#define PATTERN33PARSER_H

#include "types.h"
#include <string>
#include <vector>
#include <map>
#include <boost/regex.hpp>
#include "PatternUtils.h"
#include <sstream>

namespace pattern33 
{

class Pattern33Info
{
private:
	string pattern_str ;
	string rotation_definition ;
	int lineNo ;
public:
	Pattern33Info() { clear(); } 

	void addLine(const string& n_patternStr) {
		pattern_str+=n_patternStr; 
		lineNo++ ;
	}

	void operator+(const string& n_patternStr) { 
		addLine(n_patternStr);
	}

	void setRotateInfo(string info) { rotation_definition = info; } 

	string getPatternStr() const { return pattern_str ; }
	string getRotateInfo() const { return rotation_definition ; }
	int getLineNo() const { return lineNo ; }

	bool hasSpecificRotation() const { return !rotation_definition.empty() ; }
	bool isFull() const { return (lineNo >= 5) ; }

	void clear() {
		pattern_str.clear() ;
		rotation_definition.clear() ;
		resetLineNo() ;
	}

	void resetLineNo() { lineNo = 0 ; }
};

class Pattern33Parser
{
	friend class Pattern33Info ;
private:
    std::string m_error ; 
    static const boost::regex s_recognizer[8] ;
    static const boost::regex s_extractor[8] ;
    
    static const boost::regex s_typeChecker[NumDataType] ;
	static const boost::regex s_idChecker;

	static const uint rotationTable[8][6];

    std::map<std::string, int> m_mapConsts ;	

public:
    bool parseFromFile ( const std::string& filename, std::vector<PatternFeature>& features ) ;
    bool parseFromString ( const std::string& str, std::vector<PatternFeature>& features ) ;
	void resetError () { m_error.clear(); }	
    std::string getError() const { return m_error ; }
    

private:

	bool loadConstant( const std::string& str ) ;
	bool loadPattern ( const std::string& str, std::vector<PatternFeature>& features ) ;

    Symbol interpret ( char c ) ;
	void interpretSymbols( PatternFeature& feature , const std::string& symbols );
	void setIDandDuality( PatternFeature& feature , const std::string& ID );

    bool setError ( const std::string& msg ) ;
    bool toFeature ( PatternFeature& feature, const std::string& ID, const std::string& value, const std::string& symbols ) ;

    uint resolveValue ( DataType dt, const std::string& value );
	void printPattern(string& patternStr);

	string transRotationNums(string rotDefinition);
	bool isErrorRoation( const string& rotation ) const ;

	bool matchCommentLine(const string& line) const;
	bool matchDefineLine(const string& line) const;
};

}

#endif 
