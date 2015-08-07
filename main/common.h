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

#ifndef _COMMON_H
#define _COMMON_H

#define PAL_CLASSIC        1

//主角队伍人数可以超过3个人
#define SHOW_MP_DECREASING							//当mp减少时也显示数值
//对于最大伤害值加以限制，防止溢出
#define MAX_DAMAGE							32000	//最大伤害值
#define ADD_MUSIC									//增加了歌曲
#define ADD_SOME_POISONS_SUCCESSFULLY_ANYTIME		//某些毒可以对任何人均命中（无视敌方巫抗或我方毒抗）
#define ADD_SOME_STATUSES_SUCCESSFULLY_ANYTIME		//为敌人添加状态总是成功（无视巫抗）
//对已经死亡的玩家不执行中毒脚本
#define ENEMY_MAX_MOVE						3		//敌人最多行动次数，如果为2则是经典版
#define MAX_POISON_LEVEL					99		//复活后解除的毒的最高等级，一般为装备技能，如寿葫芦
//复活后不解除寿葫芦作用
//在状态中显示寿葫芦
//当中毒数超过8个时，能完整显示所有
#define MAX_PARAMETER						1500	//武术、灵力、防御、身法、吉运、体力、真气的最大值，默认为999
#define MAX_PARAMETER_EXTRA					100		//属性值的最大额外值，战前检测，使用物品后检测。
//修正抗性的计算方式，使其能保留小数
//对于已经死亡的角色不再能使其中毒
//可以为敌人增加相同的状态（即延长回合,至多剩余两轮的时候有效）
//可以为傀儡虫操纵角色增加有益状态，如天罡和醉仙
//当角色死亡后，立即清除其所有状态
#define INCREASE_EXTRA_PLAYER_HPMP					//回复体力真气的时候，额外回复一定百分比（根据等级）
#define INCREASE_EXTRA_HPMP_MIN_LEVEL		50
#define INCREASE_EXTRA_HPMP_MAX_LEVEL		70
//当角色中多种毒的时候排序
//修正怪物变身后的战后脚本
#define ADD_POISON_FAIL_THEN_JUMP
#define INCREASE_ATTRIBUTE_AFTER_USE_MENGSHE		//灵儿使用梦蛇后，各项属性增加
//修正脚本命令0x0061，使用新函数，之前用的函数不能正确判断等级为0的毒，比如赤毒
//添加毒之前全面检查是否已经中该毒
//修正脚本命令0x0028，使用新函数，并且不提前执行中毒脚本
//修正脚本命令0x0029，改为调用函数，修正体力为0仍能吸取生命的bug
//修正脚本命令0x0023，修正移除装备的对象
#define EDIT_SCRIT_EVENTOBJECTID_WHEN_ITIS_0XFFFF	//在战斗中，如果使用物品无需选择目标，则脚本执行函数对象id由0xffff改为使用者id；为了解决梦幻版隐蛊隐身判断问题
#define EDIT_SCRIPT_OPERATION_0X0034				//当灵葫值很多的时候，批量炼制
//濒死的时候不在因疯魔状态攻击队友
//为贴近原作，修正某些语句的执行次序
//为了提高合理性，修改某些语句的执行次序

//1.1.0----14.02.16
#define DONOT_SELECT_TARGET_RANDOMLY_TOTALLY        //不完全是随机选择目标
#define STRENGTHEN_ENEMY							//在后期加强前期的敌人、队伍多于3人，敌人加强
#define EDIT_RECOVER_VALUE_AFTER_BATTLE             //修改战后恢复百分比（作者已写算法）
#define USE_LACKY_VALUE								//除了逃跑，使用吉运值
//夺魂有一定几率不是立刻死
#define EDIT_DAMAGE_CALC							//修改伤害计算公式（包括敌人的属性计算方式）
#define REMOVE_TEMPORARY_BASEDAMAGE_WHEN_SIMULATE_MAGIC //模拟仙术时移除暂时的基础伤害（例如满天花雨，酒神，投掷等仙术）
#define ENEMY_CAN_ESCAPE							//敌人可逃跑，概率10%左右
#define STRENGTHEN_PLAYER							//有特色的加强主角，灵儿初始五灵抗性20%，阿奴毒抗巫抗各30%，林月如额外恢复
#define CAN_BEYOND_MAX_PARAMETER_SOMETIMES			//有时候可以超过属性上限
#define SORT_INVENTORY								//物品栏排序
#define LIMIT_DAMAGE_WHEN_ATTACK_MATE				//攻击队友时限制伤害值
//#define ROTATE_SOME_MAGIC_ANIM					//旋转某些法术动画
#define GAIN_MORE_HIDDEN_EXP						//可以获得更多的隐藏经验
//修正多于3人时逃跑bug
//毒等级大于99级时，无视毒抗必中
//为角色增加巫抗属性，巫抗可以提高下毒巫术的成功率
//盖罗娇可以学习阿奴的法术
//修正有跟随者的时候的人物重叠问题
//复活后不解除忘剑五诀
//可以播放avi
#define SHOW_DATA_IN_BATTLE		// 在战斗中可以选择是否显示一些相关的数据（敌人血量、灵葫值，我方抗性、有益状态剩余轮次，总灵葫值）
//#define ENABLE_LOG

//增加额外对话

#define FINISH_GAME_MORE_ONE_TIME //可以多次完成游戏

#define POISON_STATUS_EXPAND
#define SHOW_ENEMY_STATUS
// 修改战后恢复，更多真气。敌人逃跑后恢复较多。月如额外恢复，物抗20

#define EDIT_EXP_CALCULATION //修改经验的计算方式

//#define EDIT_PRESS_KEY_EFFECT
#define USE_STRCPY_INSTEAD_OF_STRDUP
#define AVOID_SCRIPT_CRASH

#ifdef _WIN32
#define PAL_HAS_AVI		
//#define BIG_RESOLUTION //更大的分辨率
#endif

#ifdef _DEBUG
//#define INVINCIBLE				//无敌模式，受攻击后不减hp不受不良状态影响
//#define KO_ENEMY_FAST
#endif // _DEBUG

//#define PAL_WIN95          1 // not valid for now
#define TITLE "SdlpalEdit - ver 1.1.2 - Build20150808"

typedef enum tagPlayerRoleID
{
	RoleID_LiXiaoYao		= 0,
	RoleID_ZhaoLingEr		= 1,  
	RoleID_LinYueRu			= 2,  
	RoleID_WuHou			= 3,  
	RoleID_ANu				= 4,  
	RoleID_GaiLuoJiao		= 5,  
} PlayerRoleID;


/*
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
1C：巫抗
*/
typedef enum tagPlayerPara
{
	Para_Level = 0x0006,
	Para_MaxHP,
	Para_MaxMP,
	Para_HP,
	Para_MP,
	Para_AttackStrength = 0x0011,
	Para_MagicStrength,
	Para_Defense,
	Para_Dexterity,
	Para_FleeRate,
	Para_PoisonResistance = 0x0016,
	Para_CloudResistance = 0x0017,
	Para_ThunderResistance,
	Para_WaterResistance,
	Para_FireResistance,
	Para_EarthResistance,
	Para_SorceryResistance = 0x001c,
} PlayerPara;

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdarg.h>
#include <assert.h>

#include "SDL.h"
#include "SDL_endian.h"

#ifdef _SDL_stdinc_h
#define malloc       SDL_malloc
#define calloc       SDL_calloc
#define free         SDL_free
#define realloc      SDL_realloc
#endif

#if SDL_VERSION_ATLEAST(2,0,0)

#define SDLK_KP1     SDLK_KP_1
#define SDLK_KP2     SDLK_KP_2
#define SDLK_KP3     SDLK_KP_3
#define SDLK_KP4     SDLK_KP_4
#define SDLK_KP5     SDLK_KP_5
#define SDLK_KP6     SDLK_KP_6
#define SDLK_KP7     SDLK_KP_7
#define SDLK_KP8     SDLK_KP_8
#define SDLK_KP9     SDLK_KP_9
#define SDLK_KP0     SDLK_KP_0

#define SDL_HWSURFACE     0

#endif

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define SWAP16(X)    (X)
#define SWAP32(X)    (X)
#else
#define SWAP16(X)    SDL_Swap16(X)
#define SWAP32(X)    SDL_Swap32(X)
#endif

#ifndef max
#define max(a, b)    (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b)    (((a) < (b)) ? (a) : (b))
#endif

#if defined (__SYMBIAN32__)

#undef  _WIN32
#undef  SDL_INIT_JOYSTICK
#define SDL_INIT_JOYSTICK     0
#define PAL_HAS_MOUSE         1
#define PAL_PREFIX            "e:/data/pal/"
#define PAL_SAVE_PREFIX       "e:/data/pal/"

#elif defined (GEKKO)

#define PAL_HAS_JOYSTICKS     1
#define PAL_HAS_MOUSE         0
#define PAL_PREFIX            "SD:/apps/sdlpal/"
#define PAL_SAVE_PREFIX       "SD:/apps/sdlpal/"

#elif defined (PSP)

#define PAL_HAS_JOYSTICKS     0
#define PAL_PREFIX            "ms0:/"
#define PAL_SAVE_PREFIX       "ms0:/PSP/SAVEDATA/SDLPAL/"

#elif defined (__IOS__)

#define PAL_PREFIX            UTIL_IOS_BasePath()
#define PAL_SAVE_PREFIX       UTIL_IOS_SavePath()
#define PAL_HAS_TOUCH         1

#elif defined (__ANDROID__)

#define PAL_PREFIX            "/mnt/sdcard/sdlpal/"
#define PAL_SAVE_PREFIX       "/mnt/sdcard/sdlpal/"
#define PAL_HAS_TOUCH         1

#elif defined (__WINPHONE__)

#define PAL_PREFIX            "Assets\\Data\\"
#define PAL_SAVE_PREFIX       "" // ???
#define PAL_HAS_TOUCH         1
#include <stdio.h>
#ifdef __cplusplus
#include <cstdio>
#endif

FILE *MY_fopen(const char *path, const char *mode);
#define fopen MY_fopen

#else
#define PAL_HAS_JOYSTICKS     1
#ifndef _WIN32_WCE
#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION <= 2
#define PAL_ALLOW_KEYREPEAT   1
#define PAL_HAS_CD            1
#endif
#if !defined (CYGWIN) && !defined (DINGOO) && !defined (GPH) && !defined (GEKKO) && !defined (__WINPHONE__)
#define PAL_HAS_MP3           1
#endif
#endif
#ifndef PAL_PREFIX
#define PAL_PREFIX            "./pal/"
#endif
#ifndef PAL_SAVE_PREFIX
#define PAL_SAVE_PREFIX       "./pal/"
#endif

#endif

#ifndef SDL_INIT_CDROM
#define SDL_INIT_CDROM        0
#endif

#ifdef _WIN32

#include <windows.h>

#if !defined(__BORLANDC__) && !defined(_WIN32_WCE)
#include <io.h>
#endif

#define vsnprintf _vsnprintf

#ifdef _MSC_VER
#pragma warning (disable:4018)
#pragma warning (disable:4028)
#pragma warning (disable:4244)
#pragma warning (disable:4305)
#pragma warning (disable:4761)
#pragma warning (disable:4996)
#endif

#ifndef _LPCBYTE_DEFINED
#define _LPCBYTE_DEFINED
typedef const BYTE *LPCBYTE;
#endif

#ifndef __WINPHONE__
#define PAL_HAS_NATIVEMIDI  1
#endif

#else

#include <unistd.h>

#define CONST               const
#ifndef FALSE
#define FALSE               0
#endif
#ifndef TRUE
#define TRUE                1
#endif
#define VOID                void
typedef char                CHAR;
typedef short               SHORT;
typedef long                LONG;

typedef unsigned long       ULONG, *PULONG;
typedef unsigned short      USHORT, *PUSHORT;
typedef unsigned char       UCHAR, *PUCHAR;

typedef unsigned short      WORD, *LPWORD;
typedef unsigned int        DWORD, *LPDWORD;
typedef int                 INT, *LPINT;
#ifndef __OBJC__
typedef int                 BOOL, *LPBOOL;
#endif
typedef unsigned int        UINT, *PUINT, UINT32, *PUINT32;
typedef unsigned char       BYTE, *LPBYTE;
typedef CONST BYTE         *LPCBYTE;
typedef float               FLOAT, *LPFLOAT;
typedef void               *LPVOID;
typedef const void         *LPCVOID;
typedef CHAR               *LPSTR;
typedef const CHAR         *LPCSTR;

#endif

#if defined (__SYMBIAN32__)
#define PAL_LARGE           static
#else
#define PAL_LARGE           /* */
#endif

#ifdef __cplusplus
}
#endif

#endif
