/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
*	CrazyArts 2009
*	http://crazyarts.do.am/
*	SAW gun, idea from HL Opposing Force
*
****/
#if !defined( OEM_BUILD ) && !defined( HLDEMO_BUILD )

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "monsters.h"
#include "player.h"
#include "gamerules.h"


enum saw_e {
	SAW_SLOWIDLE = 0,
	SAW_IDLE2,
	SAW_RELOAD1,
	SAW_RELOAD2,
	SAW_HOLSTER,
	SAW_DRAW,
	SAW_SHOOT,
	SAW_SHOOT2,
	SAW_SHOOT3
};

LINK_ENTITY_TO_CLASS( weapon_saw, CSAW );
LINK_ENTITY_TO_CLASS( weapon_m249, CSAW );

int CSAW::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556";
	p->iMaxAmmo1 = _556_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SAW_MAX_CLIP;
	p->iFlags = 0;
	p->iSlot = 5;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_SAW;
	p->iWeight = SAW_WEIGHT;

	return 1;
}

int CSAW::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

void CSAW::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_saw"); // hack to allow for old names
	Precache( );
	m_iId = WEAPON_SAW;
	SET_MODEL(ENT(pev), "models/w_saw.mdl");
	m_iDefaultAmmo = SAW_DEFAULT_GIVE;
	FallInit();// get ready to fall down.
}


void CSAW::Precache( void )
{
	PRECACHE_MODEL("models/v_saw.mdl");
	PRECACHE_MODEL("models/w_saw.mdl");
	PRECACHE_MODEL("models/p_saw.mdl");
	PRECACHE_MODEL("models/w_saw_clip.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");        
	PRECACHE_SOUND ("weapons/saw_reload2.wav");
	PRECACHE_SOUND ("weapons/357_cock1.wav");
	PRECACHE_SOUND ("weapons/saw_fire1.wav");
	PRECACHE_SOUND ("weapons/saw_fire2.wav");
	PRECACHE_SOUND ("weapons/saw_fire3.wav");

	m_iShell = PRECACHE_MODEL ("models/saw_shell.mdl");
	m_iLink	 = PRECACHE_MODEL ("models/saw_link.mdl");

	m_usFireSAW = PRECACHE_EVENT( 1, "events/saw.sc" );
}

BOOL CSAW::Deploy( )
{
	return DefaultDeploy( "models/v_saw.mdl", "models/p_saw.mdl", SAW_DRAW, "saw", UseDecrement(), pev->body );
}


void CSAW::Holster( int skiplocal /* = 0 */ )
{
	m_fInReload = FALSE;// cancel any reload in progress.
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	SendWeaponAnim( SAW_HOLSTER );
}

void CSAW::PrimaryAttack()
{
	if (m_iClip <= 0)
	{
		if (!m_fFireOnEmpty)
			Reload( );
		else
		{
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM);
			m_flNextPrimaryAttack = 0.2;
		}
		return;
	}
	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	m_iClip--;
	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
	//UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, BULLET_PLAYER_556, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
    int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
//Shells and Links for SAW gun
#if defined( OLD_WEAPONS )		
	Vector	vecShellVelocity = m_pPlayer->pev->velocity + gpGlobals->v_right * RANDOM_FLOAT(50,70) + gpGlobals->v_up * RANDOM_FLOAT(100,150) + gpGlobals->v_forward * 25;
	switch ( RANDOM_LONG(0,1) )
	{
	case 0:
		EjectBrass ( pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -12 + gpGlobals->v_forward * 32 + gpGlobals->v_right * 6 , vecShellVelocity, pev->angles.y, m_iShell, TE_BOUNCE_SHELL ); 
		break;
	case 1:
		EjectBrass ( pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -12 + gpGlobals->v_forward * 32 + gpGlobals->v_right * 6 , vecShellVelocity, pev->angles.y, m_iLink, TE_BOUNCE_SHELL ); 
		break;
	}
#endif
//END of SHELLS
	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireSAW, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = 0.066;
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}


void CSAW::Reload( void )
{
	if ( m_pPlayer->ammo_556 <= 0 )
		return;
	if ( m_pPlayer->pev->fov != 0 )
	{
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;  // 0 means reset to default fov
	}
	if (DefaultReload( 100, SAW_RELOAD1, 2.0, 0 ))
	{
		SendWeaponAnim( SAW_RELOAD2 );
		m_flSoundDelay = 1.5;
	}
}


void CSAW::WeaponIdle( void )
{
	ResetEmptySound( );
	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	// ALERT( at_console, "%.2f\n", gpGlobals->time - m_flSoundDelay );
	if (m_flSoundDelay != 0 && m_flSoundDelay <= UTIL_WeaponTimeBase() )
	{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/556_reload2.wav", RANDOM_FLOAT(0.8, 0.9), ATTN_NORM);
		m_flSoundDelay = 0;
	}
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	if (flRand <= 0.9)
	{
		iAnim = SAW_SLOWIDLE;
		m_flTimeWeaponIdle = (70.0/30.0);
	}
	else 
	{
		iAnim = SAW_IDLE2;
		m_flTimeWeaponIdle = (60.0/30.0);
	}	
	SendWeaponAnim( iAnim, UseDecrement() ? 1 : 0, 0 );
}



class C556Ammo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_saw_clip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_saw_clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_556_BOXGIVE, "556", _556_MAX_CARRY ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_556, C556Ammo );


#endif