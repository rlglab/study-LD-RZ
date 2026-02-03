#ifndef GOGAME__H
#define GOGAME__H

#include<vector>
#include "Position.h"


using namespace std;




class GoGame{
	
	
	
public:
	GoGame();
	GoGame(string sFileName);
	~GoGame();
	
	vector<Position>& getAllPosition();
	Position& getKthPosition(int index);
	bool isKthPositionExist(int index);
	
	void gainPosition(const int& iAnswer, const vector<string>& candidateSet);
	void gainPosition(Position position);
	void setSgfName(string sFileName);
	
	
private:
	vector<Position> m_vPositions;
	string m_sSgfName;
	
	
	
};

#endif