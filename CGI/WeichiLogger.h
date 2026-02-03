#ifndef WEICHILOGGER_H
#define WEICHILOGGER_H

#include "BasicType.h"
#include "WeichiBoard.h"
#include "WeichiGlobalInfo.h"
#include "WeichiUctNode.h"

class WeichiLogger {
public:
    WeichiLogger();

    static void initialize(string sDirectory);
    static void cleanUp(const WeichiBoard& board);
    static void logSgfRecord(const WeichiBoard& board, bool bWithBestSequence);
    static void logWinRate(const WeichiBoard& board, WeichiMove move);
    static void logTreeInfo(const WeichiBoard& board, WeichiMove move, bool bWriteComment, bool isImportantChild(int deep, int index, const WeichiUctNode* parent, const WeichiUctNode* child));

    static string getSystemInfo();
    static string getTreeInfo(const WeichiBoard& board, WeichiMove move, bool bWriteComment, bool isImportantChild(int deep, int index, const WeichiUctNode* parent, const WeichiUctNode* child));
    static string getTsumeGoTree(const WeichiBoard& board);    
    static string getNodeComment(NodePtr<WeichiUctNode> pParentNode, NodePtr<WeichiUctNode> pNode, bool bIsRoot = false);
    static string getBranchNodeComment(NodePtr<WeichiUctNode> pNode);
    static string getNodeColor(NodePtr<WeichiUctNode> pNode);
    static string getORWinANDLossMarks(NodePtr<WeichiUctNode> pNode);
    static string getPrunedMarks(NodePtr<WeichiUctNode> pNode);
    static string getRZoneMarks(NodePtr<WeichiUctNode> pNode);
    static string getAndWinOrLossMarks(NodePtr<WeichiUctNode> pNode);
    static string getNodeColorForGogui(NodePtr<WeichiUctNode> pNode);
    static string getWinningMarkForGogui(NodePtr<WeichiUctNode> pNode);
    static string getRZoneMarkForGogui(NodePtr<WeichiUctNode> pNode);
    static string getAndPlayerSolvedNodeMarkForGogui(NodePtr<WeichiUctNode> pNode);
    static string getDcnnUsedNet(string strNet);
    static string toMoveSgfString(WeichiMove move);
    static string toMoveGTPString(WeichiMove move);

    static bool isInterestedSolutionNode(NodePtr<WeichiUctNode> pNode);
    static bool isSaveTreeImportantChild(int deep, int index, const WeichiUctNode* parent, const WeichiUctNode* child);
    static bool isBestSequenceImportantChild(int deep, int index, const WeichiUctNode* parent, const WeichiUctNode* child);

    static string getJsonResultFile(const WeichiThreadState& state);
    static string getRootSimInfo();
    static string getTtData();
    static string getLinearTTSummary();
    static string getTTFullSummary();
    static string getTTFullSummary2();
    static string getReconstructInfo();
    static string getTTNodes();
    static void setOutputConfig(bool bOnlySolution, bool bShowBvMode);

private:
    static void logSgfRecordWithBestSequence(const string& sFullGame);
    static string logTreeInfo_r(NodePtr<WeichiUctNode> pNode, bool bWriteComment, int deep, bool isImportantChild(int deep, int index, const WeichiUctNode* parent, const WeichiUctNode* child));
    static string getSolutionTree(NodePtr<WeichiUctNode> pNode);
    static void traverseForKos(NodePtr<WeichiUctNode> pNode, bool bSolution, int depth = 0);
    static void traverseForPassWin(NodePtr<WeichiUctNode> pNode);

private:
    static string m_sDirectory;
    static string m_sBestSequence[MAX_GAME_LENGTH];
    static string m_sWinRate[MAX_GAME_LENGTH];
    static WeichiMove m_bestMove;
    static Vector<WeichiMove, MAX_GAME_LENGTH> m_moves;
    static map<int, vector<int>> m_kos_in_sol;
    static map<int, vector<int>> m_kos_in_uct;
    static vector<int> m_passwin_in_sol;
    static bool m_bOnlySolution;
};

#endif
