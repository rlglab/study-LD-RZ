#include "WeichiStopCondition.h"
#include "UctChildIterator.h"

bool WeichiStopCondition::operator()(UctNodePtr root, const WeichiThreadState& state, int sim_count, float sim_time, bool isGeneratingMove)
{
	if ( !Configure::Pondering || isGeneratingMove ) {
		if ( Configure::SimCtrl == Configure::SIMCTRL_TIME && sim_time > Configure::SimulationTimeLimit ) {
			CERR() << "Time is over (" << sim_time << ")" << endl;
			return true;
		}
		if ( !WeichiConfigure::EarlyAbort ) return false;
		if ( Configure::SimCtrl == Configure::SIMCTRL_COUNT && sim_count < WeichiConfigure::EarlyAbortCountThreshold ) return false;
		if ( Configure::SimCtrl == Configure::SIMCTRL_TIME && sim_time/Configure::SimulationTimeLimit < WeichiConfigure::EarlyAbortTimeRatio ) return false;

		// find top 2 nodes
		StatisticData best;
		StatisticData secondBest;
		for ( UctChildIterator<WeichiUctNode> it(root) ; it ; ++it ) {
			const StatisticData& childUctData = it->getUctData();
			if ( childUctData.getCount() > best.getCount() ) {
				secondBest = best;
				best = childUctData;
			} else if ( childUctData.getCount() > secondBest.getCount() ) {
				secondBest = childUctData;
			}
		}

		// check early abort condition
		if ( Configure::SimCtrl == Configure::SIMCTRL_COUNT ) {
			if ( best.getCount() - secondBest.getCount() > Configure::SimulationCountLimit - sim_count ) {
				WeichiGlobalInfo::getTreeInfo().m_bIsEarlyAbort = true;
				return true ;
			}
		} else if ( Configure::SimCtrl == Configure::SIMCTRL_TIME ) {
			float speed = sim_count/sim_time;
			if ( (best.getCount() - secondBest.getCount()) / speed > Configure::SimulationTimeLimit - sim_time ) {
				WeichiGlobalInfo::getTreeInfo().m_bIsEarlyAbort = true;
				return true ;
			}
		} else if( Configure::SimCtrl == Configure::SIMCTRL_MAXNODE_COUNT ) {
			// do nothing
		}
	}
	return false;
}