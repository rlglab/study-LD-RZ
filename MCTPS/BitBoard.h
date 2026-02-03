#ifndef __BITBOARD_H__
#define __BITBOARD_H__

#include "Vector.h"
#include "types.h"
#include <string>
using std::string;

template<int _Rows, int _Columns>
class BitBoard
{
private:
	static const int BITS = _Rows * _Columns ;
	static const int BITS_OFFSET = 6 ; 
	static const int BITS_VAR = (1<<BITS_OFFSET) ;
	static const int BITS_MASK = BITS_VAR-1;
	static const int NVARS = (BITS+BITS_VAR-1)/BITS_VAR ;

	///< last variable may not fully used, should And with this mask on revert
	static const ull mask55 = 0x5555555555555555ULL ;
	static const ull mask33 = 0x3333333333333333ULL ;
	static const ull mask0f = 0x0f0f0f0f0f0f0f0fULL ;
	static const ull mask01 = 0x0101010101010101ULL ;
	static const ull mask00ff = 0x00ff00ff00ff00ffULL ;
	static const ull DeBruijnSequence = 0x218A392CD3D5DBFULL ;
	static bool isInitialized ;

	ull m_bitmap[NVARS] ;
	static uint BSFTable[64] ;
	static uint s_nvars ;
	static uint s_resetSize ;
	static ull s_lastVarMask ;
	static ull s_leftBoundary[NVARS] ;
	static ull s_rightBoundary[NVARS] ;
	static ull s_upBoundary[NVARS] ;
	static ull s_downBoundary[NVARS] ;

public:
	/*!
		@brief  initialze some variable
		@author T.F. Liao
		@author kwchen
	*/
	static void initialize () 
	{
		assert( !isInitialized );
		isInitialized = true ;

		// initialize BitScanForward Table
		for ( uint i=0;i<64;++i ) {
			BSFTable[((1ULL << i) * DeBruijnSequence) >> 58] = i;
		}

		// initialize boundary
		memset(s_leftBoundary, 0xff, sizeof(s_leftBoundary));
		memset(s_rightBoundary, 0xff, sizeof(s_rightBoundary));
		memset(s_upBoundary, 0xff, sizeof(s_upBoundary));
		memset(s_downBoundary, 0xff, sizeof(s_downBoundary));
		for( uint i=0 ; i<_Rows ; ++i ){
			uint position ;

			position = i * _Columns ;
			s_leftBoundary[position>>BITS_OFFSET] &= ~( 1ULL << (position&BITS_MASK) ) ;

			position = (i+1) * _Columns - 1 ;
			s_rightBoundary[position>>BITS_OFFSET] &= ~( 1ULL << (position&BITS_MASK) ) ;

			position = BITS - i - 1;
			s_upBoundary[position>>BITS_OFFSET] &= ~( 1ULL << (position&BITS_MASK) ) ;

			position = i ;
			s_downBoundary[position>>BITS_OFFSET] &= ~( 1ULL << (position&BITS_MASK) ) ;
		}

		reInitailize(_Rows);
	}

	static void reInitailize( uint boardSize )
	{
		uint bits = boardSize*_Columns;
		uint nvars = (bits+BITS_VAR)/BITS_VAR;

		s_nvars = nvars;
		s_resetSize = sizeof(ull) * s_nvars;
		s_lastVarMask = (1ULL<<(bits%BITS_VAR)) - 1;
	}

	/*!
		@brief  constructor
		@author T.F. Liao
	*/
	inline BitBoard() { Reset(); }

	// copy constructor & operator=
	inline BitBoard( const BitBoard<_Rows, _Columns>& rhs ) { operator=(rhs); }
	inline BitBoard<_Rows, _Columns>& operator= ( const BitBoard<_Rows, _Columns>& rhs )
	{
		memcpy(m_bitmap, rhs.m_bitmap, s_resetSize);
		return *this;
	}
	/*!
		@brief  destructor
		@author T.F. Liao
	*/
	~BitBoard() { }
	/*!
		@brief  clean bitmap
		@author T.F. Liao
	*/
	inline void Reset ( ) { memset ( m_bitmap, 0, s_resetSize ) ; }
	/*!
		@brief  set particular bit on
		@author T.F. Liao
		@param  position [in] position of particular bit
	*/
	inline void SetBitOn (uint position)
	{
		assert ( position < BITS ) ;
		m_bitmap[position>>BITS_OFFSET] |= ( 1ULL << (position&BITS_MASK) ) ;
	}
	/*!
		@brief  set particular bit off
		@author T.F. Liao
		@param  position [in] position of particular bit
	*/
	inline void SetBitOff(uint position)
	{
		assert ( position < BITS ) ;
		m_bitmap[position>>BITS_OFFSET] &= ~( 1ULL << (position&BITS_MASK) ) ;
	}
	/*!
		@brief  check if particular bit is on
		@author T.F. Liao
		@param  position [in] position of particular bit
		@return true if the bit is on
	*/
	inline bool BitIsOn(uint position) const 
	{
		assert ( position < BITS ) ;
		return ( m_bitmap[position>>BITS_OFFSET] & ( 1ULL << (position&BITS_MASK) ) ) != 0ULL ;
	}

   
	/*!
		@brief  or with another bitboard
		@author T.F. Liao
		@param  rhs [in] another bitboard
		@return reference to self (after or operator)
	*/
	inline BitBoard<_Rows, _Columns>& operator|= ( const BitBoard<_Rows, _Columns>& rhs )
	{
		for ( uint i=0;i<s_nvars;++i ) m_bitmap[i] |= rhs.m_bitmap[i] ; 
		return *this;
	}

	/*!
		@brief  or with another bitboard
		@author T.F. Liao
		@param  rhs [in] another bitboard
		@return reference to self (after or operator)
	*/
	inline BitBoard<_Rows, _Columns>& operator&= ( const BitBoard<_Rows, _Columns>& rhs )
	{
		for ( uint i=0;i<s_nvars;++i ) m_bitmap[i] &= rhs.m_bitmap[i] ; 
		return *this;
	}

	/*!
		@brief  remove all bits in another bitboard
		@author T.F. Liao
		@param  rhs [in] another bitboard
		@return reference to self (after remove operator)
	*/
	inline BitBoard<_Rows, _Columns>& operator-= ( const BitBoard<_Rows, _Columns>& rhs )
	{
		for ( uint i=0;i<s_nvars;++i ) m_bitmap[i] &= ~rhs.m_bitmap[i] ; 
		return *this;
	}

	/*!
		@brief  xor with another bitboard
		@author T.F. Liao
		@param  rhs [in] another bitboard
		@return reference to self (after xor operator)
	*/
	inline BitBoard<_Rows, _Columns>& operator^= ( const BitBoard<_Rows, _Columns>& rhs )
	{
		for ( uint i=0;i<s_nvars;++i ) m_bitmap[i] ^= rhs.m_bitmap[i] ; 
		return *this;
	}

	/*!
		@brief  invert all bit (create new bitboard)
		@author T.F. Liao
		@return bitboard that is invertion this bitboard
	*/
	inline BitBoard<_Rows, _Columns> operator~ () const 
	{
		BitBoard bm ;
		for ( uint i=0;i<s_nvars;++i ) bm.m_bitmap[i] = ~this->m_bitmap[i] ; 
		bm.m_bitmap[s_nvars-1] &= s_lastVarMask ;
		return bm;
	}

	/*!
		@brief  remove all bits in another bitboard (create new bitboard)
		@author 5D
		@param  rhs [in] another bitboard
		@return bitboard that is remove another bitboard
	*/
	inline BitBoard<_Rows, _Columns> operator- ( const BitBoard<_Rows, _Columns>& rhs ) const 
	{
		BitBoard bm = *this ;
		for ( uint i=0;i<s_nvars;++i ) bm.m_bitmap[i] &= ~rhs.m_bitmap[i] ; 
		return bm;
	}

	/*!
		@brief  and with another bitboard (create new bitboard)
		@author T.F. Liao
		@param  rhs [in] another bitboard
		@return bitboard that is intersection of two bitboard
	*/
	inline BitBoard<_Rows, _Columns> operator& ( const BitBoard<_Rows, _Columns>& rhs ) const 
	{
		BitBoard bm = *this ;
		for ( uint i=0;i<s_nvars;++i ) bm.m_bitmap[i] &= rhs.m_bitmap[i] ; 
		return bm;
	}

	/*!
		@brief  or with another bitboard (create new bitboard)
		@author T.F. Liao
		@param  rhs [in] another bitboard
		@return bitboard that is union of two bitboard
	*/
	inline BitBoard<_Rows, _Columns> operator| ( const BitBoard<_Rows, _Columns>& rhs ) const 
	{
		BitBoard bm = *this ;
		for ( uint i=0;i<s_nvars;++i ) bm.m_bitmap[i] |= rhs.m_bitmap[i] ; 
		return bm;
	}

	/*!
		@brief  xor with another bitboard (create new bitboard)
		@author T.F. Liao
		@param  rhs [in] another bitboard
		@return bitboard that is xor of two bitboard
	*/
	inline BitBoard<_Rows, _Columns> operator^ ( const BitBoard<_Rows, _Columns>& rhs ) const 
	{
		BitBoard bm = *this ;
		for ( uint i=0;i<s_nvars;++i ) bm.m_bitmap[i] ^= rhs.m_bitmap[i] ; 
		return bm;
	}

	/*!
		@brief  check if is equal with another bitboard
		@author T.F. Liao
		@param  rhs [in] another bitboard
		@return true if they are equal
	*/
	inline bool operator== ( const BitBoard<_Rows, _Columns>& rhs ) const 
	{
		bool equal = true ;
		for (uint i=0;i<s_nvars;++i )
			equal = equal && (this->m_bitmap[i]==rhs.m_bitmap[i]) ;
		return equal ;
	}

	/*!
		@brief  check if is not equal with another bitboard
		@author T.F. Liao
		@param  rhs [in] another bitboard
		@return true if they are not equal
	*/
	inline bool operator!= ( const BitBoard<_Rows, _Columns>& rhs ) const 
	{
		return !this->operator ==(rhs) ;
	}

	/*!
		@brief  all bit shift left (create new bitboard)
		@author kwchen
		@return bitboard of the result
	*/
	inline BitBoard<_Rows, _Columns> shiftLeft () const
	{
		BitBoard<_Rows, _Columns> bitboard = *this;
		for( uint i=0 ; i<s_nvars-1 ; ++i ){
			bitboard.m_bitmap[i] >>= 1;
			bitboard.m_bitmap[i] |= this->m_bitmap[i+1] << BITS_MASK ;
			bitboard.m_bitmap[i] &= s_rightBoundary[i] ;
		}
		bitboard.m_bitmap[s_nvars-1] >>= 1;
		bitboard.m_bitmap[s_nvars-1] &= s_rightBoundary[s_nvars-1] ;
		bitboard.m_bitmap[s_nvars-1] &= s_lastVarMask;
		return bitboard;
	}

	/*!
		@brief  all bit shift right (create new bitboard)
		@author kwchen
		@return bitboard of the result
	*/
	inline BitBoard<_Rows, _Columns> shiftRight () const
	{
		BitBoard<_Rows, _Columns> bitboard = *this;
		for( uint i=s_nvars-1 ; i>0 ; --i ){
			bitboard.m_bitmap[i] <<= 1;
			bitboard.m_bitmap[i] |= this->m_bitmap[i-1] >> BITS_MASK ;
			bitboard.m_bitmap[i] &= s_leftBoundary[i] ;
		}
		bitboard.m_bitmap[0] <<= 1;
		bitboard.m_bitmap[0] &= s_leftBoundary[0] ;
		bitboard.m_bitmap[s_nvars-1] &= s_lastVarMask;
		return bitboard;
	}

	/*!
		@brief  all bit shift up (create new bitboard)
		@author kwchen
		@return bitboard of the result
	*/
	inline BitBoard<_Rows, _Columns> shiftUp () const
	{
		BitBoard<_Rows, _Columns> bitboard = *this;
		for( uint i=s_nvars-1 ; i>0 ; --i ){
			bitboard.m_bitmap[i] <<= _Columns;
			bitboard.m_bitmap[i] |= this->m_bitmap[i-1] >> (BITS_MASK-_Columns+1) ;
			bitboard.m_bitmap[i] &= s_downBoundary[i] ;
		}
		bitboard.m_bitmap[0] <<= _Columns;
		bitboard.m_bitmap[0] &= s_downBoundary[0] ;
		bitboard.m_bitmap[s_nvars-1] &= s_lastVarMask;
		return bitboard;
	}

	/*!
		@brief  all bit shift down (create new bitboard)
		@author kwchen
		@return bitboard of the result
	*/
	inline BitBoard<_Rows, _Columns> shiftDown () const
	{
		BitBoard<_Rows, _Columns> bitboard = *this;
		for( uint i=0 ; i<s_nvars-1 ; ++i ){
			bitboard.m_bitmap[i] >>= _Columns;
			bitboard.m_bitmap[i] |= this->m_bitmap[i+1] << (BITS_MASK-_Columns+1) ;
			bitboard.m_bitmap[i] &= s_upBoundary[i] ;
		}
		bitboard.m_bitmap[s_nvars-1] >>= _Columns;
		bitboard.m_bitmap[s_nvars-1] &= s_upBoundary[s_nvars-1] ;
		bitboard.m_bitmap[s_nvars-1] &= s_lastVarMask;
		return bitboard;
	}

	/*!
		@brief  dilate the bitboard (create new bitboard)
		@author kwchen
		@return dilation of the bitboard
	*/
	inline BitBoard<_Rows, _Columns> dilate () const
	{
		BitBoard<_Rows, _Columns> bitboard = *this;
		bitboard |= this->shiftDown() ;
		bitboard |= this->shiftUp() ;
		bitboard |= this->shiftLeft() ;
		bitboard |= this->shiftRight() ;
		return bitboard;
	}

	/*!
		@brief  erode the bitboard (create new bitboard)
		@author kwchen
		@return erosion of the bitboard
	*/
	inline BitBoard<_Rows, _Columns> erode () const
	{
		BitBoard<_Rows, _Columns> bitboard = *this;
		bitboard &= this->shiftDown() ;
		bitboard &= this->shiftUp() ;
		bitboard &= this->shiftLeft() ;
		bitboard &= this->shiftRight() ;
		return bitboard;
	}

	/*!
		@brief  find the area consist of the start position
		@author kwchen
		@param  position  [in] start position
		@param  bmResult  [out] bit is set to 1 if the position is in the same area of the start position, 0 otherwise
		@param  border    [in] bmResult cannot overlap with border
		@param  stopTimes [in] expand times cannot greater than stopTimes
		@return true if operation is completed, false otherwise
	*/
	inline bool floodfill ( const uint position, BitBoard<_Rows, _Columns>& bmResult, const BitBoard<_Rows, _Columns>& border=BitBoard(), uint stopTimes=BITS )
	{
		bmResult.SetBitOn(position) ;
		Vector<uint, NVARS> stack ;
		stack.push_back( position >> BITS_OFFSET ) ;
		bool inStack[NVARS] = {};
		uint fillTimes = 0;

		//mask off border
		for( uint i=0; i<s_nvars; i++ ) {
			m_bitmap[i] &= s_rightBoundary[i];
		}

		while ( !stack.empty() ) {
			const uint index = stack.back();
			stack.pop_back();
			inStack[index] = false ;
			ull originalBitmap ;

			// in-place floodfill
			do {
				originalBitmap = bmResult.m_bitmap[index] ;
				bmResult.m_bitmap[index] |= originalBitmap >> 1 ;
				bmResult.m_bitmap[index] |= originalBitmap << 1 ;
				bmResult.m_bitmap[index] |= originalBitmap >> _Columns ;
				bmResult.m_bitmap[index] |= originalBitmap << _Columns ;
				bmResult.m_bitmap[index] &= m_bitmap[index] ;
				++ fillTimes ;
				if( fillTimes>=stopTimes || (bmResult.m_bitmap[index] & border.m_bitmap[index]) ) { return false; }
			} while ( originalBitmap != bmResult.m_bitmap[index] ) ;

			// try to floodfill to previous bitmap
			if ( index > 0 ) {
				const uint prevIndex = index - 1;
				originalBitmap = bmResult.m_bitmap[prevIndex] ;
				bmResult.m_bitmap[prevIndex] |= bmResult.m_bitmap[index] << (BITS_MASK-_Columns+1) ;
				bmResult.m_bitmap[prevIndex] |= bmResult.m_bitmap[index] << (BITS_MASK);
				bmResult.m_bitmap[prevIndex] &= m_bitmap[prevIndex] ;
				if( bmResult.m_bitmap[prevIndex] != originalBitmap && !inStack[prevIndex] ) {
					inStack[prevIndex] = true ;
					stack.push_back(prevIndex) ;
				}
			}

			// try to floodfill to next bitmap
			if ( index < s_nvars - 1 ) {
				const uint nextIndex = index + 1;
				originalBitmap = bmResult.m_bitmap[nextIndex] ;
				bmResult.m_bitmap[nextIndex] |= bmResult.m_bitmap[index] >> (BITS_MASK-_Columns+1) ;
				bmResult.m_bitmap[nextIndex] |= bmResult.m_bitmap[index] >> (BITS_MASK);
				bmResult.m_bitmap[nextIndex] &= m_bitmap[nextIndex] ;
				if( bmResult.m_bitmap[nextIndex] != originalBitmap && !inStack[nextIndex] ) {
					inStack[nextIndex] = true ;
					stack.push_back(nextIndex) ;
				}
			}
		}

		return true;
	}

	/*!
		@brief  check bitboard is empty
		@author T.F. Liao
		@return true if bitboard is empty (no bit on)
	*/
	inline bool empty () const 
	{
		ull OR = 0ULL ;
		for ( uint i=0;i<s_nvars;++i ) OR |= m_bitmap[i]; 
		return (OR == 0ULL) ;
	}

	/*!
		@brief  count number of bits on
		@author T.F. Liao
		@return number of on-bits
	*/
	inline uint bitCount () const 
	{
		uint bc = 0 ;
		for ( uint i=0;i<s_nvars;++i ) 
			bc += count ( m_bitmap[i] ) ;
		return bc ;
	}

	/*!
		@brief  scan bitboard & set off returned bit
		@author T.F. Liao
		@return position of an on-bit
	*/
	inline uint bitScanForward ( )
	{
		for ( uint i=0;i<s_nvars;++i ) {
			if ( m_bitmap[i] ) {
				return (i<<BITS_OFFSET) + bitScan(m_bitmap[i]) ;
			}
		}
		return -1 ;
	}

	/*!
		@brief  scan all on-bits store to array
		@author T.F. Liao
		@param  vPos    [out] buffer for position of on-bits to be stored
		@return number of on-bits
	*/
	inline uint bitScanAll ( uint *vPos ) const 
	{
		int len = 0 ;
		for ( uint i=0;i<s_nvars;++i ) {
			if ( m_bitmap[i] ) {
				ull b = m_bitmap[i] ;
				while ( b ) {
					vPos[len++] = (i<<BITS_OFFSET) + bitScan(b) ;
				}
			}
		}
		return len ;
	}

	/*!
		@brief  scan all on-bits store to array
		@author T.F. Liao
		@param  vPos [out] buffer for position of on-bits to be stored
	*/
	template<int _BufSize>
	inline void bitScanAll ( Vector<uint, _BufSize>& vPos ) const 
	{
		assert(_BufSize >= bitCount());
		vPos.clear();
		for ( uint i=0;i<s_nvars;++i ) {
			if ( m_bitmap[i] ) {
				ull b = m_bitmap[i] ;
				while ( b ) {
					vPos.push_back((i<<BITS_OFFSET) + bitScan(b)) ;
				}
			}
		}
	}

	inline bool hasIntersection(const BitBoard<_Rows, _Columns>& rhs) const 
	{
		ull OR = 0;
		for ( uint i=0;i<s_nvars;++i ) {
			OR |= (m_bitmap[i]&rhs.m_bitmap[i]) ;
		}
		return (OR!=0ULL);
	}

	inline bool isSubsetOf( const BitBoard<_Rows, _Columns>& rhs ) const { return ((*this)-rhs).empty() ; }

	inline uint getIntersectionCount(const BitBoard<_Rows, _Columns>& rhs) const 
	{
		uint cnt = 0;
		for ( uint i=0;i<s_nvars;++i ) {
			cnt += count(m_bitmap[i]&rhs.m_bitmap[i]) ;
		}
		return cnt ;
	}

	inline uint positionOfNthRank ( uint rank )
	{
		assert( 1 <= rank && rank <= bitCount() ) ;
		uint cnt;
		for ( uint i=0;i<s_nvars;++i ) {
			cnt = count(m_bitmap[i]) ;
			if ( cnt >= rank )
				return (i<<BITS_OFFSET) + positionOfNthRank( m_bitmap[i], rank );
			rank -= cnt;
		}
		assert( false ) ;
		return -1;
	}

	inline uint getNumArea ( uint stopTimes = BITS )
	{
		BitBoard<_Rows, _Columns> bitboard = *this;
		BitBoard<_Rows, _Columns> result;
		uint pos;
		uint numArea = 0;
		while ( (pos=bitboard.bitScanForward()) != -1 ) {
			++numArea;
			if ( numArea >= stopTimes ) { break; }
			bitboard.SetBitOn(pos);
			bitboard.floodfill(pos, result);
			bitboard -= result;
		}
		return numArea;
	}

	string toString ( ) const 
	{
		return toString( _Rows, _Columns ) ;
	}

	string toString ( int nrow, int ncol ) const 
	{
		string result ;
		for ( int r = nrow-1 ; r >= 0; -- r ) {
			for ( int c = 0 ; c < ncol ; ++ c ) {
				if ( BitIsOn(r*_Columns+c) ) { result.append("1") ; }
				else { result.append("0") ; }
			}
			result.append("\n") ;
		}
		return result ;
	}

	string serialize () const
	{
		string result ;

		// bitboard to hex
		uint encode = 0;
		uint encodeCount = 0;
		for ( int r = _Rows-1 ; r >= 0; -- r ) {
			for ( int c = 0 ; c < _Columns ; ++ c ) {
				const uint index = r*_Columns+c;
				encodeCount++;
				encode <<= 1;
				if ( BitIsOn(index) ) { encode++; }

				if ( encodeCount == 4 ) {
					encodeCount = 0;
					if ( encode <= 9 ) { result.push_back('0' + encode); }
					else { result.push_back('A' + encode - 10); }
					encode = 0;
				}
			}
		}

		// last hex character
		while ( encodeCount > 0 && encodeCount < 4 ) {
			encodeCount++;
			encode <<= 1;
		}
		if ( encodeCount == 4 ) {
			if ( encode <= 9 ) { result.push_back('0' + encode); }
			else { result.push_back('A' + encode - 10); }
		}

		return result ;
	}

	void parse ( string str )
	{
		// hex to binary
		vector<uint> vDecodes;
		for ( uint i=0; i<str.size() && vDecodes.size() < BITS; ++i ) {
			uint decode = 0;
			if ( '0' <= str[i] && str[i] <= '9' ) { decode = str[i] - '0'; }
			else if ( 'A' <= str[i] && str[i] <= 'F' ) { decode = str[i] - 'A' + 10; }
			else { assert(false); return; }

			vDecodes.push_back(decode & 0x8);
			vDecodes.push_back(decode & 0x4);
			vDecodes.push_back(decode & 0x2);
			vDecodes.push_back(decode & 0x1);
		}

		// binary to bitboard
		Reset();
		uint i = 0;
		for ( int r = _Rows-1 ; r >= 0; -- r ) {
			for ( int c = 0 ; c < _Columns ; ++ c ) {
				const uint index = r*_Columns+c;
				assert ( i < vDecodes.size() );
				if ( vDecodes[i] != 0 ) { SetBitOn(index); }
				++i;
			}
		}
	}

private:

	/*!
		@brief  count number of bits on for a 64-bit variable
		@author T.F. Liao
		@return number of on-bits
	*/
	inline static uint count ( ull var ) 
	{
#if defined _MSC_VER && defined _M_X64
		return static_cast<uint>( __popcnt64( var ) );
#elif defined __GNUC__
		return static_cast<uint>( __builtin_popcountll( var ) );
#else
		var -= ( var >> 1 ) & mask55 ;
		var = ( var & mask33 ) + ( ( var >> 2 ) & mask33 );
		return ((( var + (var >> 4) ) & mask0f ) * mask01 ) >> 56 ;
#endif
		
	}

	/*!
		@brief  get position of an on-bit (also change bitboard)
		@author T.F. Liao
		@return position of an on-bit
	*/
	inline static uint bitScan ( ull & b ) 
	{
		assert ( isInitialized ) ;

#if defined _MSC_VER && defined _M_X64
		unsigned long position ;
		_BitScanForward64( &position, b ) ;
		b &= ~( 1ULL << position ) ;
		return position ;
#elif defined __GNUC__
		unsigned long position ;
		position = __builtin_ffsll( b );
		-- position ;
		b &= ~( 1ULL << position ) ;
		return position ;
#else
		/// least significant bit 
		ull lsb = (b&-(long long)b) ;
		b &= ~lsb ;
		return BSFTable[(lsb*DeBruijnSequence)>>58] ;
#endif
	}

	/*
		@brief Select the bit position with the given count (rank)
				modified from http://graphics.stanford.edu/~seander/bithacks.html
		@author kwchen
		@return the position of specified on-bit
	*/
	inline static uint positionOfNthRank ( ull b, uint rank )
	{
		assert( 1 <= rank && rank <= count(b) ) ;
		unsigned int position;           // Output: Resulting position of bit with rank r [1-64]
		unsigned int count;              // Bit count temporary.
		ull val1, val2, val3, val4; // Intermediate temporaries for bit count.

		// Do a normal parallel bit count for a 64-bit integer,
		// but store all intermediate steps.
		val1 =  b - ((b >> 1) & mask55);
		val2 = (val1 & mask33) + ((val1 >> 2) & mask33);
		val3 = (val2 + (val2 >> 4)) & mask0f;
		val4 = (val3 + (val3 >> 8)) & mask00ff;
		count = ((val4 >> 32) + (val4 >> 48)) & 0xffff;

		// Now do select!
		position  = 64;
		//if (rank > count) {position -= 32; rank -= count;}
		position -= ((count - rank) & 256) >> 3; rank -= (count & ((count - rank) >> 8));
		count  = (val4 >> (position - 16)) & 0xff;
		//if (rank > count) {position -= 16; rank -= count;}
		position -= ((count - rank) & 256) >> 4; rank -= (count & ((count - rank) >> 8));
		count  = (val3 >> (position - 8)) & 0xf;
		//if (rank > count) {position -= 8; rank -= count;}
		position -= ((count - rank) & 256) >> 5; rank -= (count & ((count - rank) >> 8));
		count  = (val2 >> (position - 4)) & 0x7;
		//if (rank > count) {position -= 4; rank -= count;}
		position -= ((count - rank) & 256) >> 6; rank -= (count & ((count - rank) >> 8));
		count  = (val1 >> (position - 2)) & 0x3;
		//if (rank > count) {position -= 2; rank -= count;}
		position -= ((count - rank) & 256) >> 7; rank -= (count & ((count - rank) >> 8));
		count  = (b >> (position - 1)) & 0x1;
		//if (rank > count) position--;
		position -= ((count - rank) & 256) >> 8;
        
		assert( 1 <= position && position <= 64 ) ;

		return position - 1 ;
	}
};

template<int _Rows, int _Columns>
uint BitBoard<_Rows, _Columns>::BSFTable[64] ;

template<int _Rows, int _Columns>
uint BitBoard<_Rows, _Columns>::s_nvars ;

template<int _Rows, int _Columns>
uint BitBoard<_Rows, _Columns>::s_resetSize ;

template<int _Rows, int _Columns>
ull BitBoard<_Rows, _Columns>::s_lastVarMask ;

template<int _Rows, int _Columns>
ull BitBoard<_Rows, _Columns>::s_leftBoundary[NVARS] ;

template<int _Rows, int _Columns>
ull BitBoard<_Rows, _Columns>::s_rightBoundary[NVARS] ;

template<int _Rows, int _Columns>
ull BitBoard<_Rows, _Columns>::s_upBoundary[NVARS] ;

template<int _Rows, int _Columns>
ull BitBoard<_Rows, _Columns>::s_downBoundary[NVARS] ;

template<int _Rows, int _Columns>
bool BitBoard<_Rows, _Columns>::isInitialized = false ;

#endif 
