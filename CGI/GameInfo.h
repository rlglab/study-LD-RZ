#ifndef GAMEINFO_H
#define GAMEINFO_H

#include "WeichiThreadState.h"

class GameInfo {
private:
	uint m_id;
	bool m_bIsValid;
	WeichiThreadState *m_state;

public:
	GameInfo( uint id )
	{
		m_id = id;
		m_bIsValid = false;
		m_state = new WeichiThreadState();
	}

	~GameInfo() {
		delete m_state;
	}

	inline uint getID() const { return m_id; }
	inline void setID( uint id ) { m_id = id; }
	inline bool isValid() const { return m_bIsValid; }
	inline void setValid( bool bValid ) { m_bIsValid = bValid; }
	inline void copyState( const WeichiThreadState& state ) {
		m_state->resetThreadState();
		const Vector<WeichiMove,MAX_NUM_GRIDS>& vPresets = state.m_board.getPresetList();
		const Vector<WeichiMove,MAX_GAME_LENGTH>& vMoves = state.m_board.getMoveList();
		for( uint i=0; i<vPresets.size(); i++ ) {
			const WeichiMove& move = vPresets[i];
			m_state->play(move,true);
		}
		for( uint i=0; i<vMoves.size(); i++ ) {
			const WeichiMove& move = vMoves[i];
			m_state->play(move,true);
		}
	}
	inline WeichiThreadState* getState() { return m_state; }
	inline const WeichiThreadState* getState() const { return m_state; }
};

#endif 
