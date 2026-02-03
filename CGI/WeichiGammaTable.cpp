#include "WeichiGammaTable.h"
#include "WeichiProbabilityPlayoutHandler.h"

uint WeichiGammaTable::MAX_NUM_FEATURES;
int WeichiGammaTable::m_dimension;
double *WeichiGammaTable::m_dMMFeatureValue;
double *WeichiGammaTable::m_dLFRFeatureValue;
double *WeichiGammaTable::m_dPlayoutFeatureValue;
double **WeichiGammaTable::m_dLFRFeatureInteractionValue;
RadiusGammaOpenAddrHashTable WeichiGammaTable::m_htRadiusPatternData;
int WeichiGammaTable::m_vNumRadiusPattern[MAX_RADIUS_SIZE];

void WeichiGammaTable::initailize()
{
	loadRadiusPatternTable();
	initializeFeatureValue();

	if( WeichiConfigure::mcts_use_mm ) { loadMMDataBase(); }
	if( WeichiConfigure::mcts_use_lfr ) { loadLFRDataBase(); }
	if( WeichiConfigure::use_probability_playout ) { loadPlayoutFeatureDataBase(); }
}

void WeichiGammaTable::loadRadiusPatternTable()
{
	CERR() << "Load radius pattern ... ";

	char cLine[4096];
	MAX_NUM_FEATURES = MOVE_FEATURE_SIZE;
	string sDirectory = WeichiConfigure::db_dir + "radius_pattern.txt";
	fstream fin(sDirectory.c_str(),ios::in);
	if( !fin ) {
		CERR() << " ==> failed! Cannot find \"" << sDirectory << "\"" << endl;
		return;
	}

	m_htRadiusPatternData.clear();
	for( int i=0; i<MAX_RADIUS_SIZE; i++ ) { m_vNumRadiusPattern[i] = 0; }

	while( fin.getline(cLine,4096) ) {
		istringstream in(cLine);
		ull key;
		int iRadius,iWin,iTotal;
		string sIndex;
		in >> iRadius >> hex >> key >> dec >> iTotal >> iWin >> sIndex;

		string token;
		istringstream ss(sIndex);
		Vector<uint,MAX_RADIUS_SIZE> vIndex;
		vIndex.setAllAs(0,MAX_RADIUS_SIZE);
		for( int i=MIN_RADIUS_SIZE; getline(ss,token,','); i++ ) {
			vIndex[i] = atoi(token.c_str());
		}

		// start to write hash key & index value
		map<HashKey64,bool> mapKeyCheck;
		Vector<Vector<HashKey64,SYMMETRY_SIZE>,MAX_RADIUS_SIZE> vKeys = WeichiRadiusPatternTable::calculateRadiusPatternRotationHashKey(vIndex);

		for( int symmetric=0; symmetric<SYMMETRY_SIZE; symmetric++ ) {
			if( mapKeyCheck.find(vKeys[iRadius][symmetric])!=mapKeyCheck.end() ) { continue; }
			m_htRadiusPatternData.store(vKeys[iRadius][symmetric],MAX_NUM_FEATURES);
			mapKeyCheck[vKeys[iRadius][symmetric]] = true;
		}

		MAX_NUM_FEATURES++;
		m_vNumRadiusPattern[iRadius]++;
	}

	fin.close();

	CERR() << " ==> succeed! Total read " << (MAX_NUM_FEATURES-MOVE_FEATURE_SIZE) << " pattern(s)." << endl;
}

void WeichiGammaTable::initializeFeatureValue()
{
	if( WeichiConfigure::mcts_use_mm ) { m_dMMFeatureValue = new double[MAX_NUM_FEATURES]; }
	if( WeichiConfigure::mcts_use_lfr ) {
		m_dLFRFeatureValue = new double[MAX_NUM_FEATURES];
		m_dLFRFeatureInteractionValue = new double*[MAX_NUM_FEATURES];
	}

	for( uint i=0; i<MAX_NUM_FEATURES; i++ ) {
		if( WeichiConfigure::mcts_use_mm ) { m_dMMFeatureValue[i] = 1.0f; }
		if( WeichiConfigure::mcts_use_lfr ) { m_dLFRFeatureValue[i] = 0.0f; }
	}

	if( WeichiConfigure::use_probability_playout ) {
		m_dPlayoutFeatureValue = new double[MAX_NUM_PLAYOUT_FEATURES];
		for( int i=0; i<MAX_NUM_PLAYOUT_FEATURES; i++ ) { m_dPlayoutFeatureValue[i] = 0.0f; }
	}
}

void WeichiGammaTable::loadMMDataBase()
{
	CERR() << "Load MM DB ... ";

	string sDirectory = WeichiConfigure::db_dir + "MM.db";
	ifstream fin(sDirectory.c_str(),ios::in|ios::binary);
	if( !fin ) {
		CERR() << "Load " << sDirectory << " failed." << endl;
		return;
	}

	fin.read(reinterpret_cast<char*>(m_dMMFeatureValue), sizeof(double)*MAX_NUM_FEATURES);
	fin.close();

	CERR() << " ==> succeed!" << endl;
}

void WeichiGammaTable::loadLFRDataBase()
{
	CERR() << "Load LFR DB ... ";

	string sDirectory = WeichiConfigure::db_dir + "LFR.db";
	ifstream fin(sDirectory.c_str(),ios::in|ios::binary);
	if( !fin ) {
		CERR() << " ==> failed! Cannot find \"" << sDirectory << "\"" << endl;
		return;
	}

	fin.read(reinterpret_cast<char*>(&m_dimension), sizeof(int));
	fin.read(reinterpret_cast<char*>(m_dLFRFeatureValue), sizeof(double)*MAX_NUM_FEATURES);
	for( uint i=0; i<MAX_NUM_FEATURES; i++ ) {
		m_dLFRFeatureInteractionValue[i] = new double[m_dimension];
		fin.read(reinterpret_cast<char*>(m_dLFRFeatureInteractionValue[i]), sizeof(double)*m_dimension);
	}
	fin.close();

	CERR() << " ==> succeed!" << endl;
}

void WeichiGammaTable::loadPlayoutFeatureDataBase()
{
	CERR() << "Load playout DB ... ";

	string sDirectory = WeichiConfigure::db_dir + "playout.db";
	ifstream fin(sDirectory.c_str(),ios::in|ios::binary);
	if( !fin ) {
		CERR() << " ==> failed! Cannot find \"" << sDirectory << "\"" << endl;
		return;
	}

	fin.read(reinterpret_cast<char*>(m_dPlayoutFeatureValue), sizeof(double)*MAX_NUM_PLAYOUT_FEATURES);
	fin.close();

	// change gamma to theta
	m_dPlayoutFeatureValue[MAX_LEGAL_3X3PATTERN_SIZE] = m_dPlayoutFeatureValue[MAX_LEGAL_3X3PATTERN_SIZE + MAX_3X3PATTERN_SIZE] = 0.0f;
	for( uint i=0; i<MAX_NUM_PLAYOUT_FEATURES; i++ ) {
		//CERR() << m_dPlayoutFeatureValue[i] << endl;
		//m_dPlayoutFeatureValue[i] *= m_dPlayoutFeatureValue[i];//2*log(m_dPlayoutFeatureValue[i]);
		//m_dPlayoutFeatureValue[i] = 1.0f;
	}

	CERR() << " ==> succeed!" << endl;
}