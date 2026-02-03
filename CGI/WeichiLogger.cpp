#include "WeichiLogger.h"
#include "UctChildIterator.h"
#include "WeichiThreadState.h"
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

string WeichiLogger::m_sDirectory;
string WeichiLogger::m_sBestSequence[MAX_GAME_LENGTH];
string WeichiLogger::m_sWinRate[MAX_GAME_LENGTH];
WeichiMove WeichiLogger::m_bestMove;
Vector<WeichiMove, MAX_GAME_LENGTH> WeichiLogger::m_moves;
map<int, vector<int>> WeichiLogger::m_kos_in_sol;
map<int, vector<int>> WeichiLogger::m_kos_in_uct;
vector<int> WeichiLogger::m_passwin_in_sol;
bool WeichiLogger::m_bOnlySolution;

WeichiLogger::WeichiLogger()
{
}

void WeichiLogger::initialize(string sDirectory)
{
    m_sDirectory = sDirectory;
    boost::filesystem::create_directories(m_sDirectory);
}

void WeichiLogger::cleanUp(const WeichiBoard& board)
{
    uint upperIndex = 0;
    for (; upperIndex < board.getMoveList().size() && upperIndex < m_moves.size(); ++upperIndex) {
        if (board.getMoveList()[upperIndex] != m_moves[upperIndex]) { break; }
    }
    ++upperIndex;
    m_moves = board.getMoveList();

    for (uint i = upperIndex + 1; i < MAX_GAME_LENGTH; ++i) {
        m_sBestSequence[i].clear();
        m_sWinRate[i].clear();
    }
}

void WeichiLogger::logSgfRecord(const WeichiBoard& board, bool bWithBestSequence)
{
    string sFileName = m_sDirectory + "/FullGame.sgf";
    fstream fSgfRecord(sFileName.c_str(), ios::out);
    fSgfRecord << board.toSgfFileString() << endl;
    fSgfRecord.close();

    if (bWithBestSequence) { logSgfRecordWithBestSequence(board.toSgfFileString()); }
}

void WeichiLogger::logWinRate(const WeichiBoard& board, WeichiMove move)
{
    cleanUp(board);

    ostringstream oss;
    oss << ToString(board.getMoveList().size() + 1) << ","
        << move.toGtpString() << ","
        << ToString(WeichiGlobalInfo::getTreeInfo().m_bestSD.getMean()) << ","
        << ToString(WeichiGlobalInfo::getTreeInfo().m_bestValueSD.getMean()) << ","
        << (WeichiGlobalInfo::getTreeInfo().m_bestSD.getMean() + WeichiGlobalInfo::getTreeInfo().m_bestValueSD.getMean()) / 2;
    m_sWinRate[board.getMoveList().size() + 1] = oss.str();

    string sFileName = m_sDirectory + "/win_rate.csv";
    fstream fWinRate(sFileName.c_str(), ios::out);
    fWinRate << "No,Move,PO,VN,AVG" << endl;

    for (uint i = 1; i < MAX_GAME_LENGTH; ++i) {
        if (i <= board.getMoveList().size() + 1) {
            if (!m_sWinRate[i].empty()) {
                fWinRate << m_sWinRate[i] << endl;
            }
        } else {
            m_sWinRate[i].clear();
        }
    }

    fWinRate.close();
}

void WeichiLogger::logTreeInfo(const WeichiBoard& board, WeichiMove move, bool bWriteComment, bool isImportantChild(int deep, int index, const WeichiUctNode* parent, const WeichiUctNode* child))
{
    cleanUp(board);
    m_bestMove = move;

    // generate best sequence string into m_sBestSequence
    m_sBestSequence[board.getMoveList().size() + 1] = getTreeInfo(board, move, bWriteComment, isImportantChild);

    // delete old best sequence sgf file
    boost::filesystem::path path(m_sDirectory);
    const boost::regex my_filter("^\\d+_.*\\.sgf");
    if (boost::filesystem::exists(path) && boost::filesystem::is_directory(path)) {
        boost::filesystem::directory_iterator end;
        for (boost::filesystem::directory_iterator it(path); it != end; ++it) {
            if (!boost::filesystem::is_regular_file(it->status())) { continue; }

            boost::smatch match;
            if (boost::regex_match(it->path().filename().string(), match, my_filter)) {
                boost::filesystem::remove(it->path());
            }
        }
    }

    // generate best sequence sgf file
    for (uint i = 1; i < MAX_GAME_LENGTH; ++i) {
        if (i <= board.getMoveList().size() + 1) {
            if (!m_sBestSequence[i].empty()) {
                string sMove = (i == board.getMoveList().size() + 1) ? move.toGtpString() : m_moves[i - 1].toGtpString();
                string sFileName = m_sDirectory + "/" + ToString(i) + "_" + sMove + ".sgf";
                fstream fLog(sFileName.c_str(), ios::out);
                fLog << m_sBestSequence[i] << endl;
                fLog.close();
            }
        } else {
            m_sBestSequence[i].clear();
        }
    }
}

string WeichiLogger::getSystemInfo()
{
    ostringstream oss;
    oss << "-----------------------------------------------\r\n";
    fstream systemInfo;
    string line;

    oss << "System Info: \r\n";

    systemInfo.open("/proc/version", ios::in);
    getline(systemInfo, line);
    oss << "\t OS: " << line << "\r\n";
    systemInfo.close();

    systemInfo.open("/proc/cpuinfo", ios::in);
    while (getline(systemInfo, line)) {
        if (line.find("model name") != std::string::npos) { break; }
    }
    oss << "\t CPU info: " << line << "\r\n";
    systemInfo.close();

    systemInfo.open("/proc/meminfo", ios::in);
    getline(systemInfo, line);
    oss << "\t Memmory info: " << line << "\r\n";
    systemInfo.close();

    return oss.str();
}

string WeichiLogger::getTreeInfo(const WeichiBoard& board, WeichiMove move, bool bWriteComment, bool isImportantChild(int deep, int index, const WeichiUctNode* parent, const WeichiUctNode* child))
{
    ostringstream oss;
    oss << board.toSgfFilePrefix();
    uint index;
    WeichiBitBoard blackBitBoard = board.getStoneBitBoard(COLOR_BLACK);
    if (blackBitBoard.bitCount() > 0) {
        oss << "AB";
        while ((index = blackBitBoard.bitScanForward()) != -1) {
            oss << "[" << WeichiMove(COLOR_BLACK, index).toSgfString(false) << "]";
        }
    }
    WeichiBitBoard whiteBitBoard = board.getStoneBitBoard(COLOR_WHITE);
    if (whiteBitBoard.bitCount() > 0) {
        oss << "AW";
        while ((index = whiteBitBoard.bitScanForward()) != -1) {
            oss << "[" << WeichiMove(COLOR_WHITE, index).toSgfString(false) << "]";
        }
    }
    deque<WeichiUctNode*> child;
    NodePtr<WeichiUctNode> pNode = WeichiGlobalInfo::getTreeInfo().m_pRoot;
    if (pNode->hasChildren()) {
        for (UctChildIterator<WeichiUctNode> it(pNode); it; ++it) {
            child.push_back(&(*it));
        }
        sort(child.begin(), child.end(),
             [](const WeichiUctNode* lhs, const WeichiUctNode* rhs) {
                 return lhs->getUctData().getCount() > rhs->getUctData().getCount();
             });

        oss << "LB";
        for (int i = 0; i < 5; ++i) {
            if (child[i]->getMove().isPass()) { continue; }
            if (!isBestSequenceImportantChild(1, i, &(*pNode), child[i])) { continue; }
            oss << "[" << child[i]->getMove().toSgfString(false) << ":" << round((child[i]->getUctData().getMean() + 1) / 2 * 100) << "]";
        }
    }

    oss << logTreeInfo_r(WeichiGlobalInfo::getTreeInfo().m_pRoot, bWriteComment, 1, isImportantChild);
    oss << ")";

    return oss.str();
}

string WeichiLogger::getTsumeGoTree(const WeichiBoard& board)
{
    ostringstream oss;
    oss << "(;";

    // preset
    string sAB, sAW;
    const Vector<WeichiMove, MAX_NUM_GRIDS>& vPresets = board.getPresetList();
    for (int iPreset = 0; iPreset < vPresets.size(); iPreset++) {
        if (vPresets[iPreset].getColor() == COLOR_BLACK) {
            sAB += "[";
            sAB += toMoveSgfString(vPresets[iPreset]);
            sAB += "]";
        } else if (vPresets[iPreset].getColor() == COLOR_WHITE) {
            sAW += "[";
            sAW += toMoveSgfString(vPresets[iPreset]);
            sAW += "]";
        }
    }
    if (sAB != "") oss << "AB" << sAB;
    if (sAW != "") oss << "AW" << sAW;

    oss << "GM[1]SZ[" << WeichiConfigure::BoardSize << "]";

    // moves
    const Vector<WeichiMove, MAX_GAME_LENGTH>& vMoves = board.getMoveList();
    // hack for white first
    bool bOnlyPassMoveBefore = (vMoves.size() == 1) && vMoves[0].getPosition() == (WeichiMove::PASS_POSITION);
    if (bOnlyPassMoveBefore) { const_cast<Vector<WeichiMove, MAX_GAME_LENGTH>&>(vMoves).clear(); }
    for (int iMove = 0; iMove < vMoves.size(); iMove++) {
        oss << ";";
        if (vMoves[iMove].getColor() == COLOR_BLACK) {
            oss << "B";
        } else {
            oss << "W";
        }
        oss << "[" << toMoveSgfString(vMoves[iMove]) << "]";
    }

    NodePtr<WeichiUctNode> pRoot = WeichiGlobalInfo::getTreeInfo().m_pRoot;
    m_kos_in_sol.clear();
    m_kos_in_uct.clear();
    m_passwin_in_sol.clear();
    traverseForKos(pRoot, true, board.getMoveList().size());
    traverseForKos(pRoot, false, board.getMoveList().size());
    traverseForPassWin(pRoot);
    if (vMoves.size() != 0 && WeichiConfigure::use_editor_tree_format) { oss << getNodeComment(pRoot, pRoot, true); }
    if (vMoves.size() != 0 && WeichiConfigure::use_editor_tree_format) { oss << getNodeColor(pRoot); }
    if (!WeichiConfigure::use_editor_tree_format) { oss << getNodeComment(pRoot, pRoot, true); }
    oss << getSolutionTree(pRoot);
    oss << ")";
    if (vMoves.size() == 0 && WeichiConfigure::use_editor_tree_format) { oss << getNodeComment(pRoot, pRoot, true); }
    if (vMoves.size() == 0 && WeichiConfigure::use_editor_tree_format) { oss << getNodeColor(pRoot); }

    return oss.str();
}

string WeichiLogger::getNodeComment(NodePtr<WeichiUctNode> pParentNode, NodePtr<WeichiUctNode> pNode, bool bIsRoot)
{
    Color liveColor = WeichiGlobalInfo::getTreeInfo().m_winColor;

    ostringstream oss;
    oss << "C[";
    if (pNode->getUctNodeStatus() != UCT_STATUS_UNKNOWN && !pNode->hasChildren()) { oss << "Terminal"
                                                                                        << "\r\n"; }
    oss << "ID: -" << pNode.getIndex() << "-\r\n";
    oss << "Status: " << getWeichiUctNodeStatus(pNode->getUctNodeStatus()) << "\r\n";
    oss << "WR: " << pNode->getUctDataWithoutVirtualLoss().getMean() << ", Count: " << pNode->getUctDataWithoutVirtualLoss().getCount();
    if (pNode->isCNNNode()) {
        oss << ", CNN node\r\n";
    } else {
        oss << ", unCNN node\r\n";
    }
    oss << "P: " << pNode->getProb() << ", VN: ";
    if (pNode->isCNNNode()) {
        oss << pNode->getCNNVNValue() << "\r\n";
    } else {
        oss << "Invalid\r\n";
    }
    oss << "Virtual Loss: " << pNode->getVirtualLossCount() << "\r\n";
    oss << "Use potential RZone: " << pNode->hasPotentialRZone() << ", CkWin: " << pNode->getCheckImmedaiteWin() << "\r\n";
    if (pNode->getWinByKoPosition() != -1) { oss << "WinByKo: " << toMoveGTPString(WeichiMove(pNode->getWinByKoPosition())) << "\r\n"; }
    if (pNode->getEqualLoss() != -1) { oss << "EqualLoss: " << toMoveGTPString(WeichiMove(pNode->getEqualLoss())) << "\r\n"; }

    oss << "TTmatched: " << pNode->getTTmatchedNodeID() << "\r\n";
    oss << "SolDepth: " << pNode->getSolDepth() << "\r\n";

    if (WeichiConfigure::check_potential_ghi_for_rzone) {
        if (pNode->getSameRZpatternIndex().second == pNode.getIndex()) {
            oss << "SameRZpattern Index: " << pNode->getSameRZpatternIndex().first << "\r\n";
        }
    }

    if (pNode->getSkipTTByLoop()) { oss << "Skip TT by Loop\r\n"; }
    if (pNode->getLoopCheck().size() != 0) {
        oss << "Check Loop: ";
        vector<NodePtr<WeichiUctNode>>& loopCheck = pNode->getLoopCheck();
        for (int i = 0; i < loopCheck.size(); ++i) {
            oss << loopCheck[i].getIndex() << " ";
        }
        oss << "\r\n";
    }

    if (pNode->getSSKID() != -1) {
        oss << "SSK: ";
        WeichiBitBoard bmSSK = WeichiGlobalInfo::getRZoneDataTable().getRZone(pNode->getSSKID());
        int pos = 0;
        while ((pos = bmSSK.bitScanForward()) != -1) {
            WeichiMove move(COLOR_NONE, pos);
            oss << toMoveGTPString(move) << " ";
        }
        oss << endl;
    }

    if (bIsRoot) {
        oss << "RootSimInfo: ";
        vector<tuple<int, double, short>>& rootSimInfo = WeichiGlobalInfo::getRootSimInfo();
        int previousPos = -1;
        int previousCount = 0;
        bool bFirst = true;
        for (int i = 0; i < rootSimInfo.size(); ++i) {
            tuple<int, double, short>& info = rootSimInfo[i];
            int count = std::get<0>(info);
            short movePos = std::get<2>(info);
            if (movePos != previousPos) {
                if (bFirst) {
                    bFirst = false;
                } else {
                    oss << ",";
                }
                oss << "(" << count << "," << toMoveGTPString(WeichiMove(movePos)) << ")";
                previousPos = movePos;
            }
        }
        oss << "\r\n";
        oss << "-----------------------------------------------\r\n";
        time_t rawtime;
        struct tm* timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        oss << "Date: " << asctime(timeinfo) << "\r\n";
        if (pNode->isProved()) { oss << "Problem Solved.\r\n"; }
        oss << "Think Time: " << WeichiGlobalInfo::getTreeInfo().m_dThinkTime << " seconds, "
            << "total " << WeichiGlobalInfo::getTreeInfo().m_nSimulation << " simulations."
            << "\r\n";
        oss << "Ko information, "
            << "kos in solution tree: " << m_kos_in_sol.size() << ", kos in uct tree: " << m_kos_in_uct.size() << "\r\n";
        oss << "Pass Win: " << m_passwin_in_sol.size() << "\r\n";
        oss << "Solution Depth: " << pNode->getSolDepth() << "\r\n";
        oss << "Configurations in this experiment:\r\n";
        oss << "\t DCNN_NET: " << getDcnnUsedNet(WeichiConfigure::dcnn_net) << "\r\n";
        oss << "\t Komi: " << WeichiConfigure::komi << "\r\n";
        oss << "\t Batch Size: " << WeichiConfigure::dcnn_default_batch_size << "\r\n";
        oss << "\t PUCT bias: " << WeichiConfigure::mcts_puct_bias << "\r\n";
        oss << "\t Problem Rotation: " << WeichiConfigure::problem_rotation << "\r\n";
        oss << "\t Use FTL: " << WeichiConfigure::dcnn_use_ftl << "\r\n";
        oss << "\t Use FTL remaining move num: " << WeichiConfigure::dcnn_ftl_remaining_move_num << "\r\n";
        oss << "\t MCTS use solver: " << WeichiConfigure::mcts_use_solver << "\r\n";
        oss << "\t MCTS use disproof: " << WeichiConfigure::mcts_use_disproof << "\r\n";
        oss << "\t Win Color: " << toChar(liveColor) << "\r\n";
        oss << "\t Black SearchGoal: " << WeichiGlobalInfo::getEndGameCondition().getSearchGoalString(COLOR_BLACK) << "\r\n";
        oss << "\t White SearchGoal: " << WeichiGlobalInfo::getEndGameCondition().getSearchGoalString(COLOR_WHITE) << "\r\n";
        oss << "\t Black CrucialStone: " << WeichiGlobalInfo::getEndGameCondition().getCruicalStoneEditorString(COLOR_BLACK) << "\r\n";
        oss << "\t White CrucialStone: " << WeichiGlobalInfo::getEndGameCondition().getCruicalStoneEditorString(COLOR_WHITE) << "\r\n";
        oss << "\t Use RZone: " << WeichiConfigure::use_rzone << "\r\n";
        oss << "\t Use Immediate Win: " << WeichiConfigure::use_immediate_win << "\r\n";
        oss << "\t Use Early Life: " << WeichiConfigure::use_early_life << "\r\n";
        oss << "\t Use Potential RZone: " << WeichiConfigure::use_potential_rzone << "\r\n";
        oss << "\t Use MCTS BV: " << WeichiConfigure::mcts_use_bv << "\r\n";
        oss << "\t Use MCTS BV Zone: " << WeichiConfigure::mcts_use_bv_zone << "\r\n";
        oss << "\t Use Board TT: " << WeichiConfigure::use_board_transposition_table << "\r\n";
        oss << "\t Use RZone TT: " << WeichiConfigure::use_rzone_transposition_table << "\r\n";
        oss << "\t Use Linear TT: " << WeichiConfigure::use_rzone_linear_transposition_table << "\r\n";
        oss << "\t Enable Zone TT ID diff: " << WeichiConfigure::use_rzone_tt_skip_on_id_diff << "\r\n";
        oss << "\t Enable Zone TT Skip Larger Pattern: " << WeichiConfigure::use_rzone_tt_skip_larger_pattern << "\r\n";
        oss << "\t Potential RZone Count limit: " << WeichiConfigure::potential_rzone_count_limit << "\r\n";
        oss << "\t Winning VN threshold: " << WeichiConfigure::win_vn_threshold << "\r\n";
        oss << "\t BV threhosld: " << WeichiConfigure::bv_threshold << "\r\n";
        oss << "\t Use dead shape detection: " << WeichiConfigure::use_dead_shape_detection << "\r\n";
        oss << "\t Use territory detection: " << WeichiConfigure::use_independent_territory_detection << "\r\n";
        oss << "\t Use opponent border rzone: " << WeichiConfigure::use_opponent_border_rzone << "\r\n";
        oss << "\t Black Ignore Ko: " << WeichiConfigure::black_ignore_ko << "\r\n";
        oss << "\t White Ignore Ko: " << WeichiConfigure::white_ignore_ko << "\r\n";
        oss << "\t CNN policy output powert softmax: " << WeichiConfigure::cnn_policy_output_power_softmax << "\r\n";
        oss << "\t Use VN turn opposite: " << WeichiConfigure::vn_turn_opposite << "\r\n";
        oss << "\t Use opp1 stone suicide like pass: " << WeichiConfigure::use_opp1stone_suicide_likepass << "\r\n";
        oss << "\t Use MCTS sequential search: " << WeichiConfigure::do_mcts_sequential_search << "\r\n";
        oss << getSystemInfo();

        oss << "-------Ko Info--------"
            << "\r\n";
        oss << "Sol Kos:"
            << "\r\n";
        for (map<int, vector<int>>::iterator it = m_kos_in_sol.begin(); it != m_kos_in_sol.end(); ++it) {
            oss << "-" << it->first << ": " << it->second.size() << "\r\n";
            vector<int>& v = it->second;
            for (vector<int>::iterator it2 = v.begin(); it2 != v.end(); ++it2) {
                oss << *it2 << " ";
            }
            oss << "\r\n";
        }

        oss << "Uct Kos:"
            << "\r\n";
        for (map<int, vector<int>>::iterator it = m_kos_in_uct.begin(); it != m_kos_in_uct.end(); ++it) {
            oss << "-" << it->first << ": " << it->second.size() << "\r\n";
            vector<int>& v = it->second;
            for (vector<int>::iterator it2 = v.begin(); it2 != v.end(); ++it2) {
                oss << *it2 << " ";
            }
            oss << "\r\n";
        }
        oss << "-------Ko Info End--------"
            << "\r\n";
    }
    oss << "]";

    return oss.str();
}

string WeichiLogger::getBranchNodeComment(NodePtr<WeichiUctNode> pNode)
{
    Color liveColor = WeichiGlobalInfo::getTreeInfo().m_winColor;

    ostringstream oss;
    oss << "C[";
    oss << "Status: " << getWeichiUctNodeStatus(pNode->getUctNodeStatus()) << "\r\n";
    oss << "WR: " << pNode->getUctDataWithoutVirtualLoss().getMean() << ", Count: " << fixed << pNode->getUctDataWithoutVirtualLoss().getCount();
    if (pNode->isCNNNode()) { oss << ", CNN node"; }
    oss << "\r\n";
    if (pNode->isCNNNode()) {
        oss << "SL: " << pNode->getProb() << ", VN: " << pNode->getCNNVNValue() << "\r\n";
    } else {
        oss << "SL: " << pNode->getProb() << ", VN: Invalid"
            << "\r\n";
    }
    oss << "]";

    return oss.str();
}

string WeichiLogger::getNodeColor(NodePtr<WeichiUctNode> pNode)
{
    ostringstream oss;

    if (pNode->hasChildren()) {
        oss << getORWinANDLossMarks(pNode);
        oss << getAndWinOrLossMarks(pNode);
        oss << getPrunedMarks(pNode); // comment to get intersection
    }

    oss << getRZoneMarks(pNode);

    return oss.str();
}

string WeichiLogger::getORWinANDLossMarks(NodePtr<WeichiUctNode> pNode)
{
    if (pNode->isUnknownStatus()) { return ""; }

    Color survivorColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
    Color killColor = AgainstColor(survivorColor);
    if (pNode->getColor() == killColor && pNode->getUctNodeStatus() != UCT_STATUS_WIN) { return ""; }
    if (pNode->getColor() == survivorColor && pNode->getUctNodeStatus() != UCT_STATUS_LOSS) { return ""; }

    WeichiBitBoard bmInterested;
    for (UctChildIterator<WeichiUctNode> it(pNode); it; ++it) {
        if (it->isProved() && it->getUctNodeStatus() == pNode->getUctNodeStatus()) {
            bmInterested.SetBitOn(it->getPosition());
        }
    }

    ostringstream oss;
    if (!bmInterested.empty()) {
        int pos = 0;
        if (pNode->getColor() == killColor) {
            oss << "BG[0xFF0000]";
        } else if (pNode->getColor() == survivorColor) {
            oss << "BG[0x0080FF]";
        }
        oss << "[";
        bool bFirst = true;
        while ((pos = bmInterested.bitScanForward()) != -1) {
            if (bFirst) {
                bFirst = false;
            } else {
                oss << ",";
            }
            oss << WeichiMove(pos).toEditorCoorString();
        }
        oss << "]";
    }

    return oss.str();
}

string WeichiLogger::getPrunedMarks(NodePtr<WeichiUctNode> pParent)
{
    Color survivorColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
    Color killColor = AgainstColor(survivorColor);

    WeichiBitBoard bmPruned;
    for (UctChildIterator<WeichiUctNode> it(pParent); it; ++it) {
        if (it->isRZonePruned()) {
            bmPruned.SetBitOn(it->getPosition());
        }
    }

    WeichiBitBoard bmRZone;
    if (pParent->getUctNodeStatus() == UCT_STATUS_WIN) { bmRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(pParent->getProofRZoneID()); } // killer's turn
    else if (pParent->getUctNodeStatus() == UCT_STATUS_LOSS) {
        bmRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(pParent->getDisproofRZoneID());
    } // survivor's turn

    // �Y�w�ҡA�n���RZone�A�N�n�\��pruned���C�ⳡ��
    if (pParent->isProved()) {
        bmPruned -= bmRZone;
    }

    ostringstream oss;
    if (!bmPruned.empty()) {
        if (pParent->getColor() == survivorColor) { oss << "BG[0x4E4E4E]"; } // grey
        else if (pParent->getColor() == killColor) {
            oss << "BG[0xF0FFFF]";
        } // skyblue
        bool bFirst = true;
        int pos = 0;
        oss << "[";
        while ((pos = bmPruned.bitScanForward()) != -1) {
            if (bFirst) {
                bFirst = false;
            } else {
                oss << ",";
            }
            oss << WeichiMove(pos).toEditorCoorString();
        }
        oss << "]";
    }

    return oss.str();
}

string WeichiLogger::getRZoneMarks(NodePtr<WeichiUctNode> pNode)
{
    if (pNode->isUnknownStatus()) { return ""; }

    WeichiBitBoard bmRZone;
    if (pNode->getUctNodeStatus() == UCT_STATUS_WIN) {
        bmRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(pNode->getProofRZoneID());
    } else if (pNode->getUctNodeStatus() == UCT_STATUS_LOSS) {
        bmRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(pNode->getDisproofRZoneID());
    }

    if (bmRZone.empty()) { return ""; }

    ostringstream oss;
    Color interestedColor;
    if (pNode->getUctNodeStatus() == UCT_STATUS_WIN) {
        interestedColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
    } else if (pNode->getUctNodeStatus() == UCT_STATUS_LOSS) {
        interestedColor = AgainstColor(WeichiGlobalInfo::getTreeInfo().m_winColor);
    }

    WeichiBitBoard bmWinPos;
    for (UctChildIterator<WeichiUctNode> it(pNode); it; ++it) {
        if (it->getColor() == interestedColor && it->isProved() && it->getUctNodeStatus() == pNode->getUctNodeStatus()) {
            bmWinPos.SetBitOn(it->getPosition());
        }
    }

    if (pNode->getUctNodeStatus() == UCT_STATUS_WIN) { oss << "BG[0x00FF00]"; } // green
    else if (pNode->getUctNodeStatus() == UCT_STATUS_LOSS) {
        oss << "BG[0xFFFF80]";
    } // light-yellow

    bool bFirst = true;
    int pos = 0;
    oss << "[";
    while ((pos = bmRZone.bitScanForward()) != -1) {
        if (bmWinPos.BitIsOn(pos)) { continue; }

        if (bFirst) {
            bFirst = false;
        } else {
            oss << ",";
        }
        oss << WeichiMove(pos).toEditorCoorString();
    }
    oss << "]";

    return oss.str();
}

string WeichiLogger::getAndWinOrLossMarks(NodePtr<WeichiUctNode> pParent)
{
    Color survivorColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
    Color killColor = AgainstColor(survivorColor);

    WeichiBitBoard bmRZone;
    if (pParent->getColor() == survivorColor) {
        bmRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(pParent->getProofRZoneID());
    } else if (pParent->getColor() == killColor) {
        bmRZone = WeichiGlobalInfo::getRZoneDataTable().getRZone(pParent->getDisproofRZoneID());
    }

    WeichiBitBoard bmLoss;
    for (UctChildIterator<WeichiUctNode> it(pParent); it; ++it) {
        // Only display the proved status that is not pruned.
        if (it->isProved() && it->getUctNodeStatus() != UCT_STATUS_RZONE_PRUNED &&
            (!bmRZone.BitIsOn(it->getPosition()) || (bmRZone.BitIsOn(it->getPosition()) && pParent->isUnknownStatus()))) {
            bmLoss.SetBitOn(it->getPosition());
        }
    }

    // TODO: refactor the following codes to avoid duplication
    if (pParent->getUctNodeStatus() == UCT_STATUS_WIN && pParent->getColor() == killColor) {
        for (UctChildIterator<WeichiUctNode> it(pParent); it; ++it) {
            if (it->getUctNodeStatus() == UCT_STATUS_WIN)
                bmLoss.SetBitOff(it->getPosition());
        }
    } else if (pParent->getUctNodeStatus() == UCT_STATUS_LOSS && pParent->getColor() == survivorColor) {
        for (UctChildIterator<WeichiUctNode> it(pParent); it; ++it) {
            if (it->getUctNodeStatus() == UCT_STATUS_LOSS)
                bmLoss.SetBitOff(it->getPosition());
        }
    }

    ostringstream oss;
    if (!bmLoss.empty()) {
        if (pParent->getColor() == survivorColor) {
            oss << "BG[0x96FFFF]";
        } else if (pParent->getColor() == killColor) {
            oss << "BG[0xADADFF]";
        }
        oss << "[";
        bool bFirst = true;
        int pos = 0;
        while ((pos = bmLoss.bitScanForward()) != -1) {
            if (bFirst) {
                bFirst = false;
            } else {
                oss << ",";
            }
            oss << WeichiMove(pos).toEditorCoorString();
        }
        oss << "]";
    }

    return oss.str();
}

string WeichiLogger::getNodeColorForGogui(NodePtr<WeichiUctNode> pNode)
{
    ostringstream oss;
    oss << "RZ";
    oss << "[" << getWinningMarkForGogui(pNode) << "]";
    oss << "[" << getRZoneMarkForGogui(pNode) << "]";
    oss << "[" << getAndPlayerSolvedNodeMarkForGogui(pNode) << "]";

    return oss.str();
}

string WeichiLogger::getWinningMarkForGogui(NodePtr<WeichiUctNode> pNode)
{
    Color survivorColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
    Color killColor = AgainstColor(survivorColor);
    WeichiBitBoard bmInterested;
    if (pNode->getColor() == killColor) {
        for (UctChildIterator<WeichiUctNode> it(pNode); it; ++it) {
            if (it->isProved() && it->getUctNodeStatus() == UCT_STATUS_WIN) {
                bmInterested.SetBitOn(it->getPosition());
                break;
            }
        }
    }

    std::string finalHex(91, '0');
    int bitValue = 0;
    int bitCount = 0;
    int hexIndex = 90;                  // 從字串的最尾端（最低位）開始填
    for (int r = 0; r <= 18; ++r) {     // 從左下角開始：Row 0, 1, 2...
        for (int c = 0; c <= 18; ++c) { // Col 0, 1, 2...
            int pos = r * 20 + c;

            // 收集位元：第一個讀到的 (index 0) 權重最小 (1<<0)
            if (bmInterested.BitIsOn(pos)) {
                bitValue |= (1 << bitCount);
            }
            bitCount++;

            // 每湊滿 4 位，或是到了最後一個點 (第 361 位)
            if (bitCount == 4 || (r == 18 && c == 18)) {
                char hexChar;
                if (bitValue < 10)
                    hexChar = (char)('0' + bitValue);
                else
                    hexChar = (char)('a' + (bitValue - 10));

                // 將字元填入對應的位置，然後 hexIndex 往左移動
                finalHex[hexIndex] = hexChar;
                hexIndex--;

                // 重置計數
                bitValue = 0;
                bitCount = 0;
            }
        }
    }

    // 在 return finalHex; 之前加入這幾行
    size_t firstNotZero = finalHex.find_first_not_of('0');

    if (string::npos == firstNotZero) {
        return "0"; // 如果全都是 0，就回傳一個 "0"
    }
    return finalHex.substr(firstNotZero);
}

string WeichiLogger::getRZoneMarkForGogui(NodePtr<WeichiUctNode> pNode)
{
    WeichiBitBoard bmInterested = pNode->getUctNodeStatus() == UCT_STATUS_WIN ? WeichiGlobalInfo::getRZoneDataTable().getRZone(pNode->getProofRZoneID()) : WeichiBitBoard();

    std::string finalHex(91, '0');
    int bitValue = 0;
    int bitCount = 0;
    int hexIndex = 90;                  // 從字串的最尾端（最低位）開始填
    for (int r = 0; r <= 18; ++r) {     // 從左下角開始：Row 0, 1, 2...
        for (int c = 0; c <= 18; ++c) { // Col 0, 1, 2...
            int pos = r * 20 + c;

            // 收集位元：第一個讀到的 (index 0) 權重最小 (1<<0)
            if (bmInterested.BitIsOn(pos)) {
                bitValue |= (1 << bitCount);
            }
            bitCount++;

            // 每湊滿 4 位，或是到了最後一個點 (第 361 位)
            if (bitCount == 4 || (r == 18 && c == 18)) {
                char hexChar;
                if (bitValue < 10)
                    hexChar = (char)('0' + bitValue);
                else
                    hexChar = (char)('a' + (bitValue - 10));

                // 將字元填入對應的位置，然後 hexIndex 往左移動
                finalHex[hexIndex] = hexChar;
                hexIndex--;

                // 重置計數
                bitValue = 0;
                bitCount = 0;
            }
        }
    }

    // 在 return finalHex; 之前加入這幾行
    size_t firstNotZero = finalHex.find_first_not_of('0');

    if (string::npos == firstNotZero) {
        return "0"; // 如果全都是 0，就回傳一個 "0"
    }

    return finalHex.substr(firstNotZero);
}

string WeichiLogger::getAndPlayerSolvedNodeMarkForGogui(NodePtr<WeichiUctNode> pNode)
{
    if (!pNode->isUnknownStatus()) { return "0"; }

    Color survivorColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
    WeichiBitBoard bmInterested;
    if (pNode->getColor() == survivorColor) {
        for (UctChildIterator<WeichiUctNode> it(pNode); it; ++it) {
            if (it->isProved() && it->getUctNodeStatus() == UCT_STATUS_WIN) {
                bmInterested.SetBitOn(it->getPosition());
            }
        }
    }

    std::string finalHex(91, '0');
    int bitValue = 0;
    int bitCount = 0;
    int hexIndex = 90;                  // 從字串的最尾端（最低位）開始填
    for (int r = 0; r <= 18; ++r) {     // 從左下角開始：Row 0, 1, 2...
        for (int c = 0; c <= 18; ++c) { // Col 0, 1, 2...
            int pos = r * 20 + c;

            // 收集位元：第一個讀到的 (index 0) 權重最小 (1<<0)
            if (bmInterested.BitIsOn(pos)) {
                bitValue |= (1 << bitCount);
            }
            bitCount++;

            // 每湊滿 4 位，或是到了最後一個點 (第 361 位)
            if (bitCount == 4 || (r == 18 && c == 18)) {
                char hexChar;
                if (bitValue < 10)
                    hexChar = (char)('0' + bitValue);
                else
                    hexChar = (char)('a' + (bitValue - 10));

                // 將字元填入對應的位置，然後 hexIndex 往左移動
                finalHex[hexIndex] = hexChar;
                hexIndex--;

                // 重置計數
                bitValue = 0;
                bitCount = 0;
            }
        }
    }

    // 在 return finalHex; 之前加入這幾行
    size_t firstNotZero = finalHex.find_first_not_of('0');

    if (string::npos == firstNotZero) {
        return "0"; // 如果全都是 0，就回傳一個 "0"
    }

    return finalHex.substr(firstNotZero);
}

string WeichiLogger::getDcnnUsedNet(string strNet)
{
    ostringstream oss;
    istringstream iss(strNet);
    string net_line;
    while (getline(iss, net_line, ':')) {
        // cout << "[" << netline << "]" << endl;
        if (net_line[0] != '#' && net_line[net_line.length() - 1] != '|') {
            oss << net_line;
            break;
        }
    }

    return oss.str();
}

string WeichiLogger::toMoveSgfString(WeichiMove move)
{
    if (WeichiConfigure::use_editor_tree_format) {
        return move.toEditorCoorString();
    } else
        return move.toSgfString(false);
}

string WeichiLogger::toMoveGTPString(WeichiMove move)
{
    if (WeichiConfigure::use_editor_tree_format) {
        return move.toEditorGtpString();
    } else
        return move.toGtpString();
}

string WeichiLogger::getSolutionTree(NodePtr<WeichiUctNode> node)
{
    if (!node->hasChildren()) { return ""; }

    // sort the winning move in the front
    deque<NodePtr<WeichiUctNode>> children;
    for (UctChildIterator<WeichiUctNode> it(node); it; ++it) { children.push_back(it); }

    sort(
        children.begin(), children.end(),
        [](const NodePtr<WeichiUctNode> lhs, const NodePtr<WeichiUctNode> rhs) {
            Color liveColor = Color(WeichiConfigure::win_color);
            Color killColor = AgainstColor(liveColor);
            return (lhs->getUctData().getCount() > rhs->getUctData().getCount());
        });

    Color ownColor = Color(WeichiConfigure::win_color);
    Color oppColor = AgainstColor(ownColor);

    ostringstream oss;
    for (int i = 0; i < children.size(); ++i) {
        // Only skip un-CNN node, leaf nodes
        if (!children[i]->isCNNNode()) {
            if (children[i]->isRZonePruned()) {
                continue;
            } else if (children[i]->isUnknownStatus()) {
                continue;
            }
        }
        if (m_bOnlySolution && !isInterestedSolutionNode(children[i])) { continue; }
        if (node->getNumChildren() > 1) { oss << "("; }
        oss << ";";
        if (children[i]->getColor() == COLOR_BLACK) {
            oss << "B";
        } else {
            oss << "W";
        }
        oss << "[" << toMoveSgfString(children[i]->getMove()) << "]";
        oss << getNodeComment(node, children[i]);
        if (WeichiConfigure::use_editor_tree_format) {
            oss << getNodeColor(children[i]);
        } else {
            oss << getNodeColorForGogui(children[i]);
        }
        oss << getSolutionTree(children[i]);
        if (node->getNumChildren() > 1) { oss << ")"; }
        if (WeichiConfigure::use_editor_tree_format) { oss << getBranchNodeComment(children[i]); }
    }

    return oss.str();
}

void WeichiLogger::traverseForKos(NodePtr<WeichiUctNode> pNode, bool bSolution, int depth)
{
    if (bSolution && pNode->isUnknownStatus()) { return; }

    if (pNode->getColor() == COLOR_WHITE && pNode->getWinByKoPosition() != -1) {
        if (bSolution) {
            m_kos_in_sol[depth].push_back(pNode.getIndex());
        } else {
            m_kos_in_uct[depth].push_back(pNode.getIndex());
        }
    }

    for (UctChildIterator<WeichiUctNode> it(pNode); it; ++it) {
        traverseForKos(it, bSolution, depth + 1);
    }

    return;
}

void WeichiLogger::traverseForPassWin(NodePtr<WeichiUctNode> pNode)
{
    if (pNode->isUnknownStatus()) { return; }
    if (pNode->getColor() == WeichiGlobalInfo::getTreeInfo().m_winColor) {
        if (pNode->getMove() == WeichiMove::PASS_POSITION) { m_passwin_in_sol.push_back(pNode.getIndex()); }
    }

    for (UctChildIterator<WeichiUctNode> it(pNode); it; ++it) {
        traverseForPassWin(it);
    }

    return;
}

bool WeichiLogger::isInterestedSolutionNode(NodePtr<WeichiUctNode> pNode)
{
    if (!pNode->isWinLossBySearch()) { return false; }

    return true;
}

bool WeichiLogger::isSaveTreeImportantChild(int deep, int index, const WeichiUctNode* parent, const WeichiUctNode* child)
{
    if (child->getUctData().getCount() < parent->getUctData().getCount() / 100) { return false; }
    if (child->getUctData().getCount() < WeichiGlobalInfo::getTreeInfo().m_pRoot->getUctData().getCount() / 1000) { return false; }

    return true;
}

bool WeichiLogger::isBestSequenceImportantChild(int deep, int index, const WeichiUctNode* parent, const WeichiUctNode* child)
{
    if (child->getUctData().getCount() < 100) { return false; }
    if (deep == 1 && index < 5) { return true; }
    if (deep == 2 && index < 5 && parent->getMove() == m_bestMove) { return true; }
    if (index == 0 && child->getUctData().getCount() > parent->getUctData().getCount() / 100) { return true; }

    return false;
}

string WeichiLogger::getJsonResultFile(const WeichiThreadState& state)
{
    ostringstream oss;
    double dMaxCount = 0;
    NodePtr<WeichiUctNode> bestNode = NodePtr<WeichiUctNode>::NULL_PTR;
    WeichiUctNodeStatus rootStatus = WeichiGlobalInfo::getTreeInfo().m_pRoot->getUctNodeStatus();

    if (rootStatus == UCT_STATUS_WIN) {
        Color winColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
        Color turnColor = AgainstColor(WeichiGlobalInfo::getTreeInfo().m_pRoot->getColor());
        if (turnColor == winColor) {
            for (UctChildIterator<WeichiUctNode> it(WeichiGlobalInfo::getTreeInfo().m_pRoot); it; ++it) {
                if (it->getUctNodeStatus() == UCT_STATUS_WIN) {
                    bestNode = it;
                    dMaxCount = it->getUctData().getCount();
                    break;
                }
            }
        } else if (turnColor == AgainstColor(winColor)) {
            // if opponent to play, output pass node
            for (UctChildIterator<WeichiUctNode> it(WeichiGlobalInfo::getTreeInfo().m_pRoot); it; ++it) {
                if (it->getPosition() == WeichiMove::PASS_POSITION) {
                    bestNode = it;
                    dMaxCount = it->getUctData().getCount();
                    break;
                }
            }
        }
    } else if (rootStatus == UCT_STATUS_LOSS) {
        Color winColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
        Color turnColor = AgainstColor(WeichiGlobalInfo::getTreeInfo().m_pRoot->getColor());
        if (turnColor == winColor) {
            // if opponent to play, output pass node
            for (UctChildIterator<WeichiUctNode> it(WeichiGlobalInfo::getTreeInfo().m_pRoot); it; ++it) {
                if (it->getPosition() == WeichiMove::PASS_POSITION) {
                    bestNode = it;
                    dMaxCount = it->getUctData().getCount();
                    break;
                }
            }
        } else if (turnColor == AgainstColor(winColor)) {
            for (UctChildIterator<WeichiUctNode> it(WeichiGlobalInfo::getTreeInfo().m_pRoot); it; ++it) {
                if (it->getUctNodeStatus() == UCT_STATUS_LOSS) {
                    bestNode = it;
                    dMaxCount = it->getUctData().getCount();
                    break;
                }
            }
        }
    } else {
        for (UctChildIterator<WeichiUctNode> it(WeichiGlobalInfo::getTreeInfo().m_pRoot); it; ++it) {
            if (it->getUctData().getCount() > dMaxCount) {
                bestNode = it;
                dMaxCount = it->getUctData().getCount();
            }
        }
    }

    WeichiMove bestMove = (bestNode != NodePtr<WeichiUctNode>::NULL_PTR) ? bestNode->getMove() : WeichiMove();
    double bestMoveWinRate = (bestNode != NodePtr<WeichiUctNode>::NULL_PTR) ? bestNode->getUctData().getMean() : 0.0;
    double bestMoveCount = (bestNode != NodePtr<WeichiUctNode>::NULL_PTR) ? bestNode->getUctData().getCount() : 0.0;
    double bestMovePolicy = (bestNode != NodePtr<WeichiUctNode>::NULL_PTR) ? bestNode->getProb() : 0.0;
    double bestMoveVN = (bestNode != NodePtr<WeichiUctNode>::NULL_PTR) ? bestNode->getCNNVNValue() : 0.0;

    oss << "{" << endl;
    oss << "\"ProblemSgfFilename\": \"" << WeichiGlobalInfo::getTreeInfo().m_problemSgfFilename << "\"," << endl;
    oss << "\"ProblemSgfUctTreeFilename\": \"" << WeichiGlobalInfo::getTreeInfo().m_problemSgfUctTreeFilename << "\"," << endl;
    oss << "\"NumSimulations\": " << WeichiGlobalInfo::getTreeInfo().m_nSimulation << "," << endl;
    oss << "\"Time\": " << WeichiGlobalInfo::getTreeInfo().m_dThinkTime << "," << endl;
    oss << "\"BlackCrucialStone\": \"" << WeichiGlobalInfo::getEndGameCondition().getCruicalStoneSgfString(COLOR_BLACK) << "\"," << endl;
    oss << "\"WhiteCrucialStone\": \"" << WeichiGlobalInfo::getEndGameCondition().getCruicalStoneSgfString(COLOR_WHITE) << "\"," << endl;
    oss << "\"BestMove\": \"" << bestMove.toGtpString() << "\"," << endl;
    oss << "\"BestMoveWinrate\": " << bestMoveWinRate << "," << endl;
    oss << "\"BestMoveCount\": " << bestMoveCount << "," << endl;
    oss << "\"BestMovePolicy\": " << bestMovePolicy << "," << endl;
    oss << "\"BestMoveVN\": " << bestMoveVN << "," << endl;
    oss << "\"RootStatus\": \"" << getWeichiUctNodeStatus(rootStatus) << "\"," << endl;
    oss << "\"NumLeafBenson\": " << WeichiGlobalInfo::getTreeInfo().m_nLeafBenson << "," << endl;
    oss << "\"NumTTmatched\": " << WeichiGlobalInfo::getTreeInfo().m_nTTmatch << "," << endl;
    oss << "\"NumTerminal\": " << WeichiGlobalInfo::getTreeInfo().m_nTerminal << "," << endl;
    oss << "\"NumKos\": " << WeichiGlobalInfo::getTreeInfo().m_nKos << "," << endl;
    oss << "\"NumLongKos\": " << WeichiGlobalInfo::getTreeInfo().m_nLongKos << "," << endl;
    oss << "\"NumRZoneFailed\": " << WeichiGlobalInfo::getTreeInfo().m_nRZoneFailed << "," << endl;
    oss << "\"TimeOfImmediateWin\": " << const_cast<WeichiLifeDeathHandler&>(state.m_lifedeathHandler).getQuickWinHandler().m_timer.getAccumulatedElapsedTime().count() << "," << endl;
    oss << "\"NumKosInSolution\": " << m_kos_in_sol.size() << endl;
    oss << "}" << endl;

    return oss.str();
}

string WeichiLogger::getRootSimInfo()
{
    ostringstream oss;
    for (int i = 0; i < WeichiGlobalInfo::getRootSimInfo().size(); i++) {
        oss << std::get<1>(WeichiGlobalInfo::getRootSimInfo()[i]) << " " << std::get<2>(WeichiGlobalInfo::getRootSimInfo()[i]) << endl;
    }

    return oss.str();
}

string WeichiLogger::getTtData()
{
    RZoneHierarchicalTT& rzoneTT = WeichiGlobalInfo::getHierTT();
    ostringstream oss;

    oss << "-----Summary-----" << endl;
    if (WeichiConfigure::use_rzone_transposition_table) {
        oss << "nTotalDepth: " << rzoneTT.getCrucialStones().size() << endl;
        oss << "nTotalEntry: " << rzoneTT.getTTSize() << endl;
        oss << "nTotalStore: " << rzoneTT.getNumTotalStore() << endl;
        oss << "nTotalReconstructStore: " << rzoneTT.getNumReconstructStore() << endl;
        oss << "nTotalLookup: " << rzoneTT.getNumTotalLookup() << endl;
        oss << "nSkipQuery: " << rzoneTT.getNumSkipEntryCompare() << endl;
        oss << "nSkipOnInternalNode: " << rzoneTT.getNumSkipOnTree() << endl;
        oss << "nFoundResult: " << rzoneTT.getNumHit() << endl;
        oss << "nBucket: " << rzoneTT.getNumBuckets() << endl;
        oss << "nHistoryMaxBucketSize: " << rzoneTT.getNumMaxBucketSize() << endl;
        oss << "nFinalMaxBucketSize: " << rzoneTT.getNumMaxBucketSize() << endl;
        oss << "tTotalStoreTime(s): " << rzoneTT.getStoreTimer().getAccumulatedElapsedTime().count() << endl;
        oss << "tTotalLookUpTime(s): " << rzoneTT.getLookupTimer().getAccumulatedElapsedTime().count() << endl;
        oss << "nTotalHitEntryCompare: " << rzoneTT.getNumHitEntryCompare() << endl;
        oss << "nTotalEntryCompare: " << rzoneTT.getNumTotalEntryCompare() << endl;
        oss << "nLargeRZonePattern: " << rzoneTT.getNumLargeRZonePattern() << endl;
        oss << "nSkipCompareLargeRZPattern: " << rzoneTT.getNumSkipCompareLargeRZPattern() << endl;
        oss << "nReconstruct: " << rzoneTT.getNumRescontruct() << endl;
        oss << "AvgTraverseNum: " << rzoneTT.getAvgTraverse().getMean() << endl;
    } else if (WeichiConfigure::use_rzone_linear_transposition_table) {
        RZoneLinearTT& linearTT = WeichiGlobalInfo::getLinearTT();
        oss << "nTotalDepth: " << 0 << endl;
        oss << "nTotalEntry: " << linearTT.getSize() << endl;
        oss << "nTotalStore: " << linearTT.getNumStore() << endl;
        oss << "nTotalReconstructStore: " << 0 << endl;
        oss << "nTotalLookup: " << linearTT.getNumLookup() << endl;
        oss << "nSkipQuery: " << linearTT.getNumSkipCompare() << endl;
        oss << "nHigherSkip: " << 0 << endl;
        oss << "nFoundResult: " << linearTT.getNumHit() << endl;
        oss << "nBucket: " << 0 << endl;
        oss << "nHistoryMaxBucketSize: " << 0 << endl;
        oss << "nFinalMaxBucketSize: " << 0 << endl;
        oss << "tTotalStoreTime(s): " << linearTT.getStoreTimer().getAccumulatedElapsedTime().count() << endl;
        oss << "tTotalLookUpTime(s): " << linearTT.getLookupTimer().getAccumulatedElapsedTime().count() << endl;
        oss << "nTotalHitEntryCompare: " << linearTT.getNumTotalHitCompare() << endl;
        oss << "nTotalEntryCompare: " << linearTT.getNumTotalEntryCompare() << endl;
        oss << "nLargeRZonePattern: " << 0 << endl;
        oss << "nSkipCompareLargeRZPattern: " << 0 << endl;
        oss << "nReconstruct: " << 0 << endl;
        oss << "AvgTraverseNum: " << linearTT.getNumTotalEntryCompare() / linearTT.getNumLookup() << endl;
    }

    oss << "-----------------" << endl;

    oss << "[Heatmap]" << endl;
    oss << rzoneTT.getHeapMapStr() << endl;
    oss << "[Tree Structure]" << endl;
    int treeNodesTmp = 0;
    oss << rzoneTT.getTableStructureStr(0, treeNodesTmp) << endl;
    oss << "[Tree Nodes]" << endl;
    int treeNodes = 0;
    oss << rzoneTT.getTableNodeStructureStr(0, treeNodes) << endl;
    oss << "[Tree Entries]" << endl;
    oss << rzoneTT.getTableEntryStructureStr(0) << endl;
    oss << "[BucketContent]" << endl;
    oss << rzoneTT.getBucketEntryString();

    return oss.str();
}

string WeichiLogger::getLinearTTSummary()
{
    RZoneLinearTT& linearTT = WeichiGlobalInfo::getLinearTT();
    return linearTT.getStatistic();
}

string WeichiLogger::getTTFullSummary()
{
    RZoneHierarchicalTT& rzoneTT = WeichiGlobalInfo::getHierTT();
    return rzoneTT.getOutputSummaryStatistic();
}

string WeichiLogger::getTTFullSummary2()
{
    RZoneHierarchicalTT& rzoneTT = WeichiGlobalInfo::getHierTT();
    return rzoneTT.getOutputSummaryStatistic2();
}

string WeichiLogger::getReconstructInfo()
{
    RZoneHierarchicalTT& rzoneTT = WeichiGlobalInfo::getHierTT();
    return rzoneTT.getReconstructInfo();
}

string WeichiLogger::getTTNodes()
{
    RZoneHierarchicalTT& rzoneTT = WeichiGlobalInfo::getHierTT();
    return rzoneTT.getSumNodesFromRoot();
}

void WeichiLogger::setOutputConfig(bool bOnlySolution, bool bShowBvMode)
{
    m_bOnlySolution = bOnlySolution;

    return;
}

void WeichiLogger::logSgfRecordWithBestSequence(const string& sFullGame)
{
    string sSgfRecordWithBestSequence = sFullGame;
    sSgfRecordWithBestSequence.pop_back();  // pop ')'
    sSgfRecordWithBestSequence += ";B[tt]"; // add a pass to prevent confusing full game with best sequences

    // iterate every file to get best sequence string
    boost::filesystem::path p(m_sDirectory);
    boost::filesystem::directory_iterator end_it;
    string sBestSequence[MAX_GAME_LENGTH];

    // collect best sequence for each move from best sequence file
    for (boost::filesystem::directory_iterator it(p); it != end_it; ++it) {
        if (boost::filesystem::is_regular_file(it->path())) {
            const string currentFilepath = it->path().string();
            const string currentFilename = it->path().filename().string();
            // here we want to find file like "101_C2.sgf"
            {
                // match .sgf in filename
                boost::smatch result;
                boost::regex pattern("\\.sgf");
                if (!boost::regex_search(currentFilename, result, pattern)) { continue; }
            }
            int moveNum = -1;
            {
                // match number in filename
                boost::smatch result;
                boost::regex pattern("^(\\d+)");
                if (!boost::regex_search(currentFilename, result, pattern)) { continue; }
                moveNum = boost::lexical_cast<int>(result[1]);
            }
            if (moveNum < 0 || moveNum >= MAX_GAME_LENGTH) {
                CERR() << "logSgfRecordWithBestSequence: moveNum error with file \"" << currentFilepath << "\"" << std::endl;
                continue;
            }

            // get all file content to sFileContent
            ifstream fFile(currentFilepath);
            string sFileContent;
            if (!fFile.is_open()) {
                CERR() << "logSgfRecordWithBestSequence: open error with file \"" << currentFilepath << "\"" << std::endl;
                continue;
            }

            fFile.seekg(0, std::ios::end);
            sFileContent.reserve(fFile.tellg());
            fFile.seekg(0, std::ios::beg);
            sFileContent.assign((std::istreambuf_iterator<char>(fFile)), std::istreambuf_iterator<char>());
            fFile.close();

            {
                // match best sequence in file content (from second '(;' to last ')', contain some best sequence and additional ')')
                boost::smatch result;
                boost::regex pattern("\\(.+?(\\(;(?:.|\\r|\\n)+\\))");
                if (!boost::regex_search(sFileContent, result, pattern)) {
                    CERR() << "logSgfRecordWithBestSequence: best sequence file error with file \"" << currentFilepath << "\"" << std::endl;
                    return;
                }
                sBestSequence[moveNum] = result.str(1);
                sBestSequence[moveNum].pop_back(); // delete additional ')'
            }
        }
    }

    // add best sequence
    for (int i = MAX_GAME_LENGTH - 1; i >= 0; --i) {
        if (sBestSequence[i].size() == 0) { continue; }
        int pos;
        int occurrence = -1;
        for (pos = 0; pos < sSgfRecordWithBestSequence.size(); ++pos) {
            if (sSgfRecordWithBestSequence[pos] == ';') { ++occurrence; }
            if (occurrence == i) { break; }
        }
        sSgfRecordWithBestSequence = sSgfRecordWithBestSequence.substr(0, pos) + "(" + sSgfRecordWithBestSequence.substr(pos) + ")";
        sSgfRecordWithBestSequence += sBestSequence[i];
    }

    sSgfRecordWithBestSequence.push_back(')');

    // save to file
    string sFileName = m_sDirectory + "/FullGameWithBestSequence.sgf";
    fstream fSgfRecordWithBestSequence(sFileName.c_str(), ios::out);
    fSgfRecordWithBestSequence << sSgfRecordWithBestSequence;
    fSgfRecordWithBestSequence.close();
}

string WeichiLogger::logTreeInfo_r(NodePtr<WeichiUctNode> pNode, bool bWriteComment, int deep, bool isImportantChild(int deep, int index, const WeichiUctNode* parent, const WeichiUctNode* child))
{
    ostringstream oss;
    if (!pNode->hasChildren()) {
        if (deep != 1 && bWriteComment) { oss << "]"; } // end of C[
        return oss.str();
    }

    if (deep == 1 && bWriteComment) { oss << "C["; }
    deque<WeichiUctNode*> child;
    WeichiUctNode* pChoose = nullptr;
    for (UctChildIterator<WeichiUctNode> it(pNode); it; ++it) {
        if (deep == 1 && it->getMove() == m_bestMove) {
            pChoose = &(*it);
            continue;
        }
        child.push_back(&(*it));
    }
    sort(child.begin(), child.end(),
         [](const WeichiUctNode* lhs, const WeichiUctNode* rhs) {
             return lhs->getUctData().getCount() > rhs->getUctData().getCount();
         });
    if (deep == 1 && pChoose != nullptr) { child.push_front(pChoose); }

    // log all children to pNode's comment
    for (int i = 0; i < child.size(); ++i) {
        if (child[i]->getMove().isPass()) { continue; }

        if (bWriteComment) {
            // oss << "Move " << child[i]->getMove().toGtpString()
            //	<< ": PO(" << child[i]->getUctData().getMean() << "/" << child[i]->getUctData().getCount() << "), "
            //	<< "VN(" << child[i]->getVNValueData().getMean() << "/" << child[i]->getVNValueData().getCount() << ")\n";
            if (deep == 1 && i == 0) {
                double dBlackWinRate = (child[i]->getUctData().getMean() + 1) / 2;
                if (child[i]->getColor() == COLOR_WHITE) { dBlackWinRate = 1.0 - dBlackWinRate; }
                oss << fixed << setprecision(1);
                oss << "B: " << dBlackWinRate * 100.0 << "%, W: " << (1.0 - dBlackWinRate) * 100.0 << "%";
            }
        }
    }
    if (bWriteComment) { oss << "]"; } // end of C[

    // log important children to its comment and call recursively
    for (int i = 0; i < child.size(); ++i) {
        if (child[i]->getMove().isPass()) { continue; }
        if (!isImportantChild(deep, i, &(*pNode), child[i])) { continue; }

        for (UctChildIterator<WeichiUctNode> it(pNode); it; ++it) {
            if (it->getMove() != child[i]->getMove()) { continue; }

            oss << "(;" << it->getMove().toSgfString(true);
            if (bWriteComment) {
                // oss << "C[Win rate: " << it->getUctData().getMean() << ", Simulation count: " << it->getUctData().getCount() << "\n";
                // oss << "VN Win rate: " << it->getVNValueData().getMean() << ", VN count: " << it->getVNValueData().getCount() << "\n\n";
                double dBlackWinRate = (child[i]->getUctData().getMean() + 1) / 2;
                if (it->getColor() == COLOR_WHITE) { dBlackWinRate = 1.0 - dBlackWinRate; }
                oss << fixed << setprecision(1);
                oss << "C[B: " << dBlackWinRate * 100.0 << "%, W: " << (1.0 - dBlackWinRate) * 100.0 << "%";
            }

            oss << logTreeInfo_r(it, bWriteComment, deep + 1, isImportantChild);
            oss << ")";
        }
    }

    return oss.str();
}
