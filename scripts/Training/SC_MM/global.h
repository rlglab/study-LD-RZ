#ifndef GLOBAL_H
#define GLOBAL_H

#include <time.h>
#include "GameCollection.h"

enum ThreadType {
	TYPE_EXIT,
	TYPE_DOMM,
	TYPE_LOGLIKELIHOOD,
	TYPE_COMPUTEVIC,
	TYPE_UPDATE_GAMMA,
	TYPE_ERROR
};

time_t g_start;
ThreadType g_type;
GameCollection g_gcol;
vector<double> g_vLogLikelihood;
vector< vector<double> > g_vDen;
vector< vector<double> > g_vVictories;
vector< vector<double> > g_vParticipations;
vector< vector<double> > g_vPresences;

#endif
