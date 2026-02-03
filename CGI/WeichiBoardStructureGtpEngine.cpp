#include "WeichiBoardStructureGtpEngine.h"
#include "GtpColorGradient.h"

void WeichiBoardStructureGtpEngine::Register()
{
	RegisterFunction("board_status", this, &WeichiBoardStructureGtpEngine::cmdBoardStatus, 0);
	RegisterFunction("class_size", this, &WeichiBoardStructureGtpEngine::cmdClassSize, 0 );
	RegisterFunction("soft_grid_number", this, &WeichiBoardStructureGtpEngine::cmdSoftGridNumber, 0);
	RegisterFunction("compact_grid_number", this, &WeichiBoardStructureGtpEngine::cmdCompactGridNumber, 0);
	RegisterFunction("radius_neighbors", this, &WeichiBoardStructureGtpEngine::cmdRadiusNeighbors, 1);
	RegisterFunction("pattern3x3_attribute", this, &WeichiBoardStructureGtpEngine::cmdPattern3x3Attribute, 2 );

	if( WeichiConfigure::use_closed_area ) {
		RegisterFunction("closed_area", this, &WeichiBoardStructureGtpEngine::cmdClosedArea, 0);
		RegisterFunction("closed_area_option", this, &WeichiBoardStructureGtpEngine::cmdClosedAreaOption, 0 ,2);
		RegisterFunction("nakade", this, &WeichiBoardStructureGtpEngine::cmdNakade, 0);
		RegisterFunction("life_and_death", this, &WeichiBoardStructureGtpEngine::cmdLifeAndDeath, 0);
	}
	
	RegisterFunction("edge_line", this, &WeichiBoardStructureGtpEngine::cmdEdgeLine, 0);
	RegisterFunction("edge_line_option", this, &WeichiBoardStructureGtpEngine::cmdEdgeLineOption, 0, 2);
}

string WeichiBoardStructureGtpEngine::getGoguiAnalyzeCommandsString()
{
	ostringstream oss;
	oss << "string/=====Board Structure=====/empty\n"
		<< "hpstring/Board Status/board_status\n"
		<< "string/Class Size/class_size\n"
		<< "sboard/Soft Grid Number/soft_grid_number\n"
		<< "sboard/Compact Grid Number/compact_grid_number\n"
		<< "gfx/Radius Neighbors/radius_neighbors %p/\n"
		<< "string/Pattern3x3 Attributes/pattern3x3_attribute %c %p\n";

	if( WeichiConfigure::use_closed_area ) {
		oss << "gfx/Closed Area/closed_area\n"
			<< "param/Closed Area Option/closed_area_option\n"
			<< "gfx/Nakade/nakade\n"
			<< "gfx/Life & Death/life_and_death\n";
	}

	oss << "gfx/Edge Line/edge_line\n"
		<< "param/Edge Line Option/edge_line_option\n";

	return oss.str();
}

void WeichiBoardStructureGtpEngine::cmdBoardStatus()
{
	reply(GTP_SUCC, m_mcts.getState().m_board.getBoardStatus());
}

void WeichiBoardStructureGtpEngine::cmdClassSize()
{
	ostringstream oss;

	oss << "WeichiBoard Size: " << sizeof(WeichiBoard) << endl;
	oss << "WeichiGrid Size : " << sizeof(WeichiGrid) << endl;
	oss << "WeichiBlock Size: " << sizeof(WeichiBlock) << endl;

	reply(GTP_SUCC, oss.str());
}

void WeichiBoardStructureGtpEngine::cmdSoftGridNumber()
{
	ostringstream oss;
	oss << endl;
	for( StaticBoard::iterator it=StaticBoard::getGoguiIterator(); it; ++it ) {
		oss << *it << " " ;
		if( *it%MAX_BOARD_SIZE==WeichiConfigure::BoardSize-1 ) { oss << endl; }
	}
	reply(GTP_SUCC, oss.str());
}

void WeichiBoardStructureGtpEngine::cmdCompactGridNumber()
{
	ostringstream oss;
	oss << endl;
	for (StaticBoard::iterator it = StaticBoard::getGoguiIterator(); it; ++it) {
		oss << WeichiMove::toCompactPosition(*it) << " ";
		if (*it%MAX_BOARD_SIZE == WeichiConfigure::BoardSize - 1) { oss << endl; }
	}
	reply(GTP_SUCC, oss.str());
}

void WeichiBoardStructureGtpEngine::cmdRadiusNeighbors()
{
	string sPosition = m_args[0];
	transform(sPosition.begin(),sPosition.end(),sPosition.begin(),::toupper);
	Move move(COLOR_NONE, sPosition);

	Color turnColor = m_mcts.getState().m_board.getToPlay();
	const WeichiGrid& grid = m_mcts.getState().m_board.getGrid(move);

	Vector<uint,MAX_RADIUS_SIZE> vPatternIndex;
	vPatternIndex.resize(MAX_RADIUS_SIZE);
	for( int iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		int iIndex = grid.getRadiusPatternRealIndex(iRadius,turnColor);
		if( iRadius==3 ) { iIndex = (iIndex<<(RADIUS_NBR_LIB_SIZE*RADIUS_NBR_LIB_BITS)) + grid.getNbrLibIndex(); }
		vPatternIndex[iRadius] = iIndex;
	}

	HashKey64 accumalateKey = 0;
	Vector<HashKey64,MAX_RADIUS_SIZE> vMinHashKey = WeichiRadiusPatternTable::calculateMinRadiusPatternHashKey(vPatternIndex);
	CERR() << setw(10) << "Radius" << setw(20) << "hash key" << setw(20) << "min hash key" << setw(10) << "real index" << endl;
	for( int iRadius=MIN_RADIUS_SIZE; iRadius<MAX_RADIUS_SIZE; iRadius++ ) {
		accumalateKey ^= WeichiRadiusPatternTable::getRadiusPatternHashKey(iRadius,vPatternIndex[iRadius]);

		CERR() << setw(10) << iRadius
				  << setw(20) << hex << accumalateKey
				  << setw(20) << vMinHashKey[iRadius] << dec 
				  << setw(10) << vPatternIndex[iRadius] << endl;
	}
	CERR() << WeichiRadiusPatternTable::getRadiusPatternDiagramString(vPatternIndex) << endl;
	
	ostringstream oss;
	oss << "LABEL ";
	const StaticGrid::RadiusGridRelation* radiusGridRelation = grid.getStaticGrid().getRadiusGridRelations();
	for( ; radiusGridRelation->m_iRadius!=-1; radiusGridRelation++ ) {
		WeichiMove move(radiusGridRelation->m_neighborPos);
		oss << move.toGtpString() << " " << radiusGridRelation->m_iRadius << " ";
	}
	oss << endl;

	reply(GTP_SUCC, oss.str());
}

void WeichiBoardStructureGtpEngine::cmdPattern3x3Attribute()
{
	Color forColor = toColor(m_args[0].at(0));
	string sMove = m_args[1] ;
	transform(sMove.begin(),sMove.end(), sMove.begin(), ::toupper) ;
	Move move (COLOR_NONE , sMove) ;

	const WeichiBoard& board = m_mcts.getState().m_board;
	const WeichiGrid& grid = board.getGrid( move.getPosition() );

	CERR() << grid.getPattern().getIndex() << endl ;	

	grid.getPattern().showAllAttribute(forColor, grid.getColor()) ;

	ostringstream oss;
	oss << endl ;
	reply(GTP_SUCC, oss.str());
}

void WeichiBoardStructureGtpEngine::cmdClosedArea()
{
	reply(GTP_SUCC, m_mcts.getState().m_board.getClosedAreaDrawingString());
}

void WeichiBoardStructureGtpEngine::cmdClosedAreaOption()
{
	ostringstream oss;
	if ( m_args.size()==0 ) {
		oss << "[bool] draw_black " << WeichiConfigure::DrawClosedAreaBlack << '\n'
			<< "[bool] draw_white " << WeichiConfigure::DrawClosedAreaWhite << '\n'
			;
	} else if( m_args.size()==2 ) {
		if( m_args[0] == "draw_black" ) {
			WeichiConfigure::DrawClosedAreaBlack = (m_args[1] == "1");
		} else if( m_args[0] == "draw_white" ) {
			WeichiConfigure::DrawClosedAreaWhite = (m_args[1] == "1");
		} else {
			reply(GTP_FAIL, "unknown parameter " + m_args[0]);
		}
	} else {
		reply(GTP_FAIL, "need 0 or 2 arguments");
	}

	reply(GTP_SUCC, oss.str());
}

void WeichiBoardStructureGtpEngine::cmdNakade()
{
	ostringstream oss;
	const FeatureList<WeichiClosedArea, MAX_NUM_CLOSEDAREA>& closedAreas = m_mcts.getState().m_board.getCloseArea();

	oss << "SQUARE ";
	for( uint iIndex=0; iIndex<closedAreas.getCapacity(); iIndex++ ) {
		const WeichiClosedArea* closedArea = closedAreas.getAt(iIndex);
		if( !closedArea->hasNakade() ) { continue; }

		oss << WeichiMove(closedArea->getNakade()).toGtpString() << ' ';
	}
	oss << endl;

	reply(GTP_SUCC, oss.str());
}

void WeichiBoardStructureGtpEngine::cmdLifeAndDeath()
{
	reply(GTP_SUCC, m_mcts.getState().m_board.getClosedAreaLifeAndDeathString());
}

void WeichiBoardStructureGtpEngine::cmdEdgeLine()
{
	const_cast<WeichiBoard&>(m_mcts.getState().m_board).m_edgeHandler.removeAllEdges();
	const_cast<WeichiBoard&>(m_mcts.getState().m_board).m_edgeHandler.findFullBoardEdges();
	ostringstream oss;
	oss << "LINE " << m_mcts.getState().m_board.getEdgeDrawingString();
	reply(GTP_SUCC, oss.str());
}

void WeichiBoardStructureGtpEngine::cmdEdgeLineOption()
{
	ostringstream oss;
	if ( m_args.size() == 0 ) {
		oss << "[bool] draw_black " << WeichiConfigure::DrawBlack << '\n'
			<< "[bool] draw_white " << WeichiConfigure::DrawWhite << '\n'
			<< "[bool] draw_strong " << WeichiConfigure::DrawStrong << '\n'
			<< "[bool] draw_weak " << WeichiConfigure::DrawWeak << '\n'
			<< "[bool] draw_connector " << WeichiConfigure::DrawConnector << '\n'
			<< "[bool] draw_cut " << WeichiConfigure::DrawCut << '\n'
			<< "[bool] draw_dragon " << WeichiConfigure::DrawDragon << '\n'
			;
	} else if( m_args.size() == 2) {
		if( m_args[0]=="draw_black" ) {
			WeichiConfigure::DrawBlack = (m_args[1] == "1");
		} else if( m_args[0]=="draw_white" ) {
			WeichiConfigure::DrawWhite = (m_args[1] == "1");
		} else if( m_args[0]=="draw_strong" ) {
			WeichiConfigure::DrawStrong = (m_args[1] == "1");
		} else if( m_args[0]=="draw_weak" ) {
			WeichiConfigure::DrawWeak = (m_args[1] == "1");
		} else if( m_args[0]=="draw_connector" ) {
			WeichiConfigure::DrawConnector = (m_args[1] == "1");
		} else if( m_args[0]=="draw_cut" ) {
			WeichiConfigure::DrawCut = (m_args[1] == "1");
		} else if( m_args[0]=="draw_dragon" ) {
			WeichiConfigure::DrawDragon = (m_args[1] == "1");
		} else {
			reply(GTP_FAIL, "unknown parameter " + m_args[0]);
		}
	} else {
		reply(GTP_FAIL, "need 0 or 2 arguments");
	}

	reply(GTP_SUCC, oss.str());
} 
