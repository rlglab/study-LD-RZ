#include "StaticBoard.h"
#include "WeichiGlobalInfo.h"
#include "WeichiMoveCache.h"
#include "WeichiDynamicKomi.h"

StaticGrid StaticBoard::s_grids[MAX_NUM_GRIDS] ;
StaticEdge StaticBoard::s_edges[MAX_NUM_GRIDS] ;
uint StaticBoard::s_revDir[NUM_DIRECTIONS] =
{
    RIGHT_IDX, // LEFT_IDX
    DOWN_IDX, // TOP_IDX
    LEFT_IDX, // RIGHT_IDX
    UPPER_IDX, // BOTTOM_IDX

    RIGHT_DOWN_IDX, // LEFT_TOP_IDX
    LEFT_DOWN_IDX, // RIGHT_TOP_IDX
    LEFT_UPPER_IDX, // RIGHT_BOTTOM_IDX
    RIGHT_UPPER_IDX, // LEFT_BOTTOM_IDX

    RIGHT2_IDX, // LEFT_LEFT_IDX
    DOWN2_IDX, // TOP_TOP_IDX
    LEFT2_IDX, // RIGHT_RIGHT_IDX
    UPPER2_IDX, // BOTTOM_BOTTOM_IDX
} ;
HashGenerator StaticBoard::s_hashGenerator ;
WeichiBitBoard StaticBoard::s_bmCandidate ;

#ifndef NDEBUG
bool StaticBoard::isInitialized = false ;
#endif

Vector<uint, 4> StaticBoard::s_vPatternAdjDir[512] ;
Vector<uint, 4> StaticBoard::s_vPatternDiagDir[512] ;
Vector<uint, 8> StaticBoard::s_vPattern3x3Dir[512] ;
Vector<uint, 4> StaticBoard::s_vPatternDirV8Adj[512] ;
Vector<uint, 4> StaticBoard::s_vPatternDirV8Diag[512] ;

void StaticBoard::initialize()
{
#ifndef NDEBUG
	isInitialized = true ;
#endif

	initializePatternDirectTable();
	WeichiBitBoard::initialize();
	WeichiFixedFeatureBits::initialize();
	WeichiPlayoutFeatureBits::initialize();
	WeichiEdgeBits::initialize();
	HashGenerator::initialize(WeichiConfigure::superko_rule);
	// WeichiRadiusPatternTable::initialize();
	// WeichiGammaTable::initailize();
	initializeWideningIndex();
	initializePattern33Table();
	initializeEdgeDirectionBits();
	initializeBoardSizeArgument(WeichiConfigure::BoardSize);
	WeichiMoveCache::initialize();
}

void StaticBoard::initializeBoardSizeArgument( uint boardSize )
{
	WeichiBitBoard::reInitailize(boardSize);
	WeichiConfigure::BoardSize = boardSize;
	WeichiConfigure::TotalGrids = WeichiConfigure::BoardSize*WeichiConfigure::BoardSize;
	WeichiDynamicKomi::setHandicapConfigure(WeichiDynamicKomi::Handicap);
	WeichiDynamicKomi::setInternalKomi(0); // set two times for initializing WeichiDynamicKomi::Old_Internal_komi

	initializeCandidates();
	initializeBoardPosition();
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) {
		s_grids[*it].initialize(*it);
		s_edges[*it].initialize(*it);
	}
	initializeBorder();
	initializeAttributeBitBoard();
}

void StaticBoard::initializeCandidates()
{
	s_bmCandidate.Reset();
	for( uint x=1; x<WeichiConfigure::BoardSize-1; x++ ) {
		for( uint y=1; y<WeichiConfigure::BoardSize-1; y++ ) {
			s_bmCandidate.SetBitOn(WeichiMove::toPosition(x,y));
		}
	}
}

static const int dir_adj[4] = { LEFT_IDX, UPPER_IDX, RIGHT_IDX, DOWN_IDX} ;
static const int dir_diag[4] = {LEFT_UPPER_IDX, RIGHT_UPPER_IDX, RIGHT_DOWN_IDX, LEFT_DOWN_IDX} ;
static const int dir_3x3[8] = { LEFT_IDX, LEFT_UPPER_IDX, UPPER_IDX, RIGHT_UPPER_IDX, 
                                RIGHT_IDX, RIGHT_DOWN_IDX, DOWN_IDX, LEFT_DOWN_IDX} ;
void StaticBoard::initializePatternDirectTable()
{

    for ( uint i=0;i<512;++i ) {
        for ( uint j=0;j<4;++j ) {
            if ( i&(1<<j) ) {
                s_vPatternAdjDir[i].push_back(dir_adj[j]) ;
                s_vPatternDiagDir[i].push_back(dir_diag[j]) ;
            }
        }
        for ( uint j=0;j<8;++j ) {
            if ( i&(1<<j) ) {
                s_vPattern3x3Dir[i].push_back(dir_3x3[j]) ;
                if ( j&1 ) { // �u�O�ר�
                    s_vPatternDirV8Diag[i].push_back(dir_3x3[j]);
                } else { // �u�O�W�U���k
                    s_vPatternDirV8Adj[i].push_back(dir_3x3[j]);
                }
            }
        }

    }
}

WeichiBitBoard StaticBoard::s_maskBorder ;
WeichiBitBoard StaticBoard::s_verticalBorder[MAX_BOARD_SIZE] ;
WeichiBitBoard StaticBoard::s_horizontalBorder[MAX_BOARD_SIZE] ;
WeichiBitBoard StaticBoard::s_emptyBoard ;
void StaticBoard::initializeBorder()
{
    // empty board
    s_emptyBoard = WeichiBitBoard();
    
    // horizontal
	for( uint i=0; i<MAX_BOARD_SIZE; i++ ) { s_horizontalBorder[i].Reset(); }
	for( uint i=0; i<WeichiConfigure::BoardSize; i++ ) { s_horizontalBorder[0].SetBitOn(i); }
    for( uint j=1; j<=WeichiConfigure::BoardSize; j++ ) { s_horizontalBorder[j] = s_horizontalBorder[j-1].shiftUp(); }

    // vertical
	for( uint i=0; i<MAX_BOARD_SIZE; i++ ) { s_verticalBorder[i].Reset(); }
	for( uint i=0; i<WeichiConfigure::BoardSize; i++ ) { s_verticalBorder[0].SetBitOn(i*MAX_BOARD_SIZE); }
	for( uint j=1; j<=WeichiConfigure::BoardSize; j++ ) { s_verticalBorder[j] = s_verticalBorder[j-1].shiftRight(); }

	// mask border
	s_maskBorder = WeichiBitBoard();
	for( StaticBoard::iterator it=StaticBoard::getIterator(); it; ++it ) { s_maskBorder.SetBitOn(*it); }
}

WeichiBitBoard StaticBoard::s_cornerBitBoard;
WeichiBitBoard StaticBoard::s_edgeBitBoard;
WeichiBitBoard StaticBoard::s_centralBitBoard;
void StaticBoard::initializeAttributeBitBoard()
{
	//corner
	s_cornerBitBoard.Reset();
	s_cornerBitBoard.SetBitOn(WeichiMove::toPosition(0,0));
	s_cornerBitBoard.SetBitOn(WeichiMove::toPosition(0,WeichiConfigure::BoardSize-1));
	s_cornerBitBoard.SetBitOn(WeichiMove::toPosition(WeichiConfigure::BoardSize-1,0));
	s_cornerBitBoard.SetBitOn(WeichiMove::toPosition(WeichiConfigure::BoardSize-1,WeichiConfigure::BoardSize-1));

	//edge
	s_edgeBitBoard.Reset();
	for( uint i=1; i<WeichiConfigure::BoardSize-1; i++ ) {
		s_edgeBitBoard.SetBitOn(WeichiMove::toPosition(0,i));
		s_edgeBitBoard.SetBitOn(WeichiMove::toPosition(WeichiConfigure::BoardSize-1,i));
		s_edgeBitBoard.SetBitOn(WeichiMove::toPosition(i,0));
		s_edgeBitBoard.SetBitOn(WeichiMove::toPosition(i,WeichiConfigure::BoardSize-1));
	}

	//central
	s_centralBitBoard.Reset();
	s_centralBitBoard = s_maskBorder;
	s_centralBitBoard -= s_cornerBitBoard;
	s_centralBitBoard -= s_edgeBitBoard;
}

uint StaticBoard::s_boardPos[MAX_NUM_GRIDS] ;
uint StaticBoard::s_goguiPos[MAX_NUM_GRIDS] ;

void StaticBoard::initializeBoardPosition()
{
	// develop board position
	uint iIndex = 0;
	
	for( uint pos=0; pos<MAX_NUM_GRIDS; pos++ ) {
		if( !WeichiMove::isValidPosition(pos) ) { continue; }
		s_boardPos[iIndex++] = pos;
	}
	s_boardPos[iIndex] = -1;
	assert( iIndex==WeichiConfigure::TotalGrids );

	// gogui board position
	iIndex = 0;
	for( int y=WeichiConfigure::BoardSize-1; y>=0; y-- ) {
		for( int x=0; x<(int)WeichiConfigure::BoardSize; x++ ) {
			s_goguiPos[iIndex++] = WeichiMove::toPosition(x,y);
		}
	}
	s_goguiPos[iIndex] = -1;
	assert( iIndex==WeichiConfigure::TotalGrids );
}

double StaticBoard::s_outerWideningWinRateThreshold;
double StaticBoard::s_wideningIndex[MAX_EXPAND_CHILD_NODE];
void StaticBoard::initializeWideningIndex()
{
	/*s_wideningIndex[0] = 0;
	for( uint i=1; i<NUM_GRIDS; i++ ) {
		s_wideningIndex[i] = s_wideningIndex[i-1] + 40 * pow(double(1.4),double(i-1));
	}*/

	/*s_wideningIndex[0] = 50;
	for( uint i=1; i<NUM_GRIDS; i++ ) {
		s_wideningIndex[i] = s_wideningIndex[i-1] + 50 * pow(double(1.3),double(i));
	}*/

	s_outerWideningWinRateThreshold = 0.2f;
	for( int i=0; i<MAX_EXPAND_CHILD_NODE; i++ ) {
		s_wideningIndex[i] = pow(10.0,i*log10(1.8));
		//cerr << i << " " << s_wideningIndex[i] << endl;
	}
}

pattern33::Pattern33 StaticBoard::s_Pattern33[pattern33::TABLE_SIZE] ;
void StaticBoard::initializePattern33Table()
{
	for( uint i=0 ; i < pattern33::TABLE_SIZE ; i++ ) {
		s_Pattern33[i].setIndex(i) ;
	}	
}

Vector<uint,8> StaticBoard::s_vEdgeDirections[pattern33::TABLE_SIZE][3] ;
Vector<WeichiEdgeBits,8> StaticBoard::s_vEdgeBits[pattern33::TABLE_SIZE][3] ;
void StaticBoard::initializeEdgeDirectionBits()
{
	for( uint index=0; index<pattern33::TABLE_SIZE; index++ ) {
		for( int color=0; color<3; color++ ) {
			Vector<WeichiEdgeBits,8> vEdgeDirectionBits;
			vEdgeDirectionBits.setAllAs(WeichiEdgeBits(),8);

			calculateEdgeBits(COLOR_BLACK,Color(color),s_Pattern33[index],vEdgeDirectionBits);
			calculateEdgeBits(COLOR_WHITE,Color(color),s_Pattern33[index],vEdgeDirectionBits);

			for( uint dir=0; dir<8; dir++ ) {
				if( vEdgeDirectionBits[dir].empty() ) { continue; }

				s_vEdgeDirections[index][color].push_back(dir);
				s_vEdgeBits[index][color].push_back(vEdgeDirectionBits[dir]);
			}
		}
	}
}

void StaticBoard::calculateEdgeBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits )
{
	calculateDiagonalBits(findColor,center_color,pattern33,vEdgeDirectionBits);
	calculateJumpBits(findColor,center_color,pattern33,vEdgeDirectionBits);
	calculateKnightBits(findColor,center_color,pattern33,vEdgeDirectionBits);
	calculateBambooBits(findColor,center_color,pattern33,vEdgeDirectionBits);
	calculatehBambooBits(findColor,center_color,pattern33,vEdgeDirectionBits);
	calculateLHalfBambooBits(findColor,center_color,pattern33,vEdgeDirectionBits);
	calculateTHalfBambooBits(findColor,center_color,pattern33,vEdgeDirectionBits);
	calculateZHalfBambooBits(findColor,center_color,pattern33,vEdgeDirectionBits);
	calculateWallJumpBits(findColor,center_color,pattern33,vEdgeDirectionBits);
	calculateWallBambooBits(findColor,center_color,pattern33,vEdgeDirectionBits);
}

void StaticBoard::calculateDiagonalBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits )
{
	if( pattern33.get0Diagonal_RU(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_DIAGONAL_RU); }
	if( pattern33.get0Diagonal_RD(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_DIAGONAL_RD); }
	if( pattern33.get1Diagonal_RD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_DIAGONAL_RD); }
	if( pattern33.get1Diagonal_LD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_DIAGONAL_LD); }
	if( pattern33.get2Diagonal_LD(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_DIAGONAL_LD); }
	if( pattern33.get2Diagonal_LU(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_DIAGONAL_LU); }
	if( pattern33.get3Diagonal_RU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_DIAGONAL_RU); }
	if( pattern33.get3Diagonal_LU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_DIAGONAL_LU); }
}

void StaticBoard::calculateJumpBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits )
{
	if( pattern33.get0Jump_R(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_JUMP_R); }
	if( pattern33.get1Jump_D(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_JUMP_D); }
	if( pattern33.get2Jump_L(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_JUMP_L); }
	if( pattern33.get3Jump_U(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_JUMP_U); }
}

void StaticBoard::calculateKnightBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits )
{
	if( pattern33.get0Knight_RRU(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_KNIGHT_RRU); }
	if( pattern33.get0Knight_RRD(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_KNIGHT_RRD); }
	if( pattern33.get4Knight_RRD(findColor,center_color) ) { vEdgeDirectionBits[4].SetBitOn(EDGE_KNIGHT_RRD); }
	if( pattern33.get4Knight_RDD(findColor,center_color) ) { vEdgeDirectionBits[4].SetBitOn(EDGE_KNIGHT_RDD); }
	if( pattern33.get1Knight_RDD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_KNIGHT_RDD); }
	if( pattern33.get1Knight_LDD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_KNIGHT_LDD); }
	if( pattern33.get5Knight_LDD(findColor,center_color) ) { vEdgeDirectionBits[5].SetBitOn(EDGE_KNIGHT_LDD); }
	if( pattern33.get5Knight_LLD(findColor,center_color) ) { vEdgeDirectionBits[5].SetBitOn(EDGE_KNIGHT_LLD); }
	if( pattern33.get2Knight_LLD(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_KNIGHT_LLD); }
	if( pattern33.get2Knight_LLU(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_KNIGHT_LLU); }
	if( pattern33.get6Knight_LLU(findColor,center_color) ) { vEdgeDirectionBits[6].SetBitOn(EDGE_KNIGHT_LLU); }
	if( pattern33.get6Knight_LUU(findColor,center_color) ) { vEdgeDirectionBits[6].SetBitOn(EDGE_KNIGHT_LUU); }
	if( pattern33.get3Knight_RUU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_KNIGHT_RUU); }
	if( pattern33.get3Knight_LUU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_KNIGHT_LUU); }
	if( pattern33.get7Knight_RUU(findColor,center_color) ) { vEdgeDirectionBits[7].SetBitOn(EDGE_KNIGHT_RUU); }
	if( pattern33.get7Knight_RRU(findColor,center_color) ) { vEdgeDirectionBits[7].SetBitOn(EDGE_KNIGHT_RRU); }
}

void StaticBoard::calculateBambooBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits )
{
	if( pattern33.get0Bamboo_DR(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_BAMBOO_DR); }
	if( pattern33.get0Bamboo_UR(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_BAMBOO_UR); }
	if( pattern33.get1Bamboo_LD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_BAMBOO_LD); }
	if( pattern33.get1Bamboo_RD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_BAMBOO_RD); }
	if( pattern33.get2Bamboo_DL(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_BAMBOO_DL); }
	if( pattern33.get2Bamboo_UL(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_BAMBOO_UL); }
	if( pattern33.get3Bamboo_LU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_BAMBOO_LU); }
	if( pattern33.get3Bamboo_RU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_BAMBOO_RU); }
}

void StaticBoard::calculatehBambooBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits )
{
	if( pattern33.get0hBamboo_LDR(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_h_BAMBOO_LDR); }
	if( pattern33.get0hBamboo_LUR(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_h_BAMBOO_LUR); }
	if( pattern33.get0hBamboo_SDR(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_h_BAMBOO_SDR); }
	if( pattern33.get0hBamboo_SUR(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_h_BAMBOO_SUR); }
	if( pattern33.get1hBamboo_LLD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_h_BAMBOO_LLD); }
	if( pattern33.get1hBamboo_LRD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_h_BAMBOO_LRD); }
	if( pattern33.get1hBamboo_SLD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_h_BAMBOO_SLD); }
	if( pattern33.get1hBamboo_SRD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_h_BAMBOO_SRD); }
	if( pattern33.get2hBamboo_LDL(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_h_BAMBOO_LDL); }
	if( pattern33.get2hBamboo_LUL(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_h_BAMBOO_LUL); }
	if( pattern33.get2hBamboo_SDL(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_h_BAMBOO_SDL); }
	if( pattern33.get2hBamboo_SUL(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_h_BAMBOO_SUL); }
	if( pattern33.get3hBamboo_LLU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_h_BAMBOO_LLU); }
	if( pattern33.get3hBamboo_LRU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_h_BAMBOO_LRU); }
	if( pattern33.get3hBamboo_SLU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_h_BAMBOO_SLU); }
	if( pattern33.get3hBamboo_SRU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_h_BAMBOO_SRU); }
}

void StaticBoard::calculateLHalfBambooBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits )
{
	if( pattern33.get0LHalfBamboo_LDR(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_L_HALF_BAMBOO_LDR); }
	if( pattern33.get0LHalfBamboo_LUR(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_L_HALF_BAMBOO_LUR); }
	if( pattern33.get0LHalfBamboo_SDR(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_L_HALF_BAMBOO_SDR); }
	if( pattern33.get0LHalfBamboo_SUR(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_L_HALF_BAMBOO_SUR); }
	if( pattern33.get1LHalfBamboo_LLD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_L_HALF_BAMBOO_LLD); }
	if( pattern33.get1LHalfBamboo_LRD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_L_HALF_BAMBOO_LRD); }
	if( pattern33.get1LHalfBamboo_SLD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_L_HALF_BAMBOO_SLD); }
	if( pattern33.get1LHalfBamboo_SRD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_L_HALF_BAMBOO_SRD); }
	if( pattern33.get2LHalfBamboo_LDL(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_L_HALF_BAMBOO_LDL); }
	if( pattern33.get2LHalfBamboo_LUL(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_L_HALF_BAMBOO_LUL); }
	if( pattern33.get2LHalfBamboo_SDL(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_L_HALF_BAMBOO_SDL); }
	if( pattern33.get2LHalfBamboo_SUL(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_L_HALF_BAMBOO_SUL); }
	if( pattern33.get3LHalfBamboo_LLU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_L_HALF_BAMBOO_LLU); }
	if( pattern33.get3LHalfBamboo_LRU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_L_HALF_BAMBOO_LRU); }
	if( pattern33.get3LHalfBamboo_SLU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_L_HALF_BAMBOO_SLU); }
	if( pattern33.get3LHalfBamboo_SRU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_L_HALF_BAMBOO_SRU); }
}

void StaticBoard::calculateTHalfBambooBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits )
{
	if( pattern33.get0THalfBamboo_LR(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_T_HALF_BAMBOO_LR); }
	if( pattern33.get0THalfBamboo_SR(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_T_HALF_BAMBOO_SR); }
	if( pattern33.get1THalfBamboo_LD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_T_HALF_BAMBOO_LD); }
	if( pattern33.get1THalfBamboo_SD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_T_HALF_BAMBOO_SD); }
	if( pattern33.get2THalfBamboo_LL(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_T_HALF_BAMBOO_LL); }
	if( pattern33.get2THalfBamboo_SL(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_T_HALF_BAMBOO_SL); }
	if( pattern33.get3THalfBamboo_LU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_T_HALF_BAMBOO_LU); }
	if( pattern33.get3THalfBamboo_SU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_T_HALF_BAMBOO_SU); }
}

void StaticBoard::calculateZHalfBambooBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits )
{
	if( pattern33.get0ZHalfBamboo_DR(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_Z_HALF_BAMBOO_DR); }
	if( pattern33.get0ZHalfBamboo_UR(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_Z_HALF_BAMBOO_UR); }
	if( pattern33.get1ZHalfBamboo_LD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_Z_HALF_BAMBOO_LD); }
	if( pattern33.get1ZHalfBamboo_RD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_Z_HALF_BAMBOO_RD); }
	if( pattern33.get2ZHalfBamboo_DL(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_Z_HALF_BAMBOO_DL); }
	if( pattern33.get2ZHalfBamboo_UL(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_Z_HALF_BAMBOO_UL); }
	if( pattern33.get3ZHalfBamboo_LU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_Z_HALF_BAMBOO_LU); }
	if( pattern33.get3ZHalfBamboo_RU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_Z_HALF_BAMBOO_RU); }
}

void StaticBoard::calculateWallJumpBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits )
{
	if( pattern33.get0WallJump_R(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_WALL_JUMP_R); }
	if( pattern33.get1WallJump_D(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_WALL_JUMP_D); }
	if( pattern33.get2WallJump_L(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_WALL_JUMP_L); }
	if( pattern33.get3WallJump_U(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_WALL_JUMP_U); }
}

void StaticBoard::calculateWallBambooBits( Color findColor, Color center_color, const pattern33::Pattern33& pattern33, Vector<WeichiEdgeBits,8>& vEdgeDirectionBits )
{
	if( pattern33.get0WallBamboo_DR(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_WALL_BAMBOO_DR); }
	if( pattern33.get0WallBamboo_UR(findColor,center_color) ) { vEdgeDirectionBits[0].SetBitOn(EDGE_WALL_BAMBOO_UR); }
	if( pattern33.get1WallBamboo_LD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_WALL_BAMBOO_LD); }
	if( pattern33.get1WallBamboo_RD(findColor,center_color) ) { vEdgeDirectionBits[1].SetBitOn(EDGE_WALL_BAMBOO_RD); }
	if( pattern33.get2WallBamboo_DL(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_WALL_BAMBOO_DL); }
	if( pattern33.get2WallBamboo_UL(findColor,center_color) ) { vEdgeDirectionBits[2].SetBitOn(EDGE_WALL_BAMBOO_UL); }
	if( pattern33.get3WallBamboo_LU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_WALL_BAMBOO_LU); }
	if( pattern33.get3WallBamboo_RU(findColor,center_color) ) { vEdgeDirectionBits[3].SetBitOn(EDGE_WALL_BAMBOO_RU); }
}

bool StaticBoard::acuteAngle(uint v0pos, uint v1pos, uint v2pos)
{
	WeichiMove m0(v0pos);
	WeichiMove m1(v1pos);
	WeichiMove m2(v2pos);

	uint v0x = m0.x(), v0y = m0.y();
	uint v1x = m1.x(), v1y = m1.y();
	uint v2x = m2.x(), v2y = m2.y();
	double len_v0v1 = distance2(v0x, v0y, v1x, v1y);
	double len_v0v2 = distance2(v0x, v0y, v2x, v2y);
	double len_v1v2 = distance2(v1x, v1y, v2x, v2y);

	if( len_v0v1 + len_v0v2 > len_v1v2 ) return true;
	return false;
}
double StaticBoard::distance2(uint x1, uint y1, uint x2, uint y2)
{
    double xLen = abs( static_cast<double>(x1) - static_cast<double>(x2) );
    double yLen = abs( static_cast<double>(y1) - static_cast<double>(y2) );

    return xLen*xLen + yLen*yLen;
} 
