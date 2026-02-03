#ifndef WEICHIRADIUSPATTERNATTRIBUTE_H
#define WEICHIRADIUSPATTERNATTRIBUTE_H

class WeichiRadiusPatternAttribute
{
public:
	WeichiRadiusPatternAttribute() { reset(); }
	
	void reset() { m_bHasKo = m_bHasSave1LibSuccess = false; }
	
	inline void setHasKo( bool hasKo ) { m_bHasKo = hasKo; }
	inline void setHasSave1LibSuccess( bool hasSave1LibSuccess ) { m_bHasSave1LibSuccess = hasSave1LibSuccess; }

	inline bool getHasKo() const { return m_bHasKo; }
	inline bool getHasSave1LibSuccess() const { return m_bHasSave1LibSuccess; }

private:
	bool m_bHasKo;
	bool m_bHasSave1LibSuccess;
};
#endif