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

#define VENOM_WIND_TIME 0.6f

enum venom_e
{
	VENOM_LONGIDLE = 0,
	VENOM_IDLE1,
	VENOM_LAUNCH,
	VENOM_RELOAD,
	VENOM_DEPLOY,
	VENOM_FIRE1, 
	VENOM_FIRE2,
	VENOM_FIRE3,
	VENOM_SPINUP,
	VENOM_SPINDOWN

};



LINK_ENTITY_TO_CLASS(weapon_venom, CVenom);
//LINK_ENTITY_TO_CLASS(weapon_9mmAR, CMP5);


//=========================================================
//=========================================================
int CVenom::SecondaryAmmoIndex(void)
{
	return m_iSecondaryAmmoType;
}

void CVenom::Spawn()
{
	//pev->classname = MAKE_STRING("weapon_venom"); // hack to allow for old names
	Precache();
	SET_MODEL(ENT(pev), "models/w_venom.mdl");
	m_iId = WEAPON_VENOM;

	m_iDefaultAmmo = VENOM_DEFAULT_GIVE;

	m_flRotationSpeed = 0;

	m_fInAttack = 0;

	FallInit();// get ready to fall down.
}


void CVenom::Precache(void)
{
	
	PRECACHE_MODEL("models/w_venom.mdl");
	PRECACHE_MODEL("models/v_venom.mdl");
	PRECACHE_MODEL("models/p_venom.mdl");

	m_iShell = PRECACHE_MODEL("models/shell.mdl");// brass shellTE_MODEL

	PRECACHE_MODEL("models/grenade.mdl");	// grenade

	PRECACHE_MODEL("models/w_venomclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");

	PRECACHE_SOUND("weapons/venom/hks1.wav");// H to the K
	PRECACHE_SOUND("weapons/venom/hks2.wav");// H to the K
	PRECACHE_SOUND("weapons/venom/hks3.wav");// H to the K

	PRECACHE_SOUND("weapons/venom/glauncher.wav");
	PRECACHE_SOUND("weapons/venom/glauncher2.wav");

	PRECACHE_SOUND("weapons/357_cock1.wav");

	m_usVenom = PRECACHE_EVENT(1, "events/venom.sc");
	m_usVenom2 = PRECACHE_EVENT(1, "events/venom2.sc");
	m_usVenom3 = PRECACHE_EVENT(1, "events/venom3.sc");
}

void CVenom::Holster(int skiplocal)
{
	m_flRotationSpeed = 0;

	CBasePlayerWeapon::Holster();
}


void CVenom::ItemPostFrame()
{

	if ((m_fInReload) && (m_pPlayer->m_flNextAttack <= 0.0))
	{
		// complete the reload. 
		int j = min(iMaxClip() - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

		// Add them to the clip
		m_iClip += j;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;

#ifndef CLIENT_DLL
		m_pPlayer->TabulateAmmo();
#endif
		m_fInReload = FALSE;
	}

	if (!m_flRotationSpeed)
	{
		m_flPlayerSpeed = m_pPlayer->pev->maxspeed;
	}

	float flLastRotationSpeed = m_flRotationSpeed;

	if ((m_pPlayer->pev->button & IN_ATTACK2) && (m_flNextSecondaryAttack <= 0.0))
	{
		if (pszAmmo2() && !m_pPlayer->m_rgAmmo[SecondaryAmmoIndex()])
		{
			m_fFireOnEmpty = TRUE;
		}

		SecondaryAttack();
		m_pPlayer->pev->button &= ~IN_ATTACK2;
	}
	else if ((m_pPlayer->pev->button & IN_ATTACK) && (m_flNextPrimaryAttack <= 0.0))
	{
		if ((m_iClip == 0 && pszAmmo1()) || (iMaxClip() == -1 && !m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()]))
		{
			m_fFireOnEmpty = TRUE;
		}

		if (m_iClip > 0)
		{
			if (m_flRotationSpeed < 0.99f)
			{
				Spinup();
			}
			else
			{
				PrimaryAttack();
			}
		}
	}
	else if (m_pPlayer->pev->button & IN_RELOAD && iMaxClip() != WEAPON_NOCLIP && !m_fInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
	}
	else if (!(m_pPlayer->pev->button & (IN_ATTACK | IN_ATTACK2)))
	{
		// no fire buttons down

		m_fFireOnEmpty = FALSE;

		// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
		if (m_iClip == 0 && !(iFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < 0.0)
		{
			Reload();
			return;
		}
		else
		{
			ReduceRotation();
		}


		// If the speed changed, modify our movement speed
		if (m_flRotationSpeed != flLastRotationSpeed)
		{
			m_pPlayer->pev->maxspeed = m_flPlayerSpeed * (1.0f - (m_flRotationSpeed * 0.5f)); //SetMaxSpeed(m_flOwnersMaxSpeed * (1.0 - (m_flRotationSpeed * 0.5)));
		}
		
		
		Spindown();
		

		WeaponIdle();
		return;
	}

	// catch all
	if (ShouldWeaponIdle())
	{
		WeaponIdle();
	}
}

int CVenom::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "338";
	p->iMaxAmmo1 = _338_MAX_CARRY;
	p->pszAmmo2 = "ARgrenades";
	p->iMaxAmmo2 = M203_GRENADE_MAX_CARRY;
	p->iMaxClip = VENOM_MAX_CLIP;
	p->iSlot = 4;
	p->iPosition = 6;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_VENOM;
	p->iWeight = VENOM_WEIGHT;

	return 1;
}

int CVenom::AddToPlayer(CBasePlayer* pPlayer)
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

BOOL CVenom::Deploy()
{
	return DefaultDeploy("models/v_venom.mdl", "models/p_venom.mdl", VENOM_DEPLOY, "338");
}


void CVenom::Spinup()
{
	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	m_flRotationSpeed = min(1, m_flRotationSpeed + (gpGlobals->frametime / VENOM_WIND_TIME));
	//PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usVenom3, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, 0, 0, 0, 0, 0, 0);
	
	if (!m_fInAttack)
	{
		SendWeaponAnim(VENOM_SPINUP);
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usVenom3, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, 0, 0, 0, 0, m_fInAttack, 0);
		m_fInAttack = 1;
	}

	
}

void CVenom::Spindown()
{
	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	if (m_fInAttack)
	{
		SendWeaponAnim(VENOM_SPINDOWN);
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usVenom3, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, 0, 0, 0, 0, m_fInAttack, 0);
		m_fInAttack = 0;
	}
	else return;
}

void CVenom::PrimaryAttack()
{
	// recoil from the shot
	float flZVel = m_pPlayer->pev->velocity.z;
	m_pPlayer->pev->velocity = m_pPlayer->pev->velocity - gpGlobals->v_forward * 25;
	m_pPlayer->pev->velocity.z = flZVel;
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
		vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, BULLET_PLAYER_338, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);
	}
	else
	{
		// single player spread
		vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_338, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);
	
		m_pPlayer->WeaponScreenPunch(1.3, 2.);

	}



	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usVenom, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.1);

	if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
	m_fInAttack = 0;
}



void CVenom::SecondaryAttack(void)
{
	
}

void CVenom::ReduceRotation(void)
{
	if (m_flRotationSpeed > 0)
	{
		m_flRotationSpeed = max(0, m_flRotationSpeed - (gpGlobals->frametime / VENOM_WIND_TIME));
	}
}

void CVenom::Reload(void)
{
	if (m_pPlayer->ammo_338 <= 0)
		return;

	/*int iResult;*/
	if (m_flRotationSpeed > 0)
	{
		ReduceRotation();
	}
	else
	{

		/*iResult = */DefaultReload(VENOM_MAX_CLIP, VENOM_RELOAD, 4.0);
	}


	/*if (iResult)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
		SetThink(&CMP5::SpawnClip);
		pev->nextthink = gpGlobals->time + 0.8;
	}*/
}

//void CMP5::SpawnClip()
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

void CVenom::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;
	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		iAnim = VENOM_LONGIDLE;
		break;

	default:
	case 1:
		iAnim = VENOM_IDLE1;
		break;
	}

	SendWeaponAnim(iAnim);

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15); // how long till we do this again.
}



class CVenomAmmoClip : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_venomclip.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_9mmARclip_empty.mdl");
		PRECACHE_MODEL("models/w_venomclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		int bResult = (pOther->GiveAmmo(AMMO_VENOMCLIP_GIVE, "338", _338_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_venomclip, CVenomAmmoClip);
//LINK_ENTITY_TO_CLASS(ammo_9mmAR, CMP5AmmoClip);



class CVenomChainammo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_chainammo.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_chainammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		int bResult = (pOther->GiveAmmo(AMMO_CHAINBOX_GIVE, "338", _338_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_venombox, CVenomChainammo);


class CVenomAmmoGrenade : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_ARgrenade.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_ARgrenade.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		int bResult = (pOther->GiveAmmo(AMMO_M203BOX_GIVE, "ARgrenades", M203_GRENADE_MAX_CARRY) != -1);

		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_venomgrenades, CVenomAmmoGrenade);
//LINK_ENTITY_TO_CLASS(ammo_ARgrenades, CMP5AmmoGrenade);


















