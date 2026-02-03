#ifndef PATTERN33TABLE_H
#define PATTERN33TABLE_H

#include "PatternUtils.h"
#include "Pattern33Entry.h"

namespace pattern33 {
#define PatternEntryMonoGetter(name) \
    uint get##name ( uint pattern_index, Color center ) { \
        return m_table[pattern_index].inn[center].name; \
    } 
#define PatternEntryDualGetter(name) \
    uint get##name ( uint pattern_index, Color forColor, Color center ) { \
        return  ( (forColor==COLOR_BLACK) ? \
                   m_table[pattern_index].inn[center].Black##name : \
                   m_table[pattern_index].inn[center].White##name  ) ;\
    } 
#define PatternEntrySetter(name) \
    static void set##name ( Pattern33Entry& entry, uint value, Symbol symbols[13] ) { \
        Color center = Sym2Color(symbols[0]) ; \
        entry.inn[center].name = value ;\
    }  


class Pattern33Builder ;

class Pattern33Table 
{
friend class Pattern33Builder ; 
private:
    Pattern33Entry  m_table[TABLE_SIZE] ; 
public:

    PatternEntrySetter(CompleteEmpty) 
    PatternEntryMonoGetter(CompleteEmpty) 
    PatternEntrySetter(EmptyGridCount) 
    PatternEntryMonoGetter(EmptyGridCount) 
    PatternEntrySetter(BlackGridCount) 
    PatternEntrySetter(WhiteGridCount) 
    PatternEntryDualGetter(GridCount) 
    PatternEntrySetter(EmptyAdjGridCount) 
    PatternEntryMonoGetter(EmptyAdjGridCount) 
    PatternEntrySetter(EmptyDiagGridCount) 
    PatternEntryMonoGetter(EmptyDiagGridCount) 
    PatternEntrySetter(BlackAdjGridCount) 
    PatternEntrySetter(WhiteAdjGridCount) 
    PatternEntryDualGetter(AdjGridCount) 
    PatternEntrySetter(BlackDiagGridCount) 
    PatternEntrySetter(WhiteDiagGridCount) 
    PatternEntryDualGetter(DiagGridCount) 
    PatternEntrySetter(BlackCutArea) 
    PatternEntrySetter(WhiteCutArea) 
    PatternEntryDualGetter(CutArea) 
    PatternEntrySetter(Black2LibJump) 
    PatternEntrySetter(White2LibJump) 
    PatternEntryDualGetter(2LibJump) 
    PatternEntrySetter(BlackAnyAdjNeighbor) 
    PatternEntrySetter(WhiteAnyAdjNeighbor) 
    PatternEntryDualGetter(AnyAdjNeighbor) 
    PatternEntrySetter(BlackApproachLib) 
    PatternEntrySetter(WhiteApproachLib) 
    PatternEntryDualGetter(ApproachLib) 
    PatternEntrySetter(BlackAtariLadder) 
    PatternEntrySetter(WhiteAtariLadder) 
    PatternEntryDualGetter(AtariLadder) 
    PatternEntrySetter(BlackAtariLadderCheckPoint) 
    PatternEntrySetter(WhiteAtariLadderCheckPoint) 
    PatternEntryDualGetter(AtariLadderCheckPoint) 
    PatternEntrySetter(BlackBamboo) 
    PatternEntrySetter(WhiteBamboo) 
    PatternEntryDualGetter(Bamboo) 
    PatternEntrySetter(BlackCloseDoor) 
    PatternEntrySetter(WhiteCloseDoor) 
    PatternEntryDualGetter(CloseDoor) 
    PatternEntrySetter(BlackCloseDoorDir) 
    PatternEntrySetter(WhiteCloseDoorDir) 
    PatternEntryDualGetter(CloseDoorDir) 
    PatternEntrySetter(BlackLine1CloseDoor) 
    PatternEntrySetter(WhiteLine1CloseDoor) 
    PatternEntryDualGetter(Line1CloseDoor) 
    PatternEntrySetter(BlackConnectCutPoint) 
    PatternEntrySetter(WhiteConnectCutPoint) 
    PatternEntryDualGetter(ConnectCutPoint) 
    PatternEntrySetter(ConnectWithOneEye) 
    PatternEntryMonoGetter(ConnectWithOneEye) 
    PatternEntrySetter(BlackDeadOnlyOneEye3NodesSet1) 
    PatternEntrySetter(WhiteDeadOnlyOneEye3NodesSet1) 
    PatternEntryDualGetter(DeadOnlyOneEye3NodesSet1) 
    PatternEntrySetter(BlackDeadOnlyOneEye3NodesSetRotation) 
    PatternEntrySetter(WhiteDeadOnlyOneEye3NodesSetRotation) 
    PatternEntryDualGetter(DeadOnlyOneEye3NodesSetRotation) 
    PatternEntrySetter(BlackDeadOnlyOneEye4NodesSet1) 
    PatternEntrySetter(WhiteDeadOnlyOneEye4NodesSet1) 
    PatternEntryDualGetter(DeadOnlyOneEye4NodesSet1) 
    PatternEntrySetter(BlackDeadOnlyOneEye4NodesSetRotation) 
    PatternEntrySetter(WhiteDeadOnlyOneEye4NodesSetRotation) 
    PatternEntryDualGetter(DeadOnlyOneEye4NodesSetRotation) 
    PatternEntrySetter(BlackDeadOnlyOneEye5NodesSet1) 
    PatternEntrySetter(WhiteDeadOnlyOneEye5NodesSet1) 
    PatternEntryDualGetter(DeadOnlyOneEye5NodesSet1) 
    PatternEntrySetter(BlackDeadOnlyOneEye5NodesSetRotation) 
    PatternEntrySetter(WhiteDeadOnlyOneEye5NodesSetRotation) 
    PatternEntryDualGetter(DeadOnlyOneEye5NodesSetRotation) 
    PatternEntrySetter(BlackDeadOnlyOneEye6NodesSet1) 
    PatternEntrySetter(WhiteDeadOnlyOneEye6NodesSet1) 
    PatternEntryDualGetter(DeadOnlyOneEye6NodesSet1) 
    PatternEntrySetter(BlackDeadOnlyOneEye6NodesSetRotation) 
    PatternEntrySetter(WhiteDeadOnlyOneEye6NodesSetRotation) 
    PatternEntryDualGetter(DeadOnlyOneEye6NodesSetRotation) 
    PatternEntrySetter(BlackDiagonal) 
    PatternEntrySetter(WhiteDiagonal) 
    PatternEntryDualGetter(Diagonal) 
    PatternEntrySetter(BlackDiagonalClosedArea) 
    PatternEntrySetter(WhiteDiagonalClosedArea) 
    PatternEntryDualGetter(DiagonalClosedArea) 
    PatternEntrySetter(BlackDiagonalProduce) 
    PatternEntrySetter(WhiteDiagonalProduce) 
    PatternEntryDualGetter(DiagonalProduce) 
    PatternEntrySetter(BlackDoubleKnightMove) 
    PatternEntrySetter(WhiteDoubleKnightMove) 
    PatternEntryDualGetter(DoubleKnightMove) 
    PatternEntrySetter(Black1Diagonal_RD) 
    PatternEntrySetter(White1Diagonal_RD) 
    PatternEntryDualGetter(1Diagonal_RD) 
    PatternEntrySetter(Black2Diagonal_LU) 
    PatternEntrySetter(White2Diagonal_LU) 
    PatternEntryDualGetter(2Diagonal_LU) 
    PatternEntrySetter(Black2Diagonal_LD) 
    PatternEntrySetter(White2Diagonal_LD) 
    PatternEntryDualGetter(2Diagonal_LD) 
    PatternEntrySetter(Black3Diagonal_RU) 
    PatternEntrySetter(White3Diagonal_RU) 
    PatternEntryDualGetter(3Diagonal_RU) 
    PatternEntrySetter(Black0Diagonal_RD) 
    PatternEntrySetter(White0Diagonal_RD) 
    PatternEntryDualGetter(0Diagonal_RD) 
    PatternEntrySetter(Black3Diagonal_LU) 
    PatternEntrySetter(White3Diagonal_LU) 
    PatternEntryDualGetter(3Diagonal_LU) 
    PatternEntrySetter(Black0Diagonal_RU) 
    PatternEntrySetter(White0Diagonal_RU) 
    PatternEntryDualGetter(0Diagonal_RU) 
    PatternEntrySetter(Black1Diagonal_LD) 
    PatternEntrySetter(White1Diagonal_LD) 
    PatternEntryDualGetter(1Diagonal_LD) 
    PatternEntrySetter(Black0Jump_R) 
    PatternEntrySetter(White0Jump_R) 
    PatternEntryDualGetter(0Jump_R) 
    PatternEntrySetter(Black2Jump_L) 
    PatternEntrySetter(White2Jump_L) 
    PatternEntryDualGetter(2Jump_L) 
    PatternEntrySetter(Black1Jump_D) 
    PatternEntrySetter(White1Jump_D) 
    PatternEntryDualGetter(1Jump_D) 
    PatternEntrySetter(Black3Jump_U) 
    PatternEntrySetter(White3Jump_U) 
    PatternEntryDualGetter(3Jump_U) 
    PatternEntrySetter(Black0Knight_RRU) 
    PatternEntrySetter(White0Knight_RRU) 
    PatternEntryDualGetter(0Knight_RRU) 
    PatternEntrySetter(Black5Knight_LLD) 
    PatternEntrySetter(White5Knight_LLD) 
    PatternEntryDualGetter(5Knight_LLD) 
    PatternEntrySetter(Black0Knight_RRD) 
    PatternEntrySetter(White0Knight_RRD) 
    PatternEntryDualGetter(0Knight_RRD) 
    PatternEntrySetter(Black6Knight_LLU) 
    PatternEntrySetter(White6Knight_LLU) 
    PatternEntryDualGetter(6Knight_LLU) 
    PatternEntrySetter(Black1Knight_RDD) 
    PatternEntrySetter(White1Knight_RDD) 
    PatternEntryDualGetter(1Knight_RDD) 
    PatternEntrySetter(Black6Knight_LUU) 
    PatternEntrySetter(White6Knight_LUU) 
    PatternEntryDualGetter(6Knight_LUU) 
    PatternEntrySetter(Black1Knight_LDD) 
    PatternEntrySetter(White1Knight_LDD) 
    PatternEntryDualGetter(1Knight_LDD) 
    PatternEntrySetter(Black7Knight_RUU) 
    PatternEntrySetter(White7Knight_RUU) 
    PatternEntryDualGetter(7Knight_RUU) 
    PatternEntrySetter(Black4Knight_RRD) 
    PatternEntrySetter(White4Knight_RRD) 
    PatternEntryDualGetter(4Knight_RRD) 
    PatternEntrySetter(Black2Knight_LLU) 
    PatternEntrySetter(White2Knight_LLU) 
    PatternEntryDualGetter(2Knight_LLU) 
    PatternEntrySetter(Black2Knight_LLD) 
    PatternEntrySetter(White2Knight_LLD) 
    PatternEntryDualGetter(2Knight_LLD) 
    PatternEntrySetter(Black7Knight_RRU) 
    PatternEntrySetter(White7Knight_RRU) 
    PatternEntryDualGetter(7Knight_RRU) 
    PatternEntrySetter(Black5Knight_LDD) 
    PatternEntrySetter(White5Knight_LDD) 
    PatternEntryDualGetter(5Knight_LDD) 
    PatternEntrySetter(Black3Knight_RUU) 
    PatternEntrySetter(White3Knight_RUU) 
    PatternEntryDualGetter(3Knight_RUU) 
    PatternEntrySetter(Black4Knight_RDD) 
    PatternEntrySetter(White4Knight_RDD) 
    PatternEntryDualGetter(4Knight_RDD) 
    PatternEntrySetter(Black3Knight_LUU) 
    PatternEntrySetter(White3Knight_LUU) 
    PatternEntryDualGetter(3Knight_LUU) 
    PatternEntrySetter(Black1Bamboo_RD) 
    PatternEntrySetter(White1Bamboo_RD) 
    PatternEntryDualGetter(1Bamboo_RD) 
    PatternEntrySetter(Black3Bamboo_RU) 
    PatternEntrySetter(White3Bamboo_RU) 
    PatternEntryDualGetter(3Bamboo_RU) 
    PatternEntrySetter(Black1Bamboo_LD) 
    PatternEntrySetter(White1Bamboo_LD) 
    PatternEntryDualGetter(1Bamboo_LD) 
    PatternEntrySetter(Black3Bamboo_LU) 
    PatternEntrySetter(White3Bamboo_LU) 
    PatternEntryDualGetter(3Bamboo_LU) 
    PatternEntrySetter(Black0Bamboo_UR) 
    PatternEntrySetter(White0Bamboo_UR) 
    PatternEntryDualGetter(0Bamboo_UR) 
    PatternEntrySetter(Black2Bamboo_UL) 
    PatternEntrySetter(White2Bamboo_UL) 
    PatternEntryDualGetter(2Bamboo_UL) 
    PatternEntrySetter(Black0Bamboo_DR) 
    PatternEntrySetter(White0Bamboo_DR) 
    PatternEntryDualGetter(0Bamboo_DR) 
    PatternEntrySetter(Black2Bamboo_DL) 
    PatternEntrySetter(White2Bamboo_DL) 
    PatternEntryDualGetter(2Bamboo_DL) 
    PatternEntrySetter(Black1hBamboo_SLD) 
    PatternEntrySetter(White1hBamboo_SLD) 
    PatternEntryDualGetter(1hBamboo_SLD) 
    PatternEntrySetter(Black3hBamboo_LLU) 
    PatternEntrySetter(White3hBamboo_LLU) 
    PatternEntryDualGetter(3hBamboo_LLU) 
    PatternEntrySetter(Black1hBamboo_SRD) 
    PatternEntrySetter(White1hBamboo_SRD) 
    PatternEntryDualGetter(1hBamboo_SRD) 
    PatternEntrySetter(Black3hBamboo_LRU) 
    PatternEntrySetter(White3hBamboo_LRU) 
    PatternEntryDualGetter(3hBamboo_LRU) 
    PatternEntrySetter(Black0hBamboo_LDR) 
    PatternEntrySetter(White0hBamboo_LDR) 
    PatternEntryDualGetter(0hBamboo_LDR) 
    PatternEntrySetter(Black2hBamboo_SDL) 
    PatternEntrySetter(White2hBamboo_SDL) 
    PatternEntryDualGetter(2hBamboo_SDL) 
    PatternEntrySetter(Black0hBamboo_LUR) 
    PatternEntrySetter(White0hBamboo_LUR) 
    PatternEntryDualGetter(0hBamboo_LUR) 
    PatternEntrySetter(Black2hBamboo_SUL) 
    PatternEntrySetter(White2hBamboo_SUL) 
    PatternEntryDualGetter(2hBamboo_SUL) 
    PatternEntrySetter(Black1hBamboo_LRD) 
    PatternEntrySetter(White1hBamboo_LRD) 
    PatternEntryDualGetter(1hBamboo_LRD) 
    PatternEntrySetter(Black3hBamboo_SRU) 
    PatternEntrySetter(White3hBamboo_SRU) 
    PatternEntryDualGetter(3hBamboo_SRU) 
    PatternEntrySetter(Black1hBamboo_LLD) 
    PatternEntrySetter(White1hBamboo_LLD) 
    PatternEntryDualGetter(1hBamboo_LLD) 
    PatternEntrySetter(Black3hBamboo_SLU) 
    PatternEntrySetter(White3hBamboo_SLU) 
    PatternEntryDualGetter(3hBamboo_SLU) 
    PatternEntrySetter(Black0hBamboo_SUR) 
    PatternEntrySetter(White0hBamboo_SUR) 
    PatternEntryDualGetter(0hBamboo_SUR) 
    PatternEntrySetter(Black2hBamboo_LUL) 
    PatternEntrySetter(White2hBamboo_LUL) 
    PatternEntryDualGetter(2hBamboo_LUL) 
    PatternEntrySetter(Black0hBamboo_SDR) 
    PatternEntrySetter(White0hBamboo_SDR) 
    PatternEntryDualGetter(0hBamboo_SDR) 
    PatternEntrySetter(Black2hBamboo_LDL) 
    PatternEntrySetter(White2hBamboo_LDL) 
    PatternEntryDualGetter(2hBamboo_LDL) 
    PatternEntrySetter(Black0LHalfBamboo_SUR) 
    PatternEntrySetter(White0LHalfBamboo_SUR) 
    PatternEntryDualGetter(0LHalfBamboo_SUR) 
    PatternEntrySetter(Black2LHalfBamboo_LUL) 
    PatternEntrySetter(White2LHalfBamboo_LUL) 
    PatternEntryDualGetter(2LHalfBamboo_LUL) 
    PatternEntrySetter(Black0LHalfBamboo_SDR) 
    PatternEntrySetter(White0LHalfBamboo_SDR) 
    PatternEntryDualGetter(0LHalfBamboo_SDR) 
    PatternEntrySetter(Black2LHalfBamboo_LDL) 
    PatternEntrySetter(White2LHalfBamboo_LDL) 
    PatternEntryDualGetter(2LHalfBamboo_LDL) 
    PatternEntrySetter(Black1LHalfBamboo_SLD) 
    PatternEntrySetter(White1LHalfBamboo_SLD) 
    PatternEntryDualGetter(1LHalfBamboo_SLD) 
    PatternEntrySetter(Black3LHalfBamboo_LLU) 
    PatternEntrySetter(White3LHalfBamboo_LLU) 
    PatternEntryDualGetter(3LHalfBamboo_LLU) 
    PatternEntrySetter(Black1LHalfBamboo_SRD) 
    PatternEntrySetter(White1LHalfBamboo_SRD) 
    PatternEntryDualGetter(1LHalfBamboo_SRD) 
    PatternEntrySetter(Black3LHalfBamboo_LRU) 
    PatternEntrySetter(White3LHalfBamboo_LRU) 
    PatternEntryDualGetter(3LHalfBamboo_LRU) 
    PatternEntrySetter(Black0LHalfBamboo_LUR) 
    PatternEntrySetter(White0LHalfBamboo_LUR) 
    PatternEntryDualGetter(0LHalfBamboo_LUR) 
    PatternEntrySetter(Black2LHalfBamboo_SUL) 
    PatternEntrySetter(White2LHalfBamboo_SUL) 
    PatternEntryDualGetter(2LHalfBamboo_SUL) 
    PatternEntrySetter(Black0LHalfBamboo_LDR) 
    PatternEntrySetter(White0LHalfBamboo_LDR) 
    PatternEntryDualGetter(0LHalfBamboo_LDR) 
    PatternEntrySetter(Black2LHalfBamboo_SDL) 
    PatternEntrySetter(White2LHalfBamboo_SDL) 
    PatternEntryDualGetter(2LHalfBamboo_SDL) 
    PatternEntrySetter(Black1LHalfBamboo_LRD) 
    PatternEntrySetter(White1LHalfBamboo_LRD) 
    PatternEntryDualGetter(1LHalfBamboo_LRD) 
    PatternEntrySetter(Black3LHalfBamboo_SRU) 
    PatternEntrySetter(White3LHalfBamboo_SRU) 
    PatternEntryDualGetter(3LHalfBamboo_SRU) 
    PatternEntrySetter(Black1LHalfBamboo_LLD) 
    PatternEntrySetter(White1LHalfBamboo_LLD) 
    PatternEntryDualGetter(1LHalfBamboo_LLD) 
    PatternEntrySetter(Black3LHalfBamboo_SLU) 
    PatternEntrySetter(White3LHalfBamboo_SLU) 
    PatternEntryDualGetter(3LHalfBamboo_SLU) 
    PatternEntrySetter(Black1ZHalfBamboo_RD) 
    PatternEntrySetter(White1ZHalfBamboo_RD) 
    PatternEntryDualGetter(1ZHalfBamboo_RD) 
    PatternEntrySetter(Black3ZHalfBamboo_LU) 
    PatternEntrySetter(White3ZHalfBamboo_LU) 
    PatternEntryDualGetter(3ZHalfBamboo_LU) 
    PatternEntrySetter(Black1ZHalfBamboo_LD) 
    PatternEntrySetter(White1ZHalfBamboo_LD) 
    PatternEntryDualGetter(1ZHalfBamboo_LD) 
    PatternEntrySetter(Black3ZHalfBamboo_RU) 
    PatternEntrySetter(White3ZHalfBamboo_RU) 
    PatternEntryDualGetter(3ZHalfBamboo_RU) 
    PatternEntrySetter(Black0ZHalfBamboo_UR) 
    PatternEntrySetter(White0ZHalfBamboo_UR) 
    PatternEntryDualGetter(0ZHalfBamboo_UR) 
    PatternEntrySetter(Black2ZHalfBamboo_DL) 
    PatternEntrySetter(White2ZHalfBamboo_DL) 
    PatternEntryDualGetter(2ZHalfBamboo_DL) 
    PatternEntrySetter(Black0ZHalfBamboo_DR) 
    PatternEntrySetter(White0ZHalfBamboo_DR) 
    PatternEntryDualGetter(0ZHalfBamboo_DR) 
    PatternEntrySetter(Black2ZHalfBamboo_UL) 
    PatternEntrySetter(White2ZHalfBamboo_UL) 
    PatternEntryDualGetter(2ZHalfBamboo_UL) 
    PatternEntrySetter(Black0THalfBamboo_SR) 
    PatternEntrySetter(White0THalfBamboo_SR) 
    PatternEntryDualGetter(0THalfBamboo_SR) 
    PatternEntrySetter(Black2THalfBamboo_LL) 
    PatternEntrySetter(White2THalfBamboo_LL) 
    PatternEntryDualGetter(2THalfBamboo_LL) 
    PatternEntrySetter(Black1THalfBamboo_SD) 
    PatternEntrySetter(White1THalfBamboo_SD) 
    PatternEntryDualGetter(1THalfBamboo_SD) 
    PatternEntrySetter(Black3THalfBamboo_LU) 
    PatternEntrySetter(White3THalfBamboo_LU) 
    PatternEntryDualGetter(3THalfBamboo_LU) 
    PatternEntrySetter(Black0THalfBamboo_LR) 
    PatternEntrySetter(White0THalfBamboo_LR) 
    PatternEntryDualGetter(0THalfBamboo_LR) 
    PatternEntrySetter(Black2THalfBamboo_SL) 
    PatternEntrySetter(White2THalfBamboo_SL) 
    PatternEntryDualGetter(2THalfBamboo_SL) 
    PatternEntrySetter(Black1THalfBamboo_LD) 
    PatternEntrySetter(White1THalfBamboo_LD) 
    PatternEntryDualGetter(1THalfBamboo_LD) 
    PatternEntrySetter(Black3THalfBamboo_SU) 
    PatternEntrySetter(White3THalfBamboo_SU) 
    PatternEntryDualGetter(3THalfBamboo_SU) 
    PatternEntrySetter(Black3WallJump_U) 
    PatternEntrySetter(White3WallJump_U) 
    PatternEntryDualGetter(3WallJump_U) 
    PatternEntrySetter(Black0WallJump_R) 
    PatternEntrySetter(White0WallJump_R) 
    PatternEntryDualGetter(0WallJump_R) 
    PatternEntrySetter(Black1WallJump_D) 
    PatternEntrySetter(White1WallJump_D) 
    PatternEntryDualGetter(1WallJump_D) 
    PatternEntrySetter(Black2WallJump_L) 
    PatternEntrySetter(White2WallJump_L) 
    PatternEntryDualGetter(2WallJump_L) 
    PatternEntrySetter(Black3WallBamboo_RU) 
    PatternEntrySetter(White3WallBamboo_RU) 
    PatternEntryDualGetter(3WallBamboo_RU) 
    PatternEntrySetter(Black3WallBamboo_LU) 
    PatternEntrySetter(White3WallBamboo_LU) 
    PatternEntryDualGetter(3WallBamboo_LU) 
    PatternEntrySetter(Black0WallBamboo_DR) 
    PatternEntrySetter(White0WallBamboo_DR) 
    PatternEntryDualGetter(0WallBamboo_DR) 
    PatternEntrySetter(Black0WallBamboo_UR) 
    PatternEntrySetter(White0WallBamboo_UR) 
    PatternEntryDualGetter(0WallBamboo_UR) 
    PatternEntrySetter(Black1WallBamboo_RD) 
    PatternEntrySetter(White1WallBamboo_RD) 
    PatternEntryDualGetter(1WallBamboo_RD) 
    PatternEntrySetter(Black1WallBamboo_LD) 
    PatternEntrySetter(White1WallBamboo_LD) 
    PatternEntryDualGetter(1WallBamboo_LD) 
    PatternEntrySetter(Black2WallBamboo_DL) 
    PatternEntrySetter(White2WallBamboo_DL) 
    PatternEntryDualGetter(2WallBamboo_DL) 
    PatternEntrySetter(Black2WallBamboo_UL) 
    PatternEntrySetter(White2WallBamboo_UL) 
    PatternEntryDualGetter(2WallBamboo_UL) 
    PatternEntrySetter(BlackElephant) 
    PatternEntrySetter(WhiteElephant) 
    PatternEntryDualGetter(Elephant) 
    PatternEntrySetter(BlackElephantThreat1) 
    PatternEntrySetter(WhiteElephantThreat1) 
    PatternEntryDualGetter(ElephantThreat1) 
    PatternEntrySetter(BlackElephantThreat2) 
    PatternEntrySetter(WhiteElephantThreat2) 
    PatternEntryDualGetter(ElephantThreat2) 
    PatternEntrySetter(BlackElephantThreat3) 
    PatternEntrySetter(WhiteElephantThreat3) 
    PatternEntryDualGetter(ElephantThreat3) 
    PatternEntrySetter(BlackElephantThreat4) 
    PatternEntrySetter(WhiteElephantThreat4) 
    PatternEntryDualGetter(ElephantThreat4) 
    PatternEntrySetter(EmptyPosition) 
    PatternEntryMonoGetter(EmptyPosition) 
    PatternEntrySetter(BlackEye) 
    PatternEntrySetter(WhiteEye) 
    PatternEntryDualGetter(Eye) 
    PatternEntrySetter(BlackEyeConnectorTigerMouth) 
    PatternEntrySetter(WhiteEyeConnectorTigerMouth) 
    PatternEntryDualGetter(EyeConnectorTigerMouth) 
    PatternEntrySetter(BlackEyeConnectorDiagonal) 
    PatternEntrySetter(WhiteEyeConnectorDiagonal) 
    PatternEntryDualGetter(EyeConnectorDiagonal) 
    PatternEntrySetter(BlackEyeConnectorFalseEye) 
    PatternEntrySetter(WhiteEyeConnectorFalseEye) 
    PatternEntryDualGetter(EyeConnectorFalseEye) 
    PatternEntrySetter(BlackEyeConnectorTigerMounthOneLibDeadBlock) 
    PatternEntrySetter(WhiteEyeConnectorTigerMounthOneLibDeadBlock) 
    PatternEntryDualGetter(EyeConnectorTigerMounthOneLibDeadBlock) 
    PatternEntrySetter(BlackEdgeBamboo) 
    PatternEntrySetter(WhiteEdgeBamboo) 
    PatternEntryDualGetter(EdgeBamboo) 
    PatternEntrySetter(BlackEdgeBambooDirAnother) 
    PatternEntrySetter(WhiteEdgeBambooDirAnother) 
    PatternEntryDualGetter(EdgeBambooDirAnother) 
    PatternEntrySetter(BlackEdgeHalfBamboo) 
    PatternEntrySetter(WhiteEdgeHalfBamboo) 
    PatternEntryDualGetter(EdgeHalfBamboo) 
    PatternEntrySetter(BlackEdgeHalfBambooDirAnother) 
    PatternEntrySetter(WhiteEdgeHalfBambooDirAnother) 
    PatternEntryDualGetter(EdgeHalfBambooDirAnother) 
    PatternEntrySetter(BlackEdgeWallBamboo) 
    PatternEntrySetter(WhiteEdgeWallBamboo) 
    PatternEntryDualGetter(EdgeWallBamboo) 
    PatternEntrySetter(BlackEdgeWallBambooDirAnother) 
    PatternEntrySetter(WhiteEdgeWallBambooDirAnother) 
    PatternEntryDualGetter(EdgeWallBambooDirAnother) 
    PatternEntrySetter(BlackEdgeWallJump) 
    PatternEntrySetter(WhiteEdgeWallJump) 
    PatternEntryDualGetter(EdgeWallJump) 
    PatternEntrySetter(BlackEdgeWallJumpDirAnother) 
    PatternEntrySetter(WhiteEdgeWallJumpDirAnother) 
    PatternEntryDualGetter(EdgeWallJumpDirAnother) 
    PatternEntrySetter(BlackEdgeJump) 
    PatternEntrySetter(WhiteEdgeJump) 
    PatternEntryDualGetter(EdgeJump) 
    PatternEntrySetter(BlackEdgeJumpDirAnother) 
    PatternEntrySetter(WhiteEdgeJumpDirAnother) 
    PatternEntryDualGetter(EdgeJumpDirAnother) 
    PatternEntrySetter(BlackEdgeBan) 
    PatternEntrySetter(WhiteEdgeBan) 
    PatternEntryDualGetter(EdgeBan) 
    PatternEntrySetter(BlackFalseEye) 
    PatternEntrySetter(WhiteFalseEye) 
    PatternEntryDualGetter(FalseEye) 
    PatternEntrySetter(BlackSize3FalseEye) 
    PatternEntrySetter(WhiteSize3FalseEye) 
    PatternEntryDualGetter(Size3FalseEye) 
    PatternEntrySetter(BlackSize3FalseEyeCheckDir) 
    PatternEntrySetter(WhiteSize3FalseEyeCheckDir) 
    PatternEntryDualGetter(Size3FalseEyeCheckDir) 
    PatternEntrySetter(BlackFixTrueEye) 
    PatternEntrySetter(WhiteFixTrueEye) 
    PatternEntryDualGetter(FixTrueEye) 
    PatternEntrySetter(BlackFixPotentialTrueEye) 
    PatternEntrySetter(WhiteFixPotentialTrueEye) 
    PatternEntryDualGetter(FixPotentialTrueEye) 
    PatternEntrySetter(BlackFixFalseEye) 
    PatternEntrySetter(WhiteFixFalseEye) 
    PatternEntryDualGetter(FixFalseEye) 
    PatternEntrySetter(BlackGoodPatternOrder) 
    PatternEntrySetter(WhiteGoodPatternOrder) 
    PatternEntryDualGetter(GoodPatternOrder) 
    PatternEntrySetter(BlackGoodPattern) 
    PatternEntrySetter(WhiteGoodPattern) 
    PatternEntryDualGetter(GoodPattern) 
    PatternEntrySetter(BlackHalfBamboo) 
    PatternEntrySetter(WhiteHalfBamboo) 
    PatternEntryDualGetter(HalfBamboo) 
    PatternEntrySetter(BlackHalfBambooThreatPatternDirection) 
    PatternEntrySetter(WhiteHalfBambooThreatPatternDirection) 
    PatternEntryDualGetter(HalfBambooThreatPatternDirection) 
    PatternEntrySetter(BlackHalfBambooThreats) 
    PatternEntrySetter(WhiteHalfBambooThreats) 
    PatternEntryDualGetter(HalfBambooThreats) 
    PatternEntrySetter(BlackJump) 
    PatternEntrySetter(WhiteJump) 
    PatternEntryDualGetter(Jump) 
    PatternEntrySetter(BlackKnight) 
    PatternEntrySetter(WhiteKnight) 
    PatternEntryDualGetter(Knight) 
    PatternEntrySetter(BlackKnightThreat1) 
    PatternEntrySetter(WhiteKnightThreat1) 
    PatternEntryDualGetter(KnightThreat1) 
    PatternEntrySetter(BlackKnightThreat2) 
    PatternEntrySetter(WhiteKnightThreat2) 
    PatternEntryDualGetter(KnightThreat2) 
    PatternEntrySetter(BlackKoRZoneCheck) 
    PatternEntrySetter(WhiteKoRZoneCheck) 
    PatternEntryDualGetter(KoRZoneCheck) 
    PatternEntrySetter(BlackIsLadder) 
    PatternEntrySetter(WhiteIsLadder) 
    PatternEntryDualGetter(IsLadder) 
    PatternEntrySetter(BlackLadderDir) 
    PatternEntrySetter(WhiteLadderDir) 
    PatternEntryDualGetter(LadderDir) 
    PatternEntrySetter(BlackLadderType) 
    PatternEntrySetter(WhiteLadderType) 
    PatternEntryDualGetter(LadderType) 
    PatternEntrySetter(BlackLibTightenType) 
    PatternEntrySetter(WhiteLibTightenType) 
    PatternEntryDualGetter(LibTightenType) 
    PatternEntrySetter(Black2LibInnerLib) 
    PatternEntrySetter(White2LibInnerLib) 
    PatternEntryDualGetter(2LibInnerLib) 
    PatternEntrySetter(BlackFalseEye2stone) 
    PatternEntrySetter(WhiteFalseEye2stone) 
    PatternEntryDualGetter(FalseEye2stone) 
    PatternEntrySetter(BlackCheckProectedConnected2lib) 
    PatternEntrySetter(WhiteCheckProectedConnected2lib) 
    PatternEntryDualGetter(CheckProectedConnected2lib) 
    PatternEntrySetter(BlackMakeTrueEye) 
    PatternEntrySetter(WhiteMakeTrueEye) 
    PatternEntryDualGetter(MakeTrueEye) 
    PatternEntrySetter(BlackMakePotentialTrueEye) 
    PatternEntrySetter(WhiteMakePotentialTrueEye) 
    PatternEntryDualGetter(MakePotentialTrueEye) 
    PatternEntrySetter(BlackMakeFalseEye) 
    PatternEntrySetter(WhiteMakeFalseEye) 
    PatternEntryDualGetter(MakeFalseEye) 
    PatternEntrySetter(BlackMakeKo) 
    PatternEntrySetter(WhiteMakeKo) 
    PatternEntryDualGetter(MakeKo) 
    PatternEntrySetter(BlackMakeKoBaseDir) 
    PatternEntrySetter(WhiteMakeKoBaseDir) 
    PatternEntryDualGetter(MakeKoBaseDir) 
    PatternEntrySetter(BlackMakeKoCheckDir) 
    PatternEntrySetter(WhiteMakeKoCheckDir) 
    PatternEntryDualGetter(MakeKoCheckDir) 
    PatternEntrySetter(BlackMoGoHane) 
    PatternEntrySetter(WhiteMoGoHane) 
    PatternEntryDualGetter(MoGoHane) 
    PatternEntrySetter(BlackMoGoCut1) 
    PatternEntrySetter(WhiteMoGoCut1) 
    PatternEntryDualGetter(MoGoCut1) 
    PatternEntrySetter(BlackMoGoCut1Exclude) 
    PatternEntrySetter(WhiteMoGoCut1Exclude) 
    PatternEntryDualGetter(MoGoCut1Exclude) 
    PatternEntrySetter(BlackMoGoCut2) 
    PatternEntrySetter(WhiteMoGoCut2) 
    PatternEntryDualGetter(MoGoCut2) 
    PatternEntrySetter(BlackMoGoEdge) 
    PatternEntrySetter(WhiteMoGoEdge) 
    PatternEntryDualGetter(MoGoEdge) 
    PatternEntrySetter(BlackMultipleDiagonal) 
    PatternEntrySetter(WhiteMultipleDiagonal) 
    PatternEntryDualGetter(MultipleDiagonal) 
    PatternEntrySetter(BlackNakade) 
    PatternEntrySetter(WhiteNakade) 
    PatternEntryDualGetter(Nakade) 
    PatternEntrySetter(BlackSZNakade) 
    PatternEntrySetter(WhiteSZNakade) 
    PatternEntryDualGetter(SZNakade) 
    PatternEntrySetter(Black2LineNakade) 
    PatternEntrySetter(White2LineNakade) 
    PatternEntryDualGetter(2LineNakade) 
    PatternEntrySetter(BlackOneOfAdjPos) 
    PatternEntrySetter(WhiteOneOfAdjPos) 
    PatternEntryDualGetter(OneOfAdjPos) 
    PatternEntrySetter(BlackOnlyOneLiberty) 
    PatternEntrySetter(WhiteOnlyOneLiberty) 
    PatternEntryDualGetter(OnlyOneLiberty) 
    PatternEntrySetter(BlackOppEmpty) 
    PatternEntrySetter(WhiteOppEmpty) 
    PatternEntryDualGetter(OppEmpty) 
    PatternEntrySetter(BlackCheckDiagonal) 
    PatternEntrySetter(WhiteCheckDiagonal) 
    PatternEntryDualGetter(CheckDiagonal) 
    PatternEntrySetter(BlackCheckDiagonalDir) 
    PatternEntrySetter(WhiteCheckDiagonalDir) 
    PatternEntryDualGetter(CheckDiagonalDir) 
    PatternEntrySetter(BlackTryMakeSpecialEye) 
    PatternEntrySetter(WhiteTryMakeSpecialEye) 
    PatternEntryDualGetter(TryMakeSpecialEye) 
    PatternEntrySetter(BlackTryMakeSpecialEyeExtendDir) 
    PatternEntrySetter(WhiteTryMakeSpecialEyeExtendDir) 
    PatternEntryDualGetter(TryMakeSpecialEyeExtendDir) 
    PatternEntrySetter(BlackTryMakeSpecialEyeCapDir) 
    PatternEntrySetter(WhiteTryMakeSpecialEyeCapDir) 
    PatternEntryDualGetter(TryMakeSpecialEyeCapDir) 
    PatternEntrySetter(BlackTryMakeSpecialEyeEnemyDir) 
    PatternEntrySetter(WhiteTryMakeSpecialEyeEnemyDir) 
    PatternEntryDualGetter(TryMakeSpecialEyeEnemyDir) 
    PatternEntrySetter(BlackTryMakeSpecialEyeTwoLibCheck) 
    PatternEntrySetter(WhiteTryMakeSpecialEyeTwoLibCheck) 
    PatternEntryDualGetter(TryMakeSpecialEyeTwoLibCheck) 
    PatternEntrySetter(BlackOnelinePatternEyeTypeIExtendDir) 
    PatternEntrySetter(WhiteOnelinePatternEyeTypeIExtendDir) 
    PatternEntryDualGetter(OnelinePatternEyeTypeIExtendDir) 
    PatternEntrySetter(BlackOnelinePatternEyeTypeICapDir) 
    PatternEntrySetter(WhiteOnelinePatternEyeTypeICapDir) 
    PatternEntryDualGetter(OnelinePatternEyeTypeICapDir) 
    PatternEntrySetter(BlackOnelinePatternEyeTypeICheckEnemy) 
    PatternEntrySetter(WhiteOnelinePatternEyeTypeICheckEnemy) 
    PatternEntryDualGetter(OnelinePatternEyeTypeICheckEnemy) 
    PatternEntrySetter(BlackOnelinePatternEyeTypeITwoLibCheck) 
    PatternEntrySetter(WhiteOnelinePatternEyeTypeITwoLibCheck) 
    PatternEntryDualGetter(OnelinePatternEyeTypeITwoLibCheck) 
    PatternEntrySetter(BlackMakeSplitToOnelinePatternEye) 
    PatternEntrySetter(WhiteMakeSplitToOnelinePatternEye) 
    PatternEntryDualGetter(MakeSplitToOnelinePatternEye) 
    PatternEntrySetter(BlackMakeSplitToOnelinePatternEyeExtendDir) 
    PatternEntrySetter(WhiteMakeSplitToOnelinePatternEyeExtendDir) 
    PatternEntryDualGetter(MakeSplitToOnelinePatternEyeExtendDir) 
    PatternEntrySetter(BlackMakeSplitToOnelinePatternEyeCapDir) 
    PatternEntrySetter(WhiteMakeSplitToOnelinePatternEyeCapDir) 
    PatternEntryDualGetter(MakeSplitToOnelinePatternEyeCapDir) 
    PatternEntrySetter(BlackMakeSplitToOnelinePatternEyeEnemyDir) 
    PatternEntrySetter(WhiteMakeSplitToOnelinePatternEyeEnemyDir) 
    PatternEntryDualGetter(MakeSplitToOnelinePatternEyeEnemyDir) 
    PatternEntrySetter(BlackWallJump) 
    PatternEntrySetter(WhiteWallJump) 
    PatternEntryDualGetter(WallJump) 
    PatternEntrySetter(BlackWallJumpMiaiDir) 
    PatternEntrySetter(WhiteWallJumpMiaiDir) 
    PatternEntryDualGetter(WallJumpMiaiDir) 
    PatternEntrySetter(BlackOnelinePatternEyeTypeLExtendDir) 
    PatternEntrySetter(WhiteOnelinePatternEyeTypeLExtendDir) 
    PatternEntryDualGetter(OnelinePatternEyeTypeLExtendDir) 
    PatternEntrySetter(BlackOnelinePatternEyeTypeLBottomDir) 
    PatternEntrySetter(WhiteOnelinePatternEyeTypeLBottomDir) 
    PatternEntryDualGetter(OnelinePatternEyeTypeLBottomDir) 
    PatternEntrySetter(BlackOneLineCaptureDiagonal) 
    PatternEntrySetter(WhiteOneLineCaptureDiagonal) 
    PatternEntryDualGetter(OneLineCaptureDiagonal) 
    PatternEntrySetter(BlackOneLineOneStoneCaptureDiagonal) 
    PatternEntrySetter(WhiteOneLineOneStoneCaptureDiagonal) 
    PatternEntryDualGetter(OneLineOneStoneCaptureDiagonal) 
    PatternEntrySetter(BlackOneLineOneStoneNextCaptureDiagonal) 
    PatternEntrySetter(WhiteOneLineOneStoneNextCaptureDiagonal) 
    PatternEntryDualGetter(OneLineOneStoneNextCaptureDiagonal) 
    PatternEntrySetter(BlackPerfectClosedArea) 
    PatternEntrySetter(WhitePerfectClosedArea) 
    PatternEntryDualGetter(PerfectClosedArea) 
    PatternEntrySetter(BlackPotentialTrueEye) 
    PatternEntrySetter(WhitePotentialTrueEye) 
    PatternEntryDualGetter(PotentialTrueEye) 
    PatternEntrySetter(BlackPushCenter3Nodes) 
    PatternEntrySetter(WhitePushCenter3Nodes) 
    PatternEntryDualGetter(PushCenter3Nodes) 
    PatternEntrySetter(BlackPushCenter4Nodes) 
    PatternEntrySetter(WhitePushCenter4Nodes) 
    PatternEntryDualGetter(PushCenter4Nodes) 
    PatternEntrySetter(BlackPushCenter5Nodes) 
    PatternEntrySetter(WhitePushCenter5Nodes) 
    PatternEntryDualGetter(PushCenter5Nodes) 
    PatternEntrySetter(BlackPushCenter6Nodes) 
    PatternEntrySetter(WhitePushCenter6Nodes) 
    PatternEntryDualGetter(PushCenter6Nodes) 
    PatternEntrySetter(BlackReduceEye) 
    PatternEntrySetter(WhiteReduceEye) 
    PatternEntryDualGetter(ReduceEye) 
    PatternEntrySetter(BlackSplit) 
    PatternEntrySetter(WhiteSplit) 
    PatternEntryDualGetter(Split) 
    PatternEntrySetter(BlackStartCAPosition) 
    PatternEntrySetter(WhiteStartCAPosition) 
    PatternEntryDualGetter(StartCAPosition) 
    PatternEntrySetter(BlackIsSplitCA) 
    PatternEntrySetter(WhiteIsSplitCA) 
    PatternEntryDualGetter(IsSplitCA) 
    PatternEntrySetter(BlackEdge) 
    PatternEntrySetter(WhiteEdge) 
    PatternEntryDualGetter(Edge) 
    PatternEntrySetter(BlackStrongCutTwoSidePoint) 
    PatternEntrySetter(WhiteStrongCutTwoSidePoint) 
    PatternEntryDualGetter(StrongCutTwoSidePoint) 
    PatternEntrySetter(BlackEdgeIsBorder) 
    PatternEntrySetter(WhiteEdgeIsBorder) 
    PatternEntryDualGetter(EdgeIsBorder) 
    PatternEntrySetter(BlackStrongCut) 
    PatternEntrySetter(WhiteStrongCut) 
    PatternEntryDualGetter(StrongCut) 
    PatternEntrySetter(BlackSequentialBamboo) 
    PatternEntrySetter(WhiteSequentialBamboo) 
    PatternEntryDualGetter(SequentialBamboo) 
    PatternEntrySetter(BlackStrongCutTwoSidePoint1) 
    PatternEntrySetter(WhiteStrongCutTwoSidePoint1) 
    PatternEntryDualGetter(StrongCutTwoSidePoint1) 
    PatternEntrySetter(BlackStrongCutTwoSidePoint2) 
    PatternEntrySetter(WhiteStrongCutTwoSidePoint2) 
    PatternEntryDualGetter(StrongCutTwoSidePoint2) 
    PatternEntrySetter(BlackMultipleStrongCut1) 
    PatternEntrySetter(WhiteMultipleStrongCut1) 
    PatternEntryDualGetter(MultipleStrongCut1) 
    PatternEntrySetter(BlackMultipleStrongCut2) 
    PatternEntrySetter(WhiteMultipleStrongCut2) 
    PatternEntryDualGetter(MultipleStrongCut2) 
    PatternEntrySetter(BlackStrongCutTwoSidePoint3) 
    PatternEntrySetter(WhiteStrongCutTwoSidePoint3) 
    PatternEntryDualGetter(StrongCutTwoSidePoint3) 
    PatternEntrySetter(BlackMultipleStrongCut3) 
    PatternEntrySetter(WhiteMultipleStrongCut3) 
    PatternEntryDualGetter(MultipleStrongCut3) 
    PatternEntrySetter(BlackStrongCutTwoSidePoint4) 
    PatternEntrySetter(WhiteStrongCutTwoSidePoint4) 
    PatternEntryDualGetter(StrongCutTwoSidePoint4) 
    PatternEntrySetter(BlackMultipleStrongCut4) 
    PatternEntrySetter(WhiteMultipleStrongCut4) 
    PatternEntryDualGetter(MultipleStrongCut4) 
    PatternEntrySetter(BlackStrongCutSecondMainPoint) 
    PatternEntrySetter(WhiteStrongCutSecondMainPoint) 
    PatternEntryDualGetter(StrongCutSecondMainPoint) 
    PatternEntrySetter(BlackStupidPattern) 
    PatternEntrySetter(WhiteStupidPattern) 
    PatternEntryDualGetter(StupidPattern) 
    PatternEntrySetter(BlackTerritory) 
    PatternEntrySetter(WhiteTerritory) 
    PatternEntryDualGetter(Territory) 
    PatternEntrySetter(BlackTigerMouth) 
    PatternEntrySetter(WhiteTigerMouth) 
    PatternEntryDualGetter(TigerMouth) 
    PatternEntrySetter(BlackTigerThreats) 
    PatternEntrySetter(WhiteTigerThreats) 
    PatternEntryDualGetter(TigerThreats) 
    PatternEntrySetter(BlackTofuFour) 
    PatternEntrySetter(WhiteTofuFour) 
    PatternEntryDualGetter(TofuFour) 
    PatternEntrySetter(BlackTrueEye) 
    PatternEntrySetter(WhiteTrueEye) 
    PatternEntryDualGetter(TrueEye) 
    PatternEntrySetter(BlackTwoDiagonalClosedArea) 
    PatternEntrySetter(WhiteTwoDiagonalClosedArea) 
    PatternEntryDualGetter(TwoDiagonalClosedArea) 
    PatternEntrySetter(BlackWeakCutTwoSidePoint) 
    PatternEntrySetter(WhiteWeakCutTwoSidePoint) 
    PatternEntryDualGetter(WeakCutTwoSidePoint) 
    PatternEntrySetter(BlackWeakCut) 
    PatternEntrySetter(WhiteWeakCut) 
    PatternEntryDualGetter(WeakCut) 
    PatternEntrySetter(BlackWeakCutSecondMainPoint) 
    PatternEntrySetter(WhiteWeakCutSecondMainPoint) 
    PatternEntryDualGetter(WeakCutSecondMainPoint) 
    PatternEntrySetter(BlackCornerWeakCut) 
    PatternEntrySetter(WhiteCornerWeakCut) 
    PatternEntryDualGetter(CornerWeakCut) 
};
extern Pattern33Table s_pattern_table;
class Pattern33
{
private:
    uint m_patternIndex ;
public:
    inline void setIndex(uint index) { m_patternIndex = index ; }
    inline void addOffset(uint offset) { m_patternIndex += offset ; }
    inline void subOffset(uint offset) { m_patternIndex -= offset ; }
    inline uint getIndex() const { return m_patternIndex ; }

    uint getCompleteEmpty(Color center=COLOR_NONE)const{ return s_pattern_table.getCompleteEmpty(m_patternIndex,center); }
    uint getEmptyGridCount(Color center=COLOR_NONE)const{ return s_pattern_table.getEmptyGridCount(m_patternIndex,center); }
    uint getGridCount(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getGridCount(m_patternIndex,forColor,center); }
    uint getEmptyAdjGridCount(Color center=COLOR_NONE)const{ return s_pattern_table.getEmptyAdjGridCount(m_patternIndex,center); }
    uint getEmptyDiagGridCount(Color center=COLOR_NONE)const{ return s_pattern_table.getEmptyDiagGridCount(m_patternIndex,center); }
    uint getAdjGridCount(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getAdjGridCount(m_patternIndex,forColor,center); }
    uint getDiagGridCount(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getDiagGridCount(m_patternIndex,forColor,center); }
    uint getCutArea(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getCutArea(m_patternIndex,forColor,center); }
    uint get2LibJump(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2LibJump(m_patternIndex,forColor,center); }
    uint getAnyAdjNeighbor(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getAnyAdjNeighbor(m_patternIndex,forColor,center); }
    uint getApproachLib(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getApproachLib(m_patternIndex,forColor,center); }
    uint getAtariLadder(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getAtariLadder(m_patternIndex,forColor,center); }
    uint getAtariLadderCheckPoint(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getAtariLadderCheckPoint(m_patternIndex,forColor,center); }
    uint getBamboo(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getBamboo(m_patternIndex,forColor,center); }
    uint getCloseDoor(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getCloseDoor(m_patternIndex,forColor,center); }
    uint getCloseDoorDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getCloseDoorDir(m_patternIndex,forColor,center); }
    uint getLine1CloseDoor(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getLine1CloseDoor(m_patternIndex,forColor,center); }
    uint getConnectCutPoint(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getConnectCutPoint(m_patternIndex,forColor,center); }
    uint getConnectWithOneEye(Color center=COLOR_NONE)const{ return s_pattern_table.getConnectWithOneEye(m_patternIndex,center); }
    uint getDeadOnlyOneEye3NodesSet1(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getDeadOnlyOneEye3NodesSet1(m_patternIndex,forColor,center); }
    uint getDeadOnlyOneEye3NodesSetRotation(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getDeadOnlyOneEye3NodesSetRotation(m_patternIndex,forColor,center); }
    uint getDeadOnlyOneEye4NodesSet1(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getDeadOnlyOneEye4NodesSet1(m_patternIndex,forColor,center); }
    uint getDeadOnlyOneEye4NodesSetRotation(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getDeadOnlyOneEye4NodesSetRotation(m_patternIndex,forColor,center); }
    uint getDeadOnlyOneEye5NodesSet1(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getDeadOnlyOneEye5NodesSet1(m_patternIndex,forColor,center); }
    uint getDeadOnlyOneEye5NodesSetRotation(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getDeadOnlyOneEye5NodesSetRotation(m_patternIndex,forColor,center); }
    uint getDeadOnlyOneEye6NodesSet1(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getDeadOnlyOneEye6NodesSet1(m_patternIndex,forColor,center); }
    uint getDeadOnlyOneEye6NodesSetRotation(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getDeadOnlyOneEye6NodesSetRotation(m_patternIndex,forColor,center); }
    uint getDiagonal(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getDiagonal(m_patternIndex,forColor,center); }
    uint getDiagonalClosedArea(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getDiagonalClosedArea(m_patternIndex,forColor,center); }
    uint getDiagonalProduce(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getDiagonalProduce(m_patternIndex,forColor,center); }
    uint getDoubleKnightMove(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getDoubleKnightMove(m_patternIndex,forColor,center); }
    uint get1Diagonal_RD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1Diagonal_RD(m_patternIndex,forColor,center); }
    uint get2Diagonal_LU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2Diagonal_LU(m_patternIndex,forColor,center); }
    uint get2Diagonal_LD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2Diagonal_LD(m_patternIndex,forColor,center); }
    uint get3Diagonal_RU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3Diagonal_RU(m_patternIndex,forColor,center); }
    uint get0Diagonal_RD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0Diagonal_RD(m_patternIndex,forColor,center); }
    uint get3Diagonal_LU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3Diagonal_LU(m_patternIndex,forColor,center); }
    uint get0Diagonal_RU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0Diagonal_RU(m_patternIndex,forColor,center); }
    uint get1Diagonal_LD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1Diagonal_LD(m_patternIndex,forColor,center); }
    uint get0Jump_R(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0Jump_R(m_patternIndex,forColor,center); }
    uint get2Jump_L(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2Jump_L(m_patternIndex,forColor,center); }
    uint get1Jump_D(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1Jump_D(m_patternIndex,forColor,center); }
    uint get3Jump_U(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3Jump_U(m_patternIndex,forColor,center); }
    uint get0Knight_RRU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0Knight_RRU(m_patternIndex,forColor,center); }
    uint get5Knight_LLD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get5Knight_LLD(m_patternIndex,forColor,center); }
    uint get0Knight_RRD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0Knight_RRD(m_patternIndex,forColor,center); }
    uint get6Knight_LLU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get6Knight_LLU(m_patternIndex,forColor,center); }
    uint get1Knight_RDD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1Knight_RDD(m_patternIndex,forColor,center); }
    uint get6Knight_LUU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get6Knight_LUU(m_patternIndex,forColor,center); }
    uint get1Knight_LDD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1Knight_LDD(m_patternIndex,forColor,center); }
    uint get7Knight_RUU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get7Knight_RUU(m_patternIndex,forColor,center); }
    uint get4Knight_RRD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get4Knight_RRD(m_patternIndex,forColor,center); }
    uint get2Knight_LLU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2Knight_LLU(m_patternIndex,forColor,center); }
    uint get2Knight_LLD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2Knight_LLD(m_patternIndex,forColor,center); }
    uint get7Knight_RRU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get7Knight_RRU(m_patternIndex,forColor,center); }
    uint get5Knight_LDD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get5Knight_LDD(m_patternIndex,forColor,center); }
    uint get3Knight_RUU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3Knight_RUU(m_patternIndex,forColor,center); }
    uint get4Knight_RDD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get4Knight_RDD(m_patternIndex,forColor,center); }
    uint get3Knight_LUU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3Knight_LUU(m_patternIndex,forColor,center); }
    uint get1Bamboo_RD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1Bamboo_RD(m_patternIndex,forColor,center); }
    uint get3Bamboo_RU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3Bamboo_RU(m_patternIndex,forColor,center); }
    uint get1Bamboo_LD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1Bamboo_LD(m_patternIndex,forColor,center); }
    uint get3Bamboo_LU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3Bamboo_LU(m_patternIndex,forColor,center); }
    uint get0Bamboo_UR(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0Bamboo_UR(m_patternIndex,forColor,center); }
    uint get2Bamboo_UL(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2Bamboo_UL(m_patternIndex,forColor,center); }
    uint get0Bamboo_DR(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0Bamboo_DR(m_patternIndex,forColor,center); }
    uint get2Bamboo_DL(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2Bamboo_DL(m_patternIndex,forColor,center); }
    uint get1hBamboo_SLD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1hBamboo_SLD(m_patternIndex,forColor,center); }
    uint get3hBamboo_LLU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3hBamboo_LLU(m_patternIndex,forColor,center); }
    uint get1hBamboo_SRD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1hBamboo_SRD(m_patternIndex,forColor,center); }
    uint get3hBamboo_LRU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3hBamboo_LRU(m_patternIndex,forColor,center); }
    uint get0hBamboo_LDR(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0hBamboo_LDR(m_patternIndex,forColor,center); }
    uint get2hBamboo_SDL(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2hBamboo_SDL(m_patternIndex,forColor,center); }
    uint get0hBamboo_LUR(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0hBamboo_LUR(m_patternIndex,forColor,center); }
    uint get2hBamboo_SUL(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2hBamboo_SUL(m_patternIndex,forColor,center); }
    uint get1hBamboo_LRD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1hBamboo_LRD(m_patternIndex,forColor,center); }
    uint get3hBamboo_SRU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3hBamboo_SRU(m_patternIndex,forColor,center); }
    uint get1hBamboo_LLD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1hBamboo_LLD(m_patternIndex,forColor,center); }
    uint get3hBamboo_SLU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3hBamboo_SLU(m_patternIndex,forColor,center); }
    uint get0hBamboo_SUR(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0hBamboo_SUR(m_patternIndex,forColor,center); }
    uint get2hBamboo_LUL(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2hBamboo_LUL(m_patternIndex,forColor,center); }
    uint get0hBamboo_SDR(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0hBamboo_SDR(m_patternIndex,forColor,center); }
    uint get2hBamboo_LDL(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2hBamboo_LDL(m_patternIndex,forColor,center); }
    uint get0LHalfBamboo_SUR(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0LHalfBamboo_SUR(m_patternIndex,forColor,center); }
    uint get2LHalfBamboo_LUL(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2LHalfBamboo_LUL(m_patternIndex,forColor,center); }
    uint get0LHalfBamboo_SDR(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0LHalfBamboo_SDR(m_patternIndex,forColor,center); }
    uint get2LHalfBamboo_LDL(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2LHalfBamboo_LDL(m_patternIndex,forColor,center); }
    uint get1LHalfBamboo_SLD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1LHalfBamboo_SLD(m_patternIndex,forColor,center); }
    uint get3LHalfBamboo_LLU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3LHalfBamboo_LLU(m_patternIndex,forColor,center); }
    uint get1LHalfBamboo_SRD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1LHalfBamboo_SRD(m_patternIndex,forColor,center); }
    uint get3LHalfBamboo_LRU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3LHalfBamboo_LRU(m_patternIndex,forColor,center); }
    uint get0LHalfBamboo_LUR(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0LHalfBamboo_LUR(m_patternIndex,forColor,center); }
    uint get2LHalfBamboo_SUL(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2LHalfBamboo_SUL(m_patternIndex,forColor,center); }
    uint get0LHalfBamboo_LDR(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0LHalfBamboo_LDR(m_patternIndex,forColor,center); }
    uint get2LHalfBamboo_SDL(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2LHalfBamboo_SDL(m_patternIndex,forColor,center); }
    uint get1LHalfBamboo_LRD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1LHalfBamboo_LRD(m_patternIndex,forColor,center); }
    uint get3LHalfBamboo_SRU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3LHalfBamboo_SRU(m_patternIndex,forColor,center); }
    uint get1LHalfBamboo_LLD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1LHalfBamboo_LLD(m_patternIndex,forColor,center); }
    uint get3LHalfBamboo_SLU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3LHalfBamboo_SLU(m_patternIndex,forColor,center); }
    uint get1ZHalfBamboo_RD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1ZHalfBamboo_RD(m_patternIndex,forColor,center); }
    uint get3ZHalfBamboo_LU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3ZHalfBamboo_LU(m_patternIndex,forColor,center); }
    uint get1ZHalfBamboo_LD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1ZHalfBamboo_LD(m_patternIndex,forColor,center); }
    uint get3ZHalfBamboo_RU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3ZHalfBamboo_RU(m_patternIndex,forColor,center); }
    uint get0ZHalfBamboo_UR(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0ZHalfBamboo_UR(m_patternIndex,forColor,center); }
    uint get2ZHalfBamboo_DL(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2ZHalfBamboo_DL(m_patternIndex,forColor,center); }
    uint get0ZHalfBamboo_DR(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0ZHalfBamboo_DR(m_patternIndex,forColor,center); }
    uint get2ZHalfBamboo_UL(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2ZHalfBamboo_UL(m_patternIndex,forColor,center); }
    uint get0THalfBamboo_SR(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0THalfBamboo_SR(m_patternIndex,forColor,center); }
    uint get2THalfBamboo_LL(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2THalfBamboo_LL(m_patternIndex,forColor,center); }
    uint get1THalfBamboo_SD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1THalfBamboo_SD(m_patternIndex,forColor,center); }
    uint get3THalfBamboo_LU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3THalfBamboo_LU(m_patternIndex,forColor,center); }
    uint get0THalfBamboo_LR(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0THalfBamboo_LR(m_patternIndex,forColor,center); }
    uint get2THalfBamboo_SL(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2THalfBamboo_SL(m_patternIndex,forColor,center); }
    uint get1THalfBamboo_LD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1THalfBamboo_LD(m_patternIndex,forColor,center); }
    uint get3THalfBamboo_SU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3THalfBamboo_SU(m_patternIndex,forColor,center); }
    uint get3WallJump_U(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3WallJump_U(m_patternIndex,forColor,center); }
    uint get0WallJump_R(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0WallJump_R(m_patternIndex,forColor,center); }
    uint get1WallJump_D(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1WallJump_D(m_patternIndex,forColor,center); }
    uint get2WallJump_L(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2WallJump_L(m_patternIndex,forColor,center); }
    uint get3WallBamboo_RU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3WallBamboo_RU(m_patternIndex,forColor,center); }
    uint get3WallBamboo_LU(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get3WallBamboo_LU(m_patternIndex,forColor,center); }
    uint get0WallBamboo_DR(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0WallBamboo_DR(m_patternIndex,forColor,center); }
    uint get0WallBamboo_UR(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get0WallBamboo_UR(m_patternIndex,forColor,center); }
    uint get1WallBamboo_RD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1WallBamboo_RD(m_patternIndex,forColor,center); }
    uint get1WallBamboo_LD(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get1WallBamboo_LD(m_patternIndex,forColor,center); }
    uint get2WallBamboo_DL(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2WallBamboo_DL(m_patternIndex,forColor,center); }
    uint get2WallBamboo_UL(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2WallBamboo_UL(m_patternIndex,forColor,center); }
    uint getElephant(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getElephant(m_patternIndex,forColor,center); }
    uint getElephantThreat1(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getElephantThreat1(m_patternIndex,forColor,center); }
    uint getElephantThreat2(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getElephantThreat2(m_patternIndex,forColor,center); }
    uint getElephantThreat3(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getElephantThreat3(m_patternIndex,forColor,center); }
    uint getElephantThreat4(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getElephantThreat4(m_patternIndex,forColor,center); }
    uint getEmptyPosition(Color center=COLOR_NONE)const{ return s_pattern_table.getEmptyPosition(m_patternIndex,center); }
    uint getEye(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEye(m_patternIndex,forColor,center); }
    uint getEyeConnectorTigerMouth(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEyeConnectorTigerMouth(m_patternIndex,forColor,center); }
    uint getEyeConnectorDiagonal(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEyeConnectorDiagonal(m_patternIndex,forColor,center); }
    uint getEyeConnectorFalseEye(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEyeConnectorFalseEye(m_patternIndex,forColor,center); }
    uint getEyeConnectorTigerMounthOneLibDeadBlock(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEyeConnectorTigerMounthOneLibDeadBlock(m_patternIndex,forColor,center); }
    uint getEdgeBamboo(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEdgeBamboo(m_patternIndex,forColor,center); }
    uint getEdgeBambooDirAnother(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEdgeBambooDirAnother(m_patternIndex,forColor,center); }
    uint getEdgeHalfBamboo(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEdgeHalfBamboo(m_patternIndex,forColor,center); }
    uint getEdgeHalfBambooDirAnother(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEdgeHalfBambooDirAnother(m_patternIndex,forColor,center); }
    uint getEdgeWallBamboo(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEdgeWallBamboo(m_patternIndex,forColor,center); }
    uint getEdgeWallBambooDirAnother(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEdgeWallBambooDirAnother(m_patternIndex,forColor,center); }
    uint getEdgeWallJump(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEdgeWallJump(m_patternIndex,forColor,center); }
    uint getEdgeWallJumpDirAnother(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEdgeWallJumpDirAnother(m_patternIndex,forColor,center); }
    uint getEdgeJump(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEdgeJump(m_patternIndex,forColor,center); }
    uint getEdgeJumpDirAnother(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEdgeJumpDirAnother(m_patternIndex,forColor,center); }
    uint getEdgeBan(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEdgeBan(m_patternIndex,forColor,center); }
    uint getFalseEye(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getFalseEye(m_patternIndex,forColor,center); }
    uint getSize3FalseEye(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getSize3FalseEye(m_patternIndex,forColor,center); }
    uint getSize3FalseEyeCheckDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getSize3FalseEyeCheckDir(m_patternIndex,forColor,center); }
    uint getFixTrueEye(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getFixTrueEye(m_patternIndex,forColor,center); }
    uint getFixPotentialTrueEye(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getFixPotentialTrueEye(m_patternIndex,forColor,center); }
    uint getFixFalseEye(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getFixFalseEye(m_patternIndex,forColor,center); }
    uint getGoodPatternOrder(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getGoodPatternOrder(m_patternIndex,forColor,center); }
    uint getGoodPattern(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getGoodPattern(m_patternIndex,forColor,center); }
    uint getHalfBamboo(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getHalfBamboo(m_patternIndex,forColor,center); }
    uint getHalfBambooThreatPatternDirection(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getHalfBambooThreatPatternDirection(m_patternIndex,forColor,center); }
    uint getHalfBambooThreats(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getHalfBambooThreats(m_patternIndex,forColor,center); }
    uint getJump(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getJump(m_patternIndex,forColor,center); }
    uint getKnight(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getKnight(m_patternIndex,forColor,center); }
    uint getKnightThreat1(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getKnightThreat1(m_patternIndex,forColor,center); }
    uint getKnightThreat2(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getKnightThreat2(m_patternIndex,forColor,center); }
    uint getKoRZoneCheck(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getKoRZoneCheck(m_patternIndex,forColor,center); }
    uint getIsLadder(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getIsLadder(m_patternIndex,forColor,center); }
    uint getLadderDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getLadderDir(m_patternIndex,forColor,center); }
    uint getLadderType(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getLadderType(m_patternIndex,forColor,center); }
    uint getLibTightenType(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getLibTightenType(m_patternIndex,forColor,center); }
    uint get2LibInnerLib(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2LibInnerLib(m_patternIndex,forColor,center); }
    uint getFalseEye2stone(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getFalseEye2stone(m_patternIndex,forColor,center); }
    uint getCheckProectedConnected2lib(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getCheckProectedConnected2lib(m_patternIndex,forColor,center); }
    uint getMakeTrueEye(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMakeTrueEye(m_patternIndex,forColor,center); }
    uint getMakePotentialTrueEye(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMakePotentialTrueEye(m_patternIndex,forColor,center); }
    uint getMakeFalseEye(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMakeFalseEye(m_patternIndex,forColor,center); }
    uint getMakeKo(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMakeKo(m_patternIndex,forColor,center); }
    uint getMakeKoBaseDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMakeKoBaseDir(m_patternIndex,forColor,center); }
    uint getMakeKoCheckDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMakeKoCheckDir(m_patternIndex,forColor,center); }
    uint getMoGoHane(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMoGoHane(m_patternIndex,forColor,center); }
    uint getMoGoCut1(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMoGoCut1(m_patternIndex,forColor,center); }
    uint getMoGoCut1Exclude(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMoGoCut1Exclude(m_patternIndex,forColor,center); }
    uint getMoGoCut2(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMoGoCut2(m_patternIndex,forColor,center); }
    uint getMoGoEdge(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMoGoEdge(m_patternIndex,forColor,center); }
    uint getMultipleDiagonal(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMultipleDiagonal(m_patternIndex,forColor,center); }
    uint getNakade(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getNakade(m_patternIndex,forColor,center); }
    uint getSZNakade(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getSZNakade(m_patternIndex,forColor,center); }
    uint get2LineNakade(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.get2LineNakade(m_patternIndex,forColor,center); }
    uint getOneOfAdjPos(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getOneOfAdjPos(m_patternIndex,forColor,center); }
    uint getOnlyOneLiberty(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getOnlyOneLiberty(m_patternIndex,forColor,center); }
    uint getOppEmpty(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getOppEmpty(m_patternIndex,forColor,center); }
    uint getCheckDiagonal(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getCheckDiagonal(m_patternIndex,forColor,center); }
    uint getCheckDiagonalDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getCheckDiagonalDir(m_patternIndex,forColor,center); }
    uint getTryMakeSpecialEye(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getTryMakeSpecialEye(m_patternIndex,forColor,center); }
    uint getTryMakeSpecialEyeExtendDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getTryMakeSpecialEyeExtendDir(m_patternIndex,forColor,center); }
    uint getTryMakeSpecialEyeCapDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getTryMakeSpecialEyeCapDir(m_patternIndex,forColor,center); }
    uint getTryMakeSpecialEyeEnemyDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getTryMakeSpecialEyeEnemyDir(m_patternIndex,forColor,center); }
    uint getTryMakeSpecialEyeTwoLibCheck(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getTryMakeSpecialEyeTwoLibCheck(m_patternIndex,forColor,center); }
    uint getOnelinePatternEyeTypeIExtendDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getOnelinePatternEyeTypeIExtendDir(m_patternIndex,forColor,center); }
    uint getOnelinePatternEyeTypeICapDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getOnelinePatternEyeTypeICapDir(m_patternIndex,forColor,center); }
    uint getOnelinePatternEyeTypeICheckEnemy(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getOnelinePatternEyeTypeICheckEnemy(m_patternIndex,forColor,center); }
    uint getOnelinePatternEyeTypeITwoLibCheck(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getOnelinePatternEyeTypeITwoLibCheck(m_patternIndex,forColor,center); }
    uint getMakeSplitToOnelinePatternEye(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMakeSplitToOnelinePatternEye(m_patternIndex,forColor,center); }
    uint getMakeSplitToOnelinePatternEyeExtendDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMakeSplitToOnelinePatternEyeExtendDir(m_patternIndex,forColor,center); }
    uint getMakeSplitToOnelinePatternEyeCapDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMakeSplitToOnelinePatternEyeCapDir(m_patternIndex,forColor,center); }
    uint getMakeSplitToOnelinePatternEyeEnemyDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMakeSplitToOnelinePatternEyeEnemyDir(m_patternIndex,forColor,center); }
    uint getWallJump(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getWallJump(m_patternIndex,forColor,center); }
    uint getWallJumpMiaiDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getWallJumpMiaiDir(m_patternIndex,forColor,center); }
    uint getOnelinePatternEyeTypeLExtendDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getOnelinePatternEyeTypeLExtendDir(m_patternIndex,forColor,center); }
    uint getOnelinePatternEyeTypeLBottomDir(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getOnelinePatternEyeTypeLBottomDir(m_patternIndex,forColor,center); }
    uint getOneLineCaptureDiagonal(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getOneLineCaptureDiagonal(m_patternIndex,forColor,center); }
    uint getOneLineOneStoneCaptureDiagonal(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getOneLineOneStoneCaptureDiagonal(m_patternIndex,forColor,center); }
    uint getOneLineOneStoneNextCaptureDiagonal(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getOneLineOneStoneNextCaptureDiagonal(m_patternIndex,forColor,center); }
    uint getPerfectClosedArea(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getPerfectClosedArea(m_patternIndex,forColor,center); }
    uint getPotentialTrueEye(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getPotentialTrueEye(m_patternIndex,forColor,center); }
    uint getPushCenter3Nodes(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getPushCenter3Nodes(m_patternIndex,forColor,center); }
    uint getPushCenter4Nodes(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getPushCenter4Nodes(m_patternIndex,forColor,center); }
    uint getPushCenter5Nodes(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getPushCenter5Nodes(m_patternIndex,forColor,center); }
    uint getPushCenter6Nodes(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getPushCenter6Nodes(m_patternIndex,forColor,center); }
    uint getReduceEye(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getReduceEye(m_patternIndex,forColor,center); }
    uint getSplit(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getSplit(m_patternIndex,forColor,center); }
    uint getStartCAPosition(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getStartCAPosition(m_patternIndex,forColor,center); }
    uint getIsSplitCA(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getIsSplitCA(m_patternIndex,forColor,center); }
    uint getEdge(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEdge(m_patternIndex,forColor,center); }
    uint getStrongCutTwoSidePoint(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getStrongCutTwoSidePoint(m_patternIndex,forColor,center); }
    uint getEdgeIsBorder(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getEdgeIsBorder(m_patternIndex,forColor,center); }
    uint getStrongCut(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getStrongCut(m_patternIndex,forColor,center); }
    uint getSequentialBamboo(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getSequentialBamboo(m_patternIndex,forColor,center); }
    uint getStrongCutTwoSidePoint1(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getStrongCutTwoSidePoint1(m_patternIndex,forColor,center); }
    uint getStrongCutTwoSidePoint2(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getStrongCutTwoSidePoint2(m_patternIndex,forColor,center); }
    uint getMultipleStrongCut1(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMultipleStrongCut1(m_patternIndex,forColor,center); }
    uint getMultipleStrongCut2(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMultipleStrongCut2(m_patternIndex,forColor,center); }
    uint getStrongCutTwoSidePoint3(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getStrongCutTwoSidePoint3(m_patternIndex,forColor,center); }
    uint getMultipleStrongCut3(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMultipleStrongCut3(m_patternIndex,forColor,center); }
    uint getStrongCutTwoSidePoint4(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getStrongCutTwoSidePoint4(m_patternIndex,forColor,center); }
    uint getMultipleStrongCut4(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getMultipleStrongCut4(m_patternIndex,forColor,center); }
    uint getStrongCutSecondMainPoint(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getStrongCutSecondMainPoint(m_patternIndex,forColor,center); }
    uint getStupidPattern(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getStupidPattern(m_patternIndex,forColor,center); }
    uint getTerritory(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getTerritory(m_patternIndex,forColor,center); }
    uint getTigerMouth(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getTigerMouth(m_patternIndex,forColor,center); }
    uint getTigerThreats(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getTigerThreats(m_patternIndex,forColor,center); }
    uint getTofuFour(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getTofuFour(m_patternIndex,forColor,center); }
    uint getTrueEye(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getTrueEye(m_patternIndex,forColor,center); }
    uint getTwoDiagonalClosedArea(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getTwoDiagonalClosedArea(m_patternIndex,forColor,center); }
    uint getWeakCutTwoSidePoint(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getWeakCutTwoSidePoint(m_patternIndex,forColor,center); }
    uint getWeakCut(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getWeakCut(m_patternIndex,forColor,center); }
    uint getWeakCutSecondMainPoint(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getWeakCutSecondMainPoint(m_patternIndex,forColor,center); }
    uint getCornerWeakCut(Color forColor,Color center=COLOR_NONE)const{ return s_pattern_table.getCornerWeakCut(m_patternIndex,forColor,center); }

	void showAllAttribute(Color forColor,Color center=COLOR_NONE) const {
        cerr << "CompleteEmpty: " <<  getCompleteEmpty(center) << endl ;
        cerr << "EmptyGridCount: " <<  getEmptyGridCount(center) << endl ;
        cerr << "GridCount: " <<  getGridCount(forColor, center) << endl ;
        cerr << "EmptyAdjGridCount: " <<  getEmptyAdjGridCount(center) << endl ;
        cerr << "EmptyDiagGridCount: " <<  getEmptyDiagGridCount(center) << endl ;
        cerr << "AdjGridCount: " <<  getAdjGridCount(forColor, center) << endl ;
        cerr << "DiagGridCount: " <<  getDiagGridCount(forColor, center) << endl ;
        cerr << "CutArea: " <<  getCutArea(forColor, center) << endl ;
        cerr << "2LibJump: " <<  get2LibJump(forColor, center) << endl ;
        cerr << "AnyAdjNeighbor: " <<  getAnyAdjNeighbor(forColor, center) << endl ;
        cerr << "ApproachLib: " <<  getApproachLib(forColor, center) << endl ;
        cerr << "AtariLadder: " <<  getAtariLadder(forColor, center) << endl ;
        cerr << "AtariLadderCheckPoint: " <<  getAtariLadderCheckPoint(forColor, center) << endl ;
        cerr << "Bamboo: " <<  getBamboo(forColor, center) << endl ;
        cerr << "CloseDoor: " <<  getCloseDoor(forColor, center) << endl ;
        cerr << "CloseDoorDir: " <<  getCloseDoorDir(forColor, center) << endl ;
        cerr << "Line1CloseDoor: " <<  getLine1CloseDoor(forColor, center) << endl ;
        cerr << "ConnectCutPoint: " <<  getConnectCutPoint(forColor, center) << endl ;
        cerr << "ConnectWithOneEye: " <<  getConnectWithOneEye(center) << endl ;
        cerr << "DeadOnlyOneEye3NodesSet1: " <<  getDeadOnlyOneEye3NodesSet1(forColor, center) << endl ;
        cerr << "DeadOnlyOneEye3NodesSetRotation: " <<  getDeadOnlyOneEye3NodesSetRotation(forColor, center) << endl ;
        cerr << "DeadOnlyOneEye4NodesSet1: " <<  getDeadOnlyOneEye4NodesSet1(forColor, center) << endl ;
        cerr << "DeadOnlyOneEye4NodesSetRotation: " <<  getDeadOnlyOneEye4NodesSetRotation(forColor, center) << endl ;
        cerr << "DeadOnlyOneEye5NodesSet1: " <<  getDeadOnlyOneEye5NodesSet1(forColor, center) << endl ;
        cerr << "DeadOnlyOneEye5NodesSetRotation: " <<  getDeadOnlyOneEye5NodesSetRotation(forColor, center) << endl ;
        cerr << "DeadOnlyOneEye6NodesSet1: " <<  getDeadOnlyOneEye6NodesSet1(forColor, center) << endl ;
        cerr << "DeadOnlyOneEye6NodesSetRotation: " <<  getDeadOnlyOneEye6NodesSetRotation(forColor, center) << endl ;
        cerr << "Diagonal: " <<  getDiagonal(forColor, center) << endl ;
        cerr << "DiagonalClosedArea: " <<  getDiagonalClosedArea(forColor, center) << endl ;
        cerr << "DiagonalProduce: " <<  getDiagonalProduce(forColor, center) << endl ;
        cerr << "DoubleKnightMove: " <<  getDoubleKnightMove(forColor, center) << endl ;
        cerr << "1Diagonal_RD: " <<  get1Diagonal_RD(forColor, center) << endl ;
        cerr << "2Diagonal_LU: " <<  get2Diagonal_LU(forColor, center) << endl ;
        cerr << "2Diagonal_LD: " <<  get2Diagonal_LD(forColor, center) << endl ;
        cerr << "3Diagonal_RU: " <<  get3Diagonal_RU(forColor, center) << endl ;
        cerr << "0Diagonal_RD: " <<  get0Diagonal_RD(forColor, center) << endl ;
        cerr << "3Diagonal_LU: " <<  get3Diagonal_LU(forColor, center) << endl ;
        cerr << "0Diagonal_RU: " <<  get0Diagonal_RU(forColor, center) << endl ;
        cerr << "1Diagonal_LD: " <<  get1Diagonal_LD(forColor, center) << endl ;
        cerr << "0Jump_R: " <<  get0Jump_R(forColor, center) << endl ;
        cerr << "2Jump_L: " <<  get2Jump_L(forColor, center) << endl ;
        cerr << "1Jump_D: " <<  get1Jump_D(forColor, center) << endl ;
        cerr << "3Jump_U: " <<  get3Jump_U(forColor, center) << endl ;
        cerr << "0Knight_RRU: " <<  get0Knight_RRU(forColor, center) << endl ;
        cerr << "5Knight_LLD: " <<  get5Knight_LLD(forColor, center) << endl ;
        cerr << "0Knight_RRD: " <<  get0Knight_RRD(forColor, center) << endl ;
        cerr << "6Knight_LLU: " <<  get6Knight_LLU(forColor, center) << endl ;
        cerr << "1Knight_RDD: " <<  get1Knight_RDD(forColor, center) << endl ;
        cerr << "6Knight_LUU: " <<  get6Knight_LUU(forColor, center) << endl ;
        cerr << "1Knight_LDD: " <<  get1Knight_LDD(forColor, center) << endl ;
        cerr << "7Knight_RUU: " <<  get7Knight_RUU(forColor, center) << endl ;
        cerr << "4Knight_RRD: " <<  get4Knight_RRD(forColor, center) << endl ;
        cerr << "2Knight_LLU: " <<  get2Knight_LLU(forColor, center) << endl ;
        cerr << "2Knight_LLD: " <<  get2Knight_LLD(forColor, center) << endl ;
        cerr << "7Knight_RRU: " <<  get7Knight_RRU(forColor, center) << endl ;
        cerr << "5Knight_LDD: " <<  get5Knight_LDD(forColor, center) << endl ;
        cerr << "3Knight_RUU: " <<  get3Knight_RUU(forColor, center) << endl ;
        cerr << "4Knight_RDD: " <<  get4Knight_RDD(forColor, center) << endl ;
        cerr << "3Knight_LUU: " <<  get3Knight_LUU(forColor, center) << endl ;
        cerr << "1Bamboo_RD: " <<  get1Bamboo_RD(forColor, center) << endl ;
        cerr << "3Bamboo_RU: " <<  get3Bamboo_RU(forColor, center) << endl ;
        cerr << "1Bamboo_LD: " <<  get1Bamboo_LD(forColor, center) << endl ;
        cerr << "3Bamboo_LU: " <<  get3Bamboo_LU(forColor, center) << endl ;
        cerr << "0Bamboo_UR: " <<  get0Bamboo_UR(forColor, center) << endl ;
        cerr << "2Bamboo_UL: " <<  get2Bamboo_UL(forColor, center) << endl ;
        cerr << "0Bamboo_DR: " <<  get0Bamboo_DR(forColor, center) << endl ;
        cerr << "2Bamboo_DL: " <<  get2Bamboo_DL(forColor, center) << endl ;
        cerr << "1hBamboo_SLD: " <<  get1hBamboo_SLD(forColor, center) << endl ;
        cerr << "3hBamboo_LLU: " <<  get3hBamboo_LLU(forColor, center) << endl ;
        cerr << "1hBamboo_SRD: " <<  get1hBamboo_SRD(forColor, center) << endl ;
        cerr << "3hBamboo_LRU: " <<  get3hBamboo_LRU(forColor, center) << endl ;
        cerr << "0hBamboo_LDR: " <<  get0hBamboo_LDR(forColor, center) << endl ;
        cerr << "2hBamboo_SDL: " <<  get2hBamboo_SDL(forColor, center) << endl ;
        cerr << "0hBamboo_LUR: " <<  get0hBamboo_LUR(forColor, center) << endl ;
        cerr << "2hBamboo_SUL: " <<  get2hBamboo_SUL(forColor, center) << endl ;
        cerr << "1hBamboo_LRD: " <<  get1hBamboo_LRD(forColor, center) << endl ;
        cerr << "3hBamboo_SRU: " <<  get3hBamboo_SRU(forColor, center) << endl ;
        cerr << "1hBamboo_LLD: " <<  get1hBamboo_LLD(forColor, center) << endl ;
        cerr << "3hBamboo_SLU: " <<  get3hBamboo_SLU(forColor, center) << endl ;
        cerr << "0hBamboo_SUR: " <<  get0hBamboo_SUR(forColor, center) << endl ;
        cerr << "2hBamboo_LUL: " <<  get2hBamboo_LUL(forColor, center) << endl ;
        cerr << "0hBamboo_SDR: " <<  get0hBamboo_SDR(forColor, center) << endl ;
        cerr << "2hBamboo_LDL: " <<  get2hBamboo_LDL(forColor, center) << endl ;
        cerr << "0LHalfBamboo_SUR: " <<  get0LHalfBamboo_SUR(forColor, center) << endl ;
        cerr << "2LHalfBamboo_LUL: " <<  get2LHalfBamboo_LUL(forColor, center) << endl ;
        cerr << "0LHalfBamboo_SDR: " <<  get0LHalfBamboo_SDR(forColor, center) << endl ;
        cerr << "2LHalfBamboo_LDL: " <<  get2LHalfBamboo_LDL(forColor, center) << endl ;
        cerr << "1LHalfBamboo_SLD: " <<  get1LHalfBamboo_SLD(forColor, center) << endl ;
        cerr << "3LHalfBamboo_LLU: " <<  get3LHalfBamboo_LLU(forColor, center) << endl ;
        cerr << "1LHalfBamboo_SRD: " <<  get1LHalfBamboo_SRD(forColor, center) << endl ;
        cerr << "3LHalfBamboo_LRU: " <<  get3LHalfBamboo_LRU(forColor, center) << endl ;
        cerr << "0LHalfBamboo_LUR: " <<  get0LHalfBamboo_LUR(forColor, center) << endl ;
        cerr << "2LHalfBamboo_SUL: " <<  get2LHalfBamboo_SUL(forColor, center) << endl ;
        cerr << "0LHalfBamboo_LDR: " <<  get0LHalfBamboo_LDR(forColor, center) << endl ;
        cerr << "2LHalfBamboo_SDL: " <<  get2LHalfBamboo_SDL(forColor, center) << endl ;
        cerr << "1LHalfBamboo_LRD: " <<  get1LHalfBamboo_LRD(forColor, center) << endl ;
        cerr << "3LHalfBamboo_SRU: " <<  get3LHalfBamboo_SRU(forColor, center) << endl ;
        cerr << "1LHalfBamboo_LLD: " <<  get1LHalfBamboo_LLD(forColor, center) << endl ;
        cerr << "3LHalfBamboo_SLU: " <<  get3LHalfBamboo_SLU(forColor, center) << endl ;
        cerr << "1ZHalfBamboo_RD: " <<  get1ZHalfBamboo_RD(forColor, center) << endl ;
        cerr << "3ZHalfBamboo_LU: " <<  get3ZHalfBamboo_LU(forColor, center) << endl ;
        cerr << "1ZHalfBamboo_LD: " <<  get1ZHalfBamboo_LD(forColor, center) << endl ;
        cerr << "3ZHalfBamboo_RU: " <<  get3ZHalfBamboo_RU(forColor, center) << endl ;
        cerr << "0ZHalfBamboo_UR: " <<  get0ZHalfBamboo_UR(forColor, center) << endl ;
        cerr << "2ZHalfBamboo_DL: " <<  get2ZHalfBamboo_DL(forColor, center) << endl ;
        cerr << "0ZHalfBamboo_DR: " <<  get0ZHalfBamboo_DR(forColor, center) << endl ;
        cerr << "2ZHalfBamboo_UL: " <<  get2ZHalfBamboo_UL(forColor, center) << endl ;
        cerr << "0THalfBamboo_SR: " <<  get0THalfBamboo_SR(forColor, center) << endl ;
        cerr << "2THalfBamboo_LL: " <<  get2THalfBamboo_LL(forColor, center) << endl ;
        cerr << "1THalfBamboo_SD: " <<  get1THalfBamboo_SD(forColor, center) << endl ;
        cerr << "3THalfBamboo_LU: " <<  get3THalfBamboo_LU(forColor, center) << endl ;
        cerr << "0THalfBamboo_LR: " <<  get0THalfBamboo_LR(forColor, center) << endl ;
        cerr << "2THalfBamboo_SL: " <<  get2THalfBamboo_SL(forColor, center) << endl ;
        cerr << "1THalfBamboo_LD: " <<  get1THalfBamboo_LD(forColor, center) << endl ;
        cerr << "3THalfBamboo_SU: " <<  get3THalfBamboo_SU(forColor, center) << endl ;
        cerr << "3WallJump_U: " <<  get3WallJump_U(forColor, center) << endl ;
        cerr << "0WallJump_R: " <<  get0WallJump_R(forColor, center) << endl ;
        cerr << "1WallJump_D: " <<  get1WallJump_D(forColor, center) << endl ;
        cerr << "2WallJump_L: " <<  get2WallJump_L(forColor, center) << endl ;
        cerr << "3WallBamboo_RU: " <<  get3WallBamboo_RU(forColor, center) << endl ;
        cerr << "3WallBamboo_LU: " <<  get3WallBamboo_LU(forColor, center) << endl ;
        cerr << "0WallBamboo_DR: " <<  get0WallBamboo_DR(forColor, center) << endl ;
        cerr << "0WallBamboo_UR: " <<  get0WallBamboo_UR(forColor, center) << endl ;
        cerr << "1WallBamboo_RD: " <<  get1WallBamboo_RD(forColor, center) << endl ;
        cerr << "1WallBamboo_LD: " <<  get1WallBamboo_LD(forColor, center) << endl ;
        cerr << "2WallBamboo_DL: " <<  get2WallBamboo_DL(forColor, center) << endl ;
        cerr << "2WallBamboo_UL: " <<  get2WallBamboo_UL(forColor, center) << endl ;
        cerr << "Elephant: " <<  getElephant(forColor, center) << endl ;
        cerr << "ElephantThreat1: " <<  getElephantThreat1(forColor, center) << endl ;
        cerr << "ElephantThreat2: " <<  getElephantThreat2(forColor, center) << endl ;
        cerr << "ElephantThreat3: " <<  getElephantThreat3(forColor, center) << endl ;
        cerr << "ElephantThreat4: " <<  getElephantThreat4(forColor, center) << endl ;
        cerr << "EmptyPosition: " <<  getEmptyPosition(center) << endl ;
        cerr << "Eye: " <<  getEye(forColor, center) << endl ;
        cerr << "EyeConnectorTigerMouth: " <<  getEyeConnectorTigerMouth(forColor, center) << endl ;
        cerr << "EyeConnectorDiagonal: " <<  getEyeConnectorDiagonal(forColor, center) << endl ;
        cerr << "EyeConnectorFalseEye: " <<  getEyeConnectorFalseEye(forColor, center) << endl ;
        cerr << "EyeConnectorTigerMounthOneLibDeadBlock: " <<  getEyeConnectorTigerMounthOneLibDeadBlock(forColor, center) << endl ;
        cerr << "EdgeBamboo: " <<  getEdgeBamboo(forColor, center) << endl ;
        cerr << "EdgeBambooDirAnother: " <<  getEdgeBambooDirAnother(forColor, center) << endl ;
        cerr << "EdgeHalfBamboo: " <<  getEdgeHalfBamboo(forColor, center) << endl ;
        cerr << "EdgeHalfBambooDirAnother: " <<  getEdgeHalfBambooDirAnother(forColor, center) << endl ;
        cerr << "EdgeWallBamboo: " <<  getEdgeWallBamboo(forColor, center) << endl ;
        cerr << "EdgeWallBambooDirAnother: " <<  getEdgeWallBambooDirAnother(forColor, center) << endl ;
        cerr << "EdgeWallJump: " <<  getEdgeWallJump(forColor, center) << endl ;
        cerr << "EdgeWallJumpDirAnother: " <<  getEdgeWallJumpDirAnother(forColor, center) << endl ;
        cerr << "EdgeJump: " <<  getEdgeJump(forColor, center) << endl ;
        cerr << "EdgeJumpDirAnother: " <<  getEdgeJumpDirAnother(forColor, center) << endl ;
        cerr << "EdgeBan: " <<  getEdgeBan(forColor, center) << endl ;
        cerr << "FalseEye: " <<  getFalseEye(forColor, center) << endl ;
        cerr << "Size3FalseEye: " <<  getSize3FalseEye(forColor, center) << endl ;
        cerr << "Size3FalseEyeCheckDir: " <<  getSize3FalseEyeCheckDir(forColor, center) << endl ;
        cerr << "FixTrueEye: " <<  getFixTrueEye(forColor, center) << endl ;
        cerr << "FixPotentialTrueEye: " <<  getFixPotentialTrueEye(forColor, center) << endl ;
        cerr << "FixFalseEye: " <<  getFixFalseEye(forColor, center) << endl ;
        cerr << "GoodPatternOrder: " <<  getGoodPatternOrder(forColor, center) << endl ;
        cerr << "GoodPattern: " <<  getGoodPattern(forColor, center) << endl ;
        cerr << "HalfBamboo: " <<  getHalfBamboo(forColor, center) << endl ;
        cerr << "HalfBambooThreatPatternDirection: " <<  getHalfBambooThreatPatternDirection(forColor, center) << endl ;
        cerr << "HalfBambooThreats: " <<  getHalfBambooThreats(forColor, center) << endl ;
        cerr << "Jump: " <<  getJump(forColor, center) << endl ;
        cerr << "Knight: " <<  getKnight(forColor, center) << endl ;
        cerr << "KnightThreat1: " <<  getKnightThreat1(forColor, center) << endl ;
        cerr << "KnightThreat2: " <<  getKnightThreat2(forColor, center) << endl ;
        cerr << "KoRZoneCheck: " <<  getKoRZoneCheck(forColor, center) << endl ;
        cerr << "IsLadder: " <<  getIsLadder(forColor, center) << endl ;
        cerr << "LadderDir: " <<  getLadderDir(forColor, center) << endl ;
        cerr << "LadderType: " <<  getLadderType(forColor, center) << endl ;
        cerr << "LibTightenType: " <<  getLibTightenType(forColor, center) << endl ;
        cerr << "2LibInnerLib: " <<  get2LibInnerLib(forColor, center) << endl ;
        cerr << "FalseEye2stone: " <<  getFalseEye2stone(forColor, center) << endl ;
        cerr << "CheckProectedConnected2lib: " <<  getCheckProectedConnected2lib(forColor, center) << endl ;
        cerr << "MakeTrueEye: " <<  getMakeTrueEye(forColor, center) << endl ;
        cerr << "MakePotentialTrueEye: " <<  getMakePotentialTrueEye(forColor, center) << endl ;
        cerr << "MakeFalseEye: " <<  getMakeFalseEye(forColor, center) << endl ;
        cerr << "MakeKo: " <<  getMakeKo(forColor, center) << endl ;
        cerr << "MakeKoBaseDir: " <<  getMakeKoBaseDir(forColor, center) << endl ;
        cerr << "MakeKoCheckDir: " <<  getMakeKoCheckDir(forColor, center) << endl ;
        cerr << "MoGoHane: " <<  getMoGoHane(forColor, center) << endl ;
        cerr << "MoGoCut1: " <<  getMoGoCut1(forColor, center) << endl ;
        cerr << "MoGoCut1Exclude: " <<  getMoGoCut1Exclude(forColor, center) << endl ;
        cerr << "MoGoCut2: " <<  getMoGoCut2(forColor, center) << endl ;
        cerr << "MoGoEdge: " <<  getMoGoEdge(forColor, center) << endl ;
        cerr << "MultipleDiagonal: " <<  getMultipleDiagonal(forColor, center) << endl ;
        cerr << "Nakade: " <<  getNakade(forColor, center) << endl ;
        cerr << "SZNakade: " <<  getSZNakade(forColor, center) << endl ;
        cerr << "2LineNakade: " <<  get2LineNakade(forColor, center) << endl ;
        cerr << "OneOfAdjPos: " <<  getOneOfAdjPos(forColor, center) << endl ;
        cerr << "OnlyOneLiberty: " <<  getOnlyOneLiberty(forColor, center) << endl ;
        cerr << "OppEmpty: " <<  getOppEmpty(forColor, center) << endl ;
        cerr << "CheckDiagonal: " <<  getCheckDiagonal(forColor, center) << endl ;
        cerr << "CheckDiagonalDir: " <<  getCheckDiagonalDir(forColor, center) << endl ;
        cerr << "TryMakeSpecialEye: " <<  getTryMakeSpecialEye(forColor, center) << endl ;
        cerr << "TryMakeSpecialEyeExtendDir: " <<  getTryMakeSpecialEyeExtendDir(forColor, center) << endl ;
        cerr << "TryMakeSpecialEyeCapDir: " <<  getTryMakeSpecialEyeCapDir(forColor, center) << endl ;
        cerr << "TryMakeSpecialEyeEnemyDir: " <<  getTryMakeSpecialEyeEnemyDir(forColor, center) << endl ;
        cerr << "TryMakeSpecialEyeTwoLibCheck: " <<  getTryMakeSpecialEyeTwoLibCheck(forColor, center) << endl ;
        cerr << "OnelinePatternEyeTypeIExtendDir: " <<  getOnelinePatternEyeTypeIExtendDir(forColor, center) << endl ;
        cerr << "OnelinePatternEyeTypeICapDir: " <<  getOnelinePatternEyeTypeICapDir(forColor, center) << endl ;
        cerr << "OnelinePatternEyeTypeICheckEnemy: " <<  getOnelinePatternEyeTypeICheckEnemy(forColor, center) << endl ;
        cerr << "OnelinePatternEyeTypeITwoLibCheck: " <<  getOnelinePatternEyeTypeITwoLibCheck(forColor, center) << endl ;
        cerr << "MakeSplitToOnelinePatternEye: " <<  getMakeSplitToOnelinePatternEye(forColor, center) << endl ;
        cerr << "MakeSplitToOnelinePatternEyeExtendDir: " <<  getMakeSplitToOnelinePatternEyeExtendDir(forColor, center) << endl ;
        cerr << "MakeSplitToOnelinePatternEyeCapDir: " <<  getMakeSplitToOnelinePatternEyeCapDir(forColor, center) << endl ;
        cerr << "MakeSplitToOnelinePatternEyeEnemyDir: " <<  getMakeSplitToOnelinePatternEyeEnemyDir(forColor, center) << endl ;
        cerr << "WallJump: " <<  getWallJump(forColor, center) << endl ;
        cerr << "WallJumpMiaiDir: " <<  getWallJumpMiaiDir(forColor, center) << endl ;
        cerr << "OnelinePatternEyeTypeLExtendDir: " <<  getOnelinePatternEyeTypeLExtendDir(forColor, center) << endl ;
        cerr << "OnelinePatternEyeTypeLBottomDir: " <<  getOnelinePatternEyeTypeLBottomDir(forColor, center) << endl ;
        cerr << "OneLineCaptureDiagonal: " <<  getOneLineCaptureDiagonal(forColor, center) << endl ;
        cerr << "OneLineOneStoneCaptureDiagonal: " <<  getOneLineOneStoneCaptureDiagonal(forColor, center) << endl ;
        cerr << "OneLineOneStoneNextCaptureDiagonal: " <<  getOneLineOneStoneNextCaptureDiagonal(forColor, center) << endl ;
        cerr << "PerfectClosedArea: " <<  getPerfectClosedArea(forColor, center) << endl ;
        cerr << "PotentialTrueEye: " <<  getPotentialTrueEye(forColor, center) << endl ;
        cerr << "PushCenter3Nodes: " <<  getPushCenter3Nodes(forColor, center) << endl ;
        cerr << "PushCenter4Nodes: " <<  getPushCenter4Nodes(forColor, center) << endl ;
        cerr << "PushCenter5Nodes: " <<  getPushCenter5Nodes(forColor, center) << endl ;
        cerr << "PushCenter6Nodes: " <<  getPushCenter6Nodes(forColor, center) << endl ;
        cerr << "ReduceEye: " <<  getReduceEye(forColor, center) << endl ;
        cerr << "Split: " <<  getSplit(forColor, center) << endl ;
        cerr << "StartCAPosition: " <<  getStartCAPosition(forColor, center) << endl ;
        cerr << "IsSplitCA: " <<  getIsSplitCA(forColor, center) << endl ;
        cerr << "Edge: " <<  getEdge(forColor, center) << endl ;
        cerr << "StrongCutTwoSidePoint: " <<  getStrongCutTwoSidePoint(forColor, center) << endl ;
        cerr << "EdgeIsBorder: " <<  getEdgeIsBorder(forColor, center) << endl ;
        cerr << "StrongCut: " <<  getStrongCut(forColor, center) << endl ;
        cerr << "SequentialBamboo: " <<  getSequentialBamboo(forColor, center) << endl ;
        cerr << "StrongCutTwoSidePoint1: " <<  getStrongCutTwoSidePoint1(forColor, center) << endl ;
        cerr << "StrongCutTwoSidePoint2: " <<  getStrongCutTwoSidePoint2(forColor, center) << endl ;
        cerr << "MultipleStrongCut1: " <<  getMultipleStrongCut1(forColor, center) << endl ;
        cerr << "MultipleStrongCut2: " <<  getMultipleStrongCut2(forColor, center) << endl ;
        cerr << "StrongCutTwoSidePoint3: " <<  getStrongCutTwoSidePoint3(forColor, center) << endl ;
        cerr << "MultipleStrongCut3: " <<  getMultipleStrongCut3(forColor, center) << endl ;
        cerr << "StrongCutTwoSidePoint4: " <<  getStrongCutTwoSidePoint4(forColor, center) << endl ;
        cerr << "MultipleStrongCut4: " <<  getMultipleStrongCut4(forColor, center) << endl ;
        cerr << "StrongCutSecondMainPoint: " <<  getStrongCutSecondMainPoint(forColor, center) << endl ;
        cerr << "StupidPattern: " <<  getStupidPattern(forColor, center) << endl ;
        cerr << "Territory: " <<  getTerritory(forColor, center) << endl ;
        cerr << "TigerMouth: " <<  getTigerMouth(forColor, center) << endl ;
        cerr << "TigerThreats: " <<  getTigerThreats(forColor, center) << endl ;
        cerr << "TofuFour: " <<  getTofuFour(forColor, center) << endl ;
        cerr << "TrueEye: " <<  getTrueEye(forColor, center) << endl ;
        cerr << "TwoDiagonalClosedArea: " <<  getTwoDiagonalClosedArea(forColor, center) << endl ;
        cerr << "WeakCutTwoSidePoint: " <<  getWeakCutTwoSidePoint(forColor, center) << endl ;
        cerr << "WeakCut: " <<  getWeakCut(forColor, center) << endl ;
        cerr << "WeakCutSecondMainPoint: " <<  getWeakCutSecondMainPoint(forColor, center) << endl ;
        cerr << "CornerWeakCut: " <<  getCornerWeakCut(forColor, center) << endl ;
    }
};
}

class PatternActionTable {
	public:
	static ull m_Pattern_Action[8][128];
	static ull m_Pattern_Remove[8];
};

#endif 
