
#ifndef ANALYZER__H
#define ANALYZER__H


#include "Position.h"
#include "GoGame.h"
#include <vector>
#include <map>
#include "CData.h"
#include <fstream>

using namespace std;

class Analyzer{
	
public:	
	Analyzer(const vector<GoGame>& goGames, string dirName);
	~Analyzer();
	
	void setTarget(); // how many positions as target position
	void singleRotation(int direction);
	void randomN_RotationAverage(int N);
	void run();
	void toCDataCandidates();
	float calculatingPredictionRate();
	void initialize(int mode, int direction, int randomNum, int record);
	void writeResult();
	void outputDirection();
private:
	vector<vector<float> > m_vCandidates; // should be 361
	vector< vector<CData> > m_vCdatas;
	
	vector<int> m_vAnswerSet;
	vector<Position> m_vPositions; // all positions need to be evaluated
	vector<GoGame> m_pGoGames;
	
	string m_DirName;
	int m_positionSize;
	int m_mode;    // 0: singleRotation,    1: randomNum
	int m_iRandomNum;
	int m_rotateDirection;
	int m_iTopN;
	int m_record;  // 0: topN, 1:rangePhase
	int m_phaseL;
	int m_phaseR;
	int writeCount = 0;
	vector<int> m_rangeL;
	vector<int> m_rangeR;
	fstream file;
	
};




#endif