#ifndef BASEPLAYOUTAGENT_H
#define BASEPLAYOUTAGENT_H

#include "types.h"

/*!
    @brief  the default implementation of PlayoutAgent
    @author T.F. Liao
*/
template<class _Move, class _PlayoutResult, class _ThreadGameState>
class BasePlayoutAgent
{
protected:
    _ThreadGameState& m_state ;

public:
    /*!
        @brief  constructor
        @author T.F. Liao
        @param  state [in] reference to _ThreadGameState
    */
    BasePlayoutAgent ( _ThreadGameState& state ) 
        : m_state ( state ) {}

    /*!
        @brief  playout the GameState
        @author T.F. Liao
        @return the simulation result of this playout
        the default implementation is:
            get playout move from GameState and play
            until the terminal state is reached
            return the evaluate result
    */
    _PlayoutResult run ( ) 
    {
        while ( !m_state.isTerminal() ) {
            _Move move = m_state.generatePlayoutMove() ;
            m_state.play ( move ) ;
        }
        // cerr << m_state.toString() << endl;
        return m_state.evaluate();
    }
};

#endif