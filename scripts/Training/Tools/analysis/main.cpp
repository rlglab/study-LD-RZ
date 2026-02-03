#include <iostream>


#define DATABASE_DIR "candidateDB"

#include "FileExplorer.h"
#include "Analyzer.h"
#include "Position.h"
#include <vector>


using namespace std;






int main(int argc, char **argv)
{
	string dirName = string(argv[1]);
	//int mode = atoi(argv[2]);      // mode = 0: singledirection , mode = 1, multiple dir 
	//int direction = atoi(argv[3]); // mode0: which direction, mode1: how many num
	//int ramdomNum = atoi(argv[3]); 
	//int topN = atoi(argv[4]);  // keep top N
	
	
	// file explorer
	FileExplorer fileExplorer(DATABASE_DIR);
	fileExplorer.viewDBEntry();
	
	cout << "exploring directory: " << argv[1] << "..." << endl;
	Analyzer analyzer( fileExplorer.traverseDirSgfResult(string(argv[1])), string(argv[1]));
	
	
	analyzer.run();
		cout << endl;
	
	
	return 0;
}