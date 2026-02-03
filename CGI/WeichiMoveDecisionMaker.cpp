#include "WeichiMoveDecisionMaker.h"
#include "WeichiThreadState.h"
#include "WeichiConfigure.h"
#include "WeichiBadMoveReader.h"

WeichiMove WeichiMoveDecisionMaker::operator() ( const WeichiThreadState& state, UctNodePtr pRoot ) 
{
	// remove virtual loss here for correctness of console message
	WeichiGlobalInfo::getRemoteInfo().m_cnnJobTable.revert();

	Color toPlay = state.getRootTurn() ;
	WeichiMove& bestMove = WeichiGlobalInfo::getTreeInfo().m_bestMove;
	StatisticData& bestSD = WeichiGlobalInfo::getTreeInfo().m_bestSD;
	StatisticData& bestValueSD = WeichiGlobalInfo::getTreeInfo().m_bestValueSD;
	bestMove = WeichiMove(toPlay); // generate pass with color = toPlay
	bestSD.reset();
	bestValueSD.reset();

	if ( pRoot.isNull() ) {
		return WeichiMove(toPlay);
	}

	if (WeichiConfigure::mcts_use_solver) {
		UctNodePtr bestChild;
		WeichiMove secondBestMove = WeichiMove(toPlay); // generate pass with color = toPlay
		StatisticData secondBestSD;
		StatisticData secondBestValueSD;
		StatisticData passSD;
		StatisticData passValueSD;

		Color winColor = WeichiGlobalInfo::getTreeInfo().m_winColor ;		
		Color killColor = AgainstColor(winColor) ;
		for (UctChildIterator<WeichiUctNode> it(pRoot); it; ++it) {
			if (it->getColor()==winColor && it->isProved()) {
				secondBestMove = bestMove;
				secondBestSD = bestSD;
				bestMove = it->getMove();
				bestSD = it->getUctDataWithoutVirtualLoss();
				secondBestValueSD = bestValueSD;
				bestValueSD = it->getVNValueData();
				bestChild = it;
				WeichiGlobalInfo::getRootSimInfo().push_back(tuple<int,double,short>(pRoot->getUctDataWithoutVirtualLoss().getCount(), bestChild->getUctDataWithoutVirtualLoss().getMean(), bestChild->getPosition()));
				break ;
			} 
			else if (it->getColor()==killColor && it->isProved()) {				
				continue ;
			}
			else if (it->getUctDataWithoutVirtualLoss().getCount() > bestSD.getCount()) {
				secondBestMove = bestMove;
				secondBestSD = bestSD;
				bestMove = it->getMove();
				bestSD = it->getUctDataWithoutVirtualLoss();
				secondBestValueSD = bestValueSD;
				bestValueSD = it->getVNValueData();
				bestChild = it;
			} else if (it->getUctDataWithoutVirtualLoss().getCount() > secondBestSD.getCount()) {
				secondBestMove = it->getMove();
				secondBestSD = it->getUctDataWithoutVirtualLoss() ;
				secondBestValueSD = it->getVNValueData();
			}
			if (it->getMove().isPass()) {
				passSD = it->getUctDataWithoutVirtualLoss();
				passValueSD = it->getVNValueData();
			}
		}

		// If is the opponent's turn
		// * Output pass, only if root is win for the liver
		// * Output bestSD in the unknowwn nodes, otherwise
		if (toPlay == killColor && pRoot->isProved()) {
			bestMove = WeichiMove(toPlay);
			bestSD = passSD;
			bestValueSD = passValueSD;
		}

	} else {
		if ( pRoot->hasChildren() ) {
			UctNodePtr bestChild;
			WeichiMove secondBestMove = WeichiMove(toPlay); // generate pass with color = toPlay
			StatisticData secondBestSD;
			StatisticData secondBestValueSD;
			StatisticData passSD;
			StatisticData passValueSD;

			for ( UctChildIterator<WeichiUctNode> it(pRoot); it; ++it ) {
				if ( it->getUctDataWithoutVirtualLoss().getCount() > bestSD.getCount() ) {
					secondBestMove = bestMove;
					secondBestSD = bestSD;
					bestMove = it->getMove();
					bestSD = it->getUctDataWithoutVirtualLoss();
					secondBestValueSD = bestValueSD;
					bestValueSD = it->getVNValueData();
					bestChild = it;
				} else if ( it->getUctDataWithoutVirtualLoss().getCount() > secondBestSD.getCount() ) {
					secondBestMove = it->getMove();
					secondBestSD = it->getUctDataWithoutVirtualLoss() ;
					secondBestValueSD = it->getVNValueData();
				}
				if ( it->getMove().isPass() ) {
					passSD = it->getUctDataWithoutVirtualLoss();
					passValueSD = it->getVNValueData();
				}
			}

			if ( WeichiConfigure::EarlyPass ) {
				int neutralPosition;

				if ( bestSD.getMean() < WeichiConfigure::ResignWinrateThreshold ) {
					if ( checkStableForEarlyPass(state, toPlay, neutralPosition) ) {
						bestMove = WeichiMove(toPlay);
						bestSD = passSD;
						bestValueSD = passValueSD;
					}
				} else if ( bestSD.getMean() > WeichiConfigure::EarlyPassWinrateThreshold && bestMove.isPass() ) {
					if ( !checkStableForEarlyPass(state, toPlay, neutralPosition) ) {
						bestMove = secondBestMove;
						bestSD = secondBestSD;
						bestValueSD = secondBestValueSD;
						bestChild->getUctData().reset(); // bestChild is not best child, reset it
					}
				}

				/*if ( bestMove.isPass() && neutralPosition != PASS_MOVE.getPosition() ) {
					WeichiMove neutralMove(toPlay, neutralPosition);
					CERR() << "Play neutral point " << neutralMove.toGtpString() << " instead of pass" << endl;
					bestSD.reset();
					bestMove = neutralMove;
				}*/
			}
		}
	}

	if ( WeichiConfigure::CaptureDeadStones && bestMove.isPass() ) {
		const Territory& territory = WeichiGlobalInfo::getSearchInfo().m_passTerritory;

		for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
			Color c = state.m_board.getGrid(*it).getColor();

			// find opponent stone
			if ( c != AgainstColor(toPlay) ) continue;

			// skip if the stone is alive
			double prob = territory.getTerritory(*it) ;
			if ( prob <=  WeichiConfigure::TerritoryThreshold && c == COLOR_WHITE ) continue;
			if ( prob >= -WeichiConfigure::TerritoryThreshold && c == COLOR_BLACK ) continue;

			// found opponent dead stone, try to capture by playing its neighbor
			const WeichiBlock* deadBlock = state.m_board.getGrid(*it).getBlock();
			WeichiBitBoard bmLiberty = deadBlock->getStonenNbrMap() - state.m_board.getBitBoard();
			int killPos;
			while ( (killPos=bmLiberty.bitScanForward()) != -1 ) {
				WeichiMove killMove(toPlay, killPos);
				if ( state.m_board.isIllegalMove(killMove, state.m_ht) ) continue;

				CERR() << "Play capture point " << killMove.toGtpString() << " instead of pass" << endl;
				bestMove = killMove;
				bestSD.reset();
				bestValueSD.reset();
				break;
			}
			if ( !bestMove.isPass() ) break;
		} 
	}

	return bestMove ;
}

bool WeichiMoveDecisionMaker::checkStableForEarlyPass( const WeichiThreadState& state, Color toPlay, int& neutralPosition ) {
	assert(WeichiConfigure::EarlyPass);
	const Territory& territory = WeichiGlobalInfo::getSearchInfo().m_passTerritory;
	bool isStableForEarlyPass = true;
	neutralPosition = PASS_MOVE.getPosition();
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it && isStableForEarlyPass ; ++it ) {
		double prob = territory.getTerritory(*it);
		WeichiMove m(toPlay, *it);

		// skip seki points
		PredictPlayInfo myInfo;
		state.m_board.getPredictPlayInfoByPlay(m, myInfo);
		if( WeichiBadMoveReader::isMutualAtari(state.m_board, m, myInfo) ) {
			continue;
		}

		if ( !state.m_board.isIllegalMove(m, state.m_ht) ) {
			// check move is neutral point by checking neighbor's safety
			bool isSafeBlackNbr = false;
			bool isSafeWhiteNbr = false;
			const int* nbr = StaticBoard::getGrid(*it).getAdjacentNeighbors();
			for ( ; *nbr != -1 ; ++nbr ) {
				double nbrProb = territory.getTerritory(*nbr);
				if ( nbrProb > WeichiConfigure::TerritoryThreshold ) { isSafeBlackNbr = true; }
				if ( nbrProb < -WeichiConfigure::TerritoryThreshold ) { isSafeWhiteNbr = true; }
			}
			if ( isSafeBlackNbr && isSafeWhiteNbr ) {
				// move is neutral point, check safety of the move
				int neutralCandidate = *it;
				if ( checkNeutralPosition(state, toPlay, neutralCandidate) ) {
					neutralPosition = neutralCandidate;
				} else {
					isStableForEarlyPass = false;
				}
				continue;
			}
		}

		// every point need to be clear except neutral points
		if ( -WeichiConfigure::TerritoryThreshold <= prob && prob <= WeichiConfigure::TerritoryThreshold ) {
			CERR() << "No early pass (unclear point " << m.toGtpString() << "(score: " << prob << "))" << endl;
			isStableForEarlyPass = false;
		}
	}
	return isStableForEarlyPass;
}

bool WeichiMoveDecisionMaker::checkNeutralPosition( const WeichiThreadState& state, Color toPlay, int& neutralPosition )
{
	const WeichiMove m(toPlay, neutralPosition);
	if ( WeichiBadMoveReader::getBadMoveType(state.m_board, m) != BADMOVE_NOT_BADMOVE ) {
		CERR() << "No early pass (bad move for neutral " << m.toGtpString() << ")" << endl;
		return false;
	}

	const Territory& territory = WeichiGlobalInfo::getSearchInfo().m_passTerritory;
	const int* nbr = StaticBoard::getGrid(neutralPosition).getAdjacentNeighbors();
	for ( ; *nbr != -1; ++nbr ) {
		double nbrProb = territory.getTerritory(*nbr);
		if ( nbrProb > WeichiConfigure::TerritoryThreshold && state.m_board.getGrid(*nbr).getColor() != COLOR_BLACK ) {
			CERR() << "Endgame is not finished (around " << m.toGtpString() << ")." << endl;
			return false;
		}
		if ( nbrProb < -WeichiConfigure::TerritoryThreshold && state.m_board.getGrid(*nbr).getColor() != COLOR_WHITE ) {
			CERR() << "Endgame is not finished (around " << m.toGtpString() << ")." << endl;
			return false;
		}
	}

	WeichiBitBoard bmLiberty;
	state.m_board.getLibertyBitBoardAfterPlay(m, bmLiberty);
	int pos;
	int safePointCount = 0;
	int replacePosition;
	while ( (pos=bmLiberty.bitScanForward()) != -1 ) {
		if ( toPlay == COLOR_BLACK && territory.getTerritory(pos) > WeichiConfigure::TerritoryThreshold ) ++safePointCount, replacePosition = pos;
		if ( toPlay == COLOR_WHITE && territory.getTerritory(pos) < -WeichiConfigure::TerritoryThreshold ) ++safePointCount, replacePosition = pos;
	}
	if ( safePointCount == 0 ) return false;
	if ( safePointCount == 1 && !state.m_board.isIllegalMove(WeichiMove(toPlay, replacePosition), state.m_ht) ) {
		CERR() << "Neutral change: " << m.toGtpString() << " to " << WeichiMove(toPlay, replacePosition).toGtpString() << endl;
		neutralPosition = replacePosition;
	}

	return true;
} 
