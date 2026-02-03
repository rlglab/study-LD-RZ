#include "BitBoard.h"
#include "WeichiMove.h"

template<int _Bits>
class WeichiBitBoard : public BitBoard<_Bits>
{
public:
    /*!
        @brief  set particular bit on (backward compatible)
        @author T.F. Liao
        @param  xIdx [in] x-coordinate of particular bit
        @param  yIdx [in] y-coordinate of particular bit
    */
    void    SetBitOn(uint xIdx, uint yIdx) { SetBitOn(WeichiMove::toPosition(xIdx,yIdx)); }
    /*!
        @brief  set particular bit off (backward compatible)
        @author T.F. Liao
        @param  xIdx [in] x-coordinate of particular bit
        @param  yIdx [in] y-coordinate of particular bit
    */
    void    SetBitOff(uint xIdx, uint yIdx) { SetBitOff(WeichiMove::toPosition(xIdx,yIdx)); }
    /*!
        @brief  check if particular bit is on
        @author T.F. Liao
        @param  xIdx [in] x-coordinate of particular bit
        @param  yIdx [in] y-coordinate of particular bit
        @return true if the bit is on
    */
    bool    BitIsOn(uint xIdx, uint yIdx) const { return BitIsOn(WeichiMove::toPosition(xIdx,yIdx)); }
};