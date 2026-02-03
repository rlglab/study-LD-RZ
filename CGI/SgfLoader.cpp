#include "SgfLoader.h"
#include "WeichiDynamicKomi.h"

bool SgfLoader::parseFromFile( const std::string& sgffile, int limit/*=MAX_GAME_LENGTH*/ )
{
	this->clear();

	ifstream fin(sgffile.c_str());
	if ( !fin ) return false;

	string sgf ;
	string line ;

	while ( getline(fin, line) ) {
		sgf += line ;
	}
	
	bool bIsSuccess = parseFromString(sgf, limit);
	m_sFileName = sgffile;

	return bIsSuccess;
}

bool SgfLoader::parseFromString( const std::string& sgfstring, int limit/*=MAX_GAME_LENGTH*/ )
{
	this->clear();

	bool slash = false; 
	bool bracket = false ;

	m_sSgfString = sgfstring;

	string key ;
	string value ;
	string buffer ;
	for ( uint p=0; p<sgfstring.length();++p ) {
		if ( slash ) {
			switch (sgfstring[p]) {
			case '\n':  case '\r':  break;
			default: buffer += sgfstring[p] ; break;
			}
			slash = false ;
		} else {
			switch(sgfstring[p]) {
			case '\\':  slash = true; break;
			case ';':
				if ( !bracket ) buffer.clear();
				else buffer += sgfstring[p];
				break;
			case '[':
				if ( !bracket) {
					if ( !buffer.empty() ) key = buffer ;
					bracket = true ;
					buffer.clear();
				}
				break;
			case ']':   
				value = buffer ; buffer.clear(); bracket = false;  
				if (handle_property(key, value) == false) { return false; }
				if (getPlayMoveLength() == limit) {
					m_vNode.resize(m_vNode.size() - 1);
					return true; /// remove last, and return
				}
				break;
			case ')':   if ( !bracket ) return true; /// ending ~
			default:
				if ( bracket ) buffer += sgfstring[p];
				else if ( isupper(sgfstring[p]) ) buffer += sgfstring[p];
			}
		}
	}

	return false;
}

std::vector<WeichiMove> SgfLoader::getPlayMove() const
{
	std::vector<WeichiMove> vMoves;
	for (int i = 1; i < m_vNode.size(); ++i) { vMoves.push_back(m_vNode[i].m_move); }
	return vMoves;
}

bool SgfLoader::handle_property( const std::string& key, const std::string& value )
{
	if (key == "AB" || key == "AW") {
		if (getPlayMoveLength() != 0) return false;
		Color c = toColor(key[1]);
		WeichiMove m(toColor(key[1]), value, getBoardSize());
		if (m.getColor() == COLOR_NONE) { return false; }
		this->addPreset(m);
	} else if (key == "MA") {
		WeichiMove m(COLOR_NONE, value, getBoardSize());
		m_vNode.back().m_vMark.push_back(m);
	} else if (key == "CR") {
		WeichiMove m(COLOR_NONE, value, getBoardSize());
		m_vNode.back().m_vCircle.push_back(m);
	} else if (key == "SQ") {
		WeichiMove m(COLOR_NONE, value, getBoardSize());
		m_vNode.back().m_vSquare.push_back(m);
	} else if (key == "TR") {
		WeichiMove m(COLOR_NONE, value, getBoardSize());
		m_vNode.back().m_vTriangle.push_back(m);
	} else if (key == "B" || key == "W") {
		Color c = toColor(key[0]);
		WeichiMove m(c, value, getBoardSize());
		if (m.getColor() == COLOR_NONE) { m = WeichiMove(c); }
		this->addPlay(m);
	} else if (key == "C") {
		m_vNode.back().m_comment = value;
	} else {
		m_sgfTag.setSgfTag(key, value);
		if (key == "RE") {
			if (value[0] == 'B') { m_winColor = COLOR_BLACK; }
			else if (value[0] == 'W') { m_winColor = COLOR_WHITE; }
			else { m_winColor = COLOR_NONE; }
		}
	}

	return true;
}

bool SgfLoader::operator() ( WeichiThreadState& state ) const
{
	state.resetThreadState();
	WeichiBoard& board = state.m_board ;

	// set AB and AW
	for (auto node: m_vNode) {
		// set AB and AW
		for (auto m : node.m_preset) {
			if (!board.preset(m)) {
				CERR() << "fail: " << m.toSgfString() << endl;
				return false;
			}
		}

		// set B and W
		WeichiMove m = node.m_move;
		if (m != PASS_MOVE && !state.play(m, true)) {
			CERR() << "fail: " << m.toSgfString() << endl;
			return false;
		}
	}

	return true;
}

void SgfLoader::clear()
{
	m_vNode.clear();
	m_vNode.resize(1, SgfNode());
	m_sFileName = "";
	m_sSgfString = "";
	m_winColor = COLOR_NONE;
}