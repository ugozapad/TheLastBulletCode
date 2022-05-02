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
#include "soundent.h"
#include "gamerules.h"

enum mp5_e
{
	MP5_LONGIDLE = 0,
	MP5_IDLE1,
	MP5_LAUNCH,
	MP5_RELOAD,
	MP5_DEPLOY,
	MP5_FIRE1,
	MP5_FIRE2,
	MP5_FIRE3,
};



LINK_ENTITY_TO_CLASS(weapon_sten, CStenGun);
//LINK_ENTITY_TO_CLASS( weapon_mp5, CMP5 );


//=========================================================
//=========================================================

void CStenGun::Spawn()
{
	pev->classname = MAKE_STRING("weapon_sten"); // hack to allow for old names
	Precache();
	SET_MODEL(ENT(pev), "models/w_sten.mdl");
	m_iId = WEAPON_STEN;

	m_iDefaultAmmo = 50;

	FallInit();// get ready to fall down.
}


void CStenGun::Precache(void)
{
	// Тут вы прекешите звуки и модели для вашего оружия
	PRECACHE_MODEL("models/v_sten.mdl");
	PRECACHE_MODEL("models/w_sten.mdl");
	PRECACHE_MODEL("models/p_sten.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");

	PRECACHE_SOUND("weapons/sten/stenshoot.wav");
	PRECACHE_SOUND("weapons/sten/stenshoot2.wav");


	// this is to hook your client-side event
	m_usStenFire = PRECACHE_EVENT(1, "events/sten.sc");
}

int CStenGun::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "mp5";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = "NULL";
	p->iMaxAmmo2 = -1;
	p->iMaxClip = MP5_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_STEN;
	p->iWeight = MP5_WEIGHT;

	return 1;
}

int CStenGun::AddToPlayer(CBasePlayer* pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}



BOOL CStenGun::Deploy()
{
	return DefaultDeploy("models/v_sten.mdl", "models/p_sten.mdl", MP5_DEPLOY, "mp5");
}

void CStenGun::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;


	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
	Vector vecDir;

#ifdef CLIENT_DLL
	if (!bIsMultiplayer())
#else
	if (g_pGameRules->IsMultiplayer())
#endif
	{
		// optimized multiplayer. Widened to make it easier to hit a moving player
		vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, BULLET_PLAYER_MP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);
	}
	else
	{
		// single player spread
		vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_MP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);


		m_pPlayer->WeaponScreenPunch(0.9, 1.2);

	}

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usStenFire, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.1);

	if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);

}



void CStenGun::Reload(void)
{
	if (m_pPlayer->ammo_mp5 <= 0)
		return;

	/*int iResult;*/


	/*iResult = */DefaultReload(MP5_MAX_CLIP, MP5_RELOAD, 1.5);


	/*if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
		SetThink(&CStenGun::SpawnClip);
		pev->nextthink = gpGlobals->time + 0.8;
	}*/
}

//void CStenGun::SpawnClip()
//{
//	int m_iClipmp5;
//	if (m_iClip == 0)
//	{
//		m_iClipmp5 = PRECACHE_MODEL("models/w_9mmARclip_empty.mdl");// ставим модель магазина.
//	}
//	else
//	{
//		m_iClipmp5 = PRECACHE_MODEL("models/w_9mmARclip.mdl");
//	}
//	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
//	Vector	vecClipVelocity = m_pPlayer->pev->velocity
//		+ gpGlobals->v_right * RANDOM_FLOAT(0, 5)
//		+ gpGlobals->v_up * RANDOM_FLOAT(-10, -15)
//		+ gpGlobals->v_forward * 1;
//	EjectBrass(pev->origin + gpGlobals->v_up * -4 + gpGlobals->v_forward * 1, vecClipVelocity, pev->angles.y, m_iClipmp5, TE_BOUNCE_NULL);//собственно вместо TE_BOUNCE_NULL можете выставить звук любого материала, а потом заменить его...(если хотите чтобы магазин издавал звук при падении).
//}

void CStenGun::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;
	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		iAnim = MP5_LONGIDLE;
		break;

	default:
	case 1:
		iAnim = MP5_IDLE1;
		break;
	}

	SendWeaponAnim(iAnim);

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15); // how long till we do this again.
}
