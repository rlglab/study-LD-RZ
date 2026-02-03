#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <cassert>
#include "types.h"
#include <algorithm>
#include <stdlib.h>

template<typename T, int _BufSize>
class Vector
{

private:
    static const int buf_size = _BufSize + 1 ;

private:
    T buffer[buf_size] ;
    uint _size ;

public:
    Vector() : _size(0) {}
    Vector(T* v, size_t sz) ;
    Vector( const Vector& fv ) ;
    Vector& operator=( const Vector& fv ) ;

    inline uint size() const { return _size ; }
    inline size_t max_size() const { return buf_size; }

    inline bool empty() const { return size()==0; }

    inline T& operator[] ( size_t n ) ;
    inline const T& operator[] ( size_t n ) const ;

    inline T& at ( size_t n ) ;
    inline const T& at ( size_t n ) const ;

    inline T& front ( ) ;
    inline const T& front ( ) const ;

    inline T& back ( ) ;
    inline const T& back ( ) const ;

    inline void push_back ( const T& x ) ;
    inline void pop_back ( ) ;

    inline void resize ( size_t new_size ) ;
    inline void clear ( ) ;

    inline T* begin () ;
    inline T* end() ;

    /*!
        @brief  swap elements within two index
        @author T.F. Liao
        @param  idx1    [in]    index to one of element to be swap
        @param  idx2    [in]    index to another element to be swap
    */
    inline void swap ( size_t idx1, size_t idx2 ) ;
    /*!
        @brief  erase the element in \idx without keep other element in order
        @author T.F. Liao
        @param  idx    [in]    index to element to be erase
    */
    inline void erase_no_order( size_t idx ) ;

    /*!
        @brief  randomly select an element in this Vector
        @author T.F. Liao
        @return const reference to the selected element
    */
    const T& randSelect ( ) const ;
    
    /*!
        @brief  set all elements in the range [0 .. n-1] as particular value
        @author T.F. Liao
        @param  val [in]    value to be set
        @param  new_size    [in]    right bound of range to be set
    */
    void setAllAs ( const T& val, uint new_size = _BufSize  ) ;

};

template<typename T, int _BufSize>
Vector<T,_BufSize>::Vector( const Vector<T,_BufSize>& fv ) 
{
    _size = fv._size;
    std::copy(fv.buffer, fv.buffer+_size, buffer) ;
}

template<typename T, int _BufSize>
Vector<T, _BufSize>::Vector( T* v, size_t sz )
{
    assert ( sz < _BufSize ) ;
    _size = sz ;
    std::copy(v, v+_size, buffer) ;
}

template<typename T, int _BufSize>
Vector<T,_BufSize>& Vector<T,_BufSize>::operator=( const Vector<T,_BufSize>& fv ) 
{
    _size = fv._size;
    std::copy(fv.buffer, fv.buffer+_size, buffer) ;
    return *this;
}

template<typename T, int _BufSize>
const T& Vector<T,_BufSize>::randSelect ( ) const 
{
    assert ( !empty() ) ;
    return at ( Random::nextInt(size()) ) ;
}

template<typename T, int _BufSize>
T& Vector<T,_BufSize>::operator[] ( size_t n ) 
{
    assert ( n>=0 && n<size() );
    return buffer[n];
}

template<typename T, int _BufSize>
const T& Vector<T,_BufSize>::operator[] ( size_t n ) const 
{
    assert ( n>=0 && n<size() );
    return buffer[n];
}

template<typename T, int _BufSize>
T& Vector<T,_BufSize>::at ( size_t n ) 
{
    assert ( n>=0 && n<size() );
    return buffer[n];
}

template<typename T, int _BufSize>
const T& Vector<T,_BufSize>::at ( size_t n ) const 
{
    assert ( n>=0 && n<size() );
    return buffer[n];
}

template<typename T, int _BufSize>
T& Vector<T,_BufSize>::front ( ) 
{
    assert ( !empty() ) ;
    return buffer[0];
}

template<typename T, int _BufSize>
const T& Vector<T,_BufSize>::front ( ) const 
{
    assert ( !empty() ) ;
    return buffer[0];
}


template<typename T, int _BufSize>
T& Vector<T,_BufSize>::back ( ) 
{
    assert ( size() > 0 );
    return buffer[size()-1];
}

template<typename T, int _BufSize>
const T& Vector<T,_BufSize>::back ( ) const 
{
    assert ( !empty() ) ;
    return buffer[size()-1];
}

template<typename T, int _BufSize>
void Vector<T,_BufSize>::push_back ( const T& x ) 
{
    assert ( size() < buf_size );
    buffer[_size++] = x;
}
template<typename T, int _BufSize>
void Vector<T,_BufSize>::pop_back ( ) 
{
    assert ( !empty() ) ;
    -- _size;
}
template<typename T, int _BufSize>
void Vector<T,_BufSize>::resize ( size_t new_size ) 
{
    assert ( new_size >= 0 && new_size < buf_size );
    _size = (uint)new_size ;
}

template<typename T, int _BufSize>
void Vector<T,_BufSize>::clear ( ) 
{
    _size = 0;
}

template<typename T, int _BufSize>
inline T* Vector<T,_BufSize>::begin () 
{
    return buffer ;
}
template<typename T, int _BufSize>
inline T* Vector<T,_BufSize>::end() 
{
    return buffer+_size;
}

template<typename T, int _BufSize>
void Vector<T,_BufSize>::setAllAs ( const T& val, uint new_size) 
{
    assert ( new_size >= 0 && new_size < buf_size ) ;
    _size = new_size;
    for ( size_t i=0 ; i < _size ; ++i ) {
        buffer[i] = val;
    }
}


template<typename T, int _BufSize>
void Vector<T, _BufSize>::swap( size_t idx1, size_t idx2 )
{
    assert ( idx1<size() && idx2<size() ) ;
    T tmp = buffer[idx1];
    buffer[idx1] = buffer[idx2];
    buffer[idx2] = tmp ;
}

template<typename T, int _BufSize>
void Vector<T, _BufSize>::erase_no_order( size_t idx )
{
    assert(idx<size()) ;
    buffer[idx] = buffer[--_size] ;
}

#endif // Vector_H 
