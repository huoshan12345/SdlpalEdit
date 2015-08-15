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

extern WORD g_rgPlayerPos[5][5][2];

static int g_iCurMiscMenuItem = 0;
static int g_iCurSubMenuItem = 0;

VOID
PAL_PlayerInfoBox(
PAL_POS         pos,
WORD            wPlayerRole,
INT             iTimeMeter,
BYTE            bTimeMeterColor,
BOOL            fUpdate
)
/*++
  Purpose:    Show the player info box.
  显示玩家信息栏。
  Parameters:    [IN]  pos - the top-left corner position of the box.
  信息栏左上角的位置。
  [IN]  wPlayerRole - the player role ID to be shown.
  显示玩家角色的名称。
  [IN]  iTimeMeter - the value of time meter. 0 = empty, 100 = full.
  计时器的值。0位空，100为满。
  [IN]  bTimeMeterColor - the color of time meter.
  计时器的颜色
  [IN]  fUpdate - whether to update the screen area or not.
  是否更新屏幕区域
  Return value:    None.
  返回值：  无。
  --*/
{
	SDL_Rect        rect;
	BYTE            bPoisonColor;
	int             i, iPartyIndex;
	WORD            wMaxLevel, w;

	const BYTE      rgStatusPos[kStatusAll][2] =
	{
		{35, 19},  // confused	// 混乱——随机攻击友方单位
		{34, 0},   // slow		// 减速——速度变慢
		{54, 1},   // sleep		// 睡眠——无法行动
		{55, 20},  // silence		// 咒封——无法施放魔法
		{0, 0},    // puppet		// 傀儡——能持续攻击（仅对已死亡的角色有效）
		{0, 0},    // bravery		// 勇气——增加物理攻击的攻击力
		{0, 0},    // protect		// 防护——防御值上升
		{0, 0},    // haste		// 急速——速度提升
		{0, 0},    // dualattack	// 双重攻击
	};
	//各种状态对应显示的文字
	const WORD      rgwStatusWord[kStatusAll] =
	{
		0x1D,  // confused
		0x1B,  // slow
		0x1C,  // sleep
		0x1A,  // silence
		0x00,  // puppet
		0x00,  // bravery
		0x00,  // protect
		0x00,  // haste
		0x00,  // dualattack
	};
	//各种状态对应显示的文字的颜色
	const BYTE      rgbStatusColor[kStatusAll] =
	{
		0x5F,  // confused
		0x0E,  // slow
		0x0E,  // sleep
		0x3C,  // silence
		0x00,  // puppet
		0x00,  // bravery
		0x00,  // protect
		0x00,  // haste
		0x00,  // dualattack
	};

	//
	// Draw the box
	//
	PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, SPRITENUM_PLAYERINFOBOX),
		gpScreen, pos);

	//
	// Draw the player face
	//
	wMaxLevel = 0;
	bPoisonColor = 0xFF;

	for (iPartyIndex = 0; iPartyIndex <= gpGlobals->wMaxPartyMemberIndex; iPartyIndex++)
	{
		if (gpGlobals->rgParty[iPartyIndex].wPlayerRole == wPlayerRole)
		{
			break;
		}
	}
	// 根据每个角色所中的毒来更改角色脸部颜色
	if (iPartyIndex <= gpGlobals->wMaxPartyMemberIndex)
	{
		for (i = 0; i < MAX_POISONS; i++)//MAX_POISONS在每个角色身上同时有效的毒的最大数量，游戏规定为16。
		{
			w = gpGlobals->rgPoisonStatus[i][iPartyIndex].wPoisonID;
			if (w != 0 && gpGlobals->g.rgObject[w].poison.wColor != 0)
			{
				if (gpGlobals->g.rgObject[w].poison.wPoisonLevel >= wMaxLevel)
				{//找出等级最大的毒
					wMaxLevel = gpGlobals->g.rgObject[w].poison.wPoisonLevel;
					bPoisonColor = (BYTE)(gpGlobals->g.rgObject[w].poison.wColor);
				}
			}
		}
	}

	if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0)
	{
		//
		// Always use the black/white color for dead players
		// and do not use the time meter
		//
		bPoisonColor = 0;
		iTimeMeter = 0;
	}

	if (bPoisonColor == 0xFF)// 若角色未中毒
	{
		PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, SPRITENUM_PLAYERFACE_FIRST + wPlayerRole),
			gpScreen, PAL_XY(PAL_X(pos) - 2, PAL_Y(pos) - 4));
	}
	else// 若角色中毒
	{
		PAL_RLEBlitMonoColor(PAL_SpriteGetFrame(gpSpriteUI, SPRITENUM_PLAYERFACE_FIRST + wPlayerRole),
			gpScreen, PAL_XY(PAL_X(pos) - 2, PAL_Y(pos) - 4), bPoisonColor, 0);
	}

#ifndef PAL_CLASSIC
	//
	// Draw a border for the Time Meter
	//
	rect.x = PAL_X(pos) + 31;
	rect.y = PAL_Y(pos) + 4;
	rect.w = 1;
	rect.h = 6;
	SDL_FillRect(gpScreen, &rect, 0xBD);

	rect.x += 39;
	SDL_FillRect(gpScreen, &rect, 0xBD);

	rect.x = PAL_X(pos) + 32;
	rect.y = PAL_Y(pos) + 3;
	rect.w = 38;
	rect.h = 1;
	SDL_FillRect(gpScreen, &rect, 0xBD);

	rect.y += 7;
	SDL_FillRect(gpScreen, &rect, 0xBD);

	//
	// Draw the Time meter bar
	//
	if (iTimeMeter >= 100)
	{
		rect.x = PAL_X(pos) + 33;
		rect.y = PAL_Y(pos) + 6;
		rect.w = 36;
		rect.h = 2;
		SDL_FillRect(gpScreen, &rect, 0x2C);
	}
	else if (iTimeMeter > 0)
	{
		rect.x = PAL_X(pos) + 33;
		rect.y = PAL_Y(pos) + 5;
		rect.w = iTimeMeter * 36 / 100;
		rect.h = 4;
		SDL_FillRect(gpScreen, &rect, bTimeMeterColor);
	}
#endif

	//
	// Draw the HP and MP value
	//
#ifdef PAL_CLASSIC
	PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, SPRITENUM_SLASH), gpScreen,
		PAL_XY(PAL_X(pos) + 49, PAL_Y(pos) + 6));
	PAL_DrawNumber(gpGlobals->g.PlayerRoles.rgwMaxHP[wPlayerRole], 4,
		PAL_XY(PAL_X(pos) + 47, PAL_Y(pos) + 8), kNumColorYellow, kNumAlignRight);
	PAL_DrawNumber(gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole], 4,
		PAL_XY(PAL_X(pos) + 26, PAL_Y(pos) + 5), kNumColorYellow, kNumAlignRight);

	PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, SPRITENUM_SLASH), gpScreen,
		PAL_XY(PAL_X(pos) + 49, PAL_Y(pos) + 22));
	PAL_DrawNumber(gpGlobals->g.PlayerRoles.rgwMaxMP[wPlayerRole], 4,
		PAL_XY(PAL_X(pos) + 47, PAL_Y(pos) + 24), kNumColorCyan, kNumAlignRight);
	PAL_DrawNumber(gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole], 4,
		PAL_XY(PAL_X(pos) + 26, PAL_Y(pos) + 21), kNumColorCyan, kNumAlignRight);
#else
	PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, SPRITENUM_SLASH), gpScreen,
		PAL_XY(PAL_X(pos) + 49, PAL_Y(pos) + 14));
	PAL_DrawNumber(gpGlobals->g.PlayerRoles.rgwMaxHP[wPlayerRole], 4,
		PAL_XY(PAL_X(pos) + 47, PAL_Y(pos) + 16), kNumColorYellow, kNumAlignRight);
	PAL_DrawNumber(gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole], 4,
		PAL_XY(PAL_X(pos) + 26, PAL_Y(pos) + 13), kNumColorYellow, kNumAlignRight);

	PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, SPRITENUM_SLASH), gpScreen,
		PAL_XY(PAL_X(pos) + 49, PAL_Y(pos) + 24));
	PAL_DrawNumber(gpGlobals->g.PlayerRoles.rgwMaxMP[wPlayerRole], 4,
		PAL_XY(PAL_X(pos) + 47, PAL_Y(pos) + 26), kNumColorCyan, kNumAlignRight);
	PAL_DrawNumber(gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole], 4,
		PAL_XY(PAL_X(pos) + 26, PAL_Y(pos) + 23), kNumColorCyan, kNumAlignRight);
#endif

	//
	// Draw Statuses
	//
	if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] > 0)
	{
		for (i = 0; i < kStatusAll; i++)
		{
			if (gpGlobals->rgPlayerStatus[wPlayerRole][i] > 0 &&
				rgwStatusWord[i] != 0)
			{
				PAL_DrawText(PAL_GetWord(rgwStatusWord[i]),
					PAL_XY(PAL_X(pos) + rgStatusPos[i][0], PAL_Y(pos) + rgStatusPos[i][1]),
					rgbStatusColor[i], TRUE, FALSE);
			}
		}
	}

	//
	// Update the screen area if needed
	//
	if (fUpdate)
	{
		rect.x = PAL_X(pos) - 2;
		rect.y = PAL_Y(pos) - 4;
		rect.w = 77;
		rect.h = 39;

		VIDEO_UpdateScreen(&rect);
	}
}

static BOOL
PAL_BattleUIIsActionValid(
BATTLEUIACTION         ActionType
)
/*++
  Purpose:    Check if the specified action is valid.
  功能：  检查选定的动作是否有效。
  Parameters:    [IN]  ActionType - the type of the action.
  动作类型
  Return value:    TRUE if the action is valid, FALSE if not.
  如果动作有效则为TRUE，否则为FALSE。
  --*/
{
	WORD     wPlayerRole, w;
	int      i;

	wPlayerRole = gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole;

	switch (ActionType)
	{
		case kBattleUIActionAttack:		// 直接攻击
		case kBattleUIActionMisc:		// 杂项
			break;

		case kBattleUIActionMagic:		// 仙术
			if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSilence] != 0)
			{ // 如果角色状态为咒封，则不能使用仙术
				return FALSE;
			}
			break;

		case kBattleUIActionCoopMagic:// 合体魔法
		{
			if (gpGlobals->wMaxPartyMemberIndex == 0)
			{		// 只有一个角色时，不能使用合体魔法
				return FALSE;
			}
			for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				w = gpGlobals->rgParty[i].wPlayerRole;

#ifndef PAL_CLASSIC
				if (gpGlobals->g.PlayerRoles.rgwHP[w] < gpGlobals->g.PlayerRoles.rgwMaxHP[w] / 5 ||
					gpGlobals->rgPlayerStatus[w][kStatusSleep] != 0 ||
					gpGlobals->rgPlayerStatus[w][kStatusConfused] != 0 ||
					gpGlobals->rgPlayerStatus[w][kStatusSilence] != 0 ||
					g_Battle.rgPlayer[i].flTimeMeter < 100 ||
					g_Battle.rgPlayer[i].state == kFighterAct)
#else
				if (PAL_IsPlayerDying(w) || !PAL_New_IfPlayerCanMove(w))
#endif
				{
					return FALSE;
				}
			}
			break;
		}
	}

	return TRUE;
}

static VOID
PAL_BattleUIDrawMiscMenu(
WORD       wCurrentItem,
BOOL       fConfirmed
)
/*++
  Purpose:    Draw the misc menu.
  画出杂项菜单。
  Parameters:    [IN]  wCurrentItem - the current selected menu item.
  现在选中的菜单项。
  [IN]  fConfirmed - TRUE if confirmed, FALSE if not.
  如果确定值为TRUE，否则为FALSE。
  Return value:    None.
  --*/
{
	INT			i, x, y;
	BYTE		bColor;
	BYTE		bItemNum;


#ifdef PAL_CLASSIC
	i = 0;
	x = 16;
	y = 14;
	MENUITEM rgMenuItem[] = {
		// value   label                     enabled   position
			{i++, BATTLEUI_LABEL_AUTO, TRUE, PAL_XY(x, y + (i * 18))},	//围攻
			{i++, BATTLEUI_LABEL_INVENTORY, TRUE, PAL_XY(x, y + (i * 18))},			//道具
			{i++, BATTLEUI_LABEL_DEFEND, TRUE, PAL_XY(x, y + (i * 18))},				//防御
			{i++, BATTLEUI_LABEL_FLEE, TRUE, PAL_XY(x, y + (i * 18))},				//逃跑
			{i++, BATTLEUI_LABEL_STATUS, TRUE, PAL_XY(x, y + (i * 18))},			//状态
#ifdef SHOW_DATA_IN_BATTLE
			{i++, BATTLEUI_LABEL_DATA, TRUE, PAL_XY(x, y + (i * 18))},				//数据
#endif	
#ifdef SHOW_ENEMY_STATUS
			{i++, BATTLEUI_LABEL_ENEMY_STATUS, TRUE, PAL_XY(x, y + (i * 18))},				//敌方状态
#endif
	};
#else
	MENUITEM rgMenuItem[] = {
		// value   label                   enabled   position
			{  0,      BATTLEUI_LABEL_ITEM,    TRUE,     PAL_XY(16, 32)  },
			{  1,      BATTLEUI_LABEL_DEFEND,  TRUE,     PAL_XY(16, 50)  },
			{  2,      BATTLEUI_LABEL_AUTO,    TRUE,     PAL_XY(16, 68)  },
			{  3,      BATTLEUI_LABEL_FLEE,    TRUE,     PAL_XY(16, 86)  },
			{  4,      BATTLEUI_LABEL_STATUS,  TRUE,     PAL_XY(16, 104) }
	};
#endif

	bItemNum = sizeof(rgMenuItem) / sizeof(MENUITEM);
	//
	// Draw the box
	//
	PAL_CreateBox(PAL_XY(2, 20), bItemNum - 1, 1, 0, FALSE);
	/*
		 PAL_CreateBox()函数
		 功能：		在屏幕上创建一个框
		 参数：		[IN]  pos - 框的位置。
		 [IN]  nRows - 框的行数。
		 [IN]  nColumns - 框的列数。
		 [IN]  iStyle - 框的类型(0 或 1)。
		 [IN]  fSaveScreen - whether save the used screen area or not.是否保存以使用的屏幕区域。
		 返回值：		指向BOX结构的指针。 		如果fSaveScreen值为false，则返回NULL。
		 */
	//
	// Draw the menu items
	//画出菜单项
	for (i = 0; i < bItemNum; i++)
	{
		bColor = MENUITEM_COLOR;

		if (i == wCurrentItem)
		{
			if (fConfirmed)
			{
				bColor = MENUITEM_COLOR_CONFIRMED;
			}
			else
			{
				bColor = MENUITEM_COLOR_SELECTED;
			}
		}

		PAL_DrawText(PAL_GetWord(rgMenuItem[i].wNumWord), rgMenuItem[i].pos, bColor, TRUE, FALSE);
		/*++
			功能：			在屏幕上画文本。
			参数：			[IN]  lpszText - 要画的文本。
			[IN]  pos - 文本的位置。
			[IN]  bColor - 文本的颜色。
			[IN]  fShadow - 文本是否有阴影效果。
			[IN]  fUpdate - 是否更新屏幕区域。
			返回值：			无。
			--*/
	}
}

static WORD
PAL_BattleUIMiscMenuUpdate(
VOID
)
/*++
  Purpose:    Update the misc menu.
  更新杂项菜单。
  Parameters:    None.
  Return value:    The selected item number. 0 if cancelled, 0xFFFF if not confirmed.
  选中的菜单项序号。如果取消则为0，如果未确定则为0xFFF。
  --*/
{
	int iMenuItemMaxIndex = 4;

#ifdef SHOW_DATA_IN_BATTLE
	iMenuItemMaxIndex++;
#endif

#ifdef SHOW_ENEMY_STATUS
	iMenuItemMaxIndex++;
#endif

	//
	// Draw the menu
	//
	PAL_BattleUIDrawMiscMenu(g_iCurMiscMenuItem, FALSE);

	//
	// Process inputs
	//
	if (g_InputState.dwKeyPress & (kKeyUp | kKeyLeft))
	{// 按上键和左键时菜单项序号减一
		g_iCurMiscMenuItem--;
		if (g_iCurMiscMenuItem < 0)
		{
			g_iCurMiscMenuItem = iMenuItemMaxIndex;
		}
	}
	else if (g_InputState.dwKeyPress & (kKeyDown | kKeyRight))
	{  // 按下键和右键时菜单项序号加一
		g_iCurMiscMenuItem++;
		if (g_iCurMiscMenuItem > iMenuItemMaxIndex) // 当菜单项序号最大时，再按下键或者右键其序号变为最最小
		{
			g_iCurMiscMenuItem = 0;
		}
	}
	else if (g_InputState.dwKeyPress & kKeySearch)
	{ // 按下左CTRL键时相当于回车键
		return g_iCurMiscMenuItem + 1;
	}
	else if (g_InputState.dwKeyPress & kKeyMenu)
	{ // 按下菜单键退出杂项菜单
		return 0;
	}

	return 0xFFFF;
}

static WORD
PAL_BattleUIMiscItemSubMenuUpdate(
VOID
)
/*++
  Purpose:    Update the item sub menu of the misc menu.
  更新杂项菜单的子菜单。
  Parameters:    None.
  Return value:    The selected item number. 0 if cancelled, 0xFFFF if not confirmed.
  选中的菜单项序号。如果取消则为0，如果未确定则为0xFFF。
  --*/
{
	int             i;
	BYTE            bColor;

	MENUITEM rgMenuItem[] = {
		// value   label                      enabled   position
			{0, BATTLEUI_LABEL_USEITEM, TRUE, PAL_XY(44, 62)},
			{1, BATTLEUI_LABEL_THROWITEM, TRUE, PAL_XY(44, 80)},
	};

	//
	// Draw the menu
	//
#ifdef PAL_CLASSIC
	PAL_BattleUIDrawMiscMenu(1, TRUE);
#else
	PAL_BattleUIDrawMiscMenu(0, TRUE);
#endif
	PAL_CreateBox(PAL_XY(30, 50), 1, 1, 0, FALSE);

	//
	// Draw the menu items
	//
	for (i = 0; i < 2; i++)
	{
		bColor = MENUITEM_COLOR;

		if (i == g_iCurSubMenuItem)
		{
			bColor = MENUITEM_COLOR_SELECTED;
		}

		PAL_DrawText(PAL_GetWord(rgMenuItem[i].wNumWord), rgMenuItem[i].pos, bColor,
			TRUE, FALSE);
	}

	//
	// Process inputs
	//
	if (g_InputState.dwKeyPress & (kKeyUp | kKeyLeft))
	{
		g_iCurSubMenuItem = 0;
	}
	else if (g_InputState.dwKeyPress & (kKeyDown | kKeyRight))
	{
		g_iCurSubMenuItem = 1;
	}
	else if (g_InputState.dwKeyPress & kKeySearch)
	{
		return g_iCurSubMenuItem + 1;
	}
	else if (g_InputState.dwKeyPress & kKeyMenu)
	{
		return 0;
	}

	return 0xFFFF;
}

VOID
PAL_BattleUIShowText(
LPCSTR        lpszText,
WORD          wDuration
)
/*++
  Purpose:    Show a text message in the battle.
  显示战斗中的文本信息。
  Parameters:    [IN]  lpszText - the text message to be shown.
  要显示的文本信息。
  [IN]  wDuration - the duration of the message, in milliseconds.
  信息的持续时间，以毫秒为单位
  Return value:    None.
  --*/
{
	if (SDL_GetTicks() < g_Battle.UI.dwMsgShowTime)
	{
		strcpy(g_Battle.UI.szNextMsg, lpszText);
		g_Battle.UI.wNextMsgDuration = wDuration;
	}
	else
	{
		strcpy(g_Battle.UI.szMsg, lpszText);
		g_Battle.UI.dwMsgShowTime = SDL_GetTicks() + wDuration;
	}
}

VOID
PAL_BattleUIPlayerReady(
WORD          wPlayerIndex
)
/*++
  Purpose:

  Start the action selection menu of the specified player.
  开始选定角色的动作选择菜单。
  Parameters:    [IN]  wPlayerIndex - the player index.
  角色的序号
  Return value:    None.
  --*/
{
#ifndef PAL_CLASSIC
	WORD w = gpGlobals->rgParty[wPlayerIndex].wPlayerRole;
#endif

	g_Battle.UI.wCurPlayerIndex = wPlayerIndex;
	g_Battle.UI.state = kBattleUISelectMove;
	g_Battle.UI.wSelectedAction = 0;
	g_Battle.UI.MenuState = kBattleMenuMain;

#ifndef PAL_CLASSIC
	//
	// Play a sound which indicates the player is ready
	//
	if (gpGlobals->rgPlayerStatus[w][kStatusPuppet] == 0 &&
		gpGlobals->rgPlayerStatus[w][kStatusSleep] == 0 &&
		gpGlobals->rgPlayerStatus[w][kStatusConfused] == 0 &&
		!g_Battle.UI.fAutoAttack && !gpGlobals->fAutoBattle)
	{
		SOUND_PlayChannel(78, 1);
	}
#endif
}

static VOID
PAL_BattleUIUseItem(
VOID
)
/*++
  Purpose:    Use an item in the battle UI.
  在战斗界面中使用物品。
  Parameters:    None.
  Return value:   None.
  --*/
{
	WORD       wSelectedItem;
	// 初始化物品选择菜单
	wSelectedItem = PAL_ItemSelectMenuUpdate();

	if (wSelectedItem != 0xFFFF)
	{
		if (wSelectedItem != 0)
		{
			g_Battle.UI.wActionType = kBattleActionUseItem;
			g_Battle.UI.wObjectID = wSelectedItem;
			// 如果选择的使用物品是应用于全部玩家角色
			if (gpGlobals->g.rgObject[wSelectedItem].item.wFlags & kItemFlagApplyToAll)
			{
				g_Battle.UI.state = kBattleUISelectTargetPlayerAll;
			}
			else // 如果选择的使用物品不是应用于全部玩家角色
			{
#ifdef PAL_CLASSIC
				g_Battle.UI.wSelectedIndex = 0;
#else
				g_Battle.UI.wSelectedIndex = g_Battle.UI.wCurPlayerIndex;
#endif
				g_Battle.UI.state = kBattleUISelectTargetPlayer; // 界面的选择状态为选择目标玩家
			}
		}
		else
		{
			g_Battle.UI.MenuState = kBattleMenuMain;
		}
	}
}

static VOID
PAL_BattleUIThrowItem(
VOID
)
/*++
  Purpose:    Throw an item in the battle UI.
  在战斗界面中投掷物品。
  Parameters:    None.
  Return value:    None.
  --*/
{
	WORD wSelectedItem = PAL_ItemSelectMenuUpdate();
	// 初始化物品选择菜单
	if (wSelectedItem != 0xFFFF)
	{
		if (wSelectedItem != 0)
		{
			g_Battle.UI.wActionType = kBattleActionThrowItem;// 动作状态为投掷物品
			g_Battle.UI.wObjectID = wSelectedItem;
			// 如果选择的投掷物品是应用于全部敌人
			if (gpGlobals->g.rgObject[wSelectedItem].item.wFlags & kItemFlagApplyToAll)
			{
				g_Battle.UI.state = kBattleUISelectTargetEnemyAll;
			}
			else
			{ // 如果选择的投掷物品是应用于单个敌人
				g_Battle.UI.wSelectedIndex = g_Battle.UI.wPrevEnemyTarget;
				g_Battle.UI.state = kBattleUISelectTargetEnemy;
			}
		}
		else
		{
			g_Battle.UI.MenuState = kBattleMenuMain;
		}
	}
}

static WORD
PAL_BattleUIPickAutoMagic(
WORD          wPlayerRole,
WORD          wRandomRange
)
/*++
  Purpose:    Pick a magic for the specified player for automatic usage.
  为选定的玩家选择一个自动释放的魔法。
  Parameters:    [IN]  wPlayerRole - the player role ID.
  玩家角色的序号
  [IN]  wRandomRange - the range of the magic power.
  魔法威力的排序
  Return value:    The object ID of the selected magic. 0 for physical attack.
  选择的魔法的序号。0表示物理攻击。
  --*/
{
	WORD             wMagic = 0, w, wMagicNum;
	int              i, iMaxPower = 0, iPower;

	if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSilence] != 0)
	{
		return 0;
	}

	for (i = 0; i < MAX_PLAYER_MAGICS; i++)
	{
		w = gpGlobals->g.PlayerRoles.rgwMagic[i][wPlayerRole];
		if (w == 0)
		{
			continue;
		}

		wMagicNum = gpGlobals->g.rgObject[w].magic.wMagicNumber;

		//
		// skip if the magic is an ultimate move or not enough MP
		//如果该仙术的基础伤害值小于0，或此仙术消耗真气为1，或者角色现有真气不足，则跳过
		if (gpGlobals->g.lprgMagic[wMagicNum].wCostMP == 1 ||
			gpGlobals->g.lprgMagic[wMagicNum].wCostMP > gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole] ||
			(SHORT)(gpGlobals->g.lprgMagic[wMagicNum].wBaseDamage) <= 0)
		{
			continue;
		}

		iPower = (SHORT)(gpGlobals->g.lprgMagic[wMagicNum].wBaseDamage) +
			RandomLong(0, wRandomRange);

		if (iPower > iMaxPower)
		{
			iMaxPower = iPower;
			wMagic = w;
		}
	}

	return wMagic;
}

VOID
PAL_BattleUIUpdate(
VOID
)
/*++
  Purpose:    Update the status of battle UI.
  更新战斗界面的状态。
  Parameters:    None.
  Return value:      None.
  --*/
{
	int              i, j, x, y;
	WORD             wPlayerRole, w;
	static int       s_iFrame = 0;

	s_iFrame++;

#ifdef SHOW_DATA_IN_BATTLE
	if (gpGlobals->fShowDataInBattle)
	{
		PAL_New_BattleUIShowData();
	}
#endif

	//若攻击方式为自动攻击并且不在自动战斗模式下（即在战斗界面中的杂项菜单中选择了“围攻”）
	if (g_Battle.UI.fAutoAttack && !gpGlobals->fAutoBattle)
	{
		//
		// Draw the "auto attack" message if in the autoattack mode.
		//
		if (g_InputState.dwKeyPress & kKeyMenu)
		{	// 若在自动攻击模式下按菜单键（ESC），则取消自动攻击
			g_Battle.UI.fAutoAttack = FALSE;
		}
		else
		{	// 如果在自动攻击模式下则显示“围攻”
			PAL_DrawText(PAL_GetWord(BATTLEUI_LABEL_AUTO), PAL_XY(280, 10),
				MENUITEM_COLOR_CONFIRMED, TRUE, FALSE);
		}
	}

	if (gpGlobals->fAutoBattle)
	{
		PAL_BattlePlayerCheckReady();
		// 当角色状态时可以接受命令时
		for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
		{
			if (g_Battle.rgPlayer[i].state == kFighterCom)
			{
				PAL_BattleUIPlayerReady(i);
				break;
			}
		}

		if (g_Battle.UI.state != kBattleUIWait)
		{
			w = PAL_BattleUIPickAutoMagic(gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole, 9999);

			if (w == 0)
			{
				g_Battle.UI.wActionType = kBattleActionAttack;
				g_Battle.UI.wSelectedIndex = PAL_BattleSelectAutoTarget();
			}
			else
			{
				g_Battle.UI.wActionType = kBattleActionMagic;
				g_Battle.UI.wObjectID = w;

				if (gpGlobals->g.rgObject[w].magic.wFlags & kMagicFlagApplyToAll)
				{
					g_Battle.UI.wSelectedIndex = (WORD)-1;
				}
				else
				{
					g_Battle.UI.wSelectedIndex = PAL_BattleSelectAutoTarget();
				}
			}

			PAL_BattleCommitAction(FALSE);
		}

		goto end;
	}

	if (g_InputState.dwKeyPress & kKeyAuto)
	{
		g_Battle.UI.fAutoAttack = !g_Battle.UI.fAutoAttack;
		g_Battle.UI.MenuState = kBattleMenuMain;
	}

#ifdef PAL_CLASSIC
	if (g_Battle.Phase == kBattlePhasePerformAction)
	{
		goto end;
	}

	if (!g_Battle.UI.fAutoAttack)
#endif
	{
		//// 画玩家信息栏
		// Draw the player info boxes.
		//
		for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
		{
			wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;
			w = (WORD)(g_Battle.rgPlayer[i].flTimeMeter);

			j = TIMEMETER_COLOR_DEFAULT;

#ifndef PAL_CLASSIC
			if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusHaste] > 0)
			{
				j = TIMEMETER_COLOR_HASTE;
			}
			else if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSlow] > 0)
			{
				j = TIMEMETER_COLOR_SLOW;
			}
#endif

			if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] != 0 ||
				gpGlobals->rgPlayerStatus[wPlayerRole][kStatusConfused] != 0 ||
				gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] != 0)
			{
				w = 0;
			}

			// 更改角色信息栏的位置。在屏幕下方
			if (gpGlobals->wMaxPartyMemberIndex >= 3)
			{
				if (i > 3)
				{
					PAL_PlayerInfoBox(PAL_XY(91 + 77 * 2, 165 - (i - 3) * 38), wPlayerRole, w, j, FALSE);
					continue;
				}

				PAL_PlayerInfoBox(PAL_XY(14 + 77 * i, 165), wPlayerRole, w, j, FALSE);
				continue;
			}

			PAL_PlayerInfoBox(PAL_XY(91 + 77 * i, 165), wPlayerRole,
				w, j, FALSE);
		}
	}

	if (g_InputState.dwKeyPress & kKeyStatus)
	{
		PAL_PlayerStatus();
		goto end;
	}

	if (g_Battle.UI.state != kBattleUIWait)
	{
		wPlayerRole = gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole;

		if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0 &&
			gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet])
		{
			g_Battle.UI.wActionType = kBattleActionAttack;

			if (PAL_PlayerCanAttackAll(gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole))
			{
				g_Battle.UI.wSelectedIndex = (WORD)-1;
			}
			else
			{
				g_Battle.UI.wSelectedIndex = PAL_BattleSelectAutoTarget();
			}

			PAL_BattleCommitAction(FALSE);
			goto end; // don't go further
		}

		//
		// Cancel any actions if player is dead or sleeping.
		//如果角色已经死亡或者进入睡眠状态则取消其任何动作。
		if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0 ||
			gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSleep] != 0 ||
			gpGlobals->rgPlayerStatus[wPlayerRole][kStatusParalyzed] != 0)
		{
			g_Battle.UI.wActionType = kBattleActionPass;
			PAL_BattleCommitAction(FALSE);
			goto end; // don't go further
		}

		if (gpGlobals->rgPlayerStatus[wPlayerRole][kStatusConfused] != 0)
		{
			g_Battle.UI.wActionType = kBattleActionAttackMate;
			PAL_BattleCommitAction(FALSE);
			goto end; // don't go further
		}

		if (g_Battle.UI.fAutoAttack)
		{
			g_Battle.UI.wActionType = kBattleActionAttack;

			if (PAL_PlayerCanAttackAll(gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole))
			{
				g_Battle.UI.wSelectedIndex = (WORD)-1;
			}
			else
			{
				g_Battle.UI.wSelectedIndex = PAL_BattleSelectAutoTarget();
			}

			PAL_BattleCommitAction(FALSE);
			goto end; // don't go further
		}

		//
		// Draw the arrow on the player's head.
		// 画玩家头上的箭头
		i = SPRITENUM_BATTLE_ARROW_CURRENTPLAYER_RED;
		if (s_iFrame & 1)
		{
			i = SPRITENUM_BATTLE_ARROW_CURRENTPLAYER;
		}

		x = g_rgPlayerPos[gpGlobals->wMaxPartyMemberIndex][g_Battle.UI.wCurPlayerIndex][0] - 8;
		y = g_rgPlayerPos[gpGlobals->wMaxPartyMemberIndex][g_Battle.UI.wCurPlayerIndex][1] - 74;

		PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, i), gpScreen, PAL_XY(x, y));
	}

	switch (g_Battle.UI.state)
	{
		case kBattleUIWait:
		{
			if (!g_Battle.fEnemyCleared)
			{
				PAL_BattlePlayerCheckReady();

				for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
				{
					if (g_Battle.rgPlayer[i].state == kFighterCom)
					{
						PAL_BattleUIPlayerReady(i);
						break;
					}
				}
			}
			break;
		}

		case kBattleUISelectMove:
			//
			// Draw the icons
			//画图标
		{
			struct
			{
				int               iSpriteNum;
				PAL_POS           pos;
				BATTLEUIACTION    action;
			} rgItems[] =
			{// 4个战斗界面中的菜单
				{SPRITENUM_BATTLEICON_ATTACK, PAL_XY(27, 140), kBattleUIActionAttack},
				{SPRITENUM_BATTLEICON_MAGIC, PAL_XY(0, 155), kBattleUIActionMagic},
				{SPRITENUM_BATTLEICON_COOPMAGIC, PAL_XY(54, 155), kBattleUIActionCoopMagic},
				{SPRITENUM_BATTLEICON_MISCMENU, PAL_XY(27, 170), kBattleUIActionMisc}
			};

			//多于4个人的时候调整战斗菜单盘的位置
			if (gpGlobals->wMaxPartyMemberIndex >= 3)
			{
				for (i = 0; i < 4; i++)
				{
					rgItems[i].pos -= (37 << 16);
				}
			}

			// 用上下左右键来选择4个菜单
			if (g_Battle.UI.MenuState == kBattleMenuMain)
			{
				if (g_InputState.dir == kDirNorth)
				{
					g_Battle.UI.wSelectedAction = 0;
				}
				else if (g_InputState.dir == kDirSouth)
				{
					g_Battle.UI.wSelectedAction = 3;
				}
				else if (g_InputState.dir == kDirWest)
				{
					if (PAL_BattleUIIsActionValid(kBattleUIActionMagic))
					{
						g_Battle.UI.wSelectedAction = 1;
					}
				}
				else if (g_InputState.dir == kDirEast)
				{
					if (PAL_BattleUIIsActionValid(kBattleUIActionCoopMagic))
					{
						g_Battle.UI.wSelectedAction = 2;
					}
				}
			}

			if (!PAL_BattleUIIsActionValid(rgItems[g_Battle.UI.wSelectedAction].action))
			{
				g_Battle.UI.wSelectedAction = 0;
			}

			for (i = 0; i < 4; i++)
			{
				if (g_Battle.UI.wSelectedAction == i)
				{
					PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, rgItems[i].iSpriteNum),
						gpScreen, rgItems[i].pos);
				}
				else if (PAL_BattleUIIsActionValid(rgItems[i].action))
				{
					PAL_RLEBlitMonoColor(PAL_SpriteGetFrame(gpSpriteUI, rgItems[i].iSpriteNum),
						gpScreen, rgItems[i].pos, 0, -4);
				}
				else
				{
					PAL_RLEBlitMonoColor(PAL_SpriteGetFrame(gpSpriteUI, rgItems[i].iSpriteNum),
						gpScreen, rgItems[i].pos, 0x10, -4);
				}
			}

			switch (g_Battle.UI.MenuState)
			{
				case kBattleMenuMain:
				{
					if (g_InputState.dwKeyPress & kKeySearch)
					{
						switch (g_Battle.UI.wSelectedAction)
						{
							//选择物攻
							case 0:
							{
								g_Battle.UI.wActionType = kBattleActionAttack;
								if (PAL_PlayerCanAttackAll(gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole))
								{
									g_Battle.UI.state = kBattleUISelectTargetEnemyAll;
								}
								else
								{
									g_Battle.UI.wSelectedIndex = g_Battle.UI.wPrevEnemyTarget;
									g_Battle.UI.state = kBattleUISelectTargetEnemy;
								}
								break;
							}

								//选择仙术
							case 1:
							{
								g_Battle.UI.MenuState = kBattleMenuMagicSelect;
								PAL_MagicSelectionMenuInit(wPlayerRole, TRUE, 0);
								break;
							}

								//选择合体法术
							case 2:
							{
								w = gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole;
								w = PAL_GetPlayerCooperativeMagic(w);

								g_Battle.UI.wActionType = kBattleActionCoopMagic;
								g_Battle.UI.wObjectID = w;

								if (gpGlobals->g.rgObject[w].magic.wFlags & kMagicFlagUsableToEnemy)
								{
									if (gpGlobals->g.rgObject[w].magic.wFlags & kMagicFlagApplyToAll)
									{
										g_Battle.UI.state = kBattleUISelectTargetEnemyAll;
									}
									else
									{
										g_Battle.UI.wSelectedIndex = g_Battle.UI.wPrevEnemyTarget;
										g_Battle.UI.state = kBattleUISelectTargetEnemy;
									}
								}
								else
								{
									if (gpGlobals->g.rgObject[w].magic.wFlags & kMagicFlagApplyToAll)
									{
										g_Battle.UI.state = kBattleUISelectTargetPlayerAll;
									}
									else
									{
#ifdef PAL_CLASSIC
										g_Battle.UI.wSelectedIndex = 0;
#else
										g_Battle.UI.wSelectedIndex = g_Battle.UI.wCurPlayerIndex;
#endif
										g_Battle.UI.state = kBattleUISelectTargetPlayer;
									}
								}
								break;
							}

								//选择杂项菜单
							case 3:
							{
								g_Battle.UI.MenuState = kBattleMenuMisc;
								g_iCurMiscMenuItem = 0;
								break;
							}
						}
					}
					else if (g_InputState.dwKeyPress & kKeyDefend)
					{
						g_Battle.UI.wActionType = kBattleActionDefend;
						PAL_BattleCommitAction(FALSE);
					}
					else if (g_InputState.dwKeyPress & kKeyForce)
					{
						w = PAL_BattleUIPickAutoMagic(gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole, 60);

						if (w == 0)
						{
							g_Battle.UI.wActionType = kBattleActionAttack;

							if (PAL_PlayerCanAttackAll(gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole))
							{
								g_Battle.UI.wSelectedIndex = (WORD)-1;
							}
							else
							{
								g_Battle.UI.wSelectedIndex = PAL_BattleSelectAutoTarget();
							}
						}
						else
						{
							g_Battle.UI.wActionType = kBattleActionMagic;
							g_Battle.UI.wObjectID = w;

							if (gpGlobals->g.rgObject[w].magic.wFlags & kMagicFlagApplyToAll)
							{
								g_Battle.UI.wSelectedIndex = (WORD)-1;
							}
							else
							{
								g_Battle.UI.wSelectedIndex = PAL_BattleSelectAutoTarget();
							}
						}

						PAL_BattleCommitAction(FALSE);
					}
					else if (g_InputState.dwKeyPress & kKeyFlee)
					{
						g_Battle.UI.wActionType = kBattleActionFlee;
						PAL_BattleCommitAction(FALSE);
					}
					else if (g_InputState.dwKeyPress & kKeyUseItem)
					{
						g_Battle.UI.MenuState = kBattleMenuUseItemSelect;
						PAL_ItemSelectMenuInit(kItemFlagUsable);
					}
					else if (g_InputState.dwKeyPress & kKeyThrowItem)
					{
						g_Battle.UI.MenuState = kBattleMenuThrowItemSelect;
						PAL_ItemSelectMenuInit(kItemFlagThrowable);
					}
					else if (g_InputState.dwKeyPress & kKeyRepeat)
					{
						PAL_BattleCommitAction(TRUE);
					}
#ifdef PAL_CLASSIC
					else if (g_InputState.dwKeyPress & kKeyMenu)
					{
						g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].state = kFighterWait;
						g_Battle.UI.state = kBattleUIWait;

						if (g_Battle.UI.wCurPlayerIndex > 0)
						{
							//
							// Revert to the previous player
							//恢复到之前的角色
							do
							{
								g_Battle.rgPlayer[--g_Battle.UI.wCurPlayerIndex].state = kFighterWait;

								if (g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.ActionType == kBattleActionThrowItem)
								{
									for (i = 0; i < MAX_INVENTORY; i++)
									{
										if (gpGlobals->rgInventory[i].wItem ==
											g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.wActionID)
										{
											gpGlobals->rgInventory[i].nAmountInUse--;
											break;
										}
									}
								}
								else if (g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.ActionType == kBattleActionUseItem)
								{
									if (gpGlobals->g.rgObject[g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.wActionID].item.wFlags & kItemFlagConsuming)
									{
										for (i = 0; i < MAX_INVENTORY; i++)
										{
											if (gpGlobals->rgInventory[i].wItem ==
												g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].action.wActionID)
											{
												gpGlobals->rgInventory[i].nAmountInUse--;
												break;
											}
										}
									}
								}
							} while (g_Battle.UI.wCurPlayerIndex > 0 &&
								(gpGlobals->g.PlayerRoles.rgwHP[gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole] == 0 ||
								gpGlobals->rgPlayerStatus[gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole][kStatusConfused] > 0 ||
								gpGlobals->rgPlayerStatus[gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole][kStatusSleep] > 0 ||
								gpGlobals->rgPlayerStatus[gpGlobals->rgParty[g_Battle.UI.wCurPlayerIndex].wPlayerRole][kStatusParalyzed] > 0));
						}
					}
#else
					else if (g_InputState.dwKeyPress & kKeyMenu)
					{
						float flMin = -1;
						j = -1;

						for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
						{
							if (g_Battle.rgPlayer[i].flTimeMeter >= 100)
							{
								g_Battle.rgPlayer[i].flTimeMeter += 100; // HACKHACK: Prevent the time meter from going below 100

								if ((g_Battle.rgPlayer[i].flTimeMeter < flMin || flMin < 0) &&
									i != (int)g_Battle.UI.wCurPlayerIndex &&
									g_Battle.rgPlayer[i].state == kFighterWait)
								{
									flMin = g_Battle.rgPlayer[i].flTimeMeter;
									j = i;
								}
							}
						}

						if (j != -1)
						{
							g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].flTimeMeter = flMin - 99;
							g_Battle.rgPlayer[g_Battle.UI.wCurPlayerIndex].state = kFighterWait;
							g_Battle.UI.state = kBattleUIWait;
						}
					}
#endif
					break;
				}

				case kBattleMenuMagicSelect:
				{
					w = PAL_MagicSelectionMenuUpdate();

					if (w != 0xFFFF)
					{
						g_Battle.UI.MenuState = kBattleMenuMain;

						if (w != 0)
						{
							g_Battle.UI.wActionType = kBattleActionMagic;
							g_Battle.UI.wObjectID = w;

							if (gpGlobals->g.rgObject[w].magic.wFlags & kMagicFlagUsableToEnemy)
							{
								if (gpGlobals->g.rgObject[w].magic.wFlags & kMagicFlagApplyToAll)
								{
									g_Battle.UI.state = kBattleUISelectTargetEnemyAll;
								}
								else
								{
									g_Battle.UI.wSelectedIndex = g_Battle.UI.wPrevEnemyTarget;
									g_Battle.UI.state = kBattleUISelectTargetEnemy;
								}
							}
							else
							{
								if (gpGlobals->g.rgObject[w].magic.wFlags & kMagicFlagApplyToAll)
								{
									g_Battle.UI.state = kBattleUISelectTargetPlayerAll;
								}
								else
								{
#ifdef PAL_CLASSIC
									g_Battle.UI.wSelectedIndex = 0;
#else
									g_Battle.UI.wSelectedIndex = g_Battle.UI.wCurPlayerIndex;
#endif
									g_Battle.UI.state = kBattleUISelectTargetPlayer;
								}
							}
						}
					}
					break;
				}

				case kBattleMenuUseItemSelect:
					PAL_BattleUIUseItem();
					break;

				case kBattleMenuThrowItemSelect:
					PAL_BattleUIThrowItem();
					break;

				case kBattleMenuMisc:
				{
					w = PAL_BattleUIMiscMenuUpdate();

					if (w != 0xFFFF)
					{
						g_Battle.UI.MenuState = kBattleMenuMain;

						switch (w)
						{
#ifdef PAL_CLASSIC
							case 2: // item  物品
#else
							case 1: // item
#endif
								g_Battle.UI.MenuState = kBattleMenuMiscItemSubMenu;
								g_iCurSubMenuItem = 0;
								break;

#ifdef PAL_CLASSIC
							case 3: // defend  防御
#else
							case 2: // defend
#endif
								g_Battle.UI.wActionType = kBattleActionDefend;
								PAL_BattleCommitAction(FALSE);
								break;

#ifdef PAL_CLASSIC
							case 1: // auto   围攻
#else
							case 3: // auto
#endif
								g_Battle.UI.fAutoAttack = TRUE;
								break;

							case 4: // flee    逃跑
								g_Battle.UI.wActionType = kBattleActionFlee;
								PAL_BattleCommitAction(FALSE);
								break;

							case 5: // status   状态
								PAL_PlayerStatus();
								break;
#ifdef SHOW_DATA_IN_BATTLE
							case 6: // 显示数据
								gpGlobals->fShowDataInBattle = PAL_SwitchMenu(gpGlobals->fShowDataInBattle);
								break;
#endif

#ifdef SHOW_ENEMY_STATUS
							case 7: // 显示敌方状态
								PAL_New_EnemyStatus();
								break;
#endif								
						}
					}
					break;
				}


				case kBattleMenuMiscItemSubMenu:
				{
#ifdef SORT_INVENTORY
					PAL_New_SortInventory();
#endif
					w = PAL_BattleUIMiscItemSubMenuUpdate();

					if (w != 0xFFFF)
					{
						g_Battle.UI.MenuState = kBattleMenuMain;

						switch (w)
						{
							case 1: // use  // 使用（物品）
								g_Battle.UI.MenuState = kBattleMenuUseItemSelect;
								PAL_ItemSelectMenuInit(kItemFlagUsable);
								break;

							case 2: // throw	// 投掷（物品）
								g_Battle.UI.MenuState = kBattleMenuThrowItemSelect;
								PAL_ItemSelectMenuInit(kItemFlagThrowable);
								break;
						}
					}
					break;
				}
			}

			break;
		}

		case kBattleUISelectTargetEnemy:
		{
			x = -1;
			y = 0;

			for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
			{
				if (g_Battle.rgEnemy[i].wObjectID != 0)
				{
					x = i;
					y++;
				}
			}

			if (x == -1)
			{
				g_Battle.UI.state = kBattleUISelectMove;
				break;
			}

			if (g_Battle.UI.wActionType == kBattleActionCoopMagic)
			{
				if (!PAL_BattleUIIsActionValid(kBattleActionCoopMagic))
				{
					g_Battle.UI.state = kBattleUISelectMove;
					break;
				}
			}

#ifdef PAL_CLASSIC
			//
			// Don't bother selecting when only 1 enemy left
			//当只剩一个敌人时不用选择目标
			if (y == 1)
			{
				g_Battle.UI.wPrevEnemyTarget = (WORD)x;
				g_Battle.UI.wSelectedIndex = g_Battle.UI.wPrevEnemyTarget;
				PAL_BattleCommitAction(FALSE);
				break;
			}
#endif
			g_Battle.UI.wSelectedIndex = min(g_Battle.UI.wSelectedIndex, x);

			for (i = 0; i <= x; i++)
			{
				if (g_Battle.rgEnemy[g_Battle.UI.wSelectedIndex].wObjectID != 0)
				{
					break;
				}
				g_Battle.UI.wSelectedIndex++;
				g_Battle.UI.wSelectedIndex %= x + 1;
			}

			//
			// Highlight the selected enemy
			//  使选中的敌人高亮
			if (s_iFrame & 1)
			{
				i = g_Battle.UI.wSelectedIndex;

				x = PAL_X(g_Battle.rgEnemy[i].pos);
				y = PAL_Y(g_Battle.rgEnemy[i].pos);

				x -= PAL_RLEGetWidth(PAL_SpriteGetFrame(g_Battle.rgEnemy[i].lpSprite, g_Battle.rgEnemy[i].wCurrentFrame)) / 2;
				y -= PAL_RLEGetHeight(PAL_SpriteGetFrame(g_Battle.rgEnemy[i].lpSprite, g_Battle.rgEnemy[i].wCurrentFrame));

				PAL_RLEBlitWithColorShift(PAL_SpriteGetFrame(g_Battle.rgEnemy[i].lpSprite, g_Battle.rgEnemy[i].wCurrentFrame),
					gpScreen, PAL_XY(x, y), 7);
			}

			if (g_InputState.dwKeyPress & kKeyMenu)
			{
				g_Battle.UI.state = kBattleUISelectMove;
			}
			else if (g_InputState.dwKeyPress & kKeySearch)
			{
				g_Battle.UI.wPrevEnemyTarget = g_Battle.UI.wSelectedIndex;
				PAL_BattleCommitAction(FALSE);
			}
			else if (g_InputState.dwKeyPress & (kKeyLeft | kKeyDown))
			{
				if (g_Battle.UI.wSelectedIndex != 0)
				{
					g_Battle.UI.wSelectedIndex--;
					while (g_Battle.UI.wSelectedIndex != 0 &&
						g_Battle.rgEnemy[g_Battle.UI.wSelectedIndex].wObjectID == 0)
					{
						g_Battle.UI.wSelectedIndex--;
					}
				}
			}
			else if (g_InputState.dwKeyPress & (kKeyRight | kKeyUp))
			{
				if (g_Battle.UI.wSelectedIndex < x)
				{
					g_Battle.UI.wSelectedIndex++;
					while (g_Battle.UI.wSelectedIndex < x &&
						g_Battle.rgEnemy[g_Battle.UI.wSelectedIndex].wObjectID == 0)
					{
						g_Battle.UI.wSelectedIndex++;
					}
				}
			}
			break;
		}

		case kBattleUISelectTargetPlayer:
		{
#ifdef PAL_CLASSIC
			//
			// Don't bother selecting when only 1 player is in the party
			//当队伍中只有一名角色时无需选择
			if (gpGlobals->wMaxPartyMemberIndex == 0)
			{
				g_Battle.UI.wSelectedIndex = 0;
				PAL_BattleCommitAction(FALSE);
			}
#endif

			j = SPRITENUM_BATTLE_ARROW_SELECTEDPLAYER;
			if (s_iFrame & 1)
			{
				j = SPRITENUM_BATTLE_ARROW_SELECTEDPLAYER_RED;
			}

			//
			// Draw arrows on the selected player
			//在选中的角色头上画箭头
			x = g_rgPlayerPos[gpGlobals->wMaxPartyMemberIndex][g_Battle.UI.wSelectedIndex][0] - 8;
			y = g_rgPlayerPos[gpGlobals->wMaxPartyMemberIndex][g_Battle.UI.wSelectedIndex][1] - 67;

			PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, j), gpScreen, PAL_XY(x, y));

			if (g_InputState.dwKeyPress & kKeyMenu)
			{
				g_Battle.UI.state = kBattleUISelectMove;
			}
			else if (g_InputState.dwKeyPress & kKeySearch)
			{
				PAL_BattleCommitAction(FALSE);
			}
			else if (g_InputState.dwKeyPress & (kKeyLeft | kKeyDown))
			{
				if (g_Battle.UI.wSelectedIndex != 0)
				{
					g_Battle.UI.wSelectedIndex--;
				}
				else
				{
					g_Battle.UI.wSelectedIndex = gpGlobals->wMaxPartyMemberIndex;
				}
			}
			else if (g_InputState.dwKeyPress & (kKeyRight | kKeyUp))
			{
				if (g_Battle.UI.wSelectedIndex < gpGlobals->wMaxPartyMemberIndex)
				{
					g_Battle.UI.wSelectedIndex++;
				}
				else
				{
					g_Battle.UI.wSelectedIndex = 0;
				}
			}

			break;
		}

		case kBattleUISelectTargetEnemyAll:
		{
#ifdef PAL_CLASSIC
			//
			// Don't bother selecting
			//敌人选择目标为全体时无需选择
			g_Battle.UI.wSelectedIndex = (WORD)-1;
			PAL_BattleCommitAction(FALSE);
#else
			if (g_Battle.UI.wActionType == kBattleActionCoopMagic)
			{
				if (!PAL_BattleUIIsActionValid(kBattleActionCoopMagic))
				{
					g_Battle.UI.state = kBattleUISelectMove;
					break;
				}
			}

			if (s_iFrame & 1)
			{
				//
				// Highlight all enemies
				//
				for (i = g_Battle.wMaxEnemyIndex; i >= 0; i--)
				{
					if (g_Battle.rgEnemy[i].wObjectID == 0)
					{
						continue;
					}

					x = PAL_X(g_Battle.rgEnemy[i].pos);
					y = PAL_Y(g_Battle.rgEnemy[i].pos);

					x -= PAL_RLEGetWidth(PAL_SpriteGetFrame(g_Battle.rgEnemy[i].lpSprite, g_Battle.rgEnemy[i].wCurrentFrame)) / 2;
					y -= PAL_RLEGetHeight(PAL_SpriteGetFrame(g_Battle.rgEnemy[i].lpSprite, g_Battle.rgEnemy[i].wCurrentFrame));

					PAL_RLEBlitWithColorShift(PAL_SpriteGetFrame(g_Battle.rgEnemy[i].lpSprite, g_Battle.rgEnemy[i].wCurrentFrame),
						gpScreen, PAL_XY(x, y), 7);
				}
			}
			if (g_InputState.dwKeyPress & kKeyMenu)
			{
				g_Battle.UI.state = kBattleUISelectMove;
			}
			else if (g_InputState.dwKeyPress & kKeySearch)
			{
				g_Battle.UI.wSelectedIndex = (WORD)-1;
				PAL_BattleCommitAction(FALSE);
			}
#endif
			break;
		}

		case kBattleUISelectTargetPlayerAll:
		{
#ifdef PAL_CLASSIC
			//
			// Don't bother selecting
			//角色选择目标为全体时无需选择
			g_Battle.UI.wSelectedIndex = (WORD)-1;
			PAL_BattleCommitAction(FALSE);
#else
			j = SPRITENUM_BATTLE_ARROW_SELECTEDPLAYER;
			if (s_iFrame & 1)
			{
				j = SPRITENUM_BATTLE_ARROW_SELECTEDPLAYER_RED;
			}
			for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				if (g_Battle.UI.wActionType == kBattleActionMagic)
				{
					w = gpGlobals->g.rgObject[g_Battle.UI.wObjectID].magic.wMagicNumber;

					if (gpGlobals->g.lprgMagic[w].wType == kMagicTypeTrance)
					{
						if (i != g_Battle.UI.wCurPlayerIndex)
							continue;
					}
				}

				//
				// Draw arrows on all players, despite of dead or not
				//
				x = g_rgPlayerPos[gpGlobals->wMaxPartyMemberIndex][i][0] - 8;
				y = g_rgPlayerPos[gpGlobals->wMaxPartyMemberIndex][i][1] - 67;

				PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, j), gpScreen, PAL_XY(x, y));
			}

			if (g_InputState.dwKeyPress & kKeyMenu)
			{
				g_Battle.UI.state = kBattleUISelectMove;
			}
			else if (g_InputState.dwKeyPress & kKeySearch)
			{
				g_Battle.UI.wSelectedIndex = (WORD)-1;
				PAL_BattleCommitAction(FALSE);
			}
#endif
			break;
		}
	}

end:
	//
	// Show the text message if there is one.
	//
#ifndef PAL_CLASSIC
	if (SDL_GetTicks() < g_Battle.UI.dwMsgShowTime)
	{
		//
		// The text should be shown in a small window at the center of the screen
		//
		PAL_POS    pos;
		int        len = strlen(g_Battle.UI.szMsg);

		//
		// Create the window box
		//
		pos = PAL_XY(160 - len * 4, 40);
		PAL_CreateSingleLineBox(pos, (len + 1) / 2, FALSE);

		//
		// Show the text on the screen
		//
		pos = PAL_XY(PAL_X(pos) + 8 + ((len & 1) << 2), PAL_Y(pos) + 10);
		PAL_DrawText(g_Battle.UI.szMsg, pos, 0, FALSE, FALSE);
	}
	else if (g_Battle.UI.szNextMsg[0] != '\0')
	{
		strcpy(g_Battle.UI.szMsg, g_Battle.UI.szNextMsg);
		g_Battle.UI.dwMsgShowTime = SDL_GetTicks() + g_Battle.UI.wNextMsgDuration;
		g_Battle.UI.szNextMsg[0] = '\0';
	}
#endif

	//
	// Draw the numbers
	//显示数字
	for (i = 0; i < BATTLEUI_MAX_SHOWNUM; i++)
	{
		if (g_Battle.UI.rgShowNum[i].wNum > 0)
		{
			if ((SDL_GetTicks() - g_Battle.UI.rgShowNum[i].dwTime) / BATTLE_FRAME_TIME > 10)
			{
				g_Battle.UI.rgShowNum[i].wNum = 0;
			}
			else
			{
				PAL_DrawNumber(g_Battle.UI.rgShowNum[i].wNum, 5,
					PAL_XY(PAL_X(g_Battle.UI.rgShowNum[i].pos), PAL_Y(g_Battle.UI.rgShowNum[i].pos) - (SDL_GetTicks() - g_Battle.UI.rgShowNum[i].dwTime) / BATTLE_FRAME_TIME),
					g_Battle.UI.rgShowNum[i].color, kNumAlignRight);
			}
		}
	}

	PAL_ClearKeyState();
}

#ifdef SHOW_DATA_IN_BATTLE
VOID
PAL_New_BattleUIShowData(
VOID
)
/*++
Purpose: 在战斗中显示一些数据。
--*/
{
	int              i, j;
	WORD             wPlayerRole, w;

	//显示敌人血量
	for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
	{
		if (g_Battle.rgEnemy[i].wObjectID == 0)
		{
			continue;
		}
		PAL_DrawNumber(g_Battle.rgEnemy[i].dwActualHealth, 5, PAL_XY(40 * i, 0), kNumColorYellow, kNumAlignRight);
	}

	//显示我方的对各属性仙术的抗性
	int startPos = 320 - 20 * (gpGlobals->wMaxPartyMemberIndex + 1);
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;
		for (j = 0; j < NUM_MAGIC_ELEMENTAL; j++)
		{
			w = PAL_GetPlayerElementalResistance(wPlayerRole, j);
			PAL_DrawNumber(w, 3, PAL_XY(startPos + 20 * i, 10 * j), kNumColorYellow, kNumAlignRight);
		}

		w = PAL_GetPlayerPoisonResistance(wPlayerRole);
		PAL_DrawNumber(w, 3, PAL_XY(startPos + 20 * i, 10 * j), kNumColorYellow, kNumAlignRight);

		j++;
		w = PAL_New_GetPlayerSorceryResistance(wPlayerRole);
		PAL_DrawNumber(w, 3, PAL_XY(startPos + 20 * i, 10 * j), kNumColorYellow, kNumAlignRight);

		j++;
		w = PAL_New_GetPlayerSorceryStrength(wPlayerRole);
		PAL_DrawNumber(w, 3, PAL_XY(startPos + 20 * i, 10 * j), kNumColorYellow, kNumAlignRight);
	}

	//显示我方有益状态的剩余轮次
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;
		for (j = kStatusPuppet; j < kStatusAll; j++)
		{
			w = gpGlobals->rgPlayerStatus[wPlayerRole][j];
			PAL_DrawNumber(min(99, w), 3, PAL_XY(startPos + 20 * i,
				75 + 10 * (j - kStatusPuppet)), kNumColorYellow, kNumAlignRight);
		}
	}

	//显示总的灵葫值
	w = gpGlobals->wCollectValue;
	PAL_DrawNumber(w, 5, PAL_XY(280, 150), kNumColorYellow, kNumAlignRight);
}
#endif

VOID
PAL_BattleUIShowNum(
WORD           wNum,
PAL_POS        pos,
NUMCOLOR       color
)
/*++
  Purpose:    Show a number on battle screen (indicates HP/MP change).
  在战斗屏幕上显示数字（表示生命值和魔法值的变化）。
  Parameters:    [IN]  wNum - number to be shown.
  要显示的数字。
  [IN]  pos - position of the number on the screen.
  数字在屏幕上的位置。
  [IN]  color - color of the number.
  数字的颜色。
  Return value:    None.
  --*/
{
	int     i;

	for (i = 0; i < BATTLEUI_MAX_SHOWNUM; i++)
	{
		if (g_Battle.UI.rgShowNum[i].wNum == 0)
		{
			g_Battle.UI.rgShowNum[i].wNum = wNum;
			g_Battle.UI.rgShowNum[i].pos = PAL_XY(PAL_X(pos) - 15, PAL_Y(pos));
			g_Battle.UI.rgShowNum[i].color = color;
			g_Battle.UI.rgShowNum[i].dwTime = SDL_GetTicks();

			break;
		}
	}
}


#ifdef SHOW_ENEMY_STATUS
VOID
PAL_New_EnemyStatus(
VOID
)
{
	PAL_LARGE BYTE		bufBackground[320 * 200];
	INT					iCurrent;
	BATTLEENEMY			be;
	INT					i, x, y, h;
	WORD				w;
	LPCBITMAPRLE		lBMR;
	PAL_POS				pos;

	const int        rgEquipPos[MAX_PLAYER_EQUIPMENTS][2] = {
			{190, 0}, {248, 40}, {252, 102}, {202, 134}, {142, 142}, {82, 126}
	};

	PAL_MKFDecompressChunk(bufBackground, 320 * 200, STATUS_BACKGROUND_FBPNUM,
		gpGlobals->f.fpFBP);

	iCurrent = 0;

	while (iCurrent >= 0 && iCurrent < MAX_ENEMIES_IN_TEAM)
	{
		be = g_Battle.rgEnemy[iCurrent];
		if (be.wObjectID == 0 || be.dwActualHealth == 0)
		{
			iCurrent++;
			continue;
		}

		// Draw the background image
		PAL_FBPBlitToSurface(bufBackground, gpScreen);

		// 怪物图像
		lBMR = PAL_SpriteGetFrame(g_Battle.rgEnemy[iCurrent].lpSprite, g_Battle.rgEnemy[iCurrent].wCurrentFrame);
		pos = PAL_XY(200, 100);
		pos = PAL_XY(PAL_X(pos) - PAL_RLEGetWidth(lBMR) / 2, PAL_Y(pos) - PAL_RLEGetHeight(lBMR) / 2);
		PAL_RLEBlitToSurface(lBMR, gpScreen, pos);

		// Draw the text labels
		i = 0;
		x = 6;
		y = 6;
		h = 19;
		PAL_DrawText(PAL_GetWord(STATUS_LABEL_EXP), PAL_XY(x, y + (i++) * h), MENUITEM_COLOR, TRUE, FALSE);
		PAL_DrawText(PAL_GetWord(STATUS_LABEL_LEVEL), PAL_XY(x, y + (i++) * h), MENUITEM_COLOR, TRUE, FALSE);
		PAL_DrawText(PAL_GetWord(STATUS_LABEL_HP), PAL_XY(x, y + (i++) * h), MENUITEM_COLOR, TRUE, FALSE);

		PAL_DrawText(PAL_GetWord(STATUS_LABEL_SORCERYRESISTANCE), PAL_XY(x, y + (i++) * h), MENUITEM_COLOR, TRUE, FALSE);
		PAL_DrawText(PAL_GetWord(STATUS_LABEL_POISONRESISTANCE), PAL_XY(x, y + (i++) * h), MENUITEM_COLOR, TRUE, FALSE);
		PAL_DrawText(PAL_GetWord(STATUS_LABEL_PHYSICALRESISTANCE), PAL_XY(x, y + (i++) * h), MENUITEM_COLOR, TRUE, FALSE);

		PAL_DrawText(PAL_GetWord(STATUS_LABEL_COLLECTVALUE), PAL_XY(x, y + (i++) * h), MENUITEM_COLOR, TRUE, FALSE);
		PAL_DrawText(PAL_GetWord(STATUS_LABEL_STEALITEM), PAL_XY(x, y + (i++) * h), MENUITEM_COLOR, TRUE, FALSE);
		PAL_DrawText(PAL_GetWord(STATUS_LABEL_ATTACKEQUIVITEM), PAL_XY(x, y + (i++) * h), MENUITEM_COLOR, TRUE, FALSE);

		PAL_DrawText(PAL_GetWord(be.wObjectID), PAL_XY(120, 6), MENUITEM_COLOR_CONFIRMED, TRUE, FALSE);

		// Draw the stats
		i = 0;
		x = 42;
		y = 11;
		h = 19;
		PAL_DrawNumber(be.e.wExp, 5, PAL_XY(x + 16, y + (i++) * h), kNumColorYellow, kNumAlignRight);
		PAL_DrawNumber(be.e.wLevel, 3, PAL_XY(x + 6, y + (i++) * h), kNumColorYellow, kNumAlignRight);
		PAL_DrawNumber(be.dwActualHealth, 5, PAL_XY(x, y + (i++) * h), kNumColorYellow, kNumAlignRight);
		//体力、体力最大值的分隔符“/”
		PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, SPRITENUM_SLASH), gpScreen, PAL_XY(x + 29, y + (i - 1) * h));
		PAL_DrawNumber(be.dwMaxHealth, 5, PAL_XY(x + 26, y + (i - 1) * h + 5), kNumColorBlue, kNumAlignRight);

		PAL_DrawNumber(PAL_New_GetEnemySorceryResistance(iCurrent), 4, PAL_XY(x, y + (i++) * h), kNumColorYellow, kNumAlignRight);
		PAL_DrawNumber(PAL_New_GetEnemyPoisonResistance(iCurrent), 4, PAL_XY(x, y + (i++) * h), kNumColorYellow, kNumAlignRight);
		PAL_DrawNumber(PAL_New_GetEnemyPhysicalResistance(iCurrent), 4, PAL_XY(x, y + (i++) * h), kNumColorYellow, kNumAlignRight);

		PAL_DrawNumber(be.e.wCollectValue, 4, PAL_XY(x, y + (i++) * h), kNumColorYellow, kNumAlignRight);
		
		PAL_DrawNumber(be.e.nStealItem, 4, PAL_XY(x, y + (i++) * h), kNumColorYellow, kNumAlignRight);
		PAL_RLEBlitToSurface(PAL_SpriteGetFrame(gpSpriteUI, SPRITENUM_SLASH), gpScreen, PAL_XY(x + 27, y + (i - 1) * h));
		WORD wStealItem = be.e.wStealItem != 0 ? be.e.wStealItem : CASH_LABEL;
		PAL_DrawText(PAL_GetWord(wStealItem), PAL_XY(x + 32, y + (i - 1) * h - 5), MENUITEM_COLOR_CONFIRMED, TRUE, FALSE);

		WORD wAttackEquivItem = be.e.wAttackEquivItem != 0 ? be.e.wAttackEquivItem : LABEL_NOTHING;
		PAL_DrawText(PAL_GetWord(wAttackEquivItem), PAL_XY(x + 32, y + (i++) * h - 5), MENUITEM_COLOR_CONFIRMED, TRUE, FALSE);

		PAL_DrawNumber(iCurrent, 1, PAL_XY(300, 180), kNumColorYellow, kNumAlignRight);

		//
		// Draw all poisons
		//将毒的名称写在状态栏里
		y = 6;

#ifdef POISON_STATUS_EXPAND
		int wPoisonIntensity = 0;
#endif

		for (i = 0; i < MAX_POISONS; i++)
		{
			w = g_Battle.rgEnemy[iCurrent].rgPoisons[i].wPoisonID;

			if (w != 0)
			{
				PAL_DrawText(PAL_GetWord(w), PAL_XY(245, y),
					(BYTE)(gpGlobals->g.rgObject[w].poison.wColor + 10), TRUE, FALSE);

#ifdef POISON_STATUS_EXPAND
				wPoisonIntensity = g_Battle.rgEnemy[iCurrent].rgPoisons[i].wPoisonIntensity;
				if (wPoisonIntensity != 0)
				{
					PAL_DrawNumber(wPoisonIntensity, 2, PAL_XY(310, y + 4), kNumColorYellow, kNumAlignRight);
				}
#endif
				y += 19;	
			}
		}

		//
		// Update the screen
		//
		VIDEO_UpdateScreen(NULL);

		//
		// Wait for input
		//
		PAL_ClearKeyState();

		while (TRUE)
		{
			UTIL_Delay(1);

			if (g_InputState.dwKeyPress & kKeyMenu)
			{
				iCurrent = -1;
				break;
			}
			else if (g_InputState.dwKeyPress & (kKeyLeft | kKeyUp))
			{
				do 
				{
					iCurrent--;
					if (iCurrent < 0)
					{
						break;
					}
					be = g_Battle.rgEnemy[iCurrent];
				} while (be.wObjectID == 0 && be.dwActualHealth == 0);
				break;
			}
			else if (g_InputState.dwKeyPress & (kKeyRight | kKeyDown | kKeySearch))
			{
				do
				{
					iCurrent++;
					if (iCurrent >= MAX_ENEMIES_IN_TEAM)
					{
						break;
					}
					be = g_Battle.rgEnemy[iCurrent];
				} while (be.wObjectID == 0 && be.dwActualHealth == 0);
				break;
			}
		}
	}
}
#endif