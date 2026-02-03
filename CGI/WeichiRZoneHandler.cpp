#include "WeichiRZoneHandler.h"

WeichiBitBoard WeichiRZoneHandler::calculateBensonRZone(const WeichiBoard& board, WeichiMove move, bool bIncludeLiberty)
{
    Color winColor = WeichiGlobalInfo::getTreeInfo().m_winColor;
    Color oppColor = AgainstColor(winColor);

    WeichiBitBoard bmCheckedInvolvedBlock, bmResult;
    Vector<const WeichiBlock*, MAX_NUM_BLOCKS> vBlocks;
    const WeichiGrid& grid = board.getGrid(move);
    if (grid.getColor() == winColor) {
        const WeichiBlock* startBlock = grid.getBlock();
        if (startBlock == NULL || startBlock->getStatus() != LAD_LIFE) { return bmResult; }
        vBlocks.push_back(startBlock);
    } else if (grid.getColor() == oppColor || grid.getColor() == COLOR_NONE) {
        const WeichiClosedArea* ca = grid.getClosedArea(winColor);
        if (ca == NULL || ca->getStatus() != LAD_LIFE) { return bmResult; }

        for (int iBlock = 0; iBlock < ca->getNumBlock(); ++iBlock) {
            const WeichiBlock* nbrBlock = board.getBlock(ca->getBlockID(iBlock));
            bmCheckedInvolvedBlock |= nbrBlock->getStoneMap();
            vBlocks.push_back(nbrBlock);
        }
    }

    while (!vBlocks.empty()) {
        const WeichiBlock* block = vBlocks.back();
        bmCheckedInvolvedBlock |= block->getStoneMap();
        if (bIncludeLiberty) { bmResult |= block->getLibertyBitBoard(board.getBitBoard()); }
        bmResult |= block->getStoneMap();
        vBlocks.pop_back();

        for (int iCA = 0; iCA < block->getNumClosedArea(); iCA++) {
            const WeichiClosedArea* ca = block->getClosedArea(iCA, board.getCloseArea());
            if (ca->getStatus() != LAD_LIFE || bmCheckedInvolvedBlock.hasIntersection(ca->getStoneMap())) {
                bmCheckedInvolvedBlock |= ca->getStoneMap();
                continue;
            }

            bmResult |= ca->getStoneMap();
            bmCheckedInvolvedBlock |= ca->getStoneMap();
            for (int iBlock = 0; iBlock < ca->getNumBlock(); ++iBlock) {
                const WeichiBlock* nbrBlock = board.getBlock(ca->getBlockID(iBlock));
                if (bmCheckedInvolvedBlock.hasIntersection(nbrBlock->getStoneMap())) { continue; }

                vBlocks.push_back(nbrBlock);
            }
        }
    }

    return bmResult;
}

WeichiBitBoard WeichiRZoneHandler::calculateDeadRZone(const WeichiBoard& board, WeichiMove move)
{
    const WeichiGrid& grid = board.getGrid(move);
    Color deadColor = grid.getColor();
    Color killColor = AgainstColor(deadColor);

    WeichiBitBoard bmCheckedInvolvedBlock, bmResult;
    Vector<const WeichiBlock*, MAX_NUM_BLOCKS> vBlocks;
    const WeichiClosedArea* ca = grid.getClosedArea(killColor);
    for (int iBlock = 0; iBlock < ca->getNumBlock(); ++iBlock) {
        const WeichiBlock* nbrBlock = board.getBlock(ca->getBlockID(iBlock));
        bmCheckedInvolvedBlock |= nbrBlock->getStoneMap();
        vBlocks.push_back(nbrBlock);
    }

    while (!vBlocks.empty()) {
        const WeichiBlock* block = vBlocks.back();
        bmCheckedInvolvedBlock |= block->getStoneMap();
        bmResult |= block->getStoneMap();
        vBlocks.pop_back();

        for (int iCA = 0; iCA < block->getNumClosedArea(); iCA++) {
            const WeichiClosedArea* ca = block->getClosedArea(iCA, board.getCloseArea());
            if (ca->getStatus() != LAD_LIFE || bmCheckedInvolvedBlock.hasIntersection(ca->getStoneMap())) {
                bmCheckedInvolvedBlock |= ca->getStoneMap();
                continue;
            }

            bmResult |= ca->getStoneMap();
            bmCheckedInvolvedBlock |= ca->getStoneMap();
            for (int iBlock = 0; iBlock < ca->getNumBlock(); ++iBlock) {
                const WeichiBlock* nbrBlock = board.getBlock(ca->getBlockID(iBlock));
                if (bmCheckedInvolvedBlock.hasIntersection(nbrBlock->getStoneMap())) { continue; }

                vBlocks.push_back(nbrBlock);
            }
        }
    }

    return bmResult;
} 
