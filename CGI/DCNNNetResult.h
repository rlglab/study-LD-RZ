#ifndef DCNNNETRESULT_H
#define DCNNNETRESULT_H

#include "BasicType.h"
#include "CandidateEntry.h"

class BaseDCNNNetResult {
public:
	virtual string toString()=0;
};

class DCNNSLNetResult: public BaseDCNNNetResult {
private:
	vector<CandidateEntry> m_vSLResult;

public:
	string toString()
	{
		return "";
	}

	inline const vector<CandidateEntry>& getSLResult() const { return m_vSLResult; }
	inline void setSLResult( vector<CandidateEntry> vResult ) { m_vSLResult = vResult; }
};

class DCNNBVVNNetResult: public BaseDCNNNetResult {
private:
	Color m_color;
	bool m_bHasBVResult;
	bool m_bHasVNResult;

	float m_fVNResult;
	Vector<float,MAX_NUM_GRIDS> m_vBVResult;

public:
	string toString()
	{
		return "";
	}

	inline void setColor( Color color ) { m_color = color; }
	inline void setHasBVResult( bool bHasBVResult ) { m_bHasBVResult = bHasBVResult; }
	inline void setHasVNResult( bool bHasVNResult ) { m_bHasVNResult = bHasVNResult; }
	inline void setVNResult( float fVNResult ) { m_fVNResult = fVNResult; }
	inline void setBVResult( Vector<float,MAX_NUM_GRIDS> vBVResult ) { m_vBVResult = vBVResult; }

	inline Color getColor() const { return m_color; }
	inline bool hasBVResult() const { return m_bHasBVResult; }
	inline bool hasVNResult() const { return m_bHasVNResult; }
	inline const float& getVNResult() const { return m_fVNResult; }
	inline const Vector<float,MAX_NUM_GRIDS>& getBVResult() const { return m_vBVResult; }	
};

class DCNNNetResult {
public:
	Color m_color;
	bool m_bHasPolicyResult;
	bool m_bHasValueResult;
	bool m_bHasBVResult;

	float m_fValueResult;
	vector<CandidateEntry> m_vPolicyResult;
	Vector<float, MAX_NUM_GRIDS> m_vBVResult ;

public:
	DCNNNetResult()
		: m_color(COLOR_NONE), m_bHasPolicyResult(false), m_bHasValueResult(false), m_bHasBVResult(false)
	{}

	inline void setColor(Color color) { m_color = color; }
	inline void setHasPolicyResult(bool bHasPolicyResult) { m_bHasPolicyResult = bHasPolicyResult; }
	inline void setHasValueResult(bool bHasValueResult) { m_bHasValueResult = bHasValueResult; }
	inline void setHasBVResult(bool bHasBVResult) { m_bHasBVResult = bHasBVResult ; } 
	inline void setPolicyResult(vector<CandidateEntry> vResult) { m_vPolicyResult = vResult; }
	inline void setValueResult(float fValueResult) { m_fValueResult = fValueResult; }
	inline void setBVResult(Vector<float, MAX_NUM_GRIDS> vResult) { m_vBVResult = vResult; }

	inline Color getColor() const { return m_color; }
	inline bool hasPolicyResult() const { return m_bHasPolicyResult; }
	inline bool hasValueResult() const { return m_bHasValueResult; }
	inline bool hasBVResult() const { return m_bHasBVResult; }
	inline const vector<CandidateEntry>& getPolicyResult() const { return m_vPolicyResult; }
	inline const float& getValueResult() const { return m_fValueResult; }
	inline const Vector<float, MAX_NUM_GRIDS>& getBVResult() const { return m_vBVResult; }

	string toString()
	{
		return "";
	}
};

#endif