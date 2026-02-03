#ifndef CDATA__H
#define CDATA__H


class CData
{
public:
	CData(int pos, float score){  iPos = pos, fScore = score ;}
	friend bool operator<(const CData& d1, const CData& d2){  return d1.fScore > d2.fScore ;}
	int iPos;
	float fScore;
	
};


#endif