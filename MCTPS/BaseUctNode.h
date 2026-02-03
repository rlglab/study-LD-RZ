#ifndef _BASEUCTNODE_
#define _BASEUCTNODE_

#include "UctNodeImpl.h"

/*!
    @brief  the default implementation of UctNode
    @author T.F. Liao
*/
template<class _Move>
class BaseUctNode : public UctNodeImpl<_Move, BaseUctNode<_Move> >
{
};


#endif 
