#ifndef GAMECOLLECTION_H
#define GAMECOLLECTION_H

#include "Basic.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

using namespace std;

class Team {
private:
	static vector<int> m_vFeature;
	
	unsigned int m_iSize;
	unsigned long long m_iStartIndex;

public:
	Team(): m_iSize(0), m_iStartIndex(m_vFeature.size()) {}
	unsigned int getSize() const { return m_iSize; }
	int getIndex( unsigned int idx ) const
	{
		unsigned long long realIndex = m_iStartIndex + idx;
		return m_vFeature[realIndex];
	}
	void append( int iFeature )
	{
		m_vFeature.push_back(iFeature);
		m_iSize++;
	}
};
vector<int> Team::m_vFeature;

Team readTeam( string& s )
{
	Team team;
	int iIndex;
	istringstream in(s);

	while( true ) {
		in >> iIndex;

		if( in ) { team.append(iIndex); }
		else { break; }
	}

	return team;
}

class Game {
public:
	Team m_winner;
	vector<Team> m_vParticipants;
};

class GameCollection {
public:
	int m_numGames;
	int m_numFeatures;
	vector<Game> m_vGame;
	vector<double> m_vGamma;
	vector<int> m_vFeatureIndex;
	vector<string> m_vFeatureName;
	vector<double> m_vVictories;
	vector<double> m_vParticipations;
	vector<double> m_vPresences;

	void readFeatureInfo( istream& in );
	void readGameCollection( istream& in );
	
	double computeTeamGamma( const Team& team ) const;
};

void GameCollection::readFeatureInfo( istream& in )
{
	{
		// read number of gammas
		string sLine;
		getline(in,sLine);
		istringstream is(sLine);
		string s;
		int numGammas = 0;
		is >> s >> numGammas;
		m_numFeatures = numGammas;
		m_vGamma.resize(numGammas);
		m_vVictories.resize(numGammas);
		m_vParticipations.resize(numGammas);
		m_vPresences.resize(numGammas);
		for( int i=0; i<numGammas; i++ ) {
			m_vGamma[i] = 1.0f;
			m_vVictories[i] = m_vParticipations[i] = m_vPresences[i] = 0;
		}
	}

	{
		// read features
		m_vFeatureIndex.push_back(0);
		int iFeatures = 0;
		in >> iFeatures;
		for( int i=0; i<iFeatures; i++ ) {
			int Gammas;
			in >> Gammas;
			int min = m_vFeatureIndex.back();
			m_vFeatureIndex.push_back(min+Gammas);
			string sName;
			in >> sName;
			m_vFeatureName.push_back(sName);
		}
	}

	ostringstream oss;
	oss << "=== total read " << m_numFeatures << " features";
	cerr << progressColor(oss.str()) << endl;
}

void GameCollection::readGameCollection( istream& in )
{
	cerr << progressColor("=== read game collection") << endl;

	string sLine;
	getline(in,sLine);

	while( in ) {
		if( sLine!="#" ) {
			cerr << ".";
			getline(in,sLine);
			continue;
		}

		// parse a game
		Game game;
			
		// winner
		getline(in,sLine);
		game.m_winner = readTeam(sLine);
		
		// participants
		getline(in,sLine);
		while( sLine[0]!='#' && sLine[0]!='!' && in ) {
			Team team = readTeam(sLine);
			game.m_vParticipants.push_back(team);
			getline(in,sLine);
		}

		// push game
		m_vGame.push_back(game);
	}

	ostringstream oss;
	oss << "=== total read " << m_vGame.size() << " games";
	cerr << progressColor(oss.str()) << endl;
}

double GameCollection::computeTeamGamma( const Team& team ) const
{
	double dGamma = 1.0;
	for( unsigned int i=0; i<team.getSize(); i++ ) {
		dGamma *= m_vGamma[team.getIndex(i)];
	}
	return dGamma;
}

#endif
