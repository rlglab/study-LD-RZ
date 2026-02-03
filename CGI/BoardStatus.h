#ifndef BOARDSTATUS_H
#define BOARDSTATUS_H

#include "Color.h"

struct BoardStatus {
	mutable bool m_bIsEarlyEndGame;
	bool m_bLastIsPass;
	bool m_bIsTwoPass;
	bool m_bIsPlayoutStatus;
	bool m_bIsExpansionStatus;
	bool m_bIsPreSimulationStatus;
	bool m_bIsPlayLightStatus;
	bool m_bLastHasDeadBlocks;
	bool m_bLastHasCombineBlocks;
	bool m_bLastHasModifyClosedArea;
	short m_ko;
	short m_lastKo;
	short m_koEater;
	Color m_colorToPlay;
};

#endif