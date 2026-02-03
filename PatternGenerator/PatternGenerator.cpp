#include "PatternGenerator.h"
#include "TimeSystem.h"

#include <iostream>
#include <fstream>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using boost::format;


PatternGenerator::PatternGenerator()
{
    /// create data field for features build by process
    PatternFeature feature ;
    feature.duality = COLOR_NONE ;
    feature.dt = NBits;
    feature.value = 0;
    for (uint i=0;i<13;++i) feature.symbols[i]=SYM_ALL;
    {
        feature.dt = TrueFalse ;
        feature.bits = 1 ;
        feature.Id = "CompleteEmpty" ;
        m_features.push_back(feature) ;
    }
    {
        feature.dt = NBits ;
        feature.bits = 4 ;
        feature.duality = COLOR_NONE ;
        feature.Id = "EmptyGridCount" ;
        m_features.push_back(feature) ;

        feature.Id = "GridCount" ;
        feature.duality = COLOR_BLACK ;
        m_features.push_back(feature) ;
    }
    {
        feature.dt = NBits ;
        feature.bits = 3 ;
        feature.duality = COLOR_NONE ;
        feature.Id = "EmptyAdjGridCount" ;
        m_features.push_back(feature) ;
        feature.Id = "EmptyDiagGridCount" ;
        m_features.push_back(feature) ;

        feature.duality = COLOR_BLACK ;
        feature.Id = "AdjGridCount" ;
        m_features.push_back(feature) ;
        feature.Id = "DiagGridCount" ;
        m_features.push_back(feature) ;
    }
    {
        feature.dt = DirVec8 ;
        feature.bits = 8 ;
        feature.Id = "CutArea" ;
        feature.duality = COLOR_BLACK ;
        m_features.push_back(feature) ;
    }
}

bool PatternGenerator::loadFile( const string& filename )
{
    vector<PatternFeature> features ;
    if ( !m_parser.parseFromFile(filename, features) ) {
        return setError( m_parser.getError() );
    }
    for ( uint i=0;i<features.size();++i ) {
        const PatternFeature& feature = features[i];
        if ( m_map.count(feature.Id) !=0 ) {
            const PatternFeature& registered_feature = m_features[m_map[feature.Id]];
            DataType dt = registered_feature.dt;
            uint bits = registered_feature.bits;
            if ( dt != feature.dt || bits != feature.bits ) {
                return setError (
                        (format("Load file %1% error, conflict datatype for %2%\n") % filename % feature.Id).str()
                      + (format("previous declaration: %1%\n") % dt_string[dt] ).str()
                      + (format("with new declaration: %1%\n") % dt_string[feature.dt]).str() );
            }
        } else {
            m_map[feature.Id] = (uint)m_features.size() ;
            m_features.push_back(feature) ;
        }
    }
    return true;
}

void PatternGenerator::generate( )
{
    /*
    vector<PatternFeature>::iterator it ;
    for ( it = m_features.begin(); it != m_features.end(); ++it ) {
        cout << it->toString() ;
    }*/
    generateEntryFile();
    generateTableFile();
    generateBuilderFile();
}

void PatternGenerator::generateEntryFile( )
{
    ofstream fout ( (s_entry+".h").c_str() ) ;
    
    string IncludingGuard = boost::to_upper_copy ( s_entry )+"_H" ;
    fout << boost::format("#ifndef %1%\n#define %1%\n") % IncludingGuard << endl;
    fout << "#include \"PatternUtils.h\"" << endl << endl ;
    
    fout << boost::format("#define PATTERN_VERSION \"%1%\"") % TimeSystem::getTimeString("YmdHis") << endl  ;
    fout << boost::format("#define PATTERN_VERSION_SZ 14") << endl << endl ;

    fout << "namespace pattern33 {" << endl ;

    fout << boost::format("struct %1% {") % s_entry << endl;
    fout << "    struct InnerEntry {" << endl;
    vector<PatternFeature>::iterator it ;
    for ( it = m_features.begin(); it != m_features.end(); ++it ) {
        if ( it->duality == COLOR_NONE )
			fout << boost::format("        uint %1%:%2%;") % it->Id % it->bits << endl;
        else {
			fout << boost::format("        uint Black%1%:%2%;") % it->Id % it->bits << endl;
			fout << boost::format("        uint White%1%:%2%;") % it->Id % it->bits << endl;
        }        
    }
    fout << "    } inn[3];" << endl;
    fout << "};" << endl;

    fout << "}" << endl ;

    fout << endl << "#endif" << endl;
    fout.close();
}

void PatternGenerator::generateTableFile( )
{
    ofstream fout ( (s_table+".h").c_str() ) ;
    string IncludingGuard = boost::to_upper_copy ( s_table )+"_H" ;
    fout << boost::format("#ifndef %1%\n#define %1%\n") % IncludingGuard << endl;

    fout << "#include \"PatternUtils.h\"" << endl ;
    fout << boost::format("#include \"%1%.h\"") % s_entry << endl;
    fout << endl ;

    fout << "namespace pattern33 {" << endl ;
    fout << 
        "#define PatternEntryMonoGetter(name) \\\n"
        "    uint get##name ( uint pattern_index, Color center ) { \\\n"
        "        return m_table[pattern_index].inn[center].name; \\\n"
        "    } \n"
        "#define PatternEntryDualGetter(name) \\\n"
        "    uint get##name ( uint pattern_index, Color forColor, Color center ) { \\\n"
        "        return  ( (forColor==COLOR_BLACK) ? \\\n"
        "                   m_table[pattern_index].inn[center].Black##name : \\\n"
        "                   m_table[pattern_index].inn[center].White##name  ) ;\\\n" 
        "    } \n"
        "#define PatternEntrySetter(name) \\\n"
        "    static void set##name ( Pattern33Entry& entry, uint value, Symbol symbols[13] ) { \\\n"
        "        Color center = Sym2Color(symbols[0]) ; \\\n"
        "        entry.inn[center].name = value ;\\\n"
        "    }  \n"   
		<< endl ;
    
    fout << endl ;
    
    fout << boost::format("class %1% ;") % s_builder << endl << endl;

    fout << boost::format("class %1% ") % s_table << endl;
    fout << "{" << endl ;
    fout << boost::format("friend class %1% ; ") % s_builder << endl;
    fout << "private:" << endl;
    fout << boost::format("    %1%  m_table[TABLE_SIZE] ; ") % s_entry << endl;
    fout << "public:" << endl;
    fout << endl ;

    vector<PatternFeature>::iterator it ;
    for ( it = m_features.begin(); it != m_features.end(); ++it ) {
        if ( it->duality == COLOR_NONE ) 
		{
			fout << boost::format("    PatternEntrySetter(%1%) ") % it->Id << endl;
			fout << boost::format("    PatternEntryMonoGetter(%1%) ") % it->Id << endl;
        } 
		else 
		{ 
			fout << boost::format("    PatternEntrySetter(Black%1%) ") % it->Id << endl;
			fout << boost::format("    PatternEntrySetter(White%1%) ") % it->Id << endl;
			fout << boost::format("    PatternEntryDualGetter(%1%) ") % it->Id << endl;			
        }
    }
    fout << "};" << endl;
    fout << "extern Pattern33Table s_pattern_table;" << endl;
    
    fout << "class Pattern33" << endl;
    fout << "{" << endl ;
    fout << "private:" << endl ;
    fout << "    uint m_patternIndex ;" << endl ;
    fout << "public:" << endl ;
    fout << "    inline void setIndex(uint index) { m_patternIndex = index ; }" << endl ;
    fout << "    inline void addOffset(uint offset) { m_patternIndex += offset ; }" << endl ;
    fout << "    inline void subOffset(uint offset) { m_patternIndex -= offset ; }" << endl ;
    fout << "    inline uint getIndex() const { return m_patternIndex ; }" << endl ;
    fout << endl;
    
    for ( it = m_features.begin(); it != m_features.end(); ++it ) {
        if ( it->duality == COLOR_NONE ) {
			fout << boost::format ( "    uint get%1%(Color center=COLOR_NONE)const{ return s_pattern_table.get%1%(m_patternIndex,center); }" ) % it->Id << endl;			
        } else {
			fout << boost::format ( "    uint get%1%(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get%1%(m_patternIndex,forColor,center); }" ) % it->Id << endl;
        }
    }

	fout << endl ;

	fout << "	void showAllAttribute(Color forColor,Color center=COLOR_NONE) const {" << endl ;


	for ( it = m_features.begin(); it != m_features.end(); ++it ) {
		if( it->duality == COLOR_NONE )
			fout << "        cerr << \"" << boost::format ("%1%" ) % it->Id << ": \" << " << boost::format(" get%1%(center)") % it->Id << " << endl ;"  << endl ;
		else
			fout << "        cerr << \"" << boost::format ("%1%" ) % it->Id << ": \" << " << boost::format(" get%1%(forColor, center)") % it->Id << " << endl ;"  << endl ;	
	}

	fout << "    }" << endl;

    fout << "};" << endl;
    
    
    fout << "}" << endl ;


	// Generate PatternActionTable
	fout << endl;
	fout << "class PatternActionTable {" << endl;
	fout << "	public:" << endl;
	fout << "	static ull m_Pattern_Action[8][128];" << endl;
	fout << "	static ull m_Pattern_Remove[8];" << endl;
	fout << "};" << endl;

    fout << endl << "#endif" << endl;
    fout.close();
}

void PatternGenerator::generateBuilderFile()
{
    ofstream fout ( (s_builder+".h").c_str() ) ;
    string IncludingGuard = boost::to_upper_copy ( s_builder )+"_H" ;
    fout << boost::format("#ifndef %1%\n#define %1%\n") % IncludingGuard << endl;

    fout << "#include \"PatternUtils.h\"" << endl ;
    fout << boost::format("#include \"%1%.h\"") % s_entry << endl;
    fout << boost::format("#include \"%1%.h\"") % s_table << endl;
    fout << "#include <string>\n"
            "#include <map>\n"
			"#define TEST_PATTERN_BUILD 0\n"
            "using std::string;\n"<< endl ;

    fout << "namespace pattern33 {" << endl ;
    fout << 
        "#define RegisterSetter(name) \\\n"
        "    RegSetter ( #name, &Pattern33Table::set##name )  "  << endl ;

    fout << endl ;

    fout << boost::format("class %1% ") % s_builder << endl;
    fout << "{" << endl;
    fout << boost::format("    typedef void (*PatternSetter)(%1%&, uint, Symbol[13]);") % s_entry << endl;
    fout << "private:" << endl;
    fout << "    string m_error ;" << endl;
    fout << "    Pattern33Entry* m_entries ;" << endl;
    fout << "    std::map<string, PatternSetter> m_mapFn ;" << endl;
	fout << "    bool useByProcedure ;" << endl;


    fout << "public:" << endl;
    fout << boost::format("    %1%(); ") % s_builder << endl;

    fout << "    inline void initByGenerator();\n"
            "    bool build(const string& dirname);\n"
            "    string getError() const { return m_error ; }\n" 
            "    void self_test() ; \n"
            << endl;
    fout << "private:\n"
            "	bool loadPreBuilt() ;\n"
            "	void storePreBuilt() ;\n"
            "	void RegSetter(const string& ID, PatternSetter fn);\n"
            "	bool setError(const string& msg) { m_error=msg; return false ; }\n"
			"	void buildByProcedure ();\n"
            "	static void buildProcedure (Pattern33Entry&, uint, Symbol[]) ;\n"
            "	static int buildCutArea( Pattern33Entry& , uint , Symbol [], Symbol ) ;\n"
            "	bool buildPattern(const PatternFeature& feature) ;\n"
            "	void buildPattern_r(Symbol symbols[13], uint value, PatternSetter fn, int depth = 0) ;\n"
			"	int getRadius3PatternIndex(Symbol symbols[13]);\n"
            "	void RotateReflect (Symbol symbols[13], uint method) ;\n"
            "	uint RotateReflect (DataType dt, uint value, uint method) ;\n"
            "	void printPattern(Symbol[]); \n"
            "	PatternFeature getDual ( const PatternFeature& feature ) ; \n" ;
    fout << "};" << endl << endl ;

    fout << boost::format("inline void %1%::initByGenerator()") % s_builder << endl ;
    fout << "{" << endl;
    vector<PatternFeature>::iterator it ;
    for ( it = m_features.begin(); it != m_features.end(); ++it ) {
		if ( it->duality == COLOR_NONE ) {
			fout << boost::format ( "    RegisterSetter(%1%) ; ") % it->Id << endl;
		} else {
			fout << boost::format ( "    RegisterSetter(Black%1%) ; ") % it->Id << endl;
			fout << boost::format ( "    RegisterSetter(White%1%) ; ") % it->Id << endl;
		}
    }
    fout << "}" << endl;

    fout << "}" << endl ;

    fout << endl << "#endif" << endl;
    fout.close();
}

void PatternGenerator::resetError()
{
    m_error.clear();
    m_parser.resetError();
}

const string PatternGenerator::s_entry = "Pattern33Entry";
const string PatternGenerator::s_table = "Pattern33Table";
const string PatternGenerator::s_builder = "Pattern33Builder" ; 
