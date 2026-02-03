#ifndef WEICHENDGAMECONDITION_H
#define WEICHENDGAMECONDITION_H

#include "BasicType.h"
#include "Color.h"
#include "Dual.h"
#include "WeichiBoard.h"
#include "WeichiSafetySolver.h"

class WeichiThreadState;

class WeichiEndGameCondition
{

public:
	friend class WeichiLifeDeathGtpEngine;

public:
	enum WeichiGroupCondition {
		CONDITION_AND,	
		CONDITION_OR,

		CONDITION_SIZE	// total size, add new element before this one
	};

public:
	WeichiEndGameCondition() {
		reset();
	}

	void reset();
	bool isEndGame(WeichiThreadState& state, Color ownColor);
	WeichiBitBoard getEndGameRZone(WeichiThreadState& state, Color ownColor);
	void setCrucialStones(WeichiThreadState& state, Color color, string sCrucialStone);
	WeichiBitBoard getCrucialStonesBitBoard();
	WeichiBitBoard getCrucialBlocksBitBoard();
	WeichiBitBoard getCrucialStones(Color color);
	string getCruicalStoneSgfString(Color color);
	string getCruicalStoneEditorString(Color color);
	string getCruicalGroupInfo(Color color);
	
	WeichiSafetySolver& getSafetySolver() { return m_safetySolver; }
	void setBoardCrucialStone(WeichiThreadState& state);
	void setSearchGoal(Color color, string sSearchGoal);
	void setRegion(string sRegion);
	inline WeichiBitBoard getRegion() { return m_bmReigon; }
	string toString();

	inline string getSearchGoalString(Color color) { return getWeichiSearchGoalString(m_searchgoal.get(color)); }

	inline string toGroupConditionString(WeichiGroupCondition status) {
		switch (status) {	
			case CONDITION_AND:		return "CONDITION_AND";
			case CONDITION_OR:		return "CONDITION_OR";

			default:
				//should not happen
				assert( false );
				return "error condition type!";
		}	
	}

private:
	bool isAlive(WeichiThreadState& state, const WeichiBlock* block);
	WeichiBitBoard getDeadBlockRZone(WeichiThreadState& state, const WeichiBlock* block);
	WeichiBitBoard getAliveBlockRZone(WeichiThreadState& state, const WeichiBlock* block);

private:
	WeichiSafetySolver m_safetySolver;
	WeichiBitBoard m_bmCrucialBlock; // TO DO: generalize it.	
	WeichiBitBoard m_bmReigon;	
	Dual<Vector<WeichiBitBoard, MAX_NUM_BLOCKS>> m_crucialGroups;	
	Dual<WeichiSearchGoal> m_searchgoal;	
	Dual<WeichiGroupCondition> m_condition;
	Dual<WeichiBitBoard> m_captureHistory;
};

#endif 
