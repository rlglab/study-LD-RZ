#ifndef WEICHIDYNAMICKOMI_H
#define WEICHIDYNAMICKOMI_H

namespace WeichiDynamicKomi {
    // The formula is modify by aya's formula in Computer Go Archive
	extern int Handicap;
	extern int Internal_Handicap_komiInit;
	extern float Internal_komi;
	extern float Old_Internal_komi;
	extern float BlackWinStoneNum;
	extern float WhiteWinStoneNum;

	void adjustDykomi( int numStone, unsigned int numMove );
	void setInternalKomi( int komiDiff );
	void setHandicapConfigure( int handicap );
    int calculateDynamicKomiDiff( int numStone, unsigned int numMove );
}

#endif