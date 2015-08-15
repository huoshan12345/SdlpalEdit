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


BATTLE g_Battle;

const WORD g_rgPlayerPos[5][5][2] = 
{					
		{{240, 170}},										// one player
		{{200, 176}, {256, 152}},						 // two players
		{{180, 180}, {234, 170}, {270, 146}},				// three players
		{{160, 180}, {217, 175}, {255, 155}, {285, 135}},  // 4 players
		{{160, 180}, {210, 175}, {240, 160}, {265, 145}, {285, 125}},  // 5 players
};

const INT ExpTypeToPlayerPara[] =
{
	0, Para_MaxHP, Para_MaxMP, Para_AttackStrength, Para_MagicStrength, Para_Defense, Para_Dexterity, Para_FleeRate
};

const INT ExpTypeToLabel[] =
{
	0, STATUS_LABEL_HP, STATUS_LABEL_MP, STATUS_LABEL_ATTACKPOWER, STATUS_LABEL_MAGICPOWER,
	STATUS_LABEL_RESISTANCE, STATUS_LABEL_DEXTERITY, STATUS_LABEL_FLEERATE
};

/*++
功能：在场景缓存中生成新的战斗场景。
参数：    无。
返回值：    无。
--*/
VOID PAL_BattleMakeScene(VOID)
{
	int          i;
	PAL_POS      pos;				// 用typedef重命名的DWORD类型。DWORD是用typedef重命名的unsigned long类型。
	LPBYTE       pSrc, pDst;			// 只想BYTE类型的指针
	BYTE         b;					// 用typedef重命名的unsigned char类型
	LPCBITMAPRLE lBMR = NULL;

	//
	// Draw the background	画背景
	//
	pSrc = g_Battle.lpBackground->pixels;
	pDst = g_Battle.lpSceneBuf->pixels;

	for (i = 0; i < g_Battle.lpSceneBuf->pitch * g_Battle.lpSceneBuf->h; i++)
	{
		b = (*pSrc & 0x0F);
		b += g_Battle.sBackgroundColorShift;

		if (b & 0x80)
		{
			b = 0;
		}
		else if (b & 0x70)
		{
			b = 0x0F;
		}

		*pDst = (b | (*pSrc & 0xF0));

		++pSrc;
		++pDst;
	}

	PAL_ApplyWave(g_Battle.lpSceneBuf);

	//
	// Draw the enemies	
	//绘制敌人

	for (i = g_Battle.wMaxEnemyIndex; i >= 0; i--)
	{
		pos = g_Battle.rgEnemy[i].pos;

		if (g_Battle.rgEnemy[i].rgwStatus[kStatusConfused] > 0 &&
			g_Battle.rgEnemy[i].rgwStatus[kStatusSleep] == 0 &&
			g_Battle.rgEnemy[i].rgwStatus[kStatusParalyzed] == 0)
		{
			//
			// Enemy is confused 
			//敌人陷入混乱状态s
			pos = PAL_XY(PAL_X(pos) + RandomLong(-1, 1), PAL_Y(pos));
		}
		lBMR = PAL_SpriteGetFrame(g_Battle.rgEnemy[i].lpSprite, g_Battle.rgEnemy[i].wCurrentFrame);

		pos = PAL_XY(PAL_X(pos) - PAL_RLEGetWidth(lBMR) / 2, PAL_Y(pos) - PAL_RLEGetHeight(lBMR));

		if (g_Battle.rgEnemy[i].wObjectID != 0)
		{
			if (g_Battle.rgEnemy[i].iColorShift)
			{
				PAL_RLEBlitWithColorShift(lBMR, g_Battle.lpSceneBuf, pos, g_Battle.rgEnemy[i].iColorShift);
			}
			else
			{
				PAL_RLEBlitToSurface(lBMR, g_Battle.lpSceneBuf, pos);
			}
		}
	}

	if (g_Battle.lpSummonSprite != NULL)
	{
		//
		// Draw the summoned god	绘制召唤神的画面
		//
		lBMR = PAL_SpriteGetFrame(g_Battle.lpSummonSprite, g_Battle.iSummonFrame);
		pos = PAL_XY(PAL_X(g_Battle.posSummon) - PAL_RLEGetWidth(lBMR) / 2, PAL_Y(g_Battle.posSummon) - PAL_RLEGetHeight(lBMR));

		PAL_RLEBlitToSurface(lBMR, g_Battle.lpSceneBuf, pos);
	}
	else
	{
		//
		// Draw the players	
		//绘制玩家
		for (i = gpGlobals->wMaxPartyMemberIndex; i >= 0; i--)
		{
			pos = g_Battle.rgPlayer[i].pos;

			if (gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusConfused] != 0 &&
				gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusSleep] == 0 &&
				gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusParalyzed] == 0 &&
				gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] > 0)
			{
				//
				// Player is confused	
				//玩家陷入混乱状态
				continue;
			}

			lBMR = PAL_SpriteGetFrame(g_Battle.rgPlayer[i].lpSprite, g_Battle.rgPlayer[i].wCurrentFrame);
			pos = PAL_XY(PAL_X(pos) - PAL_RLEGetWidth(lBMR) / 2, PAL_Y(pos) - PAL_RLEGetHeight(lBMR));

			if (g_Battle.rgPlayer[i].iColorShift != 0)
			{
				PAL_RLEBlitWithColorShift(lBMR, g_Battle.lpSceneBuf, pos, g_Battle.rgPlayer[i].iColorShift);
			}
			else if (g_Battle.iHidingTime == 0)
			{
				PAL_RLEBlitToSurface(lBMR, g_Battle.lpSceneBuf, pos);
			}
		}

		//
		// Confused players should be drawn on top of normal players
		//玩家的混乱状态应该画在普通状态上层
		for (i = gpGlobals->wMaxPartyMemberIndex; i >= 0; i--)
		{
			if (gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusConfused] != 0 &&
				gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusSleep] == 0 &&
				gpGlobals->rgPlayerStatus[gpGlobals->rgParty[i].wPlayerRole][kStatusParalyzed] == 0 &&
				gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[i].wPlayerRole] > 0)
			{
				//
				// Player is confused
				//玩家陷入混乱状态
				lBMR = PAL_SpriteGetFrame(g_Battle.rgPlayer[i].lpSprite, g_Battle.rgPlayer[i].wCurrentFrame);
				pos = PAL_XY(PAL_X(g_Battle.rgPlayer[i].pos), PAL_Y(g_Battle.rgPlayer[i].pos) + RandomLong(-1, 1));
				pos = PAL_XY(PAL_X(pos) - PAL_RLEGetWidth(lBMR) / 2, PAL_Y(pos) - PAL_RLEGetHeight(lBMR));

				if (g_Battle.rgPlayer[i].iColorShift != 0)
				{
					PAL_RLEBlitWithColorShift(lBMR, g_Battle.lpSceneBuf, pos, g_Battle.rgPlayer[i].iColorShift);
				}
				else if (g_Battle.iHidingTime == 0)
				{
					PAL_RLEBlitToSurface(lBMR, g_Battle.lpSceneBuf, pos);
				}
			}
		}
	}
}

VOID PAL_BattleBackupScene(VOID)
/*++
  目的：    Backup the scene buffer.
  备份场景缓存。
  参数：    无。
  返回值：    无。
  --*/
{	//其意义是将一个平面的一部分或全部图像整块从这个平面复制到另一个平面。
	SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreenBak, NULL);
}

VOID PAL_BattleFadeScene(VOID)
/*++
  功能：    Fade in the scene of battle.
  参数：    无。
  返回值：    无。
  --*/
{
	int               i, j, k;
	DWORD             time;
	BYTE              a, b;
	const int         rgIndex[6] = {0, 3, 1, 5, 2, 4};

	time = SDL_GetTicks();

	for (i = 0; i < 12; i++)
	{
		for (j = 0; j < 6; j++)
		{
			PAL_ProcessEvent();
			while (SDL_GetTicks() <= time)
			{
				PAL_ProcessEvent();
				SDL_Delay(1);
			}
			time = SDL_GetTicks() + 16;

			//
			// Blend the pixels in the 2 buffers, and put the result into the
			// backup buffer
			//混合两个缓存中的像素，并将结果放到备份缓存中
			for (k = rgIndex[j]; k < gpScreen->pitch * gpScreen->h; k += 6)
			{
				a = ((LPBYTE)(g_Battle.lpSceneBuf->pixels))[k];
				b = ((LPBYTE)(gpScreenBak->pixels))[k];

				if (i > 0)
				{
					if ((a & 0x0F) > (b & 0x0F))
					{
						b++;
					}
					else if ((a & 0x0F) < (b & 0x0F))
					{
						b--;
					}
				}

				((LPBYTE)(gpScreenBak->pixels))[k] = ((a & 0xF0) | (b & 0x0F));
			}

			//
			// Draw the backup buffer to the screen
			//将备份缓存中的场景画到屏幕上
			SDL_BlitSurface(gpScreenBak, NULL, gpScreen, NULL);

			PAL_BattleUIUpdate();
			VIDEO_UpdateScreen(NULL);
		}
	}

	//
	// Draw the result buffer to the screen as the final step
	//将结果缓存中的场景画到屏幕上作为最后一步。
	SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);
	PAL_BattleUIUpdate();

	VIDEO_UpdateScreen(NULL);
}

static BATTLERESULT 
PAL_BattleMain(
VOID
)
/*++
  功能：   The main battle routine.
  战斗主程序
  参数：    无。
  返回值：    The result of the battle.
  战斗结果。
  --*/
{
	int         i;
	DWORD       dwTime;

	VIDEO_BackupScreen();

	//
	// Generate the scene and draw the scene to the screen buffer
	//生成场景并且将场景存到屏幕缓存中。
	PAL_BattleMakeScene();
	SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);

	//
	// Fade out the music and delay for a while
	//停止音乐并且等待一会
	PAL_PlayMUS(0, FALSE, 1);
	UTIL_Delay(200);

	//
	// Switch the screen
	//
	VIDEO_SwitchScreen(5);

	//
	// Play the battle music
	//播放战斗音乐。
	PAL_PlayMUS(gpGlobals->wNumBattleMusic, TRUE, 0);

	//
	// Fade in the screen when needed
	//当需要时淡入画面
	if (gpGlobals->fNeedToFadeIn)
	{
		PAL_FadeIn(gpGlobals->wNumPalette, gpGlobals->fNightPalette, 1);
		gpGlobals->fNeedToFadeIn = FALSE;
	}

	//
	// Run the pre-battle scripts for each enemies
	//每个敌人都要运行战前脚本
	PAL_BattleBackupStat();
	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		g_Battle.rgEnemy[i].wScriptOnTurnStart =
			PAL_RunTriggerScript(g_Battle.rgEnemy[i].wScriptOnTurnStart, i);

		if (g_Battle.BattleResult != kBattleResultPreBattle)
		{
			break;
		}
	}
	PAL_BattleDisplayStatChange();

	if (g_Battle.BattleResult == kBattleResultPreBattle)
	{
		g_Battle.BattleResult = kBattleResultOnGoing;
	}


#ifndef PAL_CLASSIC
	PAL_UpdateTimeChargingUnit();
#endif

	dwTime = SDL_GetTicks();

	PAL_ClearKeyState();

	//
	// Run the main battle loop.
	//循环运行战斗回合
	while (TRUE)
	{
		//
		// Break out if the battle ended.
		//如果战斗结束，则同时结束战斗回合的循环。
		if (g_Battle.BattleResult != kBattleResultOnGoing)
		{
			break;
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
		dwTime = SDL_GetTicks() + BATTLE_FRAME_TIME;

		//
		// Run the main frame routine.
		//
		PAL_BattleStartFrame();

		//
		// Update the screen.
		//更新屏幕
		VIDEO_UpdateScreen(NULL);
	}

	//
	// Return the battle result
	//返回战斗结果
	return g_Battle.BattleResult;
}

static VOID 
PAL_FreeBattleSprites(
VOID
)
/*++
  功能：    Free all the loaded sprites.
  释放所有已读取的（战斗）子画面。
  参数：    无。
  返回值：    无。
  --*/
{
	int         i;

	//
	// Free all the loaded sprites
	//释放左右已读取的子画面
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		if (g_Battle.rgPlayer[i].lpSprite != NULL)
		{
			free(g_Battle.rgPlayer[i].lpSprite);
		}
		g_Battle.rgPlayer[i].lpSprite = NULL;
	}

	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].lpSprite != NULL)
		{
			free(g_Battle.rgEnemy[i].lpSprite);
		}
		g_Battle.rgEnemy[i].lpSprite = NULL;
	}

	if (g_Battle.lpSummonSprite != NULL)
	{
		free(g_Battle.lpSummonSprite);
	}
	g_Battle.lpSummonSprite = NULL;
}

VOID 
PAL_LoadBattleSprites(
VOID
)
/*++
  目的：    Load all the loaded sprites.
  载入所有已读的字画面。
  参数：    无。
  返回值：    无。
  --*/
{
	int           i, l, x, y, s;
	FILE         *fp;

	PAL_FreeBattleSprites();

	fp = UTIL_OpenRequiredFile("abc.mkf");

	//
	// Load battle sprites for players
	//读取玩家的战斗子画面
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		s = PAL_GetPlayerBattleSprite(gpGlobals->rgParty[i].wPlayerRole);

		l = PAL_MKFGetDecompressedSize(s, gpGlobals->f.fpF);

		if (l <= 0)
		{
			continue;
		}

		g_Battle.rgPlayer[i].lpSprite = UTIL_calloc(l, 1);

		PAL_MKFDecompressChunk(g_Battle.rgPlayer[i].lpSprite, l, s, gpGlobals->f.fpF);

		//
		// Set the default position for this player
		//设置此玩家的默认位置
		x = g_rgPlayerPos[gpGlobals->wMaxPartyMemberIndex][i][0];
		y = g_rgPlayerPos[gpGlobals->wMaxPartyMemberIndex][i][1];

		g_Battle.rgPlayer[i].posOriginal = PAL_XY(x, y);
		g_Battle.rgPlayer[i].pos = PAL_XY(x, y);
	}

	//
	// Load battle sprites for enemies
	//读取敌人的战斗子画面
	for (i = 0; i < MAX_ENEMIES_IN_TEAM; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID == 0)
		{
			continue;
		}
		//获取该敌人图像的长度
		l = PAL_MKFGetDecompressedSize(
			gpGlobals->g.rgObject[g_Battle.rgEnemy[i].wObjectID].enemy.wEnemyID, fp);

		if (l <= 0)
		{
			continue;
		}
		//申请内存
		g_Battle.rgEnemy[i].lpSprite = UTIL_calloc(l, 1);

		//读入图像
		PAL_MKFDecompressChunk(g_Battle.rgEnemy[i].lpSprite, l,
			gpGlobals->g.rgObject[g_Battle.rgEnemy[i].wObjectID].enemy.wEnemyID, fp);

		//
		// Set the default position for this enemy
		//设置此敌人的默认位置
		x = gpGlobals->g.EnemyPos.pos[i][g_Battle.wMaxEnemyIndex].x;
		y = gpGlobals->g.EnemyPos.pos[i][g_Battle.wMaxEnemyIndex].y;

		y += g_Battle.rgEnemy[i].e.wYPosOffset;

		g_Battle.rgEnemy[i].posOriginal = PAL_XY(x, y);
		g_Battle.rgEnemy[i].pos = PAL_XY(x, y);
	}

	fclose(fp);
}

static VOID 
PAL_LoadBattleBackground(
VOID
)
/*++
  功能：    Load the screen background picture of the battle.
  载入屏幕战斗背景图片。
  参数：    无。
  返回值：    无。
  --*/
{
	PAL_LARGE BYTE           buf[320 * 200];

	//
	// Create the surface
	//
	g_Battle.lpBackground =
		SDL_CreateRGBSurface(gpScreen->flags & ~SDL_HWSURFACE, 320, 200, 8,
		gpScreen->format->Rmask, gpScreen->format->Gmask,
		gpScreen->format->Bmask, gpScreen->format->Amask);

	if (g_Battle.lpBackground == NULL)
	{
		TerminateOnError("PAL_LoadBattleBackground(): failed to create surface!");
	}

#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_SetSurfacePalette(g_Battle.lpBackground, gpScreen->format->palette);
#else
	SDL_SetPalette(g_Battle.lpBackground, SDL_PHYSPAL | SDL_LOGPAL, VIDEO_GetPalette(), 0, 256);
#endif

	//
	// Load the picture
	//读取图片
	PAL_MKFDecompressChunk(buf, 320 * 200, gpGlobals->wNumBattleField, gpGlobals->f.fpFBP);

	//
	// Draw the picture to the surface.
	//
	PAL_FBPBlitToSurface(buf, g_Battle.lpBackground);
}

VOID 
PAL_BattleWon(
VOID
)
/*++
  功能：    Show the "you win" message and add the experience points for players.
  显示“胜利”信息并且增加玩家的经验值。
  参数：    无。
  默认值：    无。
  --*/
{
	const SDL_Rect   rect = {65, 60, 200, 100};
	const SDL_Rect   rect1 = {80, 0, 180, 200};

	int              i, j, iTotalCount;
	DWORD            dwExp;
	WORD             w, wLevel;
	BOOL             fLevelUp;
	PLAYERROLES      OrigPlayerRoles;

	//
	// Backup the initial player stats
	//
	OrigPlayerRoles = gpGlobals->g.PlayerRoles;

	if (g_Battle.iExpGained > 0)
	{
		//
		// Play the "battle win" music
		//播放“战斗胜利”的音乐
		PAL_PlayMUS(g_Battle.fIsBoss ? 2 : 3, FALSE, 0);

		//
		// Show the message about the total number of exp. and cash gained
		//显示全部获得的经验，以及获得的金钱
		PAL_CreateSingleLineBox(PAL_XY(83, 60), 8, FALSE);
		PAL_CreateSingleLineBox(PAL_XY(65, 105), 10, FALSE);

		PAL_DrawText(PAL_GetWord(BATTLEWIN_GETEXP_LABEL), PAL_XY(95, 70), 0, FALSE, FALSE);
		PAL_DrawText(PAL_GetWord(BATTLEWIN_BEATENEMY_LABEL), PAL_XY(77, 115), 0, FALSE, FALSE);
		PAL_DrawText(PAL_GetWord(BATTLEWIN_DOLLAR_LABEL), PAL_XY(197, 115), 0, FALSE, FALSE);

		PAL_DrawNumber(g_Battle.iExpGained, 5, PAL_XY(182, 74), kNumColorYellow, kNumAlignRight);
		PAL_DrawNumber(g_Battle.iCashGained, 5, PAL_XY(162, 119), kNumColorYellow, kNumAlignMid);

		VIDEO_UpdateScreen(&rect);
		PAL_WaitForKey(g_Battle.fIsBoss ? 5500 : 3000);
	}

	// Add the cash value
	gpGlobals->dwCash += g_Battle.iCashGained;

	//
	// Add the experience points for each players
	//增加每个玩家的经验值
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		fLevelUp = FALSE;

		w = gpGlobals->rgParty[i].wPlayerRole;
		if (gpGlobals->g.PlayerRoles.rgwHP[w] == 0)
		{
			continue; // don't care about dead players 已死亡的玩家不会获得经验值
		}

		wLevel = gpGlobals->g.PlayerRoles.rgwLevel[w];
#ifdef STRENGTHEN_PLAYER
		switch (w)
		{	//赵灵儿属性增强
			case RoleID_ZhaoLingEr:
			{
#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
				PAL_New_CheckMengsheAttriIncre(w, g_Battle.MSCount_Zhao);
#endif	
				int x;
				for (x = 0; x < NUM_MAGIC_ELEMENTAL; x++)
				{
					gpGlobals->g.PlayerRoles.rgwElementalResistance[x][w] =
						min(20, max(0, wLevel - 20));
				}
				break;
			}

			case RoleID_LinYueRu:
			{
				gpGlobals->g.PlayerRoles.rgwPhysicalResistance[w] = min(20, max(0, wLevel));
				break;
			}

			case RoleID_ANu:
			{
				gpGlobals->g.PlayerRoles.rgwPoisonResistance[w] = min(30, max(0, wLevel - 40));
				gpGlobals->g.PlayerRoles.rgwSorceryResistance[w] = min(30, max(0, wLevel - 40));
				gpGlobals->g.PlayerRoles.rgwSorceryStrength[w] = min(50, max(0, wLevel - 40));
				break;
			}

			default:
				break;
		}
#endif
		dwExp = gpGlobals->Exp.rgPrimaryExp[w].wExp;

#ifdef EDIT_EXP_CALCULATION
		dwExp += g_Battle.rgPlayer[i].dwExpGained;
#else
		dwExp += g_Battle.iExpGained;
#endif

		if (gpGlobals->g.PlayerRoles.rgwLevel[w] > MAX_LEVELS
#ifdef CAN_BEYOND_MAX_PARAMETER_SOMETIMES
			&& gpGlobals->g.PlayerRoles.rgwCanBeyondMaxPara[w] == 0
#endif
			)
		{
			gpGlobals->g.PlayerRoles.rgwLevel[w] = MAX_LEVELS;
		}

		WORD LevelUpExp = 0;
		while (TRUE)
		{
#ifdef EDIT_EXP_CALCULATION
			LevelUpExp = PAL_New_GetLevelUpExp(gpGlobals->g.PlayerRoles.rgwLevel[w]);
#else
			LevelUpExp = gpGlobals->g.rgLevelUpExp[min(MAX_LEVELS, gpGlobals->g.PlayerRoles.rgwLevel[w])];
#endif
			if (dwExp < LevelUpExp)
			{
				break;
			}

			dwExp -= LevelUpExp;

			if (gpGlobals->g.PlayerRoles.rgwLevel[w] < MAX_LEVELS
#ifdef CAN_BEYOND_MAX_PARAMETER_SOMETIMES
				|| gpGlobals->g.PlayerRoles.rgwCanBeyondMaxPara[w] != 0
#endif
				)
			{
				fLevelUp = TRUE;
				PAL_PlayerLevelUp(w, 1);

				gpGlobals->g.PlayerRoles.rgwHP[w] = gpGlobals->g.PlayerRoles.rgwMaxHP[w];
				gpGlobals->g.PlayerRoles.rgwMP[w] = gpGlobals->g.PlayerRoles.rgwMaxMP[w];
			}
		}

		gpGlobals->Exp.rgPrimaryExp[w].wExp = (WORD)min(0xFFFF, dwExp);

		if (fLevelUp)
		{
			// Player has gained a level. Show the message
			PAL_CreateSingleLineBox(PAL_XY(80, 0), 10, FALSE);
			PAL_CreateBox(PAL_XY(82, 32), 7, 8, 1, FALSE);

			PAL_DrawText(PAL_GetWord(gpGlobals->g.PlayerRoles.rgwName[w]), PAL_XY(110, 10), 0,
				FALSE, FALSE);
			PAL_DrawText(PAL_GetWord(STATUS_LABEL_LEVEL), PAL_XY(110 + 16 * 3, 10), 0, FALSE, FALSE);
			PAL_DrawText(PAL_GetWord(BATTLEWIN_LEVELUP_LABEL), PAL_XY(110 + 16 * 5, 10), 0, FALSE, FALSE);

			for (j = 0; j < 8; j++)
			{
				PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, SPRITENUM_ARROW),
					gpScreen, PAL_XY(183, 48 + 18 * j));
			}

			PAL_DrawText(PAL_GetWord(STATUS_LABEL_LEVEL), PAL_XY(100, 44), BATTLEWIN_LEVELUP_LABEL_COLOR,
				TRUE, FALSE);
			PAL_DrawText(PAL_GetWord(STATUS_LABEL_HP), PAL_XY(100, 62), BATTLEWIN_LEVELUP_LABEL_COLOR,
				TRUE, FALSE);
			PAL_DrawText(PAL_GetWord(STATUS_LABEL_MP), PAL_XY(100, 80), BATTLEWIN_LEVELUP_LABEL_COLOR,
				TRUE, FALSE);
			PAL_DrawText(PAL_GetWord(STATUS_LABEL_ATTACKPOWER), PAL_XY(100, 98), BATTLEWIN_LEVELUP_LABEL_COLOR,
				TRUE, FALSE);
			PAL_DrawText(PAL_GetWord(STATUS_LABEL_MAGICPOWER), PAL_XY(100, 116), BATTLEWIN_LEVELUP_LABEL_COLOR,
				TRUE, FALSE);
			PAL_DrawText(PAL_GetWord(STATUS_LABEL_RESISTANCE), PAL_XY(100, 134), BATTLEWIN_LEVELUP_LABEL_COLOR,
				TRUE, FALSE);
			PAL_DrawText(PAL_GetWord(STATUS_LABEL_DEXTERITY), PAL_XY(100, 152), BATTLEWIN_LEVELUP_LABEL_COLOR,
				TRUE, FALSE);
			PAL_DrawText(PAL_GetWord(STATUS_LABEL_FLEERATE), PAL_XY(100, 170), BATTLEWIN_LEVELUP_LABEL_COLOR,
				TRUE, FALSE);

			//
			// Draw the original stats and stats after level up
			//显示角色升级之前和升级之后的状态
			PAL_DrawNumber(OrigPlayerRoles.rgwLevel[w], 4, PAL_XY(133, 47),
				kNumColorYellow, kNumAlignRight);
			PAL_DrawNumber(gpGlobals->g.PlayerRoles.rgwLevel[w], 4, PAL_XY(195, 47),
				kNumColorYellow, kNumAlignRight);

			PAL_DrawNumber(OrigPlayerRoles.rgwHP[w], 4, PAL_XY(133, 64),
				kNumColorYellow, kNumAlignRight);
			PAL_DrawNumber(OrigPlayerRoles.rgwMaxHP[w], 4, PAL_XY(154, 68),
				kNumColorBlue, kNumAlignRight);
			PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, SPRITENUM_SLASH), gpScreen,
				PAL_XY(156, 66));
			PAL_DrawNumber(gpGlobals->g.PlayerRoles.rgwHP[w], 4, PAL_XY(195, 64),
				kNumColorYellow, kNumAlignRight);
			PAL_DrawNumber(gpGlobals->g.PlayerRoles.rgwMaxHP[w], 4, PAL_XY(216, 68),
				kNumColorBlue, kNumAlignRight);
			PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, SPRITENUM_SLASH), gpScreen,
				PAL_XY(218, 66));

			PAL_DrawNumber(OrigPlayerRoles.rgwMP[w], 4, PAL_XY(133, 82),
				kNumColorYellow, kNumAlignRight);
			PAL_DrawNumber(OrigPlayerRoles.rgwMaxMP[w], 4, PAL_XY(154, 86),
				kNumColorBlue, kNumAlignRight);
			PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, SPRITENUM_SLASH), gpScreen,
				PAL_XY(156, 84));
			PAL_DrawNumber(gpGlobals->g.PlayerRoles.rgwMP[w], 4, PAL_XY(195, 82),
				kNumColorYellow, kNumAlignRight);
			PAL_DrawNumber(gpGlobals->g.PlayerRoles.rgwMaxMP[w], 4, PAL_XY(216, 86),
				kNumColorBlue, kNumAlignRight);
			PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, SPRITENUM_SLASH), gpScreen,
				PAL_XY(218, 84));

			PAL_DrawNumber(OrigPlayerRoles.rgwAttackStrength[w] + PAL_GetPlayerAttackStrength(w) -
				gpGlobals->g.PlayerRoles.rgwAttackStrength[w],
				4, PAL_XY(133, 101), kNumColorYellow, kNumAlignRight);
			PAL_DrawNumber(PAL_GetPlayerAttackStrength(w), 4, PAL_XY(195, 101),
				kNumColorYellow, kNumAlignRight);

			PAL_DrawNumber(OrigPlayerRoles.rgwMagicStrength[w] + PAL_GetPlayerMagicStrength(w) -
				gpGlobals->g.PlayerRoles.rgwMagicStrength[w],
				4, PAL_XY(133, 119), kNumColorYellow, kNumAlignRight);
			PAL_DrawNumber(PAL_GetPlayerMagicStrength(w), 4, PAL_XY(195, 119),
				kNumColorYellow, kNumAlignRight);

			PAL_DrawNumber(OrigPlayerRoles.rgwDefense[w] + PAL_GetPlayerDefense(w) -
				gpGlobals->g.PlayerRoles.rgwDefense[w],
				4, PAL_XY(133, 137), kNumColorYellow, kNumAlignRight);
			PAL_DrawNumber(PAL_GetPlayerDefense(w), 4, PAL_XY(195, 137),
				kNumColorYellow, kNumAlignRight);

			PAL_DrawNumber(OrigPlayerRoles.rgwDexterity[w] + PAL_GetPlayerDexterity(w) -
				gpGlobals->g.PlayerRoles.rgwDexterity[w],
				4, PAL_XY(133, 155), kNumColorYellow, kNumAlignRight);
			PAL_DrawNumber(PAL_GetPlayerDexterity(w), 4, PAL_XY(195, 155),
				kNumColorYellow, kNumAlignRight);

			PAL_DrawNumber(OrigPlayerRoles.rgwFleeRate[w] + PAL_GetPlayerFleeRate(w) -
				gpGlobals->g.PlayerRoles.rgwFleeRate[w],
				4, PAL_XY(133, 173), kNumColorYellow, kNumAlignRight);
			PAL_DrawNumber(PAL_GetPlayerFleeRate(w), 4, PAL_XY(195, 173),
				kNumColorYellow, kNumAlignRight);

			//
			// Update the screen and wait for key
			//更新换面，等待按键
			VIDEO_UpdateScreen(&rect1);
			PAL_WaitForKey(3000);

			OrigPlayerRoles = gpGlobals->g.PlayerRoles;
		}

		//
		// Increasing of other hidden levels
		//
		iTotalCount = 0;

		iTotalCount += gpGlobals->Exp.rgAttackExp[w].wCount;
		iTotalCount += gpGlobals->Exp.rgDefenseExp[w].wCount;
		iTotalCount += gpGlobals->Exp.rgDexterityExp[w].wCount;
		iTotalCount += gpGlobals->Exp.rgFleeExp[w].wCount;
		iTotalCount += gpGlobals->Exp.rgHealthExp[w].wCount;
		iTotalCount += gpGlobals->Exp.rgMagicExp[w].wCount;
		iTotalCount += gpGlobals->Exp.rgMagicPowerExp[w].wCount;

		if (iTotalCount > 0)
		{
#ifdef EDIT_EXP_CALCULATION			
			EXPERIENCE *pExp = (EXPERIENCE *)(&gpGlobals->Exp);
			WORD *pPL = (WORD *)(&gpGlobals->g.PlayerRoles);
			WORD *pOPL = (WORD *)(&OrigPlayerRoles);

			EXPERIENCE rgTempExp;
			WORD wTempPlayerPara = 0;
			WORD wTempOPlayerPara = 0;

			for (j = HealthExp; j <= FleeExp; j++)
			{
				rgTempExp = pExp[j * MAX_PLAYER_ROLES + w];
				dwExp = g_Battle.rgPlayer[i].dwExpGained;
				dwExp *= rgTempExp.wCount * 2;
				dwExp /= iTotalCount;
#ifdef GAIN_MORE_HIDDEN_EXP
				dwExp *= 1 + min(1, iTotalCount / 100.0);
#endif
				dwExp += rgTempExp.wExp;

				while (dwExp >= PAL_New_GetLevelUpBaseExp(rgTempExp.wLevel))
				{
					dwExp -= PAL_New_GetLevelUpBaseExp(rgTempExp.wLevel);
					pPL[ExpTypeToPlayerPara[j] * MAX_PLAYER_ROLES + w] += RandomLong(1, 2);

					pExp[j * MAX_PLAYER_ROLES + w].wLevel++;
					if (rgTempExp.wLevel > MAX_LEVELS
#ifdef CAN_BEYOND_MAX_PARAMETER_SOMETIMES
						&& gpGlobals->g.PlayerRoles.rgwCanBeyondMaxPara[w] == 0
#endif
						)
					{
						pExp[j * MAX_PLAYER_ROLES + w].wLevel = MAX_LEVELS;
					}
					rgTempExp = pExp[j * MAX_PLAYER_ROLES + w];
				}
				pExp[j * MAX_PLAYER_ROLES + w].wExp = (WORD)min(0xFFFF, dwExp);

				wTempPlayerPara = pPL[ExpTypeToPlayerPara[j] * MAX_PLAYER_ROLES + w];
				wTempOPlayerPara = pOPL[ExpTypeToPlayerPara[j] * MAX_PLAYER_ROLES + w];
				if (wTempPlayerPara > wTempOPlayerPara)
				{
					PAL_CreateSingleLineBox(PAL_XY(83, 60), 8, FALSE);
					PAL_DrawText(PAL_GetWord(gpGlobals->g.PlayerRoles.rgwName[w]), PAL_XY(95, 70), 0, FALSE, FALSE);
					PAL_DrawText(PAL_GetWord(ExpTypeToLabel[j]), PAL_XY(143, 70), 0, FALSE, FALSE);
					PAL_DrawText(PAL_GetWord(BATTLEWIN_LEVELUP_LABEL), PAL_XY(175, 70), 0, FALSE, FALSE);
					PAL_DrawNumber(wTempPlayerPara - wTempOPlayerPara, 5, PAL_XY(188, 74), kNumColorYellow, kNumAlignRight);
					VIDEO_UpdateScreen(&rect);
					PAL_WaitForKey(3000);
				}
			}
#else

#define CHECK_HIDDEN_EXP(expname, statname, label)          \
				{                                                           \
	dwExp = g_Battle.iExpGained;                             \
	dwExp *= gpGlobals->Exp.expname[w].wCount;               \
	dwExp /= iTotalCount;                                    \
	dwExp *= 2;                                              \
															\
	dwExp += gpGlobals->Exp.expname[w].wExp;                 \
                                                            \
                                                            \
																															while (dwExp >= gpGlobals->g.rgLevelUpExp[min(MAX_LEVELS, gpGlobals->Exp.expname[w].wLevel)]) \
																																																{																			\
		dwExp -= gpGlobals->g.rgLevelUpExp[gpGlobals->Exp.expname[w].wLevel]; \
		gpGlobals->g.PlayerRoles.statname[w] += RandomLong(1, 2); \
		if (gpGlobals->Exp.expname[w].wLevel < MAX_LEVELS)    \
						{								 \
			gpGlobals->Exp.expname[w].wLevel++;                \
						}								\
																																																}                                                        \
                                                            \
   gpGlobals->Exp.expname[w].wExp = (WORD)dwExp;            \
                                                            \
   if (gpGlobals->g.PlayerRoles.statname[w] !=              \
      OrigPlayerRoles.statname[w])                          \
					{                                                        \
      PAL_CreateSingleLineBox(PAL_XY(83, 60), 8, FALSE);    \
      PAL_DrawText(PAL_GetWord(gpGlobals->g.PlayerRoles.rgwName[w]), PAL_XY(95, 70), \
         0, FALSE, FALSE);                                  \
      PAL_DrawText(PAL_GetWord(label), PAL_XY(143, 70),     \
         0, FALSE, FALSE);                                  \
      PAL_DrawText(PAL_GetWord(BATTLEWIN_LEVELUP_LABEL), PAL_XY(175, 70),  \
         0, FALSE, FALSE);                                  \
      PAL_DrawNumber(gpGlobals->g.PlayerRoles.statname[w] - \
         OrigPlayerRoles.statname[w],                       \
         5, PAL_XY(188, 74), kNumColorYellow, kNumAlignRight); \
      VIDEO_UpdateScreen(&rect);                            \
      PAL_WaitForKey(3000);                                 \
					}			 \
				}					

			CHECK_HIDDEN_EXP(rgHealthExp, rgwMaxHP, STATUS_LABEL_HP);
			CHECK_HIDDEN_EXP(rgMagicExp, rgwMaxMP, STATUS_LABEL_MP);
			CHECK_HIDDEN_EXP(rgAttackExp, rgwAttackStrength, STATUS_LABEL_ATTACKPOWER);
			CHECK_HIDDEN_EXP(rgMagicPowerExp, rgwMagicStrength, STATUS_LABEL_MAGICPOWER);
			CHECK_HIDDEN_EXP(rgDefenseExp, rgwDefense, STATUS_LABEL_RESISTANCE);
			CHECK_HIDDEN_EXP(rgDexterityExp, rgwDexterity, STATUS_LABEL_DEXTERITY);
			CHECK_HIDDEN_EXP(rgFleeExp, rgwFleeRate, STATUS_LABEL_FLEERATE);

#undef CHECK_HIDDEN_EXP

#endif
		}

		// 学习当前等级的所有法术
		j = 0;
		LEVELUPMAGIC tempMagic;
		while (j < gpGlobals->g.nLevelUpMagic)
		{
			tempMagic = gpGlobals->g.lprgLevelUpMagic[j].m[min(w, 5)];

			if (tempMagic.wMagic == 0 || tempMagic.wLevel > gpGlobals->g.PlayerRoles.rgwLevel[w])
			{
				j++;
				continue;
			}

			if (PAL_AddMagic(w, tempMagic.wMagic))
			{
				PAL_CreateSingleLineBox(PAL_XY(65, 105), 10, FALSE);

				PAL_DrawText(PAL_GetWord(gpGlobals->g.PlayerRoles.rgwName[w]),
					PAL_XY(75, 115), 0, FALSE, FALSE);
				PAL_DrawText(PAL_GetWord(BATTLEWIN_ADDMAGIC_LABEL), PAL_XY(75 + 16 * 3, 115),
					0, FALSE, FALSE);
				PAL_DrawText(PAL_GetWord(tempMagic.wMagic),
					PAL_XY(75 + 16 * 5, 115), 0x1B, FALSE, FALSE);

				VIDEO_UpdateScreen(&rect);
				PAL_WaitForKey(3000);
			}
			j++;
		}
		}

	// 运行战后脚本
	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		PAL_RunTriggerScript(g_Battle.rgEnemy[i].wScriptOnBattleEnd, i);
	}

	// 每一场战斗后自动恢复
	WORD wPercent = 0;
	WORD rgwLevel = 0;
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		w = gpGlobals->rgParty[i].wPlayerRole;
		rgwLevel = gpGlobals->g.PlayerRoles.rgwLevel[w];
		wPercent = 0;

#ifndef EDIT_RECOVER_VALUE_AFTER_BATTLE
		Pal_New_RecoverAfterBattle(w, 50);
#else
		if (gpGlobals->g.PlayerRoles.rgwHP[w] == 0)
		{
			wPercent = 10;
		}
		else if (g_Battle.iExpGained >= 0)
		{
			wPercent = min(4294967, g_Battle.iExpGained) * 500 / max(1, PAL_New_GetLevelUpExp(rgwLevel));
			wPercent = min(wPercent, 50);
		}

		if (g_Battle.BattleResult == kBattleResultEnemyFleed)
		{
			wPercent += 10;
		}

#ifdef STRENGTHEN_PLAYER
		if (w == RoleID_LinYueRu)
		{
			wPercent += min(20, rgwLevel);
		}
#endif
		Pal_New_RecoverAfterBattle(w, wPercent);
#endif
	}
	}

VOID 
PAL_BattleEnemyEscape(
VOID
)
/*++
  目的：    Enemy flee the battle.
  敌人逃跑。
  参数：    无。
  返回值：    无。
  --*/
{
	int j, x, y, w;
	BOOL f = TRUE;

	SOUND_Play(45);

	//
	// Show the animation
	//显示逃跑动画
	while (f)
	{
		f = FALSE;

		for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
		{
			if (g_Battle.rgEnemy[j].wObjectID == 0)
			{
				continue;
			}

			x = PAL_X(g_Battle.rgEnemy[j].pos) - 5;
			y = PAL_Y(g_Battle.rgEnemy[j].pos);

			g_Battle.rgEnemy[j].pos = PAL_XY(x, y);

			w = PAL_RLEGetWidth(PAL_SpriteGetFrame(g_Battle.rgEnemy[j].lpSprite, 0));

			if (x + w > 0)
			{
				f = TRUE;
			}
		}

		PAL_BattleMakeScene();
		SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);
		VIDEO_UpdateScreen(NULL);

		UTIL_Delay(10);
	}

	UTIL_Delay(500);
	g_Battle.BattleResult = kBattleResultTerminated;
}

VOID 
PAL_BattlePlayerEscape(
VOID
)
/*++
  目的：    Player flee the battle.
  玩家在战斗中逃跑。
  参数：    无。
  返回值：    无。
  --*/
{
	int         i, j;
	WORD        wPlayerRole;

	SOUND_Play(45);

	PAL_BattleUpdateFighters();

	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;

		if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] > 0)
		{
			g_Battle.rgPlayer[i].wCurrentFrame = 0;
		}
	}

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
		{
			wPlayerRole = gpGlobals->rgParty[j].wPlayerRole;

			if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] > 0)
			{
				//
				// TODO: This is still not the same as the original game
				//
				switch (j)
				{
					case 0:
						if (gpGlobals->wMaxPartyMemberIndex > 0)
						{
							g_Battle.rgPlayer[j].pos =
								PAL_XY(PAL_X(g_Battle.rgPlayer[j].pos) + 4,
								PAL_Y(g_Battle.rgPlayer[j].pos) + 6);
							break;
						}

					case 1:
						g_Battle.rgPlayer[j].pos =
							PAL_XY(PAL_X(g_Battle.rgPlayer[j].pos) + 4,
							PAL_Y(g_Battle.rgPlayer[j].pos) + 4);
						break;

					case 2:
						g_Battle.rgPlayer[j].pos =
							PAL_XY(PAL_X(g_Battle.rgPlayer[j].pos) + 6,
							PAL_Y(g_Battle.rgPlayer[j].pos) + 3);
						break;

					case 3:
						g_Battle.rgPlayer[j].pos =
							PAL_XY(PAL_X(g_Battle.rgPlayer[j].pos) + 7,
							PAL_Y(g_Battle.rgPlayer[j].pos) + 2);
						break;

					case 4:
						g_Battle.rgPlayer[j].pos =
							PAL_XY(PAL_X(g_Battle.rgPlayer[j].pos) + 8,
							PAL_Y(g_Battle.rgPlayer[j].pos) + 1);
						break;

					default:
						assert(FALSE); // Not possible
						break;
				}
			}
		}

		PAL_BattleDelay(1, 0, FALSE);
	}

	//
	// Remove all players from the screen
	// 从屏幕中移走所有角色
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		g_Battle.rgPlayer[i].pos = PAL_XY(9999, 9999);
	}

	PAL_BattleDelay(1, 0, FALSE);

	g_Battle.BattleResult = kBattleResultFleed;
}

BATTLERESULT
PAL_StartBattle(
WORD			wEnemyTeam,
BOOL			fIsBoss
)
/*++
  目的：    Start a battle.
  开始战斗。
  参数：    [IN]  wEnemyTeam - 敌人队伍人数。	the number of the enemy team.
  [IN]  fIsBoss - boss的fIsBoss值为TRUE（不允许逃跑）。
  返回值：    战斗结果。
  --*/
{
	int            i, j;
	WORD           w, wPrevWaveLevel;
	SHORT          sPrevWaveProgression;

	//
	// Set the screen waving effects
	//设置画面波浪效果
	wPrevWaveLevel = gpGlobals->wScreenWave;
	sPrevWaveProgression = gpGlobals->sWaveProgression;

	gpGlobals->sWaveProgression = 0;
	gpGlobals->wScreenWave = gpGlobals->g.lprgBattleField[gpGlobals->wNumBattleField].wScreenWave;


	WORD *p = (WORD *)(&gpGlobals->g.PlayerRoles);
	for (j = Para_MaxHP; j <= Para_FleeRate; j++)
	{
		for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
		{
			w = gpGlobals->rgParty[i].wPlayerRole;
#ifdef CAN_BEYOND_MAX_PARAMETER_SOMETIMES
			if (gpGlobals->g.PlayerRoles.rgwCanBeyondMaxPara[w] != 0)
			{
				continue;
			}
#endif
			p[j * MAX_PLAYER_ROLES + w] = min(p[j * MAX_PLAYER_ROLES + w], MAX_PARAMETER + MAX_PARAMETER_EXTRA);
		}
	}

	// Make sure everyone in the party is alive, also clear all hidden EXP count records
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		w = gpGlobals->rgParty[i].wPlayerRole;

		gpGlobals->g.PlayerRoles.rgwHP[w] = min(gpGlobals->g.PlayerRoles.rgwHP[w], gpGlobals->g.PlayerRoles.rgwMaxHP[w]);
		gpGlobals->g.PlayerRoles.rgwMP[w] = min(gpGlobals->g.PlayerRoles.rgwMP[w], gpGlobals->g.PlayerRoles.rgwMaxMP[w]);
		gpGlobals->g.PlayerRoles.rgwHP[w] = max(gpGlobals->g.PlayerRoles.rgwHP[w], 1);

		gpGlobals->rgPlayerStatus[w][kStatusPuppet] = 0;
		gpGlobals->Exp.rgHealthExp[w].wCount = 0;
		gpGlobals->Exp.rgMagicExp[w].wCount = 0;
		gpGlobals->Exp.rgAttackExp[w].wCount = 0;
		gpGlobals->Exp.rgMagicPowerExp[w].wCount = 0;
		gpGlobals->Exp.rgDefenseExp[w].wCount = 0;
		gpGlobals->Exp.rgDexterityExp[w].wCount = 0;
		gpGlobals->Exp.rgFleeExp[w].wCount = 0;
	}

	// 初始化
	memset(&g_Battle, 0, sizeof(g_Battle));

	// Clear all item-using records
	for (i = 0; i < MAX_INVENTORY; i++)
	{
		gpGlobals->rgInventory[i].nAmountInUse = 0;
	}

	// Store all enemies
	for (i = 0; i < MAX_ENEMIES_IN_TEAM; i++)
	{
		memset(&(g_Battle.rgEnemy[i]), 0, sizeof(BATTLEENEMY));
		w = gpGlobals->g.lprgEnemyTeam[wEnemyTeam].rgwEnemy[i];

		if (w == 0xFFFF)
		{
			break;
		}

		if (w != 0)
		{
			g_Battle.rgEnemy[i].e = gpGlobals->g.lprgEnemy[gpGlobals->g.rgObject[w].enemy.wEnemyID];
			g_Battle.rgEnemy[i].dwActualHealth = g_Battle.rgEnemy[i].e.wHealth;
			g_Battle.rgEnemy[i].dwMaxHealth = g_Battle.rgEnemy[i].dwActualHealth;

#ifdef STRENGTHEN_ENEMY
			g_Battle.rgEnemy[i] = PAL_New_StrengthenEnemy(g_Battle.rgEnemy[i]);
#endif
			g_Battle.rgEnemy[i].wObjectID = w;
			g_Battle.rgEnemy[i].state = kFighterWait;
			g_Battle.rgEnemy[i].wScriptOnTurnStart = gpGlobals->g.rgObject[w].enemy.wScriptOnTurnStart;
			g_Battle.rgEnemy[i].wScriptOnBattleEnd = gpGlobals->g.rgObject[w].enemy.wScriptOnBattleEnd;
			g_Battle.rgEnemy[i].wScriptOnReady = gpGlobals->g.rgObject[w].enemy.wScriptOnReady;
			g_Battle.rgEnemy[i].iColorShift = 0;
#ifndef PAL_CLASSIC
			g_Battle.rgEnemy[i].flTimeMeter = 50;
			//flTimeMeter：时间条（满条即可行动）
			//
			// HACK: Otherwise the black thief lady will be too hard to beat
			//削减女飞贼的身法值
			if (g_Battle.rgEnemy[i].e.wDexterity == 164)
			{
				g_Battle.rgEnemy[i].e.wDexterity /= ((gpGlobals->wMaxPartyMemberIndex == 0) ? 6 : 3);
			}

			//
			// HACK: Heal up automatically for final boss
			//最终boss战前自动回复体力和真气
			if (g_Battle.rgEnemy[i].dwActualHealth > 30000)
			{
				for (w = 0; w < MAX_PLAYER_ROLES; w++)
				{
					gpGlobals->g.PlayerRoles.rgwHP[w] = gpGlobals->g.PlayerRoles.rgwMaxHP[w];
					gpGlobals->g.PlayerRoles.rgwMP[w] = gpGlobals->g.PlayerRoles.rgwMaxMP[w];
				}
			}

			//
			// Yet another HACKs
			//
			if ((SHORT)g_Battle.rgEnemy[i].e.wDexterity == -32)
			{
				g_Battle.rgEnemy[i].e.wDexterity = 0; // for Grandma Knife //菜刀婆婆
			}
			else if (g_Battle.rgEnemy[i].e.wDexterity == 20)
			{
				//
				// for Fox Demon
				//
				if (gpGlobals->g.PlayerRoles.rgwLevel[0] < 15)
				{
					g_Battle.rgEnemy[i].e.wDexterity = 8;
				}
				else if (gpGlobals->g.PlayerRoles.rgwLevel[4] > 28 ||
					gpGlobals->Exp.rgPrimaryExp[4].wExp > 0)
				{
					g_Battle.rgEnemy[i].e.wDexterity = 60;
				}
			}
			else if (g_Battle.rgEnemy[i].e.wExp == 250 &&
				g_Battle.rgEnemy[i].e.wCash == 1100)
			{
				g_Battle.rgEnemy[i].e.wDexterity += 12; // for Snake Demon
			}
			else if ((SHORT)g_Battle.rgEnemy[i].e.wDexterity == -60)
			{
				g_Battle.rgEnemy[i].e.wDexterity = 15; // for Spider
			}
			else if ((SHORT)g_Battle.rgEnemy[i].e.wDexterity == -30)
			{
				g_Battle.rgEnemy[i].e.wDexterity = (WORD)-10; // for Stone Head
			}
			else if ((SHORT)g_Battle.rgEnemy[i].e.wDexterity == -16)
			{
				g_Battle.rgEnemy[i].e.wDexterity = 0; // for Zombie
			}
			else if ((SHORT)g_Battle.rgEnemy[i].e.wDexterity == -20)
			{
				g_Battle.rgEnemy[i].e.wDexterity = -8; // for Flower Demon
			}
			else if (g_Battle.rgEnemy[i].e.wLevel < 20 &&
				gpGlobals->wNumScene >= 0xD8 && gpGlobals->wNumScene <= 0xE2)
			{
				//
				// for low-level monsters in the Cave of Trial
				//
				g_Battle.rgEnemy[i].e.wLevel += 15;
				g_Battle.rgEnemy[i].e.wDexterity += 25;
			}
			else if (gpGlobals->wNumScene == 0x90)
			{
				g_Battle.rgEnemy[i].e.wDexterity += 25; // for Tower Dragons
			}
			else if (g_Battle.rgEnemy[i].e.wLevel == 2 &&
				g_Battle.rgEnemy[i].e.wCash == 48)
			{
				g_Battle.rgEnemy[i].e.wDexterity += 8; // for Miao Fists
			}
			else if (g_Battle.rgEnemy[i].e.wLevel == 4 &&
				g_Battle.rgEnemy[i].e.wCash == 240)
			{
				g_Battle.rgEnemy[i].e.wDexterity += 18; // for Fat Miao
			}
			else if (g_Battle.rgEnemy[i].e.wLevel == 16 &&
				g_Battle.rgEnemy[i].e.wMagicRate == 4 &&
				g_Battle.rgEnemy[i].e.wAttackEquivItemRate == 4)
			{
				g_Battle.rgEnemy[i].e.wDexterity += 50; // for Black Spider
			}
#endif
		}
		}

	g_Battle.wMaxEnemyIndex = i - 1;

	// Store all players
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		g_Battle.rgPlayer[i].flTimeMeter = 15.0f;
#ifndef PAL_CLASSIC
		g_Battle.rgPlayer[i].flTimeSpeedModifier = 2.0f;
		g_Battle.rgPlayer[i].sTurnOrder = -1;
#endif
		g_Battle.rgPlayer[i].wHidingTime = 0;
		g_Battle.rgPlayer[i].state = kFighterWait;
		g_Battle.rgPlayer[i].action.sTarget = -1;
		g_Battle.rgPlayer[i].fDefending = FALSE;
		g_Battle.rgPlayer[i].wCurrentFrame = 0;
		g_Battle.rgPlayer[i].iColorShift = FALSE;

#ifdef EDIT_EXP_CALCULATION
		g_Battle.rgPlayer[i].dwExpGained = 0;
#endif
	}

	//
	// Load sprites and background
	//加载 和背景
	PAL_LoadBattleSprites();//将角色、敌人的形象读入各自的lpSprite
	PAL_LoadBattleBackground();//将地图读入lpBackground

	//
	// Create the surface for scene buffer
	//
	g_Battle.lpSceneBuf =
		SDL_CreateRGBSurface(gpScreen->flags & ~SDL_HWSURFACE, 320, 200, 8,//SDL_HWSURFACE在显示内存中创建surface 
		gpScreen->format->Rmask, gpScreen->format->Gmask,
		gpScreen->format->Bmask, gpScreen->format->Amask);

	if (g_Battle.lpSceneBuf == NULL)
	{
		TerminateOnError("PAL_StartBattle(): creating surface for scene buffer failed!");
	}
#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_SetSurfacePalette(g_Battle.lpSceneBuf, gpScreen->format->palette);
#else
	SDL_SetPalette(g_Battle.lpSceneBuf, SDL_PHYSPAL | SDL_LOGPAL, VIDEO_GetPalette(), 0, 256);
#endif

	PAL_UpdateEquipments();

	g_Battle.iExpGained = 0;
	g_Battle.iCashGained = 0;

	g_Battle.fIsBoss = fIsBoss;
	g_Battle.fEnemyCleared = FALSE;
	g_Battle.fEnemyMoving = FALSE;
	g_Battle.iHidingTime = 0;
	g_Battle.wMovingPlayerIndex = 0;

	g_Battle.UI.szMsg[0] = '\0';
	g_Battle.UI.szNextMsg[0] = '\0';
	g_Battle.UI.dwMsgShowTime = 0;
	g_Battle.UI.state = kBattleUIWait;
	g_Battle.UI.fAutoAttack = FALSE;
	g_Battle.UI.wSelectedIndex = 0;
	g_Battle.UI.wPrevEnemyTarget = 0;
	g_Battle.UI.wSelectedAction = 0;

	memset(g_Battle.UI.rgShowNum, 0, sizeof(g_Battle.UI.rgShowNum));

	g_Battle.lpSummonSprite = NULL;
	g_Battle.sBackgroundColorShift = 0;

	gpGlobals->fInBattle = TRUE;
	g_Battle.BattleResult = kBattleResultPreBattle;

	PAL_BattleUpdateFighters();

	//
	// Load the battle effect sprite.
	//
	i = PAL_MKFGetChunkSize(10, gpGlobals->f.fpDATA);
	g_Battle.lpEffectSprite = UTIL_malloc(i);

	PAL_MKFReadChunk(g_Battle.lpEffectSprite, i, 10, gpGlobals->f.fpDATA);

#ifdef PAL_CLASSIC
	g_Battle.Phase = kBattlePhaseSelectAction;
	g_Battle.fRepeat = FALSE;
	g_Battle.fForce = FALSE;
	g_Battle.fFlee = FALSE;
#endif

#ifdef PAL_ALLOW_KEYREPEAT
	SDL_EnableKeyRepeat(120, 75);
#endif

	//
	// Run the main battle routine.
	//
	i = PAL_BattleMain();

#ifdef PAL_ALLOW_KEYREPEAT
	SDL_EnableKeyRepeat(0, 0);
	PAL_ClearKeyState();
	g_InputState.prevdir = kDirUnknown;
#endif

	//
	// Clear all item-using records
	//清除所有正在使用的物品数量
	for (w = 0; w < MAX_INVENTORY; w++)
	{
		gpGlobals->rgInventory[w].nAmountInUse = 0;
	}
	//
	// Clear all player status, poisons and temporary effects
	//清除所有角色状态，毒，和暂时性效果
	PAL_ClearAllPlayerStatus();
	for (w = 0; w < MAX_PLAYER_ROLES; w++)
	{
		PAL_CurePoisonByLevel(w, MAX_POISON_LEVEL);
		PAL_RemoveEquipmentEffect(w, kBodyPartExtra);
	}

	if (i == kBattleResultWon || i == kBattleResultEnemyFleed)
	{
		//
		// Player won the battle. Add the Experience points.
		//玩家获得胜利。增加玩家的经验值（调用PAL_BattleWon()函数）。
		PAL_BattleWon();
	}

	//
	// Free all the battle sprites
	//
	PAL_FreeBattleSprites();
	free(g_Battle.lpEffectSprite);

	//
	// Free the surfaces for the background picture and scene buffer
	//
	SDL_FreeSurface(g_Battle.lpBackground);
	SDL_FreeSurface(g_Battle.lpSceneBuf);

	g_Battle.lpBackground = NULL;
	g_Battle.lpSceneBuf = NULL;

	gpGlobals->fInBattle = FALSE;

	PAL_PlayMUS(gpGlobals->wNumMusic, TRUE, 1);

	//
	// Restore the screen waving effects
	//
	gpGlobals->sWaveProgression = sPrevWaveProgression;
	gpGlobals->wScreenWave = wPrevWaveLevel;

	return i;
	}

VOID
Pal_New_RecoverAfterBattle(
WORD			wPlayerRole,
WORD			wPercent
)
{
	WORD	wDiff = 0;
	WORD	wRecoverValue = 0;
	WORD	wMaxValue = 0;
	//
	// Recover automatically after each battle
	// 每一场战斗后自动恢复
	wPercent = min(100, wPercent);

	wDiff = max(0, gpGlobals->g.PlayerRoles.rgwMaxHP[wPlayerRole] - gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole]);
	wRecoverValue = min(wDiff * wPercent / 100, max(MAX_PARAMETER, gpGlobals->g.PlayerRoles.rgwMaxHP[wPlayerRole]) / 2);
	gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] += wRecoverValue;

	wDiff = max(0, gpGlobals->g.PlayerRoles.rgwMaxMP[wPlayerRole] - gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole]);
	wRecoverValue = min(wDiff * wPercent / 100, max(MAX_PARAMETER, gpGlobals->g.PlayerRoles.rgwMaxMP[wPlayerRole]) / 2);
	gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole] += wRecoverValue;
}

#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
BOOL
PAL_New_IsPlayerUsingMengshe(
WORD			wPlayerRole
)
{
	switch (wPlayerRole)
	{
		case RoleID_ZhaoLingEr:
			return (gpGlobals->rgEquipmentEffect[kBodyPartExtra].rgwSpriteNumInBattle[wPlayerRole] == 5);

		default:
			return FALSE;
	}
}

VOID
PAL_New_CheckMengsheAttriIncre(
WORD			wPlayerRole,
MENGSHECOUNT	msCount)
{
	BYTE i = 0;
	DWORD msExp = 0, msTotalCount = 0;
	DWORD msExpPart[8];

	memset(msExpPart, 0, sizeof(msExpPart));

	msTotalCount = PAL_New_GetMSTotalCount(msCount);
	if (msTotalCount == 0)
	{
		return;
	}

	msExp = g_Battle.iExpGained * min(msTotalCount / 3.0, 30);

	i = 0;
	msExpPart[i++] = g_Battle.iExpGained * min(20, msTotalCount) / 300;
	msExpPart[i++] = msExp * msCount.MaxHP / msTotalCount;
	msExpPart[i++] = msExp * msCount.MaxMP / msTotalCount;
	msExpPart[i++] = msExp * msCount.AttackStrength / msTotalCount;
	msExpPart[i++] = msExp * msCount.MagicStrength / msTotalCount;
	msExpPart[i++] = msExp * msCount.Defense / msTotalCount;
	msExpPart[i++] = msExp * msCount.Dexterity / msTotalCount;
	msExpPart[i++] = msExp * msCount.FleeRate / msTotalCount;

	i = 0;
	msExpPart[i++] += gpGlobals->Exp.rgPrimaryExp[wPlayerRole].wExp;
	msExpPart[i++] += gpGlobals->Exp.rgHealthExp[wPlayerRole].wExp;
	msExpPart[i++] += gpGlobals->Exp.rgMagicExp[wPlayerRole].wExp;
	msExpPart[i++] += gpGlobals->Exp.rgAttackExp[wPlayerRole].wExp;
	msExpPart[i++] += gpGlobals->Exp.rgMagicPowerExp[wPlayerRole].wExp;
	msExpPart[i++] += gpGlobals->Exp.rgDefenseExp[wPlayerRole].wExp;
	msExpPart[i++] += gpGlobals->Exp.rgDexterityExp[wPlayerRole].wExp;
	msExpPart[i++] += gpGlobals->Exp.rgFleeExp[wPlayerRole].wExp;

	for (i = 0; i < 8; i++)
	{
		msExpPart[i] = min(0xFFFF, msExpPart[i]);
	}

	i = 0;
	gpGlobals->Exp.rgPrimaryExp[wPlayerRole].wExp = msExpPart[i++];
	gpGlobals->Exp.rgHealthExp[wPlayerRole].wExp = msExpPart[i++];
	gpGlobals->Exp.rgMagicExp[wPlayerRole].wExp = msExpPart[i++];
	gpGlobals->Exp.rgAttackExp[wPlayerRole].wExp = msExpPart[i++];
	gpGlobals->Exp.rgMagicPowerExp[wPlayerRole].wExp = msExpPart[i++];
	gpGlobals->Exp.rgDefenseExp[wPlayerRole].wExp = msExpPart[i++];
	gpGlobals->Exp.rgDexterityExp[wPlayerRole].wExp = msExpPart[i++];
	gpGlobals->Exp.rgFleeExp[wPlayerRole].wExp = msExpPart[i++];
}

DWORD
PAL_New_GetMSTotalCount(
MENGSHECOUNT	msCount
)
{
	DWORD  msTotalCount = 0;
	msTotalCount += msCount.MaxHP;
	msTotalCount += msCount.MaxMP;
	msTotalCount += msCount.AttackStrength;
	msTotalCount += msCount.Defense;
	msTotalCount += msCount.Dexterity;
	msTotalCount += msCount.FleeRate;
	msTotalCount += msCount.MagicStrength;
	return msTotalCount;
}
#endif

#ifdef STRENGTHEN_ENEMY
BATTLEENEMY
PAL_New_StrengthenEnemy(
BATTLEENEMY			be
)
{
	FLOAT	fHPPerLevel = 0;
	FLOAT	fTimes = 0;
	DWORD	dwTempHp = 0;
	DWORD	dwTempExp = 0;
	DWORD	dwTempValue = 0;
	DWORD	dwTempLevel = 0;
	const WORD wOriginLevel = be.e.wLevel;

	INT index = PAL_New_GetPlayerIndexByPara(Para_Level, FALSE);
	WORD wPlayerRole = gpGlobals->rgParty[index].wPlayerRole;
	WORD wLevel = gpGlobals->g.PlayerRoles.rgwLevel[wPlayerRole];

	be.dwActualHealth = max(be.dwActualHealth, be.e.wHealth);

	//根据敌人的等级调整敌人血量
	if (be.e.wLevel > 99 && be.e.wHealth >= 65000)
	{
		be.dwActualHealth += 10000;
	}

	//根据我方角色的最低等级调整敌人血量
	if (max(0, wLevel - be.e.wLevel) >= 10)
	{
		dwTempLevel = wLevel - 10;
		if (gpGlobals->wMaxPartyMemberIndex != 0)
		{
			dwTempHp = PAL_New_EstimateEnemyHealthByLevel(dwTempLevel) * 0.2;
			fHPPerLevel = min(150, be.e.wHealth / (FLOAT)max(1, be.e.wLevel));
			fHPPerLevel = max(fHPPerLevel, min(150, be.e.wExp / (FLOAT)max(1, be.e.wLevel) * 10));
			dwTempHp += fHPPerLevel * dwTempLevel * 0.8;
		}
		else
		{
			dwTempHp = PAL_New_EstimateEnemyHealthByLevel(dwTempLevel) * 0.2;
			fHPPerLevel = min(100, be.e.wHealth / (FLOAT)max(1, be.e.wLevel));
			fHPPerLevel = max(fHPPerLevel, min(100, be.e.wExp / (FLOAT)max(1, be.e.wLevel) * 10));
			dwTempHp += fHPPerLevel * dwTempLevel * 0.6;
		}
		dwTempHp = max(be.dwActualHealth, dwTempHp);
		fTimes = dwTempHp / (FLOAT)(be.e.wHealth + 1);
		dwTempExp = be.e.wExp * fTimes * 0.75;
		dwTempExp = max(dwTempExp, be.e.wExp);
		dwTempExp = min(dwTempExp, 0xFFFF);
		dwTempValue = (be.e.wCollectValue == 0) ? 0 : be.e.wCollectValue * (1 + fTimes / 40) + 1;
	}
	be.dwActualHealth = max(be.dwActualHealth, dwTempHp);
	be.e.wExp = max(be.e.wExp, dwTempExp);
	be.e.wCollectValue = (be.e.wCollectValue == 0) ? 0 : max(be.e.wCollectValue, dwTempValue);
	be.e.wLevel = max(dwTempLevel, be.e.wLevel);

	//根据我方角色的数量调整敌人血量
	if (gpGlobals->wMaxPartyMemberIndex >= 3)
	{
		be.dwActualHealth += (gpGlobals->wMaxPartyMemberIndex - 2) * be.dwActualHealth / 3;
	}

#ifdef FINISH_GAME_MORE_ONE_TIME	
	be.dwActualHealth += gpGlobals->bFinishGameTime * 2000 * (1 + gpGlobals->wMaxPartyMemberIndex / 5.0);
	be.e.wExp += gpGlobals->bFinishGameTime * 1000;
	be.e.wCollectValue = (be.e.wCollectValue == 0) ? 0 : (be.e.wCollectValue + gpGlobals->bFinishGameTime);
	be.e.wLevel = max(gpGlobals->bFinishGameTime * 100 + wOriginLevel, dwTempLevel);
#endif 


#ifdef KO_ENEMY_FAST
	be.dwActualHealth = 1;
#endif 

	if (be.dwActualHealth > 100)
	{
		be.dwActualHealth -= be.dwActualHealth % 10;
	}
	be.dwMaxHealth = be.dwActualHealth;

	return be;
}
#endif


