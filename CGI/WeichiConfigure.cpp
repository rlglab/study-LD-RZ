#include "WeichiConfigure.h"
using namespace std;

namespace WeichiConfigure {
// Go option
unsigned long long CPUMask = 0x0;
bool ResignWhenCloseGame = false;
float ResignWinrateThreshold = -0.8f;
int ResignCountThreshold = 0;
bool EarlyPass = true;
float EarlyPassWinrateThreshold = 0.9f;
int EarlyPassCountThreshold = 1000;
bool EarlyAbort = true;
int EarlyAbortCountThreshold = 1000;
float EarlyAbortTimeRatio = 0.3f;
bool CaptureDeadStones = false;
bool ResetSeedInBackup = false;
int NumSimToEnsureTerritory = 5000;
int GenMovePolicy = 0;
int CacheFilterSize = 5;
int CacheStrideSize = 2;

// Go Display option
bool ShowBoardUpsideDown = false;
int ShowBoardBestSequenceLimit = 10;
bool ShowPlayoutPolicyStatistic = false;
bool EnableDcnnMessage = false;
string sGtpProgramName = "CGI Go Intelligence";
string sGtpProgramVersion = "4.0";

// Go Database option
string db_dir = "database/";
string dcnn_dir = "database/dcnn/";
string pattern_dir = "pattern/";
bool EnableOpeningBook = false;
bool alwaysBelieveOpeningBook = true;
string opening_book_control = "WINRATE";
double predictedSimulationCount = 4000.0;

// Go Develop Tool option
bool EnableDevelopmentGoguiCommand = true;
string DevelopmentCommand = "";
float SureTerrityThreshold = 0.8f;
float PossibleTerrityThreshold = 0.6f;
float CriticalityThreshold = 0.2f;
bool winratedropSaveSgf = false;
float SuperiorThreshold = 0.65f;
float InferiorThreshold = 0.45f;
unsigned int dcnn_default_gpu_device = 0;
string dcnn_train_gpu_list = "0123";

// Go Train RLDCNN option
int num_rl_game = 256;
int num_rl_start_iteration = 1;
int num_rl_iteration = 20000;
int num_add_rl_to_opp_pool = 100;
bool rl_isUseOpenning = false;
string rl_openning_dir = "";
int rl_openningMove = 100;
string rldcnn_description = "";
string rldcnn_opp_pool = "rl_dcnn_opp_pool_19.txt";
bool rldcnn_softmax_opp = true;
string rldcnn_solver_positive = "RL_Solver_Positive.prototxt";
string rldcnn_solver_negative = "RL_Solver_Negative.prototxt";
string dcnn_RL_ourNet = "";
string dcnn_RL_oppNet = "";
string dcnn_BV_VNNet = "BV_VN|0|G50|BV_VNML_G50_Go19.prototxt|BV_VNML_G50_Go19.caffemodel     |0";
bool rl_isUseVNBaseline = false;
string sDCNNRLDescription = "Go Train RLDCNN";

// Go Train Caffe DCNN option
string caffe_dcnn_net = "VN|1|G50|||";
string caffe_dcnn_solver = "T_VN_G50_Go19_Solver.prototxt";
string caffe_train_dir = "";
string caffe_test_dir = "";
bool caffe_train_recursively = true;
int caffe_komi_vn_step = 1;
bool caffe_sl_only_win_move = false;
bool caffe_use_opening = false;

// Train Caffe2 DCNN Setting
int display = 1000;
int snapshot = 10000;
string snapshot_prefix = "";
string NFS_path = "";
string run_id = "";

// Go Zero option
int zero_server_port = 15505;
string zero_train_directory = "";
int zero_start_iteration = 1;
int zero_end_iteration = 10000;
int zero_num_game = 10000;
int zero_num_simulation = 800;
float zero_threshold_ratio = 0.0f;
bool zero_training_7x7_killall = false;
bool zero_root_node_noise = true;
float zero_noise_alpha = 0.03f;
float zero_noise_epsilon = 0.25f;
float zero_resign_threshold = -0.8f;
float zero_disable_resign_prob = 0.1f;
float zero_selfplay_model_pool_threshold = 0.4f;
string zero_selfplay_machine_name = "";
string zero_selfplay_black_net = "";
string zero_selfplay_white_net = "";
string zero_selfplay_openings_file = "";
int zero_replay_buffer_size = 50;
float zero_optimization_learning_rate = 0.02;
int zero_optimization_num_worker = 8;
int zero_optimization_sgf_max_iteration = 1;
bool zero_optimization_quiet = false;
string zero_optimization_net = "";
string zero_optimization_solver = "";
int zero_optimization_snapshot_iter = 500;
string zero_optimization_sgf_directory = "";
string zero_optimization_save_name_prefix = "";
int zero_evaluator_num_game = 250;
float zero_evaluator_pass_threshold = 0.5f;

// Go Generate Sgf option
string gensgf_command = "";
string gensgf_filename_prefix = "";
string gensgf_dcnn_sl_net = "";
string gensgf_dcnn_rl_net = "";
string gensgf_opening_directory = "";

// Go Log option
bool DoSgfTreeDetailLog = false;
double LogTreeRootThresholdRatio = 0.01;
bool DoDCNNCacheLog = false;
bool DoPlayoutCacheLog = false;
string log_dir = "log/";
string log_postfix = "";
bool DoLog = false;

// Go Dynamic Time Control option
bool DynamicTimeControl = false;
float TimeControlMainTime = 900.0f;
float TimeControlByoYomiTime = 0.0f;
int TimeControlByoYomiStones = 0;
float TimeControlInitialStep = 70.0f;
float TimeControlRemainStep = 25.0f;
float TimeControlParameter = 1.4f;

// Go Rule option
unsigned int BoardSize = 19;
float komi = 7.5f;
string superko_rule = "positional";
string counting_rule = "Chinese";
bool check_pass_superko = false;

// Go UCT option
bool use_dynamic_komi = false;
unsigned int dkomi_start_step = 20; // 10~80
int dkomi_4komi = 20;               // 10~40
int dkomi_minus = 20;               //-40~40
float dkomi_slop = 10;              // 8~25
float dkomi_shift = 0.6f;           // 0.4~0.7
float dkomi_winrate_weight = 1;     // 0~1
bool use_tree_cache = true;
bool use_virtual_loss = true;
unsigned int virtual_loss_count = 1;
float ReuseThreshold = 200.0f;
double TerritoryThreshold = 0.6;
bool mcts_use_solver = false;
bool mcts_use_disproof = false;
bool use_board_transposition_table = false;
bool use_rzone_transposition_table = false;
bool use_rzone_linear_transposition_table = false;
bool use_rzone_tt_size_heuristic = false;
bool use_rzone_tt_right_first = false;
bool use_rzone_tt_skip_on_id_diff = true;
bool use_rzone_tt_skip_larger_pattern = false;
int rzone_tt_phase1_size = 1000;
float rzone_tt_expansion_threshold = 1.1f;
bool tt_optimize_for_single_ko = false;
bool check_zone_tt_ghi = true;
bool enable_pass_by_playout = true;
bool cnn_policy_output_power_softmax = false;

// Go DCNN option
bool dcnn_use_ftl = false;
bool dcnn_ftl_remaining_move_num = false;
bool vn_turn_opposite = false;
bool dcnn_relaxed_child_generation = true;
bool dcnn_use_forced_komi = false;
float dcnn_forced_komi_value = 0.0f;
int dcnn_default_batch_size = 16;
int dcnn_max_queue_size = 32;
string dcnn_net = "";
float DCNNSLSoftMaxTemperature = 1;
string sDCNNGroupDescription = "Go DCNN (net format: net(SL/BV/VN/RL) | random(0~8) | feature(F16/F17/G49/G50) | prototxt | model | thread_bind_gpu)";

// JobLevelMCTS option
string Path;
string Ignore;

// MCTS argument
float mcts_puct_bias = 1.5f;
const bool mcts_use_ucb = false;
const bool mcts_use_puct = true;
const bool mcts_use_rave = false;
const bool mcts_use_progressive_bias = false;
const bool mcts_use_progressive_widening = false;
const bool mcts_reuse_MM_playout = false;
bool do_mcts_sequential_search = false;
const double init_NodeCount = 5;

// MCTS UCT pattern feature
const bool mcts_use_mm = false;
const bool mcts_use_lfr = false;
const bool mcts_use_cnn = false;

// some data structure argument
const bool use_closed_area = true;

// playout argument
const bool use_playout = (use_probability_playout | use_rule_based_playout);
const bool use_probability_playout = false;
const bool use_rule_based_playout = false;

const bool sim_kill_suicide = true;
const bool sim_reply_save_1lib = true;
const bool sim_reply_kill_2lib = true;
const bool sim_reply_save_2lib = true;
const bool sim_reply_nakade = true;
const bool sim_reply_defense_approach_lib = true;
const bool sim_fight_ko = true;
const bool sim_reply_good_pattern = true;
const bool sim_contiguous = false;
const bool sim_345point_semeai = false;

// tsumego argument
string problem_dir = "";
string result_dir = "";
string problem_filter = "";
int win_color = 0;
int search_goal = 0;
bool use_rzone = false;
bool check_potential_ghi_for_rzone = false;
bool use_immediate_win = false;
bool use_early_life = false;
bool use_pattern_eye = false;
bool use_potential_rzone = false;
bool use_opp1stone_suicide_likepass = true;
bool mcts_use_bv = false;
bool mcts_use_bv_zone = false;
bool update_rzone_in_opponent_turn = true;
float win_vn_threshold = 0.9f;
float bv_threshold = 0.8f;
int potential_rzone_count_limit = 100;
float potential_rzone_goodchild_threshold = 0.5f;
bool use_dead_shape_detection = false;
bool use_independent_territory_detection = false;
bool only_find_winner = true;
bool use_opponent_border_rzone = false;
bool ignore_benson = true;
bool allow_proved_move_outside_rzone = false;
int problem_rotation = 0;
bool black_ignore_ko = false;
bool white_ignore_ko = false;
bool use_editor_tree_format = true;

// other argument
unsigned int TotalGrids = BoardSize * BoardSize;
int RandomizeRaveFequency = 20;
int TotalGpuNum;

bool LiveGfx = false;
bool DrawStrong = true;
bool DrawWeak = true;
bool DrawConnector = true;
bool DrawCut = true;
bool DrawBlack = true;
bool DrawWhite = true;
bool DrawDragon = true;
bool DrawClosedAreaBlack = true;
bool DrawClosedAreaWhite = true;

string edge_dir = "EDGE/Y_m_d_H_i_s/";
string sDynamicTimeControl = "Go Dynamic Time Control (see link: https://docs.google.com/spreadsheets/d/1Th-Pt9v68nY4mFnlPUShjmazCcYA1xRIu_ebDRI3KZc/edit#gid=781522963 )";

const string BV_OUTPUT_NAME = "flatten_territory";
const string SL_OUTPUT_NAME = "softmax";
const string VN_OUTPUT_NAME = "tanh";
const string GL_OUTPUT_NAME = "softmax_gl";
const string EYE_OUTPUT_NAME = "flatten_eye";
const string DRAGON_CONNECT_OUTPUT_NAME = "Connect";
const string SL_LABEL_NAME = "label_sl";
const string SL_DISTRIBUTION_LABEL_NAME = "label_sl_distribution";
const string BV_LABEL_NAME = "label_bv";
const string VN_LABEL_NAME = "label_vn";
const string EYE_LABEL_NAME = "label_eye";
const string DRAGON_CONNECT_LABEL_NAME = "label_connect";
const string GL_LABEL_NAME = "label_gl";

void setConfigureOptions(ConfigureLoader& cl)
{
    // Go option
    cl.addOption("CPU_MASK", CPUMask, ToString(CPUMask), "specify which CPU(s) can run program", "Go");
    cl.addOption("RESIGN_WHEN_CLOSE_GAME", ResignWhenCloseGame, ToString(ResignWhenCloseGame), "", "Go");
    cl.addOption("RESIGN_WINRATE_THRESHOLD", ResignWinrateThreshold, ToString(ResignWinrateThreshold), "0.05 for computer vs computer, 0.2 for computer vs human", "Go");
    cl.addOption("RESIGN_COUNT_THRESHOLD", ResignCountThreshold, ToString(ResignCountThreshold), "count threshold for resign", "Go");
    cl.addOption("EARLY_PASS", EarlyPass, ToString(EarlyPass), "AI passes when winning, useful for playing with human, see link: https://docs.google.com/document/d/1LfyY4Q-jCSEKfed5HQFzR-67IpcmQroIZCtjhuyVAsY/edit", "Go");
    cl.addOption("EARLY_PASS_WINRATE_THRESHOLD", EarlyPassWinrateThreshold, ToString(EarlyPassWinrateThreshold), "if winrate and count is sufficient, start considering early pass", "Go");
    cl.addOption("EARLY_PASS_COUNT_THRESHOLD", EarlyPassCountThreshold, ToString(EarlyPassCountThreshold), "if winrate and count is sufficient, start considering early pass", "Go");
    cl.addOption("EARLY_ABORT", EarlyAbort, ToString(EarlyAbort), "early abort if move cannot change anymore", "Go");
    cl.addOption("EARLY_ABORT_COUNT_THRESHOLD", EarlyAbortCountThreshold, ToString(EarlyAbortCountThreshold), "start checking early abort if count reaches this value", "Go");
    cl.addOption("EARLY_ABORT_TIME_RATIO", EarlyAbortTimeRatio, ToString(EarlyAbortTimeRatio), "start checking early abort if (time/think time) reaches this value", "Go");
    cl.addOption("CAPTURE_DEAD_STONES", CaptureDeadStones, ToString(CaptureDeadStones), "capture all dead stones", "Go");
    cl.addOption("RESET_SEED", ResetSeedInBackup, ToString(ResetSeedInBackup), "reset seed in State::backup", "Go");
    cl.addOption("EXTRA_SIM_FOR_TERRITORY", NumSimToEnsureTerritory, ToString(NumSimToEnsureTerritory), "number of extra simulation to ensure territory", "Go");
    cl.addOption("GENMOVE_POLICY", GenMovePolicy, ToString(GenMovePolicy), "0:UCT/1:Playout/2:DCNN Best/3:DCNN SoftMax/4:DCNN VN", "Go");
    cl.addOption("CACHE_FILTER_SIZE", CacheFilterSize, ToString(CacheFilterSize), "region cache setting", "Go");
    cl.addOption("CACHE_STRIDE_SIZE", CacheStrideSize, ToString(CacheStrideSize), "region cache setting", "Go");

    // Go Display option
    cl.addOption("SHOW_BOARD_UPSIDE_DOWN", ShowBoardUpsideDown, ToString(ShowBoardUpsideDown), "show board upside down", "Go Display");
    cl.addOption("SHOW_BOARD_BEST_SEQUENCE_LIMIT", ShowBoardBestSequenceLimit, ToString(ShowBoardBestSequenceLimit), "limit the max move sequence for detail board", "Go Display");
    cl.addOption("SHOW_PLAYOUT_STATISTIC", ShowPlayoutPolicyStatistic, ToString(ShowPlayoutPolicyStatistic), "show playout policy statistic", "Go Display");
    cl.addOption("ENABLE_DCNN_MESSAGE", EnableDcnnMessage, ToString(EnableDcnnMessage), "enable DCNN message", "Go Display");
    cl.addOption("GTP_PROGRAM_NAME", sGtpProgramName, ToString(sGtpProgramName), "gtp program name", "Go Display");
    cl.addOption("GTP_PROGRAM_VERSION", sGtpProgramVersion, ToString(sGtpProgramVersion), "gtp program version", "Go Display");

    // Go Database option
    cl.addOption("DATABASE_DIR", db_dir, ToString(db_dir), "directory of all database", "Go Database");
    cl.addOption("CNN_NET_DIR", dcnn_dir, ToString(dcnn_dir), "directory of all CNN net", "Go Database");
    cl.addOption("PATTERN_DIR", pattern_dir, ToString(pattern_dir), "directory of pattern", "Go Database");
    cl.addOption("ENABLE_OPENING_BOOK", EnableOpeningBook, ToString(EnableOpeningBook), "enable database opening book", "Go Database");
    cl.addOption("ALWAYS_BELIEVE_OPENINGBOOK", alwaysBelieveOpeningBook, ToString(alwaysBelieveOpeningBook), "enable believe opening book according predicted simulation count", "Go Database");
    cl.addOption("OPENING_BOOK_CONTROL", opening_book_control, ToString(opening_book_control), " specify control method for opening book selection (COUNT / WINRATE)", "Go Database");
    cl.addOption("OPENING_PREDICTED_SIMCOUNT", predictedSimulationCount, ToString(predictedSimulationCount), " specify predicted simulation counts in MCTS per second", "Go Database");

    // Go Develop Tool option
    cl.addOption("ENABLE_DEVELOPMENT_GOGUI_COMMAND", EnableDevelopmentGoguiCommand, ToString(EnableDevelopmentGoguiCommand), "for development gogui command", "Go Develop Tool");
    cl.addOption("DEVELOPMENT_COMMAND", DevelopmentCommand, ToString(DevelopmentCommand), "for development command", "Go Develop Tool");
    cl.addOption("SURE_TERRITORY_THRESHOLD", SureTerrityThreshold, ToString(SureTerrityThreshold), "for SHOW_BOARD_DETAIL_AFTER_GENMOVE setting", "Go Develop Tool");
    cl.addOption("POSSIBLE_TERRITORY_THRESHOLD", PossibleTerrityThreshold, ToString(PossibleTerrityThreshold), "for SHOW_BOARD_DETAIL_AFTER_GENMOVE setting", "Go Develop Tool");
    cl.addOption("CRITICALITY_THRESHOLD", CriticalityThreshold, ToString(CriticalityThreshold), "for SHOW_BOARD_DETAIL_AFTER_GENMOVE setting", "Go Develop Tool");
    cl.addOption("WINRATE_DROP_SAVE_SGF", winratedropSaveSgf, ToString(winratedropSaveSgf), "auto save sgf from specific higher winrate to specific lower winrate", "Go Develop Tool");
    cl.addOption("SUPERIOR_THRESHOLD", SuperiorThreshold, ToString(SuperiorThreshold), "for WINRATE_DROP_SAVE_SGF setting", "Go Develop Tool");
    cl.addOption("INFERIOR_THRESHOLD", InferiorThreshold, ToString(InferiorThreshold), "for WINRATE_DROP_SAVE_SGF setting", "Go Develop Tool");
    cl.addOption("DCNN_DEFAULT_GPU_DEVICE", dcnn_default_gpu_device, ToString(dcnn_default_gpu_device), "DCNN default use gpu device (for train mode and gogui command use)", "Go Develop Tool");
    cl.addOption("DCNN_TRAIN_GPU_LIST", dcnn_train_gpu_list, ToString(dcnn_train_gpu_list), "DCNN train mode gpu list", "Go Develop Tool");

    // Go Train RLDCNN option
    cl.addOption("NUM_RL_GAME", num_rl_game, ToString(num_rl_game), "num rl game", sDCNNRLDescription);
    cl.addOption("NUM_RL_START_ITERATION", num_rl_start_iteration, ToString(num_rl_start_iteration), "num rl start iteration", sDCNNRLDescription);
    cl.addOption("NUM_RL_ITERATION", num_rl_iteration, ToString(num_rl_iteration), "num rl iteration", "Go Train RLDCNN");
    cl.addOption("NUM_RL_TO_OPP_POOL", num_add_rl_to_opp_pool, ToString(num_add_rl_to_opp_pool), "num rl to opponent pool", sDCNNRLDescription);
    cl.addOption("IS_USED_OPENNING", rl_isUseOpenning, ToString(rl_isUseOpenning), "train with openning or not", sDCNNRLDescription);
    cl.addOption("OPENNING_DIRECTORY", rl_openning_dir, ToString(rl_openning_dir), "rl openning directory", sDCNNRLDescription);
    cl.addOption("OPENNING_MOVE_NUMBER", rl_openningMove, ToString(rl_openningMove), "random choose a position from the rl openning", sDCNNRLDescription);
    cl.addOption("RLDCNN_DESCRIPTION", rldcnn_description, ToString(rldcnn_description), "rl dcnn directory description", sDCNNRLDescription);
    cl.addOption("RLDCNN_OPP_POOL", rldcnn_opp_pool, ToString(rldcnn_opp_pool), "rl dcnn opponent pool file", sDCNNRLDescription);
    cl.addOption("RLDCNN_SOFT_MAX_OPP", rldcnn_softmax_opp, ToString(rldcnn_softmax_opp), "", sDCNNRLDescription);
    cl.addOption("RLDCNN_SOLVER", rldcnn_solver_positive, ToString(rldcnn_solver_positive), "our rldcnn solver file", sDCNNRLDescription);
    cl.addOption("RLDCNN_SOLVER_NEGATIVE", rldcnn_solver_negative, ToString(rldcnn_solver_negative), "our rldcnn solver file negative", sDCNNRLDescription);
    cl.addOption("RLDCNN_ourNET+", dcnn_RL_ourNet, "SL|0|G51|SL_G51_Go19.prototxt  |SL_G51_Go19.caffemodel          |", "", sDCNNRLDescription);
    cl.addOption("RLDCNN_oppNET+", dcnn_RL_oppNet, "SL|0|G51|SL_G51_Go19.prototxt  |SL_G51_Go19.caffemodel          |", "", sDCNNRLDescription);
    cl.addOption("RLDCNN_VN_BASELINE", rl_isUseVNBaseline, ToString(rl_isUseVNBaseline), "", sDCNNRLDescription);
    cl.addOption("RLDCNN_BV_VNNET", dcnn_BV_VNNet, ToString(dcnn_BV_VNNet), "", sDCNNRLDescription);

    // Go Train Caffe DCNN option
    cl.addOption("CAFFE_DCNN_NET", caffe_dcnn_net, ToString(caffe_dcnn_net), "caffe net", "Go Train Caffe DCNN");
    cl.addOption("CAFFE_DCNN_SOLVER", caffe_dcnn_solver, ToString(caffe_dcnn_solver), "caffe solver", "Go Train Caffe DCNN");
    cl.addOption("CAFFE_TRAIN_DIR", caffe_train_dir, ToString(caffe_train_dir), "caffe train directory", "Go Train Caffe DCNN");
    cl.addOption("CAFFE_TEST_DIR", caffe_test_dir, ToString(caffe_test_dir), "caffe test directory", "Go Train Caffe DCNN");
    cl.addOption("CAFFE_TRAIN_RECURSIVELY", caffe_train_recursively, ToString(caffe_train_recursively), "caffe train recursively", "Go Train Caffe DCNN");
    cl.addOption("CAFFE_KOMI_VN_STEP", caffe_komi_vn_step, ToString(caffe_komi_vn_step), "caffe komi vn step", "Go Train Caffe DCNN");
    cl.addOption("CAFFE_SL_ONLY_WIN_MOVE", caffe_sl_only_win_move, ToString(caffe_sl_only_win_move), "caffe SL only win move", "Go Train Caffe DCNN");
    cl.addOption("CAFFE_USE_OPENING", caffe_use_opening, ToString(caffe_use_opening), "Train with sgf containing opening moves. Skip opening moves (EV[] value in sgf) when picking a random position", "Go Train Caffe DCNN");

    // Train Caffe2 DCNN Setting
    cl.addOption("DISPLAY", display, ToString(display), "Training display", "Train Caffe2 DCNN Setting");
    cl.addOption("SNAPSHOT", snapshot, ToString(snapshot), "caffe2 snapshot number", "Train Caffe2 DCNN Setting");
    cl.addOption("SNAPSHOT_PREFIX", snapshot_prefix, ToString(snapshot_prefix), "caffe2 snapshot prefix", "Train Caffe2 DCNN Setting");
    cl.addOption("NFS_PATH", NFS_path, ToString(NFS_path), "NFS path for distributed training", "Train Caffe2 DCNN Setting");
    cl.addOption("RUN_ID", run_id, ToString(run_id), "distributed Training job ID", "Train Caffe2 DCNN Setting");

    // Go Zero option
    cl.addOption("ZERO_SERVER_PORT", zero_server_port, ToString(zero_server_port), "", "Go Zero");
    cl.addOption("ZERO_TRAIN_DIRECTORY", zero_train_directory, ToString(zero_train_directory), "", "Go Zero");
    cl.addOption("ZERO_START_ITERATION", zero_start_iteration, ToString(zero_start_iteration), "", "Go Zero");
    cl.addOption("ZERO_END_ITERATION", zero_end_iteration, ToString(zero_end_iteration), "", "Go Zero");
    cl.addOption("ZERO_NUM_GAME", zero_num_game, ToString(zero_num_game), "", "Go Zero");
    cl.addOption("ZERO_NUM_SIMULATION", zero_num_simulation, ToString(zero_num_simulation), "", "Go Zero");
    cl.addOption("ZERO_THRESHOLD_RATIO", zero_threshold_ratio, ToString(zero_threshold_ratio), "", "Go Zero");
    cl.addOption("ZERO_TRAINING_7x7_KILLALL", zero_training_7x7_killall, ToString(zero_training_7x7_killall), "", "Go Zero");
    cl.addOption("ZERO_ROOT_NODE_NOISE", zero_root_node_noise, ToString(zero_root_node_noise), "", "Go Zero");
    cl.addOption("ZERO_NOISE_ALPHA", zero_noise_alpha, ToString(zero_noise_alpha), "", "Go Zero");
    cl.addOption("ZERO_NOISE_EPSILON", zero_noise_epsilon, ToString(zero_noise_epsilon), "", "Go Zero");
    cl.addOption("ZERO_RESIGN_THRESHOLD", zero_resign_threshold, ToString(zero_resign_threshold), "", "Go Zero");
    cl.addOption("ZERO_DISABLE_RESIGN_PROB", zero_disable_resign_prob, ToString(zero_disable_resign_prob), "", "Go Zero");
    cl.addOption("ZERO_SELFPLAY_MODEL_POOL_THRESHOLD", zero_selfplay_model_pool_threshold, ToString(zero_selfplay_model_pool_threshold), "", "Go Zero");
    cl.addOption("ZERO_SELFPLAY_MACHINE_NAME", zero_selfplay_machine_name, ToString(zero_selfplay_machine_name), "", "Go Zero");
    cl.addOption("ZERO_SELFPLAY_BLACK_NET", zero_selfplay_black_net, "NET|1|G51|Zero_B6.prototxt      |                        |0", "", "Go Zero");
    cl.addOption("ZERO_SELFPLAY_WHITE_NET", zero_selfplay_white_net, "NET|1|G51|Zero_B6.prototxt      |                        |0", "", "Go Zero");
    cl.addOption("ZERO_SELFPLAY_OPENINGS_FILE", zero_selfplay_openings_file, "", "", "Go Zero");
    cl.addOption("ZERO_REPLAY_BUFFER_SIZE", zero_replay_buffer_size, ToString(zero_replay_buffer_size), "", "Go Zero");
    cl.addOption("ZERO_OPTIMIZATION_LEARNING_RATE", zero_optimization_learning_rate, ToString(zero_optimization_learning_rate), "", "Go Zero");
    cl.addOption("ZERO_OPTIMIZATION_NUM_WORKER", zero_optimization_num_worker, ToString(zero_optimization_num_worker), "", "Go Zero");
    cl.addOption("ZERO_OPTIMIZATION_SGF_MAX_ITERATION", zero_optimization_sgf_max_iteration, ToString(zero_optimization_sgf_max_iteration), "", "Go Zero");
    cl.addOption("ZERO_OPTIMIZATION_QUIET", zero_optimization_quiet, ToString(zero_optimization_quiet), "", "Go Zero");
    cl.addOption("ZERO_OPTIMIZATION_NET", zero_optimization_net, "NET|1|G51|Zero_B6.prototxt      |                        |0", "", "Go Zero");
    cl.addOption("ZERO_OPTIMIZATION_SOLVER", zero_optimization_solver, "", "", "Go Zero");
    cl.addOption("ZERO_OPTIMIZATION_SNAPSHOT_ITER", zero_optimization_snapshot_iter, ToString(zero_optimization_snapshot_iter), "", "Go Zero");
    cl.addOption("ZERO_OPTIMIZAIOTN_SGF_DIRECTORY", zero_optimization_sgf_directory, "", "", "Go Zero");
    cl.addOption("ZERO_OPTIMIZAIOTN_SAVE_NAME_PREFIX", zero_optimization_save_name_prefix, "", "", "Go Zero");
    cl.addOption("ZERO_EVALUATOR_NUM_GAME", zero_evaluator_num_game, ToString(zero_evaluator_num_game), "", "Go Zero");
    cl.addOption("ZERO_EVALUATOR_PASS_THRESHOLD", zero_evaluator_pass_threshold, ToString(zero_evaluator_pass_threshold), "", "Go Zero");

    // Go Generate Sgf option
    cl.addOption("GENSGF_COMMAND", gensgf_command, ToString(gensgf_command), "command: SL/RL/RL_END", "Go Generate Sgf");
    cl.addOption("GENSGF_FILENAME_PREFIX", gensgf_filename_prefix, ToString(gensgf_filename_prefix), "", "Go Generate Sgf");
    cl.addOption("GENSGF_DCNN_SL_NET+", gensgf_dcnn_sl_net, "SL   |0|G51|SL_G51_Go19_residual_20L_256.prototxt|SL_G51_Go19_residual_20L_256.caffemodel|", "", "Go Generate Sgf");
    cl.addOption("GENSGF_DCNN_SL_NET+", gensgf_dcnn_sl_net, "SL   |0|G51|SL_G51_Go19.prototxt                 |SL_G51_Go19.caffemodel                 |", "", "Go Generate Sgf");
    cl.addOption("GENSGF_DCNN_SL_NET+", gensgf_dcnn_sl_net, "SL   |0|F16|SL_F16_Go19.prototxt                 |SL_F16_Go19.caffemodel                 |", "", "Go Generate Sgf");
    cl.addOption("GENSGF_DCNN_SL_NET+", gensgf_dcnn_sl_net, "SL   |0|G49|SL_G49_Go19.prototxt                 |SL_G49_Go19.caffemodel                 |", "", "Go Generate Sgf");
    cl.addOption("GENSGF_DCNN_RL_NET+", gensgf_dcnn_rl_net, "SL   |0|G51|SL_G51_Go19_residual_20L_256.prototxt|RL_G51_Go19_residual_20L_256.caffemodel|", "", "Go Generate Sgf");
    cl.addOption("GENSGF_DCNN_RL_NET+", gensgf_dcnn_rl_net, "SL   |0|G51|SL_G51_Go19.prototxt                 |RL_G51_Go19.caffemodel                 |", "", "Go Generate Sgf");
    cl.addOption("GENSGF_DCNN_RL_NET+", gensgf_dcnn_rl_net, "SL   |0|G49|SL_G49_Go19.prototxt                 |RL_G49_Go19_iter10000.caffemodel       |", "", "Go Generate Sgf");
    cl.addOption("GENSGF_OPENING_DIRECTORY", gensgf_opening_directory, ToString(gensgf_opening_directory), "", "Go Generate Sgf");

    // Go Log option
    cl.addOption("DO_SGF_TREE_DETAIL_LOG", DoSgfTreeDetailLog, ToString(DoSgfTreeDetailLog), "do sgf tree detail log", "Go Log");
    cl.addOption("LOG_TREE_ROOT_THRESHOLD_RATIO", LogTreeRootThresholdRatio, ToString(LogTreeRootThresholdRatio), "log tree root threshold ratio", "Go Log");
    cl.addOption("DO_DCNN_CACHE_LOG", DoDCNNCacheLog, ToString(DoDCNNCacheLog), "do dcnn cache log", "Go Log");
    cl.addOption("DO_PLAYOUT_CACHE_LOG", DoPlayoutCacheLog, ToString(DoPlayoutCacheLog), "do playout cache log", "Go Log");
    cl.addOption("LOG_DIR", log_dir, ToString(log_dir), "directory to save log", "Go Log");
    cl.addOption("LOG_POSTFIX", log_postfix, ToString(log_postfix), "postfix name of log directory", "Go Log");
    cl.addOption("DO_LOG", DoLog, ToString(DoLog), "do log detail", "Go Log");

    // Go Dynamic Time Control option (see time setting link: https://docs.google.com/spreadsheets/d/1Th-Pt9v68nY4mFnlPUShjmazCcYA1xRIu_ebDRI3KZc/edit#gid=781522963)
    cl.addOption("DYNAMIC_TIME_CONTROL", DynamicTimeControl, ToString(DynamicTimeControl), "", sDynamicTimeControl);
    cl.addOption("TIMECONTROL_MAIN_TIME", TimeControlMainTime, ToString(TimeControlMainTime), "", sDynamicTimeControl);
    cl.addOption("TIMECONTROL_BYO_YOMI_TIME", TimeControlByoYomiTime, ToString(TimeControlByoYomiTime), "", sDynamicTimeControl);
    cl.addOption("TIMECONTROL_BYO_YOMI_STONES", TimeControlByoYomiStones, ToString(TimeControlByoYomiStones), "", sDynamicTimeControl);
    cl.addOption("TIMECONTROL_INITIAL_STEP", TimeControlInitialStep, ToString(TimeControlInitialStep), "28 for 9x9, 54 for 13x13, 70 for 19x19", sDynamicTimeControl);
    cl.addOption("TIMECONTROL_REMAIN_STEP", TimeControlRemainStep, ToString(TimeControlRemainStep), "16 for 9x9, 32 for 13x13, 25 for 19x19", sDynamicTimeControl);
    cl.addOption("TIMECONTROL_PARAMETER", TimeControlParameter, ToString(TimeControlParameter), "1 for 9x9, 1.2 for 13x13, 1.4 for 19x19", sDynamicTimeControl);

    // Go Rule option
    cl.addOption("BOARD_SIZE", BoardSize, ToString(BoardSize), "board size (MAX is 19)", "Go Rule");
    cl.addOption("KOMI", komi, ToString(komi), "7 for 9x9, 7.5 for 13x13 & 19x19", "Go Rule");
    cl.addOption("SUPERKO_RULE", superko_rule, superko_rule, "SuperKoRule (positional/situational/natural situational)", "Go Rule");
    cl.addOption("COUNTING_RULE", counting_rule, counting_rule, "CountingRule (Chinese/Japanese)", "Go Rule");
    cl.addOption("CHECK_PASS_SUPERKO", check_pass_superko, ToString(check_pass_superko), "Check pass for superko", "Go Rule");

    // Go UCT option
    cl.addOption("USE_DYNAMIC_KOMI", use_dynamic_komi, ToString(use_dynamic_komi), "for dynamic komi", "Go UCT");
    cl.addOption("USE_TREE_CACHE", use_tree_cache, ToString(use_tree_cache), "for tree cache", "Go UCT");
    cl.addOption("VIRTUAL_LOSS", use_virtual_loss, ToString(use_virtual_loss), "enable virtual loss feature", "Go UCT");
    cl.addOption("REUSE_THRESHOLD", ReuseThreshold, ToString(ReuseThreshold), "minimal visit count for reusing subtree", "Go UCT");
    cl.addOption("TERRITORY_THRESHOLD", TerritoryThreshold, ToString(TerritoryThreshold), "threshold for territory", "Go UCT");
    cl.addOption("PUCT_BIAS", mcts_puct_bias, ToString(mcts_puct_bias), "PUCT bias", "Go UCT");
    cl.addOption("MCTS_USE_SOLVER", mcts_use_solver, ToString(mcts_use_solver), "mcts use solver", "Go UCT");
    cl.addOption("MCTS_USE_DISPROOF", mcts_use_disproof, ToString(mcts_use_disproof), "mcts use disproof", "Go UCT");
    cl.addOption("USE_BOARD_TRANSPOSITION_TABLE", use_board_transposition_table, ToString(use_board_transposition_table), "use board transposition table", "Go UCT");
    cl.addOption("USE_RZONE_TRANSPOSITION_TABLE", use_rzone_transposition_table, ToString(use_rzone_transposition_table), "use rzone transposition table", "Go UCT");
    cl.addOption("USE_RZONE_LINEAR_TRANSPOSITION_TABLE", use_rzone_linear_transposition_table, ToString(use_rzone_linear_transposition_table), "use rzone linear transposition table", "Go UCT");
    cl.addOption("USE_RZONE_TT_SIZE_HEURISTIC", use_rzone_tt_size_heuristic, ToString(use_rzone_tt_size_heuristic), "enable rzone tt size heuristic", "Go UCT");
    cl.addOption("USE_RZONE_TT_RIGHT_FIRST", use_rzone_tt_right_first, ToString(use_rzone_tt_right_first), "rzone tt search order right first", "Go UCT");
    cl.addOption("USE_RZONE_TT_SKIP_ON_ID_DIFF", use_rzone_tt_skip_on_id_diff, ToString(use_rzone_tt_skip_on_id_diff), "enable rzone tt skip on id diff", "Go UCT");
    cl.addOption("USE_RZONE_TT_SKIP_LARGER_PATTERN", use_rzone_tt_skip_larger_pattern, ToString(use_rzone_tt_skip_larger_pattern), "skip larger zone pattern if it has smaller ones", "Go UCT");
    cl.addOption("RZONE_TT_PHASE1_SIZE", rzone_tt_phase1_size, ToString(rzone_tt_phase1_size), "rzone tt phase 1 size", "Go UCT");
    cl.addOption("RZONE_TT_EXPANSION_THRESHOLD", rzone_tt_expansion_threshold, ToString(rzone_tt_expansion_threshold), "rzone tt expansion threshold", "Go UCT");
    cl.addOption("CHECK_ZONE_TT_GHI", check_zone_tt_ghi, ToString(check_zone_tt_ghi), "true for check GHI problem in zone tt", "Go UCT");
    cl.addOption("ENABLE_PASS_BY_PLAYOUT", enable_pass_by_playout, ToString(enable_pass_by_playout), "generating pass child depending on playout or not", "Go UCT");
    cl.addOption("CNN_POLICY_OUTPUT_POWER_SOFTMAX", cnn_policy_output_power_softmax, ToString(cnn_policy_output_power_softmax), "True for power softmax, false for normal softmax", "Go UCT");
    cl.addOption("DO_MCTS_SEQUENTIAL_SEARCH", do_mcts_sequential_search, ToString(do_mcts_sequential_search), "True for search sequentially, the CPU thread would wait the GPU for forwarding", "Go UCT");

    // Go DCNN option
    cl.addOption("DCNN_USE_FTL", dcnn_use_ftl, ToString(dcnn_use_ftl), "Faster to life option", sDCNNGroupDescription);
    cl.addOption("DCNN_FTL_REMAINING_MOVE_NUM", dcnn_ftl_remaining_move_num, ToString(dcnn_ftl_remaining_move_num), "Faster to life remaining steps", sDCNNGroupDescription);
    cl.addOption("VN_TURN_OPPOSITE", vn_turn_opposite, ToString(vn_turn_opposite), "True for ELF net for opposite vn value", sDCNNGroupDescription);
    cl.addOption("DCNN_RELAXED_CHILD_GENERATION", dcnn_relaxed_child_generation, ToString(dcnn_relaxed_child_generation), "True for generating children normally for zero training", sDCNNGroupDescription);
    cl.addOption("DCNN_USE_FORCED_KOMI", dcnn_use_forced_komi, ToString(dcnn_use_forced_komi), "True for used komi of  dcnn_forced_komi_value", sDCNNGroupDescription);
    cl.addOption("DCNN_FORCED_KOMI_VALUE", dcnn_forced_komi_value, ToString(dcnn_forced_komi_value), "The value is for muiti-labelled vn", sDCNNGroupDescription);
    cl.addOption("DCNN_DEFAULT_BATCH_SIZE", dcnn_default_batch_size, ToString(dcnn_default_batch_size), "default batch size", sDCNNGroupDescription);
    cl.addOption("DCNN_MAX_QUEUE_SIZE", dcnn_max_queue_size, ToString(dcnn_max_queue_size), "max queue size", sDCNNGroupDescription);
    cl.addOption("DCNN_SL_SOFTMAX_TEMPERATURE", DCNNSLSoftMaxTemperature, ToString(DCNNSLSoftMaxTemperature), "for SL softmax temperature", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "NET  |0|F18|Zero_ELF.prototxt                       |Zero_ELF.caffemodel                       |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "SL   |0|G51|SL_G51_Go19_residual_20L_256.prototxt   |SL_G51_Go19_residual_20L_256.caffemodel   |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "SL   |0|G51|SL_G51_Go19_residual_20L_256_v2.prototxt|SL_G51_Go19_residual_20L_256_v2.caffemodel|", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "SL   |0|G51|SL_G51_Go19.prototxt                    |SL_G51_Go19.caffemodel                    |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "SL   |0|F16|SL_F16_Go19.prototxt                    |SL_F16_Go19.caffemodel                    |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "SL   |0|G49|SL_G49_Go19.prototxt                    |SL_G49_Go19.caffemodel                    |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "BV_VN|0|G49|BV_G49_Go19.prototxt                    |BV_G49_Go19.caffemodel                    |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "BV_VN|0|G50|BV_G50_Go19.prototxt                    |BV_G50_Go19.caffemodel                    |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "BV_VN|0|G50|VN_G50_Go19.prototxt                    |VN_G50_Go19.caffemodel                    |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "BV_VN|0|G50|VNML_G50_Go19.prototxt                  |VNML_G50_Go19.caffemodel                  |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "BV_VN|0|G50|BV_VN_G50_Go19.prototxt                 |BV_VN_G50_Go19.caffemodel                 |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "BV_VN|0|G50|BV_VNML_G50_Go19.prototxt               |BV_VNML_G50_Go19.caffemodel               |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "BV_VN|0|G50|BV_VNML_G50_Go19.prototxt               |BV_VNML_G50_Go19_SL_RL.caffemodel         |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "BV_VN|0|G50|BV_VNML_G50_Go19.prototxt               |BV_VNML_G50_Go19_tygem.caffemodel         |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "SL   |0|G51|SL_G51_Go19_residual_20L_256.prototxt   |RL_G51_Go19_residual_20L_256.caffemodel   |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "SL   |0|G51|SL_G51_Go19.prototxt                    |RL_G51_Go19.caffemodel                    |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "SL   |0|G49|SL_G49_Go19.prototxt                    |RL_G49_Go19_iter10000.caffemodel          |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "SL   |0|G49|SL_G49_Go9.prototxt                     |SL_G49_Go9.caffemodel                     |", "", sDCNNGroupDescription);
    cl.addOption("DCNN_NET+", dcnn_net, "|||||", "", sDCNNGroupDescription);

    // LifeDeath option
    cl.addOption("PROBLEM_DIR", problem_dir, ToString(problem_dir), "the dir of the problems", "LifeDeath");
    cl.addOption("RESULT_DIR", result_dir, ToString(result_dir), "the dir of the result", "LifeDeath");
    cl.addOption("PROBLEM_FILTER", problem_filter, ToString(problem_filter), "only run problems in the filter", "LifeDeath");
    cl.addOption("WIN_COLOR", win_color, ToString(win_color), "win color, 0 for unknown, 1 for black, 2 for white", "LifeDeath");
    cl.addOption("SEARCH_GOAL", search_goal, ToString(search_goal), "0 for unknown, 1 for kill, 2 for live, 3 for enough territory", "LifeDeath");
    cl.addOption("USE_RZONE", use_rzone, ToString(use_rzone), "use RZone", "LifeDeath");
    cl.addOption("CHECK_POTENTIAL_GHI_FOR_RZONE", check_potential_ghi_for_rzone, ToString(check_potential_ghi_for_rzone), "check potential ghi for the use of R-zone", "LifeDeath");
    cl.addOption("USE_IMMEDIATE_WIN", use_immediate_win, ToString(use_immediate_win), "use GO knowledge to achieve immediate win", "LifeDeath");
    cl.addOption("USE_EARLY_LIFE", use_early_life, ToString(use_early_life), "True: use GO knowledge to achieve early life; False: use benson", "LifeDeath");
    cl.addOption("USE_PATTERN_EYE", use_pattern_eye, ToString(use_pattern_eye), "True: use pattern life of GO knowledge to achieve early life; False: use benson", "LifeDeath");
    cl.addOption("USE_POTENTIAL_RZONE", use_potential_rzone, ToString(use_potential_rzone), "use potential rzone", "LifeDeath");
    cl.addOption("USE_OPP1STONE_SUICIDE_LIKEPASS", use_opp1stone_suicide_likepass, ToString(use_opp1stone_suicide_likepass), "use opponent's suicide R-zone as pass for pruning", "LifeDeath");
    cl.addOption("MCTS_USE_BV", mcts_use_bv, ToString(mcts_use_bv), "mcts use BV", "LifeDeath");
    cl.addOption("MCTS_USE_BV_ZONE", mcts_use_bv_zone, ToString(mcts_use_bv_zone), "mcts use BV to restrict search area", "LifeDeath");
    cl.addOption("POTENTIAL_RZONE_COUNT_LIMIT", potential_rzone_count_limit, ToString(potential_rzone_count_limit), "potential rzone count limit, often for winning move", "LifeDeath");
    cl.addOption("POTENTIAL_RZONE_GOODCHILD_THRESHOLD", potential_rzone_goodchild_threshold, ToString(potential_rzone_goodchild_threshold), "potential rzone good child threshold", "LifeDeath");
    cl.addOption("WIN_VN_THRESHOLD", win_vn_threshold, ToString(win_vn_threshold), "vn threshold for win", "LifeDeath");
    cl.addOption("BV_THRESHOLD", bv_threshold, ToString(bv_threshold), "bv threshold for recognizing as territory", "LifeDeath");
    cl.addOption("USE_DEAD_SHAPE_DETECTION", use_dead_shape_detection, ToString(use_dead_shape_detection), "use dead shape detection", "LifeDeath");
    cl.addOption("USE_OPPONENT_BORDER_RZONE", use_opponent_border_rzone, ToString(use_opponent_border_rzone), "use opponent border rzone", "LifeDeath");
    cl.addOption("ALLOW_PROVED_MOVE_OUTSIDE_RZONE", allow_proved_move_outside_rzone, ToString(allow_proved_move_outside_rzone), "true for allowing play move outside rzone", "LifeDeath");
    cl.addOption("PROBLEM_ROTATION", problem_rotation, ToString(problem_rotation), "problem for rotations", "LifeDeath");
    cl.addOption("USE_EDITOR_TREE_FORMAT", use_editor_tree_format, ToString(use_editor_tree_format), "output tree in Editor format", "LifeDeath");

    // JobLevelMCTS option
    cl.addOption("PATH", Path, Path, "the preload sgf path", "JobLevelMCTS");
    cl.addOption("IGNORE", Ignore, Ignore, "the ignore moves", "JobLevelMCTS");
}
} // namespace WeichiConfigure
