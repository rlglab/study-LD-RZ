#ifndef WEICHICONFIGURE_H
#define WEICHICONFIGURE_H

#include "ConfigureLoader.h"
#include <fstream>
#include <string>

namespace WeichiConfigure {
// Go option
extern unsigned long long CPUMask;
extern bool ResignWhenCloseGame;
extern float ResignWinrateThreshold;
extern int ResignCountThreshold;
extern bool EarlyPass;
extern float EarlyPassWinrateThreshold;
extern int EarlyPassCountThreshold;
extern bool EarlyAbort;
extern int EarlyAbortCountThreshold;
extern float EarlyAbortTimeRatio;
extern bool CaptureDeadStones;
extern bool ResetSeedInBackup;
extern int NumSimToEnsureTerritory;
extern int GenMovePolicy;
extern int CacheFilterSize;
extern int CacheStrideSize;

// Go Display option
extern bool ShowBoardUpsideDown;
extern int ShowBoardBestSequenceLimit;
extern bool ShowPlayoutPolicyStatistic;
extern bool EnableDcnnMessage;
extern string sGtpProgramName;
extern string sGtpProgramVersion;

// Go Database option
extern std::string db_dir;
extern std::string dcnn_dir;
extern std::string pattern_dir;
extern bool EnableOpeningBook;
extern bool alwaysBelieveOpeningBook;
extern std::string opening_book_control;
extern double predictedSimulationCount;

// Go Develop Tool option
extern bool EnableDevelopmentGoguiCommand;
extern string DevelopmentCommand;
extern float SureTerrityThreshold;
extern float PossibleTerrityThreshold;
extern float CriticalityThreshold;
extern bool winratedropSaveSgf;
extern float SuperiorThreshold;
extern float InferiorThreshold;
extern unsigned int dcnn_default_gpu_device;
extern string dcnn_train_gpu_list;

// Go Train RLDCNN option
extern int num_rl_game;
extern int num_rl_start_iteration;
extern int num_rl_iteration;
extern int num_add_rl_to_opp_pool;
extern bool rl_isUseOpenning;
extern string rl_openning_dir;
extern int rl_openningMove;
extern string rldcnn_description;
extern string rldcnn_opp_pool;
extern bool rldcnn_softmax_opp;
extern string rldcnn_solver_positive;
extern string rldcnn_solver_negative;
extern string dcnn_RL_ourNet;
extern string dcnn_RL_oppNet;

extern bool rl_isUseVNBaseline;
extern string dcnn_BV_VNNet;
extern string sDCNNRLDescription;

// Go Train Caffe DCNN option
extern string caffe_dcnn_net;
extern string caffe_dcnn_solver;
extern string caffe_train_dir;
extern string caffe_test_dir;
extern bool caffe_train_recursively;
extern int caffe_komi_vn_step;
extern bool caffe_sl_only_win_move;
extern bool caffe_use_opening;

// Train Caffe2 DCNN Setting
extern int display;
extern int snapshot;
extern string snapshot_prefix;
extern string NFS_path;
extern string run_id;

// Go Zero option
extern int zero_server_port;
extern string zero_train_directory;
extern int zero_start_iteration;
extern int zero_end_iteration;
extern int zero_num_game;
extern int zero_num_simulation;
extern float zero_threshold_ratio;
extern bool zero_training_7x7_killall;
extern bool zero_root_node_noise;
extern float zero_noise_alpha;
extern float zero_noise_epsilon;
extern float zero_resign_threshold;
extern float zero_disable_resign_prob;
extern float zero_selfplay_model_pool_threshold;
extern string zero_selfplay_machine_name;
extern string zero_selfplay_black_net;
extern string zero_selfplay_white_net;
extern string zero_selfplay_openings_file;
extern int zero_replay_buffer_size;
extern float zero_optimization_learning_rate;
extern int zero_optimization_num_worker;
extern int zero_optimization_sgf_max_iteration;
extern bool zero_optimization_quiet;
extern string zero_optimization_net;
extern string zero_optimization_solver;
extern int zero_optimization_snapshot_iter;
extern string zero_optimization_sgf_directory;
extern string zero_optimization_save_name_prefix;
extern int zero_evaluator_num_game;
extern float zero_evaluator_pass_threshold;

// Go Generate Sgf option
extern string gensgf_command;
extern string gensgf_filename_prefix;
extern string gensgf_dcnn_sl_net;
extern string gensgf_dcnn_rl_net;
extern string gensgf_opening_directory;

// Go Log option
extern bool DoSgfTreeDetailLog;
extern double LogTreeRootThresholdRatio;
extern bool DoDCNNCacheLog;
extern bool DoPlayoutCacheLog;
extern std::string log_dir;
extern std::string log_postfix;
extern bool DoLog;

// Go Dynamic Time Control option
extern bool DynamicTimeControl;
extern float TimeControlMainTime;
extern float TimeControlByoYomiTime;
extern int TimeControlByoYomiStones;
extern float TimeControlInitialStep;
extern float TimeControlRemainStep;
extern float TimeControlParameter;

// Go Rule option
extern unsigned int BoardSize;
extern float komi;
extern std::string superko_rule;
extern std::string counting_rule;
extern bool check_pass_superko;

// Go UCT option
extern bool use_dynamic_komi;
extern int dkomi_4komi;
extern unsigned int dkomi_start_step;
extern int dkomi_minus;
extern float dkomi_slop;
extern float dkomi_shift;
extern float dkomi_winrate_weight;
extern bool use_tree_cache;
extern bool use_virtual_loss;
extern unsigned int virtual_loss_count;
extern float ReuseThreshold;
extern double TerritoryThreshold;
extern bool mcts_use_solver;
extern bool mcts_use_disproof;
extern bool use_board_transposition_table;
extern bool use_rzone_transposition_table;
extern bool use_rzone_linear_transposition_table;
extern bool use_rzone_tt_size_heuristic;
extern bool use_rzone_tt_right_first;
extern bool use_rzone_tt_skip_on_id_diff;
extern bool use_rzone_tt_skip_larger_pattern;
extern int rzone_tt_phase1_size;
extern float rzone_tt_expansion_threshold;
extern bool tt_optimize_for_single_ko;
extern bool check_zone_tt_ghi;
extern bool enable_pass_by_playout;
extern bool cnn_policy_output_power_softmax;

// Go DCNN option
extern bool dcnn_use_ftl;
extern bool dcnn_ftl_remaining_move_num;
extern bool vn_turn_opposite;
extern bool dcnn_relaxed_child_generation;
extern bool dcnn_use_forced_komi;
extern float dcnn_forced_komi_value;
extern int dcnn_default_batch_size;
extern int dcnn_max_queue_size;
extern string dcnn_net;
extern float DCNNSLSoftMaxTemperature;
extern string sDCNNGroupDescription;

// JobLevelMCTS option
extern std::string Path;
extern std::string Ignore;

// MCTS argument
extern float mcts_puct_bias;
extern bool const mcts_use_ucb;
extern bool const mcts_use_puct;
extern bool const mcts_reuse_MM_playout;
extern bool const mcts_use_rave;
extern bool const mcts_use_progressive_bias;
extern bool const mcts_use_progressive_widening;
extern bool do_mcts_sequential_search;
extern double const init_NodeCount;

// MCTS UCT pattern feature
extern bool const mcts_use_mm;
extern bool const mcts_use_lfr;
extern bool const mcts_use_cnn;

// some data structure argument
extern bool const use_closed_area;

// playout argument
extern bool const use_playout;
extern bool const use_probability_playout;
extern bool const use_rule_based_playout;

extern bool const sim_kill_suicide;
extern bool const sim_reply_save_1lib;
extern bool const sim_reply_kill_2lib;
extern bool const sim_reply_save_2lib;
extern bool const sim_reply_defense_approach_lib;
extern bool const sim_reply_nakade;
extern bool const sim_fight_ko;
extern bool const sim_reply_good_pattern;
extern bool const sim_contiguous;
extern bool const sim_345point_semeai;

// tsumego argument
extern string problem_dir;
extern string result_dir;
extern string problem_filter;
extern int win_color;
extern int search_goal;
extern bool use_rzone;
extern bool check_potential_ghi_for_rzone;
extern bool use_immediate_win;
extern bool use_early_life;
extern bool use_pattern_eye;
extern bool use_potential_rzone;
extern bool use_opp1stone_suicide_likepass;
extern bool mcts_use_bv;
extern bool mcts_use_bv_zone;
extern bool update_rzone_in_opponent_turn;
extern float win_vn_threshold;
extern float bv_threshold;
extern int potential_rzone_count_limit;
extern float potential_rzone_goodchild_threshold;
extern bool use_dead_shape_detection;
extern bool use_independent_territory_detection;
extern bool only_find_winner;
extern bool use_opponent_border_rzone;
extern bool ignore_benson;
extern bool allow_proved_move_outside_rzone;
extern int problem_rotation;
extern bool black_ignore_ko;
extern bool white_ignore_ko;
extern bool use_editor_tree_format;

// other argument
extern unsigned int TotalGrids;
extern int RandomizeRaveFequency;
extern int TotalGpuNum;

extern bool LiveGfx;
extern bool DrawStrong;
extern bool DrawWeak;
extern bool DrawConnector;
extern bool DrawCut;
extern bool DrawBlack;
extern bool DrawWhite;
extern bool DrawDragon;
extern bool DrawClosedAreaBlack;
extern bool DrawClosedAreaWhite;

extern std::string edge_dir;
extern std::string sDynamicTimeControl;

void setConfigureOptions(ConfigureLoader& cl);

// CNN network output blob name
extern string const BV_OUTPUT_NAME;
extern string const SL_OUTPUT_NAME;
extern string const VN_OUTPUT_NAME;
extern string const GL_OUTPUT_NAME;
extern string const EYE_OUTPUT_NAME;
extern string const DRAGON_CONNECT_OUTPUT_NAME;
extern string const SL_LABEL_NAME;
extern string const SL_DISTRIBUTION_LABEL_NAME;
extern string const BV_LABEL_NAME;
extern string const VN_LABEL_NAME;
extern string const GL_LABEL_NAME;
extern string const EYE_LABEL_NAME;
extern string const DRAGON_CONNECT_LABEL_NAME;
} // namespace WeichiConfigure

#endif
