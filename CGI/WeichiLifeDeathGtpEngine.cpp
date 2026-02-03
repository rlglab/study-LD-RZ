#include "WeichiLifeDeathGtpEngine.h"
#include "GtpColorGradient.h"
#include "WeichiLogger.h"

void WeichiLifeDeathGtpEngine::Register()
{
    RegisterFunction("benson", this, &WeichiLifeDeathGtpEngine::cmdBenson, 2);
    RegisterFunction("benson_neutral", this, &WeichiLifeDeathGtpEngine::cmdBensonNeutral, 0);
    RegisterFunction("optmistic_benson", this, &WeichiLifeDeathGtpEngine::cmdOptimisticBenson, 1);
    RegisterFunction("set_win_color", this, &WeichiLifeDeathGtpEngine::cmdSetWinColor, 1);
    RegisterFunction("set_crucial_group", this, &WeichiLifeDeathGtpEngine::cmdSetCrucialGroup, 2);
    RegisterFunction("dragons", this, &WeichiLifeDeathGtpEngine::cmdDragons, 0);
    RegisterFunction("dragon_rzone", this, &WeichiLifeDeathGtpEngine::cmdDragonRZone, 1);
    RegisterFunction("dragon_rzone_all_string", this, &WeichiLifeDeathGtpEngine::cmdDragonRZoneAllString, 0);
    RegisterFunction("dragon_openedareas", this, &WeichiLifeDeathGtpEngine::cmdDragonOpenedAreas, 0);
    RegisterFunction("snakes", this, &WeichiLifeDeathGtpEngine::cmdSnakes, 0);
    RegisterFunction("snake_rzone", this, &WeichiLifeDeathGtpEngine::cmdSnakeRZone, 1);
    RegisterFunction("snake_rzone_all_string", this, &WeichiLifeDeathGtpEngine::cmdSnakeRZoneAllString, 0);
    RegisterFunction("snake_openedareas", this, &WeichiLifeDeathGtpEngine::cmdSnakeOpenedAreas, 0);
    RegisterFunction("eyes", this, &WeichiLifeDeathGtpEngine::cmdEyes, 0);
    RegisterFunction("eye_rzone", this, &WeichiLifeDeathGtpEngine::cmdEyeRZone, 1);
    RegisterFunction("eye_type", this, &WeichiLifeDeathGtpEngine::cmdEyeType, 1);
    RegisterFunction("connectors_threat", this, &WeichiLifeDeathGtpEngine::cmdConnectorsThreat, 0);
    RegisterFunction("connectors_conn", this, &WeichiLifeDeathGtpEngine::cmdConnectorsConn, 0);
    RegisterFunction("strong_connectors", this, &WeichiLifeDeathGtpEngine::cmdConnectorsConn, 0);
    RegisterFunction("immediate_win", this, &WeichiLifeDeathGtpEngine::cmdImmediateWin, 0);
    RegisterFunction("immediate_loss", this, &WeichiLifeDeathGtpEngine::cmdImmediateLoss, 0);
    RegisterFunction("output_uct_tree", this, &WeichiLifeDeathGtpEngine::cmdUctTree, 0, 1);
    RegisterFunction("capture_rzone", this, &WeichiLifeDeathGtpEngine::cmdCaptureRZone, 1);
    RegisterFunction("output_result_json_file", this, &WeichiLifeDeathGtpEngine::cmdOutputResultJsonFile, 1);
    RegisterFunction("life_death_options", this, &WeichiLifeDeathGtpEngine::cmdLifeDeathOptions, 0, 2);
    RegisterFunction("critical_points", this, &WeichiLifeDeathGtpEngine::cmdCriticalPoints, 0);
    RegisterFunction("block_connector", this, &WeichiLifeDeathGtpEngine::cmdBlockConnector, 1);
    RegisterFunction("skeleton_hash", this, &WeichiLifeDeathGtpEngine::cmdSkeletonHash, 0);
    RegisterFunction("end_game_rzone", this, &WeichiLifeDeathGtpEngine::cmdEndGameRZone, 2);
    RegisterFunction("influence_block", this, &WeichiLifeDeathGtpEngine::cmdInfluenBlock, 2);
    RegisterFunction("rzone_dilate", this, &WeichiLifeDeathGtpEngine::cmdRZoneDilate, 0);
    RegisterFunction("set_7x7killall_crucialmap", this, &WeichiLifeDeathGtpEngine::cmdSet7x7killallCrucialMap, 0);
    RegisterFunction("reset_life_and_kill_bitmap", this, &WeichiLifeDeathGtpEngine::cmdResetLifeAndKillBitmap, 0);
    RegisterFunction("set_black_live_bitmap", this, &WeichiLifeDeathGtpEngine::cmdSetBlackLiveBitmap, 1);
    RegisterFunction("set_black_kill_bitmap", this, &WeichiLifeDeathGtpEngine::cmdSetBlackKillBitmap, 1);
    RegisterFunction("set_white_live_bitmap", this, &WeichiLifeDeathGtpEngine::cmdSetWhiteLiveBitmap, 1);
    RegisterFunction("set_white_kill_bitmap", this, &WeichiLifeDeathGtpEngine::cmdSetWhiteKillBitmap, 1);
    RegisterFunction("show_live_and_kill_bitmap", this, &WeichiLifeDeathGtpEngine::cmdShowLiveAndKillBitmap, 1);
    RegisterFunction("set_black_ko_available", this, &WeichiLifeDeathGtpEngine::cmdSetBlackKoAvailable, 0);
    RegisterFunction("set_white_ko_available", this, &WeichiLifeDeathGtpEngine::cmdSetWhiteKoAvailable, 0);
    RegisterFunction("show_ko_available", this, &WeichiLifeDeathGtpEngine::cmdShowKoAvailable, 0);
    RegisterFunction("dcnn_live_and_kill_net", this, &WeichiLifeDeathGtpEngine::cmdDCNNLiveAndKillNet, 0);
    RegisterFunction("show_zone", this, &WeichiLifeDeathGtpEngine::cmdShowZone, 0);
    RegisterFunction("reset_zone", this, &WeichiLifeDeathGtpEngine::cmdResetZone, 0);
    RegisterFunction("add_zone", this, &WeichiLifeDeathGtpEngine::cmdAddZone, 1);
    RegisterFunction("compute_eyespace", this, &WeichiLifeDeathGtpEngine::cmdComputeEyeSpace, 2);
    RegisterFunction("dead_rzone", this, &WeichiLifeDeathGtpEngine::cmdDeadRZone, 1);
    RegisterFunction("7x7_black_life_zone", this, &WeichiLifeDeathGtpEngine::cmd7x7BlackLifeZone, 0);
    RegisterFunction("to_sgf_string", this, &WeichiLifeDeathGtpEngine::cmdToSgfString, 0);
}

string WeichiLifeDeathGtpEngine::getGoguiAnalyzeCommandsString()
{
    ostringstream oss;
    oss << "string/=====Life Death=====/empty\n"
        << "cboard/Benson/benson %c %p\n"
        << "cboard/Benson Neutral/benson_neutral\n"
        << "cboard/Optimistic Benson/optmistic_benson %p\n"
        << "string/Set WinColor/set_win_color %c\n"
        << "sboard/Dragons/dragons\n"
        << "cboard/Dragon RZone/dragon_rzone %p\n"
        << "string/Dragon RZone All String/dragon_rzone_all_string\n"
        << "cboard/Dragon Openedareas/dragon_openedareas\n"
        << "sboard/Snakes/snakes\n"
        << "cboard/Snake RZone/snake_rzone %p\n"
        << "string/Snake RZone All String/snake_rzone_all_string\n"
        << "cboard/Snake Openedareas/snake_openedareas\n"
        << "cboard/Eyes/eyes\n"
        << "cboard/Eye RZone/eye_rzone %p\n"
        << "string/Eye Type/eye_type %p\n"
        << "cboard/Connectors Threat/connectors_threat\n"
        << "cboard/Connectors Conn/connectors_conn\n"
        << "cboard/Immediate Win/immediate_win\n"
        << "cboard/Immediate Loss/immediate_loss\n"
        << "string/Get UCT tree/output_uct_tree\n"
        << "cboard/Capture RZone/capture_rzone %p\n"
        << "param/LifeDeath Options/life_death_options\n"
        << "cboard/Critical Points/critical_points\n"
        << "hpstring/Block Connector/block_connector %p\n"
        << "cboard/Skeleton Hash/skeleton_hash\n"
        << "cboard/End Game RZone/end_game_rzone %c %p\n"
        << "cboard/Influence Block/influence_block %c %p\n"
        << "cboard/CR Zone Dilation All/cr_zone_dilate_all\n"
        << "cboard/CR Zone Dilation Once/cr_zone_dilate_once\n"
        << "cboard/RZone Dilate/rzone_dilate\n"
        << "void/Set 7x7-killall Crucial Map/set_7x7killall_crucialmap\n"
        << "void/Reset Life and Kill Bitmap/reset_life_and_kill_bitmap\n"
        << "cboard/Set Black Live Bitmap/set_black_live_bitmap %p\n"
        << "cboard/Set Black Kill Bitmap/set_black_kill_bitmap %p\n"
        << "cboard/Set White Live Bitmap/set_white_live_bitmap %p\n"
        << "cboard/Set White Kill Bitmap/set_white_kill_bitmap %p\n"
        << "cboard/Show Live And Kill Bitmap/show_live_and_kill_bitmap %c\n"
        << "void/Set Black Ko Available/set_black_ko_available\n"
        << "void/Set White Ko Available/set_white_ko_available\n"
        << "void/Show Ko Available/show_ko_available\n"
        << "gfx/DCNN Live And Kill Net/dcnn_live_and_kill_net\n"
        << "cboard/Show Zone/show_zone\n"
        << "cboard/Reset Zone/reset_zone\n"
        << "cboard/Add Zone/add_zone %p\n"
        << "cboard/Zborder/zborder\n"
        << "cboard/Compute Eye Space/compute_eyespace %c %p\n"
        << "cboard/Dead RZone/dead_rzone %p\n"
        << "cboard/7x7 Black Life Zone/7x7_black_life_zone\n"
        << "string/To Sgf String/to_sgf_string\n";

    return oss.str();
}

void WeichiLifeDeathGtpEngine::cmdBenson()
{
    Color color = toColor(m_args[0].at(0));
    string sMove = m_args[1];
    transform(sMove.begin(), sMove.end(), sMove.begin(), ::toupper);
    WeichiMove move(color, sMove);

    WeichiBitBoard bmRZone = WeichiRZoneHandler::calculateBensonRZone(m_mcts.getState().m_board, move);

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmRZone.BitIsOn(*it))
            oss << RGB_RED.toString() << " ";
        else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdBensonNeutral()
{
    ostringstream oss;
    oss << endl;

    WeichiBitBoard bmBlackBenson = const_cast<WeichiThreadState&>(m_mcts.getState()).m_board.getBensonBitboard(COLOR_BLACK);
    WeichiBitBoard bmWhiteBenson = const_cast<WeichiThreadState&>(m_mcts.getState()).m_board.getBensonBitboard(COLOR_WHITE);

    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmBlackBenson.BitIsOn(*it) && bmWhiteBenson.BitIsOn(*it)) {
            oss << RGB_RED.toString() << " ";
        } else if (bmBlackBenson.BitIsOn(*it)) {
            oss << RGB_BLACK.toString() << " ";
        } else if (bmWhiteBenson.BitIsOn(*it)) {
            oss << RGB_WHITE.toString() << " ";
        } else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdOptimisticBenson()
{
    StopTimer timer;
    timer.start();
    string sMove = m_args[0];
    transform(sMove.begin(), sMove.end(), sMove.begin(), ::toupper);
    Move move(COLOR_NONE, sMove);

    const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath();
    const Vector<WeichiDragon, MAX_NUM_BLOCKS>& vSnakes = const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getSnakes();
    WeichiBitBoard bmResult;
    for (int iSnake = 0; iSnake < vSnakes.size(); ++iSnake) {
        const WeichiDragon& snake = vSnakes[iSnake];
        if (!snake.getStoneMap().BitIsOn(move.getPosition())) { continue; }
        bool bHasPotentialRZone = const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getQuickWinHandler().hasPotentialRZone(snake, bmResult);
        if (bHasPotentialRZone) {
            break;
        } else {
            bmResult.Reset();
        }
    }

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmResult.BitIsOn(*it))
            oss << RGB_RED.toString() << " ";
        else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    timer.stop();

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdSetWinColor()
{
    Color color = toColor(m_args[0].at(0));
    WeichiConfigure::win_color = int(color);
    WeichiGlobalInfo::getTreeInfo().m_winColor = Color(WeichiConfigure::win_color);
    ostringstream oss;
    oss << "Set win color to " << toChar(Color(WeichiConfigure::win_color));

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdSetCrucialGroup()
{
    Color color = toColor(m_args[0].at(0));
    WeichiGlobalInfo::getEndGameCondition().reset();
    WeichiGlobalInfo::getEndGameCondition().setCrucialStones(const_cast<WeichiThreadState&>(m_mcts.getState()), color, m_args[1]);

    ostringstream oss;
    oss << WeichiGlobalInfo::getEndGameCondition().getCruicalGroupInfo(color);

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdDragons()
{
    const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath();
    Vector<WeichiDragon, MAX_NUM_BLOCKS>& vDragons = const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getDragons();

    int iDragons[MAX_NUM_GRIDS];
    for (int i = 0; i < MAX_NUM_GRIDS; i++) { iDragons[i] = -1; }

    CERR() << "Dragon Size: " << vDragons.size() << endl;
    for (int iDragon = 0; iDragon < vDragons.size(); ++iDragon) {
        WeichiDragon& dragon = vDragons[iDragon];
        WeichiBitBoard bmDragon = dragon.getStoneMap();
        CERR() << "---Dragon " << iDragon << "---" << endl;
        CERR() << "Block Size: " << dragon.getNumBlocks() << endl;
        CERR() << "CA Size: " << dragon.getNumClosedAreas() << endl;
        CERR() << "OA Size: " << dragon.getNumOpenedAreas() << endl;
        int pos = 0;
        while ((pos = bmDragon.bitScanForward()) != -1) {
            iDragons[pos] = iDragon;
        }
    }

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (iDragons[*it] == -1) {
            oss << "\"\" ";
        } else {
            oss << iDragons[*it] << " ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdDragonRZone()
{
    string sMove = m_args[0];
    transform(sMove.begin(), sMove.end(), sMove.begin(), ::toupper);
    Move move(COLOR_NONE, sMove);

    WeichiBitBoard bmRZone;
    const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath();
    Vector<WeichiDragon, MAX_NUM_BLOCKS>& vDragons = const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getDragons();
    for (int iDragon = 0; iDragon < vDragons.size(); ++iDragon) {
        WeichiDragon& dragon = vDragons[iDragon];
        if (dragon.getStoneMap().BitIsOn(move.getPosition())) {
            bmRZone = dragon.getRZone();
            break;
        }
    }

    ostringstream oss;
    oss << endl;

    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmRZone.BitIsOn(*it)) {
            oss << RGB_GREEN.toString() << " ";
        } else {
            oss << "\"\" ";
        }
        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdDragonRZoneAllString()
{
    const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath();
    reply(GTP_SUCC, const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getAllDragonRZoneString());
}

void WeichiLifeDeathGtpEngine::cmdSnakes()
{
    const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath();
    Vector<WeichiDragon, MAX_NUM_BLOCKS>& vsnakes = const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getSnakes();

    int isnakes[MAX_NUM_GRIDS];
    for (int i = 0; i < MAX_NUM_GRIDS; i++) { isnakes[i] = -1; }

    CERR() << "snake Size: " << vsnakes.size() << endl;
    for (int isnake = 0; isnake < vsnakes.size(); ++isnake) {
        WeichiDragon& snake = vsnakes[isnake];
        WeichiBitBoard bmSnake = snake.getStoneMap();
        CERR() << "Block Size: " << snake.getNumBlocks() << endl;
        CERR() << "CA Size: " << snake.getNumClosedAreas() << endl;
        CERR() << "OA Size: " << snake.getNumOpenedAreas() << endl;
        int pos = 0;
        while ((pos = bmSnake.bitScanForward()) != -1) {
            isnakes[pos] = isnake;
        }
    }

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (isnakes[*it] == -1) {
            oss << "\"\" ";
        } else {
            oss << isnakes[*it] << " ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdSnakeRZone()
{
    string sMove = m_args[0];
    transform(sMove.begin(), sMove.end(), sMove.begin(), ::toupper);
    Move move(COLOR_NONE, sMove);

    WeichiBitBoard bmRZone;
    const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath();
    Vector<WeichiDragon, MAX_NUM_BLOCKS>& vsnakes = const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getSnakes();
    for (int isnake = 0; isnake < vsnakes.size(); ++isnake) {
        WeichiDragon& snake = vsnakes[isnake];
        if (snake.getStoneMap().BitIsOn(move.getPosition())) {
            bmRZone = snake.getRZone();
            break;
        }
    }

    ostringstream oss;
    oss << endl;

    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmRZone.BitIsOn(*it)) {
            oss << RGB_RED.toString() << " ";
        } else {
            oss << "\"\" ";
        }
        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdSnakeRZoneAllString()
{
    // const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath() ;
    reply(GTP_SUCC, "");
}

void WeichiLifeDeathGtpEngine::cmdSnakeOpenedAreas()
{
    const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath();
    ostringstream oss;
    oss << const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getOpenedAreasHandler().getSnakeOpenedAreasDrawingString();

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdImmediateWin()
{
    WeichiBitBoard bmImmediateWin = const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getFullBoardImmediateWin();

    ostringstream oss;
    oss << endl;
    int pos = 0;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmImmediateWin.BitIsOn(*it)) {
            oss << RGB_RED.toString() << " ";
        } else {
            oss << "\"\" ";
        }
        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdImmediateLoss()
{
    WeichiBitBoard bmImmediateLoss = const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getFullBoardImmediateLoss();

    ostringstream oss;
    oss << endl;
    int pos = 0;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmImmediateLoss.BitIsOn(*it)) {
            oss << RGB_RED.toString() << " ";
        } else {
            oss << "\"\" ";
        }
        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdEyes()
{
    const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath();
    Vector<WeichiDragon, MAX_NUM_BLOCKS>& vDragons = const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getDragons();

    WeichiBitBoard bmEye;
    for (int iDragon = 0; iDragon < vDragons.size(); ++iDragon) {
        WeichiDragon& dragon = vDragons[iDragon];
        for (int iEye = 0; iEye < dragon.getNumClosedAreas(); ++iEye) {
            WeichiClosedArea* eye = dragon.getClosedAreas()[iEye];

            if (eye->getEyeStatus() != EYE_UNKNOWN) {
                bmEye |= eye->getStoneMap();
            }
        }
    }

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmEye.BitIsOn(*it)) {
            oss << RGB_RED.toString() << " ";
        } else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdEyeRZone()
{
    string sMove = m_args[0];
    transform(sMove.begin(), sMove.end(), sMove.begin(), ::toupper);
    Move move(COLOR_NONE, sMove);

    const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath();
    WeichiBitBoard bmEye = const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getEyeRZone(move.getPosition());

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmEye.BitIsOn(*it)) {
            oss << RGB_GREEN.toString() << " ";
        } else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdEyeType()
{
    string sMove = m_args[0];
    transform(sMove.begin(), sMove.end(), sMove.begin(), ::toupper);
    Move move(COLOR_NONE, sMove);

    const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath();
    Vector<WeichiDragon, MAX_NUM_BLOCKS>& vDragons = const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getDragons();

    ostringstream oss;
    oss << endl;

    bool findEye = false;
    for (int iDragon = 0; iDragon < vDragons.size(); ++iDragon) {
        WeichiDragon& dragon = vDragons[iDragon];
        for (int iEye = 0; iEye < dragon.getNumClosedAreas(); ++iEye) {
            WeichiClosedArea* eye = dragon.getClosedAreas()[iEye];
            if (eye->getStoneMap().BitIsOn(move.getPosition())) {
                oss << "dragon " << iDragon << ": " << getEyeStatusString(eye->getEyeStatus()) << endl;
                break;
            }
        }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdDragonOpenedAreas()
{
    const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath();
    ostringstream oss;
    oss << const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getOpenedAreasHandler().getDragonOpenedAreasDrawingString();

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdConnectorsThreat()
{
    const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath();
    ostringstream oss;
    oss << const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getConnectorHandler().getConnectorThreatDrawingString();

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdConnectorsConn()
{
    const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath();
    ostringstream oss;
    oss << const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getConnectorHandler().getConnectorConnDrawingString();

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdUctTree()
{
    string filenameEditor;
    fstream fsEditor;
    if (m_args.size() == 0) {
        filenameEditor = "uct_tree_editor_" + TimeSystem::getTimeString("Y_m_d_H_i_s") + "_sol.esgf";
    } else if (m_args.size() == 1) {
        filenameEditor = m_args[0];
    }
    WeichiGlobalInfo::getTreeInfo().m_problemSgfUctTreeFilename = filenameEditor;

    WeichiLogger::setOutputConfig(false, false);
    string sSgfResultEditor = WeichiLogger::getTsumeGoTree(m_mcts.getState().m_board);
    fsEditor.open(filenameEditor.c_str(), ios::out);
    fsEditor << sSgfResultEditor;
    fsEditor.close();

    reply(GTP_SUCC, "");
}

void WeichiLifeDeathGtpEngine::cmdCaptureRZone()
{
    string sMove = m_args[0];
    transform(sMove.begin(), sMove.end(), sMove.begin(), ::toupper);
    Move move(COLOR_NONE, sMove);

    Color toPlay = m_mcts.getState().m_board.getToPlay();
    WeichiMove captureMove(toPlay, move.getPosition());
    WeichiBitBoard bmRZone = m_mcts.getState().m_board.getCaptureRZone(captureMove);

    ostringstream oss;
    oss << endl;

    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmRZone.BitIsOn(*it)) {
            oss << RGB_RED.toString() << " ";
        } else {
            oss << "\"\" ";
        }
        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdOutputResultJsonFile()
{
    string sJsonFilename = m_args[0];
    fstream fsJson;
    fsJson.open(sJsonFilename.c_str(), ios::out);
    fsJson << WeichiLogger::getJsonResultFile(m_mcts.getState());
    fsJson.close();

    string sTtData;
    if (sJsonFilename.find_first_of("/") == string::npos) {
        sTtData = "" + sJsonFilename.substr(7, sJsonFilename.find_last_of(".json") - 11) + ".tt";
    } else {
        int lastIndexSlash = sJsonFilename.find_last_of("/");
        string prefix = sJsonFilename.substr(0, lastIndexSlash + 1);
        int dotJsonIndex = sJsonFilename.find_last_of(".json");
        sTtData = prefix + sJsonFilename.substr(lastIndexSlash + 8, dotJsonIndex - lastIndexSlash - 12) + ".tt";
    }

    fstream fsTtData;
    fsTtData.open(sTtData.c_str(), ios::out);
    fsTtData << WeichiLogger::getTtData();
    fsTtData.close();

    reply(GTP_SUCC, "");
}

void WeichiLifeDeathGtpEngine::cmdLifeDeathOptions()
{
    ostringstream oss;
    if (m_args.size() == 0) {
        oss << "[bool] only_find_winner " << WeichiConfigure::only_find_winner << '\n'
            << "[bool] igore_benson " << WeichiConfigure::ignore_benson << '\n';
    } else if (m_args.size() == 2) {
        if (m_args[0] == "only_find_winner") {
            WeichiConfigure::only_find_winner = (m_args[1] == "1");
        } else if (m_args[0] == "igore_benson") {
            WeichiConfigure::ignore_benson = (m_args[1] == "1");
        } else {
            reply(GTP_FAIL, "unknown parameter " + m_args[0]);
        }
    } else {
        reply(GTP_FAIL, "need 0 or 2 arguments");
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdCriticalPoints()
{
    const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath();
    WeichiBitBoard bmCritical = const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getCriticalPoints();

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmCritical.BitIsOn(*it)) {
            oss << RGB_RED.toString() << " ";
        } else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdBlockConnector()
{
    const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath();

    string sMove = m_args[0];
    transform(sMove.begin(), sMove.end(), sMove.begin(), ::toupper);
    Move move(COLOR_NONE, sMove);

    const WeichiBlock* block = m_mcts.getState().m_board.getGrid(move.getPosition()).getBlock();
    vector<WeichiConnector*>& vConnectors = const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getConnectorHandler().getBlockConnectors(block);

    ostringstream oss;
    oss << "Connectors[" << vConnectors.size() << "]:" << endl;
    for (int iConn = 0; iConn < vConnectors.size(); ++iConn) {
        oss << "\tIndex: " << iConn << endl;
        oss << "\t\tType: " << vConnectors[iConn]->toTypeString() << endl;

        WeichiBitBoard bmConns = vConnectors[iConn]->getConnectorBitmap();
        WeichiBitBoard bmThreats = vConnectors[iConn]->getThreatBitmap();

        oss << "\t\tConnect: ";
        int pos = 0;
        while ((pos = bmConns.bitScanForward()) != -1) {
            oss << WeichiMove(COLOR_NONE, pos).toGtpString() << " ";
        }
        oss << endl;

        oss << "\t\tThreat: ";
        while ((pos = bmThreats.bitScanForward()) != -1) {
            oss << WeichiMove(COLOR_NONE, pos).toGtpString() << " ";
        }
        oss << endl;
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdSkeletonHash()
{
    const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.findFullBoardLifeDeath();
    HashKey64 hashkey = const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getRepresentativeSkeletonHashkey();
    CERR() << hashkey << endl;
    WeichiBitBoard bmSkeleton = const_cast<WeichiThreadState&>(m_mcts.getState()).m_lifedeathHandler.getRepresentativeSkeletonBitBoard();

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmSkeleton.BitIsOn(*it)) {
            oss << RGB_RED.toString() << " ";
        } else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdEndGameRZone()
{
    Color color = toColor(m_args[0].at(0));
    string sMove = m_args[1];
    transform(sMove.begin(), sMove.end(), sMove.begin(), ::toupper);
    WeichiMove move(color, sMove);

    WeichiBitBoard bmRZone = WeichiGlobalInfo::getEndGameCondition().getEndGameRZone(const_cast<WeichiThreadState&>(m_mcts.getState()), color);

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmRZone.BitIsOn(*it)) {
            oss << RGB_RED.toString() << " ";
        } else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    if (!WeichiGlobalInfo::getEndGameCondition().isEndGame(const_cast<WeichiThreadState&>(m_mcts.getState()), color)) {
        cout << "Not ENgame" << endl;
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdReleaseKo()
{
    Color color = toColor(m_args[0].at(0));
    if (color == COLOR_BLACK) {
        WeichiConfigure::black_ignore_ko = false;
    } else if (color == COLOR_WHITE) {
        WeichiConfigure::white_ignore_ko = false;
    }

    ostringstream oss;
    oss << "WeichiConfigure::black_ignore_ko: " << WeichiConfigure::black_ignore_ko << endl;
    oss << "WeichiConfigure::white_ignore_ko: " << WeichiConfigure::white_ignore_ko << endl;

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdInfluenBlock()
{
    Color color = toColor(m_args[0].at(0));
    string sMove = m_args[1];
    transform(sMove.begin(), sMove.end(), sMove.begin(), ::toupper);
    Move move(color, sMove);

    WeichiBitBoard bmInlfuence = m_mcts.getState().m_board.getInfluenceBlocks(move);

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmInlfuence.BitIsOn(*it)) {
            oss << RGB_RED.toString() << " ";
        } else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdRZoneDilate()
{
    Color ownColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
    WeichiBitBoard bmPreRZone = WeichiGlobalInfo::getZone();
    WeichiBitBoard bmDilateZone = m_mcts.getState().m_board.getAndPlayerRZone(bmPreRZone, ownColor);

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmDilateZone.BitIsOn(*it)) {
            oss << RGB_BLUE.toString() << " ";
        } else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdSet7x7killallCrucialMap()
{
    Dual<WeichiBitBoard>& lifeStones = const_cast<WeichiThreadState&>(m_mcts.getState()).m_board.getLADToLifeStones();
    Dual<WeichiBitBoard>& killStones = const_cast<WeichiThreadState&>(m_mcts.getState()).m_board.getLADToKillStones();

    lifeStones.reset();
    killStones.reset();

    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        lifeStones.m_white.SetBitOn(*it);
        killStones.m_white.SetBitOn(*it);
    }

    CERR() << "White Life Bitmap:" << endl;
    CERR() << lifeStones.m_white.toString() << endl;
    CERR() << "White Kill Bitmap:" << endl;
    CERR() << killStones.m_white.toString() << endl;

    reply(GTP_SUCC, "");
}

void WeichiLifeDeathGtpEngine::cmdResetLifeAndKillBitmap()
{
    m_LiveStones.reset();
    m_killStones.reset();

    reply(GTP_SUCC, "");
}

void WeichiLifeDeathGtpEngine::cmdSetBlackLiveBitmap()
{
    setLiveAndKillBitmap(COLOR_BLACK, GOAL_LIVE);
}

void WeichiLifeDeathGtpEngine::cmdSetBlackKillBitmap()
{
    setLiveAndKillBitmap(COLOR_BLACK, GOAL_KILL);
}

void WeichiLifeDeathGtpEngine::cmdSetWhiteLiveBitmap()
{
    setLiveAndKillBitmap(COLOR_WHITE, GOAL_LIVE);
}

void WeichiLifeDeathGtpEngine::cmdSetWhiteKillBitmap()
{
    setLiveAndKillBitmap(COLOR_WHITE, GOAL_KILL);
}

void WeichiLifeDeathGtpEngine::cmdShowLiveAndKillBitmap()
{
    ostringstream oss;
    oss << endl;

    Color color = toColor(m_args[0].at(0));
    WeichiBitBoard bmLiveStone = m_LiveStones.get(color);
    WeichiBitBoard bmKillStone = m_killStones.get(color);

    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmLiveStone.BitIsOn(*it) && bmKillStone.BitIsOn(*it)) {
            oss << RGB_GREEN.toString() << " ";
        } else if (bmLiveStone.BitIsOn(*it)) {
            oss << RGB_BLUE.toString() << " ";
        } else if (bmKillStone.BitIsOn(*it)) {
            oss << RGB_RED.toString() << " ";
        } else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdSetBlackKoAvailable()
{
    if (WeichiConfigure::black_ignore_ko) {
        WeichiConfigure::black_ignore_ko = false;
    } else {
        WeichiConfigure::black_ignore_ko = true;
    }

    cmdShowKoAvailable();
}

void WeichiLifeDeathGtpEngine::cmdSetWhiteKoAvailable()
{
    if (WeichiConfigure::white_ignore_ko) {
        WeichiConfigure::white_ignore_ko = false;
    } else {
        WeichiConfigure::white_ignore_ko = true;
    }

    cmdShowKoAvailable();
}

void WeichiLifeDeathGtpEngine::cmdShowKoAvailable()
{
    ostringstream oss;
    oss << endl;
    oss << "Black Ko is ";
    if (WeichiConfigure::black_ignore_ko) {
        oss << "not available." << endl;
    } else {
        oss << "available." << endl;
    }

    oss << "White Ko is ";
    if (WeichiConfigure::white_ignore_ko) {
        oss << "not available." << endl;
    } else {
        oss << "available." << endl;
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdDCNNLiveAndKillNet()
{
    WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
    // set live and kill bitmap
    WeichiBoard& board = state.m_board;
    board.getLADToLifeStones() = m_LiveStones;
    board.getLADToKillStones() = m_killStones;
    // set black and white ko available
    if (WeichiConfigure::black_ignore_ko) {
        board.getLADKoAvailable().get(COLOR_BLACK) = false;
    } else {
        board.getLADKoAvailable().get(COLOR_BLACK) = true;
    }
    if (WeichiConfigure::white_ignore_ko) {
        board.getLADKoAvailable().get(COLOR_WHITE) = false;
    } else {
        board.getLADKoAvailable().get(COLOR_WHITE) = true;
    }

    state.m_rootFilter.startFilter();
    WeichiCNNNet* cnnNet = state.m_dcnnNetCollection.getCNNNet();
    WeichiCNNFeatureGenerator::calDCNNFeatureAndAddToNet(state, cnnNet);
    cnnNet->forward();
    vector<CandidateEntry> vCandidate = cnnNet->getSLCandidates(0);
    sort(vCandidate.begin(), vCandidate.end());

    ostringstream oss;
    oss << "LABEL ";
    double dPreMoveScore = vCandidate[0].getScore();
    for (uint i = 0; i < vCandidate.size(); i++) {
        if (vCandidate[i].getPosition() == PASS_MOVE.getPosition() || vCandidate[i].getPosition() == -1) { continue; }
        oss << WeichiMove(vCandidate[i].getPosition()).toGtpString() << " " << vCandidate[i].getScore() << " ";
    }
    oss << endl;

    CERR() << "To Play: " << toChar(state.m_board.getToPlay()) << endl;
    // CERR() << "Other probability: ";
    // for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
    //	if (!cnnNet->isLegal(0, *it)) { CERR() << WeichiMove(*it).toGtpString() << " " << cnnNet->getSLResult(0, *it) << " "; }
    // }
    // if (!cnnNet->isLegal(0, PASS_MOVE.getPosition())) {
    //	CERR() << PASS_MOVE.toGtpString() << " " << cnnNet->getSLResult(0, PASS_MOVE.getPosition());
    // }
    CERR() << endl;

    if (WeichiConfigure::dcnn_use_ftl) {
        for (int i = 0; i < cnnNet->getVNLabelSize(); ++i) {
            CERR() << i << ": " << cnnNet->getVNResult(0, i) << "\t";
            if ((i + 1) % 5 == 0) { CERR() << endl; }
        }
    } else {
        float fCenterKomi = (WeichiConfigure::BoardSize == 7 ? 0 : 7.5);
        double dMinKomi = fCenterKomi - (cnnNet->getVNLabelSize() - 1) / 2;
        double dMaxKomi = fCenterKomi + (cnnNet->getVNLabelSize() - 1) / 2;
        for (double dKomi = dMinKomi; dKomi <= dMaxKomi; dKomi++) {
            CERR() << dKomi << ": " << cnnNet->getVNResult(0, dKomi) << "\t";
            if ((static_cast<int>(dKomi - dMaxKomi) + 1) % 5 == 0) { CERR() << endl; }
        }
    }
    CERR() << endl;

    // if (cnnNet->hasGLOutput()) {
    //	CERR() << "GL output: " << endl;
    //	vector<float> vGLResults = cnnNet->getGLResult(0);
    //	for (int i = 0; i < vGLResults.size(); ++i) {
    //		CERR() << i << ": " << vGLResults[i] << "\t";
    //		if (i != 0 && i % 5 == 0) { CERR() << endl; }
    //	}
    // }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdShowZone()
{
    WeichiBitBoard bmRZone = WeichiGlobalInfo::getZone();

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmRZone.BitIsOn(*it)) {
            oss << RGB_BLUE.toString() << " ";
        } else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdResetZone()
{
    WeichiBitBoard bmRZone = WeichiGlobalInfo::resetZone();

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmRZone.BitIsOn(*it)) {
            oss << RGB_BLUE.toString() << " ";
        } else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdAddZone()
{
    string sMove = m_args[0];
    transform(sMove.begin(), sMove.end(), sMove.begin(), ::toupper);
    WeichiMove move(COLOR_NONE, sMove);
    WeichiBitBoard bmCurrentZone = WeichiGlobalInfo::getZone();
    WeichiBitBoard bmNewRZone;
    const WeichiBlock* block = m_mcts.getState().m_board.getGrid(move.getPosition()).getBlock();
    if (block) {
        if (bmCurrentZone.BitIsOn(move.getPosition())) {
            bmNewRZone = WeichiGlobalInfo::subZone(block->getStoneMap());
        } else {
            bmNewRZone = WeichiGlobalInfo::addZone(block->getStoneMap());
        }
    } else {
        if (bmCurrentZone.BitIsOn(move.getPosition())) {
            bmNewRZone = WeichiGlobalInfo::subZonePoint(move.getPosition());
        } else {
            bmNewRZone = WeichiGlobalInfo::addZonePoint(move.getPosition());
        }
    }

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmNewRZone.BitIsOn(*it)) {
            oss << RGB_BLUE.toString() << " ";
        } else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdToSgfString()
{
    reply(GTP_SUCC, m_mcts.getState().m_board.toSgfFileString());
}

void WeichiLifeDeathGtpEngine::cmdComputeEyeSpace()
{
    Color color = toColor(m_args[0].at(0));
    string sMove = m_args[1];
    transform(sMove.begin(), sMove.end(), sMove.begin(), ::toupper);
    Move move(color, sMove);

    WeichiBoard& board = const_cast<WeichiThreadState&>(m_mcts.getState()).m_board;
    WeichiClosedArea* ca = board.getGrid(move).getClosedArea(color);
    WeichiBitBoard bmEyeSpace;
    WeichiSafetySolver& safetySolver = WeichiGlobalInfo::getEndGameCondition().getSafetySolver();
    if (ca) {
        bool isRegionSafe = safetySolver.isRegionSafe(ca, const_cast<WeichiThreadState&>(m_mcts.getState()));
        bmEyeSpace = safetySolver.getEyeSpace();
    }

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmEyeSpace.BitIsOn(*it)) {
            oss << RGB_RED.toString() << " ";
        } else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmdDeadRZone()
{
    string sMove = m_args[0];
    transform(sMove.begin(), sMove.end(), sMove.begin(), ::toupper);
    Move move(COLOR_NONE, sMove);

    WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());

    WeichiBitBoard bmDeadStone = WeichiGlobalInfo::getEndGameCondition().getDeadBlockRZone(state, state.m_board.getGrid(move).getBlock());

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmDeadStone.BitIsOn(*it)) {
            oss << RGB_RED.toString() << " ";
        } else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::setLiveAndKillBitmap(Color color, WeichiSearchGoal goal)
{
    string sMove = m_args[0];
    transform(sMove.begin(), sMove.end(), sMove.begin(), ::toupper);
    Move move(COLOR_NONE, sMove);

    Color liveColor = (goal == GOAL_LIVE) ? color : AgainstColor(color);
    Color killColor = (goal == GOAL_KILL) ? color : AgainstColor(color);
    WeichiBitBoard& bmLiveStone = m_LiveStones.get(liveColor);
    WeichiBitBoard& bmKillStone = m_killStones.get(killColor);

    if (bmLiveStone.BitIsOn(move.getPosition())) {
        bmLiveStone.SetBitOff(move.getPosition());
    } else {
        bmLiveStone.SetBitOn(move.getPosition());
    }
    if (bmKillStone.BitIsOn(move.getPosition())) {
        bmKillStone.SetBitOff(move.getPosition());
    } else {
        bmKillStone.SetBitOn(move.getPosition());
    }

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmLiveStone.BitIsOn(*it) && bmKillStone.BitIsOn(*it)) {
            oss << RGB_GREEN.toString() << " ";
        } else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
}

void WeichiLifeDeathGtpEngine::cmd7x7BlackLifeZone()
{
    string sMove = m_args[0];
    transform(sMove.begin(), sMove.end(), sMove.begin(), ::toupper);
    Move move(COLOR_NONE, sMove);

    WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
    WeichiBitBoard bmBlackLifeZone = state.m_board.getAliveArea(COLOR_BLACK);

    ostringstream oss;
    oss << endl;
    for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
        if (bmBlackLifeZone.BitIsOn(*it)) {
            oss << RGB_BLACK.toString() << " ";
        } else {
            oss << "\"\" ";
        }

        if (*it % MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
    }

    reply(GTP_SUCC, oss.str());
} 
