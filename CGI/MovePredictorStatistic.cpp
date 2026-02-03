#include "MovePredictorStatistic.h"

MovePredictorStatistic::MovePredictorStatistic()
{
}

void MovePredictorStatistic::initialize()
{
	m_vStageCounter.resize(MAX_STAGE,0);

	m_vMovePredictionRate.resize(WeichiConfigure::TotalGrids,0);
	m_vStageMovePredictionRate.resize(MAX_STAGE);
	for( uint i=0; i<m_vStageMovePredictionRate.size(); i++ ) { m_vStageMovePredictionRate[i].resize(WeichiConfigure::TotalGrids,0); }
	
	m_vContinuousStep.resize(MAX_STAGE,0);
	m_vContinuousMovePredictionRate.resize(CONTINUOUS_LIMIT);
	for( uint i=0; i<m_vContinuousMovePredictionRate.size(); i++ ) { m_vContinuousMovePredictionRate[i].resize(WeichiConfigure::TotalGrids,0); }
	m_vStageContinuousMovePredictionRate.resize(MAX_STAGE);
	for( uint i=0; i<m_vStageContinuousMovePredictionRate.size(); i++ ) {
		m_vStageContinuousMovePredictionRate[i].resize(CONTINUOUS_LIMIT);
		for( uint j=0; j<m_vStageContinuousMovePredictionRate[i].size(); j++ ) { m_vStageContinuousMovePredictionRate[i][j].resize(WeichiConfigure::TotalGrids,0); }
	}

	m_dProbabilityPrediction = 0;
	m_vStageProbabiltyPrediction.resize(MAX_STAGE,0);

	m_vWinMoveProbabiltyCount.resize(WINMOVE_PROBABILITY_SCALE,0);
	m_vStageWinMoveProbabilityCount.resize(MAX_STAGE);
	for( uint i=0; i<m_vStageWinMoveProbabilityCount.size(); i++ ) { m_vStageWinMoveProbabilityCount[i].resize(WINMOVE_PROBABILITY_SCALE,0); }
}

void MovePredictorStatistic::addStageCounter( uint moveNumber )
{
	m_vStageCounter[moveNumber/GAME_PHASE]++;
}

void MovePredictorStatistic::addMovePredictionRate( double& dWinScore, Vector<double,MAX_NUM_GRIDS>& vScore, uint moveNumber )
{
	int numSame = 0, numLarge = 0;
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		if( vScore[*it]>dWinScore ) { ++numLarge; }
		else if( vScore[*it]==dWinScore ) { ++numSame; }
	}

	for( int i=0; i<numSame; i++ ) {
		m_vMovePredictionRate[numLarge+i] += (double)1/numSame;
		m_vStageMovePredictionRate[moveNumber/GAME_PHASE][numLarge+i] += (double)1/numSame;
	}
}

void MovePredictorStatistic::addContinuousMovePredictionRate( uint limit, uint moveNumber, bool bIsWinMove, bool bIsNewGame )
{
	if( bIsNewGame ) { m_vContinuousStep[limit] = 0; }
	if( !bIsWinMove ) { m_vContinuousStep[limit] = 0; return; }

	++m_vContinuousStep[limit];
	for( uint i=0; i<m_vContinuousStep[limit]; i++ ) {
		m_vContinuousMovePredictionRate[limit][i]++;
		m_vStageContinuousMovePredictionRate[moveNumber/GAME_PHASE][limit][i]++;
	}
}

void MovePredictorStatistic::addProbabilityPrediction( double& dWinScore, double& dTotalScore, Vector<double,MAX_NUM_GRIDS>& vScore, uint moveNumber )
{
	double dAccumulatProbability = (vScore[PASS_MOVE.getPosition()]>dWinScore) ? vScore[PASS_MOVE.getPosition()] : 0.0f;
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		if( vScore[*it]>=dWinScore ) { dAccumulatProbability += vScore[*it]; }
	}

	m_dProbabilityPrediction += dAccumulatProbability/dTotalScore;
	m_vStageProbabiltyPrediction[moveNumber/GAME_PHASE] += dAccumulatProbability/dTotalScore;
}

void MovePredictorStatistic::addWinMoveProbability( double& dWinScore, double& dTotalScore, Vector<double,MAX_NUM_GRIDS>& vScore, uint moveNumber )
{
	int rank = static_cast<int>(dWinScore*100/dTotalScore);
	if( rank>100 ) { CERR() <<"............." << endl; }
	m_vWinMoveProbabiltyCount[rank]++;
	m_vStageWinMoveProbabilityCount[moveNumber/GAME_PHASE][rank]++;
}

void MovePredictorStatistic::summarizeStatisticData( const MovePredictorStatistic& rhs )
{
	for( uint i=0; i<m_vMovePredictionRate.size(); i++ ) { m_vMovePredictionRate[i] += rhs.m_vMovePredictionRate[i]; }
	for( uint i=0; i<m_vStageMovePredictionRate.size(); i++ ) {
		for( uint j=0; j<m_vStageMovePredictionRate[j].size(); j++ ) {
			m_vStageMovePredictionRate[i][j] += rhs.m_vStageMovePredictionRate[i][j];
		}
	}

	for( uint i=0; i<m_vContinuousStep.size(); i++ ) { m_vContinuousStep[i] += rhs.m_vContinuousStep[i]; }
	for( uint i=0; i<m_vStageCounter.size(); i++ ) { m_vStageCounter[i] += rhs.m_vStageCounter[i]; }
	for( uint i=0; i<m_vContinuousMovePredictionRate.size(); i++ ) {
		for( uint j=0; j<m_vContinuousMovePredictionRate[j].size(); j++ ) {
			m_vContinuousMovePredictionRate[i][j] += rhs.m_vContinuousMovePredictionRate[i][j];
		}
	}
	for( uint i=0; i<m_vStageContinuousMovePredictionRate.size(); i++ ) {
		for( uint j=0; j<m_vStageContinuousMovePredictionRate[j].size(); j++ ) {
			for( uint k=0; k<m_vStageContinuousMovePredictionRate[i][j].size(); k++ ) {
				m_vStageContinuousMovePredictionRate[i][j][k] += rhs.m_vStageContinuousMovePredictionRate[i][j][k];
			}
		}
	}

	m_dProbabilityPrediction += rhs.m_dProbabilityPrediction;
	for( uint i=0; i<m_vStageProbabiltyPrediction.size(); i++ ) { m_vStageProbabiltyPrediction[i] += rhs.m_vStageProbabiltyPrediction[i]; }

	for( uint i=0; i<m_vWinMoveProbabiltyCount.size(); i++ ) { m_vWinMoveProbabiltyCount[i] += rhs.m_vWinMoveProbabiltyCount[i]; }
	for( uint i=0; i<m_vStageWinMoveProbabilityCount.size(); i++ ) {
		for( uint j=0; j<m_vStageWinMoveProbabilityCount[i].size(); j++ ) {
			m_vStageWinMoveProbabilityCount[i][j] += rhs.m_vStageWinMoveProbabilityCount[i][j];
		}
	}
}

void MovePredictorStatistic::outputMovePredictorStatistic( string sFileName )
{
	fstream fout(sFileName.c_str(),ios::out);

	outputMovePredictionRate(fout);
	for( uint limit=0; limit<CONTINUOUS_LIMIT; limit++ ) { outputContinuousMovePredictionRate(fout,limit); }
	outputProbabilityPredictionRate(fout);
	outputWinMoveProbability(fout);
}

void MovePredictorStatistic::outputMovePredictionRate( fstream& fout )
{
	double numTotal = 0.0;
	double numAccumulation = 0.0;
	double numStageTotal[MAX_STAGE] = { 0.0f };
	double numStageAccumulation[MAX_STAGE] = { 0.0f };

	for( uint iIndex=0; iIndex<WeichiConfigure::TotalGrids; iIndex++ ) {
		numTotal += m_vMovePredictionRate[iIndex];
		for( uint stage=0; stage<MAX_STAGE; stage++ ) {
			numStageTotal[stage] += m_vStageMovePredictionRate[stage][iIndex];
		}
	}

	uint totalMoves = 0;
	for( uint stage=0; stage<MAX_STAGE; stage++ ) { totalMoves += m_vStageCounter[stage]; }

	// header line
	fout << "Move Prediction Rate:" << endl;
	fout << "rank  " << setw(6) << "All";
	for( uint stage=0; stage<MAX_STAGE; stage++ ) { fout << "  " << setw(6) << stage*30; }
	fout << endl;

	const int MAX_PREDICT_MOVES = 50;
	for( uint iIndex=0; iIndex<MAX_PREDICT_MOVES; iIndex++ ) {
		numAccumulation += m_vMovePredictionRate[iIndex];
		for( uint stage=0; stage<MAX_STAGE; stage++ ) { numStageAccumulation[stage] += m_vStageMovePredictionRate[stage][iIndex]; }

		fout << setw(4) << (iIndex+1) << setw(7) << fixed << setprecision(2) << numAccumulation*100/totalMoves << '%';
		for( uint stage=0; stage<MAX_STAGE; stage++ ) {
			if( numStageTotal[stage]==0 ) { break; }
			fout << setw(7) << numStageAccumulation[stage]*100/m_vStageCounter[stage] << '%';
		}
		fout << endl;
	}

	for( uint i=0; i<20; i++ ) { fout << "="; }
	fout << endl;
}

void MovePredictorStatistic::outputContinuousMovePredictionRate( fstream& fout, uint limit )
{
	uint totalMoves = 0;
	for( uint stage=0; stage<MAX_STAGE; stage++ ) { totalMoves += m_vStageCounter[stage]; }

	// header line
	fout << "Continuous Move Prediction Rate: (choose from top " << limit+1 << " moves)" << endl;
	fout << "step  " << setw(6) << "All";
	for( uint stage=0; stage<MAX_STAGE; stage++ ) { fout << "  " << setw(6) << stage*30; }
	fout << endl;

	for( uint iIndex=0; iIndex<WeichiConfigure::TotalGrids; iIndex++ ) {
		if( m_vContinuousMovePredictionRate[limit][iIndex]==0 ) { break; }
		fout << setw(4) << (iIndex+1) << setw(7) << fixed << setprecision(2) << m_vContinuousMovePredictionRate[limit][iIndex]*100/totalMoves << '%';

		for( uint stage=0; stage<MAX_STAGE; stage++ ) {
			if( m_vStageContinuousMovePredictionRate[stage][limit][iIndex]==0 ) { fout << setw(8) << " "; continue; }
			fout << setw(7) << m_vStageContinuousMovePredictionRate[stage][limit][iIndex]*100/m_vStageCounter[stage] << '%';
		}
		fout << endl;
	}

	for( uint i=0; i<20; i++ ) { fout << "="; }
	fout << endl;
}

void MovePredictorStatistic::outputProbabilityPredictionRate( fstream& fout )
{
	uint totalMoves = 0;
	for( uint stage=0; stage<MAX_STAGE; stage++ ) { totalMoves += m_vStageCounter[stage]; }

	// header line
	fout << "Probability Move Prediction:" << endl;
	fout << "prob  " << setw(6) << "All";
	for( uint stage=0; stage<MAX_STAGE; stage++ ) { fout << "  " << setw(6) << stage*30; }
	fout << endl;

	fout << setw(4) << " " << setw(8) << fixed << setprecision(2) << m_dProbabilityPrediction/totalMoves;
	for( uint stage=0; stage<MAX_STAGE; stage++ ) {
		if( m_vStageProbabiltyPrediction[stage]==0 ) { break; }
		fout << setw(8) << m_vStageProbabiltyPrediction[stage]/m_vStageCounter[stage];
	}
	fout << endl;

	for( uint i=0; i<20; i++ ) { fout << "="; }
	fout << endl;
}

void MovePredictorStatistic::outputWinMoveProbability( fstream& fout )
{
	// header line
	fout << "Win Move Probability" << endl;
	fout << "prob  " << setw(6) << "All";
	for( uint stage=0; stage<MAX_STAGE; stage++ ) { fout << "  " << setw(6) << stage*30; }
	fout << endl;

	for( uint iIndex=0; iIndex<WINMOVE_PROBABILITY_SCALE; iIndex++ ) {
		fout << setw(4) << iIndex << setw(8) << m_vWinMoveProbabiltyCount[iIndex];

		for( uint stage=0; stage<MAX_STAGE; stage++ ) {
			fout << setw(8) << m_vStageWinMoveProbabilityCount[stage][iIndex];
		}
		fout << endl;
	}

	for( uint i=0; i<20; i++ ) { fout << "="; }
	fout << endl;
}