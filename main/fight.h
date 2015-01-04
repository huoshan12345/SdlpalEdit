//
// Copyright (c) 2009, Wei Mingzhi <whistler_wmz@users.sf.net>.
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef FIGHT_H
#define FIGHT_H

#ifdef __cplusplus
extern "C"
{
#endif

BOOL
PAL_IsPlayerDying(
WORD		wPlayerRole
);

INT
PAL_BattleSelectAutoTarget(
VOID
);

#ifndef PAL_CLASSIC

VOID
PAL_UpdateTimeChargingUnit(
VOID
);

FLOAT
PAL_GetTimeChargingSpeed(
WORD           wDexterity
);

#endif

VOID
PAL_BattleUpdateFighters(
VOID
);

VOID
PAL_BattlePlayerCheckReady(
VOID
);

VOID
PAL_BattleStartFrame(
VOID
);

VOID
PAL_BattleCommitAction(
BOOL         fRepeat
);

VOID PAL_BattlePlayerPerformAction(WORD wPlayerIndex);

VOID PAL_BattleEnemyPerformAction(WORD wEnemyIndex);

#ifdef PAL_WIN95
VOID
PAL_BattleShowPlayerPreMagicAnim(
WORD         wPlayerIndex,
WORD         wObjectID
);
#else
VOID
PAL_BattleShowPlayerPreMagicAnim(
WORD         wPlayerIndex,
BOOL         fSummon
);
#endif

VOID
PAL_BattleDelay(
WORD       wDuration,
WORD       wObjectID,
BOOL       fUpdateGesture
);

VOID
PAL_BattleStealFromEnemy(
WORD           wTarget,
WORD           wStealRate
);

VOID
PAL_BattleSimulateMagic(
SHORT		sTarget,
WORD		wMagicObjectID,
WORD		wBaseDamage,
BOOL		fIncludeMagicDamage
);

VOID PAL_New_ApplyMagicDamageToEnemy(
SHORT		sTarget,
WORD		wBaseDamage,
WORD		wMagicObjectID,
BOOL		fOneDamageAtLeast
);

INT PAL_New_GetAlivePlayerNum(VOID);

INT PAL_New_GetHealthyPlayerNum(VOID);

INT PAL_New_GetAliveEnemyNum(VOID);

BOOL PAL_BattleDisplayStatChange(VOID);

VOID PAL_BattleBackupStat(VOID);

BOOL PAL_New_IfEnemyCanMove(WORD wEnemyIndex);

BOOL PAL_New_IfPlayerCanMove(WORD wPlayerRole);

#ifdef __cplusplus
}
#endif
#endif
