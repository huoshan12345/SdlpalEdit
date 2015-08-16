#include "main.h"
#include <math.h>

VOID
PAL_New_DecreaseHPForEnemy(
WORD			wEnemyIndex,
INT				iDamage
)
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return;
	}

	if (iDamage > 0)
	{   //使用min比较不同类型的正负数的时候有时候会不符合预期
		iDamage = min(iDamage, g_Battle.rgEnemy[wEnemyIndex].dwActualHealth);
	}
	g_Battle.rgEnemy[wEnemyIndex].dwActualHealth -= iDamage;

	g_Battle.rgEnemy[wEnemyIndex].dwMaxHealth = max(g_Battle.rgEnemy[wEnemyIndex].dwMaxHealth, 
		g_Battle.rgEnemy[wEnemyIndex].dwActualHealth);
}


VOID
PAL_New_CurePoisonForEnemyByKind(
WORD		wEnemyIndex,
WORD		wPoisonID
)
{
	int j;

	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return;
	}

	for (j = 0; j < MAX_POISONS; j++)
	{
		if (g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID == wPoisonID)
		{
#ifdef POISON_STATUS_EXPAND
			memset(&g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j], 0, sizeof(g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j]));
#else
			g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID = 0;
			g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonScript = 0;
#endif
			break;
		}
	}
}

VOID
PAL_New_CurePoisonForEnemyByLevel(
WORD		wEnemyIndex,
WORD		wMaxLevel
)
{
	int		j;
	WORD       w;

	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return;
	}

	for (j = 0; j < MAX_POISONS; j++)
	{
		w = g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID;
		if (gpGlobals->g.rgObject[w].poison.wPoisonLevel <= wMaxLevel)
		{

#ifdef POISON_STATUS_EXPAND
			memset(&g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j], 0, sizeof(g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j]));
#else
			g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID = 0;
			g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonScript = 0;
#endif
			break;
		}
	}
}


INT
PAL_New_GetPoisonIndexForEnemy(
WORD		wEnemyIndex,
WORD		wPoisonID
)
{
	int i;

	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	for (i = 0; i < MAX_POISONS; i++)
	{
		if (g_Battle.rgEnemy[wEnemyIndex].rgPoisons[i].wPoisonID == wPoisonID)
		{
			return i;
		}
	}
	return -1;
}

VOID 
PAL_New_AddPoisonForEnemy(
WORD		wEnemyIndex, 
WORD		wPoisonID
)
/*++
Purpose:   	对敌方增加指定的毒
Parameters:    [IN]  wEnemyIndex - 敌人在队伍中的序号.
[IN]  wPoisonID - the poison to be added.
Return value:    None.
注：不可以在该函数内进行毒排序，会造成脚本执行混乱
--*/
{
	int         j;
	WORD        w;

	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return;
	}

	for (j = 0; j < MAX_POISONS; j++)
	{
		if (g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID == wPoisonID)
		{
#ifdef POISON_STATUS_EXPAND // 增加毒的烈度
			INT iSuccessRate = 100;
			WORD wPoisonLevel = gpGlobals->g.rgObject[wPoisonID].poison.wPoisonLevel;
			iSuccessRate -= wPoisonLevel * 20;
			iSuccessRate = max(iSuccessRate, 0);
			iSuccessRate *= 2;
			if (PAL_New_GetTrueByPercentage(iSuccessRate))
			{
				g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonIntensity++;
			}
			switch (wPoisonLevel)
			{
				case 0:
				case 1:
				case 2:
					g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonIntensity =
						min(g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonIntensity, 4);
					break;

				case 3:
					g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonIntensity =
						min(g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonIntensity, 2);

				default:
					g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonIntensity = 
						min(g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonIntensity, 1);
					break;
			}
#endif
			return;
		}
	}

	for (j = 0; j < MAX_POISONS; j++)
	{
		w = g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID;
		if (w == 0)
		{
			break;
		}
	}

	j = min(j, MAX_POISONS - 1);
	g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID = wPoisonID;
	g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonScript = gpGlobals->g.rgObject[wPoisonID].poison.wEnemyScript;

#ifdef POISON_STATUS_EXPAND // 增加毒的烈度
	g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonIntensity = 0;
#endif
}


VOID 
PAL_New_SortPoisonsForEnemyByLevel(
WORD		wEnemyIndex
)
{
	int         i, j, PoisonNum;
	WORD        wPoisonID1, wPoisonID2;
	WORD        wPoisonLevel1, wPoisonLevel2;
	POISONSTATUS	tempPoison;

	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return;
	}

	for (j = 0, PoisonNum = 0; j < MAX_POISONS; j++)
	{
		wPoisonID1 = g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID;
		if (wPoisonID1 == 0)
		{
			g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID = 0;
		}
		else
		{
			PoisonNum++;
		}
	}

	if (PoisonNum < 2)		//中毒数目小于2不用排序
	{
		return;
	}

	for (i = 0; i < MAX_POISONS - 1; i++)
	{
		for (j = 0; j < MAX_POISONS - i - 1; j++)
		{
			wPoisonID1 = g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j].wPoisonID;
			wPoisonLevel1 = gpGlobals->g.rgObject[wPoisonID1].poison.wPoisonLevel;
			wPoisonID2 = g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j + 1].wPoisonID;
			wPoisonLevel2 = gpGlobals->g.rgObject[wPoisonID2].poison.wPoisonLevel;

			if (wPoisonLevel1 < wPoisonLevel2)
			{
				tempPoison = g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j];
				g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j] = g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j + 1];
				g_Battle.rgEnemy[wEnemyIndex].rgPoisons[j + 1] = tempPoison;
			}
		}
	}
	return;
}

WORD 
PAL_New_GetEnemySorceryResistance(
WORD		wEnemyIndex
)
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	WORD w = gpGlobals->g.rgObject[wObjectID].enemy.wResistanceToSorcery * 10;

	return min(w, 100);
}

WORD
PAL_New_GetEnemyPoisonResistance(
WORD		wEnemyIndex
)
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	WORD w = g_Battle.rgEnemy[wEnemyIndex].e.wPoisonResistance * 10;

	return min(w, 100);
}

WORD
PAL_New_GetEnemyPhysicalResistance(
WORD		wEnemyIndex
)
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	WORD w = g_Battle.rgEnemy[wEnemyIndex].e.wPhysicalResistance * 10;

	return min(w, 100);
}

WORD
PAL_New_GetEnemyElementalResistance(
WORD		wEnemyIndex,
INT			iAttrib
)
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	WORD w = g_Battle.rgEnemy[wEnemyIndex].e.wElemResistance[iAttrib] * 10;

	return min(w, 100);
}

VOID
PAL_New_SetEnemyStatus(
WORD		wEnemyIndex,
WORD		wStatusID,
WORD		wNumRound
)
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return;
	}

	WORD wNumRoundNow = g_Battle.rgEnemy[wEnemyIndex].rgwStatus[wStatusID];

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
		case kStatusPuppet:
		case kStatusBravery:
		case kStatusProtect:
		case kStatusDualAttack:
		case kStatusHaste:
		{
			if (wNumRoundNow == 0)
			{
				g_Battle.rgEnemy[wEnemyIndex].rgwStatus[wStatusID] = wNumRound;
			}
			else if (wNumRoundNow <= 2)
			{
				g_Battle.rgEnemy[wEnemyIndex].rgwStatus[wStatusID]++;
			}
			break;
		}

		default:
			assert(FALSE);
			break;
	}
}

WORD
PAL_New_GetEnemyAttackStrength(
WORD		wEnemyIndex
)
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	INT s = (SHORT)g_Battle.rgEnemy[wEnemyIndex].e.wAttackStrength;

#ifdef EDIT_DAMAGE_CALC
	s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * 7;
#else
	s += (g_Battle.rgEnemy[wEnemyIndex].e.wLevel + 6) * 6;
#endif

#ifdef FINISH_GAME_MORE_ONE_TIME
	s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * gpGlobals->bFinishGameTime / 2;
#endif

	s = max(s, 0);
	s = min(s, 0xFFFF);

	return (WORD)s;
}

WORD
PAL_New_GetEnemyMagicStrength(
WORD		wEnemyIndex
)
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	INT s = (SHORT)g_Battle.rgEnemy[wEnemyIndex].e.wMagicStrength;

#ifdef EDIT_DAMAGE_CALC

	s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * 7;
#else
	s += (g_Battle.rgEnemy[wEnemyIndex].e.wLevel + 6) * 6;
#endif

#ifdef FINISH_GAME_MORE_ONE_TIME
	s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * gpGlobals->bFinishGameTime;
#endif

	s = max(s, 0);
	s = min(s, 0xFFFF);

	return (WORD)s;
}

WORD
PAL_New_GetEnemyDefense(
WORD		wEnemyIndex
)
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	INT s = (SHORT)g_Battle.rgEnemy[wEnemyIndex].e.wDefense;
	
#ifdef EDIT_DAMAGE_CALC
	s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * 5;
#else
	s += (g_Battle.rgEnemy[wEnemyIndex].e.wLevel + 6) * 4;
#endif

#ifdef FINISH_GAME_MORE_ONE_TIME
	s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * gpGlobals->bFinishGameTime;
#endif

	s = max(s, 0);
	s = min(s, 0xFFFF);

	return (WORD)s;
}

WORD 
PAL_New_GetEnemyDexterity(
WORD		wEnemyIndex
)
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	INT s = (SHORT)g_Battle.rgEnemy[wEnemyIndex].e.wDexterity;

#ifdef EDIT_DAMAGE_CALC
	s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * 4;
#else
	s += (g_Battle.rgEnemy[wEnemyIndex].e.wLevel + 6) * 3;
#endif

#ifdef FINISH_GAME_MORE_ONE_TIME
	s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * (gpGlobals->bFinishGameTime - 1);
#endif

	s = max(s, 0);
	s = min(s, 0xFFFF);

	return (WORD)s;
}

WORD
PAL_New_GetEnemyFleeRate(
WORD		wEnemyIndex
)
{
	WORD wObjectID = g_Battle.rgEnemy[wEnemyIndex].wObjectID;
	if (wEnemyIndex >= MAX_ENEMIES_IN_TEAM || wObjectID == 0)
	{
		return 0;
	}

	INT s = (SHORT)(g_Battle.rgEnemy[wEnemyIndex].e.wFleeRate);

#ifdef EDIT_DAMAGE_CALC
	s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * 3;
#else
	s += (g_Battle.rgEnemy[wEnemyIndex].e.wLevel + 6) * 2;
#endif

#ifdef FINISH_GAME_MORE_ONE_TIME
	s += g_Battle.rgEnemy[wEnemyIndex].e.wLevel * gpGlobals->bFinishGameTime;
#endif

	s = max(s, 0);
	s = min(s, 0xFFFF);

	return (WORD)s;
}

DWORD
PAL_New_EstimateEnemyHealthByLevel(
WORD		wEnemyLevel
)
{
	DWORD dwNewHP = 0;

	if (wEnemyLevel <= 50)
	{
		dwNewHP = 50 * wEnemyLevel + 200;
	}
	else if (wEnemyLevel <= 100)
	{
		dwNewHP = (DWORD)(0.01 * pow(wEnemyLevel, 3));
		dwNewHP += 1200;
	}
	else
	{
		dwNewHP = 11200;
		dwNewHP += 50 * (wEnemyLevel - 100) + 200;
	}
	return dwNewHP;
}
