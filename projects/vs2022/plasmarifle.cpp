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
#include "gamerules.h"
//#include "xen.h"
#include "plasma.h"

enum plasmarifle_e {
	PLASMARIFLE_IDLE,
	PLASMARIFLE_IDLE2,
	PLASMARIFLE_FIDGET,
	PLASMARIFLE_SPINUP,
	PLASMARIFLE_SPIN,
	PLASMARIFLE_FIRE,
	PLASMARIFLE_FIRE_SOLID,
	PLASMARIFLE_FIRE_SEC,
	PLASMARIFLE_FIRE_SEC_SOLID,
	PLASMARIFLE_RELOAD,
	PLASMARIFLE_DRAW,
	PLASMARIFLE_HOLSTER
};

LINK_ENTITY_TO_CLASS(weapon_plasmarifle, CPlasmarifle);

void CPlasmarifle::Spawn()
{
	Precache();
	m_iId = WEAPON_PLASMARIFLE;
	SET_MODEL(ENT(pev), "models/w_plasmarifle.mdl");

	m_iDefaultAmmo = PLASMARIFLE_DEFAULT_GIVE;

	FallInit();// get ready to fall
}


void CPlasmarifle::Precache(void)
{
	PRECACHE_MODEL("models/v_plasmarifle.mdl");
	PRECACHE_MODEL("models/w_plasmarifle.mdl");
	PRECACHE_MODEL("models/p_plasmarifle.mdl");
    PRECACHE_MODEL("sprites/muzzleflashplasma.spr");

	PRECACHE_SOUND("weapons/plasmarifle_fire2.wav");
	PRECACHE_SOUND("weapons/plasmarifle_fire.wav");
	PRECACHE_SOUND("weapons/plasmarifle_spin.wav");

	PRECACHE_SOUND("items/9mmclip1.wav");

	m_iPlasmaSprite = PRECACHE_MODEL("sprites/tsplasma.spr");
	m_iMazzlePlasma = PRECACHE_MODEL("sprites/muzzleflashplasma.spr");
	UTIL_PrecacheOther("plasma");

	m_usPlasmaFire = PRECACHE_EVENT(1, "events/plasmarifle.sc");
}

int CPlasmarifle::AddToPlayer(CBasePlayer *pPlayer)
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


int CPlasmarifle::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "plasma";
	p->iMaxAmmo1 = PLASMARIFLE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = PLASMARIFLE_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 3;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_PLASMARIFLE;
	p->iWeight = PLASMARIFLE_WEIGHT;

	return 1;
}



BOOL CPlasmarifle::Deploy()
{
	return DefaultDeploy( "models/v_plasmarifle.mdl", "models/p_plasmarifle.mdl", PLASMARIFLE_DRAW, "egon", 0.6 );
}

void CPlasmarifle::Holster( )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	m_fInAttack = 0;
	m_fInReload = FALSE;
	SendWeaponAnim( PLASMARIFLE_HOLSTER );
}

void CPlasmarifle::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = GetNextAttackDelay(0.12);
		return;
	}

	if (m_iClip <= 0)
	{
		Reload();
		if (m_iClip == 0)
			PlayEmptySound();
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	// m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	UTIL_MakeVectors( m_pPlayer->pev->v_angle );
	Vector vecSrc = m_pPlayer->GetGunPosition( ) + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;

#ifndef CLIENT_DLL	
	CPlasma *pPlasma = CPlasma::CreatePlasmaRocket( vecSrc, m_pPlayer->pev->v_angle, m_pPlayer );
	UTIL_MakeVectors( m_pPlayer->pev->v_angle );
	pPlasma->pev->velocity = pPlasma->pev->velocity + gpGlobals->v_forward * 2000;
#endif

#ifndef CLIENT_DLL	
	Vector vecSrcMazzle = m_pPlayer->GetGunPosition( ) + gpGlobals->v_forward * 50 + gpGlobals->v_right * 8 + gpGlobals->v_up * -12;

	TraceResult tr;
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, vecSrcMazzle );
		WRITE_BYTE( TE_SPRITE );		// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD( vecSrcMazzle.x );	// Send to PAS because of the sound
		WRITE_COORD( vecSrcMazzle.y );
		WRITE_COORD( vecSrcMazzle.z );
		WRITE_SHORT( m_iMazzlePlasma );
		WRITE_BYTE( 3 ); // scale * 10
		WRITE_BYTE( 128 ); // framerate
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSrcMazzle  );
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD( vecSrcMazzle.x );	// X
		WRITE_COORD( vecSrcMazzle.y );	// Y
		WRITE_COORD( vecSrcMazzle.z );	// Z
		WRITE_BYTE( 15 );		// radius * 0.1
		WRITE_BYTE( 15 );		// r
		WRITE_BYTE( 220 );		// g
		WRITE_BYTE( 40 );		// b
		WRITE_BYTE( 5 );		// time * 10
		WRITE_BYTE( 10 );		// decay * 0.1
	MESSAGE_END( );
#endif

	PLAYBACK_EVENT_FULL(
		flags,
		m_pPlayer->edict(), 
		m_usPlasmaFire, 
		0.0, 
		(float *)&g_vecZero, 
		(float *)&g_vecZero, 
		vecSrc.x,
		vecSrc.y,
		*(int*)&vecSrc.z,
		m_iPlasmaSprite, 
		0,
		TRUE);


	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.12);

#if 1
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.12;
#endif
}

void CPlasmarifle::WeaponIdle( void )
{
	if (m_flTimeWeaponIdle > gpGlobals->time)
		return;

	if (m_fInAttack==1)
	{
		m_fInAttack = 0;
		return;
	}

	int iAnim;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = PLASMARIFLE_IDLE;	
		break;
	
	default:
	case 1:
		iAnim = PLASMARIFLE_IDLE2;
		break;
	}

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT ( 10, 15 );
}

void CPlasmarifle::Reload( void )
{
	if (m_iClip)
	return;

	DefaultReload( 30, PLASMARIFLE_RELOAD, 4.9, 1.4 );
}


class CPlasmaAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_plasmaclip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_plasmaclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( PLASMARIFLE_DEFAULT_GIVE, "plasma", PLASMARIFLE_MAX_CARRY ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_plasma, CPlasmaAmmo );