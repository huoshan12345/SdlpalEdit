/* -*- mode: c; tab-width: 4; c-basic-offset: 3; c-file-style: "linux" -*- */
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

#include "main.h"
#include <math.h>

BOOL
PAL_IsPlayerDying(
	WORD		wPlayerRole
	)
	/*++
	  功能：    Check if the player is dying.
	  检查角色是否濒死了
	  参数：    [IN]  wPlayerRole - the player role ID.
	  角色id
	  返回值：  TRUE if the player is dying, FALSE if not.
	  濒死返回TRUE，否则返回FALSE
	  --*/
{
	return gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] < gpGlobals->g.PlayerRoles.rgwMaxHP[wPlayerRole] / 5
		&& gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] < 200;
}

INT PAL_BattleSelectAutoTarget(VOID)
/*++
  功能：		Pick an enemy target automatically.
  自动选择一个目标敌人
  参数：		None.
  返回值：	The index of enemy. -1 if failed.
  怪物的序号，如果选择失败返回-1
  --*/
{
	int          i;

	i = (int)g_Battle.UI.wPrevEnemyTarget;

	if (i >= 0 && i <= g_Battle.wMaxEnemyIndex &&
		g_Battle.rgEnemy[i].wObjectID != 0 &&
		g_Battle.rgEnemy[i].dwActualHealth > 0)
	{
		return i;
	}

	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID != 0 &&
			g_Battle.rgEnemy[i].dwActualHealth > 0)
		{
			return i;
		}
	}

	return -1;
}

static INT
PAL_CalcBaseDamage(
	WORD wAttackStrength,
	WORD wDefense
	)
	/*++
	  功能：    Calculate the base damage value of attacking.
	  计算基础伤害值
	  参数：    [IN]  wAttackStrength - attack strength of attacker.
	  [IN]  wDefense - defense value of inflictor.
	  攻击者武术值，被攻击者防御值
	  返回值：   The base damage value of the attacking.
	  基础伤害值
	  --*/
{
	INT            iDamage;

	//
	// Formula courtesy of palxex and shenyanduxing
	//
#ifdef EDIT_DAMAGE_CALC
	if (wAttackStrength > 2 * wDefense)
	{
		iDamage = wAttackStrength * 1.5 - wDefense * 0.5;
	}
	else if (wAttackStrength > wDefense)
	{
		iDamage = wAttackStrength * 2 - wDefense * 1.5;
	}
	else if (wAttackStrength > wDefense * 0.5)
	{
		iDamage = wAttackStrength - wDefense * 0.5;
	}
	else
	{
		iDamage = 0;
	}
#else
	if (wAttackStrength > wDefense)
	{
		iDamage = (INT)(wAttackStrength * 2 - wDefense * 1.6 + 0.5);
	}
	else if (wAttackStrength > wDefense * 0.6)
	{
		iDamage = (INT)(wAttackStrength - wDefense * 0.6 + 0.5);
	}
	else
	{
		iDamage = 0;
	}
#endif

	return iDamage;
}

static INT
PAL_CalcMagicDamage(
	WORD			wMagicStrength,
	WORD			wDefense,
	SHORT			sElementalResistance[NUM_MAGIC_ELEMENTAL],
	SHORT			sPoisonResistance,
	WORD			wMagicID
	)
	/*++
		功能：     Calculate the damage of magic.
		计算法术伤害值
		参数：     [IN]  wMagicStrength - magic strength of attacker.
		[IN]  wDefense - defense value of inflictor.
		[IN]  rgwElementalResistance - inflictor's resistance to the elemental magics.
		[IN]  wPoisonResistance - inflictor's resistance to poison.
		[IN]  wMagicID - object ID of the magic.
		攻击者灵力值，被攻击者防御值，被攻击者对各属性伤害的抗性，被攻击者毒抗，法术id
		返回值： The damage value of the magic attack.
		法术伤害值
		--*/
{
	INT             iDamage;
	WORD            wElem;
	WORD	        wBaseDamage;
	WORD			wMagicNumber;

	wMagicNumber = gpGlobals->g.rgObject[wMagicID].magic.wMagicNumber;
	wBaseDamage = gpGlobals->g.lprgMagic[wMagicNumber].wBaseDamage;

	if ((SHORT)wBaseDamage < 0)
	{
		return 0;
	}

	// Formula courtesy of palxex and shenyanduxing
	wMagicStrength *= RandomFloat(1, 1.1);

	iDamage = PAL_CalcBaseDamage(wMagicStrength, wDefense);

	wElem = gpGlobals->g.lprgMagic[wMagicNumber].wElemental;

#ifdef EDIT_DAMAGE_CALC
	if (wElem >= 1 && wElem <= 6)
	{
		iDamage /= 2;
		iDamage += wBaseDamage * 2 / 3;
	}
	else
	{
		iDamage /= 3;
		iDamage += wBaseDamage;
	}

#else
	iDamage /= 4;
	iDamage += wBaseDamage;
#endif	


	switch (wElem)
	{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		{
			iDamage *= (100 - sElementalResistance[wElem - 1]) / 100.0;
			SHORT rgsMagicEffect = 10 + gpGlobals->g.lprgBattleField[gpGlobals->wNumBattleField].rgsMagicEffect[wElem - 1];
			iDamage *= rgsMagicEffect / 10.0;
			break;
		}

		case 6: //毒系
		{
			iDamage *= (100 - sPoisonResistance) / 100.0;
			break;
		}

		case 0:
		default:
			break;
	}
	return iDamage;
}

static INT
PAL_CalcPhysicalAttackDamage(
	WORD           wAttackStrength,
	WORD           wDefense,
	WORD           wAttackResistance
	)
	/*++
	  功能：    Calculate the damage value of physical attacking.
	  计算物理攻击伤害值
	  参数：    [IN]  wAttackStrength - attack strength of attacker.
	  [IN]  wDefense - defense value of inflictor.
	  [IN]  wAttackResistance - inflictor's resistance to physical attack.
	  攻击者武术值，被攻击者防御值，被攻击者物抗
	  返回值：    The damage value of the physical attacking.
	  物理攻击伤害值
	  --*/
{
	INT             iDamage;

	iDamage = PAL_CalcBaseDamage(wAttackStrength, wDefense);

	if (wAttackResistance != 0)
	{
		iDamage = iDamage * 10 / wAttackResistance;
	}

	return iDamage;
}

static WORD
PAL_GetPlayerActualDexterity(
	WORD			wPlayerRole
	)
	/*++
	  功能：    Get player's actual dexterity value in battle.
	  获得在战斗中角色的实际的身法值
	  参数：    [IN]  wPlayerRole - the player role ID.
	  角色id
	  返回值：    The player's actual dexterity value.
	  角色的实际的身法值
	  --*/
{
	WORD wDexterity = PAL_GetPlayerDexterity(wPlayerRole);

	if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusHaste] != 0)
	{
#ifdef PAL_CLASSIC
		if (gpGlobals->wMaxPartyMemberIndex < 3)
		{
			wDexterity *= 3;
		}
		else
		{
			wDexterity *= 6;
		}
#else
		wDexterity *= 6;
		wDexterity /= 5;
#endif
	}
#ifndef PAL_CLASSIC
	else if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSlow] != 0)
	{
		wDexterity *= 2;
		wDexterity /= 3;
	}
#endif

	if (PAL_IsPlayerDying(wPlayerRole))
	{
		//
		// player who is low of HP should be slower
		//
#ifdef PAL_CLASSIC
		wDexterity /= 2;
#else
		wDexterity *= 4;
		wDexterity /= 5;
#endif
	}

	return wDexterity;
}

static WORD
PAL_GetEnemyActualDexterity(
	WORD			wEnemyIndex
	)
{
	WORD wDexterity = PAL_New_GetEnemyDexterity(wEnemyIndex);
	return wDexterity;
}


#ifndef PAL_CLASSIC

VOID PAL_UpdateTimeChargingUnit(VOID)
/*++
  功能：    Update the base time unit of time-charging.
  参数：    None.
  返回值：    None.
  --*/
{
	g_Battle.flTimeChargingUnit = (FLOAT)(pow(PAL_GetPlayerDexterity(0) + 5, 0.3));
	g_Battle.flTimeChargingUnit /= PAL_GetPlayerDexterity(0);

	if (gpGlobals->bBattleSpeed > 1)
	{
		g_Battle.flTimeChargingUnit /= 1 + (gpGlobals->bBattleSpeed - 1) * 0.5;
	}
	else
	{
		g_Battle.flTimeChargingUnit /= 1.2f;
	}
}

FLOAT PAL_GetTimeChargingSpeed(WORD wDexterity)
/*++
  功能：    Calculate the time charging speed.
  计算充值速度
  参数：    [IN]  wDexterity - the dexterity value of player or enemy.
  角色或敌人的身法值
  返回值：    The time-charging speed of the player or enemy.
  充值速度
  --*/
{
	if ((g_Battle.UI.state == kBattleUISelectMove &&
		g_Battle.UI.MenuState != kBattleMenuMain) ||
		SDL_GetTicks() < g_Battle.UI.dwMsgShowTime)
	{
		//
		// Pause the time when there are submenus or text messages
		//有子菜单或有消息时，暂停一段时间
		return 0;
	}

	//
	// The battle should be faster when using Auto-Battle
	// 自动战斗时快一些
	if (gpGlobals->fAutoBattle)
	{
		wDexterity *= 3;
	}

	return g_Battle.flTimeChargingUnit * wDexterity;
}

#endif

VOID
PAL_BattleDelay(
	WORD       wDuration,
	WORD       wObjectID,
	BOOL       fUpdateGesture
	)
	/*++
	  功能：    Delay a while during battle.
	  当战斗时延迟一段时间
	  参数：    [IN]  wDuration - Number of frames of the delay.
	  [IN]  wObjectID - The object ID to be displayed during the delay.
	  [IN]  fUpdateGesture - TRUE if update the gesture for enemies, FALSE if not.
	  延迟帧数，延迟时需要显示的对象id，更新敌人动作（更新为真，否则为假）
	  返回值：    None.
	  --*/
{
	int    i, j;
	DWORD  dwTime = SDL_GetTicks() + BATTLE_FRAME_TIME;

	for (i = 0; i < wDuration; i++)
	{
		if (fUpdateGesture)
		{
			//
			// Update the gesture of enemies.
			// 更新敌人动作
			for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
			{
				if (g_Battle.rgEnemy[j].wObjectID == 0 ||
					g_Battle.rgEnemy[j].rgwStatus[kStatusSleep] != 0 ||
					g_Battle.rgEnemy[j].rgwStatus[kStatusParalyzed] != 0)
				{
					continue;
				}

				if (--g_Battle.rgEnemy[j].e.wIdleAnimSpeed == 0)
				{
					g_Battle.rgEnemy[j].wCurrentFrame++;
					g_Battle.rgEnemy[j].e.wIdleAnimSpeed =
						gpGlobals->g.lprgEnemy[gpGlobals->g.rgObject[g_Battle.rgEnemy[j].wObjectID].enemy.wEnemyID].wIdleAnimSpeed;
				}

				if (g_Battle.rgEnemy[j].wCurrentFrame >= g_Battle.rgEnemy[j].e.wIdleFrames)
				{
					g_Battle.rgEnemy[j].wCurrentFrame = 0;
				}
			}
		}

		//
		// Wait for the time of one frame. Accept input here.
		// 等待一帧的时间，在此接受输入
		PAL_ProcessEvent();
		while (SDL_GetTicks() <= dwTime)
		{
			PAL_ProcessEvent();
			SDL_Delay(1);
		}

		//
		// Set the time of the next frame.
		// 设置下一帧的时间
		dwTime = SDL_GetTicks() + BATTLE_FRAME_TIME;

		PAL_BattleMakeScene();
		SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);
		PAL_BattleUIUpdate();

		if (wObjectID != 0)
		{
			if (wObjectID == BATTLE_LABEL_ESCAPEFAIL) // HACKHACK
			{
				PAL_DrawText(PAL_GetWord(wObjectID), PAL_XY(130, 75),
					15, TRUE, FALSE);
			}
			else if ((SHORT)wObjectID < 0)
			{
				PAL_DrawText(PAL_GetWord(-((SHORT)wObjectID)), PAL_XY(170, 45),
					DESCTEXT_COLOR, TRUE, FALSE);
			}
			else
			{
				PAL_DrawText(PAL_GetWord(wObjectID), PAL_XY(210, 50),
					15, TRUE, FALSE);
			}
		}

		VIDEO_UpdateScreen(NULL);
	}
}

VOID
PAL_BattleBackupStat(
	VOID
	)
	/*++
	  功能：    Backup HP and MP values of all players and enemies.
	  备份角色和敌人的hp和mp
	  参数：    None.
	  返回值：  None.
	  --*/
{
	int          i;
	WORD         wPlayerRole;

	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID == 0)
		{
			continue;
		}
		g_Battle.rgEnemy[i].wPrevHP = g_Battle.rgEnemy[i].dwActualHealth;
	}

	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

		g_Battle.rgPlayer[i].wPrevHP =
			gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole];
		g_Battle.rgPlayer[i].wPrevMP =
			gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole];
	}
}

BOOL
PAL_BattleDisplayStatChange(
	VOID
	)
	/*++
	  功能：    Display the HP and MP changes of all players and enemies.
	  展示角色和敌人的hp和mp改变值
	  参数：    None.
	  返回值：  TRUE if there are any number displayed, FALSE if not.
	  如果有显示数字返回真，否则返回假
	  --*/
{
	int      i, x, y;
	INT      sDamage;
	WORD     wPlayerRole;
	BOOL     f = FALSE;

	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID == 0)
		{
			continue;
		}

		if (g_Battle.rgEnemy[i].wPrevHP != g_Battle.rgEnemy[i].dwActualHealth)
		{
			//
			// Show the number of damage
			//
			sDamage = g_Battle.rgEnemy[i].dwActualHealth - g_Battle.rgEnemy[i].wPrevHP;

			x = PAL_X(g_Battle.rgEnemy[i].pos) - 9;
			y = PAL_Y(g_Battle.rgEnemy[i].pos) - 115;
			y = max(10, y);

			if (sDamage > 0)
			{
				PAL_BattleUIShowNum((WORD)(sDamage), PAL_XY(x, y), kNumColorYellow);
			}
			else
			{
				PAL_BattleUIShowNum((WORD)(-sDamage), PAL_XY(x, y), kNumColorBlue);
			}
			f = TRUE;
		}
	}

	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

		if (g_Battle.rgPlayer[i].wPrevHP != gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole])
		{
			sDamage = gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] - g_Battle.rgPlayer[i].wPrevHP;

			x = PAL_X(g_Battle.rgPlayer[i].pos) - 9;
			y = PAL_Y(g_Battle.rgPlayer[i].pos) - 75;
			y = max(10, y);

			if (sDamage > 0)
			{
				PAL_BattleUIShowNum((WORD)(sDamage), PAL_XY(x, y), kNumColorYellow);
			}
			else
			{
				PAL_BattleUIShowNum((WORD)(-sDamage), PAL_XY(x, y), kNumColorBlue);
			}
			f = TRUE;
		}

		if (g_Battle.rgPlayer[i].wPrevMP != gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole])
		{
			sDamage = gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole] - g_Battle.rgPlayer[i].wPrevMP;

			x = PAL_X(g_Battle.rgPlayer[i].pos) - 9;
			y = PAL_Y(g_Battle.rgPlayer[i].pos) - 67;
			y = max(10, y);

			// Only show MP increasing
			if (sDamage > 0)
			{
				PAL_BattleUIShowNum((WORD)(sDamage), PAL_XY(x, y), kNumColorCyan);
			}
#ifdef SHOW_MP_DECREASING
			else
			{
				PAL_BattleUIShowNum((WORD)(-sDamage), PAL_XY(x, y), kNumColorBlue);
			}
#endif
			f = TRUE;
		}
	}

	return f;
}

static VOID
PAL_BattlePostActionCheck(
	BOOL fCheckPlayers
	)
	/*++
	  功能：    Essential checks after an action is executed.
	  参数：    [IN]  fCheckPlayers - TRUE if check for players, FALSE if not.
	  返回值：    None.
	  --*/
{
	int      i, j;
	BOOL     fFade = FALSE;
	BOOL     fEnemyRemaining = FALSE;

	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID == 0)
		{
			continue;
		}

		if ((g_Battle.rgEnemy[i].dwActualHealth) == 0)
		{
			// This enemy is KO'ed
			g_Battle.iExpGained += g_Battle.rgEnemy[i].e.wExp;

#ifdef EDIT_EXP_CALCULATION
			for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
			{
				WORD w = gpGlobals->rgParty[i].wPlayerRole;
				if (gpGlobals->g.PlayerRoles.rgwHP[w] != 0)
				{
					g_Battle.rgPlayer[j].dwExpGained += g_Battle.rgEnemy[i].e.wExp;
				}
				else
				{
					g_Battle.rgPlayer[j].dwExpGained += g_Battle.rgEnemy[i].e.wExp / 10;
				}
			}
#endif
			g_Battle.iCashGained += g_Battle.rgEnemy[i].e.wCash;

			SOUND_Play(g_Battle.rgEnemy[i].e.wDeathSound);
			g_Battle.rgEnemy[i].wObjectID = 0;
			fFade = TRUE;

			continue;
		}

		fEnemyRemaining = TRUE;
	}

	if (!fEnemyRemaining)
	{
		g_Battle.fEnemyCleared = TRUE;
		g_Battle.UI.state = kBattleUIWait;
	}

	if (fCheckPlayers && !gpGlobals->fAutoBattle)
	{
		for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
		{
			WORD w = gpGlobals->rgParty[i].wPlayerRole;

			if (gpGlobals->g.PlayerRoles.rgwHP[w] < g_Battle.rgPlayer[i].wPrevHP &&
				gpGlobals->g.PlayerRoles.rgwHP[w] == 0)
			{	//角色死亡，掩护的角色会愤怒。。。
				w = gpGlobals->g.PlayerRoles.rgwCoveredBy[w];

				if (PAL_New_GetPlayerIndex(w) == -1)
				{
					continue;
				}

				if (gpGlobals->g.PlayerRoles.rgwHP[w] > 0 && PAL_New_IfPlayerCanMove(w))
				{
					WORD wName = gpGlobals->g.PlayerRoles.rgwName[w];

					if (gpGlobals->g.rgObject[wName].player.wScriptOnFriendDeath != 0)
					{
						PAL_BattleDelay(10, 0, TRUE);

						PAL_BattleMakeScene();
						SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);
						VIDEO_UpdateScreen(NULL);

						g_Battle.BattleResult = kBattleResultPause;

						gpGlobals->g.rgObject[wName].player.wScriptOnFriendDeath =
							PAL_RunTriggerScript(gpGlobals->g.rgObject[wName].player.wScriptOnFriendDeath, w);

						g_Battle.BattleResult = kBattleResultOnGoing;

						PAL_ClearKeyState();
						goto end;
					}
				}
			}
		}

		for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
		{
			WORD w = gpGlobals->rgParty[i].wPlayerRole;

			if (!PAL_New_IfPlayerCanMove(w))
			{
				continue;
			}

			if (gpGlobals->g.PlayerRoles.rgwHP[w] < g_Battle.rgPlayer[i].wPrevHP)
			{
				//从正常状态到濒死状态，会有相应的脚本
				if (gpGlobals->g.PlayerRoles.rgwHP[w] > 0 && PAL_IsPlayerDying(w) &&
					g_Battle.rgPlayer[i].wPrevHP >= gpGlobals->g.PlayerRoles.rgwMaxHP[w] / 5)
				{
					SOUND_Play(gpGlobals->g.PlayerRoles.rgwDyingSound[w]);

					WORD wCover = gpGlobals->g.PlayerRoles.rgwCoveredBy[w];
					if (PAL_New_GetPlayerIndex(wCover) == -1	//掩护角色不在队伍中
						|| gpGlobals->g.PlayerRoles.rgwHP[wCover] == 0
						|| !PAL_New_IfPlayerCanMove(wCover))	//掩护角色不能行动
					{
						continue;
					}

					WORD wName = gpGlobals->g.PlayerRoles.rgwName[w];
					if (gpGlobals->g.rgObject[wName].player.wScriptOnDying != 0)
					{
						PAL_BattleDelay(10, 0, TRUE);
						PAL_BattleMakeScene();
						SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);
						VIDEO_UpdateScreen(NULL);

						g_Battle.BattleResult = kBattleResultPause;

						gpGlobals->g.rgObject[wName].player.wScriptOnDying =
							PAL_RunTriggerScript(gpGlobals->g.rgObject[wName].player.wScriptOnDying, w);

						g_Battle.BattleResult = kBattleResultOnGoing;
						PAL_ClearKeyState();
					}

					goto end;
				}
			}
		}
	}

end:
	if (fFade)
	{
		PAL_BattleBackupScene();
		PAL_BattleMakeScene();
		PAL_BattleFadeScene();
	}

	//
	// Fade out the summoned god
	//
	if (g_Battle.lpSummonSprite != NULL)
	{
		PAL_BattleUpdateFighters();
		PAL_BattleDelay(1, 0, FALSE);

		free(g_Battle.lpSummonSprite);
		g_Battle.lpSummonSprite = NULL;

		g_Battle.sBackgroundColorShift = 0;

		PAL_BattleBackupScene();
		PAL_BattleMakeScene();
		PAL_BattleFadeScene();
	}
}

VOID
PAL_BattleUpdateFighters(
	VOID
	)
	/*++
	  功能：    Update players' and enemies' gestures and locations in battle.
	  参数：    None.
	  返回值：  None.
	  --*/
{
	int        i;
	WORD       wPlayerRole;

	// Update the gesture for all players
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

		g_Battle.rgPlayer[i].pos = g_Battle.rgPlayer[i].posOriginal;
		g_Battle.rgPlayer[i].iColorShift = 0;

		if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0)
		{
			if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] == 0)
			{
				g_Battle.rgPlayer[i].wCurrentFrame = 2; // dead		//死亡
			}
			else
			{
				g_Battle.rgPlayer[i].wCurrentFrame = 0; // puppet	//傀儡虫作用
			}
		}
		else
		{
			if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] != 0 ||
				PAL_IsPlayerDying(wPlayerRole))
			{
				g_Battle.rgPlayer[i].wCurrentFrame = 1;
			}
#ifndef PAL_CLASSIC
			else if (g_Battle.rgPlayer[i].state == kFighterAct &&
				g_Battle.rgPlayer[i].action.ActionType == kBattleActionMagic &&
				!g_Battle.fEnemyCleared)
			{
				//
				// Player is using a magic
				//
				g_Battle.rgPlayer[i].wCurrentFrame = 5;
			}
#endif
			else if (g_Battle.rgPlayer[i].fDefending && !g_Battle.fEnemyCleared)
			{
				g_Battle.rgPlayer[i].wCurrentFrame = 3;
			}
			else
			{
				g_Battle.rgPlayer[i].wCurrentFrame = 0;
			}
		}
	}

	// Update the gesture for all enemies
	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID == 0)
		{
			continue;
		}

		g_Battle.rgEnemy[i].pos = g_Battle.rgEnemy[i].posOriginal;
		g_Battle.rgEnemy[i].iColorShift = 0;

		if (g_Battle.rgEnemy[i].rgwStatus[kStatusSleep] > 0 ||
			g_Battle.rgEnemy[i].rgwStatus[kStatusParalyzed] > 0)
		{
			g_Battle.rgEnemy[i].wCurrentFrame = 0;
			continue;
		}

		if (--g_Battle.rgEnemy[i].e.wIdleAnimSpeed == 0)
		{
			g_Battle.rgEnemy[i].wCurrentFrame++;
			g_Battle.rgEnemy[i].e.wIdleAnimSpeed =
				gpGlobals->g.lprgEnemy[gpGlobals->g.rgObject[g_Battle.rgEnemy[i].wObjectID].enemy.wEnemyID].wIdleAnimSpeed;
		}

		if (g_Battle.rgEnemy[i].wCurrentFrame >= g_Battle.rgEnemy[i].e.wIdleFrames)
		{
			g_Battle.rgEnemy[i].wCurrentFrame = 0;
		}
	}
}

VOID PAL_BattlePlayerCheckReady(VOID)
/*++
  功能：

  Check if there are player who is ready.

  参数：

  None.

  返回值：

  None.

  --*/
{
	float   flMax = 0;
	int     iMax = 0, i;

	//
	// Start the UI for the fastest and ready player
	//
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		if (g_Battle.rgPlayer[i].state == kFighterCom ||
			(g_Battle.rgPlayer[i].state == kFighterAct && g_Battle.rgPlayer[i].action.ActionType == kBattleActionCoopMagic))
		{
			flMax = 0;
			break;
		}
		else if (g_Battle.rgPlayer[i].state == kFighterWait)
		{
			if (g_Battle.rgPlayer[i].flTimeMeter > flMax)
			{
				iMax = i;
				flMax = g_Battle.rgPlayer[i].flTimeMeter;
			}
		}
	}

	if (flMax >= 100.0f)
	{
		g_Battle.rgPlayer[iMax].state = kFighterCom;
		g_Battle.rgPlayer[iMax].fDefending = FALSE;
	}
}

VOID PAL_BattleStartFrame(VOID)
/*++
  功能：    Called once per video frame in battle.
  参数：    None.
  返回值：    None.
  --*/
{
	int                      i, j;
	WORD                     wPlayerRole;
	DWORD		             wDexterity;
	BOOL                     fOnlyPuppet = TRUE;
	WORD					 wMoveTimes;
	BOOL					 fDexterityIncrease;
#ifndef PAL_CLASSIC
	FLOAT                    flMax;
	BOOL                     fMoved = FALSE;
	SHORT                    sMax, sMaxIndex;
#endif

	if (!g_Battle.fEnemyCleared)
	{
		PAL_BattleUpdateFighters();
	}

	// Update the scene
	PAL_BattleMakeScene();
	SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);

	//
	// Check if the battle is over
	//
	if (g_Battle.fEnemyCleared)
	{
		//
		// All enemies are cleared. Won the battle.
		//
		g_Battle.BattleResult = kBattleResultWon;
		SOUND_Play(-1);
		return;
	}
	else
	{
		BOOL fEnded = TRUE;

		for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
		{
			wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

			if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] != 0)
			{
				fOnlyPuppet = FALSE;
				fEnded = FALSE;
				break;
			}
			else if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] != 0)
			{
				fEnded = FALSE;
			}
		}

		if (fEnded)
		{
			// All players are dead. Lost the battle.
			g_Battle.BattleResult = kBattleResultLost;
			return;
		}
	}

#ifndef PAL_CLASSIC
	//
	// Check for hiding status
	//
	if (g_Battle.iHidingTime > 0)
	{
		if (PAL_GetTimeChargingSpeed(9999) > 0)
		{
			g_Battle.iHidingTime--;
		}

		if (g_Battle.iHidingTime == 0)
		{
			PAL_BattleBackupScene();
			PAL_BattleMakeScene();
			PAL_BattleFadeScene();
		}
	}

	//
	// Run the logic for all enemies
	//
	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID == 0)
		{
			continue;
		}

		if (g_Battle.rgEnemy[i].fTurnStart)
		{
			g_Battle.rgEnemy[i].wScriptOnTurnStart =
				PAL_RunTriggerScript(g_Battle.rgEnemy[i].wScriptOnTurnStart, i);

			g_Battle.rgEnemy[i].fTurnStart = FALSE;
			fMoved = TRUE;
		}
	}

	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID == 0)
		{
			continue;
		}

		switch (g_Battle.rgEnemy[i].state)
		{
			case kFighterWait:
				flMax = PAL_GetTimeChargingSpeed(PAL_GetEnemyActualDexterity(i));
				flMax /= (gpGlobals->fAutoBattle ? 2 : 1);

				if (flMax != 0)
				{
					g_Battle.rgEnemy[i].flTimeMeter += flMax;

					if (g_Battle.rgEnemy[i].flTimeMeter > 100 && flMax > 0)
					{
						if (g_Battle.iHidingTime == 0)
						{
							g_Battle.rgEnemy[i].state = kFighterCom;
						}
						else
						{
							g_Battle.rgEnemy[i].flTimeMeter = 0;
						}
					}
				}
				break;

			case kFighterCom:
				g_Battle.rgEnemy[i].wScriptOnReady =
					PAL_RunTriggerScript(g_Battle.rgEnemy[i].wScriptOnReady, i);
				g_Battle.rgEnemy[i].state = kFighterAct;
				fMoved = TRUE;
				break;

			case kFighterAct:
				if (!fMoved && (PAL_GetTimeChargingSpeed(9999) > 0 || g_Battle.rgEnemy[i].fDualMove) && !fOnlyPuppet)
				{
					fMoved = TRUE;

					g_Battle.fEnemyMoving = TRUE;

					g_Battle.wMovingEnemyIndex = i;

					g_Battle.rgEnemy[i].fDualMove =
						(!g_Battle.rgEnemy[i].fFirstMoveDone &&
							(g_Battle.rgEnemy[i].e.wDualMove >= 2 ||
								(g_Battle.rgEnemy[i].e.wDualMove != 0 && RandomLong(0, 1))));

					PAL_BattleEnemyPerformAction(i);

					g_Battle.rgEnemy[i].flTimeMeter = 0;
					g_Battle.rgEnemy[i].state = kFighterWait;
					g_Battle.fEnemyMoving = FALSE;

					if (g_Battle.rgEnemy[i].fDualMove)
					{
						g_Battle.rgEnemy[i].flTimeMeter = 100;
						g_Battle.rgEnemy[i].state = kFighterCom;
						g_Battle.rgEnemy[i].fFirstMoveDone = TRUE;
					}
					else
					{
						g_Battle.rgEnemy[i].fFirstMoveDone = FALSE;
						g_Battle.rgEnemy[i].fTurnStart = TRUE;
					}
				}
				break;
		}
	}

	//
	// Update the battle UI
	//
	PAL_BattleUIUpdate();

	//
	// Run the logic for all players
	//
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

		//
		// Skip dead players
		//
		if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0 &&
			gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] == 0)
		{
			g_Battle.rgPlayer[i].state = kFighterWait;
			g_Battle.rgPlayer[i].flTimeMeter = 0;
			g_Battle.rgPlayer[i].flTimeSpeedModifier = 1.0f;
			g_Battle.rgPlayer[i].sTurnOrder = -1;
			continue;
		}

		switch (g_Battle.rgPlayer[i].state)
		{
			case kFighterWait:
				wDexterity = PAL_GetPlayerActualDexterity(wPlayerRole);
				g_Battle.rgPlayer[i].flTimeMeter +=
					PAL_GetTimeChargingSpeed(wDexterity) * g_Battle.rgPlayer[i].flTimeSpeedModifier;
				break;

			case kFighterCom:
				break;

			case kFighterAct:
				if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] > 0)
				{
					g_Battle.rgPlayer[i].action.ActionType = kBattleActionPass;
					g_Battle.rgPlayer[i].action.flRemainingTime = 0;
				}
				else if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusConfused] > 0)
				{
					g_Battle.rgPlayer[i].action.ActionType = kBattleActionAttackMate;
					g_Battle.rgPlayer[i].action.flRemainingTime = 0;
				}
				else if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSilence] > 0 &&
					g_Battle.rgPlayer[i].action.ActionType == kBattleActionMagic)
				{
					g_Battle.rgPlayer[i].action.flRemainingTime = 0;
				}

				wDexterity = PAL_GetPlayerActualDexterity(wPlayerRole);
				g_Battle.rgPlayer[i].action.flRemainingTime -= PAL_GetTimeChargingSpeed(wDexterity);

				if (g_Battle.rgPlayer[i].action.flRemainingTime <= 0 &&
					g_Battle.rgPlayer[i].sTurnOrder == -1)
				{
					sMax = -1;

					for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
					{
						if (g_Battle.rgPlayer[j].sTurnOrder > sMax)
						{
							sMax = g_Battle.rgPlayer[j].sTurnOrder;
						}
					}

					g_Battle.rgPlayer[i].sTurnOrder = sMax + 1;
				}

				break;
		}
	}

	//
	// Preform action for player
	//
	if (!fMoved)
	{
		sMax = 9999;
		sMaxIndex = -1;

		for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
		{
			wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

			//
			// Skip dead players
			//
			if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0 &&
				gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] == 0)
			{
				continue;
			}

			if (g_Battle.rgPlayer[i].state == kFighterAct &&
				g_Battle.rgPlayer[i].sTurnOrder != -1 &&
				g_Battle.rgPlayer[i].sTurnOrder < sMax)
			{
				sMax = g_Battle.rgPlayer[i].sTurnOrder;
				sMaxIndex = i;
			}
		}

		if (sMaxIndex != -1)
		{
			//
			// Perform the action for this player.
			//
			PAL_BattlePlayerPerformAction(sMaxIndex);

			g_Battle.rgPlayer[sMaxIndex].flTimeMeter = 0;
			g_Battle.rgPlayer[sMaxIndex].flTimeSpeedModifier = 1.0f;
			g_Battle.rgPlayer[sMaxIndex].sTurnOrder = -1;
		}
	}
#else
	if (g_Battle.Phase == kBattlePhaseSelectAction)
	{
		if (g_Battle.UI.state == kBattleUIWait)
		{
			for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

				//
				// Don't select action for this player if player is KO'ed,
				// sleeped, confused or paralyzed
				//
				if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0 || !PAL_New_IfPlayerCanMove(wPlayerRole))
				{
					continue;
				}

				//
				// Start the menu for the first player whose action is not
				// yet selected
				//
				if (g_Battle.rgPlayer[i].state == kFighterWait)
				{
					g_Battle.wMovingPlayerIndex = i;
					g_Battle.rgPlayer[i].state = kFighterCom;
					PAL_BattleUIPlayerReady(i);
					break;
				}
				else if (g_Battle.rgPlayer[i].action.ActionType == kBattleActionCoopMagic)
				{
					//
					// Skip other players if someone selected coopmagic
					//
					i = gpGlobals->wMaxPartyMemberIndex + 1;
					break;
				}
			}

			if (i > gpGlobals->wMaxPartyMemberIndex)
			{
				//
				// actions for all players are decided. fill in the action queue.
				//
				g_Battle.fRepeat = FALSE;
				g_Battle.fForce = FALSE;
				g_Battle.fFlee = FALSE;

				g_Battle.iCurAction = 0;

				for (i = 0; i < MAX_ACTIONQUEUE_ITEMS; i++)
				{
					g_Battle.ActionQueue[i].wIndex = 0xFFFF;
					g_Battle.ActionQueue[i].wDexterity = 0;
					g_Battle.ActionQueue[i].fIsEmpty = TRUE;
				}

				//
				// Put all enemies into action queue
				//
				for (i = 0, j = 0; i <= g_Battle.wMaxEnemyIndex; i++)
				{
					if (g_Battle.rgEnemy[i].wObjectID == 0)
					{
						continue;
					}

					wMoveTimes = g_Battle.rgEnemy[i].e.wDualMove + 2;
					int iHealthyPlayerNum = PAL_New_GetHealthyPlayerNum();

					if (gpGlobals->wMaxPartyMemberIndex > 2)
					{
						wMoveTimes += min(2, max((iHealthyPlayerNum - 1) / 2, 0));
					}

#ifdef FINISH_GAME_MORE_ONE_TIME
					if (gpGlobals->bFinishGameTime > 0)
					{
						wMoveTimes += min(2, max(iHealthyPlayerNum - 1, 0));
						wMoveTimes += min(1, max(gpGlobals->bFinishGameTime - 1, 0));
					}
#endif 										
					if (wMoveTimes % 2 == 1 && (PAL_New_GetAliveEnemyNum() < 2) || RandomLong(0, 9) < 5)
					{
						wMoveTimes++;
					}
					wMoveTimes /= 2;

					WORD wExtraMoveTimes = 0;

#ifdef FINISH_GAME_MORE_ONE_TIME
					wExtraMoveTimes = min(1, max(gpGlobals->bFinishGameTime - 1, 0));
#endif 						
					wMoveTimes = min(wMoveTimes, ENEMY_MAX_MOVE + wExtraMoveTimes);

					fDexterityIncrease = FALSE;
					if (iHealthyPlayerNum >= 3)
					{
						fDexterityIncrease = TRUE;
					}

					for (; wMoveTimes > 0; wMoveTimes--)
					{
						g_Battle.ActionQueue[j].fIsEmpty = FALSE;
						g_Battle.ActionQueue[j].fIsEnemy = TRUE;
						g_Battle.ActionQueue[j].wIndex = i;
						g_Battle.ActionQueue[j].wDexterity = PAL_GetEnemyActualDexterity(i);
						g_Battle.ActionQueue[j].wDexterity *= RandomFloat(0.9, 1.1);

						if (fDexterityIncrease == TRUE)
						{
							g_Battle.ActionQueue[j].wDexterity *= RandomFloat(2, iHealthyPlayerNum);
						}
						j++;
					}
				}

				//
				// Put all players into action queue
				//
				for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
				{
					wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

					g_Battle.ActionQueue[j].fIsEmpty = FALSE;
					g_Battle.ActionQueue[j].fIsEnemy = FALSE;
					g_Battle.ActionQueue[j].wIndex = i;

					if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0 ||
						gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] > 0 ||
						gpGlobals->rgPlayerStatus[wPlayerRole][kStatusParalyzed] > 0)
					{
						//
						// players who are unable to move should attack physically if recovered
						// in the same turn
						//
						g_Battle.ActionQueue[j].wDexterity = 0;
						g_Battle.rgPlayer[i].action.ActionType = kBattleActionAttack;
						g_Battle.rgPlayer[i].state = kFighterAct;
					}
					else
					{
						wDexterity = PAL_GetPlayerActualDexterity(wPlayerRole);

						if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusConfused] > 0)
						{
							g_Battle.rgPlayer[i].action.ActionType = kBattleActionAttack;
							g_Battle.rgPlayer[i].state = kFighterAct;
						}

						switch (g_Battle.rgPlayer[i].action.ActionType)
						{
							case kBattleActionCoopMagic:
								wDexterity *= 10;
								break;

							case kBattleActionDefend:
								wDexterity *= 5;
								break;

							case kBattleActionMagic:
								if ((gpGlobals->g.rgObject[g_Battle.rgPlayer[i].action.wActionID].magic.wFlags & kMagicFlagUsableToEnemy) == 0)
								{
									wDexterity *= 3;
								}
								break;

							case kBattleActionFlee:
								wDexterity /= 2;
								break;

							case kBattleActionUseItem:
								wDexterity *= 3;
								break;

#ifdef EDIT_DAMAGE_CALC 
							case kBattleActionAttack:
								wDexterity *= 1.25;
								break;
#endif
							default:
								break;
						}

						if (PAL_IsPlayerDying(wPlayerRole))
						{
							wDexterity /= 2;
						}

						wDexterity *= RandomFloat(0.9, 1.1);

						g_Battle.ActionQueue[j].wDexterity = wDexterity;
					}

					j++;
				}

				//
				// Sort the action queue by dexterity value
				//
				for (i = 0; i < MAX_ACTIONQUEUE_ITEMS; i++)
				{
					for (j = i; j < MAX_ACTIONQUEUE_ITEMS; j++)
					{
						if (g_Battle.ActionQueue[i].wDexterity < g_Battle.ActionQueue[j].wDexterity)
						{
							ACTIONQUEUE t = g_Battle.ActionQueue[i];
							g_Battle.ActionQueue[i] = g_Battle.ActionQueue[j];
							g_Battle.ActionQueue[j] = t;
						}
					}
				}

				//
				// Perform the actions
				//
				g_Battle.Phase = kBattlePhasePerformAction;
			}
		}
	}
	else
	{
		//
		// Are all actions finished?
		//
#ifdef POISON_STATUS_EXPAND
		DWORD dwPrevMP = 0;
		DWORD dwPrevHP = 0;
		INT iExtraPoisonDamage = 0;
		INT iExtraPoisonMPDamage = 0;
#endif

		if (g_Battle.iCurAction >= MAX_ACTIONQUEUE_ITEMS ||
			g_Battle.ActionQueue[g_Battle.iCurAction].fIsEmpty == TRUE)
		{
			for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				g_Battle.rgPlayer[i].fDefending = FALSE;
			}

			PAL_BattleBackupStat();

			for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

				// Update statuses
				for (j = 0; j < kStatusAll; j++)
				{
					if (gpGlobals->rgPlayerStatus[wPlayerRole][j] > 0)
					{
						gpGlobals->rgPlayerStatus[wPlayerRole][j]--;
					}
				}
				//
				// Run poison scripts
				// 执行中毒脚本前、后都需要排序
				PAL_New_SortPoisonsForPlayerByLevel(wPlayerRole);
				for (j = 0; j < MAX_POISONS; j++)
				{
					if (gpGlobals->rgPoisonStatus[j][i].wPoisonID != 0
						&& gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] != 0
						)
					{
#ifdef POISON_STATUS_EXPAND
						dwPrevHP = gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole];
						dwPrevMP = gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole];

						gpGlobals->rgPoisonStatus[j][i].wPoisonScript =
							PAL_RunTriggerScript(gpGlobals->rgPoisonStatus[j][i].wPoisonScript, wPlayerRole);

						iExtraPoisonDamage = dwPrevHP - gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole];
						iExtraPoisonMPDamage = dwPrevMP - gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole];

						iExtraPoisonDamage = max(iExtraPoisonDamage, 0);
						iExtraPoisonMPDamage = max(iExtraPoisonMPDamage, 0);
						iExtraPoisonDamage *= gpGlobals->rgPoisonStatus[j][i].wPoisonIntensity;
						iExtraPoisonMPDamage *= gpGlobals->rgPoisonStatus[j][i].wPoisonIntensity;
						iExtraPoisonDamage = min(iExtraPoisonDamage, MAX_DAMAGE);
						iExtraPoisonMPDamage = min(iExtraPoisonMPDamage, MAX_DAMAGE);
						PAL_IncreaseHPMP(wPlayerRole, -(SHORT)iExtraPoisonDamage, -(SHORT)iExtraPoisonMPDamage);
#else
						gpGlobals->rgPoisonStatus[j][i].wPoisonScript =
							PAL_RunTriggerScript(gpGlobals->rgPoisonStatus[j][i].wPoisonScript, wPlayerRole);
#endif
					}
				}
				PAL_New_SortPoisonsForPlayerByLevel(wPlayerRole);

				//如果死亡了并且不是傀儡虫控制状态，则移除所有的状态
				if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0
					&& gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] == 0)
				{
					PAL_New_RemovePlayerAllStatus(wPlayerRole);
				}
			}

			for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
			{
				// Update statuses
				for (j = 0; j < kStatusAll; j++)
				{
					if (g_Battle.rgEnemy[i].rgwStatus[j] > 0)
					{
						g_Battle.rgEnemy[i].rgwStatus[j]--;
					}
				}
				//
				// Run poison scripts
				// 执行中毒脚本前、后都需要排序
				PAL_New_SortPoisonsForEnemyByLevel(i);
				for (j = 0; j < MAX_POISONS; j++)
				{
					WORD wPoisonID = g_Battle.rgEnemy[i].rgPoisons[j].wPoisonID;
					if (wPoisonID != 0)
					{
#ifdef POISON_STATUS_EXPAND
						dwPrevHP = g_Battle.rgEnemy[i].dwActualHealth;
						g_Battle.rgEnemy[i].rgPoisons[j].wPoisonScript =
							PAL_RunTriggerScript(g_Battle.rgEnemy[i].rgPoisons[j].wPoisonScript, (WORD)i);
						iExtraPoisonDamage = dwPrevHP - g_Battle.rgEnemy[i].dwActualHealth;
						WORD wPoisonIntensity = g_Battle.rgEnemy[i].rgPoisons[j].wPoisonIntensity;
						if (iExtraPoisonDamage > 0 && wPoisonIntensity > 0)
						{
							iExtraPoisonDamage *= wPoisonIntensity;
							iExtraPoisonDamage = min(iExtraPoisonDamage, g_Battle.rgEnemy[i].dwActualHealth);
							g_Battle.rgEnemy[i].dwActualHealth -= iExtraPoisonDamage;
						}
#else
						g_Battle.rgEnemy[i].rgPoisons[j].wPoisonScript =
							PAL_RunTriggerScript(g_Battle.rgEnemy[i].rgPoisons[j].wPoisonScript, (WORD)i);
#endif
						}
					}
				PAL_New_SortPoisonsForEnemyByLevel(i);
				}

			PAL_BattlePostActionCheck(FALSE);
			if (PAL_BattleDisplayStatChange())
			{
				PAL_BattleDelay(8, 0, TRUE);
			}

			if (g_Battle.iHidingTime > 0)
			{
				if (--g_Battle.iHidingTime == 0)
				{
					PAL_BattleBackupScene();
					PAL_BattleMakeScene();
					PAL_BattleFadeScene();
				}
			}

			if (g_Battle.iHidingTime == 0)
			{
				for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
				{
					if (g_Battle.rgEnemy[i].wObjectID == 0)
					{
						continue;
					}

					g_Battle.rgEnemy[i].wScriptOnTurnStart =
						PAL_RunTriggerScript(g_Battle.rgEnemy[i].wScriptOnTurnStart, i);
				}
			}

			// Clear all item-using records
			for (i = 0; i < MAX_INVENTORY; i++)
			{
				gpGlobals->rgInventory[i].nAmountInUse = 0;
			}

			// Proceed to next turn...
			g_Battle.Phase = kBattlePhaseSelectAction;
			}
		else
		{
			i = g_Battle.ActionQueue[g_Battle.iCurAction].wIndex;

			if (g_Battle.ActionQueue[g_Battle.iCurAction].fIsEnemy)
			{
				if (g_Battle.iHidingTime == 0 && !fOnlyPuppet &&
					g_Battle.rgEnemy[i].wObjectID != 0)
				{
					g_Battle.rgEnemy[i].wScriptOnReady =
						PAL_RunTriggerScript(g_Battle.rgEnemy[i].wScriptOnReady, i);

					g_Battle.fEnemyMoving = TRUE;
					g_Battle.wMovingEnemyIndex = i;
					PAL_BattleEnemyPerformAction(i);
					g_Battle.fEnemyMoving = FALSE;
				}
			}
			else if (g_Battle.rgPlayer[i].state == kFighterAct)
			{
				wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

				if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0)
				{
					if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] == 0)
					{
						g_Battle.rgPlayer[i].action.ActionType = kBattleActionPass;
					}
				}
				else if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] > 0 ||
					gpGlobals->rgPlayerStatus[wPlayerRole][kStatusParalyzed] > 0)
				{
					g_Battle.rgPlayer[i].action.ActionType = kBattleActionPass;
				}
				else if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusConfused] > 0)
				{
					g_Battle.rgPlayer[i].action.ActionType = kBattleActionAttackMate;
				}

				//
				// Perform the action for this player.
				//
				g_Battle.fPlayerMoving = TRUE;
				g_Battle.wMovingPlayerIndex = i;
				PAL_BattlePlayerPerformAction(i);
				g_Battle.fPlayerMoving = FALSE;
			}

			g_Battle.iCurAction++;
		}
		}

	// The R and F keys and Fleeing should affect all players
	if (g_Battle.UI.MenuState == kBattleMenuMain &&
		g_Battle.UI.state == kBattleUISelectMove)
	{
		if (g_InputState.dwKeyPress & kKeyRepeat)
		{
			g_Battle.fRepeat = TRUE;
		}
		else if (g_InputState.dwKeyPress & kKeyForce)
		{
			g_Battle.fForce = TRUE;
		}
	}

	if (g_Battle.fRepeat)
	{
		g_InputState.dwKeyPress = kKeyRepeat;
	}
	else if (g_Battle.fForce)
	{
		g_InputState.dwKeyPress = kKeyForce;
	}
	else if (g_Battle.fFlee)
	{
		g_InputState.dwKeyPress = kKeyFlee;
	}

	// Update the battle UI
	PAL_BattleUIUpdate();

#endif
	}

VOID PAL_BattleCommitAction(BOOL fRepeat)
/*++
  功能：

  Commit the action which the player decided.

  参数：

  [IN]  fRepeat - TRUE if repeat the last action.

  返回值：

  None.

  --*/
{
	WORD      w;

	if (!fRepeat)
	{
		g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.ActionType =
			g_Battle.UI.wActionType;
		g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.sTarget =
			(SHORT)g_Battle.UI.wSelectedIndex;
		g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.wActionID =
			g_Battle.UI.wObjectID;
	}
	else if (g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.ActionType == kBattleActionPass)
	{
		g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.ActionType = kBattleActionAttack;
		g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.sTarget = -1;
	}

	//
	// Check if the action is valid
	//
	switch (g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.ActionType)
	{
		case kBattleActionMagic:
			w = g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.wActionID;
			w = gpGlobals->g.lprgMagic[gpGlobals->g.rgObject[w].magic.wMagicNumber].wCostMP;

			if (gpGlobals->g.PlayerRoles.rgwMP[gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole] < w)
			{
				w = g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.wActionID;
				w = gpGlobals->g.lprgMagic[gpGlobals->g.rgObject[w].magic.wMagicNumber].wType;
				if (w == kMagicTypeApplyToPlayer || w == kMagicTypeApplyToParty ||
					w == kMagicTypeTrance)
				{
					g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.ActionType = kBattleActionDefend;
				}
				else
				{
					g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.ActionType = kBattleActionAttack;
					if (g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.sTarget == -1)
					{
						g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.sTarget = 0;
					}
				}
			}
			break;

#ifdef PAL_CLASSIC
		case kBattleActionUseItem:
			if ((gpGlobals->g.rgObject[g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.wActionID].item.wFlags & kItemFlagConsuming) == 0)
			{
				break;
			}

		case kBattleActionThrowItem:
			for (w = 0; w < MAX_INVENTORY; w++)
			{
				if (gpGlobals->rgInventory[w].wItem == g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.wActionID)
				{
					gpGlobals->rgInventory[w].nAmountInUse++;
					break;
				}
			}
			break;
#endif

		default:
			break;
	}

#ifndef PAL_CLASSIC
	//
	// Calculate the waiting time for the action
	//
	switch (g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.ActionType)
	{
		case kBattleActionMagic:
		{
			LPMAGIC      p;
			WORD         wCostMP;

			//
			// The base casting time of magic is set to the MP costed
			//
			w = g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.wActionID;
			p = &(gpGlobals->g.lprgMagic[gpGlobals->g.rgObject[w].magic.wMagicNumber]);
			wCostMP = p->wCostMP;

			if (wCostMP == 1)
			{
				if (p->wType == kMagicTypeSummon)
				{
					//
					// The Wine God is an ultimate move which should take long
					//
					wCostMP = 175;
				}
			}
			else if (p->wType == kMagicTypeApplyToPlayer || p->wType == kMagicTypeApplyToParty ||
				p->wType == kMagicTypeTrance)
			{
				//
				// Healing magics should take shorter
				//
				wCostMP /= 3;
			}

			g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.flRemainingTime = wCostMP + 5;
		}
		break;

		case kBattleActionAttack:
		case kBattleActionFlee:
		case kBattleActionUseItem:
		case kBattleActionThrowItem:
		default:
			//
			// Other actions take no time
			//
			g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.flRemainingTime = 0;
			break;
	}
#else
	if (g_Battle.UI.wActionType == kBattleActionFlee)
	{
		g_Battle.fFlee = TRUE;
	}
#endif

	g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].state = kFighterAct;
	g_Battle.UI.state = kBattleUIWait;

#ifndef PAL_CLASSIC
	if (g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.flRemainingTime <= 0)
	{
		SHORT sMax = -1;

		for (w = 0; w <= gpGlobals->wMaxPartyMemberIndex; w++)
		{
			if (g_Battle.rgPlayer[w].sTurnOrder > sMax)
			{
				sMax = g_Battle.rgPlayer[w].sTurnOrder;
			}
		}

		g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].sTurnOrder = sMax + 1;
	}
	else
	{
		g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].sTurnOrder = -1;
}
#endif
}

static VOID
PAL_BattleShowPlayerAttackAnim(
	WORD		wPlayerIndex,
	BOOL		fCritical
	)
	/*++
	  功能：  Show the physical attack effect for player.
	  参数：  [IN]  wPlayerIndex - the index of the player.
	  [IN]		fCritical - TRUE if this is a critical hit.
	  返回值：  None.
	  --*/
{
	WORD wPlayerRole = gpGlobals->rgParty[wPlayerIndex].wPlayerRole;
	SHORT sTarget = g_Battle.rgPlayer[wPlayerIndex].action.sTarget;

	int index, i, j;
	int enemy_x = 0, enemy_y = 0, enemy_h = 0, x, y, dist = 0;

	DWORD dwTime;

	if (sTarget != -1)
	{
		enemy_x = PAL_X(g_Battle.rgEnemy[sTarget].pos);
		enemy_y = PAL_Y(g_Battle.rgEnemy[sTarget].pos);

		enemy_h = PAL_RLEGetHeight(PAL_SpriteGetFrame(g_Battle.rgEnemy[sTarget].lpSprite, g_Battle.rgEnemy[sTarget].wCurrentFrame));

		if (sTarget >= 3)
		{
			dist = (sTarget - wPlayerIndex) * 8;
		}
	}
	else
	{
		enemy_x = 150;
		enemy_y = 100;
	}

	index = gpGlobals->g.rgwBattleEffectIndex[PAL_GetPlayerBattleSprite(wPlayerRole)][1];
	index *= 3;

	//
	// Play the attack voice
	//
	if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] > 0)
	{
		if (!fCritical)
		{
			SOUND_Play(gpGlobals->g.PlayerRoles.rgwAttackSound[wPlayerRole]);
		}
		else
		{
			SOUND_Play(gpGlobals->g.PlayerRoles.rgwCriticalSound[wPlayerRole]);
		}
	}

	//
	// Show the animation
	//
	x = enemy_x - dist + 64;
	y = enemy_y + dist + 20;

	g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 8;
	g_Battle.rgPlayer[wPlayerIndex].pos = PAL_XY(x, y);

	PAL_BattleDelay(2, 0, TRUE);

	x -= 10;
	y -= 2;
	g_Battle.rgPlayer[wPlayerIndex].pos = PAL_XY(x, y);

	PAL_BattleDelay(1, 0, TRUE);

	g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 9;
	x -= 16;
	y -= 4;

	SOUND_Play(gpGlobals->g.PlayerRoles.rgwWeaponSound[wPlayerRole]);

	x = enemy_x;
	y = enemy_y - enemy_h / 3 + 10;

	dwTime = SDL_GetTicks() + BATTLE_FRAME_TIME;

	for (i = 0; i < 3; i++)
	{
		LPCBITMAPRLE b = PAL_SpriteGetFrame(g_Battle.lpEffectSprite, index++);

		//
		// Wait for the time of one frame. Accept input here.
		//
		PAL_ProcessEvent();
		while (SDL_GetTicks() <= dwTime)
		{
			PAL_ProcessEvent();
			SDL_Delay(1);
		}

		//
		// Set the time of the next frame.
		//
		dwTime = SDL_GetTicks() + BATTLE_FRAME_TIME;

		//
		// Update the gesture of enemies.
		//
		for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
		{
			if (g_Battle.rgEnemy[j].wObjectID == 0 ||
				g_Battle.rgEnemy[j].rgwStatus[kStatusSleep] > 0 ||
				g_Battle.rgEnemy[j].rgwStatus[kStatusParalyzed] > 0)
			{
				continue;
			}

			if (--g_Battle.rgEnemy[j].e.wIdleAnimSpeed == 0)
			{
				g_Battle.rgEnemy[j].wCurrentFrame++;
				g_Battle.rgEnemy[j].e.wIdleAnimSpeed =
					gpGlobals->g.lprgEnemy[gpGlobals->g.rgObject[g_Battle.rgEnemy[j].wObjectID].enemy.wEnemyID].wIdleAnimSpeed;
			}

			if (g_Battle.rgEnemy[j].wCurrentFrame >= g_Battle.rgEnemy[j].e.wIdleFrames)
			{
				g_Battle.rgEnemy[j].wCurrentFrame = 0;
			}
		}

		PAL_BattleMakeScene();
		SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);

		PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
		x -= 16;
		y += 16;

		PAL_BattleUIUpdate();

		if (i == 0)
		{
			if (sTarget == -1)
			{
				for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
				{
					g_Battle.rgEnemy[j].iColorShift = 6;
				}
			}
			else
			{
				g_Battle.rgEnemy[sTarget].iColorShift = 6;
			}

			PAL_BattleDisplayStatChange();
			PAL_BattleBackupStat();
		}

		VIDEO_UpdateScreen(NULL);

		if (i == 1)
		{
			g_Battle.rgPlayer[wPlayerIndex].pos =
				PAL_XY(PAL_X(g_Battle.rgPlayer[wPlayerIndex].pos) + 2,
					PAL_Y(g_Battle.rgPlayer[wPlayerIndex].pos) + 1);
		}
	}

	dist = 8;

	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		g_Battle.rgEnemy[i].iColorShift = 0;
	}

	if (sTarget == -1)
	{
		for (i = 0; i < 3; i++)
		{
			for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
			{
				x = PAL_X(g_Battle.rgEnemy[j].pos);
				y = PAL_Y(g_Battle.rgEnemy[j].pos);

				x -= dist;
				y -= dist / 2;
				g_Battle.rgEnemy[j].pos = PAL_XY(x, y);
			}

			PAL_BattleDelay(1, 0, TRUE);
			dist /= -2;
		}
	}
	else
	{
		x = PAL_X(g_Battle.rgEnemy[sTarget].pos);
		y = PAL_Y(g_Battle.rgEnemy[sTarget].pos);

		for (i = 0; i < 3; i++)
		{
			x -= dist;
			dist /= -2;
			y += dist;
			g_Battle.rgEnemy[sTarget].pos = PAL_XY(x, y);

			PAL_BattleDelay(1, 0, TRUE);
		}
	}
}

static VOID
PAL_BattleShowPlayerUseItemAnim(
	WORD         wPlayerIndex,
	WORD         wObjectID,
	SHORT        sTarget
	)
	/*++
	  功能：

	  Show the "use item" effect for player.

	  参数：

	  [IN]  wPlayerIndex - the index of the player.

	  [IN]  wObjectID - the object ID of the item to be used.

	  [IN]  sTarget - the target player of the action.

	  返回值：

	  None.

	  --*/
{
	int i, j;

	PAL_BattleDelay(4, 0, TRUE);

	g_Battle.rgPlayer[wPlayerIndex].pos =
		PAL_XY(PAL_X(g_Battle.rgPlayer[wPlayerIndex].pos) - 15,
			PAL_Y(g_Battle.rgPlayer[wPlayerIndex].pos) - 7);

	g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 5;

	SOUND_Play(28);

	for (i = 0; i <= 6; i++)
	{
		if (sTarget == -1)
		{
			for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
			{
				g_Battle.rgPlayer[j].iColorShift = i;
			}
		}
		else
		{
			g_Battle.rgPlayer[sTarget].iColorShift = i;
		}

		PAL_BattleDelay(1, wObjectID, TRUE);
	}

	for (i = 5; i >= 0; i--)
	{
		if (sTarget == -1)
		{
			for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
			{
				g_Battle.rgPlayer[j].iColorShift = i;
			}
		}
		else
		{
			g_Battle.rgPlayer[sTarget].iColorShift = i;
		}

		PAL_BattleDelay(1, wObjectID, TRUE);
	}
}

#ifdef PAL_WIN95
VOID
PAL_BattleShowPlayerPreMagicAnim(
	WORD         wPlayerIndex,
	WORD         wObjectID
	)
#else
VOID
PAL_BattleShowPlayerPreMagicAnim(
	WORD         wPlayerIndex,
	BOOL         fSummon
	)
#endif
	/*++
	  功能：    Show the effect for player before using a magic.
	  参数：    [IN]  wPlayerIndex - the index of the player.
	  [IN]  fSummon - TRUE if player is using a summon magic.
	  返回值：    None.
	  --*/
{
	int   i, j;
	DWORD dwTime = SDL_GetTicks();
	WORD  wPlayerRole = gpGlobals->rgParty[wPlayerIndex].wPlayerRole;
#ifdef PAL_WIN95
	BOOL  fSummon = FALSE;
	int   iMagicNum = gpGlobals->g.rgObject[wObjectID].magic.wMagicNumber;

	if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeSummon)
		fSummon = TRUE;
#endif

	for (i = 0; i < 4; i++)
	{
		g_Battle.rgPlayer[wPlayerIndex].pos =
			PAL_XY(PAL_X(g_Battle.rgPlayer[wPlayerIndex].pos) - (4 - i),
				PAL_Y(g_Battle.rgPlayer[wPlayerIndex].pos) - (4 - i) / 2);

		PAL_BattleDelay(1, 0, TRUE);
	}

	PAL_BattleDelay(2, 0, TRUE);

	g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 5;
#ifndef PAL_WIN95
	SOUND_Play(gpGlobals->g.PlayerRoles.rgwMagicSound[wPlayerRole]);
#endif

	if (!fSummon)
	{
		int x, y, index;

		x = PAL_X(g_Battle.rgPlayer[wPlayerIndex].pos);
		y = PAL_Y(g_Battle.rgPlayer[wPlayerIndex].pos);

		index = gpGlobals->g.rgwBattleEffectIndex[PAL_GetPlayerBattleSprite(wPlayerRole)][0];
		index *= 10;
		index += 15;
#ifdef PAL_WIN95
		SOUND_Play(gpGlobals->g.PlayerRoles.rgwMagicSound[wPlayerRole]);
#endif
		for (i = 0; i < 10; i++)
		{
			LPCBITMAPRLE b = PAL_SpriteGetFrame(g_Battle.lpEffectSprite, index++);

			//
			// Wait for the time of one frame. Accept input here.
			//
			PAL_ProcessEvent();
			while (SDL_GetTicks() <= dwTime)
			{
				PAL_ProcessEvent();
				SDL_Delay(1);
			}

			//
			// Set the time of the next frame.
			//
			dwTime = SDL_GetTicks() + BATTLE_FRAME_TIME;

			//
			// Update the gesture of enemies.
			//
			for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
			{
				if (g_Battle.rgEnemy[j].wObjectID == 0 ||
					g_Battle.rgEnemy[j].rgwStatus[kStatusSleep] != 0 ||
					g_Battle.rgEnemy[j].rgwStatus[kStatusParalyzed] != 0)
				{
					continue;
				}

				if (--g_Battle.rgEnemy[j].e.wIdleAnimSpeed == 0)
				{
					g_Battle.rgEnemy[j].wCurrentFrame++;
					g_Battle.rgEnemy[j].e.wIdleAnimSpeed =
						gpGlobals->g.lprgEnemy[gpGlobals->g.rgObject[g_Battle.rgEnemy[j].wObjectID].enemy.wEnemyID].wIdleAnimSpeed;
				}

				if (g_Battle.rgEnemy[j].wCurrentFrame >= g_Battle.rgEnemy[j].e.wIdleFrames)
				{
					g_Battle.rgEnemy[j].wCurrentFrame = 0;
				}
			}

			PAL_BattleMakeScene();
			SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);

			PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

			PAL_BattleUIUpdate();

			VIDEO_UpdateScreen(NULL);
		}
	}

	PAL_BattleDelay(1, 0, TRUE);
}

static VOID
PAL_BattleShowPlayerDefMagicAnim(
	WORD         wPlayerIndex,
	WORD         wObjectID,
	SHORT        sTarget
	)
	/*++
	  功能：    Show the defensive magic effect for player.

	  参数：    [IN]  wPlayerIndex - the index of the player.
	  [IN]  wObjectID - the object ID of the magic to be used.
	  [IN]  sTarget - the target player of the action.
	  返回值：    None.
	  --*/
{
	LPSPRITE   lpSpriteEffect;
	int        l, iMagicNum, iEffectNum, n, i, j, x, y;
	DWORD      dwTime = SDL_GetTicks();

	iMagicNum = gpGlobals->g.rgObject[wObjectID].magic.wMagicNumber;
	iEffectNum = gpGlobals->g.lprgMagic[iMagicNum].wEffect;

	l = PAL_MKFGetDecompressedSize(iEffectNum, gpGlobals->f.fpFIRE);
	if (l <= 0)
	{
		return;
	}

	lpSpriteEffect = (LPSPRITE)UTIL_malloc(l);

	PAL_MKFDecompressChunk((LPBYTE)lpSpriteEffect, l, iEffectNum, gpGlobals->f.fpFIRE);

	n = PAL_SpriteGetNumFrames(lpSpriteEffect);

	g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 6;
	PAL_BattleDelay(1, 0, TRUE);

	for (i = 0; i < n; i++)
	{
		LPCBITMAPRLE b = PAL_SpriteGetFrame(lpSpriteEffect, i);

		if (i == gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay)
		{
			SOUND_Play(gpGlobals->g.lprgMagic[iMagicNum].wSound);
		}

		//
		// Wait for the time of one frame. Accept input here.
		//
		PAL_ProcessEvent();
		while (SDL_GetTicks() <= dwTime)
		{
			PAL_ProcessEvent();
			SDL_Delay(1);
		}

		//
		// Set the time of the next frame.
		//
		dwTime = SDL_GetTicks() +
			((SHORT)(gpGlobals->g.lprgMagic[iMagicNum].wSpeed) + 5) * 10;

		PAL_BattleMakeScene();
		SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);

		if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeApplyToParty)
		{
			assert(sTarget == -1);

			for (l = 0; l <= gpGlobals->wMaxPartyMemberIndex; l++)
			{
				x = PAL_X(g_Battle.rgPlayer[l].pos);
				y = PAL_Y(g_Battle.rgPlayer[l].pos);

				x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
				y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

				PAL_RLEBlitToSurface(b, gpScreen,
					PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
			}
		}
		else if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeApplyToPlayer)
		{
			assert(sTarget != -1);

			x = PAL_X(g_Battle.rgPlayer[sTarget].pos);
			y = PAL_Y(g_Battle.rgPlayer[sTarget].pos);

			x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
			y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

			PAL_RLEBlitToSurface(b, gpScreen,
				PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

			//
			// Repaint the previous player
			//
			if (sTarget > 0 && g_Battle.iHidingTime == 0)
			{
				if (gpGlobals->rgPlayerStatus[gpGlobals->rgParty[sTarget - 1].wPlayerRole][kStatusConfused] == 0)
				{
					LPCBITMAPRLE p = PAL_SpriteGetFrame(g_Battle.rgPlayer[sTarget - 1].lpSprite, g_Battle.rgPlayer[sTarget - 1].wCurrentFrame);

					x = PAL_X(g_Battle.rgPlayer[sTarget - 1].pos);
					y = PAL_Y(g_Battle.rgPlayer[sTarget - 1].pos);

					x -= PAL_RLEGetWidth(p) / 2;
					y -= PAL_RLEGetHeight(p);

					PAL_RLEBlitToSurface(p, gpScreen, PAL_XY(x, y));
				}
			}
		}
		else
		{
			assert(FALSE);
		}

		PAL_BattleUIUpdate();

		VIDEO_UpdateScreen(NULL);
	}

	free(lpSpriteEffect);

	for (i = 0; i < 6; i++)
	{
		if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeApplyToParty)
		{
			for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
			{
				g_Battle.rgPlayer[j].iColorShift = i;
			}
		}
		else
		{
			g_Battle.rgPlayer[sTarget].iColorShift = i;
		}

		PAL_BattleDelay(1, 0, TRUE);
	}

	for (i = 6; i >= 0; i--)
	{
		if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeApplyToParty)
		{
			for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
			{
				g_Battle.rgPlayer[j].iColorShift = i;
			}
		}
		else
		{
			g_Battle.rgPlayer[sTarget].iColorShift = i;
		}

		PAL_BattleDelay(1, 0, TRUE);
	}
}

#ifndef PAL_WIN95
static VOID
PAL_BattleShowPlayerOffMagicAnim(
	WORD         wPlayerIndex,
	WORD         wObjectID,
	SHORT        sTarget
	)
#else
static VOID
PAL_BattleShowPlayerOffMagicAnim(
	WORD         wPlayerIndex,
	WORD         wObjectID,
	SHORT        sTarget,
	BOOL         fSummon
	)
#endif
	/*++
	  功能：    Show the offensive magic animation for player.
	  参数：    [IN]  wPlayerIndex - the index of the player.
	  [IN]  wObjectID - the object ID of the magic to be used.
	  [IN]  sTarget - the target enemy of the action.
	  返回值：    None.
	  --*/
{
	LPSPRITE   lpSpriteEffect;
	int        l, iMagicNum, iEffectNum, n, i, k, x, y, wave, blow;
	DWORD      dwTime = SDL_GetTicks();

#ifdef ROTATE_SOME_MAGIC_ANIM
	BOOL		bRotateMagicAnim = FALSE;
	CONST WORD	wRotateMagicObjectID[20] =
	{//0x015e, //灭绝一击
		//0x0167, //御剑伏魔
		//0x0175, //大咒蛇
		0x0185,	//血魔神功
		0x0188, //火龙掌
		0x018a, //魔掌天下
		0x018d, //群魔乱舞
};
#endif

	iMagicNum = gpGlobals->g.rgObject[wObjectID].magic.wMagicNumber;
	iEffectNum = gpGlobals->g.lprgMagic[iMagicNum].wEffect;

#ifdef ROTATE_SOME_MAGIC_ANIM
	for (i = 0; i < 20; i++)
	{
		if (wObjectID == 0)
		{
			break;
		}
		else if (wObjectID == wRotateMagicObjectID[i])
		{
			bRotateMagicAnim = TRUE;
			break;
}
	}
#endif

	l = PAL_MKFGetDecompressedSize(iEffectNum, gpGlobals->f.fpFIRE);
	if (l <= 0)
	{
		return;
	}

	lpSpriteEffect = (LPSPRITE)UTIL_malloc(l);

	PAL_MKFDecompressChunk((LPBYTE)lpSpriteEffect, l, iEffectNum, gpGlobals->f.fpFIRE);

	n = PAL_SpriteGetNumFrames(lpSpriteEffect);

#ifdef PAL_WIN95
	if (wPlayerIndex != (WORD)-1)
	{
		g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 6;
	}
#endif

	PAL_BattleDelay(1, 0, TRUE);

	l = n - gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
	l *= (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wEffectTimes;
	l += n;
	l += gpGlobals->g.lprgMagic[iMagicNum].wShake;

	wave = gpGlobals->wScreenWave;
	gpGlobals->wScreenWave += gpGlobals->g.lprgMagic[iMagicNum].wWave;

#ifdef PAL_WIN95
	if (!fSummon && gpGlobals->g.lprgMagic[iMagicNum].wSound != 0)
	{
		SOUND_Play(gpGlobals->g.lprgMagic[iMagicNum].wSound);
	}
#endif

	for (i = 0; i < l; i++)
	{
		LPCBITMAPRLE b;
#ifndef PAL_WIN95
		if (i == gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay && wPlayerIndex != (WORD)-1)
		{
			g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 6;
		}
#endif
		blow = ((g_Battle.iBlow > 0) ? RandomLong(0, g_Battle.iBlow) : RandomLong(g_Battle.iBlow, 0));

		for (k = 0; k <= g_Battle.wMaxEnemyIndex; k++)
		{
			if (g_Battle.rgEnemy[k].wObjectID == 0)
			{
				continue;
			}

			x = PAL_X(g_Battle.rgEnemy[k].pos) + blow;
			y = PAL_Y(g_Battle.rgEnemy[k].pos) + blow / 2;

			g_Battle.rgEnemy[k].pos = PAL_XY(x, y);
		}

		if (l - i > gpGlobals->g.lprgMagic[iMagicNum].wShake)
		{
			if (i < n)
			{
				k = i;
			}
			else
			{
				k = i - gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
				k %= n - gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
				k += gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
			}

			b = PAL_SpriteGetFrame(lpSpriteEffect, k);

#ifndef PAL_WIN95
			if ((i - gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay) % n == 0)
			{
				SOUND_Play(gpGlobals->g.lprgMagic[iMagicNum].wSound);
			}
#endif
		}
		else
		{
			VIDEO_ShakeScreen(i, 3);
			b = PAL_SpriteGetFrame(lpSpriteEffect, (l - gpGlobals->g.lprgMagic[iMagicNum].wShake - 1) % n);
		}

		//
		// Wait for the time of one frame. Accept input here.
		//
		PAL_ProcessEvent();
		while (SDL_GetTicks() <= dwTime)
		{
			PAL_ProcessEvent();
			SDL_Delay(1);
		}

		//
		// Set the time of the next frame.
		//
		dwTime = SDL_GetTicks() +
			((SHORT)(gpGlobals->g.lprgMagic[iMagicNum].wSpeed) + 5) * 10;

		PAL_BattleMakeScene();
		SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);

		if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeNormal)
		{//普通仙术
			assert(sTarget != -1);

			x = PAL_X(g_Battle.rgEnemy[sTarget].pos);
			y = PAL_Y(g_Battle.rgEnemy[sTarget].pos);

			x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
			y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

#ifdef ROTATE_SOME_MAGIC_ANIM
			if (bRotateMagicAnim == FALSE)
			{
				PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
			}
			else
			{
				PAL_NEW_RLEBlitToSurfaceRotately(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
		}
#else
			PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
#endif

			if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
				gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
			{

#ifdef ROTATE_SOME_MAGIC_ANIM
				if (bRotateMagicAnim == FALSE)
				{
					PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
						PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
				}
				else
				{
					PAL_NEW_RLEBlitToSurfaceRotately(b, g_Battle.lpBackground,
						PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
	}
#else
				PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
					PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
#endif

			}
		}
		else if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackAll)
		{
			const int effectpos[3][2] = {{70, 140}, {100, 110}, {160, 100}};

			assert(sTarget == -1);

			for (k = 0; k < 3; k++)
			{
				x = effectpos[k][0];
				y = effectpos[k][1];

				x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
				y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

#ifdef ROTATE_SOME_MAGIC_ANIM
				if (bRotateMagicAnim == FALSE)
				{
					PAL_RLEBlitToSurface(b, gpScreen,
						PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
				}
				else
				{
					PAL_NEW_RLEBlitToSurfaceRotately(b, gpScreen,
						PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
			}
#else
				PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
#endif
				if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
					gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
				{

#ifdef ROTATE_SOME_MAGIC_ANIM
					if (bRotateMagicAnim == FALSE)
					{
						PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
							PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
					}
					else
					{
						PAL_NEW_RLEBlitToSurfaceRotately(b, g_Battle.lpBackground,
							PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
		}
#else
					PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
						PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
#endif

				}
			}
		}
		else if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackWhole ||
			gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackField)
		{
			assert(sTarget == -1);

			if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackWhole)
			{
				x = 120;
				y = 100;
			}
			else
			{
				x = 160;
				y = 200;
			}

			x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
			y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

#ifdef ROTATE_SOME_MAGIC_ANIM
			if (bRotateMagicAnim == FALSE)
			{
				PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
			}
			else
			{
				PAL_NEW_RLEBlitToSurfaceRotately(b, gpScreen,
					PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
		}
#else
			PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
#endif



			if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
				gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
			{

#ifdef ROTATE_SOME_MAGIC_ANIM
				if (bRotateMagicAnim == FALSE)
				{
					PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
						PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
				}
				else
				{
					PAL_NEW_RLEBlitToSurfaceRotately(b, g_Battle.lpBackground,
						PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
				}
#else
				PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
					PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
#endif

			}
		}
		else
		{
			assert(FALSE);
		}

		PAL_BattleUIUpdate();

		VIDEO_UpdateScreen(NULL);
	}

	gpGlobals->wScreenWave = wave;
	VIDEO_ShakeScreen(0, 0);

	free(lpSpriteEffect);

	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		g_Battle.rgEnemy[i].pos = g_Battle.rgEnemy[i].posOriginal;
	}
}

INT PAL_New_GetAliveEnemyNum()
{
	int i, num;
	for (i = 0, num = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID != 0 && g_Battle.rgEnemy[i].dwActualHealth != 0)
		{
			num++;
		}
	}
	return num;
}

static VOID PAL_BattleShowEnemyMagicAnim(WORD wObjectID, SHORT sTarget)
/*++
  功能：    Show the offensive magic animation for enemy.
  参数：    [IN]  wObjectID - the object ID of the magic to be used.
  [IN]  sTarget - the target player index of the action.
  返回值：    None.
  --*/
{
	LPSPRITE   lpSpriteEffect;
	int        l, iMagicNum, iEffectNum, n, i, k, x, y, wave, blow;
	DWORD      dwTime = SDL_GetTicks();

	iMagicNum = gpGlobals->g.rgObject[wObjectID].magic.wMagicNumber;
	iEffectNum = gpGlobals->g.lprgMagic[iMagicNum].wEffect;

	l = PAL_MKFGetDecompressedSize(iEffectNum, gpGlobals->f.fpFIRE);
	if (l <= 0)
	{
		return;
	}

	lpSpriteEffect = (LPSPRITE)UTIL_malloc(l);

	PAL_MKFDecompressChunk((LPBYTE)lpSpriteEffect, l, iEffectNum, gpGlobals->f.fpFIRE);

	n = PAL_SpriteGetNumFrames(lpSpriteEffect);

	l = n - gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
	l *= (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wEffectTimes;
	l += n;
	l += gpGlobals->g.lprgMagic[iMagicNum].wShake;

	wave = gpGlobals->wScreenWave;
	gpGlobals->wScreenWave += gpGlobals->g.lprgMagic[iMagicNum].wWave;

	for (i = 0; i < l; i++)
	{
		LPCBITMAPRLE b;

		blow = ((g_Battle.iBlow > 0) ? RandomLong(0, g_Battle.iBlow) : RandomLong(g_Battle.iBlow, 0));

		for (k = 0; k <= gpGlobals->wMaxPartyMemberIndex; k++)
		{
			x = PAL_X(g_Battle.rgPlayer[k].pos) + blow;
			y = PAL_Y(g_Battle.rgPlayer[k].pos) + blow / 2;

			g_Battle.rgPlayer[k].pos = PAL_XY(x, y);
		}

		if (l - i > gpGlobals->g.lprgMagic[iMagicNum].wShake)
		{
			if (i < n)
			{
				k = i;
			}
			else
			{
				k = i - gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
				k %= n - gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
				k += gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay;
			}

			b = PAL_SpriteGetFrame(lpSpriteEffect, k);

			if (i == gpGlobals->g.lprgMagic[iMagicNum].wSoundDelay)
			{
				SOUND_Play(gpGlobals->g.lprgMagic[iMagicNum].wSound);
			}
		}
		else
		{
			VIDEO_ShakeScreen(i, 3);
			b = PAL_SpriteGetFrame(lpSpriteEffect, (l - gpGlobals->g.lprgMagic[iMagicNum].wShake - 1) % n);
		}

		//
		// Wait for the time of one frame. Accept input here.
		//
		PAL_ProcessEvent();
		while (SDL_GetTicks() <= dwTime)
		{
			PAL_ProcessEvent();
			SDL_Delay(1);
		}

		//
		// Set the time of the next frame.
		//
		dwTime = SDL_GetTicks() +
			((SHORT)(gpGlobals->g.lprgMagic[iMagicNum].wSpeed) + 5) * 10;

		PAL_BattleMakeScene();
		SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);

		if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeNormal)
		{
			assert(sTarget != -1);

			x = PAL_X(g_Battle.rgPlayer[sTarget].pos);
			y = PAL_Y(g_Battle.rgPlayer[sTarget].pos);

			x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
			y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

			PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

			if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
				gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
			{
				PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
					PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
			}
		}
		else if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackAll)
		{
			const int effectpos[3][2] = {{180, 180}, {234, 170}, {270, 146}};

			assert(sTarget == -1);

			for (k = 0; k < 3; k++)
			{
				x = effectpos[k][0];
				y = effectpos[k][1];

				x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
				y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

				PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

				if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
					gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
				{
					PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
						PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
				}
			}
		}
		else if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackWhole ||
			gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackField)
		{
			assert(sTarget == -1);

			if (gpGlobals->g.lprgMagic[iMagicNum].wType == kMagicTypeAttackWhole)
			{
				x = 240;
				y = 150;
			}
			else
			{
				x = 160;
				y = 200;
			}

			x += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wXOffset;
			y += (SHORT)gpGlobals->g.lprgMagic[iMagicNum].wYOffset;

			PAL_RLEBlitToSurface(b, gpScreen, PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));

			if (i == l - 1 && gpGlobals->wScreenWave < 9 &&
				gpGlobals->g.lprgMagic[iMagicNum].wKeepEffect == 0xFFFF)
			{
				PAL_RLEBlitToSurface(b, g_Battle.lpBackground,
					PAL_XY(x - PAL_RLEGetWidth(b) / 2, y - PAL_RLEGetHeight(b)));
			}
		}
		else
		{
			assert(FALSE);
		}

		PAL_BattleUIUpdate();

		VIDEO_UpdateScreen(NULL);
	}

	gpGlobals->wScreenWave = wave;
	VIDEO_ShakeScreen(0, 0);

	free(lpSpriteEffect);

	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		g_Battle.rgPlayer[i].pos = g_Battle.rgPlayer[i].posOriginal;
	}
}

static VOID PAL_BattleShowPlayerSummonMagicAnim(WORD wPlayerIndex, WORD wObjectID)
/*++
  功能：    Show the summon magic animation for player.
  参数：    [IN]  wPlayerIndex - the index of the player.
  [IN]  wObjectID - the object ID of the magic to be used.
  返回值：    None.
  --*/
{
	int           i, j;
	WORD          wMagicNum = gpGlobals->g.rgObject[wObjectID].magic.wMagicNumber;
	WORD          wEffectMagicID = 0;
	DWORD         dwTime = SDL_GetTicks();

	for (wEffectMagicID = 0; wEffectMagicID < MAX_OBJECTS; wEffectMagicID++)
	{
		if (gpGlobals->g.rgObject[wEffectMagicID].magic.wMagicNumber ==
			gpGlobals->g.lprgMagic[wMagicNum].wEffect)
		{
			break;
		}
	}

	assert(wEffectMagicID < MAX_OBJECTS);

	//
	// Brighten the players
	//
	for (i = 1; i <= 10; i++)
	{
		for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
		{
			g_Battle.rgPlayer[j].iColorShift = i;
		}

		PAL_BattleDelay(1, wObjectID, TRUE);
	}

	PAL_BattleBackupScene();

#ifdef PAL_WIN95
	SOUND_Play(gpGlobals->g.lprgMagic[wMagicNum].wSound);
#endif

	//
	// Load the sprite of the summoned god
	//
	j = gpGlobals->g.lprgMagic[wMagicNum].wSummonEffect + 10;
	i = PAL_MKFGetDecompressedSize(j, gpGlobals->f.fpF);

	g_Battle.lpSummonSprite = UTIL_malloc(i);

	PAL_MKFDecompressChunk(g_Battle.lpSummonSprite, i, j, gpGlobals->f.fpF);

	g_Battle.iSummonFrame = 0;
	g_Battle.posSummon = PAL_XY(230 + (SHORT)(gpGlobals->g.lprgMagic[wMagicNum].wXOffset),
		155 + (SHORT)(gpGlobals->g.lprgMagic[wMagicNum].wYOffset));
	g_Battle.sBackgroundColorShift = (SHORT)(gpGlobals->g.lprgMagic[wMagicNum].wEffectTimes);

	//
	// Fade in the summoned god
	//
	PAL_BattleMakeScene();
	PAL_BattleFadeScene();

	//
	// Show the animation of the summoned god
	// TODO: There is still something missing here compared to the original game.
	//
	while (g_Battle.iSummonFrame < PAL_SpriteGetNumFrames(g_Battle.lpSummonSprite) - 1)
	{
		//
		// Wait for the time of one frame. Accept input here.
		//
		PAL_ProcessEvent();
		while (SDL_GetTicks() <= dwTime)
		{
			PAL_ProcessEvent();
			SDL_Delay(1);
		}

		//
		// Set the time of the next frame.
		//
		dwTime = SDL_GetTicks() +
			((SHORT)(gpGlobals->g.lprgMagic[wMagicNum].wSpeed) + 5) * 10;

		PAL_BattleMakeScene();
		SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);

		PAL_BattleUIUpdate();

		VIDEO_UpdateScreen(NULL);

		g_Battle.iSummonFrame++;
	}

	//
	// Show the actual magic effect
	//
#ifdef PAL_WIN95
	PAL_BattleShowPlayerOffMagicAnim((WORD)-1, wEffectMagicID, -1, TRUE);
#else
	PAL_BattleShowPlayerOffMagicAnim((WORD)-1, wEffectMagicID, -1);
#endif
}

static VOID PAL_BattleShowPostMagicAnim(VOID)
/*++
  功能：    Show the post-magic animation.
  参数：    None
  返回值：    None.

  --*/
{
	int         i, j, x, y, dist = 8;
	PAL_POS     rgEnemyPosBak[MAX_ENEMIES_IN_TEAM];

	for (i = 0; i < MAX_ENEMIES_IN_TEAM; i++)
	{
		rgEnemyPosBak[i] = g_Battle.rgEnemy[i].pos;
	}

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
		{
			if (g_Battle.rgEnemy[j].dwActualHealth == g_Battle.rgEnemy[j].wPrevHP)
			{
				continue;
			}

			x = PAL_X(g_Battle.rgEnemy[j].pos);
			y = PAL_Y(g_Battle.rgEnemy[j].pos);

			x -= dist;
			y -= dist / 2;

			g_Battle.rgEnemy[j].pos = PAL_XY(x, y);

			g_Battle.rgEnemy[j].iColorShift = ((i == 1) ? 6 : 0);
		}

		PAL_BattleDelay(1, 0, TRUE);
		dist /= -2;
	}

	for (i = 0; i < MAX_ENEMIES_IN_TEAM; i++)
	{
		g_Battle.rgEnemy[i].pos = rgEnemyPosBak[i];
	}

	PAL_BattleDelay(1, 0, TRUE);
}

static VOID PAL_BattlePlayerValidateAction(WORD wPlayerIndex)
/*++
  功能：    Validate player's action, fallback to other action when needed.
  参数：    [IN]  wPlayerIndex - the index of the player.
  返回值：    None.
  --*/
{
	const WORD   wPlayerRole = gpGlobals->rgParty[wPlayerIndex].wPlayerRole;
	const WORD   wObjectID = g_Battle.rgPlayer[wPlayerIndex].action.wActionID;
	const SHORT  sTarget = g_Battle.rgPlayer[wPlayerIndex].action.sTarget;
	BOOL         fValid = TRUE, fToEnemy = FALSE;
	WORD         w;
	int          i;

	switch (g_Battle.rgPlayer[wPlayerIndex].action.ActionType)
	{
		case kBattleActionAttack:
			fToEnemy = TRUE;
			break;

		case kBattleActionPass:
			break;

		case kBattleActionDefend:
			break;

		case kBattleActionMagic:
		{
			//
			// Make sure player actually has the magic to be used
			//
			for (i = 0; i < MAX_PLAYER_MAGICS; i++)
			{
				if (gpGlobals->g.PlayerRoles.rgwMagic[i][wPlayerRole] == wObjectID)
				{
					break; // player has this magic
				}
			}

			if (i >= MAX_PLAYER_MAGICS)
			{
				fValid = FALSE;
			}

			w = gpGlobals->g.rgObject[wObjectID].magic.wMagicNumber;

			if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSilence] > 0)
			{
				//
				// Player is silenced
				//
				fValid = FALSE;
			}

			if (gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole] <
				gpGlobals->g.lprgMagic[w].wCostMP)
			{
				//
				// No enough MP
				//
				fValid = FALSE;
			}

			//
			// Fallback to physical attack if player is using an offensive magic,
			// defend if player is using a defensive or healing magic
			//
			if (gpGlobals->g.rgObject[wObjectID].magic.wFlags & kMagicFlagUsableToEnemy)
			{
				if (!fValid)
				{
					g_Battle.rgPlayer[wPlayerIndex].action.ActionType = kBattleActionAttack;
				}
				else if (gpGlobals->g.rgObject[wObjectID].magic.wFlags & kMagicFlagApplyToAll)
				{
					g_Battle.rgPlayer[wPlayerIndex].action.sTarget = -1;
				}
				else if (sTarget == -1)
				{
					g_Battle.rgPlayer[wPlayerIndex].action.sTarget = PAL_BattleSelectAutoTarget();
				}

				fToEnemy = TRUE;
			}
			else
			{
				if (!fValid)
				{
					g_Battle.rgPlayer[wPlayerIndex].action.ActionType = kBattleActionDefend;
				}
				else if (gpGlobals->g.rgObject[wObjectID].magic.wFlags & kMagicFlagApplyToAll)
				{
					g_Battle.rgPlayer[wPlayerIndex].action.sTarget = -1;
				}
				else if (g_Battle.rgPlayer[wPlayerIndex].action.sTarget == -1)
				{
					g_Battle.rgPlayer[wPlayerIndex].action.sTarget = wPlayerIndex;
				}
			}
			break;
		}

		case kBattleActionCoopMagic:
		{
			fToEnemy = TRUE;

			for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				w = gpGlobals->rgParty[i].wPlayerRole;

#ifdef PAL_CLASSIC
				if (PAL_IsPlayerDying(w) || !PAL_New_IfPlayerCanMove(w))
#else
				if (PAL_IsPlayerDying(w) ||
					gpGlobals->rgPlayerStatus[w][kStatusSilence] > 0 ||
					gpGlobals->rgPlayerStatus[w][kStatusSleep] > 0 ||
					gpGlobals->rgPlayerStatus[w][kStatusConfused] > 0 ||
					g_Battle.rgPlayer[i].flTimeMeter < 100 ||
					(g_Battle.rgPlayer[i].state == kFighterAct && i != wPlayerIndex))
#endif
				{
					g_Battle.rgPlayer[wPlayerIndex].action.ActionType = kBattleActionAttack;
					break;
				}
			}

			if (g_Battle.rgPlayer[wPlayerIndex].action.ActionType == kBattleActionCoopMagic)
			{
				if (gpGlobals->g.rgObject[wObjectID].magic.wFlags & kMagicFlagApplyToAll)
				{
					g_Battle.rgPlayer[wPlayerIndex].action.sTarget = -1;
				}
				else if (sTarget == -1)
				{
					g_Battle.rgPlayer[wPlayerIndex].action.sTarget = PAL_BattleSelectAutoTarget();
				}
			}
			break;
		}

		case kBattleActionFlee:
			break;

		case kBattleActionThrowItem:
		{
			fToEnemy = TRUE;

			if (PAL_GetItemAmount(wObjectID) == 0)
			{
				g_Battle.rgPlayer[wPlayerIndex].action.ActionType = kBattleActionAttack;
			}
			else if (gpGlobals->g.rgObject[wObjectID].item.wFlags & kItemFlagApplyToAll)
			{
				g_Battle.rgPlayer[wPlayerIndex].action.sTarget = -1;
			}
			else if (g_Battle.rgPlayer[wPlayerIndex].action.sTarget == -1)
			{
				g_Battle.rgPlayer[wPlayerIndex].action.sTarget = PAL_BattleSelectAutoTarget();
			}
			break;
		}

		case kBattleActionUseItem:
		{
			if (PAL_GetItemAmount(wObjectID) == 0)
			{
				g_Battle.rgPlayer[wPlayerIndex].action.ActionType = kBattleActionDefend;
			}
			else if (gpGlobals->g.rgObject[wObjectID].item.wFlags & kItemFlagApplyToAll)
			{
				g_Battle.rgPlayer[wPlayerIndex].action.sTarget = -1;
			}
			else if (g_Battle.rgPlayer[wPlayerIndex].action.sTarget == -1)
			{
				g_Battle.rgPlayer[wPlayerIndex].action.sTarget = wPlayerIndex;
			}
			break;
		}

		case kBattleActionAttackMate:
		{
			if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusConfused] == 0)
			{
				//
				// Attack enemies instead if player is not confused
				//
				fToEnemy = TRUE;
				g_Battle.rgPlayer[wPlayerIndex].action.ActionType = kBattleActionAttack;
			}
			else
			{
				for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
				{
					if (i != wPlayerIndex &&
						gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] != 0)
					{
						break;
					}
				}

				if (i > gpGlobals->wMaxPartyMemberIndex)
				{
					//
					// Attack enemies if no one else is alive
					//
					fToEnemy = TRUE;
					g_Battle.rgPlayer[wPlayerIndex].action.ActionType = kBattleActionAttack;
				}
			}
			break;
		}
	}

	//
	// Check if player can attack all enemies at once, or attack one enemy
	//
	if (g_Battle.rgPlayer[wPlayerIndex].action.ActionType == kBattleActionAttack)
	{
		if (sTarget == -1)
		{
			if (!PAL_PlayerCanAttackAll(wPlayerRole))
			{
				g_Battle.rgPlayer[wPlayerIndex].action.sTarget = PAL_BattleSelectAutoTarget();
			}
		}
		else if (PAL_PlayerCanAttackAll(wPlayerRole))
		{
			g_Battle.rgPlayer[wPlayerIndex].action.sTarget = -1;
		}
	}

	if (fToEnemy && g_Battle.rgPlayer[wPlayerIndex].action.sTarget >= 0)
	{
		if (g_Battle.rgEnemy[g_Battle.rgPlayer[wPlayerIndex].action.sTarget].wObjectID == 0)
		{
			g_Battle.rgPlayer[wPlayerIndex].action.sTarget = PAL_BattleSelectAutoTarget();
			assert(g_Battle.rgPlayer[wPlayerIndex].action.sTarget >= 0);
		}
	}
}

VOID PAL_BattlePlayerPerformAction(WORD wPlayerIndex)
/*++
  功能：    Perform the selected action for a player.
  我方角色施展动作
  参数：    [IN]  wPlayerIndex - the index of the player.
  角色的序号
  返回值：    None.
  --*/
{
	SHORT    iDamage;
	WORD     wPlayerRole = gpGlobals->rgParty[wPlayerIndex].wPlayerRole;
	SHORT    sTarget;
	int      x, y;
	int      i, j, t;
	WORD     str, def, res, wObject, wMagicNum;
	BOOL     fCritical;
	WORD     rgwCoopPos[3][2] = {{208, 157}, {234, 170}, {260, 183}};
#ifndef PAL_CLASSIC
	BOOL     fPoisoned, fCheckPoison;
#endif

	g_Battle.wMovingPlayerIndex = wPlayerIndex;
	g_Battle.iBlow = 0;

	PAL_BattlePlayerValidateAction(wPlayerIndex);
	PAL_BattleBackupStat();

	sTarget = g_Battle.rgPlayer[wPlayerIndex].action.sTarget;

#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
	if (wPlayerRole == RoleID_ZhaoLingEr && PAL_New_IsPlayerUsingMengshe(wPlayerRole))
	{
		if (gpGlobals->rgPlayerStatus[RoleID_ZhaoLingEr][kStatusHaste] != 0)
		{
			g_Battle.MSCount_Zhao.Dexterity++;
		}
	}
#endif


	switch (g_Battle.rgPlayer[wPlayerIndex].action.ActionType)
	{
		case kBattleActionAttack:
		{
#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
			if (wPlayerRole == RoleID_ZhaoLingEr && PAL_New_IsPlayerUsingMengshe(wPlayerRole))
			{
				g_Battle.MSCount_Zhao.MaxHP++;
				g_Battle.MSCount_Zhao.AttackStrength++;
				if (gpGlobals->rgPlayerStatus[RoleID_ZhaoLingEr][kStatusBravery] != 0)
				{
					g_Battle.MSCount_Zhao.AttackStrength += RandomLong(0, 2);
				}
				if (gpGlobals->rgPlayerStatus[RoleID_ZhaoLingEr][kStatusDualAttack] != 0)
				{
					g_Battle.MSCount_Zhao.AttackStrength++;
				}
			}
#endif
			WORD wTimes = 1;
			if (sTarget != -1)
			{
				//
				// Attack one enemy
				//
				for (t = 0; t < (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusDualAttack] ? 2 : 1); t++)
				{
					str = PAL_GetPlayerAttackStrength(wPlayerRole);
					def = PAL_New_GetEnemyDefense(sTarget);
					res = PAL_New_GetEnemyPhysicalResistance(sTarget);
					fCritical = FALSE;

					iDamage = PAL_CalcPhysicalAttackDamage(str, def, res) + RandomLong(1, 2);

					if (PAL_New_GetTrueByPercentage(17) ||
						gpGlobals->rgPlayerStatus[wPlayerRole][kStatusBravery] > 0)
					{	// Critical Hit	// 普通暴击
						iDamage *= 3;
						fCritical = TRUE;
						gpGlobals->Exp.rgHealthExp[wPlayerRole].wCount += RandomLong(0, 1);
						gpGlobals->Exp.rgAttackExp[wPlayerRole].wCount += RandomLong(0, 1);
					}

					if (wPlayerRole == RoleID_LiXiaoYao && PAL_New_GetTrueByPercentage(8))
					{	// Bonus hit for Li Xiaoyao	//李逍遥，会心一击
						iDamage *= 2;
						fCritical = TRUE;
						gpGlobals->Exp.rgHealthExp[wPlayerRole].wCount += RandomLong(0, 2);
						gpGlobals->Exp.rgAttackExp[wPlayerRole].wCount += RandomLong(0, 2);
					}

					iDamage = (INT)(iDamage * RandomFloat(1, 1.125));
					iDamage = max(1, iDamage);
					iDamage = min(iDamage, g_Battle.rgEnemy[sTarget].dwActualHealth);
					g_Battle.rgEnemy[sTarget].dwActualHealth -= iDamage;

					if (t == 0)
					{
						g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 7;
						PAL_BattleDelay(4, 0, TRUE);
					}

					PAL_BattleShowPlayerAttackAnim(wPlayerIndex, fCritical);
				}
			}
			else
			{
				// Attack all enemies
				for (t = 0; t < (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusDualAttack] ? 2 : 1); t++)
				{
					int division = 1;
					const int index[MAX_ENEMIES_IN_TEAM] = {2, 1, 0, 4, 3};

					wTimes = 1;
					fCritical = FALSE;
					if (PAL_New_GetTrueByPercentage(17) ||
						gpGlobals->rgPlayerStatus[wPlayerRole][kStatusBravery] > 0)
					{	// Critical Hit	// 普通暴击
						wTimes *= 3;
						fCritical = TRUE;
						gpGlobals->Exp.rgHealthExp[wPlayerRole].wCount += RandomLong(0, 1);
						gpGlobals->Exp.rgAttackExp[wPlayerRole].wCount += RandomLong(0, 1);
					}

					if (wPlayerRole == RoleID_LiXiaoYao && PAL_New_GetTrueByPercentage(8))
					{	// Bonus hit for Li Xiaoyao	//李逍遥，会心一击
						wTimes *= 2;
						fCritical = TRUE;
						gpGlobals->Exp.rgHealthExp[wPlayerRole].wCount += RandomLong(0, 2);
						gpGlobals->Exp.rgAttackExp[wPlayerRole].wCount += RandomLong(0, 2);
					}

					if (t == 0)
					{
						g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 7;
						PAL_BattleDelay(4, 0, TRUE);
					}

					for (i = 0; i < MAX_ENEMIES_IN_TEAM; i++)
					{
						if (g_Battle.rgEnemy[index[i]].wObjectID == 0 ||
							index[i] > g_Battle.wMaxEnemyIndex)
						{
							continue;
						}

						str = PAL_GetPlayerAttackStrength(wPlayerRole);
						def = PAL_New_GetEnemyDefense(index[i]);
						res = PAL_New_GetEnemyPhysicalResistance(index[i]);

						iDamage = PAL_CalcPhysicalAttackDamage(str, def, res) + RandomLong(1, 2);
						iDamage = (INT)(iDamage * RandomFloat(1, 1.125));
						iDamage *= wTimes;
						iDamage /= division;
						iDamage = max(1, iDamage);
						iDamage = min(iDamage, g_Battle.rgEnemy[index[i]].dwActualHealth);
						g_Battle.rgEnemy[index[i]].dwActualHealth -= iDamage;

						division++;
						division = min(division, 3);
					}

					PAL_BattleShowPlayerAttackAnim(wPlayerIndex, fCritical);
				}
			}

			PAL_BattleUpdateFighters();
			PAL_BattleMakeScene();
			PAL_BattleDelay(3, 0, TRUE);

			gpGlobals->Exp.rgAttackExp[wPlayerRole].wCount++;
			gpGlobals->Exp.rgHealthExp[wPlayerRole].wCount += RandomLong(2, 3);

			break;
		}

		case kBattleActionAttackMate:
		{

#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
			if (wPlayerRole == RoleID_ZhaoLingEr && PAL_New_IsPlayerUsingMengshe(wPlayerRole))
			{
				g_Battle.MSCount_Zhao.AttackStrength += RandomLong(0, 1);
			}
#endif

			if (PAL_IsPlayerDying(wPlayerRole))
			{
				break;
			}

			//
			// Check if there is someone else who is alive
			//
			for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				if (i == wPlayerIndex)
				{
					continue;
				}

				if (gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] > 0)
				{
					break;
				}
			}

			if (i <= gpGlobals->wMaxPartyMemberIndex)
			{
				//
				// Pick a target randomly
				//
				do
				{
					sTarget = RandomLong(0, gpGlobals->wMaxPartyMemberIndex);
				} while (sTarget == wPlayerIndex || gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[sTarget].wPlayerRole] == 0);

				for (j = 0; j < 2; j++)
				{
					g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 8;
					PAL_BattleDelay(1, 0, TRUE);

					g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 0;
					PAL_BattleDelay(1, 0, TRUE);
				}

				PAL_BattleDelay(2, 0, TRUE);

				x = PAL_X(g_Battle.rgPlayer[sTarget].pos) + 30;
				y = PAL_Y(g_Battle.rgPlayer[sTarget].pos) + 12;

				g_Battle.rgPlayer[wPlayerIndex].pos = PAL_XY(x, y);
				g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 8;
				PAL_BattleDelay(5, 0, TRUE);

				g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 9;
				SOUND_Play(gpGlobals->g.PlayerRoles.rgwWeaponSound[wPlayerRole]);

				str = PAL_GetPlayerAttackStrength(wPlayerRole);
				def = PAL_GetPlayerDefense(gpGlobals->rgParty[sTarget].wPlayerRole);
				if (g_Battle.rgPlayer[sTarget].fDefending)
				{
					def *= 2;
				}

				iDamage = PAL_CalcPhysicalAttackDamage(str, def, 20 + 0.8 * PAL_New_GetPlayerPhysicalResistance(wPlayerRole));
				if (gpGlobals->rgPlayerStatus[gpGlobals->rgParty[sTarget].wPlayerRole][kStatusProtect] > 0)
				{
					iDamage /= 2;
				}

				iDamage = max(iDamage, 1);

#ifdef LIMIT_DAMAGE_WHEN_ATTACK_MATE
				iDamage = min(iDamage, 500);
#endif
				if (iDamage > (SHORT)gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[sTarget].wPlayerRole])
				{
					iDamage = gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[sTarget].wPlayerRole];
				}

				gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[sTarget].wPlayerRole] -= iDamage;

				g_Battle.rgPlayer[sTarget].pos =
					PAL_XY(PAL_X(g_Battle.rgPlayer[sTarget].pos) - 12,
						PAL_Y(g_Battle.rgPlayer[sTarget].pos) - 6);
				PAL_BattleDelay(1, 0, TRUE);

				g_Battle.rgPlayer[sTarget].iColorShift = 6;
				PAL_BattleDelay(1, 0, TRUE);

				PAL_BattleDisplayStatChange();

				g_Battle.rgPlayer[sTarget].iColorShift = 0;
				PAL_BattleDelay(4, 0, TRUE);

				PAL_BattleUpdateFighters();
				PAL_BattleDelay(4, 0, TRUE);
			}

			break;
		}

		case kBattleActionCoopMagic:
		{

#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
			if (wPlayerRole == RoleID_ZhaoLingEr && PAL_New_IsPlayerUsingMengshe(wPlayerRole))
			{
				g_Battle.MSCount_Zhao.MaxMP += RandomLong(1, 2);
				g_Battle.MSCount_Zhao.MagicStrength += RandomLong(1, 2);
				g_Battle.MSCount_Zhao.Dexterity++;
			}
#endif

			wObject = PAL_GetPlayerCooperativeMagic(gpGlobals->rgParty[wPlayerIndex].wPlayerRole);
			wMagicNum = gpGlobals->g.rgObject[wObject].magic.wMagicNumber;

			if (gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeSummon)
			{
#ifdef PAL_WIN95
				PAL_BattleShowPlayerPreMagicAnim(wPlayerIndex, wObject);
#else
				PAL_BattleShowPlayerPreMagicAnim(wPlayerIndex, TRUE);
#endif
				PAL_BattleShowPlayerSummonMagicAnim((WORD)-1, wObject);
		}
			else
			{
				for (i = 1; i <= 6; i++)
				{
					//
					// Update the position for the player who invoked the action
					//
					x = PAL_X(g_Battle.rgPlayer[wPlayerIndex].posOriginal) * (6 - i);
					y = PAL_Y(g_Battle.rgPlayer[wPlayerIndex].posOriginal) * (6 - i);

					x += rgwCoopPos[0][0] * i;
					y += rgwCoopPos[0][1] * i;

					x /= 6;
					y /= 6;

					g_Battle.rgPlayer[wPlayerIndex].pos = PAL_XY(x, y);

					//
					// Update the position for other players
					//
					t = 0;

					for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
					{
						if ((WORD)j == wPlayerIndex)
						{
							continue;
						}

						t++;

						x = PAL_X(g_Battle.rgPlayer[j].posOriginal) * (6 - i);
						y = PAL_Y(g_Battle.rgPlayer[j].posOriginal) * (6 - i);

						x += rgwCoopPos[t][0] * i;
						y += rgwCoopPos[t][1] * i;

						x /= 6;
						y /= 6;

						g_Battle.rgPlayer[j].pos = PAL_XY(x, y);
					}

					PAL_BattleDelay(1, 0, TRUE);
				}

				for (i = gpGlobals->wMaxPartyMemberIndex; i >= 0; i--)
				{
					if ((WORD)i == wPlayerIndex)
					{
						continue;
					}

					g_Battle.rgPlayer[i].wCurrentFrame = 5;

					PAL_BattleDelay(3, 0, TRUE);
				}

				g_Battle.rgPlayer[wPlayerIndex].iColorShift = 6;
				g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 5;
				SOUND_Play(157);
				PAL_BattleDelay(5, 0, TRUE);

				g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 6;
				g_Battle.rgPlayer[wPlayerIndex].iColorShift = 0;
				PAL_BattleDelay(3, 0, TRUE);

#ifdef PAL_WIN95
				PAL_BattleShowPlayerOffMagicAnim((WORD)-1, wObject, sTarget, FALSE);
#else
				PAL_BattleShowPlayerOffMagicAnim((WORD)-1, wObject, sTarget);
#endif
			}

			for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] -=
					gpGlobals->g.lprgMagic[wMagicNum].wCostMP;

				if ((SHORT)(gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole]) <= 0)
				{
					gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] = 1;
				}

				//
				// Reset the time meter for everyone when using coopmagic
				//
#ifdef PAL_CLASSIC
				g_Battle.rgPlayer[i].state = kFighterWait;
#else
				g_Battle.rgPlayer[i].flTimeMeter = 0;
				g_Battle.rgPlayer[i].flTimeSpeedModifier = 2;
#endif
			}

			PAL_BattleBackupStat(); // so that "damages" to players won't be shown

			str = 0;

			for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				str += PAL_GetPlayerAttackStrength(gpGlobals->rgParty[i].wPlayerRole);
				str += PAL_GetPlayerMagicStrength(gpGlobals->rgParty[i].wPlayerRole);
			}

			str /= 4;

			PAL_New_ApplyMagicDamageToEnemy(sTarget, str, wObject, TRUE);// Inflict damage to enemies
			PAL_BattleDisplayStatChange();
			PAL_BattleShowPostMagicAnim();
			PAL_BattleDelay(5, 0, TRUE);

			if (gpGlobals->g.lprgMagic[wMagicNum].wType != kMagicTypeSummon)
			{
				PAL_BattlePostActionCheck(FALSE);

				//
				// Move all players back to the original position
				//
				for (i = 1; i <= 6; i++)
				{
					//
					// Update the position for the player who invoked the action
					//
					x = PAL_X(g_Battle.rgPlayer[wPlayerIndex].posOriginal) * i;
					y = PAL_Y(g_Battle.rgPlayer[wPlayerIndex].posOriginal) * i;

					x += rgwCoopPos[0][0] * (6 - i);
					y += rgwCoopPos[0][1] * (6 - i);

					x /= 6;
					y /= 6;

					g_Battle.rgPlayer[wPlayerIndex].pos = PAL_XY(x, y);

					//
					// Update the position for other players
					//
					t = 0;

					for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
					{
						g_Battle.rgPlayer[j].wCurrentFrame = 0;

						if ((WORD)j == wPlayerIndex)
						{
							continue;
						}

						t++;

						x = PAL_X(g_Battle.rgPlayer[j].posOriginal) * i;
						y = PAL_Y(g_Battle.rgPlayer[j].posOriginal) * i;

						x += rgwCoopPos[t][0] * (6 - i);
						y += rgwCoopPos[t][1] * (6 - i);

						x /= 6;
						y /= 6;

						g_Battle.rgPlayer[j].pos = PAL_XY(x, y);
					}

					PAL_BattleDelay(1, 0, TRUE);
				}
			}
			break;
	}

		case kBattleActionDefend:
		{

#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
			if (wPlayerRole == RoleID_ZhaoLingEr && PAL_New_IsPlayerUsingMengshe(wPlayerRole))
			{
				g_Battle.MSCount_Zhao.Defense++;
			}
#endif

			g_Battle.rgPlayer[wPlayerIndex].fDefending = TRUE;
			gpGlobals->Exp.rgDefenseExp[wPlayerRole].wCount += 2;

			break;

		case kBattleActionFlee:

#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
			if (wPlayerRole == RoleID_ZhaoLingEr && PAL_New_IsPlayerUsingMengshe(wPlayerRole))
			{
				g_Battle.MSCount_Zhao.FleeRate += 2;
			}
#endif

			str = PAL_GetPlayerFleeRate(wPlayerRole);
			def = 0;

			for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
			{
				if (g_Battle.rgEnemy[i].wObjectID == 0)
				{
					continue;
				}

				def += (SHORT)(g_Battle.rgEnemy[i].e.wFleeRate);
#ifdef EDIT_DAMAGE_CALC
				def += g_Battle.rgEnemy[i].e.wLevel * 3;
#else
				def += (g_Battle.rgEnemy[i].e.wLevel + 6) * 2;
#endif
			}

			if ((SHORT)def < 0)
			{
				def = 0;
			}

			if (RandomLong(0, str) >= RandomLong(0, def) && !g_Battle.fIsBoss)
			{
				//
				// Successful escape
				//
				PAL_BattlePlayerEscape();
			}
			else
			{
				//
				// Failed escape
				//
				g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 0;

				for (i = 0; i < 3; i++)
				{
					x = PAL_X(g_Battle.rgPlayer[wPlayerIndex].pos) + 4;
					y = PAL_Y(g_Battle.rgPlayer[wPlayerIndex].pos) + 2;

					g_Battle.rgPlayer[wPlayerIndex].pos = PAL_XY(x, y);

					PAL_BattleDelay(1, 0, TRUE);
				}

				g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 1;
				PAL_BattleDelay(8, BATTLE_LABEL_ESCAPEFAIL, TRUE);

				gpGlobals->Exp.rgFleeExp[wPlayerRole].wCount += 2;
			}
			break;
		}

		case kBattleActionMagic:
		{

#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
			if (wPlayerRole == RoleID_ZhaoLingEr && PAL_New_IsPlayerUsingMengshe(wPlayerRole))
			{
				g_Battle.MSCount_Zhao.MaxMP++;
				g_Battle.MSCount_Zhao.MagicStrength += RandomLong(1, 2);
			}
#endif

			wObject = g_Battle.rgPlayer[wPlayerIndex].action.wActionID;
			wMagicNum = gpGlobals->g.rgObject[wObject].magic.wMagicNumber;

#ifdef PAL_WIN95
			PAL_BattleShowPlayerPreMagicAnim(wPlayerIndex, wObject);
#else
			PAL_BattleShowPlayerPreMagicAnim(wPlayerIndex,
				(gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeSummon));
#endif

			if (!gpGlobals->fAutoBattle)
			{
				gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole] -= gpGlobals->g.lprgMagic[wMagicNum].wCostMP;
				if ((SHORT)(gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole]) < 0)
				{
					gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole] = 0;
				}
			}

			if (gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeApplyToPlayer ||
				gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeApplyToParty ||
				gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeTrance)
			{
				//
				// Using a defensive magic
				//
				WORD w = 0;

				if (g_Battle.rgPlayer[wPlayerIndex].action.sTarget != -1)
				{
					w = gpGlobals->rgParty[g_Battle.rgPlayer[wPlayerIndex].action.sTarget].wPlayerRole;
				}
				else if (gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeTrance)
				{
					w = wPlayerRole;
				}

				gpGlobals->g.rgObject[wObject].magic.wScriptOnUse =
					PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].magic.wScriptOnUse, wPlayerRole);

				if (g_fScriptSuccess)
				{
					PAL_BattleShowPlayerDefMagicAnim(wPlayerIndex, wObject, sTarget);

					gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess =
						PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess, w);

					if (g_fScriptSuccess)
					{
						if (gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeTrance)
						{
							for (i = 0; i < 6; i++)
							{
								g_Battle.rgPlayer[wPlayerIndex].iColorShift = i * 2;
								PAL_BattleDelay(1, 0, TRUE);
							}

							PAL_BattleBackupScene();
							PAL_LoadBattleSprites();

							g_Battle.rgPlayer[wPlayerIndex].iColorShift = 0;

							PAL_BattleMakeScene();
							PAL_BattleFadeScene();
						}
					}
				}
			}
			else
			{
				//
				// Using an offensive magic
				//
				gpGlobals->g.rgObject[wObject].magic.wScriptOnUse =
					PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].magic.wScriptOnUse, wPlayerRole);

				if (g_fScriptSuccess)
				{
					if (gpGlobals->g.lprgMagic[wMagicNum].wType == kMagicTypeSummon)
					{
						PAL_BattleShowPlayerSummonMagicAnim(wPlayerIndex, wObject);
					}
					else
					{
#ifdef PAL_WIN95
						PAL_BattleShowPlayerOffMagicAnim(wPlayerIndex, wObject, sTarget, FALSE);
#else
						PAL_BattleShowPlayerOffMagicAnim(wPlayerIndex, wObject, sTarget);
#endif
				}

					if (sTarget == -1)
					{
#ifndef EDIT_SCRIT_EVENTOBJECTID_WHEN_ITIS_0XFFFF
						gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess =
							PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess, (WORD)sTarget);
#else
						gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess =
							PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess,
								gpGlobals->rgParty[wPlayerIndex].wPlayerRole);
#endif
			}
					else
					{
						gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess =
							PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].magic.wScriptOnSuccess, (WORD)sTarget);
					}
					str = PAL_GetPlayerMagicStrength(wPlayerRole);
					PAL_New_ApplyMagicDamageToEnemy(sTarget, str, wObject, TRUE);// Inflict damage to enemies
		}
}

			PAL_BattleDisplayStatChange();
			PAL_BattleShowPostMagicAnim();
			PAL_BattleDelay(5, 0, TRUE);

			gpGlobals->Exp.rgMagicExp[wPlayerRole].wCount += RandomLong(2, 3);
			gpGlobals->Exp.rgMagicPowerExp[wPlayerRole].wCount++;

			break;
		}

		case kBattleActionThrowItem:
		{

#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
			if (wPlayerRole == RoleID_ZhaoLingEr && PAL_New_IsPlayerUsingMengshe(wPlayerRole))
			{
				g_Battle.MSCount_Zhao.MaxHP += RandomLong(0, 1);
				g_Battle.MSCount_Zhao.AttackStrength++;
				g_Battle.MSCount_Zhao.Dexterity++;
			}
#endif

			wObject = g_Battle.rgPlayer[wPlayerIndex].action.wActionID;

			for (i = 0; i < 4; i++)
			{
				g_Battle.rgPlayer[wPlayerIndex].pos =
					PAL_XY(PAL_X(g_Battle.rgPlayer[wPlayerIndex].pos) - (4 - i),
						PAL_Y(g_Battle.rgPlayer[wPlayerIndex].pos) - (4 - i) / 2);

				PAL_BattleDelay(1, 0, TRUE);
			}

			PAL_BattleDelay(2, wObject, TRUE);

			g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 5;
			SOUND_Play(gpGlobals->g.PlayerRoles.rgwMagicSound[wPlayerRole]);

			PAL_BattleDelay(8, wObject, TRUE);

			g_Battle.rgPlayer[wPlayerIndex].wCurrentFrame = 6;
			PAL_BattleDelay(2, wObject, TRUE);

			//
			// Run the script
			//
			gpGlobals->g.rgObject[wObject].item.wScriptOnThrow =
				PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].item.wScriptOnThrow, (WORD)sTarget);

			//
			// Remove the thrown item from inventory
			//
			PAL_AddItemToInventory(wObject, -1);

			PAL_BattleDisplayStatChange();
			PAL_BattleDelay(4, 0, TRUE);
			PAL_BattleUpdateFighters();
			PAL_BattleDelay(4, 0, TRUE);

			break;
		}

		case kBattleActionUseItem:           //使用物品
		{

#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
			if (wPlayerRole == RoleID_ZhaoLingEr && PAL_New_IsPlayerUsingMengshe(wPlayerRole))
			{
				g_Battle.MSCount_Zhao.MaxHP++;
				g_Battle.MSCount_Zhao.MaxMP++;
			}
#endif

			wObject = g_Battle.rgPlayer[wPlayerIndex].action.wActionID;

			PAL_BattleShowPlayerUseItemAnim(wPlayerIndex, wObject, sTarget);

			//
			// Run the script
			//
			gpGlobals->g.rgObject[wObject].item.wScriptOnUse =
#ifndef EDIT_SCRIT_EVENTOBJECTID_WHEN_ITIS_0XFFFF
				PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].item.wScriptOnUse,
					(sTarget == -1) ? 0xFFFF : gpGlobals->rgParty[sTarget].wPlayerRole);
#else
				PAL_RunTriggerScript(gpGlobals->g.rgObject[wObject].item.wScriptOnUse,
					(sTarget == -1) ? gpGlobals->rgParty[wPlayerIndex].wPlayerRole : gpGlobals->rgParty[sTarget].wPlayerRole);
#endif
			//
			// Remove the item if the item is consuming
			//
			if (gpGlobals->g.rgObject[wObject].item.wFlags & kItemFlagConsuming)
			{
				PAL_AddItemToInventory(wObject, -1);
			}

			if (g_Battle.iHidingTime < 0)
			{
#ifdef PAL_CLASSIC
				g_Battle.iHidingTime = -g_Battle.iHidingTime;
#else
				g_Battle.iHidingTime = -g_Battle.iHidingTime * 20;

				if (gpGlobals->bBattleSpeed > 1)
				{
					g_Battle.iHidingTime *= 1 + (gpGlobals->bBattleSpeed - 1) * 0.5;
				}
				else
				{
					g_Battle.iHidingTime *= 1.2;
			}
#endif
				PAL_BattleBackupScene();
				PAL_BattleMakeScene();
				PAL_BattleFadeScene();
		}

			PAL_BattleUpdateFighters();
			PAL_BattleDisplayStatChange();
			PAL_BattleDelay(8, 0, TRUE);

			break;
		}

		case kBattleActionPass:
		{

#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
			if (wPlayerRole == RoleID_ZhaoLingEr && PAL_New_IsPlayerUsingMengshe(wPlayerRole))
			{
				g_Battle.MSCount_Zhao.FleeRate += RandomLong(0, 1);
			}
#endif
			break;
		}
	}

	//
	// Revert this player back to waiting state.
	//
	g_Battle.rgPlayer[wPlayerIndex].state = kFighterWait;
	g_Battle.rgPlayer[wPlayerIndex].flTimeMeter = 0;

	PAL_BattlePostActionCheck(FALSE);

#ifndef PAL_CLASSIC
	//
	// Only check for poisons when the battle is not ended
	//
	fCheckPoison = FALSE;

	if (g_Battle.BattleResult == kBattleResultOnGoing)
	{
		for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
		{
			if (g_Battle.rgEnemy[i].wObjectID != 0)
			{
				fCheckPoison = TRUE;
				break;
			}
		}
	}

	//
	// Check for poisons
	//
	if (fCheckPoison)
	{
		fPoisoned = FALSE;
		PAL_BattleBackupStat();

		for (i = 0; i < MAX_POISONS; i++)
		{
			wObject = gpGlobals->rgPoisonStatus[i][wPlayerIndex].wPoisonID;

			if (wObject != 0)
			{
				fPoisoned = TRUE;
				gpGlobals->rgPoisonStatus[i][wPlayerIndex].wPoisonScript =
					PAL_RunTriggerScript(gpGlobals->rgPoisonStatus[i][wPlayerIndex].wPoisonScript, wPlayerRole);
			}
		}

		if (fPoisoned)
		{
			PAL_BattleDelay(3, 0, TRUE);
			PAL_BattleUpdateFighters();
			if (PAL_BattleDisplayStatChange())
			{
				PAL_BattleDelay(6, 0, TRUE);
			}
		}
	}

	//
	// Update statuses
	//
	for (i = 0; i < kStatusAll; i++)
	{
		if (gpGlobals->rgPlayerStatus[wPlayerRole][i] > 0)
		{
			gpGlobals->rgPlayerStatus[wPlayerRole][i]--;
		}
	}
#endif
}

static INT PAL_BattleEnemySelectTargetIndex(VOID)
/*++
  功能：    Select a attackable player randomly.
  参数：    None.
  返回值：   目标序号.
  --*/
{
	int i;

	i = RandomLong(0, gpGlobals->wMaxPartyMemberIndex);

	while (gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] == 0)
	{
		i = RandomLong(0, gpGlobals->wMaxPartyMemberIndex);
	}

	return i;
}

INT PAL_New_GetAlivePlayerNum(VOID)
{
	INT i, num;
	WORD wPlayerRole;
	for (i = 0, num = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;
		if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] != 0)
		{
			num++;
		}
	}
	return num;
}

INT PAL_New_GetHealthyPlayerNum(VOID)
{
	INT i, num;
	WORD wPlayerRole;
	for (i = 0, num = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;
		if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] != 0 && PAL_New_IfPlayerCanMove(wPlayerRole))
		{
			num++;
		}
	}
	return num;
}

BOOL PAL_New_IfPlayerCanMove(WORD wPlayerRole)
{
	if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] == 0 &&
#ifdef PAL_CLASSIC
		gpGlobals->rgPlayerStatus[wPlayerRole][kStatusParalyzed] == 0 &&
#else
		gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSlow] == 0 &&
#endif
		gpGlobals->rgPlayerStatus[wPlayerRole][kStatusConfused] == 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL PAL_New_IfEnemyCanMove(WORD wEnemyIndex)
{
	if (g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusSleep] == 0 &&
		g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusParalyzed] == 0 &&
		g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusConfused] == 0 &&
		g_Battle.iHidingTime <= 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static BOOL PAL_New_IfEnemyCanEscape(WORD wEnemyIndex)
{
	WORD percentage = g_Battle.rgEnemy[wEnemyIndex].dwActualHealth * 100 / max(g_Battle.rgEnemy[wEnemyIndex].dwMaxHealth, 1);

	if (!g_Battle.fIsBoss && PAL_New_IfEnemyCanMove(wEnemyIndex))
	{
		if (PAL_New_GetAliveEnemyNum() < PAL_New_GetHealthyPlayerNum() && percentage < 25)
		{
			return TRUE;
		}
		else if (PAL_New_GetAliveEnemyNum() <= PAL_New_GetHealthyPlayerNum() && percentage <= 13)
		{
			return TRUE;
		}
	}
	return FALSE;
}

VOID
PAL_BattleEnemyPerformAction(
	WORD		wEnemyIndex
	)
	/*++
	  功能：    Perform the selected action for a player.
	  参数：    [IN]  wEnemyIndex - the index of the player.
	  返回值：    None.
	  --*/
{
	int        str, def, iCoverIndex, i, x, y, ex, ey, iSound;
	SHORT      sElementalResistance[NUM_MAGIC_ELEMENTAL];
	SHORT      sPoisonResistance;
	WORD       wPlayerRole, w, wMagic, wMagicNum;
#ifdef DONOT_SELECT_TARGET_RANDOMLY_TOTALLY
	WORD       tMagicNum;
#endif
	SHORT      sTarget;
	INT			iDamage;
	BOOL       fAutoDefend = FALSE, rgfMagAutoDefend[MAX_PLAYERS_IN_PARTY];

	g_Battle.iBlow = 0;

	PAL_BattleBackupStat();

#ifdef ENEMY_CAN_ESCAPE
	if (PAL_New_IfEnemyCanEscape(wEnemyIndex))
	{
		str = PAL_New_GetEnemyFleeRate(wEnemyIndex);
		int index = PAL_New_GetPlayerIndexByPara(Para_FleeRate, FALSE);
		wPlayerRole = gpGlobals->rgParty[index].wPlayerRole;
		def = PAL_GetPlayerFleeRate(wPlayerRole);
		def = (def > 2 * str) ? (2 * str) : def;
		if (RandomLong(0, str / max(1, PAL_New_GetAliveEnemyNum())) >= RandomLong(0, def) && PAL_New_GetTrueByPercentage(50))
		{
			PAL_BattleEnemyEscape();				// 默认结果是战斗终止
			g_Battle.BattleResult = kBattleResultEnemyFleed;// 此处改为敌人逃跑
			return;
		}
	}
#endif

	wMagic = g_Battle.rgEnemy[wEnemyIndex].e.wMagic;
	if (wMagic == 0 || wMagic == 0xFFFF)
	{
		tMagicNum = 0;
	}
	else
	{
		tMagicNum = gpGlobals->g.rgObject[wMagic].magic.wMagicNumber;
	}

#ifndef DONOT_SELECT_TARGET_RANDOMLY_TOTALLY
	sTarget = PAL_BattleEnemySelectTargetIndex();
#else    //基础值大于0时，较大几率选择生命值最低的角色。不大于0时，较大几率选择生命值最高的角色。
	if (wMagic == 0xFFFF)
	{
		sTarget = PAL_BattleEnemySelectTargetIndex();
	}
	else if (wMagic == 0 || (SHORT)gpGlobals->g.lprgMagic[tMagicNum].wBaseDamage >= 0)
	{
		if (RandomLong(0, 9) < 6)
		{
			sTarget = PAL_New_GetPlayerIndexByHealth(TRUE);
		}
		else
		{
			sTarget = PAL_BattleEnemySelectTargetIndex();
		}
	}
	else
	{
		if (RandomLong(0, 9) < 4)
		{
			sTarget = PAL_New_GetPlayerIndexByHealth(FALSE);
		}
		else
		{
			sTarget = PAL_BattleEnemySelectTargetIndex();
		}
	}
#endif

	wPlayerRole = gpGlobals->rgParty[sTarget].wPlayerRole;

	if (g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusSleep] > 0 ||
		g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusParalyzed] > 0 ||
		g_Battle.iHidingTime > 0)
	{
		//
		// Do nothing
		//
		goto end;
	}
	else if (g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusConfused] > 0)
	{
		// TODO
	}
	else if (wMagic != 0 &&
		RandomLong(0, 9) < g_Battle.rgEnemy[wEnemyIndex].e.wMagicRate &&
		g_Battle.rgEnemy[wEnemyIndex].rgwStatus[kStatusSilence] == 0)
	{
		//
		// Magical attack
		//
		if (wMagic == 0xFFFF)
		{
			//
			// Do nothing
			//
			goto end;
		}

		wMagicNum = gpGlobals->g.rgObject[wMagic].magic.wMagicNumber;

		str = PAL_New_GetEnemyMagicStrength(wEnemyIndex);

		ex = PAL_X(g_Battle.rgEnemy[wEnemyIndex].pos);
		ey = PAL_Y(g_Battle.rgEnemy[wEnemyIndex].pos);

		ex += 12;
		ey += 6;

		g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(ex, ey);
		PAL_BattleDelay(1, 0, FALSE);

		ex += 4;
		ey += 2;

		g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(ex, ey);
		PAL_BattleDelay(1, 0, FALSE);

		SOUND_Play(g_Battle.rgEnemy[wEnemyIndex].e.wMagicSound);

		for (i = 0; i < g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames; i++)
		{
			g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
				g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames + i;
			PAL_BattleDelay(g_Battle.rgEnemy[wEnemyIndex].e.wActWaitFrames, 0, FALSE);
		}

		if (g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames == 0)
		{
			PAL_BattleDelay(1, 0, FALSE);
		}

		if (gpGlobals->g.lprgMagic[wMagicNum].wSoundDelay == 0)
		{
			for (i = 0; i <= g_Battle.rgEnemy[wEnemyIndex].e.wAttackFrames; i++)
			{
				g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
					i - 1 + g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames + g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames;
				PAL_BattleDelay(g_Battle.rgEnemy[wEnemyIndex].e.wActWaitFrames, 0, FALSE);
			}
		}
		else//对于风雪冰天和大咒蛇等法术的处理
		{
			//PAL_BattleDelay(gpGlobals->g.lprgMagic[wMagicNum].wSoundDelay, 0, FALSE);
			for (i = 0; i <= g_Battle.rgEnemy[wEnemyIndex].e.wAttackFrames; i++)
			{
				g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
					i - 1 + g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames + g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames;
				PAL_BattleDelay(g_Battle.rgEnemy[wEnemyIndex].e.wActWaitFrames, 0, FALSE);
			}
		}

		if (gpGlobals->g.lprgMagic[wMagicNum].wType != kMagicTypeNormal)
		{
			sTarget = -1;

			for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				w = gpGlobals->rgParty[i].wPlayerRole;

				if (PAL_New_IfPlayerCanMove(w) && PAL_New_GetTrueByPercentage(33) && gpGlobals->g.PlayerRoles.rgwHP[w] != 0)
				{
					rgfMagAutoDefend[i] = TRUE;
					g_Battle.rgPlayer[i].wCurrentFrame = 3;
				}
				else
				{
					rgfMagAutoDefend[i] = FALSE;
				}
			}
		}
		else if (PAL_New_IfPlayerCanMove(wPlayerRole) && PAL_New_GetTrueByPercentage(33))
		{	//选择的目标不能是HP等于0的，所以不用在此判断HP不等于0
			fAutoDefend = TRUE;
			g_Battle.rgPlayer[sTarget].wCurrentFrame = 3;
		}

		//      PAL_BattleDelay(12, (WORD)(-((SHORT)wMagic)), FALSE);

		gpGlobals->g.rgObject[wMagic].magic.wScriptOnUse =
			PAL_RunTriggerScript(gpGlobals->g.rgObject[wMagic].magic.wScriptOnUse, wPlayerRole);

		if (g_fScriptSuccess)
		{
			PAL_BattleShowEnemyMagicAnim(wMagic, sTarget);

			gpGlobals->g.rgObject[wMagic].magic.wScriptOnSuccess =
				PAL_RunTriggerScript(gpGlobals->g.rgObject[wMagic].magic.wScriptOnSuccess, wPlayerRole);
		}

		if ((SHORT)(gpGlobals->g.lprgMagic[wMagicNum].wBaseDamage) > 0)
		{
			if (sTarget == -1)
			{
				//
				// damage all players
				//
				for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
				{
					w = gpGlobals->rgParty[i].wPlayerRole;
					if (gpGlobals->g.PlayerRoles.rgwHP[w] == 0)
					{	// skip dead players
						continue;
					}

					def = PAL_GetPlayerDefense(w);

					for (x = 0; x < NUM_MAGIC_ELEMENTAL; x++)
					{
						sElementalResistance[x] = (SHORT)PAL_GetPlayerElementalResistance(w, x);
					}
					sPoisonResistance = (SHORT)PAL_GetPlayerPoisonResistance(w);

					iDamage = PAL_CalcMagicDamage(str, def, sElementalResistance, sPoisonResistance, wMagic);

					iDamage /= ((g_Battle.rgPlayer[i].fDefending ? 2 : 1) *
						((gpGlobals->rgPlayerStatus[w][kStatusProtect] > 0) ? 2 : 1)) +
						(rgfMagAutoDefend[i] ? 1 : 0);

#ifdef GAIN_MORE_HIDDEN_EXP
					if (rgfMagAutoDefend[i] == TRUE)
					{
						gpGlobals->Exp.rgDefenseExp[wPlayerRole].wCount += RandomLong(0, 1);
						gpGlobals->Exp.rgDexterityExp[wPlayerRole].wCount += RandomLong(0, 1);
						gpGlobals->Exp.rgFleeExp[wPlayerRole].wCount += RandomLong(0, 1);
					}
#endif

#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
					if (wPlayerRole == RoleID_ZhaoLingEr && PAL_New_IsPlayerUsingMengshe(wPlayerRole))
					{
						if (rgfMagAutoDefend[i])
						{
							g_Battle.MSCount_Zhao.MaxMP += RandomLong(0, 1);
							g_Battle.MSCount_Zhao.Defense++;
							g_Battle.MSCount_Zhao.Dexterity++;
							g_Battle.MSCount_Zhao.FleeRate++;
						}
						if (gpGlobals->rgPlayerStatus[RoleID_ZhaoLingEr][kStatusProtect] != 0)
						{
							g_Battle.MSCount_Zhao.Defense++;
						}
					}
#endif
					iDamage = max(iDamage, 0);
					iDamage = min(iDamage, gpGlobals->g.PlayerRoles.rgwHP[w]);
#ifndef INVINCIBLE
					gpGlobals->g.PlayerRoles.rgwHP[w] -= iDamage;
#endif

					if (gpGlobals->g.PlayerRoles.rgwHP[w] == 0)
					{
						SOUND_Play(gpGlobals->g.PlayerRoles.rgwDeathSound[w]);
					}
				}
			}
			else
			{
				//
				// damage one player
				//
				def = PAL_GetPlayerDefense(wPlayerRole);

				for (x = 0; x < NUM_MAGIC_ELEMENTAL; x++)
				{
					sElementalResistance[x] = PAL_GetPlayerElementalResistance(wPlayerRole, x);
				}
				sPoisonResistance = PAL_GetPlayerPoisonResistance(wPlayerRole);

				iDamage = PAL_CalcMagicDamage(str, def, sElementalResistance, sPoisonResistance, wMagic);

				iDamage /= ((g_Battle.rgPlayer[sTarget].fDefending ? 2 : 1) *
					((gpGlobals->rgPlayerStatus[wPlayerRole][kStatusProtect] > 0) ? 2 : 1)) +
					(fAutoDefend ? 1 : 0);

#ifdef GAIN_MORE_HIDDEN_EXP
				if (fAutoDefend == TRUE)
				{
					gpGlobals->Exp.rgDefenseExp[wPlayerRole].wCount += RandomLong(0, 1);
					gpGlobals->Exp.rgDexterityExp[wPlayerRole].wCount += RandomLong(0, 1);
					gpGlobals->Exp.rgFleeExp[wPlayerRole].wCount += RandomLong(0, 1);
				}
#endif

#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
				if (wPlayerRole == RoleID_ZhaoLingEr && PAL_New_IsPlayerUsingMengshe(wPlayerRole))
				{
					if (fAutoDefend)
					{
						g_Battle.MSCount_Zhao.MaxMP += RandomLong(0, 1);
						g_Battle.MSCount_Zhao.Defense++;
						g_Battle.MSCount_Zhao.Dexterity++;
						g_Battle.MSCount_Zhao.FleeRate++;
					}
					if (gpGlobals->rgPlayerStatus[RoleID_ZhaoLingEr][kStatusProtect] != 0)
					{
						g_Battle.MSCount_Zhao.Defense++;
					}
				}
#endif
				iDamage = max(iDamage, 0);
				iDamage = min(iDamage, gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole]);

#ifndef INVINCIBLE
				gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] -= iDamage;
#endif

				if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0)
				{
					SOUND_Play(gpGlobals->g.PlayerRoles.rgwDeathSound[wPlayerRole]);
				}
			}
		}

		if (!gpGlobals->fAutoBattle)
		{
			PAL_BattleDisplayStatChange();
		}

		for (i = 0; i < 5; i++)
		{
			if (sTarget == -1)
			{
				for (x = 0; x <= gpGlobals->wMaxPartyMemberIndex; x++)
				{
					if (g_Battle.rgPlayer[x].wPrevHP ==
						gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[x].wPlayerRole])
					{
						//
						// Skip unaffected players
						//
						continue;
					}

					g_Battle.rgPlayer[x].wCurrentFrame = 4;
					if (i > 0)
					{
						g_Battle.rgPlayer[x].pos =
							PAL_XY(PAL_X(g_Battle.rgPlayer[x].pos) + (8 >> i),
								PAL_Y(g_Battle.rgPlayer[x].pos) + (4 >> i));
					}
					g_Battle.rgPlayer[x].iColorShift = ((i < 3) ? 6 : 0);
				}
			}
			else
			{
				g_Battle.rgPlayer[sTarget].wCurrentFrame = 4;
				if (i > 0)
				{
					g_Battle.rgPlayer[sTarget].pos =
						PAL_XY(PAL_X(g_Battle.rgPlayer[sTarget].pos) + (8 >> i),
							PAL_Y(g_Battle.rgPlayer[sTarget].pos) + (4 >> i));
				}
				g_Battle.rgPlayer[sTarget].iColorShift = ((i < 3) ? 6 : 0);
			}

			PAL_BattleDelay(1, 0, FALSE);
		}

		g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame = 0;
		g_Battle.rgEnemy[wEnemyIndex].pos = g_Battle.rgEnemy[wEnemyIndex].posOriginal;

		PAL_BattleDelay(1, 0, FALSE);
		PAL_BattleUpdateFighters();

		PAL_BattlePostActionCheck(TRUE);
		PAL_BattleDelay(8, 0, TRUE);
	}
	else
	{
		//
		// Physical attack
		//
		WORD wFrameBak = g_Battle.rgPlayer[sTarget].wCurrentFrame;

		str = PAL_New_GetEnemyAttackStrength(wEnemyIndex);
		def = PAL_GetPlayerDefense(wPlayerRole);

		if (g_Battle.rgPlayer[sTarget].fDefending)
		{
			def *= 2;
		}

		SOUND_Play(g_Battle.rgEnemy[wEnemyIndex].e.wAttackSound);

		iCoverIndex = -1;

		fAutoDefend = PAL_New_GetTrueByPercentage(44);

#ifdef GAIN_MORE_HIDDEN_EXP
		if (fAutoDefend == TRUE)
		{
			gpGlobals->Exp.rgDefenseExp[wPlayerRole].wCount += RandomLong(0, 1);
			gpGlobals->Exp.rgDexterityExp[wPlayerRole].wCount += RandomLong(0, 1);
			gpGlobals->Exp.rgFleeExp[wPlayerRole].wCount += RandomLong(0, 1);
		}
#endif 

#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
		if (wPlayerRole == RoleID_ZhaoLingEr && PAL_New_IsPlayerUsingMengshe(wPlayerRole))
		{
			if (fAutoDefend == TRUE)
			{
				g_Battle.MSCount_Zhao.MaxHP += RandomLong(0, 1);
				g_Battle.MSCount_Zhao.Defense++;
			}
		}
#endif

		//
		// Check if the inflictor should be protected
		//
		if ((PAL_IsPlayerDying(wPlayerRole) || !PAL_New_IfPlayerCanMove(wPlayerRole)) && fAutoDefend)
		{
			w = gpGlobals->g.PlayerRoles.rgwCoveredBy[wPlayerRole];
			iCoverIndex = PAL_New_GetPlayerIndex(w);

			if (iCoverIndex != -1)
			{
				if (PAL_IsPlayerDying(gpGlobals->rgParty[iCoverIndex].wPlayerRole)
					|| !PAL_New_IfPlayerCanMove(gpGlobals->rgParty[iCoverIndex].wPlayerRole))
				{
					iCoverIndex = -1;
				}
			}
		}

		//
		// If no one can cover the inflictor and inflictor is in a
		// bad status, don't evade
		//
		if (iCoverIndex == -1 && !PAL_New_IfPlayerCanMove(wPlayerRole))
		{
			fAutoDefend = FALSE;
		}

		for (i = 0; i < g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames; i++)
		{
			g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
				g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames + i;
			PAL_BattleDelay(2, 0, FALSE);
		}

		for (i = 0; i < 3 - g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames; i++)
		{
			x = PAL_X(g_Battle.rgEnemy[wEnemyIndex].pos) - 2;
			y = PAL_Y(g_Battle.rgEnemy[wEnemyIndex].pos) - 1;
			g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(x, y);
			PAL_BattleDelay(1, 0, FALSE);
		}
#ifdef PAL_WIN95
		if (g_Battle.rgEnemy[wEnemyIndex].e.wActionSound != 0)
#endif
		{
			SOUND_Play(g_Battle.rgEnemy[wEnemyIndex].e.wActionSound);
		}
		PAL_BattleDelay(1, 0, FALSE);

		ex = PAL_X(g_Battle.rgPlayer[sTarget].pos) - 44;
		ey = PAL_Y(g_Battle.rgPlayer[sTarget].pos) - 16;

		iSound = g_Battle.rgEnemy[wEnemyIndex].e.wCallSound;

		if (iCoverIndex != -1)
		{
			iSound = gpGlobals->g.PlayerRoles.rgwCoverSound[gpGlobals->rgParty[iCoverIndex].wPlayerRole];

			g_Battle.rgPlayer[iCoverIndex].wCurrentFrame = 3;

			x = PAL_X(g_Battle.rgPlayer[sTarget].pos) - 24;
			y = PAL_Y(g_Battle.rgPlayer[sTarget].pos) - 12;

			g_Battle.rgPlayer[iCoverIndex].pos = PAL_XY(x, y);
		}
		else if (fAutoDefend)
		{
			g_Battle.rgPlayer[sTarget].wCurrentFrame = 3;
			iSound = gpGlobals->g.PlayerRoles.rgwCoverSound[wPlayerRole];
		}

		if (g_Battle.rgEnemy[wEnemyIndex].e.wAttackFrames == 0)
		{
			g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
				g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames - 1;

			g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(ex, ey);

			PAL_BattleDelay(2, 0, FALSE);
		}
		else
		{
			for (i = 0; i <= g_Battle.rgEnemy[wEnemyIndex].e.wAttackFrames; i++)
			{
				g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame =
					g_Battle.rgEnemy[wEnemyIndex].e.wIdleFrames +
					g_Battle.rgEnemy[wEnemyIndex].e.wMagicFrames + i - 1;

				g_Battle.rgEnemy[wEnemyIndex].pos = PAL_XY(ex, ey);

				PAL_BattleDelay(g_Battle.rgEnemy[wEnemyIndex].e.wActWaitFrames, 0, FALSE);
			}
		}

		if (!fAutoDefend)
		{
			g_Battle.rgPlayer[sTarget].wCurrentFrame = 4;

			iDamage = PAL_CalcPhysicalAttackDamage(str + RandomLong(0, 2), def, 20 + 0.8 * PAL_New_GetPlayerPhysicalResistance(wPlayerRole));
			iDamage += RandomLong(0, 1);

			if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusProtect])
			{
				iDamage /= 2;
			}
			iDamage = max(iDamage, 1);
			iDamage = min(iDamage, gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole]);

#ifndef INVINCIBLE
			gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] -= iDamage;
#endif

			PAL_BattleDisplayStatChange();

			g_Battle.rgPlayer[sTarget].iColorShift = 6;
		}

#ifdef PAL_WIN95
		if (iSound != 0)
#endif
		{
			SOUND_Play(iSound);
		}

		if (iCoverIndex == -1 && !fAutoDefend &&
			g_Battle.rgEnemy[wEnemyIndex].e.wAttackEquivItemRate >= RandomLong(1, 10))
		{
			i = g_Battle.rgEnemy[wEnemyIndex].e.wAttackEquivItem;
			gpGlobals->g.rgObject[i].item.wScriptOnUse =
				PAL_RunTriggerScript(gpGlobals->g.rgObject[i].item.wScriptOnUse, wPlayerRole);
		}
		PAL_BattleDisplayStatChange();

		PAL_BattleDelay(1, 0, FALSE);

		g_Battle.rgPlayer[sTarget].iColorShift = 0;

		if (iCoverIndex != -1)
		{
			g_Battle.rgEnemy[wEnemyIndex].pos =
				PAL_XY(PAL_X(g_Battle.rgEnemy[wEnemyIndex].pos) - 10,
					PAL_Y(g_Battle.rgEnemy[wEnemyIndex].pos) - 8);
			g_Battle.rgPlayer[iCoverIndex].pos =
				PAL_XY(PAL_X(g_Battle.rgPlayer[iCoverIndex].pos) + 4,
					PAL_Y(g_Battle.rgPlayer[iCoverIndex].pos) + 2);
		}
		else
		{
			g_Battle.rgPlayer[sTarget].pos =
				PAL_XY(PAL_X(g_Battle.rgPlayer[sTarget].pos) + 8,
					PAL_Y(g_Battle.rgPlayer[sTarget].pos) + 4);
		}

		PAL_BattleDelay(1, 0, FALSE);

		if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0)
		{
			SOUND_Play(gpGlobals->g.PlayerRoles.rgwDeathSound[wPlayerRole]);
			wFrameBak = 2;
		}
		else if (PAL_IsPlayerDying(wPlayerRole))
		{
			wFrameBak = 1;
		}

		if (iCoverIndex == -1)
		{
			g_Battle.rgPlayer[sTarget].pos =
				PAL_XY(PAL_X(g_Battle.rgPlayer[sTarget].pos) + 2,
					PAL_Y(g_Battle.rgPlayer[sTarget].pos) + 1);
		}

		PAL_BattleDelay(3, 0, FALSE);

		g_Battle.rgEnemy[wEnemyIndex].pos = g_Battle.rgEnemy[wEnemyIndex].posOriginal;
		g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame = 0;

		PAL_BattleDelay(1, 0, FALSE);

		g_Battle.rgPlayer[sTarget].wCurrentFrame = wFrameBak;
		PAL_BattleDelay(1, 0, TRUE);

		g_Battle.rgPlayer[sTarget].pos = g_Battle.rgPlayer[sTarget].posOriginal;
		PAL_BattleDelay(4, 0, TRUE);

		PAL_BattleUpdateFighters();

		if (iCoverIndex == -1 && !fAutoDefend &&
			g_Battle.rgEnemy[wEnemyIndex].e.wAttackEquivItemRate >= RandomLong(1, 10))
		{
			i = g_Battle.rgEnemy[wEnemyIndex].e.wAttackEquivItem;
			gpGlobals->g.rgObject[i].item.wScriptOnUse =
				PAL_RunTriggerScript(gpGlobals->g.rgObject[i].item.wScriptOnUse, wPlayerRole);
		}

		PAL_BattlePostActionCheck(TRUE);
		}

end:
#ifndef PAL_CLASSIC
	//
	// Check poisons
	//
	if (!g_Battle.rgEnemy[wEnemyIndex].fDualMove)
	{
		PAL_BattleBackupStat();

		for (i = 0; i < MAX_POISONS; i++)
		{
			if (g_Battle.rgEnemy[wEnemyIndex].rgPoisons[i].wPoisonID != 0)
			{
				g_Battle.rgEnemy[wEnemyIndex].rgPoisons[i].wPoisonScript =
					PAL_RunTriggerScript(g_Battle.rgEnemy[wEnemyIndex].rgPoisons[i].wPoisonScript, wEnemyIndex);
			}
		}

		if (PAL_BattleDisplayStatChange())
		{
			PAL_BattleDelay(6, 0, FALSE);
		}
	}

	PAL_BattlePostActionCheck(FALSE);

	//
	// Update statuses
	//
	for (i = 0; i < kStatusAll; i++)
	{
		if (g_Battle.rgEnemy[wEnemyIndex].rgwStatus[i] > 0)
		{
			g_Battle.rgEnemy[wEnemyIndex].rgwStatus[i]--;
		}
	}
#else
	i = 0; // do nothing
#endif

	}

VOID
PAL_BattleStealFromEnemy(
	WORD		wTarget,
	WORD		wStealRate
	)
	/*++
	  功能：    Steal from the enemy.
	  参数：    [IN]  wTarget - the target enemy index.
	  [IN]		wStealRate - the rate of successful theft.
	  返回值：    None.
	  --*/
{
	int   iPlayerIndex = g_Battle.wMovingPlayerIndex;
	int   offset, x, y, i;
	char  s[256] = "";

	g_Battle.rgPlayer[iPlayerIndex].wCurrentFrame = 10;
	offset = ((INT)wTarget - iPlayerIndex) * 8;

	x = PAL_X(g_Battle.rgEnemy[wTarget].pos) + 64 - offset;
	y = PAL_Y(g_Battle.rgEnemy[wTarget].pos) + 20 - offset / 2;

	g_Battle.rgPlayer[iPlayerIndex].pos = PAL_XY(x, y);

	PAL_BattleDelay(1, 0, TRUE);

	for (i = 0; i < 5; i++)
	{
		x -= i + 8;
		y -= 4;

		g_Battle.rgPlayer[iPlayerIndex].pos = PAL_XY(x, y);

		if (i == 4)
		{
			g_Battle.rgEnemy[wTarget].iColorShift = 6;
		}

		PAL_BattleDelay(1, 0, TRUE);
	}

	g_Battle.rgEnemy[wTarget].iColorShift = 0;
	x--;
	g_Battle.rgPlayer[iPlayerIndex].pos = PAL_XY(x, y);
	PAL_BattleDelay(3, 0, TRUE);

	g_Battle.rgPlayer[iPlayerIndex].state = kFighterWait;
	g_Battle.rgPlayer[iPlayerIndex].flTimeMeter = 0;
	PAL_BattleUpdateFighters();
	PAL_BattleDelay(1, 0, TRUE);

	if (g_Battle.rgEnemy[wTarget].e.nStealItem > 0 &&
		(RandomLong(1, 10) <= wStealRate || wStealRate == 0))
	{
		if (g_Battle.rgEnemy[wTarget].e.wStealItem == 0)
		{
			// stolen coins
			int c = g_Battle.rgEnemy[wTarget].e.nStealItem / RandomLong(2, 3);
			g_Battle.rgEnemy[wTarget].e.nStealItem -= c;
			gpGlobals->dwCash += c;

			if (c > 0)
			{
				strcpy(s, PAL_GetWord(34));
				strcat(s, " ");
				strcat(s, va("%d", c));
				strcat(s, " ");
				strcat(s, PAL_GetWord(10));
			}
		}
		else
		{
			//
			// stolen item
			//
			g_Battle.rgEnemy[wTarget].e.nStealItem--;
			PAL_AddItemToInventory(g_Battle.rgEnemy[wTarget].e.wStealItem, 1);

			strcpy(s, PAL_GetWord(34));
			strcat(s, PAL_GetWord(g_Battle.rgEnemy[wTarget].e.wStealItem));
		}

		if (s[0] != '\0')
		{
#ifdef PAL_CLASSIC
			PAL_StartDialog(kDialogCenterWindow, 0, 0, FALSE);
			PAL_ShowDialogText(s);
#else
			PAL_BattleUIShowText(s, 800);
#endif
		}
	}
}

VOID
PAL_BattleSimulateMagic(
	SHORT		sTarget,
	WORD		wMagicObjectID,
	WORD		wBaseDamage,
	BOOL		fIncludeMagicDamage
	)
	/*++
	  功能：  Simulate a magic for players. Mostly used in item throwing script.
	  参数：
	  [IN]  sTarget - the target enemy index. -1 = all enemies.
	  [IN]  wMagicObjectID - the object ID of the magic to be simulated.
	  [IN]  wBaseDamage - the base damage of the simulation.
	  返回值：  None.
	  --*/
{
	WORD	wMagicDamage, wMagicNumber;

	if (gpGlobals->g.rgObject[wMagicObjectID].magic.wFlags & kMagicFlagApplyToAll)
	{
		sTarget = -1;
	}
	else if (sTarget == -1)
	{
		sTarget = PAL_BattleSelectAutoTarget();
	}
	//
	// Show the magic animation
	//
#ifdef PAL_WIN95
	PAL_BattleShowPlayerOffMagicAnim(0xFFFF, wMagicObjectID, sTarget, FALSE);
#else
	PAL_BattleShowPlayerOffMagicAnim(0xFFFF, wMagicObjectID, sTarget);
#endif

	wMagicNumber = gpGlobals->g.rgObject[wMagicObjectID].magic.wMagicNumber;
	wMagicDamage = gpGlobals->g.lprgMagic[wMagicNumber].wBaseDamage;

	if (fIncludeMagicDamage == FALSE ||
		wMagicObjectID == 0x017b ||
		wMagicObjectID == 0x0181)
	{
		gpGlobals->g.lprgMagic[wMagicNumber].wBaseDamage = 0;
	}

	if ((short)wMagicDamage > 0 || wBaseDamage > 0)
	{
		PAL_New_ApplyMagicDamageToEnemy(sTarget, wBaseDamage, wMagicObjectID, FALSE);
	}
	gpGlobals->g.lprgMagic[wMagicNumber].wBaseDamage = wMagicDamage;
}

VOID
PAL_New_ApplyMagicDamageToEnemy(
	SHORT		sTarget,
	WORD		wBaseDamage,
	WORD		wMagicObjectID,
	BOOL		fOneDamageAtLeast
	)
{
	INT		iDamage;
	int     i, j;
	WORD	def;
	SHORT	sElemResistance[NUM_MAGIC_ELEMENTAL];
	SHORT	sPoisonResistance;
	WORD	wMagicNumber = gpGlobals->g.rgObject[wMagicObjectID].magic.wMagicNumber;

	if ((SHORT)(gpGlobals->g.lprgMagic[wMagicNumber].wBaseDamage) < 0)
	{
		return;
	}

	if (sTarget == -1)
	{	// Apply to all enemies
		for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
		{
			if (g_Battle.rgEnemy[i].wObjectID == 0)
			{
				continue;
			}

			def = PAL_New_GetEnemyDefense(i);

			for (j = 0; j < NUM_MAGIC_ELEMENTAL; j++)
			{
				sElemResistance[j] = (PAL_New_GetEnemyElementalResistance((WORD)i, j) - 50) * 2;
			}
			sPoisonResistance = (PAL_New_GetEnemyPoisonResistance(i) - 50) * 2;

			iDamage = PAL_CalcMagicDamage(wBaseDamage, def, sElemResistance, sPoisonResistance, wMagicObjectID);
			iDamage = fOneDamageAtLeast ? max(1, iDamage) : max(0, iDamage);
			iDamage = min(iDamage, MAX_DAMAGE);
			iDamage = min(iDamage, g_Battle.rgEnemy[i].dwActualHealth);

			g_Battle.rgEnemy[i].dwActualHealth -= iDamage;
		}
	}
	else
	{	// Apply to one enemy
		def = PAL_New_GetEnemyDefense(sTarget);

		for (j = 0; j < NUM_MAGIC_ELEMENTAL; j++)
		{
			sElemResistance[j] = (PAL_New_GetEnemyElementalResistance((WORD)sTarget, j) - 50) * 2;
		}
		sPoisonResistance = (PAL_New_GetEnemyPoisonResistance((WORD)sTarget) - 50) * 2;

		iDamage = PAL_CalcMagicDamage(wBaseDamage, (WORD)def, sElemResistance, sPoisonResistance, wMagicObjectID);
		iDamage = fOneDamageAtLeast ? max(1, iDamage) : max(0, iDamage);
		iDamage = min(iDamage, MAX_DAMAGE);
		iDamage = min(iDamage, g_Battle.rgEnemy[sTarget].dwActualHealth);

		g_Battle.rgEnemy[sTarget].dwActualHealth -= iDamage;
	}
}

