#include "GoGame.h"





void GoGame::gainPosition( const int& iAnswer, const vector<string>& candidateSet)
{
	Position position;
	position.setPosition(m_sSgfName, iAnswer, candidateSet);
	m_vPositions.push_back(position);
	
}

GoGame::GoGame()
{
	
}
GoGame::GoGame(string sFileName)
{
	m_sSgfName = sFileName;
}

GoGame::~GoGame()
{
	
}

vector<Position>& GoGame::getAllPosition()
{
	return m_vPositions;
}

Position& GoGame::getKthPosition(int index)
{
	return m_vPositions[index];
}


void GoGame::gainPosition(Position position)
{
	m_vPositions.push_back(position);
}

void GoGame::setSgfName(string sFileName)
{
	m_sSgfName = sFileName;
}

bool GoGame::isKthPositionExist(int index)
{
	return (index >= m_vPositions.size()) ? false : true;
}