#include "arguments.h"
#include "types.h"

#include "WeichiCNNServer.h"
#include "WeichiProxy.h"
#include "WeichiWorker.h"

#include "concepts.h"
#include "strops.h"

#include "WeichiGtpEngine.h"

#include "ConfigureLoader.h"
#include "Pattern33Builder.h"
#include "ProgramOption.h"
#include "WeichiConfigure.h"

#include "GenSgf.h"
#include "RLDCNN.h"
#include "SimulationBalancing.h"
#include "TCaffe.h"
#include "TNetFight.h"
#include "Train.h"
#include "TsumegoSolver.h"
#include "WeichiGammaTable.h"
#include "WeichiJLMctsHandler.h"
#include "WeichiLogger.h"
#include "ZeroOptimization.h"
#include "ZeroSelfPlay.h"
#include "ZeroWorkerHandler.h"

#if defined(_MSC_VER)
#include <Windows.h>
#else
#include <sched.h>
#endif

void initializeStatic()
{
    pattern33::Pattern33Builder builder;
    if (!builder.build(WeichiConfigure::pattern_dir)) {
        CERR() << "Build Pattern Fail, " << builder.getError() << endl;
        exit(1);
    }
    WeichiBoard::initializeStatic();
    WeichiGlobalInfo::getTreeInfo().m_dRemainTime.m_black = WeichiConfigure::TimeControlMainTime;
    WeichiGlobalInfo::getTreeInfo().m_dRemainTime.m_white = WeichiConfigure::TimeControlMainTime;
    if (WeichiConfigure::DoLog) {
        string sDirectoryName = WeichiConfigure::log_dir + TimeSystem::getTimeString("Y_m_d_H_i_s");
        if (WeichiConfigure::log_postfix != "") { sDirectoryName += "_" + WeichiConfigure::log_postfix; }
        WeichiLogger::initialize(sDirectoryName);
    }
}

void setMasterDCNNNet(MCTS& mcts)
{
    // master thread will use first thread to set all CNN net, and also display net initialize error message
    WeichiThreadState& state = const_cast<WeichiThreadState&>(mcts.getState());
    vector<string> vParamString = splitToVector(WeichiConfigure::dcnn_net, ':');
    state.m_dcnnNetCollection.initialize(1, vParamString, -1, true);
}

void runGTP()
{
    if (Configure::Pondering) {
        Configure::Pondering = false;
        CERR() << "Pondering is not supported in " << arguments::mode << " mode" << endl;
    }

    MCTS mcts;
    setMasterDCNNNet(mcts);
    WeichiGtpEngine engine(Logger::getTeeOut(), mcts);

    CERR() << "Program start successfully." << endl;

    string line;
    while (getline(cin, line)) {
        engine.runCommand(line);
        if (engine.hasQuit())
            break;
    }
}

void runPerformanceTest()
{
    if (WeichiConfigure::CPUMask) {
#if defined(_MSC_VER)
        DWORD_PTR mask = (DWORD_PTR)WeichiConfigure::CPUMask;
        SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
        SetProcessAffinityMask(GetCurrentProcess(), mask);
#else
        cpu_set_t mask;
        CPU_ZERO(&mask);
        unsigned long long cpu_mask = WeichiConfigure::CPUMask;
        for (int i = 0; cpu_mask; ++i) {
            if (cpu_mask & 1)
                CPU_SET(i, &mask);
            cpu_mask >>= 1;
        }
        sched_setaffinity(0, sizeof(mask), &mask);
#endif
    }

    MCTS mcts;
    mcts.genmove();
}

void runJLMCTS()
{
    MCTS mcts;
    setMasterDCNNNet(mcts);
    WeichiJLMctsHandler jlMctsHandler(mcts);
    jlMctsHandler.solve();
    COUT() << jlMctsHandler.getResult();
}

void runAsCNNWorker()
{
    using namespace Configure;
    try {
        if (Configure::Pondering) {
            Configure::Pondering = false;
            CERR() << "Pondering is not supported in " << arguments::mode << " mode" << endl;
        }
        MCTS mcts;
        setMasterDCNNNet(mcts);
        WeichiWorker::initialize(mcts, Configure::server_host, Configure::server_port);
        WeichiWorker::run();
    } catch (std::exception& e) {
        CERR() << e.what() << endl;
    }
}

void runAsCNNServer()
{
    using namespace Configure;
    try {
        WeichiCNNServer server;
        setMasterDCNNNet(server.getMCTS());
        server.run();
    } catch (std::exception& e) {
        CERR() << e.what() << endl;
    }
}

void runProxy()
{
    WeichiProxy proxy(Configure::server_port);
    proxy.run();
}

void runRLDCNN()
{
    RLDCNN rldcnn;
    rldcnn.start(WeichiConfigure::num_rl_iteration);
}

void runTNetFight()
{
    TNetFightMaster tNetFight;
    tNetFight.run();
}

void runTsumegoSolver()
{
    CERR() << "[Run as Tsumego Solver]" << endl;
    CERR() << "Problem Dir: " << WeichiConfigure::problem_dir << endl;
    CERR() << "Result Dir: " << WeichiConfigure::result_dir << endl;

    MCTS mcts;
    setMasterDCNNNet(mcts);
    TsumegoSolver solver(mcts);
    solver.run();
}

void runTrain()
{
    Train train;
    train.start();
}

void runCaffeTrain()
{
    TCaffeMaster tCaffeMaster;
    tCaffeMaster.train();
}

void runGenSgfServer()
{
    GenSgfServer genSgfServer(Configure::server_port);
    genSgfServer.run();
}

void runGenSgfWorker()
{
    GenSgfMaster genSgfMaster(Configure::server_host, ToString(Configure::server_port), Configure::NumThread);
    genSgfMaster.run();
}

void runSBServer()
{
    int numClient = 4;
    SBServer sbServer(Configure::server_port, numClient);
    sbServer.run();
}

void runSBWorker()
{
    SBClient sbClient(Configure::server_host, ToString(Configure::server_port), Configure::NumThread);

    sbClient.run();
}

void runZeroServer()
{
    ZeroWorkerHandler server(WeichiConfigure::zero_server_port);
    server.run();
}

void runZeroSelfPlay()
{
    ZeroSelfPlayMaster master(Configure::NumThread);
    master.run();
}

void runZeroOptimization()
{
    ZeroOptimizationMaster master(Configure::NumThread);
    master.run();
}

int main(int argc, char* argv[])
{
    {
        using namespace arguments;
        initProgramOption();
        initConfigure();

        if (!po.parse(argc, argv)) {
            cerr << po.getErrorMsg() << endl;
            cerr << po.getUsage(argv[0]) << endl;
            exit(1);
        }
        if (gen_file_name.length() != 0) {
            // check overwrite configure file
            ifstream f(gen_file_name.c_str());
            if (f.good()) {
                char ans = '0';
                while (ans != 'y' && ans != 'n') {
                    cerr << gen_file_name << " already exist, do you want to overwrite it? [y/n]" << endl;
                    cin >> ans;
                }
                if (ans == 'y') { cerr << "overwrite " << gen_file_name << endl; }
                if (ans == 'n') {
                    cerr << "didn't overwrite " << gen_file_name << endl;
                    f.close();
                    return 0;
                }
            }
            f.close();

            // write configure file
            ofstream fout(gen_file_name.c_str());
            if (!fout) {
                cerr << "unable to open file " << gen_file_name << " to write." << endl;
                exit(1);
            }
            fout << cl.getDefaultConfigure() << endl;
            fout.close();
            return 0;
        }
        loadConfigure(conf_file, conf_str);
        Logger::initialize();
        if (!checkAndInitConfigure()) {
            exit(1);
        }
        reset();
    }
    {
        initializeStatic();
        if (arguments::mode == "gtp") {
            runGTP();
        } else if (arguments::mode == "jlmcts") {
            runJLMCTS();
        } else if (arguments::mode == "stat") {
            CERR() << "size: " << endl;
#define SHOWSIZE(x) CERR() << #x << ": " << sizeof(x) << endl;
            SHOWSIZE(MCTS);
            SHOWSIZE(pattern33::Pattern33Entry);
            SHOWSIZE(WeichiMove);
            SHOWSIZE(WeichiThreadState);
            SHOWSIZE(WeichiLifeDeathHandler);
            SHOWSIZE(WeichiUctNode);
            SHOWSIZE(TTentry);
            SHOWSIZE(WeichiBitBoard);
            SHOWSIZE(WeichiClosedArea);
            SHOWSIZE(BoardStatus);
            SHOWSIZE(CandidateList);
            SHOWSIZE(WeichiGrid);
            SHOWSIZE(WeichiBlock);
            SHOWSIZE(BoundingBox);
            SHOWSIZE(WeichiDragon);
            SHOWSIZE(WeichiDragonHandler);
            SHOWSIZE(WeichiRZoneDataTableEntry);
            SHOWSIZE(WeichiLifeDeathHandler);
            SHOWSIZE(WeichiMoveFeatureHandler);
            SHOWSIZE(WeichiLocalSearchHandler);
            exit(0);
        } else if (arguments::mode == "pattern") {
            void pattern_test();
            pattern_test();
        } else if (arguments::mode == "perf") {
            runPerformanceTest();
        } else if (arguments::mode == "cnnworker") {
            runAsCNNWorker();
        } else if (arguments::mode == "cnnserver") {
            runAsCNNServer();
        } else if (arguments::mode == "train") {
            runTrain();
        } else if (arguments::mode == "proxy") {
            runProxy();
        } else if (arguments::mode == "rldcnn") {
            runRLDCNN();
        } else if (arguments::mode == "tcaffe") {
            runCaffeTrain();
        } else if (arguments::mode == "gensgf_server") {
            runGenSgfServer();
        } else if (arguments::mode == "gensgf_worker") {
            runGenSgfWorker();
        } else if (arguments::mode == "sb_server") {
            runSBServer();
        } else if (arguments::mode == "sb_worker") {
            runSBWorker();
        } else if (arguments::mode == "zero_server") {
            runZeroServer();
        } else if (arguments::mode == "zero_selfplay") {
            runZeroSelfPlay();
        } else if (arguments::mode == "zero_optimization") {
            runZeroOptimization();
        } else if (arguments::mode == "tnetfight") {
            runTNetFight();
        } else if (arguments::mode == "tsumego_solver") {
            runTsumegoSolver();
        } else {
            CERR() << "Unknown mode \"" + arguments::mode + "\"" << endl;
            return 1;
        }
    }
    return 0;
}

namespace arguments {
void initProgramOption()
{
    po.regOption("gen", gen_file_name, "generate default configure file");
    po.regOption("mode", mode, "specify running mode (gtp or jlmcts)");
    po.regOption("conf_file", conf_file, "specify configure file (load first)");
    po.regOption("conf_str", conf_str, "specify configure string (overwrite conf_file)");
}

void initConfigure()
{
    MCTS::setConfigureOptions(cl);
    WeichiConfigure::setConfigureOptions(cl);
}

void loadConfigure(const string& conf_file, const string& conf_str)
{
    if (conf_file.length() != 0) {
        if (!cl.loadConfigureFromFile(conf_file)) {
            cerr << "Loading from file: " << cl.getErrorMsg() << endl;
            exit(1);
        }
    }
    if (conf_str.length() != 0) {
        if (!cl.loadConfigureFromString(conf_str)) {
            cerr << "Loading from string: " << cl.getErrorMsg() << endl;
            exit(1);
        }
    }
}

bool checkAndInitConfigure()
{
    // play game information
    CERR() << "===== for play game setting =====" << endl;
    CERR() << "Total use " << Configure::NumThread << " thread(s), MAX_PAGE: " << Configure::MaxPageNum << "." << endl;
    CERR() << "Pondering control: " << ToString(Configure::Pondering)
           << ", Board size: " << WeichiConfigure::BoardSize << ", komi: " << WeichiConfigure::komi;
    CERR() << ", Problem Rotation: " << WeichiConfigure::problem_rotation << endl;
    if (WeichiConfigure::use_dynamic_komi) {
        CERR() << "(use dynamic komi)";
    } else {
        CERR() << "(disable dynamic komi)";
    }
    CERR() << ", superko_rule: \"" << WeichiConfigure::superko_rule << "\"";
    CERR() << ", counting_rule: \"" << WeichiConfigure::counting_rule << "\"." << endl;
    if (WeichiConfigure::counting_rule != "Japanese" && WeichiConfigure::counting_rule != "Chinese") {
        CERR() << "counting_rule should be \"Japanese\" or \"Chinese\"" << endl;
        return false;
    }
    if (WeichiConfigure::DynamicTimeControl && Configure::SimCtrl == Configure::SIMCTRL_TIME) {
        CERR() << "use \"dynamic time\" control with parameter "
               << WeichiConfigure::TimeControlInitialStep << ", "
               << WeichiConfigure::TimeControlRemainStep << ", "
               << WeichiConfigure::TimeControlParameter << "." << endl;
    } else if (Configure::SimCtrl == Configure::SIMCTRL_COUNT) {
        CERR() << "use \"count\" control with simulation count: " << Configure::SimulationCountLimit << "." << endl;
    } else if (Configure::SimCtrl == Configure::SIMCTRL_TIME) {
        CERR() << "use \"time\" control with time: " << Configure::SimulationTimeLimit << "(s)." << endl;
    } else {
        CERR() << "unknown control." << endl;
    }
    if (WeichiConfigure::EarlyPass) {
        CERR() << "use \"early pass\" with parameter "
               << "win_rate: " << WeichiConfigure::EarlyPassWinrateThreshold
               << ", count: " << WeichiConfigure::EarlyPassCountThreshold << "." << endl;
    } else {
        CERR() << "disable \"early pass\"." << endl;
    }
    if (WeichiConfigure::EarlyAbort) {
        CERR() << "use \"early abort\" with parameter count: " << WeichiConfigure::EarlyAbortCountThreshold << "." << endl;
    } else {
        CERR() << "disable \"early abort\"." << endl;
    }
    if (WeichiConfigure::cnn_policy_output_power_softmax) {
        CERR() << "Enable \"CNN Policy Output Power Softmax\"." << endl;
    } else {
        CERR() << "disable \"CNN Policy Output Power Softmax\"." << endl;
    }
    if (WeichiConfigure::vn_turn_opposite) {
        CERR() << "Enable \"VN turn opposite\"." << endl;
    } else {
        CERR() << "disable \"VN turn opposite\"." << endl;
    }
    CERR() << "Resign threshold: " << WeichiConfigure::ResignWinrateThreshold << "." << endl;
    if (WeichiConfigure::use_board_transposition_table) {
        CERR() << "Enable \"Board Transposition Table\"." << endl;
    } else {
        CERR() << "Disable \"Board Transposition Table\"." << endl;
    }
    if (WeichiConfigure::use_rzone_transposition_table) {
        CERR() << "Enable \"RZone Transposition Table\"." << endl;
    } else {
        CERR() << "Disable \"RZone Transposition Table\"." << endl;
    }
    if (WeichiConfigure::mcts_use_solver) {
        CERR() << "Enable \"MCTS solver\"." << endl;
    } else {
        CERR() << "Disable \"MCTS solver\"." << endl;
    }
    if (WeichiConfigure::mcts_use_disproof) {
        CERR() << "Enable \"MCTS disproof\"." << endl;
    } else {
        CERR() << "Disable \"MCTS disproof\"." << endl;
    }
    if (WeichiConfigure::allow_proved_move_outside_rzone) {
        CERR() << "Enable \"Allow Proved move outside rzone\"." << endl;
    } else {
        CERR() << "Disable \"Allow Proved move outside rzone\"." << endl;
    }
    WeichiGlobalInfo::getTreeInfo().m_winColor = Color(WeichiConfigure::win_color);
    CERR() << "Win Color is \"" << toString(WeichiGlobalInfo::getTreeInfo().m_winColor) << "\"" << endl;
    CERR() << "Search Goal is \"" << getWeichiSearchGoalString(WeichiSearchGoal(WeichiConfigure::search_goal)) << "\"" << endl;
    if (WeichiConfigure::use_rzone) {
        CERR() << "Enable \"Relevance-Zone\"." << endl;
    } else {
        CERR() << "Disable \"Relevance-Zone\"." << endl;
    }
    if (WeichiConfigure::use_immediate_win) {
        CERR() << "Enable \"Go Immediate Win\"." << endl;
    } else {
        CERR() << "Disable \"Go Immediate Win\"." << endl;
    }
    if (WeichiConfigure::use_early_life) {
        CERR() << "Enable \"Go Early Life\"." << endl;
    } else {
        CERR() << "Disable \"Go Early Life\"." << endl;
    }
    if (WeichiConfigure::use_pattern_eye) {
        CERR() << "Enable \"Go Pattern Eye\"." << endl;
    } else {
        CERR() << "Disable \"Go Pattern Eye\"." << endl;
    }
    if (WeichiConfigure::use_potential_rzone) {
        CERR() << "Enable \"potential rzone\"." << endl;
    } else {
        CERR() << "Disable \"potential rzone\"." << endl;
    }
    if (WeichiConfigure::black_ignore_ko) {
        CERR() << "Enable \"black_ignore_ko\", it means black can not use ko to win." << endl;
    } else {
        CERR() << "Disable \"black ignore_ko\"." << endl;
    }
    if (WeichiConfigure::white_ignore_ko) {
        CERR() << "Enable \"white_ignore_ko\", it means white can not use ko to win." << endl;
    } else {
        CERR() << "Disable \"white ignore_ko\"." << endl;
    }
    if (WeichiConfigure::mcts_use_bv) {
        CERR() << "Enable \"MCTS USE BV\"." << endl;
    } else {
        CERR() << "Disable \"MCTS USE BV\"." << endl;
    }
    if (WeichiConfigure::mcts_use_bv_zone) {
        CERR() << "Enable \"MCTS USE BV to restric search area\"." << endl;
    } else {
        CERR() << "Disable \"MCTS USE BV Zone\"." << endl;
    }
    CERR() << "RZone TT phase 1 size:\t" << WeichiConfigure::rzone_tt_phase1_size << endl;
    CERR() << "RZone TT expansion threshold:\t" << WeichiConfigure::rzone_tt_expansion_threshold << endl;

    CERR() << endl;

    // DCNN net setting
    CERR() << "===== for DCNN net setting =====" << endl;
#ifdef USE_CAFFE
    if (!WeichiConfigure::EnableDcnnMessage) {
        CERR() << "disable dcnn log message." << endl;
        FLAGS_minloglevel = 2;
    } else {
        CERR() << "enable dcnn log message." << endl;
    }
#endif

    // check num thread
    uint minThreadNum = 0;
    uint maxThreadNum = std::min(static_cast<uint>(Configure::MaxPageNum) - 1, static_cast<uint>(MAX_NUM_THREADS));
    vector<string> vParamString = splitToVector(WeichiConfigure::dcnn_net, ':');
    for (uint i = 0; i < vParamString.size(); i++) {
        vector<string> vArgument = splitToVector(vParamString[i], '|');
        if (vArgument.size() < 6) { continue; }
        string sThreadBindGpu = vArgument[5];
        minThreadNum = (sThreadBindGpu.size() > minThreadNum) ? static_cast<uint>(sThreadBindGpu.size()) : minThreadNum;
    }
    WeichiConfigure::TotalGpuNum = minThreadNum;
    if (WeichiConfigure::TotalGpuNum > 0) { WeichiConfigure::TotalGpuNum--; }

    if (Configure::NumThread < minThreadNum || Configure::NumThread > maxThreadNum) {
        CERR() << endl;
        CERR() << "Error: num thread should be between min=" << minThreadNum << " and max=" << maxThreadNum << "." << endl;
        CERR() << "note: min = 1 + sl + cache + bv," << endl;
        CERR() << "      max = min(max page num - 1, MAX_NUM_THREADS)." << endl;
        return false;
    }

    CERR() << endl;
    return true;
}

void printDefaultConfigure(ostream& os)
{
    os << cl.getDefaultConfigure() << endl;
}

void reset()
{
    po.reset();
    cl.reset();
}

} // namespace arguments 
