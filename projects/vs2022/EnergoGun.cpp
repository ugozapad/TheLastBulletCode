///***
//*
//*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
//*
//*	This product contains software technology licensed from Id
//*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
//*	All Rights Reserved.
//*
//*   Use, distribution, and modification of this source code and/or resulting
//*   object code is restricted to non-commercial enhancements to products from
//*   Valve LLC.  All other use, distribution, or modification is prohibited
//*   without written permission from Valve LLC.
//*
//****/
//
//#include "extdll.h"
//#include "util.h"
//#include "cbase.h"
//#include "monsters.h"
//#include "weapons.h"
//#include "nodes.h"
//#include "player.h"
//#include "soundent.h"
//#include "gamerules.h"
//
//enum mp5_e
//{
//	MP5_LONGIDLE = 0,
//	MP5_IDLE1,
//	MP5_LAUNCH,
//	MP5_RELOAD,
//	MP5_DEPLOY,
//	MP5_FIRE1,
//	MP5_FIRE2,
//	MP5_FIRE3,
//};
//
//
//
//LINK_ENTITY_TO_CLASS(weapon_energo, CEnergo);
////LINK_ENTITY_TO_CLASS( weapon_mp5, CMP5 );
//
//
////=========================================================
////=========================================================
//
//void CEnergo::Spawn()
//{
//	pev->classname = MAKE_STRING("weapon_energo"); // hack to allow for old names
//	Precache();
//	SET_MODEL(ENT(pev), "models/w_energo.mdl");
//	m_iId = WEAPON_ENERGO;
//
//	m_iDefaultAmmo = 60;
//
//	FallInit();// get ready to fall down.
//}
//
//
//void CEnergo::Precache(void)
//{
//	// Тут вы прекешите звуки и модели для вашего оружия
//	PRECACHE_MODEL("models/v_energo.mdl");
//	PRECACHE_MODEL("models/w_energo.mdl");
//	PRECACHE_MODEL("models/p_energo.mdl");
//
//	PRECACHE_SOUND("items/9mmclip1.wav");
//
//	PRECACHE_SOUND("items/clipinsert1.wav");
//	PRECACHE_SOUND("items/cliprelease1.wav");
//
//	PRECACHE_SOUND("weapons/ethereal_shoot1.wav");
//	PRECACHE_SOUND("weapons/ethereal_shoot1.wav");
//	PRECACHE_SOUND("weapons/ethereal_shoot1.wav");
//
//	// this is to hook your client-side event
//	m_usEnergoFire = PRECACHE_EVENT(1, "events/energogun.sc");
//}
////веном-пулемет, как и енергоган перемещены на 5 слот в игре, но 4 в коде. на 6 место, ибо не хочу туда сюда перемещать оригинальные пушки.
//int CEnergo::GetItemInfo(ItemInfo* p)
//{
//	p->pszName = STRING(pev->classname);
//	p->pszAmmo1 = "uranium";
//	p->iMaxAmmo1 = 220;
//	p->pszAmmo2 = "NULL";
//	p->iMaxAmmo2 = -1;
//	p->iMaxClip = 60;
//	p->iSlot = 4;
//	p->iPosition = 7;
//	p->iFlags = 0;
//	p->iId = m_iId = WEAPON_ENERGO;
//	p->iWeight = MP5_WEIGHT;
//
//	return 1;
//}
//
//
//int CEnergo::AddToPlayer(CBasePlayer* pPlayer)
//{
//	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
//	{
//		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
//		WRITE_BYTE(m_iId);
//		MESSAGE_END();
//		return TRUE;
//	}
//	return FALSE;
//}
//
//
//
//BOOL CEnergo::Deploy()
//{
//	return DefaultDeploy("models/v_energo.mdl", "models/p_energo.mdl", MP5_DEPLOY, "mp5");
//}
//
//void CEnergo::PrimaryAttack()
//{
//	// don't fire underwater
//	if (m_pPlayer->pev->waterlevel == 3)
//	{
//		PlayEmptySound();
//		m_flNextPrimaryAttack = 0.15;
//		return;
//	}
//
//	if (m_iClip <= 0)
//	{
//		PlayEmptySound();
//		m_flNextPrimaryAttack = 0.15;
//		return;
//	}
//
//	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
//	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
//
//	m_iClip--;
//
//
//	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
//
//	// player "shoot" animation
//	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
//
//	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);//(первыая строчка для разброса, ниже после вспышки дианмик света.)
//	Vector vecSrc = m_pPlayer->GetGunPosition();
//	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
//	Vector vecDir;
//
//#ifdef CLIENT_DLL
//	if (!bIsMultiplayer())
//#else
//	if (g_pGameRules->IsMultiplayer())
//#endif
//	{
//		// optimized multiplayer. Widened to make it easier to hit a moving player
//		vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, BULLET_PLAYER_MP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);
//	}
//	else
//	{
//		// single player spread
//		vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_MP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);
//
//
//		m_pPlayer->WeaponScreenPunch(0.5, 3.2); //(типо разброс  , первое по горизонтали, второе - по вертикали)
//
//	}
//
//	int flags;
//#if defined( CLIENT_WEAPONS )
//	flags = FEV_NOTHOST;
//#else
//	flags = 0;
//#endif
//
//	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usEnergoFire, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0);
//
//	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
//		// HEV suit - indicate out of ammo condition
//		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
//
//	m_flNextPrimaryAttack = GetNextAttackDelay(0.1);
//
//	if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
//		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
//
//	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
//
//}
//
//
//
//void CEnergo::Reload(void)
//{
//	if (m_pPlayer->ammo_mp44 <= 0)
//		return;
//	if (m_iClip == 60) return;
//
//	/*int iResult;*/
//
//
//	/*iResult = */DefaultReload(MP5_MAX_CLIP, MP5_RELOAD, 1.5);
//
//
//	/*if (iResult)
//	{
//		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
//		SetThink(&CEnergo::SpawnClip);
//		pev->nextthink = gpGlobals->time + 0.8;
//	}*/
//}
//
////void CEnergo::SpawnClip()
////{
////	int m_iClipmp5;
////	if (m_iClip == 0)
////	{
////		m_iClipmp5 = PRECACHE_MODEL("models/w_9mmARclip_empty.mdl");// ставим модель магазина.
////	}
////	else
////	{
////		m_iClipmp5 = PRECACHE_MODEL("models/w_9mmARclip.mdl");
////	}
////	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
////	Vector	vecClipVelocity = m_pPlayer->pev->velocity
////		+ gpGlobals->v_right * RANDOM_FLOAT(0, 5)
////		+ gpGlobals->v_up * RANDOM_FLOAT(-10, -15)
////		+ gpGlobals->v_forward * 1;
////	EjectBrass(pev->origin + gpGlobals->v_up * -4 + gpGlobals->v_forward * 1, vecClipVelocity, pev->angles.y, m_iClipmp5, TE_BOUNCE_NULL);//собственно вместо TE_BOUNCE_NULL можете выставить звук любого материала, а потом заменить его...(если хотите чтобы магазин издавал звук при падении).
////}
//
//void CEnergo::WeaponIdle(void)
//{
//	ResetEmptySound();
//
//	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
//
//	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
//		return;
//
//	int iAnim;
//	switch (RANDOM_LONG(0, 1))
//	{
//	case 0:
//		iAnim = MP5_LONGIDLE;
//		break;
//
//	default:
//	case 1:
//		iAnim = MP5_IDLE1;
//		break;
//	}
//
//	SendWeaponAnim(iAnim);
//
//	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15); // how long till we do this again.
//}