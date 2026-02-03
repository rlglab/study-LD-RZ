#include "DbMove.h"

ostream &operator<<(ostream &s, DbMove move) {
	MoveLocation loc(move.getValue());
	s << "(" << loc.x << ", " << loc.y << ")(" << loc.x2 << ", " << loc.y2 << ")";
	return s;
}