/***
*
* Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
* This product contains software technology licensed from Id
* Software, Inc. ("Id Technology"). Id Technology (c) 1996 Id Software, Inc.
* All Rights Reserved.
*
* Use, distribution, and modification of this source code and/or resulting
* object code is restricted to non-commercial enhancements to products from
* Valve LLC. All other use, distribution, or modification is prohibited
* without written permission from Valve LLC.
*
*
*
*
* This weapon written by Ku2zoff
*
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
#include "shake.h"

#include "nodes.h"
#include "soundent.h"

enum sniperrifle_e {
    SNIPERRIFLE_DRAW = 0,
    SNIPERRIFLE_SLOWIDLE,
    SNIPERRIFLE_FIRE1,
    SNIPERRIFLE_FIRELASTROUND,
    SNIPERRIFLE_RELOAD,
    //SNIPERRIFLE_RELOAD2,
    SNIPERRIFLE_SLOWIDLEEMPTY,
    SNIPERRIFLE_HOLSTER,
};

LINK_ENTITY_TO_CLASS(weapon_sniperrifle, CSniperrifle);

int CSniperrifle::GetItemInfo(ItemInfo* p)
{
    p->pszName = STRING(pev->classname);
    p->pszAmmo1 = "k43";
    p->iMaxAmmo1 = 70;
    p->pszAmmo2 = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip = 5;
    p->iFlags = 0;
    p->iSlot = 3;
    p->iPosition = 5;
    p->iId = m_iId = WEAPON_SNIPERRIFLE;
    p->iWeight = SNIPERRIFLE_WEIGHT;

    return 1;
}

int CSniperrifle::AddToPlayer(CBasePlayer* pPlayer)
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
void CSniperrifle::Spawn()
{
    pev->classname = MAKE_STRING("weapon_sniperrifle"); // hack to allow for old names
    Precache();
    m_iId = WEAPON_SNIPERRIFLE;
    SET_MODEL(ENT(pev), "models/w_sniper.mdl");

    m_iDefaultAmmo = 5;

    FallInit();// get ready to fall down.
}


void CSniperrifle::Precache(void)
{
    PRECACHE_MODEL("models/v_sniper.mdl");
    PRECACHE_MODEL("models/w_sniper.mdl");
    PRECACHE_MODEL("models/p_sniper.mdl");

    PRECACHE_MODEL("models/w_sniper_clip.mdl");//ammo
    PRECACHE_SOUND("items/9mmclip1.wav");

    PRECACHE_SOUND("weapons/sniper_miss.wav");
    PRECACHE_SOUND("weapons/kar_shoot.wav");
    PRECACHE_SOUND("weapons/sniper_zoom.wav");
    

    m_usFireSniper = PRECACHE_EVENT(1, "events/sniper.sc");
}

BOOL CSniperrifle::Deploy()
{
    return DefaultDeploy("models/v_sniper.mdl", "models/p_sniper.mdl", SNIPERRIFLE_DRAW, "bow", UseDecrement());
}


void CSniperrifle::Holster(int skiplocal /* = 0 */)
{
    m_fInReload = FALSE;// cancel any reload in progress.

    if (m_fInZoom)
    {
        m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
        m_fInZoom = false;
    }
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
    m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
    SendWeaponAnim(SNIPERRIFLE_HOLSTER);
}

void CSniperrifle::SecondaryAttack(void)
{
    if (m_pPlayer->pev->fov == 0)
    {
        m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 15; // 0 means reset to default fov
        m_fInZoom = 1;
#ifndef CLIENT_DLL
        UTIL_ScreenFade(m_pPlayer, Vector(0, 0, 0), 0.5, 0.25, 255, FFADE_IN);
#endif
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sniper_zoom.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0, 0xF));
    }
    else if (m_pPlayer->pev->fov == 15)
    {
        m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 10;
        m_fInZoom = 1;
#ifndef CLIENT_DLL
        UTIL_ScreenFade(m_pPlayer, Vector(0, 0, 0), 0.5, 0.25, 255, FFADE_IN);
#endif
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sniper_zoom.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0, 0xF));
    }
    else if (m_pPlayer->pev->fov == 10)
    {
        m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
        m_fInZoom = 0;
#ifndef CLIENT_DLL
        UTIL_ScreenFade(m_pPlayer, Vector(0, 0, 0), 0.5, 0.25, 255, FFADE_IN);
#endif
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/sniper_zoom.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0, 0xF));
    }
    pev->nextthink = UTIL_WeaponTimeBase() + 0.1;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
}

void CSniperrifle::PrimaryAttack(void)
{
    Shoot(0.0020, 1.5, TRUE);
}

void CSniperrifle::Shoot(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
    if (m_pPlayer->pev->waterlevel == 3)
    {
        EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sniper_miss.wav", 0.8, ATTN_NORM);
        m_flNextPrimaryAttack = 0.15;
        return;
    }
    if (m_iClip <= 0)
    {
        if (m_fFireOnEmpty)
        {
            EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sniper_miss.wav", 0.8, ATTN_NORM);
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
        }
        return;
    }

    m_iClip--;

    m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

    int flags;

#if defined( CLIENT_WEAPONS )
    flags = FEV_NOTHOST;
#else
    flags = 0;
#endif

    // player "shoot" animation
    m_pPlayer->SetAnimation(PLAYER_ATTACK1);

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

    Vector vecSrc = m_pPlayer->GetGunPosition();
    Vector vecAiming;

    if (fUseAutoAim)
    {
        vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);
    }
    else
    {
        vecAiming = gpGlobals->v_forward;
    }

    Vector vecDir;
    vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 8192, BULLET_PLAYER_K43, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);

    PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), fUseAutoAim ? m_usFireSniper : m_usFireSniper, 0.0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, 0, 0, (m_iClip == 0) ? 1 : 0, 0);

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

    if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        // HEV suit - indicate out of ammo condition
        m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

  //  m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}

void CSniperrifle::Reload(void)
{
    if (m_fInZoom)
    {
        SecondaryAttack();
    }

    if (m_pPlayer->ammo_wrifle <= 0)
        return;

    if (m_pPlayer->pev->fov != 0)
    {
        m_fInZoom = FALSE;
        m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
    }

    /*if (m_iClip == 0)*/
        DefaultReload(5, SNIPERRIFLE_RELOAD, 5.6);
    //else
    //    DefaultReload(5, SNIPERRIFLE_RELOAD2, 3);

}


void CSniperrifle::WeaponIdle(void)
{
    m_pPlayer->GetAutoaimVector(AUTOAIM_2DEGREES); // get the autoaim vector but ignore it; used for autoaim crosshair in DM

    if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
    {
        float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
        if (flRand <= 0.75)
        {
            if (m_iClip)
            {
                SendWeaponAnim(SNIPERRIFLE_SLOWIDLE);
            }
            else
            {
                SendWeaponAnim(SNIPERRIFLE_SLOWIDLEEMPTY);
            }
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 12.0;
        }

    }
}

class CSniperrifleAmmo : public CBasePlayerAmmo
{
    void Spawn(void)
    {
        Precache();
        SET_MODEL(ENT(pev), "models/w_sniper_clip.mdl");
        CBasePlayerAmmo::Spawn();
    }
    void Precache(void)
    {
        PRECACHE_MODEL("models/w_sniper_clip.mdl");
        PRECACHE_SOUND("items/9mmclip1.wav");
    }
    BOOL AddAmmo(CBaseEntity* pOther)
    {
        if (pOther->GiveAmmo(10, "k43", 120) != -1)
        {
            EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
            return TRUE;
        }
        return FALSE;
    }
};
LINK_ENTITY_TO_CLASS(ammo_k43, CSniperrifleAmmo);

#endif