#ifndef WEICHISEMEAIINFO_H
#define WEICHISEMEAIINFO_H

class WeichiSemeaiInfo {
private:
	uint m_types;
	uint m_shareLib;
	uint m_exFavoriteLib;
	uint m_exUnderdogLib;
public:
	uint getType() const { return m_types; }
	uint getShareLiberty() const { return m_shareLib; }
	uint getExFavoriteLiberty() const { return m_exFavoriteLib; }
	uint getExUnderdogLiberty() const { return m_exUnderdogLib; }

	void setType( uint type ) { m_types = type; }
	void setShareLiberty( uint shareLib ) { m_shareLib = shareLib; }
	void setExFavoriteLiberty( uint exFLib ) { m_exFavoriteLib = exFLib; }
	void setExUnderdogLiberty( uint exULib ) { m_exUnderdogLib = exULib; }
};

#endif