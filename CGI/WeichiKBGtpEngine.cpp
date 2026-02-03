#include "WeichiKBGtpEngine.h"
#include "WeichiBadMoveReader.h"
#include "WeichiLadderReader.h"

void WeichiKbGtpEngine::Register()
{
	RegisterFunction("save_block", this, &WeichiKbGtpEngine::cmdSaveBlock, 1);
	RegisterFunction("kill_block", this, &WeichiKbGtpEngine::cmdKillBlock, 1);
	RegisterFunction("simple_fast_ladder_path", this, &WeichiKbGtpEngine::cmdSimpleFastLadderPath, 1);
	RegisterFunction("complicate_ladder_path", this, &WeichiKbGtpEngine::cmdComplicateLadderPath, 1);
	RegisterFunction("bad_moves", this, &WeichiKbGtpEngine::cmdBadMoves, 0);
	RegisterFunction("life_territory", this, &WeichiKbGtpEngine::cmdLifeTerritory, 0);
	RegisterFunction("potential_true_eye", this, &WeichiKbGtpEngine::cmdPotentialTrueEye, 0);
	RegisterFunction("two_liberty_false_eye", this, &WeichiKbGtpEngine::cmdTwoLibertyFalseEye, 0);
	RegisterFunction("self_atari", this, &WeichiKbGtpEngine::cmdSelfAtari, 0);
	RegisterFunction("mutual_atari_moves", this, &WeichiKbGtpEngine::cmdMutualAtariMoves, 0);
	RegisterFunction("filling_own_eye_shape_moves", this, &WeichiKbGtpEngine::cmdFillingOwnEyeShapeMoves, 0);
	RegisterFunction("not_making_nakade_in_opp_ca_moves", this, &WeichiKbGtpEngine::cmdNotMakingNakadeInOppCA, 0);
}

string WeichiKbGtpEngine::getGoguiAnalyzeCommandsString()
{
	ostringstream oss;
	oss << "string/=====Knowledge Base=====/empty\n"
		<< "gfx/Save Block/save_block %p\n"
		<< "gfx/Kill Block/kill_block %p\n"
		<< "gfx/Simple Fast Ladder Path/simple_fast_ladder_path %p\n"
		<< "gfx/Complicate Ladder Path/complicate_ladder_path %p\n"
		<< "gfx/Bad Moves/bad_moves\n"
		<< "gfx/Life Territory/life_territory\n"
		<< "gfx/Potential True Eye/potential_true_eye\n"
		<< "gfx/Two Liberty False Eye/two_liberty_false_eye\n"
		<< "gfx/Self-Atari/self_atari\n"
		<< "gfx/Mutual Atari Moves/mutual_atari_moves\n"
		<< "gfx/Filling Own Eye Shape Moves/filling_own_eye_shape_moves\n"
		<< "gfx/Not Making Nakade in Opponent CA Moves/not_making_nakade_in_opp_ca_moves\n"
		;

	return oss.str();
}

void WeichiKbGtpEngine::cmdSaveBlock()
{
	string sPosition = m_args[0];
	transform(sPosition.begin(),sPosition.end(),sPosition.begin(),::toupper);
	Move move(COLOR_NONE, sPosition);

	const WeichiBoard& board = m_mcts.getState().m_board;
	const WeichiGrid& grid = board.getGrid(move.getPosition());
	if( grid.isEmpty() ) { reply(GTP_FAIL, "must not be empty point."); return; }
	if( grid.getBlock()->getLiberty()>2 ) { reply(GTP_FAIL, "must <=2 liberty."); return; }

	WeichiLocalSearchHandler& localSearchHandler = const_cast<WeichiLocalSearchHandler&>(m_mcts.getState().m_localSearchHandler);
	localSearchHandler.findFullBoardBlockLocalSequence();

	ostringstream oss;
	const WeichiLocalSequence* localSequence = grid.getBlock()->getSaveLocalSequence();
	CERR() << "save result: " << getWeichiBlockSearchResultString(localSequence->getResult()) << endl;

	oss << "SQUARE";
	for( uint iNum=0; iNum<localSequence->getNumSuccess(); iNum++ ) { oss << ' ' << WeichiMove(localSequence->getSuccessPos(iNum)).toGtpString(); }
	oss << endl << "TRIANGLE";
	for( uint iNum=0; iNum<localSequence->getNumUnknown(); iNum++ ) { oss << ' ' << WeichiMove(localSequence->getUnknownPos(iNum)).toGtpString(); }
	oss << endl << "MARK";
	for( uint iNum=0; iNum<localSequence->getNumFailed(); iNum++ ) { oss << ' ' << WeichiMove(localSequence->getFailedPos(iNum)).toGtpString(); }

	reply(GTP_SUCC, oss.str());
}

void WeichiKbGtpEngine::cmdKillBlock()
{
	string sPosition = m_args[0];
	transform(sPosition.begin(),sPosition.end(),sPosition.begin(),::toupper);
	Move move(COLOR_NONE, sPosition);

	const WeichiBoard& board = m_mcts.getState().m_board;
	const WeichiGrid& grid = board.getGrid(move.getPosition());
	if( grid.isEmpty() ) { reply(GTP_FAIL, "must not be empty point."); return; }
	if( grid.getBlock()->getLiberty()>2 ) { reply(GTP_FAIL, "must <=2 liberty."); return; }

	WeichiLocalSearchHandler& localSearchHandler = const_cast<WeichiLocalSearchHandler&>(m_mcts.getState().m_localSearchHandler);
	localSearchHandler.findFullBoardBlockLocalSequence();

	ostringstream oss;
	const WeichiLocalSequence* localSequence = grid.getBlock()->getKillLocalSequence();
	CERR() << "kill result: " << getWeichiBlockSearchResultString(localSequence->getResult()) << endl;

	oss << "SQUARE";
	for( uint iNum=0; iNum<localSequence->getNumSuccess(); iNum++ ) { oss << ' ' << WeichiMove(localSequence->getSuccessPos(iNum)).toGtpString(); }
	oss << endl << "TRIANGLE";
	for( uint iNum=0; iNum<localSequence->getNumUnknown(); iNum++ ) { oss << ' ' << WeichiMove(localSequence->getUnknownPos(iNum)).toGtpString(); }
	oss << endl << "MARK";
	for( uint iNum=0; iNum<localSequence->getNumFailed(); iNum++ ) { oss << ' ' << WeichiMove(localSequence->getFailedPos(iNum)).toGtpString(); }

	reply(GTP_SUCC, oss.str());
}

void WeichiKbGtpEngine::cmdSimpleFastLadderPath()
{
	string sPosition = m_args[0];
	transform(sPosition.begin(),sPosition.end(),sPosition.begin(),::toupper);
	Move move(COLOR_NONE, sPosition);

	ostringstream oss;
	const WeichiBoard& board = m_mcts.getState().m_board;
	const WeichiGrid& grid = board.getGrid(move.getPosition());
	if( grid.isEmpty() ) { reply(GTP_SUCC, oss.str()); return; }

	const WeichiBlock* block = grid.getBlock();
	WeichiLadderType type = WeichiLadderReader::getLadderType(board,block);

	CERR() << getWeichiLadderTypetring(type) << endl;
	if( type!=LADDER_UNKNOWN ) {
		if( WeichiLadderReader::isInSimpleFastLadder(board,grid.getColor(),grid,type) ) { CERR() << "in simple fast ladder" << endl; }
		else { CERR() << "not in simple fast ladder" << endl; }

		uint pos;
		WeichiBitBoard bmLadderPath = grid.getStaticGrid().getLadderPath(type);
		oss << "SQUARE";
		while( (pos=bmLadderPath.bitScanForward())!=-1 ) {
			oss << ' ' <<WeichiMove(pos).toGtpString(); 
		}
	}

	reply(GTP_SUCC, oss.str());
}

void WeichiKbGtpEngine::cmdComplicateLadderPath()
{
	string sPosition = m_args[0];
	transform(sPosition.begin(),sPosition.end(),sPosition.begin(),::toupper);
	Move move(COLOR_NONE, sPosition);

	ostringstream oss;
	WeichiThreadState& state = const_cast<WeichiThreadState&>(m_mcts.getState());
	WeichiBoard& board = state.m_board;
	OpenAddrHashTable& ht = state.m_ht;
	const WeichiGrid& grid = board.getGrid(move.getPosition());
	if( grid.isEmpty() ) { reply(GTP_SUCC, oss.str()); return; }

	const WeichiBlock* block = grid.getBlock();
	if( !WeichiLadderReader::isInComplicateLadder(board,ht,block) ) { reply(GTP_SUCC, oss.str()); return; }

	WeichiMiniSearch::startMiniPlay(board);
	const uint savePos = block->getLastLiberty(board.getBitBoard());
	WeichiBlockSearchResult result = WeichiLadderReader::save_ladder(board,ht,block->getiFirstGrid(),savePos);
	CERR() << getWeichiBlockSearchResultString(result) << endl;
	WeichiMiniSearch::endMiniPlay(board);

	reply(GTP_SUCC, oss.str());
}

void WeichiKbGtpEngine::cmdBadMoves()
{
	const WeichiBoard& board = m_mcts.getState().m_board;
	ostringstream oss;
	oss << "SQUARE";

	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		WeichiMove move(board.getToPlay(),*it);
		if( !board.getGrid(move).isEmpty() ) { continue; }
		if( WeichiBadMoveReader::getBadMoveType(board,move)!=BADMOVE_NOT_BADMOVE ) {
			oss << ' ' << move.toGtpString();
		}
	}
	oss << "\n";

	reply(GTP_SUCC, oss.str());
}

void WeichiKbGtpEngine::cmdLifeTerritory()
{
	const WeichiBoard& board = m_mcts.getState().m_board;
	ostringstream oss;
	oss << "SQUARE";

	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		WeichiMove move(board.getToPlay(),*it);
		if( !board.getGrid(move).isEmpty() ) { continue; }
		if( WeichiBadMoveReader::isLifeTerritory(board,move) ) {
			oss << ' ' << move.toGtpString();
		}
	}
	oss << "\n";

	reply(GTP_SUCC, oss.str());
}

void WeichiKbGtpEngine::cmdPotentialTrueEye()
{
	const WeichiBoard& board = m_mcts.getState().m_board;
	ostringstream oss;
	oss << "SQUARE";

	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		WeichiMove move(board.getToPlay(),*it);
		if( !board.getGrid(move).isEmpty() ) { continue; }
		if( WeichiBadMoveReader::isPotentialTrueEye(board,move) ) {
			oss << ' ' << move.toGtpString();
		}
	}
	oss << "\n";

	reply(GTP_SUCC, oss.str());
}

void WeichiKbGtpEngine::cmdTwoLibertyFalseEye()
{
	const WeichiBoard& board = m_mcts.getState().m_board;
	ostringstream oss;
	oss << "SQUARE";

	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		WeichiMove move(board.getToPlay(),*it);
		if( !board.getGrid(move).isEmpty() ) { continue; }

		PredictPlayInfo myInfo;
		board.getPredictPlayInfoByPlay(move,myInfo);
		if( WeichiBadMoveReader::isTwoLibFalseEye(board,move,myInfo) ) {
			oss << ' ' << move.toGtpString();
		}
	}
	oss << "\n";

	reply(GTP_SUCC, oss.str());
}

void WeichiKbGtpEngine::cmdSelfAtari()
{
	const WeichiBoard& board = m_mcts.getState().m_board;
	ostringstream oss;
	oss << "SQUARE";

	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		WeichiMove move(board.getToPlay(),*it);
		if( !board.getGrid(move).isEmpty() ) { continue; }

		PredictPlayInfo myInfo;
		board.getPredictPlayInfoByPlay(move,myInfo);
		if( WeichiBadMoveReader::isBadSelfAtari(board,move,myInfo) ) {
			oss << ' ' << move.toGtpString();
		}
	}
	oss << "\n";

	reply(GTP_SUCC, oss.str());
}

void WeichiKbGtpEngine::cmdMutualAtariMoves()
{
	const WeichiBoard& board = m_mcts.getState().m_board;
	ostringstream oss;
	oss << "SQUARE";

	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		WeichiMove move(board.getToPlay(),*it);
		if( !board.getGrid(move).isEmpty() ) { continue; }

		PredictPlayInfo myInfo;
		board.getPredictPlayInfoByPlay(move,myInfo);
		if( WeichiBadMoveReader::isMutualAtari(board,move,myInfo) ) {
			oss << ' ' << move.toGtpString();
		}
	}
	oss << "\n";

	reply(GTP_SUCC, oss.str());
}

void WeichiKbGtpEngine::cmdFillingOwnEyeShapeMoves()
{
	const WeichiBoard& board = m_mcts.getState().m_board;
	ostringstream oss;
	oss << "SQUARE";

	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		WeichiMove move(board.getToPlay(), *it);
		if( !board.getGrid(move).isEmpty() ) { continue; }

		if( WeichiBadMoveReader::isFillingOwnEyeShape(board, move) ) {
			oss << ' ' << move.toGtpString();
		}
	}
	oss << "\n";

	reply(GTP_SUCC, oss.str());
}

void WeichiKbGtpEngine::cmdNotMakingNakadeInOppCA()
{
	const WeichiBoard& board = m_mcts.getState().m_board;
	ostringstream oss;
	oss << "SQUARE";

	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		WeichiMove move(board.getToPlay(), *it);
		if( !board.getGrid(move).isEmpty() ) { continue; }

		PredictPlayInfo myInfo;
		board.getPredictPlayInfoByPlay(move,myInfo);
		if( WeichiBadMoveReader::isNotMakingNakadeInOppCA(board, move, myInfo) ) {
			oss << ' ' << move.toGtpString();
		}
	}
	oss << "\n";

	reply(GTP_SUCC, oss.str());
}