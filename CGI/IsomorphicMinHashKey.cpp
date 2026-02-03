#include "IsomorphicMinHashKey.h"

MoveLocation IsomorphicMinHashkey::staticCentralPosition = MoveLocation(10, 10, 10, 10);

void IsomorphicMinHashkey::setStaticCentralPosition(const int& boardSize)
{
	staticCentralPosition = MoveLocation(boardSize / 2, boardSize / 2, boardSize / 2, boardSize / 2);
}

MoveLocation IsomorphicMinHashkey::getSymmetricCoordinate(const MoveLocation& position, const SymmetryType& rotateAndReflectType)
{
	if(position.isNullMove())
		return position;
	MoveLocation difference = position - staticCentralPosition;
	if(shouldReflect(rotateAndReflectType)) {
		difference.x *= -1;
		if(difference.m_iSize == 2)
			difference.x2 *= -1;
	}
	// clockwise rotation
	int iNumberOfRotateTime = rotateAndReflectType % 4;
	for(int i = 0; i < iNumberOfRotateTime; i++) {
		std::swap(difference.x, difference.y);
		difference.y *= -1;
		if(difference.m_iSize == 2) {
			std::swap(difference.x2, difference.y2);
			difference.y2 *= -1;
		}
	}
	return difference + staticCentralPosition;
}

MoveLocation IsomorphicMinHashkey::getRestoredSymmetricCoordinate(const MoveLocation& position, const SymmetryType& rotateAndReflectType)
{
	MoveLocation difference = position - staticCentralPosition;
	int iNumberOfRotateTime = rotateAndReflectType % 4;
	// counterclockwise rotation
	for(int i = 0; i < iNumberOfRotateTime; i++) {
		std::swap(difference.x, difference.y);
		difference.x *= -1;
		if(difference.m_iSize == 2) {
			std::swap(difference.x2, difference.y2);
			difference.x2 *= -1;
		}
	}
	if(shouldReflect(rotateAndReflectType)) {
		difference.x *= -1;
		if(difference.m_iSize == 2)
			difference.x2 *= -1;
	}
	return difference + staticCentralPosition;
}

void IsomorphicMinHashkey::getAllSymmetricCoordinate(const MoveLocation& position, vector<MoveLocation>& vSymmetricLocation)
{
	vSymmetricLocation.clear();
	for(int symmetryType = SYM_NORMAL; symmetryType <= SYM_HORIZONTAL_REFLECTION_ROTATE_270; symmetryType++) {
		MoveLocation isomorphicLocation = IsomorphicMinHashkey::getSymmetricCoordinate(position, static_cast<SymmetryType>(symmetryType));
		if(find(vSymmetricLocation.begin(), vSymmetricLocation.end(), isomorphicLocation) == vSymmetricLocation.end()) {
			vSymmetricLocation.push_back(isomorphicLocation);
		}
	}
}

bool IsomorphicMinHashkey::shouldReflect(const SymmetryType& rotateAndReflectType)
{
	return rotateAndReflectType >= 4;
} 
