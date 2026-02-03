#ifndef POSITION__H
#define POSITION__H

#include <vector>
#include <string>

using namespace std;

enum SymmetryType
{
	SYM_NORMAL, SYM_ROTATE_90, SYM_ROTATE_180, SYM_ROTATE_270,
	SYM_HORIZONTAL_REFLECTION, SYM_HORIZONTAL_REFLECTION_ROTATE_90,
	SYM_HORIZONTAL_REFLECTION_ROTATE_180, SYM_HORIZONTAL_REFLECTION_ROTATE_270,
	SYMMERTY_SIZE
};


class Position{
	
	public:
		Position();
		~Position();
		
		void setPosition(const string& sBelongFile, const int& iAnswer, const vector<string>& candidateSet);
		vector<float>& getSingleRotationCandidate(int symmetryType);
		vector<float> getRandomNumAverage(int randomNum);
		
		
		
		string getBelongFile();
		int getAnswer();
		
	private:
		vector<vector<float> > m_vCandidate;
		int m_iAnswer;
		string m_sBelongFile;
	
	
};



#endif