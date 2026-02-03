#ifndef WEICHIPLAYOUTAGNET_H
#define WEICHIPLAYOUTAGNET_H

#include "BasePlayoutAgent.h"
#include "WeichiMove.h"
#include "WeichiPlayoutResult.h"
#include "WeichiThreadState.h"
#include "WeichiProbabilityPlayoutHandler.h"
#include "WeichiMoveCache.h"

#include <stdlib.h>

class WeichiPlayoutAgent : public BasePlayoutAgent<WeichiMove, WeichiPlayoutResult, WeichiThreadState>
{
	friend class TrainGenerator;
	friend class WeichiBaseGtpEngine;
	friend class WeichiPlayoutGtpEngine;
private:
	const WeichiBoard& m_board;
	WeichiBitBoard m_bmSkipMove;
	string m_sPolicyInfo;
	WeichiPlayoutPolicy m_usedPolicy;
	Vector<uint,MAX_NUM_GRIDS> m_vCandidateMoves;

public:
	bool m_bCalculateGradient;
	Vector<double,MAX_NUM_PLAYOUT_FEATURES> m_vGradients;

    WeichiPlayoutAgent ( WeichiThreadState& state )
        : BasePlayoutAgent<WeichiMove, WeichiPlayoutResult, WeichiThreadState>( state )
        , m_usedPolicy( POLICY_UNKNOWN )
		, m_board( state.m_board )
		, m_bCalculateGradient( false )
    {
    }

    /*!
        @brief  run entire playout
        @author kwchen
    */
	WeichiPlayoutResult run() 
	{
		if (!WeichiConfigure::use_playout) { return (m_state.isTerminal() ? m_state.eval() : WeichiPlayoutResult(0)); }

		WeichiMove lastmove = PASS_MOVE ;
		bool abort = false ;

		if( m_board.hasPrevMove() ) { lastmove = m_board.getPrevMove(); }
		if( WeichiConfigure::DoPlayoutCacheLog ) { WeichiGlobalInfo::getLogInfo().m_cacheLogger.writeTreeMove(m_board,m_state.m_moves); }

		while( !m_state.isTerminal() ) {
			m_sPolicyInfo = "";
			m_usedPolicy = POLICY_UNKNOWN ;
			WeichiMove move = PASS_MOVE ;

			move = genPlayoutMove(lastmove) ;
			if ( m_usedPolicy == POLICY_ABORT ) {
				abort = true;
				break;
			}

			if( WeichiConfigure::ShowPlayoutPolicyStatistic ) { ++WeichiGlobalInfo::getSearchInfo().m_policyStat[m_usedPolicy].get(move.getColor()); }

			assertToFile( !(move==PASS_MOVE), const_cast<WeichiBoard*>(&m_board) );
			assertToFile( !m_board.isIllegalMove(move, m_state.m_ht), const_cast<WeichiBoard*>(&m_board) );
			lastmove = move ;
			m_state.play ( move ) ;

			if( WeichiConfigure::DoPlayoutCacheLog ) {
				WeichiGlobalInfo::getLogInfo().m_cacheLogger.writeOneMove(m_board,move,getWeichiPlayoutPolicyString(m_usedPolicy)+" "+m_sPolicyInfo);
			}
		}

		if( abort ) {
			return m_state.abortEval();
		} else {
			m_state.preCalculateForEval();
			WeichiPlayoutResult result = m_state.eval();
			return result;
		}
	}

    friend class WeichiGtpEngine;
private:
	void findSpecificPolicyCandidates( const WeichiMove& lastMove, Vector<uint,MAX_NUM_GRIDS>& vCandidates, WeichiPlayoutPolicy policy );

	void calcualteLocalPlayoutFeature( const WeichiMove& lastMove );

	/*!
		@brief  try to kill suicide move
		@author T.F. Liao
		@return move to kill a suicide move, or PASS_MOVE if no such move
		if last move of opponent is a suicide move, kill it
	*/
	WeichiMove genMoveToKillSuicide( const WeichiMove& lastMove );

	/*!
		@brief  try to save one liberty block (atari by last opponent move)
		@author T.F. Liao
		@return move to save one liberty block
	*/
	void genMoveToReplySaveOneLibBlock( const WeichiMove& lastMove, Vector<uint,MAX_NUM_GRIDS>& vSaveMoves );

	/*!
		@brief  try to save two-liberty block (cause by last opponent move)
		@author T.F. Liao
		@return move to save two-liberty block
	*/
	void genMoveToReplySaveTwoLibBlock( const WeichiMove& lastMove, Vector<uint,MAX_NUM_GRIDS>& vSaveMoves );

	void genMoveToReplyKillTwoLibBlock( const WeichiMove& lastMove, Vector<uint,MAX_NUM_GRIDS>& vKillMoves );
	void genMoveToReplyDefenseApproachLib( const WeichiMove& lastMove, Vector<uint,MAX_NUM_GRIDS>& vDefenseMoves );

	/*!
		@brief  push ko point if any
		@author T.F. Liao
		@return move to push ko
	*/
	void genMoveToFightKo( Vector<uint,MAX_NUM_GRIDS>& vCandidateMoves );

	/*!
		@brief  try to find if good pattern nearing last opponent move
		@author T.F. Liao
		@return good pattern move nearing last opponent move
	*/
	void genMoveToReplyGoodPattern( const WeichiMove& lastMove, Vector<uint,MAX_NUM_GRIDS>& vGoodPatternMoves );

	/*!
		@brief  try to nakede if opponent create a closed area
		@author T.F. Liao
		@return move to nakade
	*/
	void genMoveToReplyLastCANakade( const WeichiMove& lastMove, Vector<uint,MAX_NUM_GRIDS>& vNakadeMoves );

	void genMoveTo345PointSemeai( const WeichiMove& lastMove, Vector<uint,MAX_NUM_GRIDS>& vSemeaiMoves );

	/*!
		@brief  generate a random move
		@author T.F. Liao
		@return move that random selected from legal moves
	*/
	WeichiMove genRandomMove();

	/*!
		@brief  randomly select a move from moveset with check the rule
		@author T.F. Liao
		@return move that random selected from moveset
	*/
	WeichiMove randomSelectMove( Color turnColor, const Vector<uint,MAX_NUM_GRIDS>& moveset );

    enum RandomMoveType {
        IGNORE_MOVE,
        REMOVE_CANDIDATE,
        FINE
    };
	RandomMoveType checkRandomMove( const WeichiMove& move );

    inline WeichiMove genPlayoutMove( const WeichiMove& lastMove );
	inline WeichiMove genRuleBasedPlayoutMove( const WeichiMove& lastMove );
	inline WeichiMove genProbabilityPlayoutMove( const WeichiMove& lastMove );

	inline WeichiMove genReplyLastMove( const WeichiMove& lastMove );
	inline WeichiMove genUCTCacheMove( const WeichiMove& lastMove );

	// probability playout
	void addContiguousFeatures( const WeichiMove& lastMove );
	void addKill1LibFeatures( const WeichiBlock* block );
	void addSave1LibFeatures( const WeichiBlock* block );
	bool addSave1LibByCaptureFeatures( const WeichiBlock* block );
	void addSave1LibByExtendFeatures( const WeichiBlock* block, const bool bCanSaveByCapture );
	void addSave1LibByCaptureExtendNbrFeatures( const WeichiBlock* block, const bool bCanSaveByCapture, const PredictPlayInfo& pInfo );
	void addKill2LibFeatures( const WeichiBlock* block );
	void addSave2LibFeatures( const WeichiBlock* block );
	void addSave2LibByCaptureFeatures( const WeichiBlock* block );
	void addSave2LibByExtendAndJumpFeatures( const WeichiBlock* block );
	void addNakadeFeatures( const WeichiMove& lastMove );
	void addMakeEyeFeatures( const WeichiMove& lastMove );
	void addMakeEyeDetailFeatures( const WeichiGrid& grid );
	void addFixEyeFeatures( const WeichiMove& lastMove );
	void addDestroyEyeFeatures( const WeichiMove& lastMove );
	void add2345SemeaiFeatures( const WeichiBlock* block );
	uint adjustSemeaiPoint( const uint position );
	void addCASemeaiFeature( const WeichiBlock* block );

	inline void addProbabilityFeature( const uint position, Color turnColor, WeichiPlayoutPolicy policy );
	inline void addProbabilityFeatures( WeichiBitBoard& bmCandidates, Color turnColor, WeichiPlayoutPolicy policy );
	inline void addProbabilityFeatures( const Vector<uint,MAX_NUM_GRIDS>& vCandidate, Color turnColor, WeichiPlayoutPolicy policy );

    inline const WeichiKnowledgeBase& getKB() const { return m_state.m_kb ; }
    inline WeichiKnowledgeBase& getKB() { return m_state.m_kb ; }
};

WeichiMove WeichiPlayoutAgent::genPlayoutMove( const WeichiMove& lastMove )
{
	// abort policy
	WeichiMove move = PASS_MOVE;
	if( m_board.getMoveList().size()>=MAX_GAME_LENGTH ) {
		m_usedPolicy = POLICY_ABORT;
		return move;
	}

	if( WeichiConfigure::use_rule_based_playout ) { return genRuleBasedPlayoutMove(lastMove); }
	else if( WeichiConfigure::use_probability_playout ) { return genProbabilityPlayoutMove(lastMove); }

	// should not be here
	assertToFile( false, const_cast<WeichiBoard*>(&m_board) );
	return WeichiMove(m_board.getToPlay());
}

WeichiMove WeichiPlayoutAgent::genRuleBasedPlayoutMove( const WeichiMove& lastMove )
{
	WeichiMove move = PASS_MOVE;

	m_bmSkipMove.Reset();

	// Uct cache move
	if( !lastMove.isPass() && !(move=genUCTCacheMove(lastMove)).isPass() ) {
		m_usedPolicy = POLICY_CACHE;
		return move;
	}

	// reply last move policy
	if( !lastMove.isPass() && !(move=genReplyLastMove(lastMove)).isPass() ) { return move; }

	// random policy
	if( !(move=genRandomMove()).isPass() ) {
		m_usedPolicy = POLICY_RANDOM;
		return move;
	}

	m_usedPolicy = POLICY_PASS;
	return move ;
}

WeichiMove WeichiPlayoutAgent::genProbabilityPlayoutMove( const WeichiMove& lastMove )
{
	m_bmSkipMove.Reset();
	WeichiMove move = PASS_MOVE;
	Color turnColor = m_board.getToPlay();

	// Uct cache move
	if( !lastMove.isPass() && !(move=genUCTCacheMove(lastMove)).isPass() ) {
		m_usedPolicy = POLICY_CACHE;
		return move;
	}
	
	WeichiProbabilityPlayoutHandler& probabilityHandler = m_state.m_board.m_probabilityHandler;
	calcualteLocalPlayoutFeature(lastMove);

	if (m_state.m_board.hasKo()) { probabilityHandler.skipPosition(m_state.m_board.getKo(), turnColor); }

	while( true ) {
		uint position = probabilityHandler.chooseBySoftMax(turnColor,m_state.m_board.getBitBoard());
		WeichiMove move(turnColor,position);

		if( checkRandomMove(move)==FINE ) {
			
			//////////////////////////////////////////////////////////////////////////
			// log
			/*bool bIsMatch = false;
			WeichiPlayoutPolicy policy = POLICY_UNKNOWN;
			for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
				const WeichiGrid& grid = m_board.getGrid(*it);
				if( !grid.isEmpty() ) { continue; }

				const ProbabilityGrid& probGrid = probabilityHandler.getProbabilityTable(turnColor).getProbGrid(*it);
				if( probGrid.getPlayoutFeatureBits().BitIsOn(policy) ) {
					bIsMatch = true;
					if( *it==move.getPosition() ) { cerr << "win move: "; }
					else { cerr << "loss move: "; }
					cerr << WeichiMove(*it).toGtpString() << ", " << toChar(turnColor) << endl;
				}
			}
			if( bIsMatch ) {
				m_board.showColorBoard();
				cerr << "previous move: " << m_board.getPrevMove().toGtpString() << ", " << toChar(m_board.getPrevMove().getColor()) << endl;
				int k;
				cin >> k;
			}*/
			//////////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////////////
			// calculate gradients
			if( m_bCalculateGradient && !move.isPass() ) {
				double dTotalScore = 0.0f;
				Vector<bool,MAX_NUM_GRIDS> vFineMove;
				vFineMove.resize(MAX_NUM_GRIDS);
				for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
					const WeichiGrid& grid = m_board.getGrid(*it);
					if( !grid.isEmpty() ) { continue; }
					const ProbabilityGrid& probGrid = probabilityHandler.getProbabilityTable(turnColor).getProbGrid(*it);
					if( probGrid.isSkipGrid() ) { continue; }

					vFineMove[*it] = (checkRandomMove(WeichiMove(turnColor,*it))==FINE);
					if( !vFineMove[*it] ) { continue; }

					dTotalScore += probGrid.getScore();
				}
				for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
					const WeichiGrid& grid = m_board.getGrid(*it);
					if( !grid.isEmpty() ) { continue; }
					if( !vFineMove[*it] ) { continue; }

					const ProbabilityGrid& probGrid = probabilityHandler.getProbabilityTable(turnColor).getProbGrid(*it);
					if( probGrid.isSkipGrid() ) { continue; }

					const double dProbability = probGrid.getScore() / dTotalScore;

					const Vector<uint,MAX_NUM_PLAYOUT_FEATURE_GROUPS> vFeatures = probGrid.getFeatureIndexs();
					for( uint i=0; i<vFeatures.size(); i++ ) {
						uint iIndex = vFeatures[i];
						//double dG = m_vGradients[iIndex];

						if( *it==position ) { m_vGradients[iIndex] += 1-dProbability; }
						else { m_vGradients[iIndex] -= dProbability; }
					}
				}
			}
			//////////////////////////////////////////////////////////////////////////

			probabilityHandler.recover(turnColor);

			return move;
		} else {
			probabilityHandler.skipPosition(position,turnColor);
		}
	}
	
	// should not be here
	assertToFile( false, const_cast<WeichiBoard*>(&m_board) );
	return WeichiMove(turnColor);
}

WeichiMove WeichiPlayoutAgent::genReplyLastMove( const WeichiMove& lastMove )
{
	assertToFile( !lastMove.isPass(), const_cast<WeichiBoard*>(&m_board) );

	WeichiMove move = PASS_MOVE;
	Color turnColor = m_board.getToPlay();

	if( WeichiConfigure::sim_kill_suicide ) {
		move = genMoveToKillSuicide(lastMove);
		if( !move.isPass() ) {
			m_usedPolicy = POLICY_KILL_SUICIDE;
			return move;
		}
	}

	if( WeichiConfigure::sim_reply_save_1lib ) {
		genMoveToReplySaveOneLibBlock(lastMove,m_vCandidateMoves);
		move = randomSelectMove(turnColor,m_vCandidateMoves);
		if( !move.isPass() ) {
			m_usedPolicy = POLICY_REPLY_SAVE_1LIB;
			return move;
		}
	}

	if( WeichiConfigure::sim_reply_kill_2lib ) {
		genMoveToReplyKillTwoLibBlock(lastMove,m_vCandidateMoves);
		move = randomSelectMove(turnColor,m_vCandidateMoves);
		if( !move.isPass() ) {
			m_usedPolicy = POLICY_REPLY_KILL_2LIB;
			return move;
		}
	}

	if( WeichiConfigure::sim_reply_save_2lib ) {
		genMoveToReplySaveTwoLibBlock(lastMove,m_vCandidateMoves);
		move = randomSelectMove(turnColor,m_vCandidateMoves);
		if( !move.isPass() ) {
			m_usedPolicy = POLICY_REPLY_SAVE_2LIB;
			return move;
		}
	}

	if( WeichiConfigure::sim_reply_defense_approach_lib ) {
		genMoveToReplyDefenseApproachLib(lastMove,m_vCandidateMoves);
		move = randomSelectMove(turnColor,m_vCandidateMoves);
		if( !move.isPass() ) {
			m_usedPolicy = POLICY_REPLY_DEFENSE_APPROACH_LIB;
			return move;
		}
	}

	if( WeichiConfigure::sim_reply_nakade ) {
		genMoveToReplyLastCANakade(lastMove,m_vCandidateMoves);
		move = randomSelectMove(turnColor,m_vCandidateMoves);
		if( !move.isPass() ) {
			m_usedPolicy = POLICY_REPLY_NAKADE;
			return move;
		}
	}

	if( WeichiConfigure::sim_fight_ko ) {
		genMoveToFightKo(m_vCandidateMoves);
		move = randomSelectMove(turnColor,m_vCandidateMoves);
		if( !move.isPass() ) {
			m_usedPolicy = POLICY_FIGHT_KO;
			return move;
		}
	}

	if( WeichiConfigure::sim_reply_good_pattern ) {
		genMoveToReplyGoodPattern(lastMove,m_vCandidateMoves);
		move = randomSelectMove(turnColor,m_vCandidateMoves);
		if( !move.isPass() ) {
			m_usedPolicy = POLICY_REPLY_GOOD_PATTERN;
			return move;
		}
	}

	return WeichiMove(turnColor);
}

WeichiMove WeichiPlayoutAgent::genUCTCacheMove( const WeichiMove& lastMove )
{
	if( Random::nextInt(10)<1 ) { return PASS_MOVE; }

	WeichiMove replyMove = WeichiGlobalInfo::getSearchInfo().m_moveCache.getReplyMove(m_board, lastMove);
	if (checkRandomMove(replyMove) == FINE) { return replyMove; }

	return PASS_MOVE;
}

void WeichiPlayoutAgent::addProbabilityFeature( const uint position, Color turnColor, WeichiPlayoutPolicy policy )
{
	if( position==PASS_MOVE.getPosition() ) { return; }

	WeichiProbabilityPlayoutHandler& probabilityHandler = m_state.m_board.m_probabilityHandler;
	probabilityHandler.addAdditionalFeature(position,turnColor,policy);
}

void WeichiPlayoutAgent::addProbabilityFeatures( WeichiBitBoard& bmCandidates, Color turnColor, WeichiPlayoutPolicy policy )
{
	uint pos;
	WeichiProbabilityPlayoutHandler& probabilityHandler = m_state.m_board.m_probabilityHandler;
	while( (pos=bmCandidates.bitScanForward())!=-1 ) {
		probabilityHandler.addAdditionalFeature(pos,turnColor,policy);
	}
}

void WeichiPlayoutAgent::addProbabilityFeatures( const Vector<uint,MAX_NUM_GRIDS>& vCandidate, Color turnColor, WeichiPlayoutPolicy policy )
{
	WeichiProbabilityPlayoutHandler& probabilityHandler = m_state.m_board.m_probabilityHandler;
	for( uint i=0; i<vCandidate.size(); i++ ) {
		probabilityHandler.addAdditionalFeature(vCandidate[i],turnColor,policy);
	}
}

#endif // WEICHIPLAYOUTAGNET_H