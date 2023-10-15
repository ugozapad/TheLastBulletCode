//Start SpaceArmor
class CSpaceArmor : public CItemBattery
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/newArmor/w_spacearmor.mdl");
		CItem::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/newArmor/w_spacearmor.mdl");
		PRECACHE_SOUND("TLB/armor/spacearmor.wav");
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->pev->deadflag != DEAD_NO)
		{
			return FALSE;
		}

		if ((pPlayer->pev->armorvalue < MAX_NORMAL_BATTERY) &&
			(pPlayer->pev->weapons & (1 << WEAPON_SUIT)))
		{
			int pct;
			char szcharge[64];

			pPlayer->pev->armorvalue += 65;
			pPlayer->pev->armorvalue = min(pPlayer->pev->armorvalue, MAX_NORMAL_BATTERY);

			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "TLB/armor/spacearmor.wav", 1, ATTN_NORM);

			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
			WRITE_STRING(STRING(pev->classname));
			MESSAGE_END();


			// Suit reports new power level
			// For some reason this wasn't working in release build -- round it.
			pct = (int)((float)(pPlayer->pev->armorvalue * 100.0) * (1.0 / MAX_NORMAL_BATTERY) + 0.5);
			pct = (pct / 5);
			if (pct > 0)
				pct--;

			sprintf(szcharge, "!HEV_%1dP", pct);

			//EMIT_SOUND_SUIT(ENT(pev), szcharge);
			pPlayer->SetSuitUpdate(szcharge, FALSE, SUIT_NEXT_IN_30SEC);
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(item_spacearmor, CSpaceArmor);

//End Space Armor


//Start Bonus Armor
class CBonusArmor : public CItemBattery
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/newArmor/w_bonusarm.mdl");
		CItem::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/newArmor/w_bonusarm.mdl");
		PRECACHE_SOUND("TLB/armor/spacearmor.wav");
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->pev->deadflag != DEAD_NO)
		{
			return FALSE;
		}

		if ((pPlayer->pev->armorvalue < MAX_NORMAL_BATTERY) &&
			(pPlayer->pev->weapons & (1 << WEAPON_SUIT)))
		{
			int pct;
			char szcharge[64];

			pPlayer->pev->armorvalue += 25;
			pPlayer->pev->armorvalue = min(pPlayer->pev->armorvalue, MAX_NORMAL_BATTERY);

			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "TLB/armor/spacearmor.wav", 1, ATTN_NORM);

			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
			WRITE_STRING(STRING(pev->classname));
			MESSAGE_END();


			// Suit reports new power level
			// For some reason this wasn't working in release build -- round it.
			pct = (int)((float)(pPlayer->pev->armorvalue * 100.0) * (1.0 / MAX_NORMAL_BATTERY) + 0.5);
			pct = (pct / 5);
			if (pct > 0)
				pct--;

			sprintf(szcharge, "!HEV_%1dP", pct);

			//EMIT_SOUND_SUIT(ENT(pev), szcharge);
			pPlayer->SetSuitUpdate(szcharge, FALSE, SUIT_NEXT_IN_30SEC);
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(item_bonusarmor, CBonusArmor);

//End Bonus Armor
