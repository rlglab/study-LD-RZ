#include "MoveLocation.h"

MoveLocation::MoveLocation(void):
x(-1), y(-1), x2(-1), y2(-1), m_iSize(1)
{
}

MoveLocation::MoveLocation(const int& x, const int& y):
x(x), y(y), x2(-1), y2(-1), m_iSize(1)
{
}

MoveLocation::MoveLocation(const int& x, const int& y, const int& x2, const int& y2, bool bSort):
x(x), y(y), x2(x2), y2(y2), m_iSize(2)
{
	if(bSort)
		sortMove();
}

MoveLocation::MoveLocation(const Point& firstLocation):
x(firstLocation.getX()), y(firstLocation.getY()), x2(-1), y2(-1), m_iSize(1)
{
}

MoveLocation::MoveLocation(const Point& firstLocation, const Point& secondLocation, bool bSort):
x(firstLocation.getX()), y(firstLocation.getY()), x2(secondLocation.getX()), y2(secondLocation.getY()), m_iSize(2)
{
	if(bSort)
		sortMove();
}

MoveLocation::MoveLocation(const int& iMoveValue)
{
	setupByMoveValue(iMoveValue);
}

MoveLocation::~MoveLocation(void)
{
}

bool MoveLocation::isNullMove() const
{
	assert(m_iSize == 1 || m_iSize == 2);
	if(m_iSize == 1)
		return x == -1 && y == -1;
	else
		return x == -1 && y == -1 && x2 == -1 && y2 == -1;
}

Point MoveLocation::getFirstLocation()
{
	assert(m_iSize == 1 || m_iSize == 2);
	return Point(x, y);
}

Point MoveLocation::getSecondLocation()
{
	assert(m_iSize == 2);
	return Point(x2, y2);
}

vector<Point> MoveLocation::getAllLocations()
{
	assert(m_iSize == 1 || m_iSize == 2);
	vector<Point> vLocations;
	vLocations.push_back(Point(x, y));
	if(m_iSize == 2)
		vLocations.push_back(Point(x2, y2));
	return vLocations;
}

void MoveLocation::setupByMoveValue(const int& iMoveValue)
{
    m_iSize = ((iMoveValue & 0xFFC00) == 0xFFC00)?1:2;
    assert(m_iSize == 1 || m_iSize == 2);
	/* 
                y2  |   x2  |   y   |   x      x2, y2 is for connect6
     binary   00000 | 00000 | 00000 | 00000
     
     e.g.     y2=31 | x2=31 |  y=5  |  x=3     set all position default value as 11111
     binary   11111 | 11111 | 00101 | 00100 -> moveValue: 1047716
	*/
	x = iMoveValue & 0x1F;
	x = (x == 0x1F)?-1:x;
	y = (iMoveValue & 0x3E0) >> 5;
	y = (y == 0x1F)?-1:y;
	if(m_iSize == 2) {
		x2 = (iMoveValue & 0x7C00) >> 10;
		x2 = (x2 == 0x1F)?-1:x2;
		y2 = (iMoveValue & 0xF8000) >> 15;
		y2 = (y2 == 0x1F)?-1:y2;
	}
}

int MoveLocation::getMoveValue()
{
	assert(m_iSize == 1 || m_iSize == 2);
	int iMoveValue = 0;
	if(x == -1 && y == -1)
		iMoveValue = 0x3FF;
	else
		iMoveValue = (x & 0x1F) | (y & 0x1F) << 5;
	if(m_iSize == 1)
		iMoveValue |= 0xFFC00;
	else {
		if(x2 == -1 && y2 == -1)
			iMoveValue |= 0xFFC00;
		else
			iMoveValue |= (x2 & 0x1F) << 10 | (y2 & 0x1F) << 15;
	}
	return iMoveValue;
}

MoveLocation MoveLocation::operator=(const Point& firstLocation)
{
	m_iSize = 1;
	x = firstLocation.getX();
	y = firstLocation.getY();
	return *this;
}


bool MoveLocation::operator==(const MoveLocation moveLocation) const
{
	return m_iSize == moveLocation.m_iSize &&
		   x == moveLocation.x &&
		   y == moveLocation.y &&
		   x2 == moveLocation.x2 &&
		   y2 == moveLocation.y2;
}

void MoveLocation::sortMove()
{
	assert(m_iSize == 2);
	if(x >= x2) {
		if(x > x2) 
			swapMove();
		else if(y > y2)
			swapMove();
	}
}

void MoveLocation::swapMove()
{
	int iTemp = 0;
	iTemp = x;
	x = x2;
	x2 = iTemp;
	iTemp = y;
	y = y2;
	y2 = iTemp;
}

MoveLocation MoveLocation::operator-(const MoveLocation moveLocation) const
{
	if(m_iSize == 1)
		return MoveLocation(x - moveLocation.x, y - moveLocation.y);
	else
		return MoveLocation(x - moveLocation.x, y - moveLocation.y, x2 - moveLocation.x2, y2 - moveLocation.y2, false);
}

MoveLocation MoveLocation::operator+(const MoveLocation moveLocation) const
{
	if(m_iSize == 1)
		return MoveLocation(x + moveLocation.x, y + moveLocation.y);
	else
		return MoveLocation(x + moveLocation.x, y + moveLocation.y, x2 + moveLocation.x2, y2 + moveLocation.y2);
}

bool MoveLocation::operator<(const MoveLocation moveLocation) const
{
    assert(m_iSize == moveLocation.m_iSize);
    if(x != moveLocation.x)
        return x < moveLocation.x;
    else if(y != moveLocation.y)
        return y < moveLocation.y;
    else if(x2 != moveLocation.x2)
        return x2 < moveLocation.x2;
    else
        return y2 < moveLocation.y2;
} 
