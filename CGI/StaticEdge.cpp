#include "StaticEdge.h"
#include "WeichiMove.h"

void StaticEdge::initialize( uint position )
{
	initializeDiagonal(position);
	//initializeJump(position);
	//initializeKnight(position);
	initializeBamboo(position);
	initializeLHalfBamboo(position);
	initializeTHalfBamboo(position);
	initializeZHalfBamboo(position);
	initializehBamboo(position);
	initializeWallJump(position);
	initializeWallBamboo(position);
}

void StaticEdge::initializeDiagonal( uint position )
{
	/************************************************************************/
	/* Diagonal: (4 type)                                                   */
	/*  + + + +                                                             */
	/*  + ¡´ A +                                                             */
	/*  + B ¡´ +                                                             */
	/*  + + + +                                                             */
	/*                                                                      */
	/*  real Stone:			all black stone                                 */
	/*  end Stone :			two black stone                                 */
	/*  eroded Stone:		none(A,B?)                                      */
	/*  virtual CA Stone:	none                                            */
	/*  CA Stone:			A,B                                             */
	/*  threat groups:      A,B                                             */
	/************************************************************************/

	EdgeProperty edgeproperty;
	Point realStone[StoneNum] = { Point(0,0), Point(1,1), Point() };
	Point endStone[StoneNum] = { Point(0,0), Point(1,1), Point() };
	Point erodedStone[StoneNum] = { Point() };
	Point virtualCAStone[StoneNum] = { Point() };
	Point CAStone[StoneNum] = { Point(0,1), Point(1,0), Point() };
	Point ThreatStone[StoneNum] = { Point(0,1), Point(1,0), Point() };
	WeichiEdgeType id[4] = {EDGE_DIAGONAL_RU, EDGE_DIAGONAL_RD, EDGE_DIAGONAL_LD, EDGE_DIAGONAL_LU};

	edgeproperty.copy(edgeproperty.m_realStone,realStone);
	edgeproperty.copy(edgeproperty.m_endStone,endStone);
	edgeproperty.copy(edgeproperty.m_erodedStone,erodedStone);
	edgeproperty.copy(edgeproperty.m_virtualCAStone,virtualCAStone);
	edgeproperty.copy(edgeproperty.m_CAStone,CAStone);
	edgeproperty.copy(edgeproperty.m_ThreatStone,ThreatStone);

	consturctEdge(position,4,id,DIR4_X,edgeproperty);
}

void StaticEdge::initializeJump( uint position )
{
	/************************************************************************/
	/* Jump: (4 type)                                                       */
	/*  + + + + +                                                           */
	/*  + + ¡´ + +                                                           */
	/*  + B A C +                                                           */
	/*  + + ¡´ + +                                                           */
	/*  + + + + +                                                           */
	/*                                                                      */
	/*  real Stone:			all black stone                                 */
	/*  end Stone :			two black stone                                 */
	/*  eroded Stone:		A,B,C                                           */
	/*  virtual CA Stone:	A                                               */
	/*  CA Stone:			B,C                                             */
	/*  threat groups:      A                                               */
	/************************************************************************/

	EdgeProperty edgeproperty;
	Point realStone[StoneNum] = { Point(0,0), Point(0,2), Point() };
	Point endStone[StoneNum] = { Point(0,0), Point(0,2), Point() };
	Point erodedStone[StoneNum] = { Point(0,1), Point(1,1), Point(-1,1), Point() };
	Point virtualCAStone[StoneNum] = { Point(0,1), Point() };
	Point CAStone[StoneNum] = { Point(1,1), Point(-1,1), Point() };
	Point ThreatStone[StoneNum] = { Point(0,1), Point() };
	WeichiEdgeType id[4] = {EDGE_JUMP_U, EDGE_JUMP_R, EDGE_JUMP_D, EDGE_JUMP_L};

	edgeproperty.copy(edgeproperty.m_realStone,realStone);
	edgeproperty.copy(edgeproperty.m_endStone,endStone);
	edgeproperty.copy(edgeproperty.m_erodedStone,erodedStone);
	edgeproperty.copy(edgeproperty.m_virtualCAStone,virtualCAStone);
	edgeproperty.copy(edgeproperty.m_CAStone,CAStone);
	edgeproperty.copy(edgeproperty.m_ThreatStone,ThreatStone);

	consturctEdge(position,4,id,DIR4_CROSS,edgeproperty);
}

void StaticEdge::initializeKnight( uint position )
{
	/************************************************************************/
	/* Knight: (8 type)                                                     */
	/*  + + + +                                                             */
	/*  + C ¡´ +                                                             */
	/*  + B A +                                                             */
	/*  + ¡´ D +                                                             */
	/*  + + + +                                                             */
	/*                                                                      */
	/*  real Stone:			all black stone                                 */
	/*  end Stone :			two black stone                                 */
	/*  eroded Stone:		A,B(C,D?)                                       */
	/*  virtual CA Stone:	A,B(main and second?)                           */
	/*  CA Stone:			C,D                                             */
	/*  threat groups:      A,B,C,D                                         */
	/************************************************************************/

	EdgeProperty edgeproperty;
	Point realStone[StoneNum] = { Point(0,0), Point(1,2), Point() };
	Point endStone[StoneNum] = { Point(0,0), Point(1,2), Point() };
	Point erodedStone[StoneNum] = { Point(0,1), Point(1,1), Point() };
	Point virtualCAStone[StoneNum] = { Point(0,1), Point(1,1), Point() };
	Point CAStone[StoneNum] = { Point(0,2), Point(1,0), Point() };
	Point ThreatStone[StoneNum] = { Point(0,0), Point(1,2), Point(0,2), Point(1,0), Point() };
	WeichiEdgeType id[8] = {EDGE_KNIGHT_RUU, EDGE_KNIGHT_RRU, EDGE_KNIGHT_RRD, EDGE_KNIGHT_RDD, 
							EDGE_KNIGHT_LDD, EDGE_KNIGHT_LLD, EDGE_KNIGHT_LLU, EDGE_KNIGHT_LUU};

	edgeproperty.copy(edgeproperty.m_realStone,realStone);
	edgeproperty.copy(edgeproperty.m_endStone,endStone);
	edgeproperty.copy(edgeproperty.m_erodedStone,erodedStone);
	edgeproperty.copy(edgeproperty.m_virtualCAStone,virtualCAStone);
	edgeproperty.copy(edgeproperty.m_CAStone,CAStone);
	edgeproperty.copy(edgeproperty.m_ThreatStone,ThreatStone);

	consturctEdge(position,8,id,DIR8,edgeproperty);
}

void StaticEdge::initializeBamboo( uint position )
{
	/************************************************************************/
	/* Bamboo: (8 type)                                                     */
	/*  + + + + + +                                                         */
	/*  + + ¡´ ¡´ + +                                                         */
	/*  + C A B D +                                                         */
	/*  + + ¡´ ¡´ + +                                                         */
	/*  + + + + + +                                                         */
	/*                                                                      */
	/*  (In point A view)                                                   */
	/*  real Stone:			all black stone                                 */
	/*  end Stone :			two black stone below and above point A         */
	/*  eroded Stone:		A,B                                             */
	/*  virtual CA Stone:	A                                               */
	/*  CA Stone:			B,C                                             */
	/*  threat groups:      A,B                                             */
	/************************************************************************/

	EdgeProperty edgeproperty;
	Point realStone[StoneNum] = { Point(0,0), Point(0,2), Point(1,0), Point(1,2), Point() };
	Point endStone[StoneNum] = { Point(0,0), Point(0,2), Point() };
	Point erodedStone[StoneNum] = { Point(0,1), Point(1,1), Point() };
	Point virtualCAStone[StoneNum] = { Point(0,1), Point() };
	Point CAStone[StoneNum] = { Point(-1,1), Point(1,1), Point() };
	Point ThreatStone[StoneNum] = { Point(0,1), Point(1,1), Point() };
	WeichiEdgeType id[8] = {EDGE_BAMBOO_RU, EDGE_BAMBOO_UR, EDGE_BAMBOO_DR, EDGE_BAMBOO_RD,
							EDGE_BAMBOO_LD, EDGE_BAMBOO_DL, EDGE_BAMBOO_UL, EDGE_BAMBOO_LU};

	edgeproperty.copy(edgeproperty.m_realStone,realStone);
	edgeproperty.copy(edgeproperty.m_endStone,endStone);
	edgeproperty.copy(edgeproperty.m_erodedStone,erodedStone);
	edgeproperty.copy(edgeproperty.m_virtualCAStone,virtualCAStone);
	edgeproperty.copy(edgeproperty.m_CAStone,CAStone);
	edgeproperty.copy(edgeproperty.m_ThreatStone,ThreatStone);

	consturctEdge(position,8,id,DIR8,edgeproperty);
}

void StaticEdge::initializeLHalfBamboo( uint position )
{
	/************************************************************************/
	/* L-Type Half Bamboo: (16 type)                                        */
	/*  + + + + +                                                           */
	/*  + + ¡´ D +                                                           */
	/*  + C A B +                                                           */
	/*  + + ¡´ ¡´ +                                                           */
	/*  + + + + +                                                           */
	/*                                                                      */
	/*  real Stone:			all black stone                                 */
	/*  end Stone :			two black stone below and above point A         */
	/*  eroded Stone:		A,B,C,D                                         */
	/*  virtual CA Stone:	A                                               */
	/*  CA Stone:			B,C                                             */
	/*  threat groups:      A,B                                             */
	/*                      A,C                                             */
	/*                      A,D                                             */
	/************************************************************************/

	//for Long attribute in L-Type Half Bamboo
	{
		EdgeProperty edgeproperty;
		Point realStone[StoneNum] = { Point(0,0), Point(0,2), Point(1,0), Point() };
		Point endStone[StoneNum] = { Point(0,0), Point(0,2), Point() };
		Point erodedStone[StoneNum] = { Point(0,1), Point(1,1), Point(-1,1), Point(1,2), Point() };
		Point virtualCAStone[StoneNum] = { Point(0,1), Point() };
		Point CAStone[StoneNum] = { Point(1,1), Point(-1,1), Point() };
		Point ThreatStone[StoneNum] = { Point(0,1), Point(1,1), Point(-1,1), Point() };
		WeichiEdgeType id[8] = {EDGE_L_HALF_BAMBOO_LRU, EDGE_L_HALF_BAMBOO_LUR, EDGE_L_HALF_BAMBOO_LDR, EDGE_L_HALF_BAMBOO_LRD,
								EDGE_L_HALF_BAMBOO_LLD, EDGE_L_HALF_BAMBOO_LDL, EDGE_L_HALF_BAMBOO_LUL, EDGE_L_HALF_BAMBOO_LLU};

		edgeproperty.copy(edgeproperty.m_realStone,realStone);
		edgeproperty.copy(edgeproperty.m_endStone,endStone);
		edgeproperty.copy(edgeproperty.m_erodedStone,erodedStone);
		edgeproperty.copy(edgeproperty.m_virtualCAStone,virtualCAStone);
		edgeproperty.copy(edgeproperty.m_CAStone,CAStone);
		edgeproperty.copy(edgeproperty.m_ThreatStone,ThreatStone);

		consturctEdge(position,8,id,DIR8,edgeproperty);
	}

	//for Short attribute in L-Type Half Bamboo
	{
		EdgeProperty edgeproperty;
		Point realStone[StoneNum] = { Point(0,0), Point(0,2), Point(1,2), Point() };
		Point endStone[StoneNum] = { Point(0,0), Point(0,2), Point() };
		Point erodedStone[StoneNum] = { Point(0,1), Point(1,1), Point(-1,1), Point(1,0), Point() };
		Point virtualCAStone[StoneNum] = { Point(0,1), Point() };
		Point CAStone[StoneNum] = { Point(1,1), Point(-1,1), Point() };
		Point ThreatStone[StoneNum] = { Point(0,1), Point(1,1), Point(-1,1), Point() };
		WeichiEdgeType id[8] = {EDGE_L_HALF_BAMBOO_SRU, EDGE_L_HALF_BAMBOO_SUR, EDGE_L_HALF_BAMBOO_SDR, EDGE_L_HALF_BAMBOO_SRD,
								EDGE_L_HALF_BAMBOO_SLD, EDGE_L_HALF_BAMBOO_SDL, EDGE_L_HALF_BAMBOO_SUL, EDGE_L_HALF_BAMBOO_SLU};

		edgeproperty.copy(edgeproperty.m_realStone,realStone);
		edgeproperty.copy(edgeproperty.m_endStone,endStone);
		edgeproperty.copy(edgeproperty.m_erodedStone,erodedStone);
		edgeproperty.copy(edgeproperty.m_virtualCAStone,virtualCAStone);
		edgeproperty.copy(edgeproperty.m_CAStone,CAStone);
		edgeproperty.copy(edgeproperty.m_ThreatStone,ThreatStone);

		consturctEdge(position,8,id,DIR8,edgeproperty);
	}
}

void StaticEdge::initializeTHalfBamboo( uint position )
{
	/************************************************************************/
	/* T-Type Half Bamboo: (8 type)                                         */
	/*  + + + + +                                                           */
	/*  + D ¡´ E +                                                           */
	/*  + B A C +                                                           */
	/*  + ¡´ ¡´ ¡´ +                                                           */
	/*  + + + + +                                                           */
	/*                                                                      */
	/*  real Stone:			all black stone                                 */
	/*  end Stone :			two black stone below and above point A         */
	/*  eroded Stone:		A(B,C,D,E?)                                     */
	/*  virtual CA Stone:	A                                               */
	/*  CA Stone:			B,C                                             */
	/*  threat groups:      A,B                                             */
	/*                      A,C                                             */
	/************************************************************************/

	//for Long attribute in T-Type Half Bamboo
	{
		EdgeProperty edgeproperty;
		Point realStone[StoneNum] = { Point(0,0), Point(0,2), Point(1,0), Point(-1,0), Point() };
		Point endStone[StoneNum] = { Point(0,0), Point(0,2), Point() };
		Point erodedStone[StoneNum] = { Point(0,1), Point() };
		Point virtualCAStone[StoneNum] = { Point(0,1), Point() };
		Point CAStone[StoneNum] = { Point(1,1), Point(-1,1), Point() };
		Point ThreatStone[StoneNum] = { Point(0,1), Point(1,1), Point(-1,1), Point() };
		WeichiEdgeType id[4] = {EDGE_T_HALF_BAMBOO_LU, EDGE_T_HALF_BAMBOO_LR, EDGE_T_HALF_BAMBOO_LD, EDGE_T_HALF_BAMBOO_LL};

		edgeproperty.copy(edgeproperty.m_realStone,realStone);
		edgeproperty.copy(edgeproperty.m_endStone,endStone);
		edgeproperty.copy(edgeproperty.m_erodedStone,erodedStone);
		edgeproperty.copy(edgeproperty.m_virtualCAStone,virtualCAStone);
		edgeproperty.copy(edgeproperty.m_CAStone,CAStone);
		edgeproperty.copy(edgeproperty.m_ThreatStone,ThreatStone);

		consturctEdge(position,4,id,DIR4_CROSS,edgeproperty);
	}

	//for Short attribute in T-Type Half Bamboo
	{
		EdgeProperty edgeproperty;
		Point realStone[StoneNum] = { Point(0,0), Point(0,2), Point(1,2), Point(-1,2), Point() };
		Point endStone[StoneNum] = { Point(0,0), Point(0,2), Point() };
		Point erodedStone[StoneNum] = { Point(0,1), Point() };
		Point virtualCAStone[StoneNum] = { Point(0,1), Point() };
		Point CAStone[StoneNum] = { Point(1,1), Point(-1,1), Point() };
		Point ThreatStone[StoneNum] = { Point(0,1), Point(1,1), Point(-1,1), Point() };
		WeichiEdgeType id[4] = {EDGE_T_HALF_BAMBOO_SU, EDGE_T_HALF_BAMBOO_SR, EDGE_T_HALF_BAMBOO_SD, EDGE_T_HALF_BAMBOO_SL};

		edgeproperty.copy(edgeproperty.m_realStone,realStone);
		edgeproperty.copy(edgeproperty.m_endStone,endStone);
		edgeproperty.copy(edgeproperty.m_erodedStone,erodedStone);
		edgeproperty.copy(edgeproperty.m_virtualCAStone,virtualCAStone);
		edgeproperty.copy(edgeproperty.m_CAStone,CAStone);
		edgeproperty.copy(edgeproperty.m_ThreatStone,ThreatStone);

		consturctEdge(position,4,id,DIR4_CROSS,edgeproperty);
	}
}

void StaticEdge::initializeZHalfBamboo( uint position )
{
	/************************************************************************/
	/* Z-Type Half Bamboo: (8 type)                                         */
	/*  + + + + +                                                           */
	/*  + ¡´ ¡´ D +                                                           */
	/*  + B A C +                                                           */
	/*  + E ¡´ ¡´ +                                                           */
	/*  + + + + +                                                           */
	/*                                                                      */
	/*  real Stone:			all black stone                                 */
	/*  end Stone :			two black stone below and above point A         */
	/*  eroded Stone:		A(B,C,D,E?)                                     */
	/*  virtual CA Stone:	A                                               */
	/*  CA Stone:			B,C                                             */
	/*  threat groups:      A,B                                             */
	/*                      A,C                                             */
	/************************************************************************/

	EdgeProperty edgeproperty;
	Point realStone[StoneNum] = { Point(0,0), Point(1,0), Point(0,2), Point(-1,2), Point() };
	Point endStone[StoneNum] = { Point(0,0), Point(0,2), Point() };
	Point erodedStone[StoneNum] = { Point(0,1), Point() };
	Point virtualCAStone[StoneNum] = { Point(0,1), Point() };
	Point CAStone[StoneNum] = { Point(-1,1), Point(1,1), Point() };
	Point ThreatStone[StoneNum] = { Point(0,1), Point(1,1), Point(-1,1), Point() };
	WeichiEdgeType id[8] = {EDGE_Z_HALF_BAMBOO_RU, EDGE_Z_HALF_BAMBOO_UR, EDGE_Z_HALF_BAMBOO_DR, EDGE_Z_HALF_BAMBOO_RD,
							EDGE_Z_HALF_BAMBOO_LD, EDGE_Z_HALF_BAMBOO_DL, EDGE_Z_HALF_BAMBOO_UL, EDGE_Z_HALF_BAMBOO_LU};

	edgeproperty.copy(edgeproperty.m_realStone,realStone);
	edgeproperty.copy(edgeproperty.m_endStone,endStone);
	edgeproperty.copy(edgeproperty.m_erodedStone,erodedStone);
	edgeproperty.copy(edgeproperty.m_virtualCAStone,virtualCAStone);
	edgeproperty.copy(edgeproperty.m_CAStone,CAStone);
	edgeproperty.copy(edgeproperty.m_ThreatStone,ThreatStone);

	consturctEdge(position,8,id,DIR8,edgeproperty);
}

void StaticEdge::initializehBamboo( uint position )
{
	/************************************************************************/
	/* h-Type Bamboo: (16 type)                                             */
	/*  + + + + + +                                                         */
	/*  + + ¡´ ¡´ D +                                                         */
	/*  + E B A C +                                                         */
	/*  + + ¡´ ¡´ ¡´ +                                                         */
	/*  + + + + + +                                                         */
	/*                                                                      */
	/*  real Stone:			all black stone                                 */
	/*  end Stone :			two black stone below and above point A         */
	/*  eroded Stone:		A(B,C,D,E?)                                     */
	/*  virtual CA Stone:	A                                               */
	/*  CA Stone:			B,C                                             */
	/*  threat groups:      A,C                                             */
	/************************************************************************/

	//for Long attribute in h-Type Half Bamboo
	{
		EdgeProperty edgeproperty;
		Point realStone[StoneNum] = { Point(0,0), Point(1,0), Point(0,2), Point(-1,0), Point(-1,2), Point() };
		Point endStone[StoneNum] = { Point(0,0), Point(0,2), Point() };
		Point erodedStone[StoneNum] = { Point(0,1), Point() };
		Point virtualCAStone[StoneNum] = { Point(0,1), Point() };
		Point CAStone[StoneNum] = { Point(1,1), Point(-1,1), Point() };
		Point ThreatStone[StoneNum] = { Point(0,1), Point(1,1), Point() };
		WeichiEdgeType id[8] = {EDGE_h_BAMBOO_LRU, EDGE_h_BAMBOO_LUR, EDGE_h_BAMBOO_LDR, EDGE_h_BAMBOO_LRD,
								EDGE_h_BAMBOO_LLD, EDGE_h_BAMBOO_LDL, EDGE_h_BAMBOO_LUL, EDGE_h_BAMBOO_LLU};

		edgeproperty.copy(edgeproperty.m_realStone,realStone);
		edgeproperty.copy(edgeproperty.m_endStone,endStone);
		edgeproperty.copy(edgeproperty.m_erodedStone,erodedStone);
		edgeproperty.copy(edgeproperty.m_virtualCAStone,virtualCAStone);
		edgeproperty.copy(edgeproperty.m_CAStone,CAStone);
		edgeproperty.copy(edgeproperty.m_ThreatStone,ThreatStone);

		consturctEdge(position,8,id,DIR8,edgeproperty);
	}

	//for Short attribute in h-Type Half Bamboo
	{
		EdgeProperty edgeproperty;
		Point realStone[StoneNum] = { Point(0,0), Point(1,2), Point(0,2), Point(-1,0), Point(-1,2), Point() };
		Point endStone[StoneNum] = { Point(0,0), Point(0,2), Point() };
		Point erodedStone[StoneNum] = { Point(0,1), Point() };
		Point virtualCAStone[StoneNum] = { Point(0,1), Point() };
		Point CAStone[StoneNum] = { Point(1,1), Point(-1,1), Point() };
		Point ThreatStone[StoneNum] = { Point(0,1), Point(1,1), Point() };
		WeichiEdgeType id[8] = {EDGE_h_BAMBOO_SRU, EDGE_h_BAMBOO_SUR, EDGE_h_BAMBOO_SDR, EDGE_h_BAMBOO_SRD,
								EDGE_h_BAMBOO_SLD, EDGE_h_BAMBOO_SDL, EDGE_h_BAMBOO_SUL, EDGE_h_BAMBOO_SLU};

		edgeproperty.copy(edgeproperty.m_realStone,realStone);
		edgeproperty.copy(edgeproperty.m_endStone,endStone);
		edgeproperty.copy(edgeproperty.m_erodedStone,erodedStone);
		edgeproperty.copy(edgeproperty.m_virtualCAStone,virtualCAStone);
		edgeproperty.copy(edgeproperty.m_CAStone,CAStone);
		edgeproperty.copy(edgeproperty.m_ThreatStone,ThreatStone);

		consturctEdge(position,8,id,DIR8,edgeproperty);
	}
}

void StaticEdge::initializeWallJump( uint position )
{
	/************************************************************************/
	/* Wall-Jump: (4 type)                                                  */
	/*  ---------                                                           */
	/*  D B A C E                                                           */
	/*  + + ¡´ + +                                                           */
	/*  + + + + +                                                           */
	/*                                                                      */
	/*  real Stone:			black stone                                     */
	/*  end Stone :			black stone,A                                   */
	/*  eroded Stone:		A,B,C                                           */
	/*  virtual CA Stone:	A                                               */
	/*  CA Stone:			B,C                                             */
	/*  threat groups:      A,B                                             */
	/*                      A,C                                             */
	/************************************************************************/

	EdgeProperty edgeproperty;
	Point realStone[StoneNum] = { Point(0,0), Point() };
	Point endStone[StoneNum] = { Point(0,0), Point(0,1), Point() };
	Point erodedStone[StoneNum] = { Point(0,1), Point(1,1), Point(-1,1), Point() };
	Point virtualCAStone[StoneNum] = { Point(0,1), Point() };
	Point CAStone[StoneNum] = { Point(1,1), Point(-1,1), Point() };
	Point ThreatStone[StoneNum] = { Point(0,1), Point(1,1), Point(-1,1), Point() };
	WeichiEdgeType id[1];
	ROTATION rotation[1];

	edgeproperty.copy(edgeproperty.m_realStone,realStone);
	edgeproperty.copy(edgeproperty.m_endStone,endStone);
	edgeproperty.copy(edgeproperty.m_erodedStone,erodedStone);
	edgeproperty.copy(edgeproperty.m_virtualCAStone,virtualCAStone);
	edgeproperty.copy(edgeproperty.m_CAStone,CAStone);
	edgeproperty.copy(edgeproperty.m_ThreatStone,ThreatStone);

	//for setting Wall-Jump(check if line=2)
	WeichiMove m(position);

	if( m.y()+1==WeichiConfigure::BoardSize-1 ) {
		id[0] = EDGE_WALL_JUMP_U;
		rotation[0] = ROTATION_NONE;
		consturctEdge(position,1,id,rotation,edgeproperty);
	}

	if( m.x()+1==WeichiConfigure::BoardSize-1 ) {
		id[0] = EDGE_WALL_JUMP_R;
		rotation[0] = ROTATION_4DIRECTION_CROSS_1_4;
		consturctEdge(position,1,id,rotation,edgeproperty);
	}

	if( m.y()+1==2 ) {
		id[0] = EDGE_WALL_JUMP_D;
		rotation[0] = ROTATION_4DIRECTION_CROSS_2_4;
		consturctEdge(position,1,id,rotation,edgeproperty);
	}

	if( m.x()+1==2 ) {
		id[0] = EDGE_WALL_JUMP_L;
		rotation[0] = ROTATION_4DIRECTION_CROSS_3_4;
		consturctEdge(position,1,id,rotation,edgeproperty);
	}
}

void StaticEdge::initializeWallBamboo( uint position )
{
	/************************************************************************/
	/* Wall-Bamboo: (8 type)                                                */
	/*  -----------                                                         */
	/*  + C A B D +                                                         */
	/*  + + ¡´ ¡´ + +                                                         */
	/*  + + + + + +                                                         */
	/*                                                                      */
	/*  (In point A view)                                                   */
	/*  real Stone:			black stone,A                                   */
	/*  end Stone :			black stone,A                                   */
	/*  eroded Stone:		A,B                                             */
	/*  virtual CA Stone:	A                                               */
	/*  CA Stone:			B,C                                             */
	/*  threat groups:      A,B                                             */
	/************************************************************************/

	EdgeProperty edgeproperty;
	Point realStone[StoneNum] = { Point(0,0), Point(1,0), Point() };
	Point endStone[StoneNum] = { Point(0,0), Point(0,1), Point() };
	Point erodedStone[StoneNum] = { Point(0,1), Point(1,1), Point() };
	Point virtualCAStone[StoneNum] = { Point(0,1), Point() };
	Point CAStone[StoneNum] = { Point(1,1), Point(-1,1), Point() };
	Point ThreatStone[StoneNum] = { Point(0,1), Point(1,1), Point() };
	WeichiEdgeType id[2];
	ROTATION rotation[2];

	edgeproperty.copy(edgeproperty.m_realStone,realStone);
	edgeproperty.copy(edgeproperty.m_endStone,endStone);
	edgeproperty.copy(edgeproperty.m_erodedStone,erodedStone);
	edgeproperty.copy(edgeproperty.m_virtualCAStone,virtualCAStone);
	edgeproperty.copy(edgeproperty.m_CAStone,CAStone);
	edgeproperty.copy(edgeproperty.m_ThreatStone,ThreatStone);

	//for setting Wall-Bamboo(check if line=2)
	WeichiMove m(position);

	if( m.y()+1==WeichiConfigure::BoardSize-1 ) {
		id[0] = EDGE_WALL_BAMBOO_RU;	id[1] = EDGE_WALL_BAMBOO_LU;
		rotation[0] = ROTATION_NONE; rotation[1] = ROTATION_8DIRECTION_7_8;
		consturctEdge(position,2,id,rotation,edgeproperty);
	}

	if( m.x()+1==WeichiConfigure::BoardSize-1 ) {
		id[0] = EDGE_WALL_BAMBOO_UR;	id[1] = EDGE_WALL_BAMBOO_DR;
		rotation[0] = ROTATION_8DIRECTION_1_8; rotation[1] = ROTATION_8DIRECTION_2_8;
		consturctEdge(position,2,id,rotation,edgeproperty);
	}

	if( m.y()+1==2 ) {
		id[0] = EDGE_WALL_BAMBOO_RD;	id[1] = EDGE_WALL_BAMBOO_LD;
		rotation[0] = ROTATION_8DIRECTION_3_8; rotation[1] = ROTATION_8DIRECTION_4_8;
		consturctEdge(position,2,id,rotation,edgeproperty);
	}

	if( m.x()+1==2 ) {
		id[0] = EDGE_WALL_BAMBOO_UL;	id[1] = EDGE_WALL_BAMBOO_DL;
		rotation[0] = ROTATION_8DIRECTION_5_8; rotation[1] = ROTATION_8DIRECTION_6_8;
		consturctEdge(position,2,id,rotation,edgeproperty);
	} 
}

bool StaticEdge::isEdgeRealPointExist( uint center_pos, Point* realPoint, ROTATION roateType )
{
	WeichiMove move(center_pos);
	for( uint i=0; realPoint[i].isValid(); i++ ) {
		Point rotatePoint = realPoint[i].roate(roateType);
		uint x = move.x() + rotatePoint.getX();
		uint y = move.y() + rotatePoint.getY();

		if( !isLineInBoard(x) || !isLineInBoard(y) ) { return false; }
	}

	return true;
}

WeichiBitBoard StaticEdge::getPointBitBoard( uint center_pos, Point* point, ROTATION rotateType )
{
	WeichiBitBoard bmBoard;
	WeichiMove move(center_pos);

	for( uint i=0; point[i].isValid(); i++ ) {
		Point rotatePoint = point[i].roate(rotateType);
		uint x = move.x() + rotatePoint.getX();
		uint y = move.y() + rotatePoint.getY();

		if( !isLineInBoard(x) || !isLineInBoard(y) ) { continue; }
		bmBoard.SetBitOn(WeichiMove::toPosition(x,y));
	}

	return bmBoard;
}

void StaticEdge::consturctEdge( uint position, uint size, WeichiEdgeType* type, const ROTATION* rotation, EdgeProperty edgeProperty )
{
	WeichiMove move(position);

	for( uint index=0; index<size; index++ ) {
		if( !isEdgeRealPointExist(position,edgeProperty.m_realStone,rotation[index]) ) { continue; }

		// set another end stone position
		Point rotatePoint = edgeProperty.m_endStone[1].roate(rotation[index]);
		uint x = move.x() + rotatePoint.getX();
		uint y = move.y() + rotatePoint.getY();
		m_edges[type[index]].setAnotherEndStonePos(WeichiMove::toPosition(x,y));
		
		// set other attribute
		m_edges[type[index]].setRealStone(getPointBitBoard(position,edgeProperty.m_realStone,rotation[index]));
		m_edges[type[index]].setEndStone(getPointBitBoard(position,edgeProperty.m_endStone,rotation[index]));
		m_edges[type[index]].setErodedStone(getPointBitBoard(position,edgeProperty.m_erodedStone,rotation[index]));
		m_edges[type[index]].setVirtualCAStone(getPointBitBoard(position,edgeProperty.m_virtualCAStone,rotation[index]));
		m_edges[type[index]].setCAStone(getPointBitBoard(position,edgeProperty.m_CAStone,rotation[index]));
		m_edges[type[index]].setThreatStone(getPointBitBoard(position,edgeProperty.m_virtualCAStone,rotation[index]));
	}
}

StaticEdge::Point StaticEdge::Point::symmetryXAXIS( StaticEdge::Point rotatePoint )
{
	Point newPoint;

	newPoint.setX(rotatePoint.getX());
	newPoint.setY(-rotatePoint.getY());

	return newPoint;
}

StaticEdge::Point StaticEdge::Point::symmetryYAXIS( StaticEdge::Point rotatePoint )
{
	Point newPoint;

	newPoint.setX(-rotatePoint.getX());
	newPoint.setY(rotatePoint.getY());

	return newPoint;
}

StaticEdge::Point StaticEdge::Point::symmetryXEQUALY( StaticEdge::Point rotatePoint )
{
	Point newPoint;

	newPoint.setX(rotatePoint.getY());
	newPoint.setY(rotatePoint.getX());

	return newPoint;
}

StaticEdge::Point StaticEdge::Point::symmetryXEQUALNEGATIVEY( StaticEdge::Point rotatePoint )
{
	Point newPoint;

	newPoint.setX(-rotatePoint.getY());
	newPoint.setY(-rotatePoint.getX());

	return newPoint;
}

StaticEdge::Point StaticEdge::Point::roate( ROTATION rotation )
{
	Point rotatePoint(m_x,m_y);

	switch( rotation ) {
	case ROTATION_NONE:					return rotatePoint;
	case ROTATION_4DIRECTION_CROSS_1_4:	return symmetryXEQUALY(rotatePoint);
	case ROTATION_4DIRECTION_CROSS_2_4:	return symmetryXAXIS(rotatePoint);
	case ROTATION_4DIRECTION_CROSS_3_4:	return symmetryXEQUALNEGATIVEY(rotatePoint);
	case ROTATION_4DIRECTION_X_1_4:		return symmetryXAXIS(rotatePoint);
	case ROTATION_4DIRECTION_X_2_4:		return symmetryCENTER(rotatePoint);
	case ROTATION_4DIRECTION_X_3_4:		return symmetryYAXIS(rotatePoint);
	case ROTATION_8DIRECTION_1_8:		return symmetryXEQUALY(rotatePoint);
	case ROTATION_8DIRECTION_2_8:		return symmetryXAXIS(symmetryXEQUALY(rotatePoint));
	case ROTATION_8DIRECTION_3_8:		return symmetryXAXIS(rotatePoint);
	case ROTATION_8DIRECTION_4_8:		return symmetryCENTER(rotatePoint);
	case ROTATION_8DIRECTION_5_8:		return symmetryXEQUALY(symmetryCENTER(rotatePoint));
	case ROTATION_8DIRECTION_6_8:		return symmetryXEQUALNEGATIVEY(symmetryYAXIS(rotatePoint));
	case ROTATION_8DIRECTION_7_8:		return symmetryYAXIS(rotatePoint);
	default:							return rotatePoint;
	}
}

StaticEdge::Point StaticEdge::Point::symmetryCENTER( StaticEdge::Point rotatePoint )
{
	Point newPoint;

	newPoint.setX(-rotatePoint.getX());
	newPoint.setY(-rotatePoint.getY());

	return newPoint;
}

const StaticEdge::ROTATION StaticEdge::DIR4_CROSS[4] = {
	ROTATION_NONE, 
	ROTATION_4DIRECTION_CROSS_1_4, 
	ROTATION_4DIRECTION_CROSS_2_4, 
	ROTATION_4DIRECTION_CROSS_3_4
};
const StaticEdge::ROTATION StaticEdge::DIR4_X[4] = {
	ROTATION_NONE, 
	ROTATION_4DIRECTION_X_1_4, 
	ROTATION_4DIRECTION_X_2_4, 
	ROTATION_4DIRECTION_X_3_4
};
const StaticEdge::ROTATION StaticEdge::DIR8[8] = {
	ROTATION_NONE, 
	ROTATION_8DIRECTION_1_8, 
	ROTATION_8DIRECTION_2_8, 
	ROTATION_8DIRECTION_3_8,
	ROTATION_8DIRECTION_4_8,
	ROTATION_8DIRECTION_5_8,
	ROTATION_8DIRECTION_6_8,
	ROTATION_8DIRECTION_7_8
};