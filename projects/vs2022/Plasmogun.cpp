#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

#define BOLT_AIR_VELOCITY 900
#define BOLT_WATER_VELOCITY 900

// UNDONE: Save/restore this? Don''t forget to set classname and LINK_ENTITY_TO_CLASS()
//
// OVERLOADS SOME ENTVARS:
//
// speed - the ideal magnitude of my velocity
class CNailBolt : public CBaseEntity
{
	void Spawn(void);
	void Precache(void);
	int Classify(void);
	void EXPORT BubbleThink(void);
	void EXPORT BoltTouch(CBaseEntity* pOther);
	void EXPORT ExplodeThink(void);

	int m_iTrail;

public:
	static CNailBolt* BoltCreate(void);
};
LINK_ENTITY_TO_CLASS(nail_bolt, CNailBolt);

CNailBolt* CNailBolt::BoltCreate(void)
{
	// Create a new entity with CCrossbowBolt private data
	CNailBolt* pBolt = GetClassPtr((CNailBolt*)NULL);
	pBolt->pev->classname = MAKE_STRING("nail");
	pBolt->Spawn();

	return pBolt;
}

void CNailBolt::Spawn()
{
	Precache();
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;

	pev->gravity = 0.5;

	SET_MODEL(ENT(pev), "models/shell.mdl");

	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	SetTouch(BoltTouch);
	SetThink(BubbleThink);
	pev->nextthink = gpGlobals->time + 0.15;
}


void CNailBolt::Precache()
{
	PRECACHE_MODEL("models/shell.mdl");
	PRECACHE_SOUND("weapons/xbow_hitbod1.wav");
	PRECACHE_SOUND("weapons/xbow_hitbod2.wav");
	PRECACHE_SOUND("weapons/xbow_fly1.wav");
	PRECACHE_SOUND("weapons/xbow_hit1.wav");
	PRECACHE_SOUND("fvox/beep.wav");
	m_iTrail = PRECACHE_MODEL("sprites/streak.spr");
}


int CNailBolt::Classify(void)
{
	return CLASS_NONE;
}

void CNailBolt::BoltTouch(CBaseEntity* pOther)
{
	SetTouch(NULL);
	SetThink(NULL);

	if (pOther->pev->takedamage)
	{
		TraceResult tr = UTIL_GetGlobalTrace();
		entvars_t* pevOwner;

		pevOwner = VARS(pev->owner);

		// UNDONE: this needs to call TraceAttack instead
		ClearMultiDamage();

		if (pOther->IsPlayer())
		{
			pOther->TraceAttack(pevOwner, gSkillData.plrDmgCrossbowClient, pev->velocity.Normalize(), &tr; , DMG_NEVERGIB);
		}
		else
		{
			pOther->TraceAttack(pevOwner, gSkillData.plrDmgCrossbowMonster, pev->velocity.Normalize(), &tr; , DMG_BULLET | DMG_NEVERGIB);
		}

		ApplyMultiDamage(pev, pevOwner);

		pev->velocity = Vector(0, 0, 0);
		// play body "thwack" sound
		switch (RANDOM_LONG(0, 1))
		{
		case 0:
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/xbow_hitbod1.wav", 1, ATTN_NORM); break;
		case 1:
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/xbow_hitbod2.wav", 1, ATTN_NORM); break;
		}

		if (!g_pGameRules->IsMultiplayer())
		{
			Killed(pev, GIB_NEVER);
		}
	}
	else
	{
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/xbow_hit1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0, 7));

		SetThink(SUB_Remove);
		pev->nextthink = gpGlobals->time;// this will get changed below if the bolt is allowed to stick in what it hit.

		if (FClassnameIs(pOther->pev, "worldspawn"))
		{
			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = pev->velocity.Normalize();
			UTIL_SetOrigin(pev, pev->origin - vecDir * 12);
			pev->angles = UTIL_VecToAngles(vecDir);
			pev->solid = SOLID_NOT;
			pev->movetype = MOVETYPE_FLY;
			pev->velocity = Vector(0, 0, 0);
			pev->avelocity.z = 0;
			pev->angles.z = RANDOM_LONG(0, 360);
			pev->nextthink = gpGlobals->time + 10.0;
		}

		if (UTIL_PointContents(pev->origin) != CONTENTS_WATER)
		{
			UTIL_Sparks(pev->origin);
		}
	}

	if (g_pGameRules->IsMultiplayer())
	{
		SetThink(ExplodeThink);
		pev->nextthink = gpGlobals->time + 0.15;
	}
}

void CNailBolt::BubbleThink(void)
{
	pev->nextthink = gpGlobals->time + 0.15;

	if (pev->waterlevel == 0)
		return;

	UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 1);
}

void CNailBolt::ExplodeThink(void)
{
	int iContents = UTIL_PointContents(pev->origin);

	pev->dmg = 40;

	entvars_t* pevOwner;

	if (pev->owner)
		pevOwner = VARS(pev->owner);
	else
		pevOwner = NULL;

	pev->owner = NULL; // can''t traceline attack owner if this is set


	UTIL_Remove(this);
}

enum nailgun_e {
	NAILGUN_IDLE1 = 0, // full
	NAILGUN_LONGIDLE, // empty
	NAILGUN_SHOOT, // full
	NAILGUN_RELOAD, // from empty
};


class CNailgun : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int iItemSlot() { return 3; }
	int GetItemInfo(ItemInfo* p);

	void FireBolt(void);
	void FireSniperBolt(void);
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	int AddToPlayer(CBasePlayer* pPlayer);
	BOOL Deploy();
	void Holster(int skiplocal = 0);
	void Reload(void);
	void WeaponIdle(void);

	int m_fInZoom; // don''t save this
};
LINK_ENTITY_TO_CLASS(weapon_nailgun, CNailgun);

void CNailgun::Spawn()
{
	Precache();
	m_iId = WEAPON_NAILGUN;
	SET_MODEL(ENT(pev), "models/w_crossbow.mdl");

	m_iDefaultAmmo = NAILGUN_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}

int CNailgun::AddToPlayer(CBasePlayer* pPlayer)
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

void CNailgun::Precache(void)
{
	PRECACHE_MODEL("models/w_crossbow.mdl");
	PRECACHE_MODEL("models/v_crossbow.mdl");
	PRECACHE_MODEL("models/p_crossbow.mdl");

	PRECACHE_SOUND("weapons/xbow_fire1.wav");
	PRECACHE_SOUND("weapons/xbow_reload1.wav");

	UTIL_PrecacheOther("nailgun_nail");
}


int CNailgun::GetItemInfo(ItemInfo* p)
{


	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "nail";
	p->iMaxAmmo1 = 80;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 80;
	p->iSlot = 2;
	p->iPosition = 3;
	p->iId = WEAPON_NAILGUN;
	p->iFlags = 0;
	p->iWeight = NAILGUN_WEIGHT;
	return 1;
}


BOOL CNailgun::Deploy()
{
	if (m_iClip)
		return DefaultDeploy("models/v_crossbow.mdl", "models/p_crossbow.mdl", NAILGUN_SHOOT, "nail");
	return DefaultDeploy("models/v_crossbow.mdl", "models/p_crossbow.mdl", NAILGUN_SHOOT, "nail");
}

void CNailgun::Holster(int skiplocal /* = 0 */)
{
	m_fInReload = FALSE;// cancel any reload in progress.

	if (m_fInZoom)
	{
		SecondaryAttack();
	}

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.15;


}

void CNailgun::PrimaryAttack(void)
{

	{
		SendWeaponAnim(NAILGUN_SHOOT);
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
	}


	FireBolt();


}

// this function only gets called in multiplayer
void CNailgun::FireSniperBolt()
{
	m_flNextPrimaryAttack = gpGlobals->time + 0.15;

	if (m_iClip == 0)
	{
		PlayEmptySound();
		return;
	}

	TraceResult tr;

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
	m_iClip--;

	// make twang sound


	if (m_iClip)
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/xbow_reload1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0, 0xF));
	}
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 0)
	{

	}

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	Vector anglesAim = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
	UTIL_MakeVectors(anglesAim);
	Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2;
	Vector vecDir = gpGlobals->v_forward;

	UTIL_TraceLine(vecSrc, vecSrc + vecDir * 8192, dont_ignore_monsters, m_pPlayer->edict(), &tr;);

	if (tr.pHit->v.takedamage)
	{
		switch (RANDOM_LONG(0, 1))
		{
		case 0:
			EMIT_SOUND(tr.pHit, CHAN_BODY, "weapons/xbow_hitbod1.wav", 1, ATTN_NORM); break;
		case 1:
			EMIT_SOUND(tr.pHit, CHAN_BODY, "weapons/xbow_hitbod2.wav", 1, ATTN_NORM); break;
		}

		ClearMultiDamage();
		CBaseEntity::Instance(tr.pHit)->TraceAttack(m_pPlayer->pev, 10, vecDir, &tr; , DMG_BULLET | DMG_NEVERGIB);
		ApplyMultiDamage(pev, m_pPlayer->pev);
	}
	else
	{
		// create a bolt
		CNailgunBolt* pBolt = CNailgunBolt::BoltCreate();
		pBolt->pev->origin = vecSrc;
		pBolt->pev->angles = anglesAim;
		pBolt->pev->solid = SOLID_NOT;
		pBolt->SetTouch(NULL);
		pBolt->SetThink(SUB_Remove);

		if (m_pPlayer->pev->waterlevel == 3)
		{
			pBolt->pev->velocity = vecDir * BOLT_WATER_VELOCITY;
			pBolt->pev->speed = BOLT_WATER_VELOCITY;
		}
		else
		{
			pBolt->pev->velocity = vecDir * BOLT_AIR_VELOCITY;
			pBolt->pev->speed = BOLT_AIR_VELOCITY;
		}
		pBolt->pev->avelocity.z = 10;
		EMIT_SOUND(pBolt->edict(), CHAN_WEAPON, "weapons/xbow_hit1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM);

		if (UTIL_PointContents(tr.vecEndPos) != CONTENTS_WATER)
		{
			UTIL_Sparks(tr.vecEndPos);
		}

		if (FClassnameIs(tr.pHit, "worldspawn"))
		{
			// let the bolt sit around for a while if it hit static architecture
			pBolt->pev->nextthink = gpGlobals->time + 7.0;
		}
		else
		{
			pBolt->pev->nextthink = gpGlobals->time;
		}
	}
}

void CNailgun::FireBolt()
{
	TraceResult tr;

	if (m_iClip == 0)
	{
		PlayEmptySound();
		return;
	}

	SendWeaponAnim(NAILGUN_SHOOT);
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

	m_iClip--;

	// make twang sound
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/xbow_fire1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0, 0xF));

	if (m_iClip)
	{


		SendWeaponAnim(NAILGUN_SHOOT);
	}
	else if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 0)
	{
		SendWeaponAnim(NAILGUN_SHOOT);
	}

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	SendWeaponAnim(NAIL_SHOOT);
	Vector anglesAim = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
	UTIL_MakeVectors(anglesAim);

	// Vector vecSrc = pev->origin + gpGlobals->v_up * 16 + gpGlobals->v_forward * 20 + gpGlobals->v_right * 4;
	anglesAim.x = -anglesAim.x;
	Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2;
	Vector vecDir = gpGlobals->v_forward;

	//CBaseEntity *pBolt = CBaseEntity::Create( "nailgun_bolt", vecSrc, anglesAim, m_pPlayer->edict() );
	CNailgunBolt* pBolt = CNailgunBolt::BoltCreate();
	pBolt->pev->origin = vecSrc;
	pBolt->pev->angles = anglesAim;
	pBolt->pev->owner = m_pPlayer->edict();

	if (m_pPlayer->pev->waterlevel == 3)
	{
		pBolt->pev->velocity = vecDir * BOLT_WATER_VELOCITY;
		pBolt->pev->speed = BOLT_WATER_VELOCITY;
	}
	else
	{
		pBolt->pev->velocity = vecDir * BOLT_AIR_VELOCITY;
		pBolt->pev->speed = BOLT_AIR_VELOCITY;
	}
	pBolt->pev->avelocity.z = 10;

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = gpGlobals->time + 0.08;

	m_flNextSecondaryAttack = gpGlobals->time + 0.08;
	if (m_iClip != 0)
		m_flTimeWeaponIdle = gpGlobals->time + 2.0;
	else
		m_flTimeWeaponIdle = 0.75;

	m_pPlayer->pev->punchangle.x -= 0;
}


void CNailgun::SecondaryAttack()
{

}


void CNailgun::Reload(void)
{
	if (m_fInZoom)
	{
		SecondaryAttack();
	}

	if (DefaultReload(200, CROSSBOW_RELOAD, 2.5))
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/xbow_reload1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0, 0xF));
	}
}


void CNailgun::WeaponIdle(void)
{
	m_pPlayer->GetAutoaimVector(AUTOAIM_2DEGREES); // get the autoaim vector but ignore it; used for autoaim crosshair in DM

	ResetEmptySound();

	if (m_flTimeWeaponIdle < gpGlobals->time)
	{
		float flRand = RANDOM_FLOAT(0, 1);
		if (flRand <= 0.75)
		{
			if (m_iClip)
			{
				SendWeaponAnim(NAILGUN_IDLE1);
			}
			else
			{
				SendWeaponAnim(NAILGUN_LONGIDLE);
			}
			m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT(10, 15);
		}
		else
		{
			if (m_iClip)
			{

				m_flTimeWeaponIdle = gpGlobals->time + 90.0 / 30.0;
			}
			else
			{

				m_flTimeWeaponIdle = gpGlobals->time + 80.0 / 30.0;
			}
		}
	}
}



class CNailgunAmmo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_crossbow_clip.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_crossbow_clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		if (pOther->GiveAmmo(AMMO_NAILGUNCLIP_GIVE, "nail", NAIL_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS(ammo_nailgun, CNailgunAmmo);



#endif