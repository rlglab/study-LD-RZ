#ifndef GTPCOLORGRADIENT_H
#define GTPCOLORGRADIENT_H

#include "BasicType.h"
#include "GtpRGBColor.h"

class GtpColorGradient {
private:
	double m_dStart;
	double m_dEnd;
	GtpRGBColor m_startRGB;
	GtpRGBColor m_endRGB;
public:
	GtpColorGradient( const GtpRGBColor startRGB, const GtpRGBColor endRGB, double dStart, double dEnd )
		: m_startRGB(startRGB), m_endRGB(endRGB)
		, m_dStart(dStart), m_dEnd(dEnd)
	{
		assert( m_dStart<m_dEnd );
	}
	GtpRGBColor colorOf( double dScore )
	{
		assert( dScore>=m_dStart && dScore<=m_dEnd );

		double r = (dScore - m_dStart) / (m_dEnd - m_dStart);
		assert( r>=0.0f && r<=1.0f );
		return r * m_startRGB + (1 - r) * m_endRGB;
	}
};

#endif