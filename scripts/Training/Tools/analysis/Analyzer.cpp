#include "Analyzer.h"
#include <algorithm>

#include <iostream>


Analyzer::Analyzer(const vector<GoGame>& goGames, string dirName)
{
	m_pGoGames = goGames;
	m_DirName = dirName;
	file.open((m_DirName+ ".txt").c_str(), ios::out);
	
	m_rangeL.push_back(0);
	m_rangeL.push_back(30);
	m_rangeL.push_back(60);
	m_rangeL.push_back(90);
	m_rangeL.push_back(120);
	m_rangeL.push_back(150);
	m_rangeL.push_back(180);
	m_rangeL.push_back(210);
	m_rangeL.push_back(240);
	m_rangeL.push_back(270);
	m_rangeL.push_back(300);
	
	m_rangeR.push_back(30);
	m_rangeR.push_back(60);
	m_rangeR.push_back(90);
	m_rangeR.push_back(120);
	m_rangeR.push_back(150);
	m_rangeR.push_back(180);
	m_rangeR.push_back(210);
	m_rangeR.push_back(240);
	m_rangeR.push_back(270);
	m_rangeR.push_back(300);
	m_rangeR.push_back(400);
}
Analyzer::~Analyzer()
{
	
}
	
void Analyzer::setTarget()
{
	
} 

void Analyzer::outputDirection()
{
	switch(m_rotateDirection)
	{
		case 0: file << "SYM_NORMAL, "; break;
		case 1: file << "SYM_ROTATE_90, ";break;
		case 2: file << "SYM_ROTATE_180, ";break;
		case 3: file << "SYM_ROTATE_270, ";break;
		case 4: file << "SYM_HORIZONTAL_REFLECTION, ";break;
		case 5: file << "SYM_HORIZONTAL_REFLECTION_ROTATE_90, ";break;
		case 6: file << "SYM_HORIZONTAL_REFLECTION_ROTATE_180, ";break;
		case 7: file << "SYM_HORIZONTAL_REFLECTION_ROTATE_270, ";break;
	}
	
}

void Analyzer::writeResult()
{
	cout << writeCount++ << ": ";
	float result = calculatingPredictionRate();
	if(m_mode ==0){
		if(m_record == 0)  {  outputDirection(); file << "top " << m_iTopN << ":\t\t\t\t\t" << result << endl ; }
		else if (m_record == 1) { outputDirection(); file << "phase " << m_phaseL << "-" << m_phaseR << ":\t\t\t\t\t" << result << endl ; }
	}
	else if(m_mode == 1){
		if(m_record == 0)  file << "Random " << m_iRandomNum << ", top " << m_iTopN << ":\t\t\t\t\t" << result <<  endl ;
		else if (m_record == 1) file << "Random " << m_iRandomNum << ", phase " << m_phaseL << "-" << m_phaseR << ":\t\t\t\t\t" << result << endl ;
	}
}

void Analyzer::run()
{
	/*
	 mode 0: singleDirection
	 mode 1: Random Average
	 record 0: topN 
	 record 1: range phase (topN = 1)
	 
	 
	*/
	
	m_mode = 0;
	m_record = 0;
	for(int i=0;i<8;++i){
		for(int j=1; j<=20 ; ++j){
			m_rotateDirection = i;
			m_iTopN = j;
			singleRotation(m_rotateDirection);
			toCDataCandidates();
			writeResult();
			m_vAnswerSet.clear();
			m_vCandidates.clear();
		}
	}
	
	m_mode = 1;
	m_record = 0;
	for(int i=0;i<8;++i){
		for(int j=1; j<=20 ; ++j){
			m_iRandomNum = i+1;
			m_iTopN = j;
			randomN_RotationAverage(i);
			toCDataCandidates();
			writeResult();
			m_vAnswerSet.clear();
			m_vCandidates.clear();
		}
	}
	
	m_mode = 0;
	m_record = 1;
	m_iTopN = 1;
	for(int i=0;i<8;++i){
		for(int j=0 ; j<m_rangeL.size() ; ++j){
			m_rotateDirection = i;
			m_phaseL = m_rangeL[j];
			m_phaseR = m_rangeR[j];
			
			singleRotation(i);
			toCDataCandidates();
			writeResult();
			m_vAnswerSet.clear();
			m_vCandidates.clear();
		}
	}
	
	m_mode = 1;
	m_record = 1;
	m_iTopN = 1;
	for(int i=0;i<8;++i){
		for(int j=0 ; j<m_rangeL.size() ; ++j){
			m_iRandomNum = i+1;
			m_phaseL = m_rangeL[j];
			m_phaseR = m_rangeR[j];
			
			randomN_RotationAverage(i);
			toCDataCandidates();
			writeResult();
			m_vAnswerSet.clear();
			m_vCandidates.clear();
		}
	}
	
	
	
	file.close();
}


void Analyzer::singleRotation(int direction)
{
	
	for(int i=0;i<m_pGoGames.size();++i){
		vector<Position> positions;
		if(m_record ==0) { positions = m_pGoGames[i].getAllPosition(); }
		else if(m_record == 1) {
			for(int j=m_phaseL; j < m_phaseR ; ++j){
				if(m_pGoGames[i].isKthPositionExist(j)) {
					positions.push_back(m_pGoGames[i].getKthPosition(j));
				}
				else break;
			}
		}
		for(int j=0;j<positions.size();++j){
			m_vCandidates.push_back(positions[j].getSingleRotationCandidate(direction));
			m_vAnswerSet.push_back(positions[j].getAnswer());
		}
	}
}

void Analyzer::randomN_RotationAverage(int N)
{
	
	for(int i=0;i<m_pGoGames.size();++i){
		vector<Position> positions;
		if(m_record ==0) { positions = m_pGoGames[i].getAllPosition(); }
		else if(m_record == 1) {
			for(int j=m_phaseL; j < m_phaseR ; ++j){
				if(m_pGoGames[i].isKthPositionExist(j)) {
					positions.push_back(m_pGoGames[i].getKthPosition(j));
				}
				else break;
			}
		}
		for(int j=0;j<positions.size();++j){
			m_vCandidates.push_back(positions[j].getRandomNumAverage(N));
			m_vAnswerSet.push_back(positions[j].getAnswer());
		}
	}
	
	
}


 

void Analyzer::toCDataCandidates()
{
	m_vCdatas.clear();
	for(int i=0;i<m_vCandidates.size();++i){
		vector<CData> cds;
		for(int j=0;j<m_vCandidates[i].size();++j){
			CData cd(j, m_vCandidates[i][j]);
			cds.push_back(cd);
		}
		sort(cds.begin(), cds.end());
		//for(int i=0;i<cds.size();++i) cout << cds[i].fScore << endl;
		m_vCdatas.push_back(cds);
	}
	
}





float Analyzer::calculatingPredictionRate()  // test , sum all the point
{
	int total = m_vCandidates.size();
	cout << "total " << total << " positions\r" ;
	//cout << "checking consistency... " << "answerSet size: " << m_vAnswerSet.size() << endl;
	
	
	
	float hit = 0;
	
	for(int i=0;i<m_vCandidates.size();++i){
		//cout << m_vAnswerSet[i] << " " << m_vCdatas[i][0].iPos << endl;
		
		if(m_vAnswerSet[i] == 380) { total--; continue; } // pass move
		else{
			for(int j=0;j<m_iTopN;++j){
				if(m_vAnswerSet[i] == m_vCdatas[i][j].iPos) {
					hit++;
					break;
				}
			}
		}
	}
	
	return hit/total;

	
	
	
	
	
}