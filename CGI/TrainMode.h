#ifndef TRAINMODE_H
#define TRAINMODE_H

enum TrainMode {
	TRAINMODE_EXIT,
	TRAINMODE_COLLECT_RADIUS_PATTERN,
	TRAINMODE_GENERATE_RADIUS3_SYMMETRIC_INDEX_DB,
	TRAINMODE_GENERATE_UCT_FEATURES,
	TRAINMODE_GENERATE_PLAYOUT_FEATURES,
	TRAINMODE_GENERATE_CNN_ROTATATION_DB,
	TRAINMODE_CALCULATE_UCT_MOVE_PREDICTION,
	TRAINMODE_CALCULATE_PLAYOUT_MOVE_PREDICTION,
	TRAINMODE_LOG_NET_RECORD,
	TRAINMODE_WRITE_DEVELOPMENT_COMMAND,
	TRAINMODE_FIND_SGF_ATTRIBUTE,
	TRAINMODE_FIND_REPETITION_SGF,
	TRAINMODE_COLLECT_OPENING,
	TRAINMODE_FTL_POLICY,		
	TRAINMODE_ADD_REGION,			
	TRAINMODE_MERGE_TO_RAW_SGF,
	TRAINMODE_CHECK_SGF,
	TRAINMODE_GENERATE_JSON,
	TRAINMODE_SIZE	// total size, add new element before this one
};
inline string getTrainModeString( TrainMode mode )
{
	switch( mode ) {
	case TRAINMODE_EXIT:								return "End train mode";
	case TRAINMODE_COLLECT_RADIUS_PATTERN:				return "Collect radius pattern";
	case TRAINMODE_GENERATE_RADIUS3_SYMMETRIC_INDEX_DB:	return "Generate radius3 symmetric index db";
	case TRAINMODE_GENERATE_UCT_FEATURES:				return "Generate UCT features";
	case TRAINMODE_GENERATE_PLAYOUT_FEATURES:			return "Generate playout features";
	case TRAINMODE_GENERATE_CNN_ROTATATION_DB:			return "Generate CNN rotation database";
	case TRAINMODE_CALCULATE_UCT_MOVE_PREDICTION:		return "Calculate UCT move prediction rate";
	case TRAINMODE_CALCULATE_PLAYOUT_MOVE_PREDICTION:	return "Calculate playout move prediction rate";
	case TRAINMODE_LOG_NET_RECORD:						return "Log Net record";
	case TRAINMODE_WRITE_DEVELOPMENT_COMMAND:			return "Write development command";
	case TRAINMODE_FIND_SGF_ATTRIBUTE:					return "Find sgf attribute";
	case TRAINMODE_FIND_REPETITION_SGF:					return "Find repetition sgf";
	case TRAINMODE_COLLECT_OPENING:						return "Collect Go Opening from sgf";
	case TRAINMODE_FTL_POLICY:							return "Get the policy of marked FTL move";		
	case TRAINMODE_ADD_REGION:							return "Add region for a given sgf seperated by wall";			
	case TRAINMODE_MERGE_TO_RAW_SGF:					return "Merge to raw sgfs";
	case TRAINMODE_CHECK_SGF:							return "Check sgf";
	case TRAINMODE_GENERATE_JSON:						return "Generate json";
	default:
		//should not happen
		assert( false );
		return "error mode!";
	}
}

#endif 
