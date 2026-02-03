#ifndef WEICHICANDIDATE_H
#define WEICHICANDIDATE_H

#include "BasicType.h"
#include "BitBoard.h"

class CandidateList
{
private:
    uint m_nCandidates ;
    uint m_vCandidates [MAX_NUM_GRIDS] ;
    short m_vIndex [MAX_NUM_GRIDS] ;
public:
    CandidateList()
    {
        m_nCandidates = 0 ;
        memset ( m_vIndex, -1, sizeof m_vIndex );
    }
    CandidateList ( const WeichiBitBoard& bitboard ) 
    {
        Vector<uint,MAX_NUM_GRIDS> vCand ;
        bitboard.bitScanAll(vCand) ;
        m_nCandidates = vCand.size();

        // m_nCandidates = bitboard.bitScanAll(m_vCandidates) ;
        memset ( m_vIndex, -1, sizeof m_vIndex );
        for ( uint i=0;i<m_nCandidates; ++ i ) {
            m_vCandidates[i] = vCand[i] ;
            m_vIndex[m_vCandidates[i]] = i ;
        }
    }

    inline void addCandidate ( uint pos ) 
    {
        assert ( invariance() ) ;
        if ( m_vIndex[pos] != -1 ) return ;
        m_vCandidates[m_nCandidates] = pos ;
        m_vIndex[pos] = m_nCandidates ++;
        assert ( invariance() ) ;
    }

    inline void removeCandidate ( uint pos ) 
    {
        assert ( invariance() ) ;
        if ( m_vIndex[pos] == -1 ) return ;
        int index = m_vIndex[pos] ;
        int last = m_vCandidates[index] = m_vCandidates[--m_nCandidates];
        m_vIndex[last] = index ;
        m_vIndex[pos] = -1;
        assert ( invariance() ) ;
    }

    void swap ( uint idx1, uint idx2 ) 
    {
        assert ( invariance() ) ;

        assert (idx1<m_nCandidates && idx2<m_nCandidates) ;

        { // exchange index
            uint pos1 = m_vCandidates[idx1],
                pos2 = m_vCandidates[idx2] ;
            short tmp = m_vIndex[pos1] ;
            m_vIndex[pos1] = m_vIndex[pos2];
            m_vIndex[pos2] = tmp ;
        }
        { // exchange value
            uint tmp = m_vCandidates[idx1];
            m_vCandidates[idx1] = m_vCandidates[idx2];
            m_vCandidates[idx2] = tmp ; 
        }

        assert ( invariance() ) ;
    }


    uint operator[] ( size_t index ) const 
    {
        assert ( index < m_nCandidates ) ;
        return m_vCandidates[index] ;
    }
    bool contains ( uint candidate ) const 
    {
        assert ( candidate < MAX_NUM_GRIDS ) ;
        return ( m_vIndex[candidate] != -1 ) ;
    }

    inline uint size() const { return m_nCandidates; }
    inline uint getNumCandidate () const { return m_nCandidates; }
    inline const uint* getCandidates () const { return m_vCandidates ; }

    void printCandidates () 
    {
        printf ( "candidates(%d): ", m_nCandidates ) ;
        for ( uint i=0;i<m_nCandidates;++i ) {
            printf ( "%d ", m_vCandidates[i] ) ;
        } printf ( "\n") ;
    }
    bool invariance () 
    {
        uint count = 0;
        for ( uint i=0;i<MAX_NUM_GRIDS;++i ) {
            if ( m_vIndex[i] != -1 ) {
                int idx = m_vIndex[i] ;
                ++ count ;
                assert ( m_vCandidates[idx] == i ) ;
                assert ( static_cast<uint>(idx) < m_nCandidates ) ;
            }
        }
        assert ( count == m_nCandidates ) ;
        return true; 
    }

};


#endif 