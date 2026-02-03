#ifndef DCNNNETCOLLECTION_H
#define DCNNNETCOLLECTION_H

#include "BasicType.h"
#include "WeichiCNNNet.h"
#include "WeichiCNNSLNet.h"
#include "WeichiCNNBVVNNet.h"

class DCNNNetCollection {
private:
	bool m_bHasCNNNet;
	vector<WeichiCNNNet*> m_vNet;
	vector<WeichiCNNSLNet*> m_vSLNet;
	vector<WeichiCNNBVVNNet*> m_vBVVNNet;

public:
	DCNNNetCollection()
		: m_bHasCNNNet(false)
	{
		m_vNet.push_back(nullptr);
		m_vSLNet.push_back(nullptr);
		m_vBVVNNet.push_back(nullptr);
	}

	~DCNNNetCollection()
	{
		while (!m_vNet.empty()) {
			if (m_vNet.back() != nullptr) {
				delete m_vNet.back();
			}
			m_vNet.pop_back();
		}
		while (!m_vSLNet.empty()) {
			if (m_vSLNet.back() != nullptr) {
				delete m_vSLNet.back();
			}
			m_vSLNet.pop_back();
		}
		while (!m_vBVVNNet.empty()) {
			if (m_vBVVNNet.back() != nullptr) {
				delete m_vBVVNNet.back();
			}
			m_vBVVNNet.pop_back();
		}
	}

	void initialize( int thread_id, vector<string> vParamString, int gpu_id = -1, bool bDisplayErrorMessage=false )
	{
		// bind thread to GPU
		for( uint i=0; i<vParamString.size(); i++ ) {
			CNNNetParam param(vParamString[i]);
			string sThreadBindGpu = param.m_sThreadBindGpu;

			if( sThreadBindGpu.length()<thread_id ) { continue; }
			if( sThreadBindGpu[thread_id-1]==' ' ) { continue; }

			int gpuId = (gpu_id >= 0) ? gpu_id : (sThreadBindGpu[thread_id-1]-'0');
			addNet(param, gpuId, bDisplayErrorMessage);
		}
	}

	inline bool hasNet() const { return m_bHasCNNNet; }
	inline bool hasNet( WeichiCNNNetType netType ) const
	{
		switch( netType ) {
			case CNN_NET: return (m_vNet[0] != nullptr);
			case CNN_NET_SL: return (m_vSLNet[0]!=nullptr);
			case CNN_NET_BV_VN: return (m_vBVVNNet[0]!=nullptr);
			default: return false;
		}
	}
	inline bool hasCNNNet() const { return (m_vNet[0] != NULL); }
	inline bool hasSLNet() const { return (m_vSLNet[0]!=NULL); }
	inline bool hasBVVNNet() const { return (m_vBVVNNet[0]!=NULL); }
	inline BaseCNNNet* getNet( WeichiCNNNetType netType, int id=0 )
	{
		switch( netType ) {
			case CNN_NET: return m_vNet[id];
			case CNN_NET_SL: return m_vSLNet[id];
			case CNN_NET_BV_VN: return m_vBVVNNet[id];
			default: return nullptr;
		}
	}
	inline WeichiCNNNet* getCNNNet(int id = 0) { return m_vNet[id]; }
	inline WeichiCNNSLNet* getSLNet( int id=0 ) { return m_vSLNet[id]; }
	inline WeichiCNNBVVNNet* getBVVNNet( int id=0 ) { return m_vBVVNNet[id]; }
	inline uint getNumCNNNet() { return static_cast<uint>(m_vNet.size()); }
	inline uint getNumSLNet() { return static_cast<uint>(m_vSLNet.size()); }
	inline uint getNumBVVNNet() { return static_cast<uint>(m_vBVVNNet.size()); }

	inline vector<WeichiCNNNetType> getNetCollectionType() {
		vector<WeichiCNNNetType> vNetCollectionType;
		for( uint i=0; i<CNN_NET_SIZE; i++ ) {
			WeichiCNNNetType netType = static_cast<WeichiCNNNetType>(i);
			if( !hasNet(netType) ) { continue; }

			vNetCollectionType.push_back(netType);
		}
		return vNetCollectionType;
	}

private:
	void addNet( const CNNNetParam& param, int gpu_id, bool bDisplayErrorMessage=false )
	{
		WeichiCNNNetType type = param.m_netType;

		switch( type ) {
			case CNN_NET:
				if( m_vNet[0]==nullptr ) { m_vNet[0] = new WeichiCNNNet(param); }
				else { m_vNet.push_back(new WeichiCNNNet(param)); }
				m_vNet.back()->setGpuID(gpu_id);
				m_vNet.back()->loadNetWork(bDisplayErrorMessage);
				break;
			case CNN_NET_SL:
				if( m_vSLNet[0]==nullptr ) { m_vSLNet[0] = new WeichiCNNSLNet(param); }
				else { m_vSLNet.push_back(new WeichiCNNSLNet(param)); }
				m_vSLNet.back()->setGpuID(gpu_id);
				m_vSLNet.back()->loadNetWork(bDisplayErrorMessage);
				break;
			case CNN_NET_BV_VN:
				if( m_vBVVNNet[0]==nullptr ) { m_vBVVNNet[0] = new WeichiCNNBVVNNet(param); }
				else { m_vBVVNNet.push_back(new WeichiCNNBVVNNet(param)); }
				m_vBVVNNet.back()->setGpuID(gpu_id);
				m_vBVVNNet.back()->loadNetWork(bDisplayErrorMessage);
				break;
			default:
				return;
		}

		m_bHasCNNNet = true;
	}
};

#endif