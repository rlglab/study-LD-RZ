#ifndef WEICHICAHANDLER_H
#define WEICHICAHANDLER_H

#include "BasicType.h"
#include "BoardMoveStack.h"
#include "BoundingBox.h"
#include "FeatureList.h"
#include "WeichiClosedArea.h"
#include "WeichiGrid.h"

class WeichiBoard;

class WeichiCAHandler {
    friend class WeichiBoard;
    friend class WeichiUctAccessor;
    friend class TMiniMCTS; // remove this!!!!!
private:
    struct Benson {
        short m_blockNumHealthyCA[MAX_GAME_LENGTH];
        FeatureIndexList<uint, MAX_GAME_LENGTH> m_blockIDs;
        FeatureIndexList<uint, MAX_NUM_CLOSEDAREA> m_closedAreaIDs;
    };
    struct BackupFlag {
        Dual<int> m_numLifeStone;
        Dual<WeichiBitBoard> m_bmLife;
        FeatureList<WeichiClosedArea, MAX_NUM_CLOSEDAREA> m_closedAreas;
        Dual<FeatureIndexList<uint, MAX_NUM_GRIDS>> m_nakadeLists;
    };
    WeichiBoard* m_board;
    Dual<int> m_numLifeStone;
    Dual<WeichiBitBoard> m_bmLife;
    FeatureList<WeichiClosedArea, MAX_NUM_CLOSEDAREA> m_closedAreas;
    Dual<FeatureIndexList<uint, MAX_NUM_GRIDS>> m_nakadeLists;
    BackupFlag m_backupFlag;
    Vector<BackupFlag, MAX_NUM_GRIDS> m_backupStacks;

public:
    WeichiCAHandler();

    void reset();
    void initialize(WeichiBoard* board);
    void setBackupFlag();
    void restoreBackupFlag();
    void storeMoveBackup();
    void resumeMoveBackup();
    BackupFlag getCurrentBackupFlag();

    inline void findFullBoardUCTClosedArea()
    {
        setFullBoardClosedArea(COLOR_BLACK);
        setFullBoardClosedArea(COLOR_WHITE);
        setFullBoardBensonLife();
    }
    void removeAllClosedArea();
    void updateClosedArea(WeichiGrid& grid, BoardMoveStack& moveStack);
    void updateClosedAreaLifeAndDeath(WeichiGrid& grid);
    inline FeatureIndexList<uint, MAX_NUM_GRIDS>& getNakadeList(Color color) { return m_nakadeLists.get(color); }
    inline const FeatureIndexList<uint, MAX_NUM_GRIDS>& getNakadeList(Color color) const { return m_nakadeLists.get(color); }
    inline void preCalculateForEval()
    {
        if (m_closedAreas.getCapacity() == 0) { findFullBoardUCTClosedArea(); }
    }
    inline WeichiBitBoard getBensonBitboard(Color color) const { return m_bmLife.get(color); }
    bool isGameEnd() const;
    bool isEnoughTerritoryGameEnd() const;
    bool isEnoughTerritory(Color winColor) const;
    float eval(Vector<Color, MAX_NUM_GRIDS>& territory) const;
    std::string getClosedAreaDrawingString() const;
    std::string getClosedAreaLifeAndDeathString() const;

private:
    void setFullBoardClosedArea(Color findColor);
    inline void setFullBoardBensonLife()
    {
        Benson benson;
        findBensonSet(benson);
        findBensonLife(benson);
        setBensonLife(benson);
    }
    void findBensonSet(Benson& benson) const;
    void findBensonLife(Benson& benson) const;
    void setBensonLife(Benson& benson);
    void combineBlockClosedArea(WeichiBlock* mainBlock, Vector<WeichiBlock*, 4>& vCombineBlocks);
    void updateDeadBlockClosedArea(const Vector<WeichiBlock*, 4>& vDeadBlocks);
    void findAllClosedArea(WeichiGrid& grid);
    void removeClosedArea(WeichiClosedArea* closedArea);
    void mergeAndRemoveClosedArea(WeichiClosedArea* mainClosedArea, WeichiClosedArea* mergeClosedArea);
    inline void addGridToClosedArea(WeichiGrid& grid, WeichiClosedArea* closedArea)
    {
        Color myColor = closedArea->getColor();
        grid.setClosedArea(closedArea, myColor);
        closedArea->addPoint(grid.getPosition());
    }
    inline void removeGridFromClosedArea(WeichiGrid& grid, WeichiClosedArea* closedArea)
    {
        grid.setClosedArea(NULL, grid.getColor());
        closedArea->removePoint(grid.getPosition());
        WeichiBlock* block = grid.getBlock();
        if (!block->hasClosedArea(closedArea)) {
            block->addClosedArea(closedArea);
            closedArea->addBlockID(block->GetID());
        }
        reCheckClosedAreaNakade(closedArea);
    }
    void addDeadBlockToClosedArea(WeichiBlock* deadBlock, WeichiClosedArea* closedArea);
    void setClosedAreaAttribute(WeichiClosedArea* closedArea, Color caColor, WeichiBitBoard& bmStone, uint numStone);
    void reCheckClosedAreaNakade(WeichiClosedArea* closedArea);
    void findOneClosedArea(uint startPos, Color findColor, const BoundingBox& boundingBox, uint maxSize);

    void updateOwnCALifeAndDeath(WeichiGrid& grid);
    void updateOppCALifeAndDeath(WeichiGrid& grid);
    void traversalBensonSet(WeichiBlock* startBlock, Benson& benson);
    bool isCAHealthy(const WeichiClosedArea* closedArea) const;
    bool isLADEndGame() const;
    bool isAnyStoneLife(WeichiBitBoard bmStone, Color c) const;
    bool isNeedToCheckNakade(const uint numClosedArea) const { return (numClosedArea >= 3 && numClosedArea <= 6); }
    uint findNakadePoint(const WeichiClosedArea* closedArea) const;
    bool isNakadePoint(const WeichiGrid& grid, WeichiClosedArea* closedArea) const
    {
        if (!grid.isEmpty()) { return false; }

        Color oppColor = AgainstColor(closedArea->getColor());
        switch (closedArea->getNumStone()) {
            case 3: return (grid.getPattern().getPushCenter3Nodes(oppColor) > 0);
            case 4: return (grid.getPattern().getPushCenter4Nodes(oppColor) > 0);
            case 5: return (grid.getPattern().getPushCenter5Nodes(oppColor) > 0);
            case 6: return (grid.getPattern().getPushCenter6Nodes(oppColor) > 0);
            default: return false;
        }
    }

    float gameEndeval(Vector<Color, MAX_NUM_GRIDS>& territory) const;
    float earlyEndeval(Vector<Color, MAX_NUM_GRIDS>& territory) const;

    std::string getBitBoardGtpFormatString(WeichiBitBoard bmBoard) const;

    bool invariance() const;
    bool invariance_checkGridCAExist() const;
    bool invariance_checkFullBoardCA(Color findColor) const;
    bool invariance_checkBensonLife() const;
    bool invariance_checkNumLifeStone() const;
};

#endif