#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <random>
#include <string>
#define MAX_BUFFER 4096
#define TRAINING_CHECK_CONVERGED_TIMES 5
#define TRAINING_ITERATION_THRESHOLD 100
#define TRAINING_DIFFERENCE_THRESHOLD 0.01
#define TRAINING_MATCH_DIFFERENCE_THRESHOLD 5
#define STOP_STATE 2000000

using namespace std;

class Team {
public:
	string toString()
	{
		ostringstream oss;
		for( unsigned int i=0; i<m_vFeatures.size(); i++ ) {
			oss << m_vFeatures[i] << " ";
		}
		return oss.str();
	}
	vector<int> m_vFeatures;
};

class State {
public:
	void clear()
	{
		m_winTeam.m_vFeatures.clear();
		m_vTeam.clear();
	}
	string toString()
	{
		ostringstream oss;
		oss << "win : " << m_winTeam.toString() << endl;
		for( unsigned int i=0; i<m_vTeam.size(); i++ ) {
			oss << "lose: " << m_vTeam[i].toString() << endl;
		}
		return oss.str();
	}
	Team m_winTeam;
	vector<Team> m_vTeam;
};

const double g_alpha = 0.001;
const double g_lambda_w = 0.001;
const double g_lambda_v = 0.002;
int g_dimension = 5;
bool g_bIsTesting;
double *g_weight;
double **g_L2vector;
int g_numFeatures;
int g_matchDifference;
double g_dPreRate;
string g_sTestFileName;
string g_sTrainFileName;
vector<State> g_state;

void LFR();
void initialize();
void setNumFeature();
bool isConverged( int iteration );
bool getOneState( fstream& fin, State& state, bool bIsStart );
double estimateY( const Team& team, bool bShowDetail=false );
void modifyWeight( const Team& winTeam, const Team& team, bool bIsWinTeam=false );
Team readTeam( char* line );
void calculatePredictionRate();
void saveDB();
void readDB();
void showDB();
void readTrainingData();

int main( int argc, char* argv[] )
{
	if( argc<2 || argc>3 ) {
		cerr << "./LFR trainFile.txt" << endl;
		cerr << "./LFR modeTest testFile.txt" << endl;
		return 0;
	}
	
	if( argc==2 ) {
		g_bIsTesting = false;
		g_sTrainFileName = argv[1];
		initialize();
		LFR();
		saveDB();
	} else {
		g_bIsTesting = true;
		g_sTestFileName = argv[2];
		initialize();
		calculatePredictionRate();
	}
}

void initialize()
{
	default_random_engine generator;
	normal_distribution<double> distribution(0,0.1);

	cerr << "initialize..." << endl;
	
	setNumFeature();
	g_state.clear();
	
	g_weight = new double[g_numFeatures];
	for( int i=0; i<g_numFeatures; i++ ) {
		g_weight[i] = 0.0f;
	}

	g_L2vector = new double*[g_numFeatures];
	for( int i=0; i<g_numFeatures; i++ ) {
		g_L2vector[i] = new double[g_dimension];
		for( int j=0; j<g_dimension; j++ ) {
			g_L2vector[i][j] = distribution(generator);
		}
	}
	
	g_dPreRate = 0.0f;
	g_matchDifference = 0;
	
	readDB();
	if( !g_bIsTesting ) { readTrainingData(); }
}

void setNumFeature()
{
	char cLine[MAX_BUFFER];
	string sFileName;
	if( g_bIsTesting ) { sFileName = g_sTestFileName; }
	else { sFileName = g_sTrainFileName; }
	fstream fin(sFileName.c_str(),ios::in);
	
	if( !fin ) {
		cerr << "Can't open " << g_sTrainFileName << endl;
		exit(0);
	}
	
	string sExclamation;
	fin.getline(cLine,MAX_BUFFER);
	if( cLine[0]!='!' ) {
		cerr << "File format error!" << endl;
		exit(0);
	}
	
	istringstream in(cLine);
	in >> sExclamation >> g_numFeatures;
}

bool getOneState( fstream& fin, State& state, bool bIsStart )
{
	char cLine[MAX_BUFFER];
	if( bIsStart ) {
		uint twoStart = 0;
		while( fin.getline(cLine,MAX_BUFFER) ) {
			if( cLine[0]=='!' ) { ++twoStart; }
			if( twoStart==2 ) { break; }
		}
		
		fin.getline(cLine,MAX_BUFFER);
		if( cLine[0]!='#' ) {
			cerr << "File format error!" << endl;
			exit(0);
		}
	}
	
	state.clear();
	
	bool bIsSuccess = false;
	bool bIsFirstLine = true;
	while( fin.getline(cLine,MAX_BUFFER) ) {
		if( bIsFirstLine ) {
			state.m_winTeam = readTeam(cLine);
			bIsFirstLine = false;
			bIsSuccess = true;
			continue;
		}
		
		if( cLine[0]=='#' ) {
			// end header
			break;
		} else {
			// save lose team
			state.m_vTeam.push_back(readTeam(cLine));
		}
	}
	
	return bIsSuccess;
}

Team readTeam( char* line )
{
	Team team;
	int iIndex;
	istringstream in(line);
	
	while( true ) {
		in >> iIndex;
		if( iIndex>=g_numFeatures ) { cerr << "error feature No." << iIndex << endl; }
		if( in ) { team.m_vFeatures.push_back(iIndex); }
		else { break; }
	}
	
	return team;
}

double estimateY( const Team& team, bool bShowDetail )
{
	double dScore = g_weight[0];
	double dInteraction[g_dimension];
	
	if( bShowDetail ) { cerr << "g_weight[0]: " << g_weight[0] << " " << dScore << endl; }
	
	for( int i=0; i<g_dimension; i++ ) { dInteraction[i] = 0.0f; }
	
	for( int i=0; i<team.m_vFeatures.size(); i++ ) {
		const int feature_i = team.m_vFeatures[i];
		
		dScore += g_weight[feature_i];
		for( int k=0; k<g_dimension; k++ ) {
			dScore += dInteraction[k] * g_L2vector[feature_i][k];
			dInteraction[k] += g_L2vector[feature_i][k];
		}
	}
	
	if( bShowDetail ) { cout << endl << endl; }
	
	return dScore;
}

void modifyWeight( const Team& winTeam, const Team& team, bool bIsWinTeam )
{
	double calY = estimateY( team );
	double calWinTeam = estimateY( winTeam );
	if( calY + 1E-7 < calWinTeam ) { return; }
	
	double deltaY;
	if( bIsWinTeam ) { deltaY = calY - 1; }
	else { deltaY = calY; }
	
	// w0
	g_weight[0] -= g_alpha * deltaY;
	
	// each wi
	for ( int i=0; i<team.m_vFeatures.size(); i++ ) {
		const int feature_i = team.m_vFeatures[i];
		g_weight[feature_i] -= g_alpha * (deltaY + g_lambda_w * g_weight[feature_i]);
	}
	
	// each vif
	for ( int k=0; k<g_dimension; k++ ) {
		double dSum = 0.0;
		double dGradient = 0.0;
		for( int i=0; i<team.m_vFeatures.size(); i++ ) {
			const int feature_i = team.m_vFeatures[i];
			dSum += g_L2vector[feature_i][k];
		}
		
		for( int i=0; i<team.m_vFeatures.size(); i++ ) {
			const int feature_i = team.m_vFeatures[i];
			dGradient = dSum - g_L2vector[feature_i][k];
			g_L2vector[feature_i][k] -= g_alpha * (deltaY * dGradient + g_lambda_v * g_L2vector[feature_i][k]);
		}
	}
}

void readTrainingData()
{
	cerr << "reading data file..." << endl;

	State state;
	int iCounter = 0;
	fstream fin(g_sTrainFileName.c_str(),ios::in);
	getOneState(fin,state,true);
	
	do {
		g_state.push_back(state);
		cerr << ++iCounter << "        \r";
	} while( getOneState(fin,state,false) );
}

void LFR()
{
	int iIteration = 0;
	cerr << "start to do LFR..." << endl;
	
	do {
		cerr << "Iteration: " << ++iIteration << endl;
		for( unsigned int i=0; i<g_state.size(); i++ ) {
			if( i%10000==0 ) { cerr << ++i << "        \r"; }
			State& state = g_state[i];
			modifyWeight( state.m_winTeam, state.m_winTeam, true );
			for ( int s=0; s<state.m_vTeam.size(); s++ ) {
				modifyWeight( state.m_winTeam, state.m_vTeam[s] );
			}
		}
	} while( !isConverged(iIteration) );
}

bool isConverged( int iteration )
{
	if( iteration%TRAINING_CHECK_CONVERGED_TIMES!=0 ) { return false; }
	
	saveDB();

	double dWinScore;
	int matchState = 0;
	int totalState = 0;

	for( unsigned int i=0; i<g_state.size(); i++ ) {
		State& state = g_state[i];
		
		bool bIsMatch = true;
		dWinScore = estimateY(state.m_winTeam);
		
		for( int s=0; s<state.m_vTeam.size(); s++ ) {			
			if( estimateY(state.m_vTeam[s])>=dWinScore ) {
				bIsMatch = false;
				break;
			}
		}
		
		if( bIsMatch ) { matchState++; }
		totalState++;
		
		if( totalState%10000==0 ) {
			cerr << "\rmatchState = " << matchState << ", totalState = " << totalState 
				 << ", prediction rate = " << ((double)matchState * 100 / totalState) << " %" << "    \r";
		}
	}
	
	double dCurrentRate = ((double)matchState * 100 / totalState);
	double dDifference = dCurrentRate-g_dPreRate;
	cerr << "previous prediction rate = " << g_dPreRate
		 << " %; current prediction rate = " << dCurrentRate
		 << " %; difference = " << dDifference << " %" <<endl;
		 
	g_dPreRate = dCurrentRate;
	
	if( dDifference<TRAINING_DIFFERENCE_THRESHOLD && iteration>TRAINING_ITERATION_THRESHOLD ) { g_matchDifference++; }
	else { g_matchDifference = 0; }
	
	if( iteration>TRAINING_ITERATION_THRESHOLD && g_matchDifference>TRAINING_MATCH_DIFFERENCE_THRESHOLD ) { return true; }
	
	return false;
}

void calculatePredictionRate()
{
	State state;
	double dWinScore;
	double matchState = 0;
	double totalState = 0;
	
	cerr << "calculate prediction rate..." << endl;
	
	fstream fin(g_sTestFileName.c_str(),ios::in);
	getOneState(fin,state,true);
	do {		
		bool bIsMatch = true;
		double dSameScore = 1;
		dWinScore = estimateY(state.m_winTeam);
		for( int s=0; s<state.m_vTeam.size(); s++ ) {
			double Y = estimateY(state.m_vTeam[s]);
			if( Y==dWinScore ) {
				dSameScore += 1;
			} else if( Y>dWinScore ){
				bIsMatch = false;
				break;
			}
		}

		if( bIsMatch ) { matchState += 1/dSameScore; }
		totalState += 1;
		
		cerr << "\rmatchState = " << matchState << ", totalState = " << totalState 
			 << ", prediction rate = " << (matchState * 100 / totalState) << " %" << "    \r";

	} while( getOneState(fin,state,false) );
	
	cerr << endl;
}

void saveDB()
{
	ofstream fout("LFR.db",ios::out|ios::binary);
	if( !fout ) {
		printf("Save db failed.\n");
		return;
	}
	fout.write(reinterpret_cast<char*>(&g_dimension), sizeof(int));
	fout.write(reinterpret_cast<char*>(g_weight), sizeof(double)*g_numFeatures);
	for( int i=0; i<g_numFeatures; i++ ) {
		fout.write(reinterpret_cast<char*>(g_L2vector[i]), sizeof(double)*g_dimension);
	}
	fout.close();
}

void readDB()
{
	ifstream fin("LFR.db",ios::in|ios::binary);
	if( !fin ) {
		printf("Load db failed.\n");
		return;
	}
	fin.read(reinterpret_cast<char*>(&g_dimension), sizeof(int));
	fin.read(reinterpret_cast<char*>(g_weight), sizeof(double)*g_numFeatures);
	for( int i=0; i<g_numFeatures; i++ ) {
		fin.read(reinterpret_cast<char*>(g_L2vector[i]), sizeof(double)*g_dimension);
	}
	fin.close();
}

void showDB()
{
	for( int i=0; i<g_numFeatures; i++ ) {
		if( g_weight[i]>1 || g_weight[i]<-1 ) { cout << "g_weight[" << i << "] = " << g_weight[i] << endl; }
		//cout << "g_weight[" << i << "] = " << g_weight[i] << endl;
	}
	
	for( int i=0; i<g_numFeatures; i++ ) {
		for( int j=0; j<g_dimension; j++ ) {
			if( g_L2vector[i][j]>1 || g_L2vector[i][j]<-1 ) { cout << "g_L2vector[" << i << "][" << j << "] = " << g_L2vector[i][j] << endl; }
			//cout << "g_L2vector[" << i << "][" << j << "] = " << g_L2vector[i][j] << endl;
		}
	}
}
