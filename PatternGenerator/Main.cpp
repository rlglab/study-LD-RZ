#include "Pattern33Parser.h"
#include "PatternGenerator.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
using namespace std;
using namespace pattern33;

Pattern33Parser parser;
PatternGenerator generator ;


int main ( int argc, char* argv[])
{

	if ( argc != 2 ) {
		cerr << "Usage: " << argv[0] << " <dirname>" <<endl;
		return 1 ;
	}
	using namespace boost::filesystem;
	using boost::format;
	string dirname = argv[1];

	if ( !exists(dirname) ) {
		cerr << format("directory \"%1%\" not exists.") % dirname << endl;
		return 1;
	}

	bool hasError = false ;
	directory_iterator end ;
	for ( directory_iterator it(dirname); it!=end; ++it) {
		vector<PatternFeature> features;
		string file = it->path().string();
		if ( is_directory(file) ) continue;
		cerr << format("Loading file \"%1%\" ... ") % file << endl;
		if ( !generator.loadFile ( file ) ) {
			cerr << generator.getError() << endl;
			hasError = true ;
			generator.resetError();			
		}
	}
	if ( !hasError ) {
		generator.generate();
		cerr << "Files generated." << endl;
	}

	return 0;
}