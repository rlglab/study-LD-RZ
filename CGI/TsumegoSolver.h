#ifndef TSUMEGO_SOLVER_H
#define TSUMEGO_SOLVER_H

#include "BasicType.h"
#include "WeichiGtpUtil.h"
#include "concepts.h"

class TsumegoSolver {

public:
    TsumegoSolver(MCTS& instance) : m_mcts(instance) {}

public:
    void run();

private:
    void solve(string filename);
    void setFullBoardClosedArea();
    void changeTurnColorIfNeeded(boost::property_tree::ptree& property);
    void changeBoardSize(uint boardsize);
    void changeKomi(float komi);
    void clearBoard();
    void saveResult();
    void saveJsonResult();
    void saveTTresult();
    void saveTTsummary();
    void saveTTnodes();
    void saveReconstructInfo();
    void saveSolutionTree();
    string getReusltFileName();
    string getReusltTTName();
    string getReusltTTSummaryName();
    string getReusltTTSummaryName2();
    string getTTnodesFilename();
    string getReconstructInfoFilename();

    void setProblemFileName(boost::property_tree::ptree& property);
    void setWinColor(boost::property_tree::ptree& property);
    void setKoRules(boost::property_tree::ptree& property);
    void setEndGameConditions(boost::property_tree::ptree& property);
    void setRegion(boost::property_tree::ptree& property);
    void displaySettings();

private:
    MCTS& m_mcts;
    SgfLoader m_sgfLoader;
    string m_problemName;
    fstream m_fResult;
    fstream m_fTTResult;
    fstream m_fTTsummary;
    fstream m_fTTsummary2;
    fstream m_fTTnodes;
    fstream m_fReconstructInfo;
};

#endif
