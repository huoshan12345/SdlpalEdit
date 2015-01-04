#ifndef ENEMY_H
#define ENEMY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "common.h"

VOID
PAL_New_DecreaseHPForEnemy(
WORD		wEnemyIndex,
INT			iDamage
);

INT PAL_New_GetPoisonIndexForEnemy(
WORD		wEnemyIndex,
WORD		wPoisonID
);

WORD 
PAL_New_GetEnemySorceryResistance(
WORD		wEnemyIndex
);

WORD
PAL_New_GetEnemyPoisonResistance(
WORD		wEnemyIndex
);

WORD
PAL_New_GetEnemyPhysicalResistance(
WORD		wEnemyIndex
);

WORD
PAL_New_GetEnemyElementalResistance(
WORD		wEnemyIndex, 
INT			iAttrib
);

VOID
PAL_New_SetEnemyStatus(
WORD		wEnemyIndex,
WORD		wStatusID,
WORD		wNumRound
);

VOID 
PAL_New_SortPoisonsForEnemyByLevel(
WORD		wEnemyIndex
);

VOID 
PAL_New_AddPoisonForEnemy(
WORD		wEnemyIndex, 
WORD		wPoisonID
);

VOID
PAL_New_CurePoisonForEnemyByKind(
WORD		wEnemyIndex,
WORD		wPoisonID
);

VOID
PAL_New_CurePoisonForEnemyByLevel(
WORD		wEnemyIndex,
WORD		wMaxLevel
);

WORD
PAL_New_GetEnemyAttackStrength(
WORD		wEnemyIndex
);

WORD
PAL_New_GetEnemyMagicStrength(
WORD		wEnemyIndex
);

WORD
PAL_New_GetEnemyDefense(
WORD		wEnemyIndex
);

WORD
PAL_New_GetEnemyDexterity(
WORD		wEnemyIndex
);

WORD
PAL_New_GetEnemyFleeRate(
WORD		wEnemyIndex
);

DWORD
PAL_New_EstimateEnemyHealthByLevel(
WORD		wEnemyLevel
);

#ifdef __cplusplus
}
#endif

#endif