#include "FileExplorer.h"
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <iostream>
#include <exception>

using namespace std;

FileExplorer::FileExplorer(string sRootDirName)
{
	m_sRootDirName = sRootDirName;
	initialize();
	

}

FileExplorer::~FileExplorer()
{
}


void FileExplorer::initialize()
{
	exploreDirectory();
	
	
	
}

vector<string> FileExplorer::getdirContent(string dirName ){
    DIR *dp;
    struct dirent *dirp;
	string path = m_sRootDirName + "/" + dirName; 
	vector<string> files;
    if((dp = opendir(path.c_str())) == NULL){
        cout << "Error(" << errno << ") opening " << path << endl;
        return files;
    }
    while((dirp = readdir(dp)) != NULL){
		if(string(dirp->d_name) == "." || string(dirp->d_name) == "..") continue;
        files.push_back(string(dirp->d_name));
    }
    closedir(dp);
   
}

vector<string> FileExplorer::getdirectory(string subDir)
{
	vector<string> result; 
	DIR *dp;
    struct dirent *dirp;
	if((dp = opendir( (m_sRootDirName + "/" +  subDir).c_str())) == NULL){
        cout << "Error(" << errno << ") opening " << m_sRootDirName << endl;
        return result;
    }
	
	while((dirp = readdir(dp)) != NULL){
		//cout << string(dirp->d_name) + " !!!" << endl;
		if(string(dirp->d_name) == "." || string(dirp->d_name) == "..") continue;
		result.push_back(string(dirp->d_name));
    }
	
}


void FileExplorer::exploreDirectory()
{
	DIR *dp;
    struct dirent *dirp;
	if((dp = opendir(m_sRootDirName.c_str())) == NULL){
        cout << "Error(" << errno << ") opening " << m_sRootDirName << endl;
        return ;
    }
	while((dirp = readdir(dp)) != NULL){
		//cout << string(dirp->d_name) + " !!!" << endl;
		if(string(dirp->d_name) == "." || string(dirp->d_name) == "..") continue;
		
		m_DB_entry[string(dirp->d_name)] = getdirContent(string(dirp->d_name));
		
    }
	
}


vector<GoGame> FileExplorer::traverseDirSgfResult(string dir)
{
	vector<string> dirContent = getdirectory(dir);

	vector<GoGame> goGames;
	for(int i=0;i<dirContent.size();++i){
		GoGame g= traverseOneSgfResult(dir, dirContent[i]);
		goGames.push_back(g);
	}
	return goGames;
	
}


GoGame FileExplorer::traverseOneSgfResult(string dir, string fileName)
{
	string path = m_sRootDirName + "/" + dir + "/" + fileName;
	fstream fin(path.c_str(), ios::in);
	
	if(!fin){
		cerr << path << " not found!  " << endl;
	}
	
	string belongFile;
	string oneLine;
	int count = 0;
	int iAnswer;
	
	fin >> belongFile;
	
	GoGame goGame(fileName);
	//vector<Position> positions;
	
		while(fin >> iAnswer){
			
				getline(fin,oneLine); // lazy pass escape 
				vector<string> rotationSet;
				for(int i=0;i<8;++i){
					getline(fin, oneLine);
					rotationSet.push_back(oneLine);	
				}
				goGame.gainPosition(iAnswer, rotationSet);
				count++;
			
			
		}

	return goGame;
}


void FileExplorer::viewDBEntry()
{
	cout << m_sRootDirName << "----------------------" << endl;
	for( map<string, vector<string> >::iterator it= m_DB_entry.begin() ; it!= m_DB_entry.end(); it++  ){
		cout << "|  " << it->first << "\t size: " << it->second.size() << endl;
		//for(int i=0;i<it->second.size();++i){
		//	cout << "\t| " << it->second[i] << endl; 
		//}
		
	}
	cout << "---------------------------------" << endl;
	
}