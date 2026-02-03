#ifndef SGFLOADER_H
#define SGFLOADER_H

#include "BasicType.h"
#include "SgfTag.h"
#include "WeichiMove.h"
#include "WeichiThreadState.h"

class SgfNode {
public:
	WeichiMove m_move;
	string m_comment;
	vector<WeichiMove> m_preset;
	vector<WeichiMove> m_vMark;
	vector<WeichiMove> m_vCircle;
	vector<WeichiMove> m_vSquare;
	vector<WeichiMove> m_vTriangle;

	SgfNode() {
		m_move = PASS_MOVE;
		m_comment = "";
		m_preset.clear();
		m_vMark.clear();
		m_vCircle.clear();
		m_vSquare.clear();
		m_vTriangle.clear();
	}
};

class SgfLoader 
{
	SgfTag m_sgfTag;
	Color m_winColor;
	std::string m_sFileName;
	std::string m_sSgfString;
	std::vector<SgfNode> m_vNode;

public:
	SgfLoader() { clear(); }
	bool operator()(WeichiThreadState& state) const ;
	bool parseFromFile(const std::string& sgffile, int limit=MAX_GAME_LENGTH) ;
	bool parseFromString(const std::string& sgfstring, int limit=MAX_GAME_LENGTH);
	std::vector<WeichiMove> getPlayMove() const;
	std::vector<WeichiMove>& getPreset() { return m_vNode[0].m_preset; }
	const std::vector<WeichiMove>& getPreset() const { return m_vNode[0].m_preset; }
	inline int getPlayMoveLength() const { return m_vNode.size() - 1; }
	std::vector<SgfNode>& getSgfNode() { return m_vNode; }
	const std::vector<SgfNode>& getSgfNode() const { return m_vNode; }
	Color getWinner() const { return m_winColor; }
	
	// tag info
	std::string getFileName() const { return m_sFileName; }
	std::string getSgfString() const { return m_sSgfString; }
	bool hasSgfTag(const std::string& sTagName) const { return m_sgfTag.hasSgfTag(sTagName); }
	std::string getSgfTag(const std::string& sTagName) const { return m_sgfTag.getSgfTag(sTagName); }
	int getHandicap() const { return atoi(m_sgfTag.getSgfTag("HA").c_str()); }
	uint getBoardSize() const { return atoi(m_sgfTag.getSgfTag("SZ").c_str()); }
	float getKomi() const { return static_cast<float>(atof(m_sgfTag.getSgfTag("KM").c_str())); }
	std::string getPlayerRank(Color color) const { return (color == COLOR_BLACK) ? m_sgfTag.getSgfTag("BR") : m_sgfTag.getSgfTag("WR"); }
	std::string getPlayerName(Color color) const { return (color == COLOR_BLACK) ? m_sgfTag.getSgfTag("PB") : m_sgfTag.getSgfTag("PW"); }

private:
	void clear();
	void addPreset(WeichiMove m) { m_vNode.back().m_preset.push_back(m); }
	void addPlay(WeichiMove m) { m_vNode.push_back(SgfNode()); m_vNode.back().m_move = m; }
	bool handle_property(const std::string& key, const std::string& value);
};

#endif 
