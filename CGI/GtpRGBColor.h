#ifndef GTPRGBCOLOR_H
#define GTPRGBCOLOR_H

#include "BasicType.h"

class GtpRGBColor {
public:
	unsigned char m_R;
	unsigned char m_G;
	unsigned char m_B;
public:
	GtpRGBColor( unsigned char r, unsigned char g, unsigned char b ): m_R(r), m_G(g), m_B(b)
	{
	}
	inline GtpRGBColor operator+ ( const GtpRGBColor& rhs ) const
	{
		return GtpRGBColor(	m_R + rhs.m_R,
							m_G + rhs.m_G,
							m_B + rhs.m_B );
	}
	inline bool operator== ( const GtpRGBColor& rhs ) const
	{
		return ( m_R==rhs.m_R && m_G==rhs.m_G && m_B==rhs.m_B );
	}
	inline const string toString() const
	{
		ostringstream oss;
		oss << '#' << hex << setfill('0')
			<< setw(2) << int(m_R)
			<< setw(2) << int(m_G)
			<< setw(2) << int(m_B) << dec;
		return oss.str();
	}
};

inline GtpRGBColor operator* ( double f, GtpRGBColor& RGBColor )
{
	return GtpRGBColor(	static_cast<unsigned char>(f * RGBColor.m_R),
						static_cast<unsigned char>(f * RGBColor.m_G),
						static_cast<unsigned char>(f * RGBColor.m_B));
}

/// pre-defined some color
const GtpRGBColor RGB_BLACK = GtpRGBColor(0,0,0);
const GtpRGBColor RGB_GRAY = GtpRGBColor(143,143,143);
const GtpRGBColor RGB_WHITE = GtpRGBColor(255,255,255);
const GtpRGBColor RGB_RED = GtpRGBColor(255,0,0);
const GtpRGBColor RGB_GREEN = GtpRGBColor(0,255,0);
const GtpRGBColor RGB_BLUE = GtpRGBColor(0,0,255);

#endif