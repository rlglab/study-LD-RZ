#ifndef H_TT_SUMMARY
#define H_TT_SUMMARY

#include "TTstatistic.h"
#include <sstream>

class TTSumary {
public:
	TTSumary(int nRecons, int IDdiff, int TTSize, TTstatistic stat, int nBlack, int nWhite, int nEmpty, int nNone) {
		m_nRecons = nRecons;
		m_IDdiff = IDdiff;
		m_nTTSize = TTSize;
		m_stat = stat;
		m_nBlack = nBlack;
		m_nWhite = nWhite;
		m_nEmpty = nEmpty;
		m_nNone = nNone;
	}

	string toString() const {
		ostringstream oss;
		oss << m_nRecons << "\t";
		oss << m_IDdiff << "\t";
		oss << m_nTTSize << "\t";
		oss << m_stat.m_nCount << "\t";
		oss << m_stat.m_nEntry << "\t";
		oss << m_stat.m_nMaxEntry << "\t";
		oss << m_stat.m_nCompare << "\t";
		oss << m_stat.m_nMaxCompare << "\t";
		oss << m_stat.m_tree.m_nNullLeftEdge << "\t";
		oss << m_stat.m_tree.m_nNullRightEdge << "\t";
		oss << m_stat.m_tree.m_nIdSkipLeftEdge << "\t";
		oss << m_stat.m_tree.m_nIdSkipRightEdge << "\t";
		oss << m_stat.m_tree.m_nFoundStopLeftEdge << "\t";
		oss << m_stat.m_tree.m_nFoundStopRightEdge << "\t";
		oss << m_stat.m_tree.m_nCompareLeftEdge << "\t";
		oss << m_stat.m_tree.m_nCompareRightEdge << "\t";
		oss << m_stat.m_tree.m_nNullTotalLength << "\t";
		oss << m_stat.m_tree.m_nIdSkipTotalLength << "\t";
		oss << m_stat.m_tree.m_nFoundStopTotalLength << "\t";
		oss << m_stat.m_tree.m_nCompareTotalLength << "\t";
		oss << m_stat.m_tree.m_nNullPath << "\t";
		oss << m_stat.m_tree.m_nIdSkipPath << "\t";
		oss << m_stat.m_tree.m_nFoundStopPath << "\t";
		oss << m_stat.m_tree.m_nComparePath << "\t";
		oss << m_stat.m_tree.m_nMaxNullPathLength << "\t";
		oss << m_stat.m_tree.m_nMaxIdSkipPathLength << "\t";
		oss << m_stat.m_tree.m_nMaxFoundStopPathLength << "\t";
		oss << m_stat.m_tree.m_nMaxComparePathLength << "\t";
		oss << m_stat.m_tree.m_nFound << "\t";
		oss << m_nBlack << "\t";
		oss << m_nWhite << "\t";
		oss << m_nEmpty << "\t";
		oss << m_nNone << "\t";
		oss << std::endl;
		return oss.str();
	}

private:
	int m_nRecons;
	int m_IDdiff;
	int m_nTTSize;
	int m_nBlack;
	int m_nWhite;
	int m_nEmpty;
	int m_nNone;
	TTstatistic m_stat;
};

#endif