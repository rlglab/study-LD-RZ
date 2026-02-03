#ifndef FEATUREPTRLIST_H
#define FEATUREPTRLIST_H

#include "BasicType.h"

template<class _FeaturePtr, int _max_size, int _max_id=_max_size>
class FeaturePtrList
{
private:
    uint m_nFeature ;
    _FeaturePtr m_vFeaturePtr [_max_size] ;
    short m_vIndex [_max_id+1] ;
public:
    FeaturePtrList() 
    {
        clear();
    }
    FeaturePtrList & operator= ( const FeaturePtrList& rhs ){
        assert(sizeof(m_vIndex)==sizeof(rhs.m_vIndex));
        m_nFeature = rhs.m_nFeature;
        //memcpy(m_vFeaturePtr, rhs.m_vFeaturePtr, rhs.m_nFeature*sizeof(_FeaturePtr) );
        //memcpy(m_vIndex, rhs.m_vIndex, sizeof(m_vIndex));
        for(uint i=0 ; i<m_nFeature ; i++ ){
            m_vFeaturePtr[i] = rhs.m_vFeaturePtr[i];
        }
        assert(sizeof(m_vIndex)==sizeof(rhs.m_vIndex));
        for(uint i=0 ; i<_max_id+1 ; i++ ){
            m_vIndex[i] = rhs.m_vIndex[i];
        }
        return *this;
    }
    inline void addFeature ( _FeaturePtr ptr ) 
    {
        assert(invariance());
        uint id = ptr->GetID();
        if ( m_vIndex[id] != -1 ) return ;
        m_vFeaturePtr[m_nFeature] = ptr ;
        m_vIndex[id] = m_nFeature ++;
        assert(invariance());
    }

    inline void removeFeature ( _FeaturePtr ptr ) 
    {
        assert ( invariance() ) ;
        uint id = ptr->GetID();
        if ( m_vIndex[id] == -1 ) assert(false) ;
        int index = m_vIndex[id] ;
        _FeaturePtr last = m_vFeaturePtr[index] = m_vFeaturePtr[--m_nFeature];
        m_vIndex[last->GetID()] = index ;
        m_vIndex[id] = -1;
        assert ( invariance() ) ;
    }

    inline void removeFeature ( uint idx ) 
    {
        assert ( invariance() ) ;
        assert ( idx < size() ) ;
        uint id = m_vFeaturePtr[idx]->GetID();
        _FeaturePtr last = m_vFeaturePtr[idx] = m_vFeaturePtr[--m_nFeature];
        m_vIndex[last->GetID()] = idx ;
        m_vIndex[id] = -1;
        assert ( invariance() ) ;
    }

    const _FeaturePtr& operator[] ( size_t index ) const 
    {
        assert ( index < m_nFeature ) ;
        return m_vFeaturePtr[index] ;
    }
    
    _FeaturePtr& operator[] ( size_t index ) 
    {
        assert ( index < m_nFeature ) ;
        return m_vFeaturePtr[index] ;
    }

    bool contains ( _FeaturePtr ptr ) const 
    {
        uint id = ptr->GetID();
        assert ( id <= _max_id ) ;
        return ( m_vIndex[id] != -1 ) ;
    }

    uint getIndex ( _FeaturePtr ptr ) const 
    {
        assert(contains(ptr)) ;
        uint id = ptr->GetID();
        return m_vIndex[id] ;
    }

    inline uint size() const { return m_nFeature; }
    inline uint getNumFeature () const { return m_nFeature; }
    inline const uint* getFeaturePtrs () const { return m_vFeaturePtr ; }

    inline void clear() { m_nFeature = 0; memset ( m_vIndex, -1, sizeof m_vIndex ); }
private:
    bool invariance () 
    {
        uint count = 0;
        for ( uint i=0;i<=_max_id;++i ) {
            if ( m_vIndex[i] != -1 ) {
                int idx = m_vIndex[i] ;
                ++ count ;
                assert ( static_cast<uint>(idx) < m_nFeature ) ;
                assert ( m_vFeaturePtr[idx]->GetID() == i ) ;
            }
        }
        assert ( count == m_nFeature ) ;
        return true; 
    }
};


#endif // FEATUREPTRLIST_H 
