/* -*- mode: c; tab-width: 4; c-basic-offset: 3; c-file-style: "linux" -*- */
//
// Copyright (c) 2009, Wei Mingzhi <whistler_wmz@users.sf.net>.
// All rights reserved.
//
// Based on PALx Project by palxex.
// Copyright (c) 2006-2008, Pal Lockheart <palxex@gmail.com>.
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


BOOL            g_fScriptSuccess = TRUE;
static int      g_iCurEquipPart = -1;


static WORD 
PAL_NEW_CheckAndGetLegalEnemyTarget(
WORD wEnemyIndex
)
/*++
  功能：    检查对象是否是一个合法的敌人目标
  参数：    对象id
  返回值：  一个合法的敌人目标
  --*/
{
	WORD		i;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || g_Battle.rgEnemy[wEnemyIndex].wObjectID == 0)
	{
		do
		{
			i = (WORD)RandomLong(0, g_Battle.wMaxEnemyIndex);
		} while (g_Battle.rgEnemy[i].wObjectID == 0);

		return i;
	}
	else
	{
		return wEnemyIndex;
	}
}

static WORD 
PAL_NEW_CheckAndGetLegalPlayerTarget(
WORD wPlayerRole
)
/*++
  功能：    检查对象是否是一个合法的我方角色目标
  参数：    对象id
  返回值：  一个合法的我方角色目标id
  --*/
{
	INT		i;
	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		if (wPlayerRole == gpGlobals->rgParty[i].wPlayerRole)
		{
			return wPlayerRole;
		}
	}

	if (i > gpGlobals->wMaxPartyMemberIndex)
	{
		i = RandomLong(0, gpGlobals->wMaxPartyMemberIndex);
		wPlayerRole = gpGlobals->rgParty[i].wPlayerRole;
	}
	return wPlayerRole;
}

static BOOL
PAL_NPCWalkTo(
WORD           wEventObjectID,
INT            x,
INT            y,
INT            h,
INT            iSpeed
)
/*++
  Purpose:

  Make the specified event object walk to the map position specified by (x, y, h)
  at the speed of iSpeed.

  Parameters:

  [IN]  wEventObjectID - the event object to move.

  [IN]  x - Column number of the tile.

  [IN]  y - Line number in the map.

  [IN]  h - Each line in the map has two lines of tiles, 0 and 1.
  (See map.h for details.)

  [IN]  iSpeed - the speed to move.

  Return value:

  TRUE if the event object has successfully moved to the specified position,
  FALSE if still need more moving.

  --*/
{
	LPEVENTOBJECT    pEvtObj;
	int              xOffset, yOffset;

	pEvtObj = &(gpGlobals->g.lprgEventObject[wEventObjectID - 1]);

	xOffset = (x * 32 + h * 16) - pEvtObj->x;
	yOffset = (y * 16 + h * 8) - pEvtObj->y;

	if (yOffset < 0)
	{
		pEvtObj->wDirection = ((xOffset < 0) ? kDirWest : kDirNorth);
	}
	else
	{
		pEvtObj->wDirection = ((xOffset < 0) ? kDirSouth : kDirEast);
	}

	if (abs(xOffset) < iSpeed * 2 || abs(yOffset) < iSpeed * 2)
	{
		pEvtObj->x = x * 32 + h * 16;
		pEvtObj->y = y * 16 + h * 8;
	}
	else
	{
		PAL_NPCWalkOneStep(wEventObjectID, iSpeed);
	}

	if (pEvtObj->x == x * 32 + h * 16 && pEvtObj->y == y * 16 + h * 8)
	{
		pEvtObj->wCurrentFrameNum = 0;
		return TRUE;
	}

	return FALSE;
}

static VOID
PAL_PartyWalkTo(
INT            x,
INT            y,
INT            h,
INT            iSpeed
)
/*++
  Purpose:

  Make the party walk to the map position specified by (x, y, h)
  at the speed of iSpeed.

  Parameters:

  [IN]  x - Column number of the tile.

  [IN]  y - Line number in the map.

  [IN]  h - Each line in the map has two lines of tiles, 0 and 1.
  (See map.h for details.)

  [IN]  iSpeed - the speed to move.

  Return value:

  None.

  --*/
{
	int           xOffset, yOffset, i, dx, dy;
	DWORD         t;

	xOffset = x * 32 + h * 16 - PAL_X(gpGlobals->viewport) - PAL_X(gpGlobals->partyoffset);
	yOffset = y * 16 + h * 8 - PAL_Y(gpGlobals->viewport) - PAL_Y(gpGlobals->partyoffset);

	t = 0;

	while (xOffset != 0 || yOffset != 0)
	{
		PAL_ProcessEvent();
		while (SDL_GetTicks() <= t)
		{
			PAL_ProcessEvent();
			SDL_Delay(1);
		}

		t = SDL_GetTicks() + FRAME_TIME;

		//
		// Store trail
		//
		for (i = 3; i >= 0; i--)
		{
			gpGlobals->rgTrail[i + 1] = gpGlobals->rgTrail[i];
		}
		gpGlobals->rgTrail[0].wDirection = gpGlobals->wPartyDirection;
		gpGlobals->rgTrail[0].x = PAL_X(gpGlobals->viewport) + PAL_X(gpGlobals->partyoffset);
		gpGlobals->rgTrail[0].y = PAL_Y(gpGlobals->viewport) + PAL_Y(gpGlobals->partyoffset);

		if (yOffset < 0)
		{
			gpGlobals->wPartyDirection = ((xOffset < 0) ? kDirWest : kDirNorth);
		}
		else
		{
			gpGlobals->wPartyDirection = ((xOffset < 0) ? kDirSouth : kDirEast);
		}

		dx = PAL_X(gpGlobals->viewport);
		dy = PAL_Y(gpGlobals->viewport);

		if (abs(xOffset) <= iSpeed * 2)
		{
			dx += xOffset;
		}
		else
		{
			dx += iSpeed * (xOffset < 0 ? -2 : 2);
		}

		if (abs(yOffset) <= iSpeed)
		{
			dy += yOffset;
		}
		else
		{
			dy += iSpeed * (yOffset < 0 ? -1 : 1);
		}

		//
		// Move the viewport
		//
		gpGlobals->viewport = PAL_XY(dx, dy);

		PAL_UpdatePartyGestures(TRUE);
		PAL_GameUpdate(FALSE);
		PAL_MakeScene();
		VIDEO_UpdateScreen(NULL);

		xOffset = x * 32 + h * 16 - PAL_X(gpGlobals->viewport) - PAL_X(gpGlobals->partyoffset);
		yOffset = y * 16 + h * 8 - PAL_Y(gpGlobals->viewport) - PAL_Y(gpGlobals->partyoffset);
	}

	PAL_UpdatePartyGestures(FALSE);
}

static VOID
PAL_PartyRideEventObject(
WORD           wEventObjectID,
INT            x,
INT            y,
INT            h,
INT            iSpeed
)
/*++
  Purpose:

  Move the party to the specified position, riding the specified event object.

  Parameters:

  [IN]  wEventObjectID - the event object to be ridden.

  [IN]  x - Column number of the tile.

  [IN]  y - Line number in the map.

  [IN]  h - Each line in the map has two lines of tiles, 0 and 1.
  (See map.h for details.)

  [IN]  iSpeed - the speed to move.

  Return value:

  TRUE if the party and event object has successfully moved to the specified
  position, FALSE if still need more moving.

  --*/
{
	int              xOffset, yOffset, dx, dy, i;
	DWORD            t;
	LPEVENTOBJECT    p;

	p = &(gpGlobals->g.lprgEventObject[wEventObjectID - 1]);

	xOffset = x * 32 + h * 16 - PAL_X(gpGlobals->viewport) - PAL_X(gpGlobals->partyoffset);
	yOffset = y * 16 + h * 8 - PAL_Y(gpGlobals->viewport) - PAL_Y(gpGlobals->partyoffset);

	t = 0;

	while (xOffset != 0 || yOffset != 0)
	{
		PAL_ProcessEvent();
		while (SDL_GetTicks() <= t)
		{
			PAL_ProcessEvent();
			SDL_Delay(1);
		}

		t = SDL_GetTicks() + FRAME_TIME;

		if (yOffset < 0)
		{
			gpGlobals->wPartyDirection = ((xOffset < 0) ? kDirWest : kDirNorth);
		}
		else
		{
			gpGlobals->wPartyDirection = ((xOffset < 0) ? kDirSouth : kDirEast);
		}

		if (abs(xOffset) > iSpeed * 2)
		{
			dx = iSpeed * (xOffset < 0 ? -2 : 2);
		}
		else
		{
			dx = xOffset;
		}

		if (abs(yOffset) > iSpeed)
		{
			dy = iSpeed * (yOffset < 0 ? -1 : 1);
		}
		else
		{
			dy = yOffset;
		}

		//
		// Store trail
		//
		for (i = 3; i >= 0; i--)
		{
			gpGlobals->rgTrail[i + 1] = gpGlobals->rgTrail[i];
		}

		gpGlobals->rgTrail[0].wDirection = gpGlobals->wPartyDirection;
		gpGlobals->rgTrail[0].x = PAL_X(gpGlobals->viewport) + dx + PAL_X(gpGlobals->partyoffset);
		gpGlobals->rgTrail[0].y = PAL_Y(gpGlobals->viewport) + dy + PAL_Y(gpGlobals->partyoffset);

		//
		// Move the viewport
		//
		gpGlobals->viewport =
			PAL_XY(PAL_X(gpGlobals->viewport) + dx, PAL_Y(gpGlobals->viewport) + dy);

		p->x += dx;
		p->y += dy;

		PAL_GameUpdate(FALSE);
		PAL_MakeScene();
		VIDEO_UpdateScreen(NULL);

		xOffset = x * 32 + h * 16 - PAL_X(gpGlobals->viewport) - PAL_X(gpGlobals->partyoffset);
		yOffset = y * 16 + h * 8 - PAL_Y(gpGlobals->viewport) - PAL_Y(gpGlobals->partyoffset);
	}
}

static VOID
PAL_MonsterChasePlayer(
WORD         wEventObjectID,
WORD         wSpeed,
WORD         wChaseRange,
BOOL         fFloating
)
/*++
  Purpose:

  Make the specified event object chase the players.

  Parameters:

  [IN]  wEventObjectID - the event object ID of the monster.

  [IN]  wSpeed - the speed of chasing.

  [IN]  wChaseRange - sensitive range of the monster.

  [IN]  fFloating - TRUE if monster is floating (i.e., ignore the obstacles)

  Return value:

  None.

  --*/
{
	LPEVENTOBJECT    pEvtObj = &gpGlobals->g.lprgEventObject[wEventObjectID - 1];
	WORD             wMonsterSpeed = 0, prevx, prevy;
	int              x, y, i, j, l;

	if (gpGlobals->wChaseRange != 0)
	{
		x = PAL_X(gpGlobals->viewport) + PAL_X(gpGlobals->partyoffset) - pEvtObj->x;
		y = PAL_Y(gpGlobals->viewport) + PAL_Y(gpGlobals->partyoffset) - pEvtObj->y;

		if (x == 0)
		{
			x = RandomLong(0, 1) ? -1 : 1;
		}

		if (y == 0)
		{
			y = RandomLong(0, 1) ? -1 : 1;
		}

		prevx = pEvtObj->x;
		prevy = pEvtObj->y;

		i = prevx % 32;
		j = prevy % 16;

		prevx /= 32;
		prevy /= 16;
		l = 0;

		if (i + j * 2 >= 16)
		{
			if (i + j * 2 >= 48)
			{
				prevx++;
				prevy++;
			}
			else if (32 - i + j * 2 < 16)
			{
				prevx++;
			}
			else if (32 - i + j * 2 < 48)
			{
				l = 1;
			}
			else
			{
				prevy++;
			}
		}

		prevx = prevx * 32 + l * 16;
		prevy = prevy * 16 + l * 8;

		//
		// Is the party near to the event object?
		//
		if (abs(x) + abs(y) * 2 < wChaseRange * 32 * gpGlobals->wChaseRange)
		{
			if (x < 0)
			{
				if (y < 0)
				{
					pEvtObj->wDirection = kDirWest;
				}
				else
				{
					pEvtObj->wDirection = kDirSouth;
				}
			}
			else
			{
				if (y < 0)
				{
					pEvtObj->wDirection = kDirNorth;
				}
				else
				{
					pEvtObj->wDirection = kDirEast;
				}
			}

			if (x != 0)
			{
				x = pEvtObj->x + x / abs(x) * 16;
			}
			else
			{
				x = pEvtObj->x;
			}

			if (y != 0)
			{
				y = pEvtObj->y + y / abs(y) * 8;
			}
			else
			{
				y = pEvtObj->y;
			}

			if (fFloating)
			{
				wMonsterSpeed = wSpeed;
			}
			else
			{
				if (!PAL_CheckObstacle(PAL_XY(x, y), TRUE, wEventObjectID))
				{
					wMonsterSpeed = wSpeed;
				}
				else
				{
					pEvtObj->x = prevx;
					pEvtObj->y = prevy;
				}

				for (l = 0; l < 4; l++)
				{
					switch (l)
					{
						case 0:
							pEvtObj->x -= 4;
							pEvtObj->y += 2;
							break;

						case 1:
							pEvtObj->x -= 4;
							pEvtObj->y -= 2;
							break;

						case 2:
							pEvtObj->x += 4;
							pEvtObj->y -= 2;
							break;

						case 3:
							pEvtObj->x += 4;
							pEvtObj->y += 2;
							break;
					}

					if (PAL_CheckObstacle(PAL_XY(pEvtObj->x, pEvtObj->y), FALSE, 0))
					{
						pEvtObj->x = prevx;
						pEvtObj->y = prevy;
					}
				}
			}
		}
	}

	PAL_NPCWalkOneStep(wEventObjectID, wMonsterSpeed);
}

static VOID
PAL_AdditionalCredits(
VOID
)
/*++
  Purpose:

  Show the additional credits.

  Parameters:

  None.

  Return value:

  None.

  --*/
{
	LPCSTR rgszStrings[] = {
		"SDLPAL (http://sdlpal.codeplex.com/)",
#ifdef PAL_CLASSIC
		"         (\xB8\x67\xA8\xE5\xAF\x53\xA7\x4F\xBD\x67  " __DATE__ ")",
#else
		"                    (" __DATE__ ")",
#endif
		" ",
		"  (c) 2009-2014, Wei Mingzhi",
		"      <whistler_wmz@users.sf.net>.",
#ifdef __SYMBIAN32__
		"  Symbian S60 \xB2\xBE\xB4\xD3 (c) 2009, netwan.",
#endif
#ifdef GPH
		"  GPH Caanoo & Wiz \xB2\xBE\xB4\xD3 (c) 2011, Rikku2000.",
#endif
#ifdef GEKKO
		"  Nintendo WII \xB2\xBE\xB4\xD3 (c) 2012, Rikku2000.",
#endif
#ifdef DINGOO
		"  DINGOO & Dingux \xB2\xBE\xB4\xD3 (c) 2011, Rikku2000.",
#endif
		" ",
		"\xA5\xBB\xB5\x7B\xA6\xA1\xAC\x4F\xA6\xDB\xA5\xD1\xB3\x6E\xC5\xE9\xA1\x41\xAB\xF6\xB7\xD3"
		" GNU General",
		"Public License (GPLv3) \xB5\x6F\xA7\x47",
		" ",
		"                 ...\xAB\xF6 Enter \xB5\xB2\xA7\xF4",
		""
	};

	int        i = 0;

#ifdef PAL_WIN95
	extern BOOL fIsBig5;
	fIsBig5 = TRUE;
#endif

	PAL_DrawOpeningMenuBackground();

	while (rgszStrings[i][0] != '\0')
	{
		PAL_DrawText(rgszStrings[i], PAL_XY(25, 20 + i * 16), DESCTEXT_COLOR, TRUE, FALSE);
		i++;
	}

	PAL_SetPalette(0, FALSE);
	VIDEO_UpdateScreen(NULL);

	PAL_WaitForKey(0);
}

static WORD
PAL_InterpretInstruction(
WORD           wScriptEntry,
WORD           wEventObjectID
)
/*++
  Purpose:

  Interpret and execute one instruction in the script.

  Parameters:

  [IN]  wScriptEntry - The script entry to execute.

  [IN]  wEventObjectID - The event object ID which invoked the script.

  Return value:

  The address of the next script instruction to execute.

  --*/
{
	LPEVENTOBJECT		pEvtObj, pCurrent;
	LPSCRIPTENTRY		pScript;
	INT					iPlayerRole, i, j, x, y;
	WORD				w, wCurEventObjectID;
	WORD				wPlayerRole;

	pScript = &(gpGlobals->g.lprgScriptEntry[wScriptEntry]);

	if (wEventObjectID != 0)
	{
		pEvtObj = &(gpGlobals->g.lprgEventObject[wEventObjectID - 1]);
	}
	else
	{
		pEvtObj = NULL;
	}

	if (pScript->rgwOperand[0] == 0 || pScript->rgwOperand[0] == 0xFFFF)
	{
		pCurrent = pEvtObj;
		wCurEventObjectID = wEventObjectID;
	}
	else
	{
		i = pScript->rgwOperand[0] - 1;
		if (i > 0x9000)
		{
			// HACK for Dream 2.11 to avoid crash
			i -= 0x9000;
		}
		pCurrent = &(gpGlobals->g.lprgEventObject[i]);
		wCurEventObjectID = pScript->rgwOperand[0];
	}

	if (pScript->rgwOperand[0] < MAX_PLAYABLE_PLAYER_ROLES)
	{
		iPlayerRole = gpGlobals->rgParty[pScript->rgwOperand[0]].wPlayerRole;
	}
	else
	{
		iPlayerRole = gpGlobals->rgParty[0].wPlayerRole;
	}

	switch (pScript->wOperation)
	{
			// walk one step
		case 0x000B:
		case 0x000C:
		case 0x000D:
		case 0x000E:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pEvtObj == NULL)
			{
				break;
			}
#endif
			pEvtObj->wDirection = pScript->wOperation - 0x000B;
			PAL_NPCWalkOneStep(wEventObjectID, 2);
			break;
		}

			// Set the direction and/or gesture for event object
		case 0x000F:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pEvtObj == NULL)
			{
				break;
			}
#endif
			if (pScript->rgwOperand[0] != 0xFFFF)
			{
				pEvtObj->wDirection = pScript->rgwOperand[0];
			}

			if (pScript->rgwOperand[1] != 0xFFFF)
			{
				pEvtObj->wCurrentFrameNum = pScript->rgwOperand[1];
			}

			break;
		}

			// Walk straight to the specified position
		case 0x0010:
		{
			if (!PAL_NPCWalkTo(wEventObjectID, pScript->rgwOperand[0], pScript->rgwOperand[1],
				pScript->rgwOperand[2], 3))
			{
				wScriptEntry--;
			}
			break;
		}

			// Walk straight to the specified position, at a lower speed
		case 0x0011:
		{
			if ((wEventObjectID & 1) ^ (gpGlobals->dwFrameNum & 1))
			{
				if (!PAL_NPCWalkTo(wEventObjectID, pScript->rgwOperand[0], pScript->rgwOperand[1],
					pScript->rgwOperand[2], 2))
				{
					wScriptEntry--;
				}
			}
			else
			{
				wScriptEntry--;
			}
			break;
		}

			// Set the position of the event object, relative to the party
		case 0x0012:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pCurrent == NULL)
			{
				break;
			}
#endif
			pCurrent->x =
				pScript->rgwOperand[1] + PAL_X(gpGlobals->viewport) + PAL_X(gpGlobals->partyoffset);
			pCurrent->y =
				pScript->rgwOperand[2] + PAL_Y(gpGlobals->viewport) + PAL_Y(gpGlobals->partyoffset);
			break;
		}

			// Set the position of the event object
		case 0x0013:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pCurrent == NULL)
			{
				break;
			}
#endif
			pCurrent->x = pScript->rgwOperand[1];
			pCurrent->y = pScript->rgwOperand[2];
			break;
		}

			// Set the gesture of the event object
		case 0x0014:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pEvtObj == NULL)
			{
				break;
			}
#endif
			pEvtObj->wCurrentFrameNum = pScript->rgwOperand[0];
			pEvtObj->wDirection = kDirSouth;
			break;
		}

			// Set the direction and gesture for a party member
		case 0x0015:
		{
			gpGlobals->wPartyDirection = pScript->rgwOperand[0];
			gpGlobals->rgParty[pScript->rgwOperand[2]].wFrame =
				gpGlobals->wPartyDirection * 3 + pScript->rgwOperand[1];
			break;
		}

			// Set the direction and gesture for an event object
		case 0x0016:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pCurrent == NULL)
			{
				break;
			}
#endif
			if (pScript->rgwOperand[0] != 0)
			{
				pCurrent->wDirection = pScript->rgwOperand[1];
				pCurrent->wCurrentFrameNum = pScript->rgwOperand[2];
			}
			break;
		}

			// set the player's extra attribute // 若装备不存在，则战斗后不保留
		case 0x0017:
			/*
			17 参数1 参数2 参数3	   装备引起的属性增加指令
			参数1：主角属性地址——装备位置
			0B：头部
			0C：身体
			0D：肩部
			0E：武器
			0F：脚部
			10：配戴

			参数2：主角属性地址——基本属性
			00：状态表情图像
			01：战斗模型
			02：地图模型
			03：名字
			04：可否攻击全体
			05：无效？
			06：等级
			07：体力最大值
			08：真气最大值
			09：体力
			0A：真气
			11：武术
			12：灵力
			13：防御
			14：身法
			15：吉运
			16：毒抗
			17：风抗
			18：雷抗
			19：水抗
			1A：火抗
			1B：土抗
			......
			41：合体法术
			指向data.mkf的第4个子文件
			指向的地址 = 该主角的地址偏移 + 参数2*12
			主角地址偏移：
			李逍遥 0 赵灵儿 2 林月如 4
			巫后 6 阿奴 8 盖罗娇10
			参数3：整数
			作用	   若该主角参数1的地址指向的位置没有装备东西，则将参数3的值增加到参数2指向的地址
			*/
		{
			WORD *p;

			i = pScript->rgwOperand[0] - 0xB;

			p = (WORD *)(&gpGlobals->rgEquipmentEffect[i]); // HACKHACK

			p[pScript->rgwOperand[1] * MAX_PLAYER_ROLES + wEventObjectID] =
				(SHORT)pScript->rgwOperand[2];
			break;
		}

			// Equip the selected item
		case 0x0018:
		{
			i = pScript->rgwOperand[0] - 0x0B;
			g_iCurEquipPart = i;

			//
			// The wEventObjectID parameter here should indicate the player role
			//
			PAL_RemoveEquipmentEffect(wEventObjectID, i);

			if (gpGlobals->g.PlayerRoles.rgwEquipment[i][wEventObjectID] != pScript->rgwOperand[1])
			{
				w = gpGlobals->g.PlayerRoles.rgwEquipment[i][wEventObjectID];
				gpGlobals->g.PlayerRoles.rgwEquipment[i][wEventObjectID] = pScript->rgwOperand[1];

				PAL_AddItemToInventory(pScript->rgwOperand[1], -1);

				if (w != 0)
				{
					PAL_AddItemToInventory(w, 1);
				}

				gpGlobals->wLastUnequippedItem = w;
			}
			break;
		}

			// Increase/decrease the player's attribute // 永久改变
		case 0x0019:
			/*
			19 参数1 参数2 参数3	   属性增加指令
			参数1：主角属性地址——基本属性
			00：状态表情图像
			01：战斗模型
			02：地图模型
			03：名字
			04：可否攻击全体
			05：无效？
			06：等级
			07：体力最大值
			08：真气最大值
			09：体力
			0A：真气
			11：武术
			12：灵力
			13：防御
			14：身法
			15：吉运
			16：毒抗
			17：风抗
			18：雷抗
			19：水抗
			1A：火抗
			1B：土抗
			......
			41：合体法术
			参数2：整数
			参数3：对象主角（可省略）
			01：李逍遥
			02：赵灵儿
			03：林月如
			04：巫后
			05：阿奴
			06：盖罗娇
			作用		   将参数2的数值增加到参数3指向的主角或选定主角（参数3省略时）的参数1的位置
			*/
		{
			WORD *p = (WORD *)(&gpGlobals->g.PlayerRoles); // HACKHACK

			if (pScript->rgwOperand[2] == 0)
			{
				iPlayerRole = wEventObjectID;
			}
			else
			{
				iPlayerRole = pScript->rgwOperand[2] - 1;
			}

			p[pScript->rgwOperand[0] * MAX_PLAYER_ROLES + iPlayerRole] +=
				(SHORT)pScript->rgwOperand[1];

#ifdef CAN_BEYOND_MAX_PARAMETER_SOMETIMES
			if (gpGlobals->g.PlayerRoles.rgwCanBeyondMaxPara[iPlayerRole] == 0)
			{
#endif
				p[pScript->rgwOperand[0] * MAX_PLAYER_ROLES + iPlayerRole] =
					min(p[pScript->rgwOperand[0] * MAX_PLAYER_ROLES + iPlayerRole], MAX_PARAMETER + MAX_PARAMETER_EXTRA);
#ifdef CAN_BEYOND_MAX_PARAMETER_SOMETIMES
			}
#endif
			break;
		}

			// Set player's stat
		case 0x001A:
			/*
			1A 参数1 参数2 参数3	   属性写入指令
			参数1：主角属性地址
			参数2：整数
			参数3：对象主角（可省略）
			作用	   将参数2的数值直接写入参数3指向的主角或选定主角（参数3省略时）的参数1的位置
			*/
		{
			WORD *p = (WORD *)(&gpGlobals->g.PlayerRoles); // HACKHACK

			if (g_iCurEquipPart != -1)
			{
				//
				// In the progress of equipping items
				//
				p = (WORD *)&(gpGlobals->rgEquipmentEffect[g_iCurEquipPart]);
			}

			if (pScript->rgwOperand[2] == 0)
			{
				//
				// Apply to the current player. The wEventObjectID should
				// indicate the player role.
				//
				iPlayerRole = wEventObjectID;
			}
			else
			{
				iPlayerRole = pScript->rgwOperand[2] - 1;
			}

			p[pScript->rgwOperand[0] * MAX_PLAYER_ROLES + iPlayerRole] =
				(SHORT)pScript->rgwOperand[1];
			break;
		}

			// Increase/decrease player's HP
		case 0x001B:
			/*
			1B 参数1 参数2	   恢复体力指令
			参数1：是否全体
			0：单体
			1：全体
			参数2：整数
			作用	   给指定主角或全体恢复参数2大小的体力
			*/
		{
			if (pScript->rgwOperand[0])
			{
				//
				// Apply to everyone
				//
				for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
				{
					w = gpGlobals->rgParty[i].wPlayerRole;
					PAL_IncreaseHPMP(w, (SHORT)(pScript->rgwOperand[1]), 0);
				}
			}
			else
			{
				//
				// Apply to one player. The wEventObjectID parameter should indicate the player role.
				//
				if (!PAL_IncreaseHPMP(wEventObjectID, (SHORT)(pScript->rgwOperand[1]), 0))
				{
					g_fScriptSuccess = FALSE;
				}
			}
			break;
		}

			// Increase/decrease player's MP
		case 0x001C:
		{
			/*
			1C 参数1 参数2	   恢复真气指令
			参数1：是否全体
			参数2：整数
			作用	   给指定主角或全体恢复参数2大小的真气
			*/
			if (pScript->rgwOperand[0])
			{
				//
				// Apply to everyone
				//
				for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
				{
					w = gpGlobals->rgParty[i].wPlayerRole;
					PAL_IncreaseHPMP(w, 0, (SHORT)(pScript->rgwOperand[1]));
				}
			}
			else
			{
				//
				// Apply to one player. The wEventObjectID parameter should indicate the player role.
				//
				if (!PAL_IncreaseHPMP(wEventObjectID, 0, (SHORT)(pScript->rgwOperand[1])))
				{
					g_fScriptSuccess = FALSE;
				}
			}
			break;
		}

			//Increase / decrease player's HP and MP
		case 0x001D:
		{
			/*
			1D 参数1 参数2 参数3	   恢复体力和真气指令
			参数1：是否全体
			参数2：整数
			参数3：整数（可省略）
			作用	   如参数3省略，则给给指定主角或全体恢复参数2大小的体力和真气
			如参数3存在，则给指定主角或全体恢复参数2大小的体力和参数3大小的真气
			*/
			if (pScript->rgwOperand[0])
			{
				//
				// Apply to everyone
				//
				for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
				{
					w = gpGlobals->rgParty[i].wPlayerRole;
					PAL_IncreaseHPMP(w, (SHORT)(pScript->rgwOperand[1]), (SHORT)(pScript->rgwOperand[1]));
				}
			}
			else
			{
				//
				// Apply to one player. The wEventObjectID parameter should indicate the player role.
				//
				if (!PAL_IncreaseHPMP(wEventObjectID, (SHORT)(pScript->rgwOperand[1]), (SHORT)(pScript->rgwOperand[1])))
				{
					g_fScriptSuccess = FALSE;
				}
			}
			break;
		}

			// Increase or decrease cash by the specified amount
		case 0x001E:
		{
			/*
			1E 参数1 参数2	   金钱改变指令
			参数1：整数
			参数2：指令地址（可省略）
			作用	   金钱改变参数1的数值（增加或减少）
			若参数1为负数，并且钱不够，则跳转参数2指向的地址
			*/
			if ((SHORT)(pScript->rgwOperand[0]) < 0 &&
				gpGlobals->dwCash < (WORD)(-(SHORT)(pScript->rgwOperand[0])))
			{
				//
				// not enough cash
				//
				wScriptEntry = pScript->rgwOperand[1] - 1;
			}
			else
			{
				gpGlobals->dwCash += (SHORT)(pScript->rgwOperand[0]);
			}
			break;
		}

			// Add item to inventory
		case 0x001F:
		{
			PAL_AddItemToInventory(pScript->rgwOperand[0], (SHORT)(pScript->rgwOperand[1]));
			break;
		}

			// Remove item from inventory
		case 0x0020:
		{
			if (!PAL_AddItemToInventory(pScript->rgwOperand[0],
				-((pScript->rgwOperand[1] == 0) ? 1 : pScript->rgwOperand[1])))
			{
				//
				// Try removing equipped item
				//
				x = pScript->rgwOperand[1];
				if (x == 0)
				{
					x = 1;
				}

				for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
				{
					w = gpGlobals->rgParty[i].wPlayerRole;

					for (j = 0; j < MAX_PLAYER_EQUIPMENTS; j++)
					{
						if (gpGlobals->g.PlayerRoles.rgwEquipment[j][w] == pScript->rgwOperand[0])
						{
							PAL_RemoveEquipmentEffect(w, j);
							gpGlobals->g.PlayerRoles.rgwEquipment[j][w] = 0;

							if (--x == 0)
							{
								i = 9999;
								break;
							}
						}
					}
				}

				if (x > 0 && pScript->rgwOperand[2] != 0)
				{
					wScriptEntry = pScript->rgwOperand[2] - 1;
				}
			}
			break;
		}

			// Inflict damage to the enemy
		case 0x0021:
			/*
			21 参数1 参数2	   伤敌指令
			参数1：是否全体
			参数2：整数
			作用	   对选定敌人或全体造成参数2数量的伤害（伤害值为负时变为增加血量）
			*/
		{
			SHORT sDamage = (SHORT)pScript->rgwOperand[1];//可正可负
			if (pScript->rgwOperand[0])
			{
				// Inflict damage to all enemies
				for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
				{
					PAL_New_DecreaseHPForEnemy(i, sDamage);
				}
			}
			else
			{
				// Inflict damage to one enemy
				PAL_New_DecreaseHPForEnemy(wEventObjectID, sDamage);
			}
			break;
		}

		// Revive player 角色复活
		case 0x0022:
		{
			if (pScript->rgwOperand[0])
			{
				//
				// Apply to everyone
				//
				g_fScriptSuccess = FALSE;

				for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
				{
					w = gpGlobals->rgParty[i].wPlayerRole;
					if (gpGlobals->g.PlayerRoles.rgwHP[w] == 0)
					{
						gpGlobals->g.PlayerRoles.rgwHP[w] =
							gpGlobals->g.PlayerRoles.rgwMaxHP[w] * pScript->rgwOperand[1] / 10;

						PAL_CurePoisonByLevel(w, MAX_POISON_LEVEL);
						for (x = 0; x < kStatusAll; x++)
						{
							PAL_RemovePlayerStatus(w, x);
						}

						g_fScriptSuccess = TRUE;
					}
				}
			}
			else
			{
				//
				// Apply to one player
				//
				if (gpGlobals->g.PlayerRoles.rgwHP[wEventObjectID] == 0)
				{
					gpGlobals->g.PlayerRoles.rgwHP[wEventObjectID] =
						gpGlobals->g.PlayerRoles.rgwMaxHP[wEventObjectID] * pScript->rgwOperand[1] / 10;

					PAL_CurePoisonByLevel(wEventObjectID, MAX_POISON_LEVEL);
					for (x = 0; x < kStatusAll; x++)
					{
						PAL_RemovePlayerStatus(wEventObjectID, x);
					}
				}
				else
				{
					g_fScriptSuccess = FALSE;
				}
			}
			break;
		}

			// Remove equipment from the specified player
		case 0x0023:
		{
			iPlayerRole = pScript->rgwOperand[0];

			if (pScript->rgwOperand[1] == 0)
			{
				//
				// Remove all equipments
				//
				for (i = 0; i < MAX_PLAYER_EQUIPMENTS; i++)
				{
					w = gpGlobals->g.PlayerRoles.rgwEquipment[i][iPlayerRole];
					if (w != 0)
					{
						PAL_AddItemToInventory(w, 1);
						gpGlobals->g.PlayerRoles.rgwEquipment[i][iPlayerRole] = 0;
					}
					PAL_RemoveEquipmentEffect(iPlayerRole, i);
				}
			}
			else
			{
				w = gpGlobals->g.PlayerRoles.rgwEquipment[pScript->rgwOperand[1] - 1][iPlayerRole];
				if (w != 0)
				{
					PAL_RemoveEquipmentEffect(iPlayerRole, pScript->rgwOperand[1] - 1);
					PAL_AddItemToInventory(w, 1);
					gpGlobals->g.PlayerRoles.rgwEquipment[pScript->rgwOperand[1] - 1][iPlayerRole] = 0;
				}
			}
			break;
		}

			// Set the autoscript entry address for an event object
		case 0x0024:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pCurrent == NULL)
			{
				break;
			}
#endif
			if (pScript->rgwOperand[0] != 0)
			{
				pCurrent->wAutoScript = pScript->rgwOperand[1];
			}
			break;
		}

			// Set the trigger script entry address for an event object
		case 0x0025:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pCurrent == NULL)
			{
				break;
			}
#endif
			if (pScript->rgwOperand[0] != 0)
			{
				pCurrent->wTriggerScript = pScript->rgwOperand[1];
			}
			break;
		}

			// Show the buy item menu
		case 0x0026:
		{
			PAL_MakeScene();
			VIDEO_UpdateScreen(NULL);
			PAL_BuyMenu(pScript->rgwOperand[0]);
			break;
		}

			// Show the sell item menu
		case 0x0027:
		{
			PAL_MakeScene();
			VIDEO_UpdateScreen(NULL);
			PAL_SellMenu();
			break;
		}

			// Apply poison to enemy
		case 0x0028:
			/*
			28 参数1 参数2	   敌方中毒指令
			参数1 是否全体
			参数2 中毒代号
			作用	   敌方选定对象或全体中参数2代表的毒
			*/
		{
			BOOL fAll = (pScript->rgwOperand[0] == 0) ? FALSE : TRUE;
			WORD wPoisonID = pScript->rgwOperand[1];
			BOOL fJump = FALSE;
			BOOL fAlwaysSuccess = FALSE;
			WORD wSorceryResistance = 0;
			INT iSuccessRate = 0;
			WORD wBaseSuccessRate = 100;

			if (g_Battle.fPlayerMoving)
			{
				wPlayerRole = gpGlobals->rgParty[g_Battle.wMovingPlayerIndex].wPlayerRole;
				wPlayerRole = PAL_NEW_CheckAndGetLegalPlayerTarget(wPlayerRole);	//如果
				wBaseSuccessRate += PAL_New_GetPlayerSorceryResistance(wPlayerRole);//巫抗越高下毒成功率越高
			}

#ifdef ADD_SOME_POISONS_SUCCESSFULLY_ANYTIME
			if (pScript->rgwOperand[2] == 0xffff
				|| gpGlobals->g.rgObject[pScript->rgwOperand[1]].poison.wPoisonLevel >= 99)
			{
				fAlwaysSuccess = TRUE;
			}
#endif

			if (fAll)
			{	// 全体
				for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
				{
					w = g_Battle.rgEnemy[i].wObjectID;
					if (w != 0)
					{
						wSorceryResistance = PAL_New_GetEnemySorceryResistance(i);
						iSuccessRate = wBaseSuccessRate - wSorceryResistance;
						if (fAlwaysSuccess || PAL_New_GetTrueByPercentage(iSuccessRate))
						{
							PAL_New_AddPoisonForEnemy(i, wPoisonID);
						}
#ifdef ADD_POISON_FAIL_THEN_JUMP
						else if (pScript->rgwOperand[2] != 0x0000 && pScript->rgwOperand[2] != 0xffff)
						{
							fJump = TRUE;
						}
#endif
					}
				}
			}
			else
			{	//单体
				wEventObjectID = PAL_NEW_CheckAndGetLegalEnemyTarget(wEventObjectID);
				wSorceryResistance = PAL_New_GetEnemySorceryResistance(wEventObjectID);
				iSuccessRate = wBaseSuccessRate - wSorceryResistance;

				if (fAlwaysSuccess || PAL_New_GetTrueByPercentage(iSuccessRate))
				{
					PAL_New_AddPoisonForEnemy(wEventObjectID, wPoisonID);
				}
#ifdef ADD_POISON_FAIL_THEN_JUMP
				else if (pScript->rgwOperand[2] != 0x0000 && pScript->rgwOperand[2] != 0xffff)
				{
					fJump = TRUE;
				}
			}
			if (fJump == TRUE)
			{
				wScriptEntry = pScript->rgwOperand[2] - 1;
			}
#else
		}
#endif
			break;
		}

			// Apply poison to player
		case 0x0029:
			/*
			29 参数1 参数2	   我方中毒指令
			参数1 是否全体
			参数2 中毒代号
			作用	   我方选定对象或全体中参数2代表的毒
			*/
		{
			BOOL fAll = (pScript->rgwOperand[0] == 0) ? FALSE : TRUE;
			WORD wPoisonID = pScript->rgwOperand[1];
			BOOL fJump = FALSE;
			BOOL fAlwaysSuccess = FALSE;
			WORD wPoisonResistance = 0;

#ifdef ADD_SOME_POISONS_SUCCESSFULLY_ANYTIME
			if (pScript->rgwOperand[2] == 0xffff
				|| gpGlobals->g.rgObject[pScript->rgwOperand[1]].poison.wPoisonLevel >= 99)
			{
				fAlwaysSuccess = TRUE;
			}
#endif
			if (fAll)
			{	// Apply to everyone
				for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
				{
					w = gpGlobals->rgParty[i].wPlayerRole;
					wPoisonResistance = PAL_GetPlayerPoisonResistance(w);
					if (fAlwaysSuccess || !PAL_New_GetTrueByPercentage(wPoisonResistance))
					{
						PAL_AddPoisonForPlayer(w, pScript->rgwOperand[1]);
					}
#ifdef ADD_POISON_FAIL_THEN_JUMP
					else if (pScript->rgwOperand[2] != 0x0000 && pScript->rgwOperand[2] != 0xffff)
					{
						fJump = TRUE;
					}
#endif
				}
			}
			else
			{	// Apply to one player
				wPoisonResistance = PAL_GetPlayerPoisonResistance(wEventObjectID);
				if (fAlwaysSuccess || !PAL_New_GetTrueByPercentage(wPoisonResistance))
				{
					PAL_AddPoisonForPlayer(wEventObjectID, pScript->rgwOperand[1]);
				}
#ifdef ADD_POISON_FAIL_THEN_JUMP
				else if (pScript->rgwOperand[2] != 0x0000 && pScript->rgwOperand[2] != 0xffff)
				{
					fJump = TRUE;
				}
			}

			if (fJump == TRUE)
			{
				wScriptEntry = pScript->rgwOperand[2] - 1;
			}
#else
		}
#endif
			break;
		}

			// Cure poison by object ID for enemy
		case 0x002A:
		{
			/*
			2A 参数1 参数2	   敌方解毒指令
			参数1 是否全体
			参数2 中毒代号
			作用	   敌方选定对象或全体解除参数2代表的毒
			*/
			BOOL fAll = (pScript->rgwOperand[0] == 0) ? FALSE : TRUE;
			WORD wPoisonID = pScript->rgwOperand[1];

			if (fAll)
			{	// Apply to all enemies
				for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
				{
					if (g_Battle.rgEnemy[i].wObjectID != 0)
					{
						PAL_New_CurePoisonForEnemyByKind(i, wPoisonID);
					}
				}
			}
			else
			{	// Apply to one enemy
				PAL_New_CurePoisonForEnemyByKind(wEventObjectID, wPoisonID);
			}
			break;
		}

			// Cure poison by object ID for player
		case 0x002B:
		{
			/*
			我方解毒指令
			参数1 是否全体
			参数2 中毒代号
			作用:		我方选定对象或全体解除参数2代表的毒
			*/
			BOOL fAll = (pScript->rgwOperand[0] == 0) ? FALSE : TRUE;
			WORD wPoisonID = pScript->rgwOperand[1];

			if (fAll)
			{
				for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
				{
					w = gpGlobals->rgParty[i].wPlayerRole;
					PAL_CurePoisonByKind(w, wPoisonID);
				}
			}
			else
			{
				PAL_CurePoisonByKind(wEventObjectID, wPoisonID);
			}
			break;
		}

			// Cure poisons by level
		case 0x002C:
		{
			/*
			2C 参数1 参数2		我方多重解毒指令
			参数1 是否全体
			参数2 整数
			作用		我方选定对象或全体解除毒性不高于参数2的毒
			*/
			BOOL fAll = (pScript->rgwOperand[0] == 0) ? FALSE : TRUE;
			WORD wPoisonID = pScript->rgwOperand[1];

			if (fAll)
			{
				for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
				{
					w = gpGlobals->rgParty[i].wPlayerRole;
					PAL_CurePoisonByLevel(w, wPoisonID);
				}
			}
			else
			{
				PAL_CurePoisonByLevel(wEventObjectID, wPoisonID);
			}
			break;
		}

			// Set the status for player
		case 0x002D:
			/*
			2D 参数1 参数2		   我方特殊状态指令
			参数1 状态代号		    00：疯魔
			01：定身
			02：昏睡
			03：咒封
			04：死者继续攻击
			05：普通攻击加强
			06：防御加强
			07：身法加强
			08：两次攻击
			参数2 整
			作用		   我方选定对象获得参数1代表特殊状态，持续参数2回合
			*/
		{

#ifdef INVINCIBLE
			if (pScript->rgwOperand[0] < 4)
			{
				break;
			}
#endif
			BOOL fAlwaysSuccess = FALSE;
			WORD wSorceryResistance = PAL_New_GetPlayerSorceryResistance(wEventObjectID);
			
			if (gpGlobals->fInBattle)
			{
				WORD wPlayerIndex = PAL_New_GetPlayerIndex(wEventObjectID);
				if (wPlayerIndex != 0xffff && g_Battle.rgPlayer[wPlayerIndex].fDefending)
				{
					wSorceryResistance += 10;
				}
			}

			INT iSuccessRate = 0;
			WORD wBaseSuccessRate = 100;
			WORD wStatusID = pScript->rgwOperand[0];
			WORD wNumRound = pScript->rgwOperand[1];

#ifdef ADD_SOME_STATUSES_SUCCESSFULLY_ANYTIME
			if (pScript->rgwOperand[2] == 0xffff && wStatusID >= 4)	//有益状态总是成功
			{
				fAlwaysSuccess = TRUE;
			}
#endif
			iSuccessRate = wBaseSuccessRate - wSorceryResistance;
			if (fAlwaysSuccess || PAL_New_GetTrueByPercentage(iSuccessRate))
			{   //仅对不良状态有抗性  //有益状态直接加
				PAL_SetPlayerStatus(wEventObjectID, wStatusID, wNumRound);
			}
			else if (pScript->rgwOperand[2] != 0x0000 && pScript->rgwOperand[2] != 0xffff)
			{
				wScriptEntry = pScript->rgwOperand[2] - 1;
			}
			break;
		}

			// Set the status for enemy
		case 0x002E:
			/*
			2E 参数1 参数2 参数3	   敌方特殊状态指令
			参数1 状态代号
			参数2 整数
			参数3 脚本地址
			作用	   敌方选定对象获得参数1代表特殊状态，持续参数2回合，若不受该状态影响，则
			跳转参数3指向的脚本地址
			*/
		{
			BOOL fAlwaysSuccess = FALSE;
			BOOL fSorceryIsFull = FALSE;
			WORD wSorceryResistance = 0;
			INT iSuccessRate = 0;
			WORD wBaseSuccessRate = 100;
			WORD wStatusID = pScript->rgwOperand[0];
			WORD wNumRound = pScript->rgwOperand[1];

			wEventObjectID = PAL_NEW_CheckAndGetLegalEnemyTarget(wEventObjectID);
			wSorceryResistance = PAL_New_GetEnemySorceryResistance(wEventObjectID);
			fSorceryIsFull = wSorceryResistance >= 100 ? TRUE : FALSE;

			if (g_Battle.fPlayerMoving && !fSorceryIsFull)
			{
				wPlayerRole = gpGlobals->rgParty[g_Battle.wMovingPlayerIndex].wPlayerRole;
				wPlayerRole = PAL_NEW_CheckAndGetLegalPlayerTarget(wPlayerRole);	//如果
				wBaseSuccessRate += PAL_New_GetPlayerSorceryResistance(wPlayerRole);//巫抗越高设置状态成功率越高
			}

#ifdef ADD_SOME_STATUSES_SUCCESSFULLY_ANYTIME
			if (pScript->rgwOperand[2] == 0xffff)
			{
				fAlwaysSuccess = TRUE;
			}
#endif
			iSuccessRate = wBaseSuccessRate - wSorceryResistance;
			if (fAlwaysSuccess || PAL_New_GetTrueByPercentage(iSuccessRate))
			{
				PAL_New_SetEnemyStatus(wEventObjectID, wStatusID, wNumRound);
			}
			else
			{
				wScriptEntry = pScript->rgwOperand[2] - 1;
			}
			break;
		}

			// Remove player's status
		case 0x002F:
			/*
			2F 参数1	   我方解除特殊状态指令
			参数1 状态代号
			00：疯魔
			01：定身
			02：昏睡
			03：咒封
			04：死者继续攻击
			05：普通攻击加强
			06：防御加强
			07：身法加强
			08：两次攻击
			作用	   我方选定对象解除参数1代表的特殊状态
			*/
		{
			PAL_RemovePlayerStatus(wEventObjectID, pScript->rgwOperand[0]);
			break;
		}

			// Increase player's stat temporarily by percent // 按百分比暂时增加角色属性
		case 0x0030:
			/*
			11：武术
			12：灵力
			13：防御
			14：身法
			15：吉运
			16：毒抗
			17：风抗
			18：雷抗
			19：水抗
			1A：火抗
			1B：土抗
			*/
		{
			WORD *p = (WORD *)(&gpGlobals->rgEquipmentEffect[kBodyPartExtra]); // HACKHACK
			WORD *p1 = (WORD *)(&gpGlobals->g.PlayerRoles);

			if (pScript->rgwOperand[2] == 0)
			{
				iPlayerRole = wEventObjectID;
			}
			else
			{
				iPlayerRole = pScript->rgwOperand[2] - 1;
			}

			p[pScript->rgwOperand[0] * MAX_PLAYER_ROLES + iPlayerRole] =
				p1[pScript->rgwOperand[0] * MAX_PLAYER_ROLES + iPlayerRole] *
				(SHORT)pScript->rgwOperand[1] / 100;

			break;
		}

			// Change battle sprite temporarily for player
		case 0x0031:
		{
			gpGlobals->rgEquipmentEffect[kBodyPartExtra].rgwSpriteNumInBattle[wEventObjectID] =
				pScript->rgwOperand[0];
			break;
		}

			// collect the enemy for items // 收集妖怪炼丹
		case 0x0033:
		{
			if (g_Battle.rgEnemy[wEventObjectID].e.wCollectValue != 0)
			{
				gpGlobals->wCollectValue +=
					g_Battle.rgEnemy[wEventObjectID].e.wCollectValue;
			}
			else
			{
				wScriptEntry = pScript->rgwOperand[0] - 1;
			}
			break;
		}

			// Transform collected enemies into items // 灵葫炼丹
		case 0x0034:
			/*
			34 参数1	   灵葫炼丹指令
			参数1：脚本地址
			作用	   调用灵葫炼丹程序，根据现有灵葫值，从data.mkf第1个子文件的第一组数据中随
			机炼成一种物品，如灵葫值为0，则跳转参数1指向的脚本地址
			*/
		{
			if (gpGlobals->wCollectValue > 0)
			{
				char s[256];

#ifndef EDIT_SCRIPT_OPERATION_0X0034
				i = RandomLong(1, gpGlobals->wCollectValue);
				if (i > 9)
				{
					i = 9;
				}
#else
				if (gpGlobals->wCollectValue > 370)
				{
					i = 9;
					gpGlobals->wCollectValue -= i * 30;
					i--;
					PAL_AddItemToInventory(gpGlobals->g.lprgStore[0].rgwItems[i], 30);
					PAL_StartDialog(kDialogCenterWindow, 0, 0, FALSE);
					strcpy(s, PAL_GetWord(42));
					strcat(s, PAL_GetWord(gpGlobals->g.lprgStore[0].rgwItems[i]));
					strcat(s, " *30");
					PAL_ShowDialogText(s);
					break;
				}
				else if (gpGlobals->wCollectValue < 100)
				{
					i = RandomLong(1, 9);
					if (i > gpGlobals->wCollectValue)
					{
						i = gpGlobals->wCollectValue;
					}
				}
				else
				{
					i = RandomLong(1, gpGlobals->wCollectValue);
					if (i > 9)
					{
						i = 9;
					}
				}
#endif
				gpGlobals->wCollectValue -= i;
				i--;
				PAL_AddItemToInventory(gpGlobals->g.lprgStore[0].rgwItems[i], 1);
				PAL_StartDialog(kDialogCenterWindow, 0, 0, FALSE);
				strcpy(s, PAL_GetWord(42));
				strcat(s, PAL_GetWord(gpGlobals->g.lprgStore[0].rgwItems[i]));
				PAL_ShowDialogText(s);
			}
			else
			{
				wScriptEntry = pScript->rgwOperand[0] - 1;
			}
			break;
		}

			// Shake the screen // 摇动屏幕
		case 0x0035:
		{
			i = pScript->rgwOperand[1];
			if (i == 0)
			{
				i = 4;
			}
			VIDEO_ShakeScreen(pScript->rgwOperand[0], i);
			if (!pScript->rgwOperand[0])
			{
				VIDEO_UpdateScreen(NULL);
			}
			break;
		}

			// Set the current playing RNG animation // 设置当前正在播放的RNG动画
		case 0x0036:
		{
			gpGlobals->iCurPlayingRNG = pScript->rgwOperand[0];
			break;
		}

			// Play RNG animation
		case 0x0037:
		{
			PAL_RNGPlay(gpGlobals->iCurPlayingRNG,
				pScript->rgwOperand[0],
				pScript->rgwOperand[1] > 0 ? pScript->rgwOperand[1] : 999,
				pScript->rgwOperand[2] > 0 ? pScript->rgwOperand[2] : 16);
			break;
		}

			// Teleport the party out of the scene
		case 0x0038:
		{
			if (!gpGlobals->fInBattle &&
				gpGlobals->g.rgScene[gpGlobals->wNumScene - 1].wScriptOnTeleport != 0)
			{
				gpGlobals->g.rgScene[gpGlobals->wNumScene - 1].wScriptOnTeleport =
					PAL_RunTriggerScript(gpGlobals->g.rgScene[gpGlobals->wNumScene - 1].wScriptOnTeleport, 0xFFFF);
			}
			else
			{
				//
				// failed
				//
				g_fScriptSuccess = FALSE;
				wScriptEntry = pScript->rgwOperand[0] - 1;
			}
			break;
		}

			// Drain HP from enemy // 吸取生命
		case 0x0039:
			/*
			吸取生命指令
			参数0：整数
			作用:		吸取选定对象"参数0"点生命，用于补充动作对象的体力
			*/
		{
			SHORT sDamage = min((SHORT)pScript->rgwOperand[0], g_Battle.rgEnemy[wEventObjectID].dwActualHealth);
			g_Battle.rgEnemy[wEventObjectID].dwActualHealth -= sDamage;

			if (g_Battle.fPlayerMoving)
			{
				wPlayerRole = gpGlobals->rgParty[g_Battle.wMovingPlayerIndex].wPlayerRole;
			}
			else
			{
				int index = PAL_New_GetPlayerIndexByHealth(TRUE);
				wPlayerRole = gpGlobals->rgParty[index].wPlayerRole;
				wPlayerRole = PAL_NEW_CheckAndGetLegalPlayerTarget(wPlayerRole);
			}

			PAL_IncreaseHPMP(wPlayerRole, sDamage, 0);
			break;
		}

			// Player flee from the battle // 战斗中逃走
		case 0x003A:
		{
			if (g_Battle.fIsBoss)
			{
				//
				// Cannot flee from bosses
				//
				wScriptEntry = pScript->rgwOperand[0] - 1;
			}
			else
			{
				PAL_BattlePlayerEscape();
			}
			break;
		}

			// Ride the event object to the specified position, at a low speed
		case 0x003F:
		{
			PAL_PartyRideEventObject(wEventObjectID, pScript->rgwOperand[0], pScript->rgwOperand[1],
				pScript->rgwOperand[2], 2);
			break;
		}

			// set the trigger method for a event object
		case 0x0040:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pCurrent == NULL)
			{
				break;
			}
#endif
			if (pScript->rgwOperand[0] != 0)
			{
				pCurrent->wTriggerMode = pScript->rgwOperand[1];
			}
			break;
		}

			// Mark the script as failed // 令脚本执行失败
		case 0x0041:
		{
			g_fScriptSuccess = FALSE;
			break;
		}

			// Simulate a magic for player // 模拟法术
		case 0x0042:
		{
			i = (SHORT)(pScript->rgwOperand[2]) - 1;
			if (i < 0)
			{
				i = wEventObjectID;
			}
			PAL_BattleSimulateMagic(i, pScript->rgwOperand[0], pScript->rgwOperand[1], TRUE);
			break;
		}

			// Set background music
		case 0x0043:
		{
			gpGlobals->wNumMusic = pScript->rgwOperand[0];
			PAL_PlayMUS(pScript->rgwOperand[0], (pScript->rgwOperand[0] != 0x3D), pScript->rgwOperand[1]);
			break;
		}

			// Ride the event object to the specified position, at the normal speed
		case 0x0044:
		{
			PAL_PartyRideEventObject(wEventObjectID, pScript->rgwOperand[0], pScript->rgwOperand[1],
				pScript->rgwOperand[2], 4);
			break;
		}

			// Set battle music
		case 0x0045:
		{
			gpGlobals->wNumBattleMusic = pScript->rgwOperand[0];
			break;
		}

			// Set the party position on the map
		case 0x0046:
		{
			int xOffset, yOffset, x, y;

			xOffset =
				((gpGlobals->wPartyDirection == kDirWest || gpGlobals->wPartyDirection == kDirSouth)
				? 16 : -16);
			yOffset =
				((gpGlobals->wPartyDirection == kDirWest || gpGlobals->wPartyDirection == kDirNorth)
				? 8 : -8);

			x = pScript->rgwOperand[0] * 32 + pScript->rgwOperand[2] * 16;
			y = pScript->rgwOperand[1] * 16 + pScript->rgwOperand[2] * 8;

			x -= PAL_X(gpGlobals->partyoffset);
			y -= PAL_Y(gpGlobals->partyoffset);

			gpGlobals->viewport = PAL_XY(x, y);

			x = PAL_X(gpGlobals->partyoffset);
			y = PAL_Y(gpGlobals->partyoffset);

			for (i = 0; i < MAX_PLAYABLE_PLAYER_ROLES; i++)
			{
				gpGlobals->rgParty[i].x = x;
				gpGlobals->rgParty[i].y = y;
				gpGlobals->rgTrail[i].x = x + PAL_X(gpGlobals->viewport);
				gpGlobals->rgTrail[i].y = y + PAL_Y(gpGlobals->viewport);
				gpGlobals->rgTrail[i].wDirection = gpGlobals->wPartyDirection;

				x += xOffset;
				y += yOffset;
			}
		
			break;
		}

			// Play sound effect
		case 0x0047:
		{
			SOUND_Play(pScript->rgwOperand[0]);
			break;
		}

			// Set the state of event object
		case 0x0049:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pCurrent == NULL)
			{
				break;
			}
#endif
			pCurrent->sState = pScript->rgwOperand[1];
			break;
		}

			// Set the current battlefield
		case 0x004A:
		{
			gpGlobals->wNumBattleField = pScript->rgwOperand[0];
			break;
		}

			// Nullify the event object for a short while
		case 0x004B:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pEvtObj == NULL)
			{
				break;
			}
#endif
			pEvtObj->sVanishTime = -15;
			break;
		}

			// chase the player
		case 0x004C:
		{
			WORD wMaxDistance = pScript->rgwOperand[0]; // max. distance
			WORD wSpeed = pScript->rgwOperand[1]; // speed

			if (wMaxDistance == 0)
			{
				wMaxDistance = 8;
			}
			if (wSpeed == 0)
			{
				wSpeed = 4;
			}

			PAL_MonsterChasePlayer(wEventObjectID, wSpeed, wMaxDistance, pScript->rgwOperand[2]);
			break;
		}

			// wait for any key
		case 0x004D:
		{
			PAL_WaitForKey(0);
			break;
		}

			// Load the last saved game
		case 0x004E:
		{
			PAL_FadeOut(1);
			PAL_InitGameData(gpGlobals->bCurrentSaveSlot);
			return 0; // don't go further
		}

			// Fade the screen to red color (game over)
		case 0x004F:
		{
			PAL_FadeToRed();
			break;
		}

			// screen fade out
		case 0x0050:
		{
			VIDEO_UpdateScreen(NULL);
			PAL_FadeOut(pScript->rgwOperand[0] ? pScript->rgwOperand[0] : 1);
			gpGlobals->fNeedToFadeIn = TRUE;
			break;
		}

			// screen fade in
		case 0x0051:
		{
			VIDEO_UpdateScreen(NULL);
			PAL_FadeIn(gpGlobals->wNumPalette, gpGlobals->fNightPalette,
				((SHORT)(pScript->rgwOperand[0]) > 0) ? pScript->rgwOperand[0] : 1);
			gpGlobals->fNeedToFadeIn = FALSE;
			break;
		}

			// hide the event object for a while, default 800 frames
		case 0x0052:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pEvtObj == NULL)
			{
				break;
			}
#endif
			pEvtObj->sState *= -1;
			pEvtObj->sVanishTime = (pScript->rgwOperand[0] ? pScript->rgwOperand[0] : 800);
			break;
		}

			// use the day palette
		case 0x0053:
		{
			gpGlobals->fNightPalette = FALSE;
			break;
		}

			// use the night palette
		case 0x0054:
		{
			gpGlobals->fNightPalette = TRUE;
			break;
		}

			//Add magic to a player
		case 0x0055:
		{
			i = pScript->rgwOperand[1];
			if (i == 0)
			{
				i = wEventObjectID;
			}
			else
			{
				i--;
			}
			PAL_AddMagic(i, pScript->rgwOperand[0]);
			break;
		}

			// 移除仙术
		case 0x0056:
		{
			i = pScript->rgwOperand[1];
			if (i == 0)
			{
				i = wEventObjectID;
			}
			else
			{
				i--;
			}
			PAL_RemoveMagic(i, pScript->rgwOperand[0]);
			break;
		}

			//根据真气设定仙术的基础伤害值
		case 0x0057:
		{
			WORD wBaseDamage = 0;
			WORD wCostMP = gpGlobals->g.PlayerRoles.rgwMP[wEventObjectID];

#ifdef FINISH_GAME_MORE_ONE_TIME
			if (gpGlobals->bFinishGameTime < 2)
			{
				wCostMP = min(wCostMP, MAX_PARAMETER + MAX_PARAMETER_EXTRA);
			}
#endif
			i = ((pScript->rgwOperand[1] == 0) ? 8 : pScript->rgwOperand[1]);
			j = gpGlobals->g.rgObject[pScript->rgwOperand[0]].magic.wMagicNumber;
			wBaseDamage = min(wCostMP * i, MAX_DAMAGE);
			gpGlobals->g.lprgMagic[j].wBaseDamage = wBaseDamage;
			gpGlobals->g.PlayerRoles.rgwMP[wEventObjectID] -= wCostMP;
			break;
		}

			// Jump if there is less than the specified number of the specified items in the inventory
		case 0x0058:
		{
			if (PAL_GetItemAmount(pScript->rgwOperand[0]) < (SHORT)(pScript->rgwOperand[1]))
			{
				wScriptEntry = pScript->rgwOperand[2] - 1;
			}
			break;
		}

			// Change to the specified scene
		case 0x0059:
		{
			if (pScript->rgwOperand[0] > 0 && pScript->rgwOperand[0] <= MAX_SCENES &&
				gpGlobals->wNumScene != pScript->rgwOperand[0])
			{
				//
				// Set data to load the scene in the next frame
				//
				gpGlobals->wNumScene = pScript->rgwOperand[0];
				PAL_SetLoadFlags(kLoadScene);
				gpGlobals->fEnteringScene = TRUE;
				gpGlobals->wLayer = 0;
			}
			break;
		}

			// Halve the player's HP
		case 0x005A:
		{
			wPlayerRole = wEventObjectID;
			gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] /= 2;
			break;
		}

			// Halve the enemy's HP
		case 0x005B:
		{
			WORD wMaxDamage = pScript->rgwOperand[0];
			DWORD dwActualHealth = g_Battle.rgEnemy[wEventObjectID].dwActualHealth;
			INT iDamage = min(dwActualHealth / 2 + 1, wMaxDamage);
			iDamage = min(iDamage, dwActualHealth);
			g_Battle.rgEnemy[wEventObjectID].dwActualHealth -= iDamage;
			break;
		}

			// Hide for a while
		case 0x005C:
		{
			g_Battle.iHidingTime = (INT)(pScript->rgwOperand[0]);
			break;
		}

			// Jump if player doesn't have the specified poison // 如果角色没有中某一种特定的毒，则跳转
		case 0x005D:
		{
			if (PAL_New_GetPoisonIndexForPlayer(wEventObjectID, pScript->rgwOperand[0]) == -1)
			{
				wScriptEntry = pScript->rgwOperand[1] - 1;
			}
			break;
		}

			// Jump if enemy doesn't have the specified poison
		case 0x005E:
		{
			if (PAL_New_GetPoisonIndexForEnemy(wEventObjectID, pScript->rgwOperand[0]) == -1)
			{
				wScriptEntry = pScript->rgwOperand[1] - 1;
			}
			break;
		}

			// Kill the player immediately
		case 0x005F:
		{
#ifdef USE_LACKY_VALUE
			WORD str = PAL_GetPlayerFleeRate(wEventObjectID);
			WORD wSource = PAL_NEW_CheckAndGetLegalEnemyTarget(g_Battle.wMovingEnemyIndex);
			SHORT def = (SHORT)(g_Battle.rgEnemy[wSource].e.wFleeRate);
			def += g_Battle.rgEnemy[wSource].e.wLevel * 3;
			def = max(def, 0);
			WORD rStr = RandomLong(0, str);
			WORD rHStr = RandomLong(0, str / 2);
			WORD rDef = RandomLong(0, def);


			if ((str > def && rStr > rDef) || (str <= def && str * 2 > def && rHStr > rDef))
			{
				gpGlobals->g.PlayerRoles.rgwHP[wEventObjectID] = 1;
				gpGlobals->Exp.rgFleeExp[wEventObjectID].wCount += RandomLong(0, 1);
			}
			else
			{
				gpGlobals->g.PlayerRoles.rgwHP[wEventObjectID] = 0;
				gpGlobals->Exp.rgFleeExp[wEventObjectID].wCount += RandomLong(1, 2);
			}
#else
			gpGlobals->g.PlayerRoles.rgwHP[wEventObjectID] = 0;
#endif 
			break;
		}

			// Kill the enemy immediately
		case 0x0060:
		{
			g_Battle.rgEnemy[wEventObjectID].dwActualHealth = 0;
			break;
		}

			// Jump if player is not poisoned
		case 0x0061:
		{
			if (PAL_New_IsPlayerPoisoned(wEventObjectID) == FALSE) //之前用的函数不能正确判断等级为0的毒，比如赤毒
			{
				wScriptEntry = pScript->rgwOperand[0] - 1;
			}
			break;
		}

			// Pause enemy chasing for a while
		case 0x0062:
		{
			gpGlobals->wChasespeedChangeCycles = pScript->rgwOperand[0];
			gpGlobals->wChaseRange = 0;
			break;
		}

			// Speed up enemy chasing for a while
		case 0x0063:
		{
			gpGlobals->wChasespeedChangeCycles = pScript->rgwOperand[0];
			gpGlobals->wChaseRange = 3;
			break;
		}

			// Jump if enemy's HP is more than the specified percentage
		case 0x0064:
		{
			if ((g_Battle.rgEnemy[wEventObjectID].dwActualHealth) * 100 >
				(g_Battle.rgEnemy[wEventObjectID].dwMaxHealth) * pScript->rgwOperand[0])
			{
				wScriptEntry = pScript->rgwOperand[1] - 1;
			}
			break;
		}

			// Set the player's sprite
		case 0x0065:
		{
			gpGlobals->g.PlayerRoles.rgwSpriteNum[pScript->rgwOperand[0]] = pScript->rgwOperand[1];
			if (!gpGlobals->fInBattle && pScript->rgwOperand[2])
			{
				PAL_SetLoadFlags(kLoadPlayerSprite);
				PAL_LoadResources();
			}
			break;
		}

			// Throw weapon to enemy
		case 0x0066:
		{
			WORD wBaseDamage = pScript->rgwOperand[1] * 5;
			wBaseDamage += RandomLong(0, 4);

			if (g_Battle.fPlayerMoving)
			{
				wPlayerRole = gpGlobals->rgParty[g_Battle.wMovingPlayerIndex].wPlayerRole;
				wBaseDamage += gpGlobals->g.PlayerRoles.rgwAttackStrength[wPlayerRole];
				wBaseDamage += PAL_GetPlayerAttackStrength(wPlayerRole);
			}
			else
			{
				wBaseDamage += 100;
			}

#ifdef REMOVE_TEMPORARY_BASEDAMAGE_WHEN_SIMULATE_MAGIC
			wBaseDamage += RandomLong(200, 205);
			PAL_BattleSimulateMagic((SHORT)wEventObjectID, pScript->rgwOperand[0], wBaseDamage, FALSE);
#else
			PAL_BattleSimulateMagic((SHORT)wEventObjectID, pScript->rgwOperand[0], wBaseDamage, TRUE);
#endif
			break;
		}

			// Enemy use magic
		case 0x0067:
		{
			g_Battle.rgEnemy[wEventObjectID].e.wMagic = pScript->rgwOperand[0];
			g_Battle.rgEnemy[wEventObjectID].e.wMagicRate =
				((pScript->rgwOperand[1] == 0) ? 10 : pScript->rgwOperand[1]);
			break;
		}

			// Jump if it's enemy's turn
		case 0x0068:
		{
			if (g_Battle.fEnemyMoving)
			{
				wScriptEntry = pScript->rgwOperand[0] - 1;
			}
			break;
		}

			// Enemy escape in battle
		case 0x0069:
		{
			PAL_BattleEnemyEscape();
			break;
		}

			// Steal from the enemy
		case 0x006A:
		{
			PAL_BattleStealFromEnemy(wEventObjectID, pScript->rgwOperand[0]);
			break;
		}

			// Blow away enemies
		case 0x006B:
		{
			g_Battle.iBlow = (SHORT)(pScript->rgwOperand[0]);
			break;
		}

			// Walk the NPC in one step
		case 0x006C:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pCurrent == NULL)
			{
				break;
			}
#endif
			pCurrent->x += (SHORT)(pScript->rgwOperand[1]);
			pCurrent->y += (SHORT)(pScript->rgwOperand[2]);
			PAL_NPCWalkOneStep(wCurEventObjectID, 0);
			break;
		}

			// Set the enter script and teleport script for a scene
		case 0x006D:
		{
			if (pScript->rgwOperand[0])
			{
				if (pScript->rgwOperand[1])
				{
					gpGlobals->g.rgScene[pScript->rgwOperand[0] - 1].wScriptOnEnter =
						pScript->rgwOperand[1];
				}

				if (pScript->rgwOperand[2])
				{
					gpGlobals->g.rgScene[pScript->rgwOperand[0] - 1].wScriptOnTeleport =
						pScript->rgwOperand[2];
				}

				if (pScript->rgwOperand[1] == 0 && pScript->rgwOperand[2] == 0)
				{
					gpGlobals->g.rgScene[pScript->rgwOperand[0] - 1].wScriptOnEnter = 0;
					gpGlobals->g.rgScene[pScript->rgwOperand[0] - 1].wScriptOnTeleport = 0;
				}
			}
			break;
		}

			// Move the player to the specified position in one step
		case 0x006E:
		{
			for (i = MAX_PLAYABLE_PLAYER_ROLES - 2; i >= 0; i--)
			{
				gpGlobals->rgTrail[i + 1] = gpGlobals->rgTrail[i];
			}
			gpGlobals->rgTrail[0].wDirection = gpGlobals->wPartyDirection;
			gpGlobals->rgTrail[0].x = PAL_X(gpGlobals->viewport) + PAL_X(gpGlobals->partyoffset);
			gpGlobals->rgTrail[0].y = PAL_Y(gpGlobals->viewport) + PAL_Y(gpGlobals->partyoffset);

			gpGlobals->viewport = PAL_XY(
				PAL_X(gpGlobals->viewport) + (SHORT)(pScript->rgwOperand[0]),
				PAL_Y(gpGlobals->viewport) + (SHORT)(pScript->rgwOperand[1]));

			gpGlobals->wLayer = pScript->rgwOperand[2] * 8;

			if (pScript->rgwOperand[0] != 0 || pScript->rgwOperand[1] != 0)
			{
				PAL_UpdatePartyGestures(TRUE);
			}
			break;
		}

			// Sync the state of current event object with another event object
		case 0x006F:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pEvtObj == NULL || pCurrent == NULL)
			{
				break;
			}
#endif
			if (pCurrent->sState == (SHORT)(pScript->rgwOperand[1]))
			{
				pEvtObj->sState = (SHORT)(pScript->rgwOperand[1]);
			}
			break;
		}

			// Walk the party to the specified position
		case 0x0070:
		{
			PAL_PartyWalkTo(pScript->rgwOperand[0], pScript->rgwOperand[1], pScript->rgwOperand[2], 2);
			break;
		}

			// Wave the screen
		case 0x0071:
		{
			gpGlobals->wScreenWave = pScript->rgwOperand[0];
			gpGlobals->sWaveProgression = (SHORT)(pScript->rgwOperand[1]);
			break;
		}

			// Fade the screen to scene
		case 0x0073:
		{
			VIDEO_BackupScreen();
			PAL_MakeScene();
			VIDEO_FadeScreen(pScript->rgwOperand[0]);
			break;
		}

			// Jump if not all players are full HP
		case 0x0074:
		{
			for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				w = gpGlobals->rgParty[i].wPlayerRole;
				if (gpGlobals->g.PlayerRoles.rgwHP[w] < gpGlobals->g.PlayerRoles.rgwMaxHP[w])
				{
					wScriptEntry = pScript->rgwOperand[0] - 1;
					break;
				}
			}
			break;
		}

			// Set the player party
		case 0x0075:
		{
			gpGlobals->wMaxPartyMemberIndex = 0;
			for (i = 0; i < 3; i++)
			{
				if (pScript->rgwOperand[i] != 0)
				{
					gpGlobals->rgParty[gpGlobals->wMaxPartyMemberIndex].wPlayerRole =
						pScript->rgwOperand[i] - 1;

					g_Battle.rgPlayer[gpGlobals->wMaxPartyMemberIndex].action.ActionType =
						kBattleActionAttack;

					gpGlobals->wMaxPartyMemberIndex++;
				}
			}

			if (gpGlobals->wMaxPartyMemberIndex == 0)
			{
				// HACK for Dream 2.11
				gpGlobals->rgParty[0].wPlayerRole = 0;
				gpGlobals->wMaxPartyMemberIndex = 1;
			}

			gpGlobals->wMaxPartyMemberIndex--;

			//
			// Reload the player sprites
			//
			PAL_SetLoadFlags(kLoadPlayerSprite);
			PAL_LoadResources();

			memset(gpGlobals->rgPoisonStatus, 0, sizeof(gpGlobals->rgPoisonStatus));
			PAL_UpdateEquipments();
			break;
		}

			// Show FBP picture
		case 0x0076:
		{
#ifdef PAL_WIN95
			SDL_FillRect(gpScreen, NULL, 0);
			VIDEO_UpdateScreen(NULL);
#else
			PAL_EndingSetEffectSprite(0);
			PAL_ShowFBP(pScript->rgwOperand[0], pScript->rgwOperand[1]);
#endif
			break;
		}

			// Stop current playing music
		case 0x0077:
		{
			PAL_PlayMUS(0, FALSE,
				(pScript->rgwOperand[0] == 0) ? 2.0f : (FLOAT)(pScript->rgwOperand[0]) * 2);
			gpGlobals->wNumMusic = 0;
			break;
		}

			// FIXME: ???
		case 0x0078:
		{
			break;
		}

			// Jump if the specified player is in the party
		case 0x0079:
		{
			for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				if (gpGlobals->g.PlayerRoles.rgwName[gpGlobals->rgParty[i].wPlayerRole] ==
					pScript->rgwOperand[0])
				{
					wScriptEntry = pScript->rgwOperand[1] - 1;
					break;
				}
			}
			break;
		}

			// Walk the party to the specified position, at a higher speed
		case 0x007A:
		{
			PAL_PartyWalkTo(pScript->rgwOperand[0], pScript->rgwOperand[1], pScript->rgwOperand[2], 4);
			break;
		}

			// Walk the party to the specified position, at the highest speed
		case 0x007B:
		{
			PAL_PartyWalkTo(pScript->rgwOperand[0], pScript->rgwOperand[1], pScript->rgwOperand[2], 8);
			break;
		}

			// Walk straight to the specified position
		case 0x007C:
		{
			if ((wEventObjectID & 1) ^ (gpGlobals->dwFrameNum & 1))
			{
				if (!PAL_NPCWalkTo(wEventObjectID, pScript->rgwOperand[0], pScript->rgwOperand[1],
					pScript->rgwOperand[2], 4))
				{
					wScriptEntry--;
				}
			}
			else
			{
				wScriptEntry--;
			}
			break;
		}

			// Move the event object
		case 0x007D:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pCurrent == NULL)
			{
				break;
			}
#endif
			pCurrent->x += (SHORT)(pScript->rgwOperand[1]);
			pCurrent->y += (SHORT)(pScript->rgwOperand[2]);
			break;
		}

			// Set the layer of event object
		case 0x007E:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pCurrent == NULL)
			{
				break;
			}
#endif
			pCurrent->sLayer = (SHORT)(pScript->rgwOperand[1]);
			break;
		}

			// Move the viewport
		case 0x007F:
		{
			if (pScript->rgwOperand[0] == 0 && pScript->rgwOperand[1] == 0)
			{
				//
				// Move the viewport back to normal state
				//
				x = gpGlobals->rgParty[0].x - 160;
				y = gpGlobals->rgParty[0].y - 112;

				gpGlobals->viewport =
					PAL_XY(PAL_X(gpGlobals->viewport) + x, PAL_Y(gpGlobals->viewport) + y);
				gpGlobals->partyoffset = PAL_XY(160, 112);

				for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
				{
					gpGlobals->rgParty[i].x -= x;
					gpGlobals->rgParty[i].y -= y;
				}

				if (pScript->rgwOperand[2] != 0xFFFF)
				{
					PAL_MakeScene();
					VIDEO_UpdateScreen(NULL);
				}
			}
			else
			{
				DWORD time;

				i = 0;

				x = (SHORT)(pScript->rgwOperand[0]);
				y = (SHORT)(pScript->rgwOperand[1]);

				time = SDL_GetTicks() + FRAME_TIME;

				do
				{
					if (pScript->rgwOperand[2] == 0xFFFF)
					{
						x = PAL_X(gpGlobals->viewport);
						y = PAL_Y(gpGlobals->viewport);

						gpGlobals->viewport =
							PAL_XY(pScript->rgwOperand[0] * 32 - 160, pScript->rgwOperand[1] * 16 - 112);

						x -= PAL_X(gpGlobals->viewport);
						y -= PAL_Y(gpGlobals->viewport);

						for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
						{
							gpGlobals->rgParty[j].x += x;
							gpGlobals->rgParty[j].y += y;
						}
					}
					else
					{
						gpGlobals->viewport =
							PAL_XY(PAL_X(gpGlobals->viewport) + x, PAL_Y(gpGlobals->viewport) + y);
						gpGlobals->partyoffset =
							PAL_XY(PAL_X(gpGlobals->partyoffset) - x, PAL_Y(gpGlobals->partyoffset) - y);

						for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
						{
							gpGlobals->rgParty[j].x -= x;
							gpGlobals->rgParty[j].y -= y;
						}
					}

					if (pScript->rgwOperand[2] != 0xFFFF)
					{
						PAL_GameUpdate(FALSE);
					}

					PAL_MakeScene();
					VIDEO_UpdateScreen(NULL);

					//
					// Delay for one frame
					//
					PAL_ProcessEvent();
					while (SDL_GetTicks() < time)
					{
						PAL_ProcessEvent();
						SDL_Delay(1);
					}
					time = SDL_GetTicks() + FRAME_TIME;
				} while (++i < (SHORT)(pScript->rgwOperand[2]));
			}
			break;
		}

			// Toggle day/night palette
		case 0x0080:
		{
			gpGlobals->fNightPalette = !(gpGlobals->fNightPalette);
			PAL_PaletteFade(gpGlobals->wNumPalette, gpGlobals->fNightPalette,
				!(pScript->rgwOperand[0]));
			break;
		}

			// Jump if the player is not facing the specified event object
		case 0x0081:
		{
			if (pScript->rgwOperand[0] <= gpGlobals->g.rgScene[gpGlobals->wNumScene - 1].wEventObjectIndex ||
				pScript->rgwOperand[0] > gpGlobals->g.rgScene[gpGlobals->wNumScene].wEventObjectIndex)
			{
				//
				// The event object is not in the current scene
				//
				wScriptEntry = pScript->rgwOperand[2] - 1;
				g_fScriptSuccess = FALSE;
				break;
			}
#ifdef AVOID_SCRIPT_CRASH
			if (pCurrent == NULL)
			{
				break;
			}
#endif
			x = pCurrent->x;
			y = pCurrent->y;

			x +=
				((gpGlobals->wPartyDirection == kDirWest || gpGlobals->wPartyDirection == kDirSouth)
				? 16 : -16);
			y +=
				((gpGlobals->wPartyDirection == kDirWest || gpGlobals->wPartyDirection == kDirNorth)
				? 8 : -8);

			x -= PAL_X(gpGlobals->viewport) + PAL_X(gpGlobals->partyoffset);
			y -= PAL_Y(gpGlobals->viewport) + PAL_Y(gpGlobals->partyoffset);

			if (abs(x) + abs(y * 2) < pScript->rgwOperand[1] * 32 + 16)
			{
				if (pScript->rgwOperand[1] > 0)
				{
					//
					// Change the trigger mode so that the object can be triggered in next frame
					//
#ifdef AVOID_SCRIPT_CRASH
					if (pCurrent == NULL)
					{
						break;
					}
#endif
					pCurrent->wTriggerMode = kTriggerTouchNormal + pScript->rgwOperand[1];
				}
			}
			else
			{
				wScriptEntry = pScript->rgwOperand[2] - 1;
				g_fScriptSuccess = FALSE;
			}
		
			break;
		}
			
			// Walk straight to the specified position, at a high speed
		case 0x0082:
		{
			if (!PAL_NPCWalkTo(wEventObjectID, pScript->rgwOperand[0], pScript->rgwOperand[1],
				pScript->rgwOperand[2], 8))
			{
				wScriptEntry--;
			}
			break;
		}

			// Jump if event object is not in the specified zone of the current event object
		case 0x0083:
		{
			if (pScript->rgwOperand[0] <= gpGlobals->g.rgScene[gpGlobals->wNumScene - 1].wEventObjectIndex ||
				pScript->rgwOperand[0] > gpGlobals->g.rgScene[gpGlobals->wNumScene].wEventObjectIndex)
			{
				//
				// The event object is not in the current scene
				//
				wScriptEntry = pScript->rgwOperand[2] - 1;
				g_fScriptSuccess = FALSE;
				break;
			}

#ifdef AVOID_SCRIPT_CRASH
			if (pEvtObj == NULL || pCurrent == NULL)
			{
				break;
			}
#endif
			x = pEvtObj->x - pCurrent->x;
			y = pEvtObj->y - pCurrent->y;

			if (abs(x) + abs(y * 2) >= pScript->rgwOperand[1] * 32 + 16)
			{
				wScriptEntry = pScript->rgwOperand[2] - 1;
				g_fScriptSuccess = FALSE;
			}
			break;
		}

			// Place the item which player used as an event object to the scene
		case 0x0084:
		{
			if (pScript->rgwOperand[0] <= gpGlobals->g.rgScene[gpGlobals->wNumScene - 1].wEventObjectIndex ||
				pScript->rgwOperand[0] > gpGlobals->g.rgScene[gpGlobals->wNumScene].wEventObjectIndex)
			{
				//
				// The event object is not in the current scene
				//
				wScriptEntry = pScript->rgwOperand[2] - 1;
				g_fScriptSuccess = FALSE;
				break;
			}

			x = PAL_X(gpGlobals->viewport) + PAL_X(gpGlobals->partyoffset);
			y = PAL_Y(gpGlobals->viewport) + PAL_Y(gpGlobals->partyoffset);

			x +=
				((gpGlobals->wPartyDirection == kDirWest || gpGlobals->wPartyDirection == kDirSouth)
				? -16 : 16);
			y +=
				((gpGlobals->wPartyDirection == kDirWest || gpGlobals->wPartyDirection == kDirNorth)
				? -8 : 8);

			if (PAL_CheckObstacle(PAL_XY(x, y), FALSE, 0))
			{
				wScriptEntry = pScript->rgwOperand[2] - 1;
				g_fScriptSuccess = FALSE;
			}
			else
			{
#ifdef AVOID_SCRIPT_CRASH
				if (pCurrent == NULL)
				{
					break;
				}
#endif
				pCurrent->x = x;
				pCurrent->y = y;
				pCurrent->sState = (SHORT)(pScript->rgwOperand[1]);
			}
			break;
		}

			// Delay for a period
		case 0x0085:
		{
			UTIL_Delay(pScript->rgwOperand[0] * 80);
			break;
		}

			// Jump if the specified item is not equipped
		case 0x0086:
		{
			y = FALSE;
			for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				w = gpGlobals->rgParty[i].wPlayerRole;
				for (x = 0; x < MAX_PLAYER_EQUIPMENTS; x++)
				{
					if (gpGlobals->g.PlayerRoles.rgwEquipment[x][w] == pScript->rgwOperand[0])
					{
						y = TRUE;
						i = 999;
						break;
					}
				}
			}
			if (!y)
			{
				wScriptEntry = pScript->rgwOperand[2] - 1;
			}
			break;
		}

			// Animate the event object
		case 0x0087:
		{
			PAL_NPCWalkOneStep(wCurEventObjectID, 0);
			break;
		}

			// Set the base damage of magic according to amount of money // 根据所剩金钱设置基础伤害值
		case 0x0088:
		{
			i = ((gpGlobals->dwCash > 5000) ? 5000 : gpGlobals->dwCash);
			gpGlobals->dwCash -= i;
			j = gpGlobals->g.rgObject[pScript->rgwOperand[0]].magic.wMagicNumber;
			gpGlobals->g.lprgMagic[j].wBaseDamage = i * 2 / 5;
			break;
		}

			// Set the battle result
		case 0x0089:
		{
			g_Battle.BattleResult = pScript->rgwOperand[0];
			break;
		}

			// Enable Auto-Battle for next battle
		case 0x008A:
		{
			gpGlobals->fAutoBattle = TRUE;
			break;
		}

			// change the current palette
		case 0x008B:
		{
			gpGlobals->wNumPalette = pScript->rgwOperand[0];
			if (!gpGlobals->fNeedToFadeIn)
			{
				PAL_SetPalette(gpGlobals->wNumPalette, FALSE);
			}
			break;
		}

			// Fade from/to color
		case 0x008C:
		{
			PAL_ColorFade(pScript->rgwOperand[1], (BYTE)(pScript->rgwOperand[0]),
				pScript->rgwOperand[2]);
			gpGlobals->fNeedToFadeIn = FALSE;
			break;
		}

			// Increase player's level
		case 0x008D:
		{
			PAL_PlayerLevelUp(wEventObjectID, pScript->rgwOperand[0]);
			break;
		}

			// Halve the cash amount
		case 0x008F:
		{
			gpGlobals->dwCash /= 2;
			break;
		}

			// Set the object script
		case 0x0090:
		{
			gpGlobals->g.rgObject[pScript->rgwOperand[0]].rgwData[2 + pScript->rgwOperand[2]] =
				pScript->rgwOperand[1];
			break;
		}

			// Jump if the enemy is not alone
		case 0x0091:
		{
			if (gpGlobals->fInBattle)
			{
				for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
				{
					if (i != wEventObjectID &&
						g_Battle.rgEnemy[i].wObjectID == g_Battle.rgEnemy[wEventObjectID].wObjectID)
					{
						wScriptEntry = pScript->rgwOperand[0] - 1;
						break;
					}
				}
			}
			break;
		}

			// Show a magic-casting animation for a player in battle
		case 0x0092:
		{
			if (gpGlobals->fInBattle)
			{
				if (pScript->rgwOperand[0] != 0)
				{
					PAL_BattleShowPlayerPreMagicAnim(pScript->rgwOperand[0] - 1, FALSE);
					g_Battle.rgPlayer[pScript->rgwOperand[0] - 1].wCurrentFrame = 6;
				}

				for (i = 0; i < 5; i++)
				{
					for (j = 0; j <= gpGlobals->wMaxPartyMemberIndex; j++)
					{
						g_Battle.rgPlayer[j].iColorShift = i * 2;
					}
					PAL_BattleDelay(1, 0, TRUE);
				}
				PAL_BattleBackupScene();
				PAL_BattleUpdateFighters();
				PAL_BattleMakeScene();
				PAL_BattleFadeScene();
			}
			break;
		}

			// Fade the screen. Update scene in the process.
		case 0x0093:
		{
			PAL_SceneFade(gpGlobals->wNumPalette, gpGlobals->fNightPalette,
				(SHORT)(pScript->rgwOperand[0]));
			gpGlobals->fNeedToFadeIn = ((SHORT)(pScript->rgwOperand[0]) < 0);
			break;
		}

			// Jump if the state of event object is the specified one
		case 0x0094:
		{
#ifdef AVOID_SCRIPT_CRASH
			if (pCurrent == NULL)
			{
				break;
			}
#endif
			if (pCurrent->sState == (SHORT)(pScript->rgwOperand[1]))
			{
				wScriptEntry = pScript->rgwOperand[2] - 1;
			}
			break;
		}

			// Jump if the current scene is the specified one
		case 0x0095:
		{
			if (gpGlobals->wNumScene == pScript->rgwOperand[0])
			{
				wScriptEntry = pScript->rgwOperand[1] - 1;
			}
			break;
		}

			// Show the ending animation
		case 0x0096:
		{
#ifndef PAL_WIN95
			PAL_EndingAnimation();
#endif
			break;
		}

			// Ride the event object to the specified position, at a higher speed
		case 0x0097:
		{
			PAL_PartyRideEventObject(wEventObjectID, pScript->rgwOperand[0], pScript->rgwOperand[1],
				pScript->rgwOperand[2], 8);
			break;
		}

			// Set follower of the party
		case 0x0098:
		{
			if (pScript->rgwOperand[0] > 0 && gpGlobals->wMaxPartyMemberIndex < 4)
			{
				gpGlobals->nFollower = 1;
				gpGlobals->rgParty[gpGlobals->wMaxPartyMemberIndex + 1].wPlayerRole = pScript->rgwOperand[0];

				PAL_SetLoadFlags(kLoadPlayerSprite);
				PAL_LoadResources();

				//
				// Update the position and gesture for the follower
				//
				gpGlobals->rgParty[gpGlobals->wMaxPartyMemberIndex + 1].x =
					gpGlobals->rgTrail[gpGlobals->wMaxPartyMemberIndex].x - PAL_X(gpGlobals->viewport);
				gpGlobals->rgParty[gpGlobals->wMaxPartyMemberIndex + 1].y =
					gpGlobals->rgTrail[gpGlobals->wMaxPartyMemberIndex].y - PAL_Y(gpGlobals->viewport);
				gpGlobals->rgParty[gpGlobals->wMaxPartyMemberIndex + 1].wFrame =
					gpGlobals->rgTrail[gpGlobals->wMaxPartyMemberIndex].wDirection * 3;
			}
			else
			{
				gpGlobals->nFollower = 0;
			}
			break;
		}

			// Change the map for the specified scene
		case 0x0099:
		{
			if (pScript->rgwOperand[0] == 0xFFFF)
			{
				gpGlobals->g.rgScene[gpGlobals->wNumScene - 1].wMapNum = pScript->rgwOperand[1];
				PAL_SetLoadFlags(kLoadScene);
				PAL_LoadResources();
			}
			else
			{
				gpGlobals->g.rgScene[pScript->rgwOperand[0] - 1].wMapNum = pScript->rgwOperand[1];
			}
			break;
		}

			// Set the state for multiple event objects
		case 0x009A:
		{
			for (i = pScript->rgwOperand[0]; i <= pScript->rgwOperand[1]; i++)
			{
				gpGlobals->g.lprgEventObject[i - 1].sState = pScript->rgwOperand[2];
			}
			break;
		}

			// Fade to the current scene // FIXME: This is obviously wrong
		case 0x009B:
		{
			VIDEO_BackupScreen();
			PAL_MakeScene();
			VIDEO_FadeScreen(2);
			break;
		}

			// Enemy duplicate itself // 怪物复制自身
		case 0x009C:
		{
			w = 0;

			for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
			{
				if (g_Battle.rgEnemy[i].wObjectID != 0)
				{
					w++;
				}
			}

			if (w != 1)
			{
				//
				// Duplication is only possible when only 1 enemy left
				//
				if (pScript->rgwOperand[1] != 0)
				{
					wScriptEntry = pScript->rgwOperand[1] - 1;
				}
				break;
			}

			w = pScript->rgwOperand[0];
			if (w == 0)
			{
				w = 1;
			}

			for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
			{
				if (w > 0 && g_Battle.rgEnemy[i].wObjectID == 0)
				{
					w--;

					memset(&(g_Battle.rgEnemy[i]), 0, sizeof(BATTLEENEMY));

					g_Battle.rgEnemy[i].wObjectID = g_Battle.rgEnemy[wEventObjectID].wObjectID;
					g_Battle.rgEnemy[i].e = g_Battle.rgEnemy[wEventObjectID].e;
					g_Battle.rgEnemy[i].dwMaxHealth = g_Battle.rgEnemy[wEventObjectID].dwMaxHealth;
					g_Battle.rgEnemy[i].dwActualHealth = g_Battle.rgEnemy[wEventObjectID].dwActualHealth;

					g_Battle.rgEnemy[i].wScriptOnTurnStart = g_Battle.rgEnemy[wEventObjectID].wScriptOnTurnStart;
					g_Battle.rgEnemy[i].wScriptOnBattleEnd = g_Battle.rgEnemy[wEventObjectID].wScriptOnBattleEnd;
					g_Battle.rgEnemy[i].wScriptOnReady = g_Battle.rgEnemy[wEventObjectID].wScriptOnReady;

					g_Battle.rgEnemy[i].state = kFighterWait;
					g_Battle.rgEnemy[i].flTimeMeter = 50;
					g_Battle.rgEnemy[i].iColorShift = 0;
				}
			}

			PAL_LoadBattleSprites();

			for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
			{
				if (g_Battle.rgEnemy[i].wObjectID == 0)
				{
					continue;
				}
				g_Battle.rgEnemy[i].pos = g_Battle.rgEnemy[wEventObjectID].pos;
			}

			for (i = 0; i < 10; i++)
			{
				for (j = 0; j <= g_Battle.wMaxEnemyIndex; j++)
				{
					x = (PAL_X(g_Battle.rgEnemy[j].pos) + PAL_X(g_Battle.rgEnemy[j].posOriginal)) / 2;
					y = (PAL_Y(g_Battle.rgEnemy[j].pos) + PAL_Y(g_Battle.rgEnemy[j].posOriginal)) / 2;

					g_Battle.rgEnemy[j].pos = PAL_XY(x, y);
				}

				PAL_BattleDelay(1, 0, TRUE);
			}

			PAL_BattleUpdateFighters();
			PAL_BattleDelay(1, 0, TRUE);
			break;
		}

			// Enemy summons another monster
		case 0x009E:
		{
			WORD wEmptyEnemySiteNum = 0;
			WORD wNewEnemyID = pScript->rgwOperand[0];//召唤怪物的id
			WORD wNewEnemyNum = max(pScript->rgwOperand[1], 1);//召唤怪物的数量

			if (wNewEnemyID == 0 || wNewEnemyID == 0xFFFF)
			{
				wNewEnemyID = g_Battle.rgEnemy[wEventObjectID].wObjectID;
			}

			for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
			{
				if (g_Battle.rgEnemy[i].wObjectID == 0)
				{
					wEmptyEnemySiteNum++;
				}
			}

			if (wEmptyEnemySiteNum < wNewEnemyNum || !PAL_New_IfEnemyCanMove(wEventObjectID))
			{
				if (pScript->rgwOperand[2] != 0)
				{	//召唤失败，则跳转
					wScriptEntry = pScript->rgwOperand[2] - 1;
				}
			}
			else
			{
				for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
				{
					if (g_Battle.rgEnemy[i].wObjectID == 0)
					{
						memset(&(g_Battle.rgEnemy[i]), 0, sizeof(BATTLEENEMY));

						g_Battle.rgEnemy[i].wObjectID = wNewEnemyID;
						g_Battle.rgEnemy[i].e = gpGlobals->g.lprgEnemy[gpGlobals->g.rgObject[wNewEnemyID].enemy.wEnemyID];
						g_Battle.rgEnemy[i].dwMaxHealth = g_Battle.rgEnemy[i].e.wHealth;
						g_Battle.rgEnemy[i].dwActualHealth = g_Battle.rgEnemy[i].e.wHealth;
#ifdef STRENGTHEN_ENEMY
						g_Battle.rgEnemy[i] = PAL_New_StrengthenEnemy(g_Battle.rgEnemy[i]);
#endif
						g_Battle.rgEnemy[i].state = kFighterWait;
						g_Battle.rgEnemy[i].wScriptOnTurnStart = gpGlobals->g.rgObject[wNewEnemyID].enemy.wScriptOnTurnStart;
						g_Battle.rgEnemy[i].wScriptOnBattleEnd = gpGlobals->g.rgObject[wNewEnemyID].enemy.wScriptOnBattleEnd;
						g_Battle.rgEnemy[i].wScriptOnReady = gpGlobals->g.rgObject[wNewEnemyID].enemy.wScriptOnReady;
						g_Battle.rgEnemy[i].flTimeMeter = 50;
						g_Battle.rgEnemy[i].iColorShift = 8;

						wNewEnemyNum--;
						if (wNewEnemyNum <= 0)
						{
							break;
						}
					}
				}

				PAL_BattleDelay(2, 0, TRUE);

				PAL_BattleBackupScene();
				PAL_LoadBattleSprites();
				PAL_BattleMakeScene();
				SOUND_Play(212);
				PAL_BattleFadeScene();

				for (i = 0; i <= g_Battle.wMaxEnemyIndex; i++)
				{
					g_Battle.rgEnemy[i].iColorShift = 0;
				}

				PAL_BattleBackupScene();
				PAL_BattleMakeScene();
				PAL_BattleFadeScene();
			}
			break;
		}

			// Enemy transforms into something else
		case 0x009F:
		{
			WORD wEnemyIndex = wEventObjectID;
			WORD wNewEnemyID = pScript->rgwOperand[0];

			if (PAL_New_IfEnemyCanMove(wEnemyIndex))
			{
				WORD wPrveLevel = g_Battle.rgEnemy[wEnemyIndex].e.wLevel;
				WORD wPrevCollectValue = g_Battle.rgEnemy[wEnemyIndex].e.wCollectValue;
				WORD wPrevExp = g_Battle.rgEnemy[wEnemyIndex].e.wExp;

				g_Battle.rgEnemy[wEnemyIndex].wObjectID = wNewEnemyID;
				g_Battle.rgEnemy[wEnemyIndex].e =
					gpGlobals->g.lprgEnemy[gpGlobals->g.rgObject[wNewEnemyID].enemy.wEnemyID];

				g_Battle.rgEnemy[wEnemyIndex].e.wLevel = max(g_Battle.rgEnemy[wEnemyIndex].e.wLevel, wPrveLevel);
				g_Battle.rgEnemy[wEnemyIndex].e.wCollectValue = max(g_Battle.rgEnemy[wEnemyIndex].e.wCollectValue, wPrevCollectValue);
				g_Battle.rgEnemy[wEnemyIndex].e.wExp = max(g_Battle.rgEnemy[wEnemyIndex].e.wExp, wPrevExp);

				g_Battle.rgEnemy[wEnemyIndex].wCurrentFrame = 0;
				g_Battle.rgEnemy[wEnemyIndex].wScriptOnBattleEnd =
					gpGlobals->g.rgObject[wNewEnemyID].enemy.wScriptOnBattleEnd; //修正怪物变身后的战后脚本

				for (i = 0; i < MAX_ENEMIES_IN_TEAM; i++)
				{
					g_Battle.rgEnemy[wEnemyIndex].iColorShift = i;
					PAL_BattleDelay(1, 0, FALSE);
				}

				g_Battle.rgEnemy[wEnemyIndex].iColorShift = 0;

				PAL_BattleBackupScene();
				PAL_LoadBattleSprites();
				PAL_BattleMakeScene();
				PAL_BattleFadeScene();
			}
			break;
		}

			// Quit game
		case 0x00A0:
		{
#ifdef PAL_WIN95
			PAL_EndingScreen();
#endif
			PAL_AdditionalCredits();
			PAL_Shutdown();
			exit(0);
			break;
		}

			//Set the positions of all party members to the same as the first one
		case 0x00A1:
		{

			for (i = 0; i < MAX_PLAYABLE_PLAYER_ROLES; i++)
			{
				gpGlobals->rgTrail[i].wDirection = gpGlobals->wPartyDirection;
				gpGlobals->rgTrail[i].x = gpGlobals->rgParty[0].x + PAL_X(gpGlobals->viewport);
				gpGlobals->rgTrail[i].y = gpGlobals->rgParty[0].y + PAL_Y(gpGlobals->viewport);
			}
			for (i = 1; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				gpGlobals->rgParty[i].x = gpGlobals->rgParty[0].x;
				gpGlobals->rgParty[i].y = gpGlobals->rgParty[0].y - 1;
			}
			PAL_UpdatePartyGestures(FALSE);
			break;
		}

			// Jump to one of the following instructions randomly
		case 0x00A2:
		{
			wScriptEntry += RandomLong(0, pScript->rgwOperand[0] - 1);
			break;
		}

			// Play CD music. Use the RIX music for fallback.
		case 0x00A3:
		{
			if (!SOUND_PlayCDA(pScript->rgwOperand[0]))
			{
				PAL_PlayMUS(pScript->rgwOperand[1], TRUE, 0);
			}
			break;
		}

			// Scroll FBP to the screen
		case 0x00A4:
		{
#ifndef PAL_WIN95
			if (pScript->rgwOperand[0] == 68)
			{
				//
				// HACKHACK: to make the ending picture show correctly
				//
				PAL_ShowFBP(69, 0);
				PAL_ScrollFBP(pScript->rgwOperand[0], pScript->rgwOperand[2], TRUE);
			}
			else
			{
				PAL_ScrollFBP(pScript->rgwOperand[0], pScript->rgwOperand[2], pScript->rgwOperand[1]);
			}
#endif
			break;
		}

			// Show FBP picture with sprite effects
		case 0x00A5:
		{
#ifndef PAL_WIN95
			if (pScript->rgwOperand[1] != 0xFFFF)
			{
				PAL_EndingSetEffectSprite(pScript->rgwOperand[1]);
			}
			PAL_ShowFBP(pScript->rgwOperand[0], pScript->rgwOperand[2]);
#endif
			break;
		}

			// backup screen
		case 0x00A6:
		{
			VIDEO_BackupScreen();
			break;
		}

			//接下来是新增加的命令

			// 结尾1：消灭水魔兽
		case 0x00B0:
		{
#ifdef PAL_HAS_AVI
			if (access(va("%s/avi/4.AVI", PAL_PREFIX), 0) == 0 && access(va("%s/avi/5.AVI", PAL_PREFIX), 0) == 0)
			{
				PAL_PlayAVI(va("%s/avi/4.AVI", PAL_PREFIX), FALSE);
				PAL_PlayAVI(va("%s/avi/5.AVI", PAL_PREFIX), FALSE);
			}
			else
			{
				PAL_NEW_EndingScreen1();
			}
#else
			PAL_NEW_EndingScreen1();
#endif		
			break;
		}

			// 结尾2：全剧终&主角人物诗&工作人员介绍
		case 0x00B1:
		{
#ifdef PAL_HAS_AVI
			if (access(va("%s/avi/6.AVI", PAL_PREFIX), 0) == 0)
			{
				PAL_PlayAVI(va("%s/avi/6.AVI", PAL_PREFIX), TRUE);
			}
			else
			{
				PAL_NEW_EndingScreen2();
			}
#else
			PAL_NEW_EndingScreen2();
#endif		
			break;
		}

			// 穿越回最初场景
		case 0x00B2:
		{
#ifdef FINISH_GAME_MORE_ONE_TIME

			PAL_InitGameData(-1);
			return 0; // don't go further
#else
			break;
#endif

		}

			// 增加队中人员（3人以上），参考命令0075
		case 0x0100:
		{
			for (i = 0; i < 3; i++)
			{
				if (pScript->rgwOperand[i] != 0)
				{
					gpGlobals->wMaxPartyMemberIndex++;

					if (gpGlobals->wMaxPartyMemberIndex > 5)
					{
						break;
					}

					gpGlobals->rgParty[gpGlobals->wMaxPartyMemberIndex].wPlayerRole =
						pScript->rgwOperand[i] - 1;

					g_Battle.rgPlayer[gpGlobals->wMaxPartyMemberIndex].action.ActionType =
						kBattleActionAttack;
				}
			}
			//
			// Reload the player sprites
			//
			PAL_SetLoadFlags(kLoadPlayerSprite);
			PAL_LoadResources();
			PAL_UpdateEquipments();
			break;
		}

			// 新命令：移除我方角色的额外装备效果
		case 0x0101:
		{
			PAL_RemoveEquipmentEffect(wEventObjectID, kBodyPartExtra);

			PAL_BattleBackupScene();
			PAL_LoadBattleSprites();
			PAL_BattleMakeScene();
			PAL_BattleFadeScene();
			break;
		}

			// 新命令：参考命令0075，设置队伍成员（适用于3人以上，顺序固定）领队人物延续
		case 0x0102:
		{
			WORD wLeaderPlayerRole = gpGlobals->rgParty[0].wPlayerRole;
			gpGlobals->wMaxPartyMemberIndex = 0;
			for (i = 0; i < MAX_PLAYER_ROLES; i++)
			{
				if (pScript->rgwOperand[0] & (1 << i) && gpGlobals->wMaxPartyMemberIndex < MAX_PLAYERS_IN_PARTY)
				{
					gpGlobals->rgParty[gpGlobals->wMaxPartyMemberIndex].wPlayerRole = i;
					g_Battle.rgPlayer[gpGlobals->wMaxPartyMemberIndex].action.ActionType = kBattleActionAttack;
					gpGlobals->wMaxPartyMemberIndex++;
				}
			}
			if (gpGlobals->wMaxPartyMemberIndex != 0)
			{
				gpGlobals->wMaxPartyMemberIndex--;
			}

			int index = PAL_New_GetPlayerIndex(wLeaderPlayerRole);
			if (index != -1 && index != 0)
			{
				WORD temp = gpGlobals->rgParty[0].wPlayerRole;
				gpGlobals->rgParty[0].wPlayerRole = gpGlobals->rgParty[index].wPlayerRole;
				gpGlobals->rgParty[index].wPlayerRole = temp;
			}
			//
			// Reload the player sprites
			//
			PAL_SetLoadFlags(kLoadPlayerSprite);
			PAL_LoadResources();
			PAL_UpdateEquipments();
			break;
		}

			// 如果队伍中没有指定的角色则跳转（参考0079）
		case 0x0103:
		{
			BOOL jumpFlag = TRUE;
			for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
			{
				if (gpGlobals->rgParty[i].wPlayerRole == pScript->rgwOperand[0] - 1)
				{
					jumpFlag = FALSE;
					break;
				}
			}
			if (jumpFlag)
			{
				wScriptEntry = pScript->rgwOperand[1] - 1;
			}
			break;
		}

			//到这里说明出错了
		default:
		{
			TerminateOnError("SCRIPT: Invalid Instruction at %4x: (%4x - %4x, %4x, %4x)",
				wScriptEntry, pScript->wOperation, pScript->rgwOperand[0],
				pScript->rgwOperand[1], pScript->rgwOperand[2]);
			break;
		}
	}

	return wScriptEntry + 1;
}

WORD
PAL_RunTriggerScript(
WORD           wScriptEntry,
WORD           wEventObjectID
)
/*++
  Purpose:

  Runs a trigger script.

  Parameters:

  [IN]  wScriptEntry - The script entry to execute.

  [IN]  wEventObjectID - The event object ID which invoked the script.

  Return value:

  The entry point of the script.

  --*/
{
	static WORD       wLastEventObject = 0;

	WORD              wNextScriptEntry;
	BOOL              fEnded;
	LPSCRIPTENTRY     pScript;
	LPEVENTOBJECT     pEvtObj = NULL;
	int               i;
	int               randomNum;
	extern BOOL       g_fUpdatedInBattle; // HACKHACK

	wNextScriptEntry = wScriptEntry;
	fEnded = FALSE;
	g_fUpdatedInBattle = FALSE;

	if (wEventObjectID == 0xFFFF)
	{
		wEventObjectID = wLastEventObject;
	}

	wLastEventObject = wEventObjectID;

	if (wEventObjectID != 0)
	{
		pEvtObj = &(gpGlobals->g.lprgEventObject[wEventObjectID - 1]);
	}

	g_fScriptSuccess = TRUE;

	//
	// Set the default dialog speed.
	//
	PAL_DialogSetDelayTime(3);

	while (wScriptEntry != 0 && !fEnded)
	{
		pScript = &(gpGlobals->g.lprgScriptEntry[wScriptEntry]);

#ifdef ENABLE_LOG
		UTIL_WriteLog(LOG_DEBUG, "[SCRIPT] %.4x: %.4x %.4x %.4x %.4x\n", wScriptEntry,
			pScript->wOperation, pScript->rgwOperand[0], pScript->rgwOperand[1],
			pScript->rgwOperand[2], pScript->rgwOperand[3]);
#endif

		switch (pScript->wOperation)
		{
			case 0x0000:
				//
				// Stop running
				//
				fEnded = TRUE;
				break;

			case 0x0001:
				//
				// Stop running and replace the entry with the next line
				//
				fEnded = TRUE;
				wNextScriptEntry = wScriptEntry + 1;
				break;

			case 0x0002:
				//
				// Stop running and replace the entry with the specified one
				//
				if (pScript->rgwOperand[1] == 0 ||
					++(pEvtObj->nScriptIdleFrame) < pScript->rgwOperand[1])
				{
					fEnded = TRUE;
					wNextScriptEntry = pScript->rgwOperand[0];
				}
				else
				{
					//
					// failed
					//
					pEvtObj->nScriptIdleFrame = 0;
					wScriptEntry++;
				}
				break;

			case 0x0003:
				//
				// unconditional jump
				//
				if (pScript->rgwOperand[1] == 0 ||
					++(pEvtObj->nScriptIdleFrame) < pScript->rgwOperand[1])
				{
					wScriptEntry = pScript->rgwOperand[0];
				}
				else
				{
					//
					// failed
					//
					pEvtObj->nScriptIdleFrame = 0;
					wScriptEntry++;
				}
				break;

			case 0x0004:
				//
				// Call script
				//
				PAL_RunTriggerScript(pScript->rgwOperand[0],
					((pScript->rgwOperand[1] == 0) ? wEventObjectID : pScript->rgwOperand[1]));
				wScriptEntry++;
				break;

			case 0x0005:
				//
				// Redraw screen
				//
				PAL_ClearDialog(TRUE);

				if (PAL_DialogIsPlayingRNG())
				{
					VIDEO_RestoreScreen();
				}
				else if (gpGlobals->fInBattle)
				{
					PAL_BattleMakeScene();
					SDL_BlitSurface(g_Battle.lpSceneBuf, NULL, gpScreen, NULL);
					VIDEO_UpdateScreen(NULL);
				}
				else
				{
					if (pScript->rgwOperand[2])
					{
						PAL_UpdatePartyGestures(FALSE);
					}

					PAL_MakeScene();

					VIDEO_UpdateScreen(NULL);
					UTIL_Delay((pScript->rgwOperand[1] == 0) ? 60 : (pScript->rgwOperand[1] * 60));
				}

				wScriptEntry++;
				break;

			case 0x0006:
				//
				// Jump to the specified address by the specified rate
				//
				/*
				06 参数1 参数2		  判断、跳转指令
				参数1：整数
				参数2：脚本地址（可以省略）		  指向sss.mkf的第5个子文件		  指向的地址 = 参数2 * 8
				作用		  脚本执行中，遇到06，则系统产生一个0到100的随机数，若该随机数不大于参数1
				，则继续执行06之后的内容，否则：
				若参数2被省略，则中断脚本执行，并将06指令所在的脚本地址写入脚本调用处
				若参数2存在，则跳转参数2指向的脚本地址，继续执行该地址之后的内容
				*/
				randomNum = RandomLong(1, 100);
				if (randomNum > pScript->rgwOperand[0])
				{
					wScriptEntry = pScript->rgwOperand[1];
					continue;
				}
				else
				{
					wScriptEntry++;
				}
				break;

			case 0x0007:
				//
				// Start battle
				//
				i = PAL_StartBattle(pScript->rgwOperand[0], !pScript->rgwOperand[2]);

				if (i == kBattleResultLost && pScript->rgwOperand[1] != 0)
				{
					wScriptEntry = pScript->rgwOperand[1];
				}
				else if (i == kBattleResultFleed && pScript->rgwOperand[2] != 0)
				{
					wScriptEntry = pScript->rgwOperand[2];
				}
				else
				{
					wScriptEntry++;
				}
				gpGlobals->fAutoBattle = FALSE;
				break;

			case 0x0008:
				//
				// Replace the entry with the next instruction
				//
				wScriptEntry++;
				wNextScriptEntry = wScriptEntry;
				break;

				// wait for the specified number of frames
			case 0x0009:
			{
				DWORD        time;

				PAL_ClearDialog(TRUE);

				time = SDL_GetTicks() + FRAME_TIME;

				for (i = 0; i < (pScript->rgwOperand[0] ? pScript->rgwOperand[0] : 1); i++)
				{
					PAL_ProcessEvent();
					while (SDL_GetTicks() < time)
					{
						PAL_ProcessEvent();
						SDL_Delay(1);
					}

					time = SDL_GetTicks() + FRAME_TIME;

					if (pScript->rgwOperand[2])
					{
						PAL_UpdatePartyGestures(FALSE);
					}

					PAL_GameUpdate(pScript->rgwOperand[1] ? TRUE : FALSE);
					PAL_MakeScene();
					VIDEO_UpdateScreen(NULL);
				}

				wScriptEntry++;
				break;
			}

				// Goto the specified address if player selected no
			case 0x000A:
			{
				PAL_ClearDialog(FALSE);

				if (!PAL_ConfirmMenu())
				{
					wScriptEntry = pScript->rgwOperand[0];
				}
				else
				{
					wScriptEntry++;
				}
				break;
			}

				// Show dialog in the middle part of the screen
			case 0x003B:
			{
				PAL_ClearDialog(TRUE);
				PAL_StartDialog(kDialogCenter, (BYTE)pScript->rgwOperand[0], 0,
					pScript->rgwOperand[2] ? TRUE : FALSE);
				wScriptEntry++;
				break;
			}

			case 0x003C:
				//
				// Show dialog in the upper part of the screen
				//
				PAL_ClearDialog(TRUE);
				PAL_StartDialog(kDialogUpper, (BYTE)pScript->rgwOperand[1],
					pScript->rgwOperand[0], pScript->rgwOperand[2] ? TRUE : FALSE);
				wScriptEntry++;
				break;

			case 0x003D:
				//
				// Show dialog in the lower part of the screen
				//
				PAL_ClearDialog(TRUE);
				PAL_StartDialog(kDialogLower, (BYTE)pScript->rgwOperand[1],
					pScript->rgwOperand[0], pScript->rgwOperand[2] ? TRUE : FALSE);
				wScriptEntry++;
				break;

			case 0x003E:
				//
				// Show text in a window at the center of the screen
				//
				PAL_ClearDialog(TRUE);
				PAL_StartDialog(kDialogCenterWindow, (BYTE)pScript->rgwOperand[0], 0, FALSE);
				wScriptEntry++;
				break;

			case 0x008E:
				//
				// Restore the screen
				//
				PAL_ClearDialog(TRUE);
				VIDEO_RestoreScreen();
				VIDEO_UpdateScreen(NULL);
				wScriptEntry++;
				break;

			case 0xFFFF:
				//
				// Print dialog text
				//
				PAL_ShowDialogText(PAL_GetMsg(pScript->rgwOperand[0]));
				wScriptEntry++;
				break;

				//接下来是新增加的命令
			case 0x00FF:
			{
				// 新命令：随机跳转到之后的n条后的地址中的一条，
				// 执行这一条后，从这条命令开始处的n条后继续执行
				// n为 pScript->rgwOperand[0]的值
				WORD wNewScriptEntry = wScriptEntry + RandomLong(1, pScript->rgwOperand[0]);
				WORD wJumpScriptEntry = PAL_InterpretInstruction(wNewScriptEntry, wEventObjectID);

				/*
				if (wJumpScriptEntry < pScript->rgwOperand[0] + 1 && wJumpScriptEntry > wScriptEntry)
				{//若新指令没有跳转或跳转在该命令则旧条命令开始处的n条后继续执行

				wScriptEntry += pScript->rgwOperand[0] + 1;
				}
				else
				{//若该条指令有跳转，则从跳转处开始执行
				//若新指令有跳转但跳转地址为新指令地址的下一条，无法跳转
				//会进入上面的if处
				wScriptEntry = wJumpScriptEntry;
				}
				*/
				//还是设置成不允许中途跳转吧，以免发生奇怪的事情
				wScriptEntry += pScript->rgwOperand[0] + 1;

				break;
			}

			default:
				PAL_ClearDialog(TRUE);
				wScriptEntry = PAL_InterpretInstruction(wScriptEntry, wEventObjectID);
				break;
		}
	}

	PAL_EndDialog();
	g_iCurEquipPart = -1;

	return wNextScriptEntry;
}

WORD
PAL_RunAutoScript(
WORD           wScriptEntry,
WORD           wEventObjectID
)
/*++
  Purpose:

  Runs the autoscript of the specified event object.

  Parameters:

  [IN]  wScriptEntry - The script entry to execute.

  [IN]  wEventObjectID - The event object ID which invoked the script.

  Return value:

  The address of the next script instruction to execute.

  --*/
{
	LPSCRIPTENTRY          pScript;
	LPEVENTOBJECT          pEvtObj;
#ifdef PAL_WIN95
	int                    iDescLine = 0;
#endif

begin:
	pScript = &(gpGlobals->g.lprgScriptEntry[wScriptEntry]);
	pEvtObj = &(gpGlobals->g.lprgEventObject[wEventObjectID - 1]);

	//
	// For autoscript, we should interpret one instruction per frame (except
	// jumping) and save the address of next instruction.
	//
	switch (pScript->wOperation)
	{
		case 0x0000:
			//
			// Stop running
			//
			break;

		case 0x0001:
			//
			// Stop running and replace the entry with the next line
			//
			wScriptEntry++;
			break;

		case 0x0002:
			//
			// Stop running and replace the entry with the specified one
			//
			if (pScript->rgwOperand[1] == 0 ||
				++(pEvtObj->wScriptIdleFrameCountAuto) < pScript->rgwOperand[1])
			{
				wScriptEntry = pScript->rgwOperand[0];
			}
			else
			{
				pEvtObj->wScriptIdleFrameCountAuto = 0;
				wScriptEntry++;
			}
			break;

		case 0x0003:
			//
			// unconditional jump
			//
			if (pScript->rgwOperand[1] == 0 ||
				++(pEvtObj->wScriptIdleFrameCountAuto) < pScript->rgwOperand[1])
			{
				wScriptEntry = pScript->rgwOperand[0];
				goto begin;
			}
			else
			{
				pEvtObj->wScriptIdleFrameCountAuto = 0;
				wScriptEntry++;
			}
			break;

		case 0x0004:
			//
			// Call subroutine
			//
			PAL_RunTriggerScript(pScript->rgwOperand[0],
				pScript->rgwOperand[1] ? pScript->rgwOperand[1] : wEventObjectID);
			wScriptEntry++;
			break;

		case 0x0006:
			//
			// jump to the specified address by the specified rate
			//
			if (RandomLong(1, 100) >= pScript->rgwOperand[0] && pScript->rgwOperand[1] != 0)
			{
				wScriptEntry = pScript->rgwOperand[1];
				goto begin;
			}
			else
			{
				wScriptEntry++;
			}
			break;

		case 0x0009:
			//
			// Wait for a certain number of frames
			//
			if (++(pEvtObj->wScriptIdleFrameCountAuto) >= pScript->rgwOperand[0])
			{
				//
				// waiting ended; go further
				//
				pEvtObj->wScriptIdleFrameCountAuto = 0;
				wScriptEntry++;
			}
			break;

		case 0xFFFF:
#ifdef PAL_WIN95
			iDescLine = (wEventObjectID & ~(1 << 15));
			if (wEventObjectID & (1 << 15))
			{
				PAL_DrawText(PAL_GetMsg(pScript->rgwOperand[0]), PAL_XY(75, iDescLine * 16 + 150), DESCTEXT_COLOR, TRUE, FALSE);
			}
			else
			{
				PAL_DrawText(PAL_GetMsg(pScript->rgwOperand[0]), PAL_XY(100, iDescLine * 16 + 3), DESCTEXT_COLOR, TRUE, FALSE);
			}
			iDescLine++;
#endif
			wScriptEntry++;
			break;

#ifdef PAL_WIN95
		case 0x00A7:
			wScriptEntry++;
			break;
#endif

		default:
			//
			// Other operations
			//
			wScriptEntry = PAL_InterpretInstruction(wScriptEntry, wEventObjectID);
			break;
	}

	return wScriptEntry;
}


