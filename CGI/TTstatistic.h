#ifndef H_RZONE_TT_STATISTIC
#define H_RZONE_TT_STATISTIC

#include "BasicType.h"

class TTstatistic
{
public:
	TTstatistic() { reset(); }
	inline void reset()
	{
		m_nCount = 0;
		m_nEntry = 0;
		m_nMaxEntry = 0;
		m_nCompare = 0;
		m_nMaxCompare = 0;
		m_tree.reset();
	}

public:
	static TTstatistic mergeStat(TTstatistic& left, TTstatistic& right) {
		TTstatistic stat;
		stat.m_nCount = left.m_nCount + right.m_nCount;
		stat.m_nEntry = left.m_nEntry + right.m_nEntry;
		stat.m_nMaxEntry = max(left.m_nMaxEntry, right.m_nMaxEntry);
		stat.m_nCompare = left.m_nCompare + right.m_nCompare;
		stat.m_nMaxCompare = max(left.m_nMaxCompare, right.m_nMaxCompare);
		stat.m_tree.m_nNullLeftEdge = left.m_tree.m_nNullLeftEdge + right.m_tree.m_nNullLeftEdge;
		stat.m_tree.m_nNullRightEdge = left.m_tree.m_nNullRightEdge + right.m_tree.m_nNullRightEdge;
		stat.m_tree.m_nIdSkipLeftEdge = left.m_tree.m_nIdSkipLeftEdge + right.m_tree.m_nIdSkipLeftEdge;
		stat.m_tree.m_nIdSkipRightEdge = left.m_tree.m_nIdSkipRightEdge + right.m_tree.m_nIdSkipRightEdge;
		stat.m_tree.m_nFoundStopLeftEdge = left.m_tree.m_nFoundStopLeftEdge + right.m_tree.m_nFoundStopLeftEdge;
		stat.m_tree.m_nFoundStopRightEdge = left.m_tree.m_nFoundStopRightEdge + right.m_tree.m_nFoundStopRightEdge;
		stat.m_tree.m_nCompareLeftEdge = left.m_tree.m_nCompareLeftEdge + right.m_tree.m_nCompareLeftEdge;
		stat.m_tree.m_nCompareRightEdge = left.m_tree.m_nCompareRightEdge + right.m_tree.m_nCompareRightEdge;

		stat.m_tree.m_nNullTotalLength = left.m_tree.m_nNullTotalLength + right.m_tree.m_nNullTotalLength;
		stat.m_tree.m_nIdSkipTotalLength = left.m_tree.m_nIdSkipTotalLength + right.m_tree.m_nIdSkipTotalLength;
		stat.m_tree.m_nFoundStopTotalLength = left.m_tree.m_nFoundStopTotalLength + right.m_tree.m_nFoundStopTotalLength;
		stat.m_tree.m_nCompareTotalLength = left.m_tree.m_nCompareTotalLength + right.m_tree.m_nCompareTotalLength;

		stat.m_tree.m_nNullPath = left.m_tree.m_nNullPath + right.m_tree.m_nNullPath;
		stat.m_tree.m_nIdSkipPath = left.m_tree.m_nIdSkipPath + right.m_tree.m_nIdSkipPath;
		stat.m_tree.m_nFoundStopPath = left.m_tree.m_nFoundStopPath + right.m_tree.m_nFoundStopPath;
		stat.m_tree.m_nComparePath = left.m_tree.m_nComparePath + right.m_tree.m_nComparePath;

		stat.m_tree.m_nMaxNullPathLength = max(left.m_tree.m_nMaxNullPathLength, right.m_tree.m_nMaxNullPathLength);
		stat.m_tree.m_nMaxIdSkipPathLength = max(left.m_tree.m_nMaxIdSkipPathLength, right.m_tree.m_nMaxIdSkipPathLength);
		stat.m_tree.m_nMaxFoundStopPathLength = max(left.m_tree.m_nMaxFoundStopPathLength, right.m_tree.m_nMaxFoundStopPathLength);
		stat.m_tree.m_nMaxComparePathLength = max(left.m_tree.m_nMaxComparePathLength, right.m_tree.m_nMaxComparePathLength);

		stat.m_tree.m_nFound = left.m_tree.m_nFound + right.m_tree.m_nFound;

		return stat;
	}

public:
	void copy(TTstatistic& stat) {
		m_nCount = stat.m_nCount;
		m_nEntry = stat.m_nEntry;
		m_nMaxEntry = stat.m_nMaxEntry;
		m_nCompare = stat.m_nCompare;
		m_nMaxCompare = stat.m_nMaxCompare;
		m_tree.m_nNullLeftEdge = stat.m_tree.m_nNullLeftEdge;
		m_tree.m_nNullRightEdge = stat.m_tree.m_nNullRightEdge;
		m_tree.m_nIdSkipLeftEdge = stat.m_tree.m_nIdSkipLeftEdge;
		m_tree.m_nIdSkipRightEdge = stat.m_tree.m_nIdSkipRightEdge;
		m_tree.m_nFoundStopLeftEdge = stat.m_tree.m_nFoundStopLeftEdge;
		m_tree.m_nFoundStopRightEdge = stat.m_tree.m_nFoundStopRightEdge;
		m_tree.m_nCompareLeftEdge = stat.m_tree.m_nCompareLeftEdge;
		m_tree.m_nCompareRightEdge = stat.m_tree.m_nCompareRightEdge;
		m_tree.m_nNullTotalLength = stat.m_tree.m_nNullTotalLength;
		m_tree.m_nIdSkipTotalLength = stat.m_tree.m_nIdSkipTotalLength;
		m_tree.m_nFoundStopTotalLength = stat.m_tree.m_nFoundStopTotalLength;
		m_tree.m_nCompareTotalLength = stat.m_tree.m_nCompareTotalLength;
		m_tree.m_nNullPath = stat.m_tree.m_nNullPath;
		m_tree.m_nIdSkipPath = stat.m_tree.m_nIdSkipPath;
		m_tree.m_nFoundStopPath = stat.m_tree.m_nFoundStopPath;
		m_tree.m_nComparePath = stat.m_tree.m_nComparePath;
		m_tree.m_nMaxNullPathLength = stat.m_tree.m_nMaxNullPathLength;
		m_tree.m_nMaxIdSkipPathLength = stat.m_tree.m_nMaxIdSkipPathLength;
		m_tree.m_nMaxFoundStopPathLength = stat.m_tree.m_nMaxFoundStopPathLength;
		m_tree.m_nMaxComparePathLength = stat.m_tree.m_nMaxComparePathLength;
		m_tree.m_nFound = stat.m_tree.m_nFound;
	}

public:
	class TTtree
	{
	public:
		TTtree() { reset(); }
		void reset() {
			m_nNullLeftEdge = 0;
			m_nNullRightEdge = 0;
			m_nIdSkipLeftEdge = 0;
			m_nIdSkipRightEdge = 0;
			m_nFoundStopLeftEdge = 0;
			m_nFoundStopRightEdge = 0;
			m_nCompareLeftEdge = 0;
			m_nCompareRightEdge = 0;
			m_nNullTotalLength = 0;
			m_nIdSkipTotalLength = 0;
			m_nFoundStopTotalLength = 0;
			m_nCompareTotalLength = 0;
			m_nNullPath = 0;
			m_nIdSkipPath = 0;
			m_nFoundStopPath = 0;
			m_nComparePath = 0;
			m_nMaxNullPathLength = 0;
			m_nMaxIdSkipPathLength = 0;
			m_nMaxFoundStopPathLength = 0;
			m_nMaxComparePathLength = 0;
			m_nFound = 0;
		}
	public:
		// # edge
		ull m_nNullLeftEdge;
		ull m_nNullRightEdge;
		ull m_nIdSkipLeftEdge;
		ull m_nIdSkipRightEdge;
		ull m_nFoundStopLeftEdge;
		ull m_nFoundStopRightEdge;
		ull m_nCompareLeftEdge;
		ull m_nCompareRightEdge;
		// # total length
		ull m_nNullTotalLength;
		ull m_nIdSkipTotalLength;
		ull m_nFoundStopTotalLength;
		ull m_nCompareTotalLength;
		// # path
		ull m_nNullPath;
		ull m_nIdSkipPath;
		ull m_nFoundStopPath;
		ull m_nComparePath;
		// # max
		ull m_nMaxNullPathLength;
		ull m_nMaxIdSkipPathLength;
		ull m_nMaxFoundStopPathLength;
		ull m_nMaxComparePathLength;
		ull m_nFound;
	};

public:
	ull m_nCount;
	ull m_nEntry;
	ull m_nMaxEntry;
	ull m_nCompare;
	ull m_nMaxCompare;
	TTtree m_tree;
};


#endif