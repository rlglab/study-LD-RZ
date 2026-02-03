#ifndef FILE_EXPLORER__H
#define FILE_EXPLORER__H


#include <map>
#include <vector>
#include <string>
#include <fstream>
#include "Position.h"
#include "GoGame.h"

using namespace std;



class FileExplorer{
	

public:

	FileExplorer(string sRootDirName);
	~FileExplorer();
	void initialize();
	void viewDBEntry();
	
	
	GoGame traverseOneSgfResult(string dir, string fileName);
	vector<GoGame> traverseDirSgfResult(string dir);
	
	
	
private:
	void exploreDirectory();  // initialize DB entry
	vector<string> getdirContent(string dirName);
	vector<string> getdirectory(string subDir);
	
	
	
	
	
	map<string, vector<string> > m_DB_entry;
	string m_sRootDirName;
	
};




#endif