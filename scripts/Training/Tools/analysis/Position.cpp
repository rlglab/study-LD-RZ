#include "Position.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <random>

Position::Position()
{
	m_vCandidate.resize(8); // 8 rotation
}
	
	
Position::~Position()
{
		
}

string Position::getBelongFile()
{
	return m_sBelongFile;
}
	
void Position::setPosition(const string& sBelongFile, const int& iAnswer, const vector<string>& candidateSet)
{
	m_sBelongFile = sBelongFile;
	m_iAnswer = iAnswer;
	
	for(int i=0;i<candidateSet.size();++i){ // candidateSet size should be 8
		
		stringstream ss(candidateSet[i]);
		float tmp;
		for(int j=0;j<361;++j){
			ss >> tmp;
			//cout << tmp << " " << j << endl;
			m_vCandidate[i].push_back(tmp);
		}
	}
}

int Position::getAnswer()
{
	return m_iAnswer;
}

vector<float>& Position::getSingleRotationCandidate(int symmetryType)
{
	return m_vCandidate[symmetryType];
}

vector<float> Position::getRandomNumAverage(int randomNum)
{
	//random num = 0~7
	srand(time(NULL));
	vector<int> randomBox;
	randomBox.resize(8);
	for(int i=0;i<8;++i){
		randomBox[i] = i;
	}
	random_shuffle(randomBox.begin(), randomBox.end());
	
	vector<float> vCandidate;
	vCandidate.resize(361);
	for(int i=0;i<361;++i) vCandidate[i] = 0; // initialize
	for(int i=0; i<=randomNum;++i){
		for(int j=0;j<361;++j){ 
				vCandidate[j] += (m_vCandidate[ randomBox[i] ][j]) / (randomNum + 1);
		}
	}
	
	return vCandidate;
	
}

