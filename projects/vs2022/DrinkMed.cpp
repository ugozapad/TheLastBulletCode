/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "items.h"
#include "gamerules.h"

extern int gmsgItemPickup;

class CcacaoHealthKit : public CItem
{
	void Spawn(void);
	void Precache(void);
	BOOL MyTouch(CBasePlayer *pPlayer);

	/*
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];
	*/

};


LINK_ENTITY_TO_CLASS(item_cacaohealthkit, CcacaoHealthKit);

/*
TYPEDESCRIPTION	CEatHealthKit::m_SaveData[] =
{

};


IMPLEMENT_SAVERESTORE( CHealthKit, CItem);
*/

void CcacaoHealthKit::Spawn(void)
{
	Precache();
	if (pev->model)
	{
		SET_MODEL(ENT(pev), STRING(pev->model));
	}
	else
	{
		SET_MODEL(ENT(pev), "models/newHealthykit/w_cacao.mdl");
	}
	CItem::Spawn();
}

void CcacaoHealthKit::Precache(void)
{
	if (pev->model)
	{
		PRECACHE_MODEL((char*)STRING(pev->model));
	}
	else
	{
		PRECACHE_MODEL("models/newHealthykit/w_cacao.mdl");
	}
	PRECACHE_SOUND("TLB/healtyeat/cacao.wav");
}

BOOL CcacaoHealthKit::MyTouch(CBasePlayer *pPlayer)
{
	if (pPlayer->pev->deadflag != DEAD_NO)
	{
		return FALSE;
	}

	if (pPlayer->TakeHealth(gSkillData.cacaohealth, DMG_GENERIC))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
		WRITE_STRING(STRING(pev->classname));
		MESSAGE_END();

		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "TLB/healtyeat/cacao.wav", 1, ATTN_NORM);

		if (g_pGameRules->ItemShouldRespawn(this))
		{
			Respawn();
		}
		else
		{
			UTIL_Remove(this);
		}

		return TRUE;
	}

	return FALSE;
}



