#ifndef MOVEPREDICTORSTATISTIC_H
#define MOVEPREDICTORSTATISTIC_H

#include "BasicType.h"
#include "StaticBoard.h"
#include <vector>

class MovePredictorStatistic
{
public:
	static const int CONTINUOUS_LIMIT = 10;

private:
	vector<uint> m_vStageCounter;

	// move prediction
	vector<double> m_vMovePredictionRate;
	vector< vector<double> > m_vStageMovePredictionRate;
	
	// continuous move prediction
	vector<uint> m_vContinuousStep;
	vector< vector<double> > m_vContinuousMovePredictionRate;
	vector< vector< vector<double> > > m_vStageContinuousMovePredictionRate;

	// probability prediction
	double m_dProbabilityPrediction;
	vector<double> m_vStageProbabiltyPrediction;

	// win move probability
	vector<uint> m_vWinMoveProbabiltyCount;
	vector< vector<uint> > m_vStageWinMoveProbabilityCount;

	static const int GAME_PHASE = 30;
	static const int MAX_STAGE = MAX_GAME_LENGTH/GAME_PHASE+1;
	static const int WINMOVE_PROBABILITY_SCALE = 100;

public:
	MovePredictorStatistic();

	void initialize();
	void addStageCounter( uint moveNumber );
	void addMovePredictionRate( double& dWinScore, Vector<double,MAX_NUM_GRIDS>& vScore, uint moveNumber );
	void addContinuousMovePredictionRate( uint limit, uint moveNumber, bool bIsWinMove, bool bIsNewGame );
	void addProbabilityPrediction( double& dWinScore, double& dTotalScore, Vector<double,MAX_NUM_GRIDS>& vScore, uint moveNumber );
	void addWinMoveProbability( double& dWinScore, double& dTotalScore, Vector<double,MAX_NUM_GRIDS>& vScore, uint moveNumber );

	void summarizeStatisticData( const MovePredictorStatistic& rhs );
	void outputMovePredictorStatistic( string sFileName );

private:
	void outputMovePredictionRate( fstream& fout );
	void outputContinuousMovePredictionRate( fstream& fout, uint limit );
	void outputProbabilityPredictionRate( fstream& fout );
	void outputWinMoveProbability( fstream& fout );
};

#endif