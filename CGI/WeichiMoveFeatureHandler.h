#ifndef WEICHIMOVEFEATUREHANDLER_H
#define WEICHIMOVEFEATUREHANDLER_H

#include "BasicType.h"
#include "Dual.h"
#include "WeichiMove.h"
#include "WeichiMoveFeature.h"
#include "WeichiBoard.h"
#include "WeichiGammaTable.h"
#include "WeichiMiniSearch.h"
#include "WeichiBadMoveReader.h"
#include "WeichiLowLibReader.h"
#include "WeichiLocalSearchHandler.h"
#include "CandidateEntry.h"
#include "Territory.h"

class WeichiMoveFeatureHandler
{
	friend class WeichiGtpEngine;
private:
	class BlockLocalSequenceData {
	public:
		WeichiBitBoard m_bmAlreadySet;
		Vector<uint,MAX_NUM_GRIDS> m_vNumStone;

		BlockLocalSequenceData() { m_vNumStone.setAllAs(0,MAX_NUM_GRIDS); }
	};

	struct NumStoneAndDecay {
		WeichiBitBoard m_bmKill;
		WeichiBitBoard m_bmSave;
		WeichiBitBoard m_bmDecay;
		Vector<uint,MAX_NUM_GRIDS> m_vDecay;
		Vector<uint,MAX_NUM_GRIDS> m_vKillNum;
		Vector<uint,MAX_NUM_GRIDS> m_vSaveNum;
	};
	
	WeichiBoard& m_board;
	OpenAddrHashTable& m_ht;
	WeichiLocalSearchHandler& m_localSearchHandler;

	Vector<WeichiMoveFeature,MAX_NUM_GRIDS> m_vMoveFeatureList;
	Vector<CandidateEntry,MAX_NUM_GRIDS> m_vUCTCandidateList;
	Vector<PredictPlayInfo,MAX_NUM_GRIDS> m_vPredictPlayInfoList;
	Vector<WeichiRadiusPatternAttribute,MAX_NUM_GRIDS> m_vRadiusPatternAttrList;

public:
	WeichiMoveFeatureHandler( WeichiBoard& board, OpenAddrHashTable& ht, WeichiLocalSearchHandler& localSearchHandler )
		: m_board(board), m_ht(ht), m_localSearchHandler(localSearchHandler)
	{
		m_vMoveFeatureList.resize(MAX_NUM_GRIDS);
		m_vPredictPlayInfoList.resize(MAX_NUM_GRIDS);
		m_vRadiusPatternAttrList.resize(MAX_NUM_GRIDS);
	}

	const Vector<WeichiMoveFeature,MAX_NUM_GRIDS>& calculateFullBoardCandidates( Color turnColor );
	const Vector<CandidateEntry,MAX_NUM_GRIDS>& getUCTCandidateList( bool bConsiderPass=false );
	inline const Vector<WeichiRadiusPatternAttribute,MAX_NUM_GRIDS>& getRadiusPatternAttribute() const { return m_vRadiusPatternAttrList; }

private:
	void calculateAllMoveFeature( Color turnColor );
	void calculateAllBlockLocalSequenceFeature( Color turnColor );
	void calculateAllRadiusFeature( Color turnColor );
	bool isNeedCalculateCandidate( const WeichiMove& move );
	void calculateNormalCandidate( const WeichiMove& move );
	void calculatePassCandidate();

	// for normal feature
	WeichiMoveFeatureType findDisToBorderFeature( const WeichiMove& move ) const;
	WeichiMoveFeatureType findLongDisToBorderFeature( const WeichiMove& move ) const;
	WeichiMoveFeatureType findDisToPrevMoveFeature( const WeichiMove& move ) const;
	WeichiMoveFeatureType findDisToPreOwnMoveFeature( const WeichiMove& move ) const;
	WeichiMoveFeatureType findSelfAtariFeature( const WeichiMove& move ) const;
	WeichiMoveFeatureType findKillOwnNew2LibFeature( const WeichiMove& move ) const;
	WeichiMoveFeatureType findKoFeature( const WeichiMove& move ) const;
	WeichiMoveFeatureType findKoRank( const WeichiMove& move, WeichiMoveFeatureType baseType ) const;
	WeichiMoveFeatureType findPassMoveFeature( const WeichiMove& move ) const;

	// for full board feature
	void findFullBoardNakadeFeature( Color turnColor );
	void findNakadeWithPrevMoveFeature( const WeichiMove& prevMove, WeichiBitBoard bmCACheckIndex, WeichiMoveFeatureType type );
	void addNakadeFeature( const WeichiClosedArea* closedArea, WeichiMoveFeatureType type );
	void findFullBoardCFGDistance();
	void findBlockCFGDistance( const WeichiBlock* block, WeichiMoveFeatureType baseType );
	void findFullBoardDecayFeature();
	void findDecayNearPrevious( WeichiBitBoard& bmNearPrevBlockCheckIndex );
	void findBlockDecayFeature( const WeichiBlock* block, const uint pos, WeichiBitBoard& bmAllDecay, Vector<uint,MAX_NUM_GRIDS>& vDecay );
	void findFullBoardAtariFeature( Color turnColor );
	bool isGoodDoubleAtari( const WeichiMove& atariMove, Color beAtariColor );
	void findFullBoardCaptureFeature( Color turnColor );
	void findFullBoardCaptureSuccessFeature( Color findColor, BlockLocalSequenceData& data );
	void findFullBoardKill2LibFeature( Color turnColor );
	void findFullBoardKill2LibSuccessFeature( Color findColor, BlockLocalSequenceData& data );
	void findFullBoardKill2LibUnknownFeature( Color findColor, BlockLocalSequenceData& data );
	void findFullBoardKill2LibFailedFeature( Color findColor, BlockLocalSequenceData& data );
	void findFullBoardSave1LibFeature( Color turnColor );
	void findFullBoardSave1LibSuccessFeature( Color findColor, BlockLocalSequenceData& data );
	void findFullBoardSave1LibUnknownFeature( Color findColor, BlockLocalSequenceData& data );
	void findFullBoardSave1LibFailedFeature( Color findColor, BlockLocalSequenceData& data );
	void findFullBoardSave2LibFeature( Color turnColor );
	void findFullBoardSave2LibSuccessFeature( Color findColor, BlockLocalSequenceData& data );
	void findFullBoardSave2LibUnknownFeature( Color findColor, BlockLocalSequenceData& data );
	void findFullBoardSave2LibFailedFeature( Color findColor, BlockLocalSequenceData& data );

	// common feature to use
	void setNumStoneFeature( WeichiBitBoard& bmStone, Vector<uint,MAX_NUM_GRIDS>& vStoneNum, WeichiMoveFeatureType baseType );

	// capture feature
	/*void findFullBoardCaptureFeature( Color turnColor, NumStoneAndDecay& nestedFeature );
	void findCaptureWithPreviousMoveFeature();
	void findBlockCaptureFeature( const WeichiBlock* block, NumStoneAndDecay& nestedFeature );*/
	
	// atari feature
	/*void findFullBoardAtariFeature( Color turnColor, NumStoneAndDecay& nestedFeature );
	bool isGoodDoubleAtari( const WeichiMove& atariMove, Color beAtariColor );
	void findDoubleAtariFeature( Color atariColor );
	void findAtariWithPreviousMoveFeature();
	void findBlockAtariFeature( const WeichiBlock* block, NumStoneAndDecay& nestedFeature );
	WeichiMoveFeatureType findReplaceAtariFeature( const WeichiMove& replaceMove, const WeichiBlock* beAtariBlock );*/

	// 1 liberty feature
	//void findFullBoardOneLibFeature( Color turnColor, NumStoneAndDecay& nestedFeature );
	//void findOneLibBlockFeature( const WeichiBlock* block, NumStoneAndDecay& nestedFeature );

	// 2 liberty feature
	/*void findFullBoardTwoLibFeature( Color turnColor, NumStoneAndDecay& nestedFeature );
	void findTwoLibWithPreviousMoveFeature();
	void findTwoLibBlockFeature( const WeichiBlock* block, NumStoneAndDecay& nestedFeature );*/
	
	inline int distanceBetweenTwoMoves( WeichiMove m1, WeichiMove m2 ) const
	{
		assertToFile( !m1.isPass() && !m2.isPass(), const_cast<WeichiBoard*>(&m_board) );

		int diffX = (m1.x()>m2.x()) ? (m1.x()-m2.x()) : (m2.x()-m1.x());
		int diffY = (m1.y()>m2.y()) ? (m1.y()-m2.y()) : (m2.y()-m1.y());
		int diffMax = (diffX>diffY) ? diffX : diffY;

		assertToFile( (diffX+diffY+diffMax)>=2, const_cast<WeichiBoard*>(&m_board) );

		return (diffX+diffY+diffMax);
	}
	inline WeichiMoveFeatureType getShiftMoveFeatureType( uint rank, uint baseRank, WeichiMoveFeatureType baseFeature, WeichiMoveFeatureType topFeature ) const
	{
		WeichiMoveFeatureType shiftType = static_cast<WeichiMoveFeatureType>(baseFeature+rank-baseRank);
		return (shiftType>topFeature) ? topFeature : shiftType;
	}
	inline void addMoveFixedFeature( WeichiMoveFeatureType type, WeichiMoveFeature& moveFeature ) const
	{
		if( moveFeature.hasFixedFeature(MF_NOT_IN_CANDIDATE) || type==MF_NOT_IN_CANDIDATE ) { return; }
		if( moveFeature.hasFixedFeature(type) ) { return; }
		moveFeature.addFixedFeature(type);
	}
	inline void setBitBoardFeature( WeichiBitBoard& bmBoard, WeichiMoveFeatureType type )
	{
		uint pos;
		while( (pos=bmBoard.bitScanForward())!=-1 ) {
			addMoveFixedFeature(type,m_vMoveFeatureList[pos]);
		}
	}
	string getCNNFeatureBitBoardString( WeichiBitBoard bmBoard )
	{
		ostringstream oss;
		for( uint i=0; i<MAX_NUM_GRIDS; i++ ) {
			if( bmBoard.BitIsOn(i) ) { oss << "1"; }
			else { oss << "0"; }
		}

		return oss.str();
	}
	
	// invariance
	bool invariance() const;
	bool invariance_checkMoveFeature( WeichiMoveFeature moveFeature ) const;
	bool invariance_checkOnlyOneBitOn( WeichiMoveFeature moveFeature, uint startIndex, uint endIndex ) const;
};

#endif