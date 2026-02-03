#ifndef _DBMOVE_H_
#define _DBMOVE_H_

#include <iostream>
#include "MoveLocation.h"
using namespace std;

// const int db_mask[4] = { 0x0000001F, 0x000003E0, 0x00007C00, 0x000F8000 };
// const int db_shift[4] = { 0, 5, 10, 15 };
// (19, 19)(19, 19) -> 642675
#define NONEMOVE 642675

class DbMove {
public:
	// unsigned int x1:5;
	// unsigned int y1:5;
	unsigned int value ;
	DbMove() {
		setValue(NONEMOVE);
	}
	DbMove(int move) {
		setValue(move);
	}
	~DbMove() {}
	int getValue() {
		return value ;
	}
	void setValue(int val) {
		value = val ;
	}
	bool operator==(DbMove move) {
		return this->getValue() == move.getValue();
	}
	bool operator!=(DbMove move) {
		return this->getValue() != move.getValue();
	}
	bool operator==(int value) {
		return this->getValue() == value;
	}
	bool operator!=(int value) {
		return this->getValue() != value;
	}
	friend ostream &operator<<(ostream &s, DbMove move);
	bool operator=(DbMove move) {
		this->setValue(move.getValue());
		return true;
	}
	bool operator=(int value) {
		this->setValue(value);
		return true;
	}
};

#endif 
