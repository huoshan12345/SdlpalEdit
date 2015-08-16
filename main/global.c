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

//复活后不解除的毒的ID，0x0233是寿葫芦毒，0x0245是忘剑五诀 
const WORD ExcludePoisonIDAfterRevive[] = {0x0233, 0x0245};

LPGLOBALVARS gpGlobals = NULL;

#define DO_BYTESWAP(buf, size)                                   \
for (i = 0; i < (size) / 2; i++)                              \
{                                                             \
	((LPWORD)(buf))[i] = SWAP16(((LPWORD)(buf))[i]);           \
}

#define LOAD_DATA(buf, size, chunknum, fp)                       \
{                                                             \
	PAL_MKFReadChunk((LPBYTE)(buf), (size), (chunknum), (fp)); \
	DO_BYTESWAP(buf, size);                                    \
}

INT PAL_InitGlobals(VOID)
/*++
  Purpose:    Initialize global data.
  Parameters:    None.
  Return value:    0 = success, -1 = error.
  --*/
{
	if (gpGlobals == NULL)
	{
		gpGlobals = (LPGLOBALVARS)calloc(1, sizeof(GLOBALVARS));
		if (gpGlobals == NULL)
		{
			return -1;
		}
	}

	//
	// Open files
	//
	gpGlobals->f.fpFBP = UTIL_OpenRequiredFile("fbp.mkf");
	gpGlobals->f.fpMGO = UTIL_OpenRequiredFile("mgo.mkf");
	gpGlobals->f.fpBALL = UTIL_OpenRequiredFile("ball.mkf");
	gpGlobals->f.fpDATA = UTIL_OpenRequiredFile("data.mkf");
	gpGlobals->f.fpF = UTIL_OpenRequiredFile("f.mkf");
	gpGlobals->f.fpFIRE = UTIL_OpenRequiredFile("fire.mkf");
	gpGlobals->f.fpRGM = UTIL_OpenRequiredFile("rgm.mkf");
	gpGlobals->f.fpSSS = UTIL_OpenRequiredFile("sss.mkf");
#ifndef PAL_WIN95
	gpGlobals->lpObjectDesc = PAL_LoadObjectDesc(va("%s%s", PAL_PREFIX, "desc.dat"));
#endif
	gpGlobals->bCurrentSaveSlot = 1;

	return 0;
}

VOID PAL_FreeGlobals(VOID)
/*++
  Purpose:    Free global data.
  Parameters:    None.
  Return value:    None.
  --*/
{
	if (gpGlobals != NULL)
	{
		//
		// Close all opened files
		//
		UTIL_CloseFile(gpGlobals->f.fpFBP);
		UTIL_CloseFile(gpGlobals->f.fpMGO);
		UTIL_CloseFile(gpGlobals->f.fpBALL);
		UTIL_CloseFile(gpGlobals->f.fpDATA);
		UTIL_CloseFile(gpGlobals->f.fpF);
		UTIL_CloseFile(gpGlobals->f.fpFIRE);
		UTIL_CloseFile(gpGlobals->f.fpRGM);
		UTIL_CloseFile(gpGlobals->f.fpSSS);

		//
		// Free the game data
		//
		free(gpGlobals->g.lprgEventObject);
		free(gpGlobals->g.lprgScriptEntry);
		free(gpGlobals->g.lprgStore);
		free(gpGlobals->g.lprgEnemy);
		free(gpGlobals->g.lprgEnemyTeam);
		free(gpGlobals->g.lprgMagic);
		free(gpGlobals->g.lprgBattleField);
		free(gpGlobals->g.lprgLevelUpMagic);

		//
		// Free the object description data
		//
#ifndef PAL_WIN95
		PAL_FreeObjectDesc(gpGlobals->lpObjectDesc);
#endif
		PAL_New_FreeNewText(gpGlobals->lpObjectNewMsg);
		//
		// Delete the instance
		//
		free(gpGlobals);
	}

	gpGlobals = NULL;
}

static VOID PAL_ReadGlobalGameData(VOID)
/*++
  Purpose:    Read global game data from data files.
  Parameters:    None.
  Return value:    None.
  --*/
{
	const GAMEDATA    *p = &gpGlobals->g;
	unsigned int       i;

	LOAD_DATA(p->lprgScriptEntry, p->nScriptEntry * sizeof(SCRIPTENTRY),
		4, gpGlobals->f.fpSSS);

	LOAD_DATA(p->lprgStore, p->nStore * sizeof(STORE), 0, gpGlobals->f.fpDATA);
	LOAD_DATA(p->lprgEnemy, p->nEnemy * sizeof(ENEMY), 1, gpGlobals->f.fpDATA);
	LOAD_DATA(p->lprgEnemyTeam, p->nEnemyTeam * sizeof(ENEMYTEAM),
		2, gpGlobals->f.fpDATA);
	LOAD_DATA(p->lprgMagic, p->nMagic * sizeof(MAGIC), 4, gpGlobals->f.fpDATA);
	LOAD_DATA(p->lprgBattleField, p->nBattleField * sizeof(BATTLEFIELD),
		5, gpGlobals->f.fpDATA);
	LOAD_DATA(p->lprgLevelUpMagic, p->nLevelUpMagic * sizeof(LEVELUPMAGIC_ALL),
		6, gpGlobals->f.fpDATA);
	LOAD_DATA(p->rgwBattleEffectIndex, sizeof(p->rgwBattleEffectIndex),
		11, gpGlobals->f.fpDATA);
	PAL_MKFReadChunk((LPBYTE)&(p->EnemyPos), sizeof(p->EnemyPos),
		13, gpGlobals->f.fpDATA);
	DO_BYTESWAP(&(p->EnemyPos), sizeof(p->EnemyPos));
	PAL_MKFReadChunk((LPBYTE)(p->rgLevelUpExp), sizeof(p->rgLevelUpExp),
		14, gpGlobals->f.fpDATA);
	DO_BYTESWAP(p->rgLevelUpExp, sizeof(p->rgLevelUpExp));
}

static VOID
PAL_InitGlobalGameData(
	VOID
	)
	/*++
	  Purpose:

	  Initialize global game data.

	  Parameters:

	  None.

	  Return value:

	  None.

	  --*/
{
	int        len;

#define PAL_DOALLOCATE(fp, num, type, lptype, ptr, n)                            \
									{                                                                             \
	len = PAL_MKFGetChunkSize(num, fp);                                        \
	ptr = (lptype)malloc(len);                                                 \
	n = len / sizeof(type);                                                    \
	if (ptr == NULL)                                                           \
									{                                                                          \
	TerminateOnError("PAL_InitGlobalGameData(): Memory allocation error!"); \
									}                                                                          \
									}

	//
	// If the memory has not been allocated, allocate first.
	//
	if (gpGlobals->g.lprgEventObject == NULL)
	{
		PAL_DOALLOCATE(gpGlobals->f.fpSSS, 0, EVENTOBJECT, LPEVENTOBJECT,
			gpGlobals->g.lprgEventObject, gpGlobals->g.nEventObject);

		PAL_DOALLOCATE(gpGlobals->f.fpSSS, 4, SCRIPTENTRY, LPSCRIPTENTRY,
			gpGlobals->g.lprgScriptEntry, gpGlobals->g.nScriptEntry);

		PAL_DOALLOCATE(gpGlobals->f.fpDATA, 0, STORE, LPSTORE,
			gpGlobals->g.lprgStore, gpGlobals->g.nStore);

		PAL_DOALLOCATE(gpGlobals->f.fpDATA, 1, ENEMY, LPENEMY,
			gpGlobals->g.lprgEnemy, gpGlobals->g.nEnemy);

		PAL_DOALLOCATE(gpGlobals->f.fpDATA, 2, ENEMYTEAM, LPENEMYTEAM,
			gpGlobals->g.lprgEnemyTeam, gpGlobals->g.nEnemyTeam);

		PAL_DOALLOCATE(gpGlobals->f.fpDATA, 4, MAGIC, LPMAGIC,
			gpGlobals->g.lprgMagic, gpGlobals->g.nMagic);

		PAL_DOALLOCATE(gpGlobals->f.fpDATA, 5, BATTLEFIELD, LPBATTLEFIELD,
			gpGlobals->g.lprgBattleField, gpGlobals->g.nBattleField);

		PAL_DOALLOCATE(gpGlobals->f.fpDATA, 6, LEVELUPMAGIC_ALL, LPLEVELUPMAGIC_ALL,
			gpGlobals->g.lprgLevelUpMagic, gpGlobals->g.nLevelUpMagic);

		PAL_ReadGlobalGameData();
	}
#undef PAL_DOALLOCATE
}

static VOID
PAL_LoadDefaultGame(
	VOID
	)
	/*++
	  Purpose:

	  Load the default game data.

	  Parameters:

	  None.

	  Return value:

	  None.

	  --*/
{
	const GAMEDATA    *p = &gpGlobals->g;
	UINT32             i;

	//
	// Load the default data from the game data files.
	//
	LOAD_DATA(p->lprgEventObject, p->nEventObject * sizeof(EVENTOBJECT),
		0, gpGlobals->f.fpSSS);
	PAL_MKFReadChunk((LPBYTE)(p->rgScene), sizeof(p->rgScene), 1, gpGlobals->f.fpSSS);
	DO_BYTESWAP(p->rgScene, sizeof(p->rgScene));
	PAL_MKFReadChunk((LPBYTE)(p->rgObject), sizeof(p->rgObject), 2, gpGlobals->f.fpSSS);
	DO_BYTESWAP(p->rgObject, sizeof(p->rgObject));

	PAL_MKFReadChunk((LPBYTE)(&(p->PlayerRoles)), sizeof(PLAYERROLES),
		3, gpGlobals->f.fpDATA);
	DO_BYTESWAP(&(p->PlayerRoles), sizeof(PLAYERROLES));

	//
	// Set some other default data.
	//
	gpGlobals->dwCash = 0;
	gpGlobals->wNumMusic = 0;
	gpGlobals->wNumPalette = 0;
	gpGlobals->wNumScene = 1;
	gpGlobals->wCollectValue = 0;
	gpGlobals->fNightPalette = FALSE;
	gpGlobals->wMaxPartyMemberIndex = 0;
	gpGlobals->viewport = PAL_XY(0, 0);
	gpGlobals->wLayer = 0;
	gpGlobals->wChaseRange = 1;
#ifndef PAL_CLASSIC
	gpGlobals->bBattleSpeed = 2;
#endif

#ifdef FINISH_GAME_MORE_ONE_TIME
	gpGlobals->bFinishGameTime = 0;
#endif

	memset(gpGlobals->rgInventory, 0, sizeof(gpGlobals->rgInventory));
	memset(gpGlobals->rgPoisonStatus, 0, sizeof(gpGlobals->rgPoisonStatus));
	memset(gpGlobals->rgParty, 0, sizeof(gpGlobals->rgParty));
	memset(gpGlobals->rgTrail, 0, sizeof(gpGlobals->rgTrail));
	memset(&(gpGlobals->Exp), 0, sizeof(gpGlobals->Exp));

	for (i = 0; i < MAX_PLAYER_ROLES; i++)
	{
		gpGlobals->Exp.rgPrimaryExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		gpGlobals->Exp.rgHealthExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		gpGlobals->Exp.rgMagicExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		gpGlobals->Exp.rgAttackExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		gpGlobals->Exp.rgMagicPowerExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		gpGlobals->Exp.rgDefenseExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		gpGlobals->Exp.rgDexterityExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		gpGlobals->Exp.rgFleeExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
	}

	gpGlobals->fEnteringScene = TRUE;
}

static INT
PAL_LoadGame(
	LPCSTR         szFileName
	)
	/*++
	  Purpose:

	  Load a saved game.

	  Parameters:

	  [IN]  szFileName - file name of saved game.

	  Return value:

	  0 if success, -1 if failed.

	  --*/
{
	FILE                     *fp;
	PAL_LARGE SAVEDGAME       s;
	UINT32                    i;

	//
	// Try to open the specified file
	//
	fp = fopen(szFileName, "rb");
	if (fp == NULL)
	{
		return -1;
	}

	//
	// Read all data from the file and close.
	//
	fread(&s, sizeof(SAVEDGAME), 1, fp);
	fclose(fp);

	//
	// Adjust endianness
	//
	DO_BYTESWAP(&s, sizeof(SAVEDGAME));

	//
	// Cash amount is in DWORD, so do a wordswap in Big-Endian.
	//
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	s.dwCash = ((s.dwCash >> 16) | (s.dwCash << 16));
#endif

	//
	// Get all the data from the saved game struct.
	//
	gpGlobals->viewport = PAL_XY(s.wViewportX, s.wViewportY);

#ifdef BIG_RESOLUTION
	gpGlobals->viewport = PAL_XY(PAL_X(gpGlobals->viewport) - (0.5 * g_iTextureWidth - 160),
		PAL_Y(gpGlobals->viewport) - (0.56 * g_iTextureHeight - 112));
#endif

	gpGlobals->wMaxPartyMemberIndex = s.nPartyMember;
	gpGlobals->wNumScene = s.wNumScene;
	gpGlobals->fNightPalette = (s.wPaletteOffset != 0);
	gpGlobals->wPartyDirection = s.wPartyDirection;
	gpGlobals->wNumMusic = s.wNumMusic;
	gpGlobals->wNumBattleMusic = s.wNumBattleMusic;
	gpGlobals->wNumBattleField = s.wNumBattleField;
	gpGlobals->wScreenWave = s.wScreenWave;
	gpGlobals->sWaveProgression = 0;
	gpGlobals->wCollectValue = s.wCollectValue;
	gpGlobals->wLayer = s.wLayer;
	gpGlobals->wChaseRange = s.wChaseRange;
	gpGlobals->wChasespeedChangeCycles = s.wChasespeedChangeCycles;
	gpGlobals->nFollower = s.nFollower;
	gpGlobals->dwCash = s.dwCash;

#ifdef FINISH_GAME_MORE_ONE_TIME
	gpGlobals->bFinishGameTime = s.bFinishGameTime;
#endif

#ifndef PAL_CLASSIC
	gpGlobals->bBattleSpeed = s.wBattleSpeed;
	if (gpGlobals->bBattleSpeed > 5 || gpGlobals->bBattleSpeed == 0)
	{
		gpGlobals->bBattleSpeed = 2;
	}
#endif

	memcpy(gpGlobals->rgParty, s.rgParty, sizeof(gpGlobals->rgParty));
	memcpy(gpGlobals->rgTrail, s.rgTrail, sizeof(gpGlobals->rgTrail));

#ifdef BIG_RESOLUTION
	for (i = 0; i < MAX_PLAYABLE_PLAYER_ROLES; i++)
	{
		gpGlobals->rgParty[i].x += 0.5 * g_iTextureWidth - 160;
		gpGlobals->rgParty[i].y += 0.56 * g_iTextureHeight - 112;
		gpGlobals->rgTrail[i].x += 0.5 * g_iTextureWidth - 160;
		gpGlobals->rgTrail[i].y += 0.56 * g_iTextureHeight - 112;
	}
#endif

	gpGlobals->Exp = s.Exp;
	gpGlobals->g.PlayerRoles = s.PlayerRoles;
	memset(gpGlobals->rgPoisonStatus, 0, sizeof(gpGlobals->rgPoisonStatus));
	memcpy(gpGlobals->rgInventory, s.rgInventory, sizeof(gpGlobals->rgInventory));
	memcpy(gpGlobals->g.rgScene, s.rgScene, sizeof(gpGlobals->g.rgScene));
	memcpy(gpGlobals->g.rgObject, s.rgObject, sizeof(gpGlobals->g.rgObject));
	memcpy(gpGlobals->g.lprgEventObject, s.rgEventObject,
		sizeof(EVENTOBJECT)* gpGlobals->g.nEventObject);

	gpGlobals->fEnteringScene = FALSE;

	PAL_CompressInventory();

	//
	// Success
	//
	return 0;
}

VOID
PAL_SaveGame(
	LPCSTR         szFileName,
	WORD           wSavedTimes
	)
	/*++
	  Purpose:

	  Save the current game state to file.

	  Parameters:

	  [IN]  szFileName - file name of saved game.

	  Return value:

	  None.

	  --*/
{
	FILE                     *fp;
	PAL_LARGE SAVEDGAME       s;
	UINT32                    i;

	memset(&s, 0, sizeof(s));
	//
	// Put all the data to the saved game struct.
	//
	s.wViewportX = PAL_X(gpGlobals->viewport);
	s.wViewportY = PAL_Y(gpGlobals->viewport);
	s.nPartyMember = gpGlobals->wMaxPartyMemberIndex;
	s.wNumScene = gpGlobals->wNumScene;
	s.wPaletteOffset = (gpGlobals->fNightPalette ? 0x180 : 0);
	s.wPartyDirection = gpGlobals->wPartyDirection;
	s.wNumMusic = gpGlobals->wNumMusic;
	s.wNumBattleMusic = gpGlobals->wNumBattleMusic;
	s.wNumBattleField = gpGlobals->wNumBattleField;
	s.wScreenWave = gpGlobals->wScreenWave;
	s.wCollectValue = gpGlobals->wCollectValue;
	s.wLayer = gpGlobals->wLayer;
	s.wChaseRange = gpGlobals->wChaseRange;
	s.wChasespeedChangeCycles = gpGlobals->wChasespeedChangeCycles;
	s.nFollower = gpGlobals->nFollower;
#ifdef FINISH_GAME_MORE_ONE_TIME
	s.bFinishGameTime = gpGlobals->bFinishGameTime;
	memset(s.rgbReserved, 0, sizeof(s.rgbReserved));
#else
	s.rgwReserved2[0] = 0;
	s.rgwReserved2[1] = 0;
	s.rgwReserved2[2] = 0;
#endif
	s.dwCash = gpGlobals->dwCash;
#ifndef PAL_CLASSIC
	s.wBattleSpeed = gpGlobals->bBattleSpeed;
#else
	s.wBattleSpeed = 2;
#endif

	memcpy(s.rgParty, gpGlobals->rgParty, sizeof(gpGlobals->rgParty));
	memcpy(s.rgTrail, gpGlobals->rgTrail, sizeof(gpGlobals->rgTrail));
	s.Exp = gpGlobals->Exp;
	s.PlayerRoles = gpGlobals->g.PlayerRoles;

	memset(s.rgbReserved2, 0, sizeof(s.rgbReserved2));

	memcpy(s.rgInventory, gpGlobals->rgInventory, sizeof(gpGlobals->rgInventory));
	memcpy(s.rgScene, gpGlobals->g.rgScene, sizeof(gpGlobals->g.rgScene));
	memcpy(s.rgObject, gpGlobals->g.rgObject, sizeof(gpGlobals->g.rgObject));
	memcpy(s.rgEventObject, gpGlobals->g.lprgEventObject,
		sizeof(EVENTOBJECT)* gpGlobals->g.nEventObject);

	s.wSavedTimes = wSavedTimes;

	//
	// Adjust endianness
	//
	DO_BYTESWAP(&s, sizeof(SAVEDGAME));

	//
	// Cash amount is in DWORD, so do a wordswap in Big-Endian.
	//
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	s.dwCash = ((s.dwCash >> 16) | (s.dwCash << 16));
#endif

	//
	// Try writing to file
	//
	fp = fopen(szFileName, "wb");
	if (fp == NULL)
	{
		return;
	}

	i = PAL_MKFGetChunkSize(0, gpGlobals->f.fpSSS);
	i += sizeof(SAVEDGAME) - sizeof(EVENTOBJECT)* MAX_EVENT_OBJECTS;

	fwrite(&s, i, 1, fp);
	fclose(fp);
}

VOID
PAL_InitGameData(
	INT         iSaveSlot
	)
	/*++
	  Purpose:

	  Initialize the game data (used when starting a new game or loading a saved game).

	  Parameters:

	  [IN]  iSaveSlot - Slot of saved game.

	  Return value:

	  None.

	  --*/
{
	PAL_InitGlobalGameData();

	if (iSaveSlot > 0)
	{
		gpGlobals->bCurrentSaveSlot = (BYTE)iSaveSlot;
	}


	if (iSaveSlot < 0)
	{
#ifdef FINISH_GAME_MORE_ONE_TIME
		PAL_New_GoBackAndLoadDefaultGame();
#endif
	}
	else if (iSaveSlot == 0 || PAL_LoadGame(va("%s%d%s", PAL_SAVE_PREFIX, iSaveSlot, ".rpg")) != 0)
	{
		//
		// Cannot load the saved game file. Load the defaults.
		//
		PAL_LoadDefaultGame();
	}


	gpGlobals->fGameStart = TRUE;
	gpGlobals->fNeedToFadeIn = FALSE;
	gpGlobals->iCurInvMenuItem = 0;
	gpGlobals->fInBattle = FALSE;

	memset(gpGlobals->rgPlayerStatus, 0, sizeof(gpGlobals->rgPlayerStatus));
	PAL_UpdateEquipments();
}

BOOL
PAL_AddItemToInventory(
	WORD          wObjectID,
	INT           iNum
	)
	/*++
	  Purpose:  Add or remove the specified kind of item in the inventory.

	  Parameters:  [IN]  wObjectID - object number of the item.
	  [IN]  iNum - number to be added (positive value) or removed (negative value).
	  Return value:  TRUE if succeeded, FALSE if failed.
	  --*/
{
	int          index;
	BOOL         fFound;


	if (wObjectID == 0)
	{
		return FALSE;
	}

	if (iNum == 0)
	{
		iNum = 1;
	}

	index = 0;
	fFound = FALSE;

	//
	// Search for the specified item in the inventory
	//
	while (index < MAX_INVENTORY)
	{
		if (gpGlobals->rgInventory[index].wItem == wObjectID)
		{
			fFound = TRUE;
			break;
		}
		else if (gpGlobals->rgInventory[index].wItem == 0)
		{
			break;
		}
		index++;
	}

	if (iNum > 0)
	{
		//
		// Add item
		//
		if (index >= MAX_INVENTORY)
		{
			//
			// inventory is full. cannot add item
			//
			return FALSE;
		}

		if (fFound)
		{
			gpGlobals->rgInventory[index].nAmount += iNum;
			if (gpGlobals->rgInventory[index].nAmount > MAX_ITEM_NUM_IN_INVENTORY)
			{
				//
				// Maximum number is 99
				//
				gpGlobals->rgInventory[index].nAmount = MAX_ITEM_NUM_IN_INVENTORY;
			}
		}
		else
		{
			gpGlobals->rgInventory[index].wItem = wObjectID;
			if (iNum > MAX_ITEM_NUM_IN_INVENTORY)
			{
				iNum = MAX_ITEM_NUM_IN_INVENTORY;
			}
			gpGlobals->rgInventory[index].nAmount = iNum;
		}

		return TRUE;
	}
	else
	{
		//
		// Remove item
		//
		if (fFound)
		{
			iNum *= -1;
			if (gpGlobals->rgInventory[index].nAmount < iNum)
			{
				//
				// This item has been run out
				//
				gpGlobals->rgInventory[index].nAmount = 0;
				return FALSE;
			}

			gpGlobals->rgInventory[index].nAmount -= iNum;
			return TRUE;
		}

		return FALSE;
	}
}

INT
PAL_GetItemAmount(
	WORD        wItem
	)
	/*++
	  Purpose:    Get the amount of the specified item in the inventory.
	  Parameters:    [IN]  wItem - the object ID of the item.
	  Return value:    The amount of the item in the inventory.
	  --*/
{
	int i;

	for (i = 0; i < MAX_INVENTORY; i++)
	{
		if (gpGlobals->rgInventory[i].wItem == 0)
		{
			break;
		}

		if (gpGlobals->rgInventory[i].wItem == wItem)
		{
			return gpGlobals->rgInventory[i].nAmount;
		}
	}

	return 0;
}

VOID
PAL_CompressInventory(
	VOID
	)
	/*++
	  Purpose:    Remove all the items in inventory which has a number of zero.
	  Parameters:    None.
	  Return value:    None.
	  --*/
{
	int i, j;

	j = 0;

	for (i = 0; i < MAX_INVENTORY; i++)
	{
		if (gpGlobals->rgInventory[i].wItem == 0)
		{
			break;
		}

		if (gpGlobals->rgInventory[i].nAmount > 0)
		{
			gpGlobals->rgInventory[j] = gpGlobals->rgInventory[i];
			j++;
		}
	}

	for (; j < MAX_INVENTORY; j++)
	{
		gpGlobals->rgInventory[j].nAmount = 0;
		gpGlobals->rgInventory[j].nAmountInUse = 0;
		gpGlobals->rgInventory[j].wItem = 0;
	}
}

BOOL
PAL_IncreaseHPMP(
	WORD          wPlayerRole,
	SHORT         sHP,
	SHORT         sMP
	)
	/*++
	  Purpose:

	  Increase or decrease player's HP and/or MP.

	  Parameters:

	  [IN]  wPlayerRole - the number of player role.

	  [IN]  sHP - number of HP to be increased (positive value) or decrased
	  (negative value).

	  [IN]  sMP - number of MP to be increased (positive value) or decrased
	  (negative value).

	  Return value:

	  TRUE if the operation is succeeded, FALSE if not.

	  --*/
{
	INT iHP, iMP = 0;

#ifdef INCREASE_EXTRA_PLAYER_HPMP
	WORD extraLevel = 0;
	WORD levelDifference = 0;
	FLOAT times = 1;
#endif

	//
	// Only care about alive players
	//
	if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0)
	{
		return FALSE;
	}

	iHP = gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole];
	iMP = gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole];
	//
	// change HP
	//
	iHP += sHP;

#ifdef INCREASE_EXTRA_PLAYER_HPMP 
	if (gpGlobals->g.PlayerRoles.rgwLevel[wPlayerRole] >= INCREASE_EXTRA_HPMP_MIN_LEVEL && sHP > 0)
	{
		levelDifference = INCREASE_EXTRA_HPMP_MAX_LEVEL - INCREASE_EXTRA_HPMP_MIN_LEVEL;
		levelDifference = max(levelDifference, 1);
		extraLevel = gpGlobals->g.PlayerRoles.rgwLevel[wPlayerRole] - INCREASE_EXTRA_HPMP_MIN_LEVEL;
		extraLevel = min(levelDifference, extraLevel);
		times = ((float)max(max(gpGlobals->g.PlayerRoles.rgwMaxHP[wPlayerRole], MAX_PARAMETER) - 1000, 0) / 1000.0) * ((float)extraLevel / (float)levelDifference);
#ifdef FINISH_GAME_MORE_ONE_TIME
		times = min(times, gpGlobals->bFinishGameTime + 1);
#endif
		iHP += sHP * times;
	}
#endif

	iHP = max(iHP, 0);
	iHP = min(gpGlobals->g.PlayerRoles.rgwMaxHP[wPlayerRole], iHP);
	gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] = iHP;

	//
	// Change MP
	//
	iMP += sMP;

#ifdef INCREASE_EXTRA_PLAYER_HPMP 
	if (gpGlobals->g.PlayerRoles.rgwLevel[wPlayerRole] >= INCREASE_EXTRA_HPMP_MIN_LEVEL && sMP > 0)
	{
		levelDifference = INCREASE_EXTRA_HPMP_MAX_LEVEL - INCREASE_EXTRA_HPMP_MIN_LEVEL;
		levelDifference = max(levelDifference, 1);
		extraLevel = gpGlobals->g.PlayerRoles.rgwLevel[wPlayerRole] - INCREASE_EXTRA_HPMP_MIN_LEVEL;
		extraLevel = min(levelDifference, extraLevel);
		times = ((float)max(max(gpGlobals->g.PlayerRoles.rgwMaxMP[wPlayerRole], MAX_PARAMETER) - 1000, 0) / 1000.0) * ((float)extraLevel / (float)levelDifference);
#ifdef FINISH_GAME_MORE_ONE_TIME
		times = min(times, gpGlobals->bFinishGameTime + 1);
#endif
		iMP += sMP * times;
	}
#endif

	iMP = max(iMP, 0);
	iMP = min(iMP, gpGlobals->g.PlayerRoles.rgwMaxMP[wPlayerRole]);
	gpGlobals->g.PlayerRoles.rgwMP[wPlayerRole] = iMP;

	return TRUE;

}

VOID
PAL_UpdateEquipments(
	VOID
	)
	/*++
	  Purpose:

	  Update the effects of all equipped items for all players.

	  Parameters:

	  None.

	  Return value:

	  None.

	  --*/
{
	int      i, j;
	WORD     w;

	memset(&(gpGlobals->rgEquipmentEffect), 0, sizeof(gpGlobals->rgEquipmentEffect));

	for (i = 0; i < MAX_PLAYER_ROLES; i++)
	{
		for (j = 0; j < MAX_PLAYER_EQUIPMENTS; j++)
		{
			w = gpGlobals->g.PlayerRoles.rgwEquipment[j][i];

			if (w != 0)
			{
				gpGlobals->g.rgObject[w].item.wScriptOnEquip =
					PAL_RunTriggerScript(gpGlobals->g.rgObject[w].item.wScriptOnEquip, (WORD)i);
			}
		}
	}
}

VOID
PAL_RemoveEquipmentEffect(
	WORD			wPlayerRole,
	WORD			wEquipPart
	)
	/*++
	  Purpose:

	  Remove all the effects of the equipment for the player.

	  Parameters:

	  [IN]  wPlayerRole - the player role.

	  [IN]  wEquipPart - the part of the equipment.

	  Return value:

	  None.

	  --*/
{
	WORD       *p;
	int         i;

	p = (WORD *)(&gpGlobals->rgEquipmentEffect[wEquipPart]); // HACKHACK

	for (i = 0; i < sizeof(PLAYERROLES) / sizeof(PLAYERS); i++)
	{
		p[i * MAX_PLAYER_ROLES + wPlayerRole] = 0;
	}

	//
	// Reset some parameters to default when appropriate
	//
	if (wEquipPart == kBodyPartHand)
	{
		//
		// reset the dual attack status
		//
		gpGlobals->rgPlayerStatus[wPlayerRole][kStatusDualAttack] = 0;
	}
	else if (wEquipPart == kBodyPartWear)
	{   //解除寿葫芦毒
		PAL_CurePoisonByKind(wPlayerRole, 0x0233);
	}
}

VOID PAL_AddPoisonForPlayer(
	WORD			wPlayerRole,
	WORD			wPoisonID
	)
	/*++
	  Purpose:    Add the specified poison to the player.
	  对我方角色增加指定的毒
	  Parameters:    [IN]  wPlayerRole - the player role ID.
	  [IN]  wPoisonID - the poison to be added.
	  角色id，毒的id
	  Return value:    None.
	  注：不可以在该函数内进行毒排序，会造成脚本执行混乱
	  --*/
{
	int			i, index, iPoisonIndex;
	WORD			w;

	index = PAL_New_GetPlayerIndex(wPlayerRole);
	WORD wPoisonLevel = gpGlobals->g.rgObject[wPoisonID].poison.wPoisonLevel;

	if (index == -1 || gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0)
	{
		return;
	}

	iPoisonIndex = PAL_New_GetPoisonIndexForPlayer(wPlayerRole, wPoisonID);
	if (iPoisonIndex != -1)
	{
		// already poisoned
#ifdef POISON_STATUS_EXPAND // 增加毒的烈度
		INT iSuccessRate = 100;
		iSuccessRate -= wPoisonLevel * 20;
		iSuccessRate = max(iSuccessRate, 0);
		//iSuccessRate *= (100 - PAL_GetPlayerPoisonResistance(wPlayerRole)) / 100.0;
		if (PAL_New_GetTrueByPercentage(iSuccessRate))
		{
			gpGlobals->rgPoisonStatus[iPoisonIndex][index].wPoisonIntensity++;
		}
		switch (wPoisonLevel)
		{
			case 0:
			case 1:
				gpGlobals->rgPoisonStatus[iPoisonIndex][index].wPoisonIntensity =
					min(gpGlobals->rgPoisonStatus[iPoisonIndex][index].wPoisonIntensity, 3);
				break;

			case 2:
				gpGlobals->rgPoisonStatus[iPoisonIndex][index].wPoisonIntensity =
					min(gpGlobals->rgPoisonStatus[iPoisonIndex][index].wPoisonIntensity, 2);
				break;

			case 3:
				gpGlobals->rgPoisonStatus[iPoisonIndex][index].wPoisonIntensity =
					min(gpGlobals->rgPoisonStatus[iPoisonIndex][index].wPoisonIntensity, 1);
			default:
				gpGlobals->rgPoisonStatus[iPoisonIndex][index].wPoisonIntensity = 0;
				break;
		}
#endif
		return;
	}

	for (i = 0; i < MAX_POISONS; i++)
	{
		w = gpGlobals->rgPoisonStatus[i][index].wPoisonID;
		if (w == wPoisonID)
		{	// already poisoned
			return;
		}
		else if (w == 0)
		{
			break;
		}
	}

	i = min(i, MAX_POISONS - 1);
	gpGlobals->rgPoisonStatus[i][index].wPoisonID = wPoisonID;
	gpGlobals->rgPoisonStatus[i][index].wPoisonScript = gpGlobals->g.rgObject[wPoisonID].poison.wPlayerScript;

#ifdef POISON_STATUS_EXPAND // 增加毒的烈度
#ifdef FINISH_GAME_MORE_ONE_TIME
	if (wPoisonLevel < 3)
	{
		gpGlobals->rgPoisonStatus[i][index].wPoisonIntensity = gpGlobals->bFinishGameTime;
	}
#endif
#endif
}

VOID
PAL_CurePoisonByKind(
	WORD			wPlayerRole,
	WORD			wPoisonID
	)
	/*++
	  Purpose:

	  Remove the specified poison from the player.

	  Parameters:

	  [IN]  wPlayerRole - the player role ID.

	  [IN]  wPoisonID - the poison to be removed.

	  Return value:

	  None.

	  --*/
{
	int i, index;

	for (index = 0; index <= gpGlobals->wMaxPartyMemberIndex; index++)
	{
		if (gpGlobals->rgParty[index].wPlayerRole == wPlayerRole)
		{
			break;
		}
	}

	if (index > gpGlobals->wMaxPartyMemberIndex)
	{
		return; // don't go further
	}

	for (i = 0; i < MAX_POISONS; i++)
	{
		if (gpGlobals->rgPoisonStatus[i][index].wPoisonID == wPoisonID)
		{
#ifdef POISON_STATUS_EXPAND
			memset(&gpGlobals->rgPoisonStatus[i][index], 0, sizeof(gpGlobals->rgPoisonStatus[i][index]));
#else
			gpGlobals->rgPoisonStatus[i][index].wPoisonID = 0;
			gpGlobals->rgPoisonStatus[i][index].wPoisonScript = 0;
#endif
		}
	}
}

VOID
PAL_CurePoisonByLevel(
	WORD			wPlayerRole,
	WORD			wMaxLevel
	)
	/*++
	  Purpose:    Remove the poisons which have a maximum level of wMaxLevel from the player.

	  Parameters:    [IN]  wPlayerRole - the player role ID.
	  [IN]  wMaxLevel - the maximum level of poisons to be removed.

	  Return value:    None.
	  --*/
{
	int        i, index;
	WORD       w;

	for (index = 0; index <= gpGlobals->wMaxPartyMemberIndex; index++)
	{
		if (gpGlobals->rgParty[index].wPlayerRole == wPlayerRole)
		{
			break;
		}
	}

	if (index > gpGlobals->wMaxPartyMemberIndex)
	{
		return; // don't go further
	}

	for (i = 0; i < MAX_POISONS; i++)
	{
		w = gpGlobals->rgPoisonStatus[i][index].wPoisonID;

		if (PAL_New_IsAnItemInArray(w, ExcludePoisonIDAfterRevive))
		{
			continue;
		}
		if (gpGlobals->g.rgObject[w].poison.wPoisonLevel <= wMaxLevel)
		{
#ifdef POISON_STATUS_EXPAND
			memset(&gpGlobals->rgPoisonStatus[i][index], 0, sizeof(gpGlobals->rgPoisonStatus[i][index]));
#else
			gpGlobals->rgPoisonStatus[i][index].wPoisonID = 0;
			gpGlobals->rgPoisonStatus[i][index].wPoisonScript = 0;
#endif
		}
	}
}

BOOL
PAL_New_IsAnItemInArray(
	WORD			item,
	const WORD		items[]
	)
{
	int i = 0;
	int length = sizeof(items) / sizeof(WORD);
	BOOL result = FALSE;

	for (i = 0; i < length; i++)
	{
		if (item == items[i])
		{
			result = TRUE;
			break;
		}
	}
	return result;
}

BOOL
PAL_IsPlayerPoisonedByLevel(
	WORD			wPlayerRole,
	WORD			wMinLevel
	)
	/*++
	  Purpose:

	  Check if the player is poisoned by poisons at a minimum level of wMinLevel.

	  Parameters:

	  [IN]  wPlayerRole - the player role ID.

	  [IN]  wMinLevel - the minimum level of poison.

	  Return value:

	  TRUE if the player is poisoned by poisons at a minimum level of wMinLevel;
	  FALSE if not.

	  --*/
{
	int         i, index;
	WORD        w;

	for (index = 0; index <= gpGlobals->wMaxPartyMemberIndex; index++)
	{
		if (gpGlobals->rgParty[index].wPlayerRole == wPlayerRole)
		{
			break;
		}
	}

	if (index > gpGlobals->wMaxPartyMemberIndex)
	{
		return FALSE; // don't go further
	}

	for (i = 0; i < MAX_POISONS; i++)
	{
		w = gpGlobals->rgPoisonStatus[i][index].wPoisonID;
		w = gpGlobals->g.rgObject[w].poison.wPoisonLevel;

		if (w >= 99)
		{
			//
			// Ignore poisons which has a level of 99 (usually effect of equipment)
			//
			continue;
		}

		if (w >= wMinLevel)
		{
			return TRUE;
		}
	}

	return FALSE;
}

INT
PAL_New_GetPoisonIndexForPlayer(
	WORD			wPlayerRole,
	WORD			wPoisonID
	)
	/*++
	  Purpose:    Check if the player is poisoned by the specified poison.

	  Parameters:		[IN]  wPlayerRole - the player role ID.
	  [IN]  wPoisonID - the poison to be checked.

	  Return value:		如果没有中这种毒，返回-1；
	  如果中毒，则返回该毒在该角色的rgPoisonStatus的位置序号
	  --*/
{
	int i, index;

	index = PAL_New_GetPlayerIndex(wPlayerRole);
	if (index == -1)
	{
		return -1;
	}

	for (i = 0; i < MAX_POISONS; i++)
	{
		if (gpGlobals->rgPoisonStatus[i][index].wPoisonID == wPoisonID)
		{
			return i;
		}
	}
	return -1;
}

BOOL
PAL_New_IsPlayerPoisoned(
	WORD			wPlayerRole
	)
	/*++
	  Purpose:    Check if the player is poisoned.
	  Parameters:    [IN]  wPlayerRole - the player role ID.
	  Return value:    TRUE if player is poisoned;
	  FALSE if not.
	  --*/
{
	int i, index;

	index = PAL_New_GetPlayerIndex(wPlayerRole);
	if (index == -1)
	{
		return FALSE;
	}

	for (i = 0; i < MAX_POISONS; i++)
	{
		if (gpGlobals->rgPoisonStatus[i][index].wPoisonID != 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

VOID
PAL_New_SortPoisonsForPlayerByLevel(
	WORD			wPlayerRole
)
{
	int         i, j, index, PoisonNum;
	WORD        wPoisonID1, wPoisonID2;
	WORD        wPoisonLevel1, wPoisonLevel2;
	POISONSTATUS	tempPoison;

	for (index = 0; index <= gpGlobals->wMaxPartyMemberIndex; index++)
	{
		if (gpGlobals->rgParty[index].wPlayerRole == wPlayerRole)break;
	}

	if (index > gpGlobals->wMaxPartyMemberIndex)return; // don't go further

	for (j = 0, PoisonNum = 0; j < MAX_POISONS; j++)
	{
		wPoisonID1 = gpGlobals->rgPoisonStatus[j][index].wPoisonID;
		if (wPoisonID1 == 0) gpGlobals->rgPoisonStatus[j][index].wPoisonScript = 0;
		else PoisonNum++;
	}

	if (PoisonNum < 2)	return;	//中毒数目小于2不用排序


	for (i = 0; i < MAX_POISONS - 1; i++)
	{
		for (j = 0; j < MAX_POISONS - i - 1; j++)
		{
			wPoisonID1 = gpGlobals->rgPoisonStatus[j][index].wPoisonID;
			wPoisonLevel1 = gpGlobals->g.rgObject[wPoisonID1].poison.wPoisonLevel;
			wPoisonID2 = gpGlobals->rgPoisonStatus[j + 1][index].wPoisonID;
			wPoisonLevel2 = gpGlobals->g.rgObject[wPoisonID2].poison.wPoisonLevel;

			if (wPoisonLevel1 < wPoisonLevel2)
			{
				tempPoison = gpGlobals->rgPoisonStatus[j][index];
				gpGlobals->rgPoisonStatus[j][index] = gpGlobals->rgPoisonStatus[j + 1][index];
				gpGlobals->rgPoisonStatus[j + 1][index] = tempPoison;
			}
		}
	}
}

VOID
PAL_New_SortInventory(
	)
{
	int         i, j;
	WORD        ItemID1, ItemID2;
	WORD		ItemNum;
	INVENTORY   TempItem;
	INVENTORY	TempInventory[MAX_INVENTORY];

	memset(TempInventory, 0, sizeof(TempInventory));

	for (i = 0, j = 0; i < MAX_INVENTORY; i++)
	{
		TempItem = gpGlobals->rgInventory[i];
		if (TempItem.wItem != 0 && TempItem.nAmount != 0)
		{
			TempInventory[j] = TempItem;
			j++;
		}
	}
	ItemNum = j;

	for (i = 0; i < ItemNum; i++)
	{
		for (j = 0; j < ItemNum - i - 1; j++)
		{
			ItemID1 = TempInventory[j].wItem;
			ItemID2 = TempInventory[j + 1].wItem;


			if (ItemID1 > ItemID2)
			{
				TempItem = TempInventory[j];
				TempInventory[j] = TempInventory[j + 1];
				TempInventory[j + 1] = TempItem;
			}
		}
	}

	for (i = 0; i < MAX_INVENTORY; i++)
	{
		gpGlobals->rgInventory[i] = TempInventory[i];
	}

	return;
}

WORD
PAL_GetPlayerAttackStrength(
	WORD			wPlayerRole
	)
	/*++
	  Purpose:

	  Get the player's attack strength, count in the effect of equipments.

	  Parameters:

	  [IN]  wPlayerRole - the player role ID.

	  Return value:

	  The total attack strength of the player.

	  --*/
{
	WORD       w;
	int        i;

	w = gpGlobals->g.PlayerRoles.rgwAttackStrength[wPlayerRole];

	for (i = 0; i <= MAX_PLAYER_EQUIPMENTS; i++)
	{
		w += gpGlobals->rgEquipmentEffect[i].rgwAttackStrength[wPlayerRole];
	}

	return w;
}

WORD
PAL_GetPlayerMagicStrength(
	WORD			wPlayerRole
	)
	/*++
	  Purpose:

	  Get the player's magic strength, count in the effect of equipments.

	  Parameters:

	  [IN]  wPlayerRole - the player role ID.

	  Return value:

	  The total magic strength of the player.

	  --*/
{
	WORD       w;
	int        i;

	w = gpGlobals->g.PlayerRoles.rgwMagicStrength[wPlayerRole];

	for (i = 0; i <= MAX_PLAYER_EQUIPMENTS; i++)
	{
		w += gpGlobals->rgEquipmentEffect[i].rgwMagicStrength[wPlayerRole];
	}

	return w;
}

WORD
PAL_GetPlayerDefense(
	WORD			wPlayerRole
	)
	/*++
	  Purpose:

	  Get the player's defense value, count in the effect of equipments.

	  Parameters:

	  [IN]  wPlayerRole - the player role ID.

	  Return value:

	  The total defense value of the player.

	  --*/
{
	WORD       w;
	int        i;

	w = gpGlobals->g.PlayerRoles.rgwDefense[wPlayerRole];

	for (i = 0; i <= MAX_PLAYER_EQUIPMENTS; i++)
	{
		w += gpGlobals->rgEquipmentEffect[i].rgwDefense[wPlayerRole];
	}

	return w;
}

WORD
PAL_GetPlayerDexterity(
	WORD           wPlayerRole
	)
	/*++
	  Purpose:

	  Get the player's dexterity, count in the effect of equipments.

	  Parameters:

	  [IN]  wPlayerRole - the player role ID.

	  Return value:

	  The total dexterity of the player.

	  --*/
{
	WORD       w;
	int        i;

	w = gpGlobals->g.PlayerRoles.rgwDexterity[wPlayerRole];

#ifdef PAL_CLASSIC
	for (i = 0; i <= MAX_PLAYER_EQUIPMENTS; i++)
#else
	for (i = 0; i <= MAX_PLAYER_EQUIPMENTS - 1; i++)
#endif
	{
		w += gpGlobals->rgEquipmentEffect[i].rgwDexterity[wPlayerRole];
	}

	return w;
}

WORD
PAL_GetPlayerFleeRate(
	WORD           wPlayerRole
	)
	/*++
	  Purpose:

	  Get the player's flee rate, count in the effect of equipments.

	  Parameters:

	  [IN]  wPlayerRole - the player role ID.

	  Return value:

	  The total flee rate of the player.

	  --*/
{
	WORD       w;
	int        i;

	w = gpGlobals->g.PlayerRoles.rgwFleeRate[wPlayerRole];

	for (i = 0; i <= MAX_PLAYER_EQUIPMENTS; i++)
	{
		w += gpGlobals->rgEquipmentEffect[i].rgwFleeRate[wPlayerRole];
	}

	return w;
}

WORD
PAL_GetPlayerPoisonResistance(
	WORD           wPlayerRole
	)
	/*++
	  Purpose:

	  Get the player's resistance to poisons, count in the effect of equipments.

	  Parameters:

	  [IN]  wPlayerRole - the player role ID.

	  Return value:

	  The total resistance to poisons of the player.

	  --*/
{
	WORD       w;
	int        i;

	w = gpGlobals->g.PlayerRoles.rgwPoisonResistance[wPlayerRole];

	for (i = 0; i <= MAX_PLAYER_EQUIPMENTS; i++)
	{
		w += gpGlobals->rgEquipmentEffect[i].rgwPoisonResistance[wPlayerRole];
	}

	return min(w, 100);
}

WORD
PAL_New_GetPlayerSorceryResistance(
	WORD			wPlayerRole
	)
	/*++
		Purpose:Get the player's resistance to Sorcery, count in the effect of equipments.
		Parameters:[IN]  wPlayerRole - the player role ID.
		Return value:The total resistance to Sorcery of the player.
		--*/
{
	WORD       w;
	int        i;

	w = gpGlobals->g.PlayerRoles.rgwSorceryResistance[wPlayerRole];

	for (i = 0; i <= MAX_PLAYER_EQUIPMENTS; i++)
	{
		w += gpGlobals->rgEquipmentEffect[i].rgwSorceryResistance[wPlayerRole];
	}

	return min(100, w);
}

WORD
PAL_New_GetPlayerSorceryStrength(
	WORD			wPlayerRole
	)
	/*++
	Purpose:Get the player's resistance to Sorcery, count in the effect of equipments.
	Parameters:[IN]  wPlayerRole - the player role ID.
	Return value:The total resistance to Sorcery of the player.
	--*/
{
	WORD       w;
	int        i;

	w = gpGlobals->g.PlayerRoles.rgwSorceryStrength[wPlayerRole];

	for (i = 0; i <= MAX_PLAYER_EQUIPMENTS; i++)
	{
		w += gpGlobals->rgEquipmentEffect[i].rgwSorceryStrength[wPlayerRole];
	}

	return min(100, w);
}

WORD
PAL_GetPlayerElementalResistance(
	WORD			wPlayerRole,
	INT				iAttrib
	)
	/*++
	  Purpose:

	  Get the player's resistance to attributed magics, count in the effect
	  of equipments.

	  Parameters:

	  [IN]  wPlayerRole - the player role ID.

	  [IN]  iAttrib - the attribute of magics.

	  Return value:

	  The total resistance to the attributed magics of the player.

	  --*/
{
	WORD       w;
	int        i;

	w = gpGlobals->g.PlayerRoles.rgwElementalResistance[iAttrib][wPlayerRole];

	for (i = 0; i <= MAX_PLAYER_EQUIPMENTS; i++)
	{
		w += gpGlobals->rgEquipmentEffect[i].rgwElementalResistance[iAttrib][wPlayerRole];
	}

	return min(w, 100);
}

WORD
PAL_New_GetPlayerPhysicalResistance(
	WORD			wPlayerRole
	)
{
	WORD       w;
	int        i;

	w = gpGlobals->g.PlayerRoles.rgwPhysicalResistance[wPlayerRole];

	for (i = 0; i <= MAX_PLAYER_EQUIPMENTS; i++)
	{
		w += gpGlobals->rgEquipmentEffect[i].rgwPhysicalResistance[wPlayerRole];
	}

	return min(100, w);
}

WORD
PAL_GetPlayerBattleSprite(
	WORD             wPlayerRole
	)
	/*++
	  Purpose:

	  Get player's battle sprite.

	  Parameters:

	  [IN]  wPlayerRole - the player role ID.

	  Return value:

	  Number of the player's battle sprite.

	  --*/
{
	int       i;
	WORD      w;

	w = gpGlobals->g.PlayerRoles.rgwSpriteNumInBattle[wPlayerRole];

	for (i = 0; i <= MAX_PLAYER_EQUIPMENTS; i++)
	{
		if (gpGlobals->rgEquipmentEffect[i].rgwSpriteNumInBattle[wPlayerRole] != 0)
		{
			w = gpGlobals->rgEquipmentEffect[i].rgwSpriteNumInBattle[wPlayerRole];
		}
	}

	return w;
}

WORD
PAL_GetPlayerCooperativeMagic(
	WORD             wPlayerRole
	)
	/*++
	  Purpose:

	  Get player's cooperative magic.

	  Parameters:

	  [IN]  wPlayerRole - the player role ID.

	  Return value:

	  Object ID of the player's cooperative magic.

	  --*/
{
	int       i;
	WORD      w;

	w = gpGlobals->g.PlayerRoles.rgwCooperativeMagic[wPlayerRole];

	for (i = 0; i <= MAX_PLAYER_EQUIPMENTS; i++)
	{
		if (gpGlobals->rgEquipmentEffect[i].rgwCooperativeMagic[wPlayerRole] != 0)
		{
			w = gpGlobals->rgEquipmentEffect[i].rgwCooperativeMagic[wPlayerRole];
		}
	}

	return w;
}

BOOL
PAL_PlayerCanAttackAll(
	WORD        wPlayerRole
	)
	/*++
	  Purpose:

	  Check if the player can attack all of the enemies in one move.

	  Parameters:

	  [IN]  wPlayerRole - the player role ID.

	  Return value:

	  TRUE if player can attack all of the enemies in one move, FALSE if not.

	  --*/
{
	int       i;
	BOOL      f;

	f = FALSE;

	for (i = 0; i <= MAX_PLAYER_EQUIPMENTS; i++)
	{
		if (gpGlobals->rgEquipmentEffect[i].rgwAttackAll[wPlayerRole] != 0)
		{
			f = TRUE;
			break;
		}
	}

	return f;
}

BOOL
PAL_AddMagic(
	WORD           wPlayerRole,
	WORD           wMagic
	)
	/*++
	  Purpose:

	  Add a magic to the player.

	  Parameters:

	  [IN]  wPlayerRole - the player role ID.

	  [IN]  wMagic - the object ID of the magic.

	  Return value:

	  TRUE if succeeded, FALSE if failed.

	  --*/
{
	int            i;

	for (i = 0; i < MAX_PLAYER_MAGICS; i++)
	{
		if (gpGlobals->g.PlayerRoles.rgwMagic[i][wPlayerRole] == wMagic)
		{
			//
			// already have this magic
			//
			return FALSE;
		}
	}

	for (i = 0; i < MAX_PLAYER_MAGICS; i++)
	{
		if (gpGlobals->g.PlayerRoles.rgwMagic[i][wPlayerRole] == 0)
		{
			break;
		}
	}

	if (i >= MAX_PLAYER_MAGICS)
	{
		//
		// Not enough slots
		//
		return FALSE;
	}

	gpGlobals->g.PlayerRoles.rgwMagic[i][wPlayerRole] = wMagic;
	return TRUE;
}

VOID
PAL_RemoveMagic(
	WORD           wPlayerRole,
	WORD           wMagic
	)
	/*++
	  Purpose:

	  Remove a magic to the player.

	  Parameters:

	  [IN]  wPlayerRole - the player role ID.

	  [IN]  wMagic - the object ID of the magic.

	  Return value:

	  None.

	  --*/
{
	int            i;

	for (i = 0; i < MAX_PLAYER_MAGICS; i++)
	{
		if (gpGlobals->g.PlayerRoles.rgwMagic[i][wPlayerRole] == wMagic)
		{
			gpGlobals->g.PlayerRoles.rgwMagic[i][wPlayerRole] = 0;
			break;
		}
	}
}

VOID
PAL_SetPlayerStatus(
	WORD         wPlayerRole,
	WORD         wStatusID,
	WORD         wNumRound
	)
	/*++
	  Purpose:

	  Set one of the statuses for the player.

	  Parameters:

	  [IN]  wPlayerRole - the player ID.

	  [IN]  wStatusID - the status to be set.

	  [IN]  wNumRound - the effective rounds of the status.

	  Return value:

	  None.

	  --*/
{
#ifndef PAL_CLASSIC
	if (wStatusID == kStatusSlow &&
		gpGlobals->rgPlayerStatus[wPlayerRole][kStatusHaste] > 0)
	{
		//
		// Remove the haste status
		//
		PAL_RemovePlayerStatus(wPlayerRole, kStatusHaste);
		return;
	}

	if (wStatusID == kStatusHaste &&
		gpGlobals->rgPlayerStatus[wPlayerRole][kStatusSlow] > 0)
	{
		//
		// Remove the slow status
		//
		PAL_RemovePlayerStatus(wPlayerRole, kStatusSlow);
		return;
	}
#endif

	switch (wStatusID)
	{
		case kStatusConfused:
		case kStatusSleep:
		case kStatusSilence:
#ifdef PAL_CLASSIC
		case kStatusParalyzed:
#else
		case kStatusSlow:
#endif
			//
			// for "bad" statuses, don't set the status when we already have it
			//
			if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] != 0 &&
				gpGlobals->rgPlayerStatus[wPlayerRole][wStatusID] == 0)
			{
				gpGlobals->rgPlayerStatus[wPlayerRole][wStatusID] = wNumRound;
			}
			break;

		case kStatusPuppet:
			//
			// only allow dead players for "puppet" status
			//
			if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0 &&
				gpGlobals->rgPlayerStatus[wPlayerRole][wStatusID] < wNumRound)
			{
				gpGlobals->rgPlayerStatus[wPlayerRole][wStatusID] = wNumRound;
			}
			break;

		case kStatusBravery:
		case kStatusProtect:
		case kStatusDualAttack:
		case kStatusHaste:
			//
			// for "good" statuses, reset the status if the status to be set lasts longer
			//
			if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] != 0 &&
				gpGlobals->rgPlayerStatus[wPlayerRole][wStatusID] < wNumRound)
			{
				gpGlobals->rgPlayerStatus[wPlayerRole][wStatusID] = wNumRound;
			}
			else if (gpGlobals->g.PlayerRoles.rgwHP[wPlayerRole] == 0 &&
				gpGlobals->rgPlayerStatus[wPlayerRole][kStatusPuppet] != 0 &&
				gpGlobals->rgPlayerStatus[wPlayerRole][wStatusID] < wNumRound)
			{
				if (wStatusID == kStatusBravery || wStatusID == kStatusDualAttack)
				{
					gpGlobals->rgPlayerStatus[wPlayerRole][wStatusID] = wNumRound;
				}
			}
			break;

		default:
			assert(FALSE);
			break;
	}
}

VOID
PAL_RemovePlayerStatus(
	WORD         wPlayerRole,
	WORD         wStatusID
	)
	/*++
	  Purpose:    Remove one of the status for player.
	  对角色解除某一个状态
	  Parameters:	[IN]  wPlayerRole - the player ID.
	  [IN]  wStatusID - the status to be set.
	  Return value:    None.
	  --*/
{
	//
	// Don't remove effects of equipments
	//
	if (gpGlobals->rgPlayerStatus[wPlayerRole][wStatusID] <= 999)
	{
		gpGlobals->rgPlayerStatus[wPlayerRole][wStatusID] = 0;
	}
}

VOID
PAL_New_RemovePlayerAllStatus(
	WORD         wPlayerRole
	)
{
	int x;
	for (x = 0; x < kStatusAll; x++)
	{
		PAL_RemovePlayerStatus(wPlayerRole, x);
	}
}


VOID
PAL_ClearAllPlayerStatus(
	VOID
	)
	/*++
	  Purpose:    Clear all player status.
	  清除所有角色的状态
	  Parameters:    None.
	  Return value:    None.
	  --*/
{
	int      i, j;

	for (i = 0; i < MAX_PLAYER_ROLES; i++)
	{
		for (j = 0; j < kStatusAll; j++)
		{
			//
			// Don't remove effects of equipments
			//
			if (gpGlobals->rgPlayerStatus[i][j] <= 999)
			{
				gpGlobals->rgPlayerStatus[i][j] = 0;
			}
		}
	}
}

VOID
PAL_PlayerLevelUp(
	WORD          wPlayerRole,
	WORD          wNumLevel
	)
	/*++
	  Purpose:

	  Increase the player's level by wLevels.

	  Parameters:

	  [IN]  wPlayerRole - player role ID.

	  [IN]  wNumLevel - number of levels to be increased.

	  Return value:

	  None.

	  --*/
{
	WORD          i;

	// Add the level
	gpGlobals->g.PlayerRoles.rgwLevel[wPlayerRole] += wNumLevel;
	if (gpGlobals->g.PlayerRoles.rgwLevel[wPlayerRole] > MAX_LEVELS
#ifdef CAN_BEYOND_MAX_PARAMETER_SOMETIMES
		&& gpGlobals->g.PlayerRoles.rgwCanBeyondMaxPara[wPlayerRole] == 0
#endif
		)
	{
		gpGlobals->g.PlayerRoles.rgwLevel[wPlayerRole] = MAX_LEVELS;
	}

	for (i = 0; i < wNumLevel; i++)
	{
		//
		// Increase player's stats
		//
		gpGlobals->g.PlayerRoles.rgwMaxHP[wPlayerRole] += 10 + RandomLong(0, 4);
		gpGlobals->g.PlayerRoles.rgwMaxMP[wPlayerRole] += 6 + RandomLong(0, 6);
		gpGlobals->g.PlayerRoles.rgwAttackStrength[wPlayerRole] += 4 + RandomLong(0, 1);
		gpGlobals->g.PlayerRoles.rgwMagicStrength[wPlayerRole] += 4 + RandomLong(0, 1);
		gpGlobals->g.PlayerRoles.rgwDefense[wPlayerRole] += 2 + RandomLong(0, 1);
		gpGlobals->g.PlayerRoles.rgwDexterity[wPlayerRole] += 2 + RandomLong(0, 1);
		gpGlobals->g.PlayerRoles.rgwFleeRate[wPlayerRole] += 2;
	}

#ifdef CAN_BEYOND_MAX_PARAMETER_SOMETIMES
	if (gpGlobals->g.PlayerRoles.rgwCanBeyondMaxPara[wPlayerRole] == 0)
	{
#endif

#define STAT_LIMIT(t) { if ((t) > MAX_PARAMETER) (t) = MAX_PARAMETER; }
		STAT_LIMIT(gpGlobals->g.PlayerRoles.rgwMaxHP[wPlayerRole]);
		STAT_LIMIT(gpGlobals->g.PlayerRoles.rgwMaxMP[wPlayerRole]);
		STAT_LIMIT(gpGlobals->g.PlayerRoles.rgwAttackStrength[wPlayerRole]);
		STAT_LIMIT(gpGlobals->g.PlayerRoles.rgwMagicStrength[wPlayerRole]);
		STAT_LIMIT(gpGlobals->g.PlayerRoles.rgwDefense[wPlayerRole]);
		STAT_LIMIT(gpGlobals->g.PlayerRoles.rgwDexterity[wPlayerRole]);
		STAT_LIMIT(gpGlobals->g.PlayerRoles.rgwFleeRate[wPlayerRole]);
#undef STAT_LIMIT

#ifdef CAN_BEYOND_MAX_PARAMETER_SOMETIMES
	}
#endif


	//
	// Reset experience points to zero
	//
	gpGlobals->Exp.rgPrimaryExp[wPlayerRole].wExp = 0;
	gpGlobals->Exp.rgPrimaryExp[wPlayerRole].wLevel =
		gpGlobals->g.PlayerRoles.rgwLevel[wPlayerRole];
}

#ifdef FINISH_GAME_MORE_ONE_TIME
VOID
PAL_New_GoBackAndLoadDefaultGame(
	VOID
	)
	/*++
	Purpose:穿越回去，除了主角的各项属性，其余的都恢复到最初的数据
	--*/
{
	GAMEDATA    *p = &gpGlobals->g;
	UINT32             i;
	PLAYERROLES playerRoles;

	memset(&playerRoles, 0, sizeof(playerRoles));
	//
	// Load the default data from the game data files.
	//
	LOAD_DATA(p->lprgEventObject, p->nEventObject * sizeof(EVENTOBJECT),
		0, gpGlobals->f.fpSSS);
	PAL_MKFReadChunk((LPBYTE)(p->rgScene), sizeof(p->rgScene), 1, gpGlobals->f.fpSSS);
	DO_BYTESWAP(p->rgScene, sizeof(p->rgScene));
	PAL_MKFReadChunk((LPBYTE)(p->rgObject), sizeof(p->rgObject), 2, gpGlobals->f.fpSSS);
	DO_BYTESWAP(p->rgObject, sizeof(p->rgObject));

	PAL_MKFReadChunk((LPBYTE)(&playerRoles), sizeof(PLAYERROLES), 3, gpGlobals->f.fpDATA);
	DO_BYTESWAP(&(playerRoles), sizeof(PLAYERROLES));

	// Set some other default data.
	gpGlobals->wNumMusic = 0;
	gpGlobals->wNumPalette = 0;
	gpGlobals->wNumScene = 1;
	gpGlobals->fNightPalette = FALSE;
	gpGlobals->wMaxPartyMemberIndex = 0;
	gpGlobals->viewport = PAL_XY(0, 0);
	gpGlobals->wLayer = 0;
	gpGlobals->wChaseRange = 1;

#ifndef PAL_CLASSIC
	gpGlobals->bBattleSpeed = 2;
#endif

	gpGlobals->bFinishGameTime++;

	memset(gpGlobals->rgPoisonStatus, 0, sizeof(gpGlobals->rgPoisonStatus));
	memset(gpGlobals->rgParty, 0, sizeof(gpGlobals->rgParty));
	memset(gpGlobals->rgTrail, 0, sizeof(gpGlobals->rgTrail));
	//memset(&(gpGlobals->Exp), 0, sizeof(gpGlobals->Exp));

	for (i = 0; i < MAX_PLAYER_ROLES; i++)
	{
		gpGlobals->Exp.rgPrimaryExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		gpGlobals->Exp.rgHealthExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		gpGlobals->Exp.rgMagicExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		gpGlobals->Exp.rgAttackExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		gpGlobals->Exp.rgMagicPowerExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		gpGlobals->Exp.rgDefenseExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		gpGlobals->Exp.rgDexterityExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
		gpGlobals->Exp.rgFleeExp[i].wLevel = p->PlayerRoles.rgwLevel[i];
	}

#ifdef FINISH_GAME_MORE_ONE_TIME

#ifdef CAN_BEYOND_MAX_PARAMETER_SOMETIMES
	gpGlobals->g.PlayerRoles.rgwCanBeyondMaxPara[RoleID_GaiLuoJiao] = 1;
	gpGlobals->g.PlayerRoles.rgwCanBeyondMaxPara[RoleID_WuHou] = 1;
#endif

	switch (gpGlobals->bFinishGameTime)
	{
		case 0:
		case 1:
		{
			gpGlobals->dwCash = 0;
			gpGlobals->wCollectValue = 0;
			memset(gpGlobals->rgInventory, 0, sizeof(gpGlobals->rgInventory));
			// 此处还应去除每个人的绝技，例如酒神、剑灭、梦蛇、满天花雨、无心咒等
			// 还原佩戴的物品
			for (i = 0; i < MAX_PLAYER_ROLES; i++)
			{
				gpGlobals->g.PlayerRoles.rgwEquipment[kBodyPartWear][i] = playerRoles.rgwEquipment[kBodyPartWear][i];
			}
#ifdef CAN_BEYOND_MAX_PARAMETER_SOMETIMES
			gpGlobals->g.PlayerRoles.rgwCanBeyondMaxPara[RoleID_LinYueRu] = 1;
			gpGlobals->g.PlayerRoles.rgwCanBeyondMaxPara[RoleID_ANu] = 1;
#endif
			break;
		}

		default:
		{
#ifdef CAN_BEYOND_MAX_PARAMETER_SOMETIMES
			gpGlobals->g.PlayerRoles.rgwCanBeyondMaxPara[RoleID_LiXiaoYao] = 1;
			gpGlobals->g.PlayerRoles.rgwCanBeyondMaxPara[RoleID_ZhaoLingEr] = 1;
			gpGlobals->g.PlayerRoles.rgwCanBeyondMaxPara[RoleID_LinYueRu] = 1;
			gpGlobals->g.PlayerRoles.rgwCanBeyondMaxPara[RoleID_ANu] = 1;
#endif
			break;
		}
	}
#endif

	PAL_PlayerLevelUp(RoleID_ZhaoLingEr, 4);
	PAL_PlayerLevelUp(RoleID_LinYueRu, 6);
	PAL_PlayerLevelUp(RoleID_ANu, 45);
	PAL_PlayerLevelUp(RoleID_WuHou, 80);
	PAL_PlayerLevelUp(RoleID_GaiLuoJiao, 80);

	for (i = 0; i < MAX_PLAYER_ROLES; i++)
	{
		gpGlobals->g.PlayerRoles.rgwAvatar[i] = playerRoles.rgwAvatar[i];
		gpGlobals->g.PlayerRoles.rgwSpriteNumInBattle[i] = playerRoles.rgwSpriteNumInBattle[i];
		gpGlobals->g.PlayerRoles.rgwSpriteNum[i] = playerRoles.rgwSpriteNum[i];
		gpGlobals->g.PlayerRoles.rgwWalkFrames[i] = playerRoles.rgwWalkFrames[i];
		gpGlobals->g.PlayerRoles.rgwHP[i] = gpGlobals->g.PlayerRoles.rgwMaxHP[i];
		gpGlobals->g.PlayerRoles.rgwMP[i] = gpGlobals->g.PlayerRoles.rgwMaxMP[i];
	}

	gpGlobals->fEnteringScene = TRUE;
}
#endif

INT
PAL_New_GetPlayerIndex(
	WORD		wPlayerRole
	)
{
	int		i;

	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		if (wPlayerRole == gpGlobals->rgParty[i].wPlayerRole)
		{
			break;
		}
	}

	if (i > gpGlobals->wMaxPartyMemberIndex)
	{
		return -1;//没有找到
	}
	else
	{
		return i;
	}


}

WORD
PAL_New_GetPlayerID(
	WORD		wPlayerIndex
	)
{
	if (wPlayerIndex > MAX_PLAYERS_IN_PARTY)
	{
		return 0xFFFF;
	}
	else
	{
		return gpGlobals->rgParty[wPlayerIndex].wPlayerRole;
	}
}

//fGetLowest为真找属性最低的角色序号，bLow为假找属性最高的角色序号，
INT
PAL_New_GetPlayerIndexByPara(
	PlayerPara	pp,
	BOOL		fGetLowest)
{
	WORD *p = (WORD *)(&gpGlobals->g.PlayerRoles);
	WORD *p1[MAX_PLAYER_EQUIPMENTS + 1] =
	{
		(WORD *)(&gpGlobals->rgEquipmentEffect[0]),
		(WORD *)(&gpGlobals->rgEquipmentEffect[1]),
		(WORD *)(&gpGlobals->rgEquipmentEffect[2]),
		(WORD *)(&gpGlobals->rgEquipmentEffect[3]),
		(WORD *)(&gpGlobals->rgEquipmentEffect[4]),
		(WORD *)(&gpGlobals->rgEquipmentEffect[5]),
		(WORD *)(&gpGlobals->rgEquipmentEffect[6]),
	};
	int i, j, max, maxIndex, min, minIndex, cur;
	WORD w = 0;

	w = gpGlobals->rgParty[0].wPlayerRole;
	cur = p[pp * MAX_PLAYER_ROLES + w];
	for (i = 0; i <= MAX_PLAYER_EQUIPMENTS; i++)
	{
		cur += p1[i][pp * MAX_PLAYER_ROLES + w];
	}
	min = cur;
	minIndex = 0;
	max = cur;
	maxIndex = 0;

	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		w = gpGlobals->rgParty[i].wPlayerRole;
		cur = p[pp * MAX_PLAYER_ROLES + w];
		for (j = 0; j <= MAX_PLAYER_EQUIPMENTS; j++)
		{
			cur += p1[j][pp * MAX_PLAYER_ROLES + w];
		}

		if (max < cur)
		{
			max = cur;
			maxIndex = i;
		}
		if (min > cur)
		{
			min = cur;
			minIndex = i;
		}
	}

	if (fGetLowest == TRUE)
	{
		return minIndex;
	}
	else
	{
		return maxIndex;
	}
}

INT
PAL_New_GetPlayerIndexByHealth(
	BOOL		fGetLowest
	)
	/*++
	功能：    选中生命值最低（高）的角色.
	参数：    fLow为真是选择最低，反之选择最高.
	返回值：   目标序号.
	--*/
{
	int i;
	int maxHP = 0;
	int maxHPIndex = 0;
	int minHP = 0;
	int minHPIndex = 0;
	int curHP = 0;
	WORD w = 0;

	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		w = gpGlobals->rgParty[i].wPlayerRole;
		curHP = gpGlobals->g.PlayerRoles.rgwHP[w];

		if (curHP != 0)
		{
			minHP = curHP;
			minHPIndex = i;
			maxHP = curHP;
			maxHPIndex = i;
			break;
		}
	}

	for (i = 0; i <= gpGlobals->wMaxPartyMemberIndex; i++)
	{
		w = gpGlobals->rgParty[i].wPlayerRole;
		curHP = gpGlobals->g.PlayerRoles.rgwHP[w];
		if (curHP == 0)
		{
			continue;
		}
		if (maxHP < curHP && PAL_New_IfPlayerCanMove(w))
		{
			maxHP = curHP;
			maxHPIndex = i;
		}
		if (minHP > curHP)
		{
			minHP = curHP;
			minHPIndex = i;
		}
	}

	if (fGetLowest == TRUE)
	{
		return minHPIndex;
	}
	else
	{
		return maxHPIndex;
	}
}

BOOL
PAL_New_GetTrueByPercentage(
	WORD		wPercentage
	)
	/*++
	Purpose:根据百分比返回真值

	Parameters:	[IN]  wPercentage - 百分数

	Return value:有（输入%）的可能返回真值，其余返回假
	--*/
{
	if (RandomLong(0, 99) < wPercentage)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

WORD
PAL_New_GetLevelUpBaseExp(
	WORD		wLevel
	)
{
	WORD wExp = 0;

	if (wLevel < 52)
	{
		wExp = 25 * wLevel * (wLevel - 1) / 2 + 15;
	}
	else
	{
		wExp = 32000;
	}
	return wExp;
}

DWORD
PAL_New_GetLevelUpExp(
	WORD		wLevel
	)
{
	WORD wTimes = wLevel / 100;
	WORD wRemainLevel = wLevel % 100;
	DWORD dwExp = wTimes * PAL_New_GetLevelUpBaseExp(100) + PAL_New_GetLevelUpBaseExp(wRemainLevel);
	return min(dwExp, 99999);
}