#include "TTentry.h"

TTentry::TTentry() {
	m_bSkip = false;		
	m_koPosition = -1;
	m_nodeID = -1;
	m_turnColor = COLOR_NONE;
	m_pNode = UctNodePtr::NULL_PTR;
	m_status = UCT_STATUS_UNKNOWN;
	m_bmBlackPiece.Reset();
	m_bmWhitePiece.Reset();
	m_bmEmptyPiece.Reset();
	m_bmRZone.Reset();	
}

bool TTentry::equal(const TTentry& entry) const {
	return (m_turnColor == entry.m_turnColor &&
		m_bmBlackPiece == entry.m_bmBlackPiece &&
		m_bmWhitePiece == entry.m_bmWhitePiece &&
		m_bmEmptyPiece == entry.m_bmEmptyPiece &&
		m_koPosition == entry.m_koPosition);
}

bool TTentry::isSubsetOf(const TTentry& entry) const {
	return (m_turnColor == entry.m_turnColor &&
		m_bmBlackPiece.isSubsetOf(entry.m_bmBlackPiece) &&
		m_bmWhitePiece.isSubsetOf(entry.m_bmWhitePiece) &&
		m_bmEmptyPiece.isSubsetOf(entry.m_bmEmptyPiece) &&
		m_koPosition == entry.m_koPosition);
}

string TTentry::toString() const {
	ostringstream oss;
	oss << "Skip: " << m_bSkip << endl;	
	oss << "KoPosition: " << m_koPosition << endl;
	oss << "nodeID: " << m_nodeID << endl;
	oss << "toPlay: " << toChar(m_turnColor) << endl;
	oss << "Status: " << getWeichiUctNodeStatus(m_status) << endl;
	for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
		if (m_bmBlackPiece.BitIsOn(*it)) { oss << "B"; }
		else if (m_bmWhitePiece.BitIsOn(*it)) { oss << "W"; }
		else if (m_bmEmptyPiece.BitIsOn(*it)) { oss << " "; }
		else { oss << "."; }

		if (*it%MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
	}
	return oss.str();
}

string TTentry::toSgfString() const {
	ostringstream oss;	
	oss << toChar(m_turnColor) << "\t";
	oss << getWeichiUctNodeStatus(m_status) << "\t";
	WeichiBitBoard bm;
	bm = m_bmBlackPiece;
	int pos = 0;
	while ((pos = bm.bitScanForward()) != -1) { oss << WeichiMove(COLOR_BLACK, pos).toSgfString(); }
	oss << "\t";
	bm = m_bmWhitePiece;
	while ((pos = bm.bitScanForward()) != -1) { oss << WeichiMove(COLOR_WHITE, pos).toSgfString(); }
	oss << "\t";
	bm = m_bmEmptyPiece;
	while ((pos = bm.bitScanForward()) != -1) { oss << WeichiMove(COLOR_NONE, pos).toSgfString(); }	
	oss << "\t";
	oss << m_koPosition << endl;
	
	return oss.str();
}

string TTentry::to77Hashkey() const
{
	ostringstream oss;
	oss << toChar(m_turnColor) << "\t";
	oss << getWeichiUctNodeStatus(m_status) << "\t";
	oss << toPatternType(m_bmRZone) << "\t";
	oss << to64bithashFor7x7(m_bmRZone) << "\t";
	oss << to64bithashFor7x7(m_bmBlackPiece) << "\t";
	oss << to64bithashFor7x7(m_bmWhitePiece) << "\t";

	return oss.str();
}

string TTentry::toZoneInfoForGuei(int& ret_rotation) const
{
	//ull bmZone = to64bithashFor7x7(m_bmRZone);
	//ull bmBlack = to64bithashFor7x7(m_bmBlackPiece);
	//ull bmWhite = to64bithashFor7x7(m_bmWhitePiece);

	//Zone7x7Bitboard bm77(bmZone, bmBlack, bmWhite);
	//int rotation = bm77.normalize();

	//HashKey64 hashkey;
	//for (int i = 0; i < 7; i++) {
	//	for (int j = 0; j < 7; j++) {
	//		Zone7x7Bitboard::PieceType pieceType = bm77.get(i, j);
	//		if (pieceType == Zone7x7Bitboard::PieceType::IRRELEVANT) { continue; }
	//		Color c;
	//		int position = 20 * i + j;
	//		switch (pieceType) {
	//		case Zone7x7Bitboard::PieceType::ZONE_EMPTY: c = COLOR_NONE; break;
	//		case Zone7x7Bitboard::PieceType::ZONE_BLACK: c = COLOR_BLACK; break;
	//		case Zone7x7Bitboard::PieceType::ZONE_WHITE: c = COLOR_WHITE; break;
	//		default: cerr << "Error Type" << endl; break;
	//		}
	//		hashkey ^= StaticBoard::getHashGenerator().getZHashKeyOf(c, position);
	//	}
	//}
	//if (m_turnColor == COLOR_BLACK) { hashkey ^= StaticBoard::getHashGenerator().getTurnKey(); }
	//ret_rotation = rotation;

	ostringstream oss;
	//oss << (ull)hashkey << "\t";
	//oss << bm77.zone << "\t";
	//oss << bm77.black << "\t";
	//oss << bm77.white << "\t";
	//oss << toChar(m_turnColor) << "\t";
	////oss << getWeichiUctNodeStatus(m_status) << "\t";

	//if (m_koPosition != -1) {
	//	const StaticGrid& sKoGrid = StaticBoard::getGrid(m_koPosition);
	//	ull bmShortKo = 0;
	//	bmShortKo |= (1ULL << (7 * sKoGrid.y() + sKoGrid.x()));
	//	Zone7x7Bitboard bmKo(bmShortKo, 0, 0);
	//	bmKo.rotate(rotation);
	//	ull bitmap_value = bmKo.zone;
	//	int final_ko = -2;
	//	for (int i = 0; i < 64; ++i) {
	//		ull value = (1ULL << i);
	//		if (value == bitmap_value) {
	//			final_ko = i;
	//			break;
	//		}
	//	}
	//	oss << final_ko << "\t";
	//}
	//else {
	//	oss << -1 << "\t";
	//}

	return oss.str();
}

ull TTentry::to64bithashFor7x7(WeichiBitBoard bm)
{
	ull value = 0;
	int coor = 0;
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		if (bm.BitIsOn(*it)) { value |= (1ULL << coor); }

		++coor;
	}

	return value;
}

int TTentry::toActionNormalizedPosition(int action, int rotation)
{
	int result;
	//int position = action;
	//const StaticGrid& sGrid = StaticBoard::getGrid(position);
	//ull bm64 = 0;
	//bm64 |= (1ULL << (7 * sGrid.y() + sGrid.x()));
	//Zone7x7Bitboard bm77Guei(bm64, 0, 0);
	//bm77Guei.rotate(rotation);

	//ull bitmap_value = bm77Guei.zone;
	//int final_pos = -2;
	//for (int i = 0; i < 64; ++i) {
	//	ull value = (1ULL << i);
	//	if (value == bitmap_value) {
	//		final_pos = i;
	//		break;
	//	}
	//}

	//if (position == 399) { result = 49; }
	//else { result = final_pos; }

	return result;
}

string TTentry::toActionListNormalizedPositionString(vector<int>& vActions, int rotation)
{
	ostringstream oss;
	oss << "[";
	bool bFirst = true;
	for (int i = 0; i < vActions.size(); ++i) {
		if (bFirst) { bFirst = false; }
		else { oss << "\t"; }

		int position = vActions[i];
		oss << toActionNormalizedPosition(position, rotation);
	}
	oss << "]";

	return oss.str();
}

string TTentry::toPatternType(WeichiBitBoard bm) const
{
	int type = 0; // 0 = center, 1= edge, 2=corner
	for (StaticBoard::iterator it = StaticBoard::getIterator(); it; ++it) {
		if (bm.BitIsOn(*it)) { 
			if (StaticBoard::getGrid(*it).isEdge()) { type = 1; }
			else if (StaticBoard::getGrid(*it).isCorner()) { type = 2; break; }
		}
	}

	ostringstream oss;
	switch (type) {
		case 0:	oss << "Center"; break;
		case 1:	oss << "Side"; break;
		case 2: oss << "Corner"; break;
	}

	return oss.str();
}

void TTentry::clear()
{
	m_bSkip = false;		
	m_koPosition = -1;
	m_nodeID = -1;
	m_solDiffDepth = -1;
	m_turnColor = COLOR_NONE;
	m_pNode = UctNodePtr::NULL_PTR;
	m_status = UCT_STATUS_UNKNOWN;
	m_bmBlackPiece.Reset();
	m_bmWhitePiece.Reset();
	m_bmEmptyPiece.Reset();
	m_bmRZone.Reset();	

	return;
}