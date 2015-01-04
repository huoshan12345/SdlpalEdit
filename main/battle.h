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

#ifndef BATTLE_H
#define BATTLE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "global.h"
#include "uibattle.h"



#define       BATTLE_FPS               25
#define       BATTLE_FRAME_TIME        (1000 / BATTLE_FPS)

typedef enum tagBATTLERESULT
{
	kBattleResultWon = 3,			// player won the battle
	kBattleResultLost = 1,			// player lost the battle
	kBattleResultEnemyFleed = 0x7FFF,// 敌人逃跑
	kBattleResultFleed = 0xFFFF,	// player fleed from the battle
	kBattleResultTerminated = 0,	// battle terminated with scripts
	kBattleResultOnGoing = 1000,	// the battle is ongoing
	kBattleResultPreBattle = 1001,	// running pre-battle scripts
	kBattleResultPause = 1002,		// battle pause
	
} BATTLERESULT;

typedef enum tagFIGHTERSTATE
{
	kFighterWait,  // waiting time
	kFighterCom,   // accepting command
	kFighterAct,   // doing the actual move
} FIGHTERSTATE;

typedef enum tagBATTLEACTIONTYPE
{
	kBattleActionPass,          // do nothing						无动作
	kBattleActionDefend,        // defend							防御
	kBattleActionAttack,        // physical attack					物攻
	kBattleActionMagic,         // use magic							使用法术
	kBattleActionCoopMagic,     // use cooperative magic				使用合体法术
	kBattleActionFlee,          // flee from the battle				逃跑
	kBattleActionThrowItem,     // throw item onto enemy				投掷物品
	kBattleActionUseItem,       // use item							使用物品	
	kBattleActionAttackMate,    // attack teammate (confused only)	攻击队友（疯魔状态下）
} BATTLEACTIONTYPE;

typedef struct tagBATTLEACTION
{
	BATTLEACTIONTYPE   ActionType;
	WORD               wActionID;   // item/magic to use
	SHORT              sTarget;     // -1 for everyone
	FLOAT              flRemainingTime;  // remaining waiting time before the action start
} BATTLEACTION;

typedef struct tagBATTLEENEMY
{
	WORD               wObjectID;              // Object ID of this enemy
	ENEMY              e;                      // detailed data of this enemy
	WORD               rgwStatus[kStatusAll];  // status effects
	FLOAT              flTimeMeter;            // time-charging meter (0 = empty, 100 = full).
	POISONSTATUS       rgPoisons[MAX_POISONS]; // poisons
	LPSPRITE           lpSprite;
	PAL_POS            pos;                    // current position on the screen
	PAL_POS            posOriginal;            // original position on the screen
	WORD               wCurrentFrame;          // current frame number
	FIGHTERSTATE       state;                  // state of this enemy
	BOOL               fTurnStart;
	BOOL               fFirstMoveDone;
	BOOL               fDualMove;
	WORD               wScriptOnTurnStart;	//战前脚本,一般为战前的对话
	WORD               wScriptOnBattleEnd;	//战后脚本，例如获得某某物品
	WORD               wScriptOnReady;		//战时脚本，一般为敌人的出招脚本
	DWORD              wPrevHP;				// HP value prior to action（改成DWORD类型了）
	INT                iColorShift;

	DWORD              dwActualHealth;
	DWORD              dwMaxHealth;
} BATTLEENEMY;

// We only put some data used in battle here; other data can be accessed in the global data.
typedef struct tagBATTLEPLAYER
{
	INT                iColorShift;
	FLOAT              flTimeMeter;          // time-charging meter (0 = empty, 100 = full).
	FLOAT              flTimeSpeedModifier;
	WORD               wHidingTime;          // remaining hiding time
	LPSPRITE           lpSprite;
	PAL_POS            pos;                  // current position on the screen
	PAL_POS            posOriginal;          // original position on the screen
	WORD               wCurrentFrame;        // current frame number
	FIGHTERSTATE       state;                // state of this player
	BATTLEACTION       action;               // action to perform
	BOOL               fDefending;           // TRUE if player is defending
	WORD               wPrevHP;              // HP value prior to action
	WORD               wPrevMP;              // MP value prior to action
#ifndef PAL_CLASSIC
	SHORT              sTurnOrder;           // turn order
#endif

#ifdef EDIT_EXP_CALCULATION
	DWORD				dwExpGained;
#endif

} BATTLEPLAYER;

typedef struct tagSUMMON
{
	LPSPRITE           lpSprite;
	WORD               wCurrentFrame;
} SUMMON;

#define MAX_BATTLE_ACTIONS    256
#define MAX_KILLED_ENEMIES    256

#ifdef PAL_CLASSIC

typedef enum tabBATTLEPHASE
{
	kBattlePhaseSelectAction,
	kBattlePhasePerformAction
} BATTLEPHASE;

typedef enum tabACTIONQUEUETYPE
{
	kActionQueueTypeEmpty,
	kActionQueueTypeEnemy,
	kActionQueueTypePlayer,
} ACTIONQUEUETYPE;

typedef struct tagACTIONQUEUE
{
	BOOL		fIsEmpty;
	BOOL       fIsEnemy;
	DWORD       wDexterity;
	WORD       wIndex;
} ACTIONQUEUE;

#define MAX_ACTIONQUEUE_ITEMS (MAX_PLAYERS_IN_PARTY + MAX_ENEMIES_IN_TEAM * ENEMY_MAX_MOVE)

#endif

typedef struct tagMENGSHECOUNT
{
	WORD			MaxHP;
	WORD			MaxMP;
	WORD			AttackStrength;
	WORD			MagicStrength;
	WORD			Defense;
	WORD			Dexterity;
	WORD			FleeRate;
} MENGSHECOUNT;

typedef struct tagBATTLE
{
	BATTLEPLAYER     rgPlayer[MAX_PLAYERS_IN_PARTY];
	BATTLEENEMY      rgEnemy[MAX_ENEMIES_IN_TEAM];

	WORD             wMaxEnemyIndex;

	SDL_Surface     *lpSceneBuf;			//战斗画面的图层，包括角色、敌人
	SDL_Surface     *lpBackground;			//战斗中地图画面的图层

	SHORT            sBackgroundColorShift;

	LPSPRITE         lpSummonSprite;       // sprite of summoned god
	PAL_POS          posSummon;
	INT              iSummonFrame;         // current frame of the summoned god

	INT              iExpGained;           // total experience value gained
	INT              iCashGained;          // total cash gained

	BOOL             fIsBoss;              // TRUE if boss fight
	BOOL             fEnemyCleared;        // TRUE if enemies are cleared
	BATTLERESULT     BattleResult;

	FLOAT            flTimeChargingUnit;   // the base waiting time unit

	BATTLEUI         UI;

	LPBYTE           lpEffectSprite;

	BOOL             fEnemyMoving;         // TRUE if enemy is moving

	BOOL             fPlayerMoving;         // TRUE if player is moving

	INT              iHidingTime;          // Time of hiding

	WORD             wMovingPlayerIndex;   // current moving player index

	WORD             wMovingEnemyIndex;   // new, current moving Enemy index

	int              iBlow;

#ifdef PAL_CLASSIC
	BATTLEPHASE      Phase;
	ACTIONQUEUE      ActionQueue[MAX_ACTIONQUEUE_ITEMS];
	int              iCurAction;
	BOOL             fRepeat;              // TRUE if player pressed Repeat
	BOOL             fForce;               // TRUE if player pressed Force
	BOOL             fFlee;                // TRUE if player pressed Flee
#endif

#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
	MENGSHECOUNT		MSCount_Zhao;
#endif

} BATTLE;

extern BATTLE g_Battle;

VOID PAL_LoadBattleSprites(VOID);

VOID PAL_BattleMakeScene(VOID);

VOID PAL_BattleBackupScene(VOID);

VOID PAL_BattleFadeScene(VOID);

VOID PAL_BattleEnemyEscape(VOID);

VOID PAL_BattlePlayerEscape(VOID);

VOID PAL_BattleWon(VOID);

BATTLERESULT PAL_StartBattle(WORD wEnemyTeam, BOOL fIsBoss);

VOID
Pal_New_RecoverAfterBattle(
WORD		wPlayerRole,
WORD		wPercent
);

#ifdef INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE
BOOL PAL_New_IsPlayerUsingMengshe(WORD wPlayerRole);

VOID PAL_New_CheckMengsheAttriIncre(WORD wPlayerRole, MENGSHECOUNT msCount);

DWORD PAL_New_GetMSTotalCount(MENGSHECOUNT msCount);
#endif

#ifdef STRENGTHEN_ENEMY
BATTLEENEMY  PAL_New_StrengthenEnemy(BATTLEENEMY be);
#endif

#ifdef __cplusplus
}
#endif

#endif
