#ifndef __MOVELOCATION_H__
#define __MOVELOCATION_H__

#include <assert.h>
#include <vector>
#include "Point.h"

using std::vector;

/*! General class for move location
	This class is used as move location in Go and connect6 generalized.
	@author chaochin
	@date 2014/08/22
*/

class MoveLocation
{
public:
	/*! Constructor
		Generalize for both one location moving and two location moving.
	*/
	MoveLocation(void);
	MoveLocation(const int& x, const int& y);
	MoveLocation(const int& x, const int& y, const int& x2, const int& y2, bool bSort = true);
	MoveLocation(const Point& firstLocation);
	MoveLocation(const Point& firstLocation, const Point& secondLocation, bool bSort = true);
	MoveLocation(const int& iMoveValue);
	~MoveLocation(void);
	/*! Check Null Move
		@return true if location is null move, (-1, -1).
	*/
	bool isNullMove() const;
	/*! Get first location
		@return first location.
	*/
	Point getFirstLocation();
	/*! Get second location
		@return second location.
	*/
	Point getSecondLocation();
	/*! Get all locations
		@return vector of locations.
	*/
	vector<Point> getAllLocations();
	/*! Setup locations by move value
		@param iMoveValue Move value stored in database.
	*/
	void setupByMoveValue(const int& iMoveValue);
	/*! Get by move value
		@return Move value for storing in database.
	*/
	int getMoveValue();
	/*! Override assignment 
		Only cast first location (CPoint) into MoveLocation.
		@param firstLocation first move location
		@return MoveLocation with size 1.
	*/
	MoveLocation operator=(const Point& firstLocation);
	/*! Casting CPoint override
		Only cast first location in MoveLocation into CPoint.
		@return CPoint of first location.
	*/
	bool operator==(const MoveLocation moveLocation) const;
	/*! Override subtraction
		@param moveLocation move location
		@return subtraction of two move location.
	*/
	MoveLocation operator-(const MoveLocation moveLocation) const;
	/*! Override addition
		@param moveLocation move location
		@return addition of two move location.
	*/
	MoveLocation operator+(const MoveLocation moveLocation) const;
  /*! Override lesser than
    @param moveLocation move location
    @return true if lesser than, false otherwise
  */
  bool operator<(const MoveLocation moveLocation) const;

private:
	void sortMove();
	void swapMove();

public:
	int x; //!< x of first location
	int y; //!< y of first location
	int x2; //!< x of second location
	int y2; //!< y of second location
	short m_iSize; //!< size of locations
};

#endif