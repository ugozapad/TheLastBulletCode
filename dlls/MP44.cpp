#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "soundent.h"//подключаем новые файлы
#include "gamerules.h"

enum mp5_e//загружаем анимации
{
	MP5_LONGIDLE = 0,//Длинное бездействие
	MP5_IDLE1,//Бездействие
	MP5_LAUNCH,//Выстрел подстволкой
	MP5_RELOAD,//Перезарядка
	MP5_DEPLOY,//Взятие в руки
	MP5_FIRE1,//Выстрел
	MP5_FIRE2,//Выстрел
	MP5_FIRE3,//Выстрел
};

LINK_ENTITY_TO_CLASS(weapon_mp44, CMP44);

void CMP44::Spawn()
{
	pev->classname = MAKE_STRING("weapon_mp44"); // здесь имя энтити которое надо будет затем вписать в фгд
	Precache();
	SET_MODEL(ENT(pev), "models/w_mp44.mdl"); // Тут должна быть модель вашего оружия
	m_iId = WEAPON_MP44;

	m_iDefaultAmmo = 50;

	FallInit();
}

void CMP44::Precache(void)
{
	// Тут вы прекешите звуки и модели для вашего оружия
	PRECACHE_MODEL("models/v_mp44.mdl");
	PRECACHE_MODEL("models/w_mp44.mdl");
	PRECACHE_MODEL("models/p_mp44.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");

	PRECACHE_SOUND("weapons/hks1.wav");
	PRECACHE_SOUND("weapons/hks2.wav");
	PRECACHE_SOUND("weapons/hks3.wav");

	// this is to hook your client-side event
	m_usMp44Fire = PRECACHE_EVENT(1, "events/mp44.sc");
}

int CMP44::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "mp44"; //наше имя боеприпасов (чтобы не расходовать запас MP5) 
	p->iMaxAmmo1 = 200; // максимально количество патронов с собой
	p->iMaxClip = 45; //количество патронов в обойме
	p->iSlot = 2; //Слот оружия начиная с 0
	p->iPosition = 4; //Позиция в слоте начиная с 0
	p->iId = m_iId = WEAPON_MP44;
	p->iWeight = MP5_WEIGHT; //Вес оружия
	p->pszAmmo2 = NULL; //Нет вторичной амуниции
	p->iMaxAmmo2 = -1; //Нет вторичных боеприпасов

	return 1;
}





int CMP44::AddToPlayer(CBasePlayer *pPlayer)
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

BOOL CMP44::Deploy()//Поднимаем оружие
{
	return DefaultDeploy("models/v_mp44.mdl"/* модель в руках игрока*/, "models/p_mp44.mdl"/*модель в руках противника*/, MP5_DEPLOY/*анимация      поднятия*/, "mp5"/*перфикс анимации игрока*/);
}

void CMP44::WeaponIdle(void) //Бездействие
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > gpGlobals->time)
		return;

	int iAnim; //обьявляем целую пременную которая будет храить номер случайной анимации
	switch (RANDOM_LONG(0, 1))//случайное число от 0 до 1 для выбора анимации
	{
	case 0:  //если 0 то длинное бездействие
		iAnim = MP5_LONGIDLE;
		break;

	default:
	case 1: //если не ноль, а что-то другое то просто бездействие
		iAnim = MP5_IDLE1;
		break;
	}

	SendWeaponAnim(iAnim); //проиграть ать анимацию которая содержится в переменной

	m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT(10, 15); // через сколько снова запустить функцию бездействия
}


void CMP44::PrimaryAttack()//Первичная атака
{

	//не стрелять под водой
	if (m_pPlayer->pev->waterlevel == 3)//если игрок под водой
	{
		PlayEmptySound();//играть звук невозможности стрелять
		m_flNextPrimaryAttack = 0.15;//когда следующий выстрел
		return;//выходим из этой функции
	}

	if (m_iClip <= 0)//если патронов 0 или меньше 0 то
	{
		PlayEmptySound();//играть звук невозможности стрелять
		m_flNextPrimaryAttack = 0.15;//когда следующий выстрел
		return;//выходим из этой функции
	}

		
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME; //устанавливаем громкость оружия
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH; //устанавливаем яркость вспышки оружия

	m_iClip--; //уменьшаем количество патронов на 1 


	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// проигрывание анимации стрельбы игроком
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	//присваиваем вектору vecSrc положение пушки
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);//присваиваем вектору vecAiming место попадания пули
	Vector vecDir;
	//теперь стреляем
	vecDir = m_pPlayer->FireBulletsPlayer(1/*количество пуль одновременно*/, vecSrc/*откуда вылетает пуля(положение пушки) */, vecAiming/*куда попадает пуля*/, VECTOR_CONE_1DEGREES/*векторный конус разброса*/, 8192/*дальность*/, BULLET_PLAYER_MP44AMM/*тип пуль (повреждение)*/, 2/* количество трассирующих пуль*/, 0, m_pPlayer->pev, m_pPlayer->random_seed);
		
	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	
	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usMp44Fire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0); //Посылаем эвент на клиент

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV костюм говорит что нет патронов
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;//когда следующий выстрел

	if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);


}


void CMP44::Reload(void)//Перезарядка
{
	DefaultReload(100/*максимальный размер обоймы*/, MP5_RELOAD/*анимация перезарядки*/, 4 /*время перезарядки*/);
}



class CMP44AMMO : public CBasePlayerAmmo //класс наших боеприпасов
{
	void Spawn(void)
	{
		Precache();//загружаем
		SET_MODEL(ENT(pev), "models/w_mp44_ammo.mdl");//устанавливаем модель энтити
		CBasePlayerAmmo::Spawn();//отображаем на карте
	}
	void Precache(void)//здесь загружаем звук и модель
	{
		PRECACHE_MODEL("models/w_mp44_ammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity *pOther) //добавление амуниции 
	{
		int bResult = (pOther->GiveAmmo(45/*сколько даём патронов*/, "mp44"/*какие патроны пополняем*/, 200/*максимальное количество патронов с собой*/) != -1);
		if (bResult)//если игрок взял
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);//играть звук поднятия патронов с земли
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_mp44, CMP44AMMO); //создаём патроны из класса