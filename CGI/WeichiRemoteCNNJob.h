#ifndef __WEICHIREMOTECNNJOB_H__
#define __WEICHIREMOTECNNJOB_H__

#include "WeichiBaseRemoteJob.h"
#include "CandidateEntry.h"

class WeichiRemoteCNNJob : public WeichiBaseRemoteJob
{
private:
	// Job content
	WeichiCNNNetType m_cnnType;

	// SL result
	vector<vector<CandidateEntry> > m_SLResult;

	// VN result
	vector<Color> m_vColors;
	vector<vector<float> > m_vValues;

public:
	WeichiRemoteCNNJob() { clear(); }
	void clear() { WeichiBaseRemoteJob::clear(); m_cnnType = CNN_NET_UNKNOWN; }

	const WeichiCNNNetType& getCnnType() const { return m_cnnType; }
	void setCnnType( const WeichiCNNNetType& type ) { m_cnnType = type; }

	void setResult( const string& sJobResult )
	{
		// serialize function: WeichiUctCNNHandler::runRemoteJob
		if ( m_cnnType != CNN_NET ) { return; }

		m_SLResult.resize(m_vJobs.size());
		m_vColors.clear();
		m_vValues.resize(m_vJobs.size());
		istringstream iss(sJobResult);
		for ( uint i = 0; i < m_vJobs.size(); ++i ) {
			// set SL results
			// format: candidateSize (pos score)*candidateSize
			vector<CandidateEntry>& vCandidates = m_SLResult[i];
			vCandidates.clear();
			uint candidateSize;
			iss >> candidateSize;
			for ( uint j = 0; j < candidateSize; ++j ) {
				uint pos;
				float score;
				iss >> pos >> score;
				vCandidates.push_back(CandidateEntry(pos, score));
			}

			// set VN results
			// format: valueSize color (VNvalue)*valueSize
			m_vValues[i].clear();
			uint valueSize;
			std::string sColor;
			Color color;
			iss >> valueSize >> sColor;
			color = toColor(sColor[0]);
			m_vColors.push_back(color);
			for ( uint j = 0; j < valueSize; ++j ) {
				float fValue;
				iss >> fValue;
				m_vValues[i].push_back(fValue);
			}
		}

	}

	const vector<vector<CandidateEntry> >& getSLResult () const { return m_SLResult; }
	void setSLResult ( const vector<vector<CandidateEntry> >& SLResult ) { m_SLResult = SLResult; }

	const vector<Color>& getVNColors() const { return m_vColors; }
	void setVNColors( const vector<Color>& vColors ) { m_vColors = vColors; }

	const vector<vector<float> >& getVNValues() const { return m_vValues; }
	void setVNValues( const vector<vector<float> >& vValues ) { m_vValues = vValues; }

	void produceJobContent( WeichiCNNNetType type, uint batchSize );
	bool update(WeichiThreadState& state, WeichiUctAccessor& uctAccessor, ThreadSharedData<WeichiUctNode>& threadData, node_manage::NodeAllocator<WeichiUctNode>& nodeAllocator);
	void revert();
};

#endif 
