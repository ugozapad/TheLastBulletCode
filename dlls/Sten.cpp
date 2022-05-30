#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#include "soundent.h"//подключаем новые файлы
#include "gamerules.h"

enum sten_e//загружаем анимации
{
	STEN_LONGIDLE = 0,//Длинное бездействие
	STEN_IDLE1,//Бездействие
	STEN_LAUNCH,//Выстрел подстволкой
	STEN_RELOAD,//Перезарядка
	STEN_DEPLOY,//Взятие в руки
	STEN_FIRE1,//Выстрел
	STEN_FIRE2,//Выстрел
	STEN_FIRE3,//Выстрел
};

LINK_ENTITY_TO_CLASS(weapon_sten, CStenGun);

void CStenGun::Spawn()
{
	pev->classname = MAKE_STRING("weapon_sten"); // здесь имя энтити которое надо будет затем вписать в фгд
	Precache();
	SET_MODEL(ENT(pev), "models/w_sten.mdl"); // Тут должна быть модель вашего оружия
	m_iId = WEAPON_STEN;

	m_iDefaultAmmo = 50;

	FallInit();
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

int CStenGun::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "mp5";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->iMaxClip = MP5_MAX_CLIP;
	p->iSlot = 2; // слот в худе ( помните также, что елси здесь вы написали например 2 то в игре будет занят 3 слот, т.к в коде исчесление начинаеться от нуля)
	p->iPosition = 3; // позиция в слоте ( то же самое что и в слоте - елси вам надо занять 4 позицию , то пишите 3 в коде)
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_STEN;
	p->iWeight = MP5_WEIGHT; //
	p->pszAmmo2 = NULL; //
	p->iMaxAmmo2 = -1;

	return 1;
}


int CStenGun::AddToPlayer(CBasePlayer *pPlayer)
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


BOOL CStenGun::Deploy()//Поднимаем оружие
{
	return DefaultDeploy("models/v_sten.mdl"/* модель в руках игрока*/, "models/p_sten.mdl"/*модель в руках противника*/, STEN_DEPLOY/*анимация      поднятия*/, "mp5"/*перфикс анимации игрока*/);
}



void CStenGun::WeaponIdle(void) //Бездействие
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > gpGlobals->time)
		return;

	int iAnim; //обьявляем целую пременную которая будет храить номер случайной анимации
	switch (RANDOM_LONG(0, 1))//случайное число от 0 до 1 для выбора анимации
	{
	case 0:  //если 0 то длинное бездействие
		iAnim = STEN_LONGIDLE;
		break;

	default:
	case 1: //если не ноль, а что-то другое то просто бездействие
		iAnim = STEN_IDLE1;
		break;
	}

	SendWeaponAnim(iAnim); //проиграть ать анимацию которая содержится в переменной

	m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT(10, 15); // через сколько снова запустить функцию бездействия
}

void CStenGun::PrimaryAttack()//Первичная атака
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
	vecDir = m_pPlayer->FireBulletsPlayer(1/*количество пуль одновременно*/, vecSrc/*откуда вылетает пуля(положение пушки) */, vecAiming/*куда попадает пуля*/, VECTOR_CONE_1DEGREES/*векторный конус разброса*/, 8192/*дальность*/, BULLET_PLAYER_MP5/*тип пуль (повреждение)*/, 2/* количество трассирующих пуль*/, 0, m_pPlayer->pev, m_pPlayer->random_seed);


	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usStenFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0); //Посылаем эвент на клиент

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV костюм говорит что нет патронов
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;//когда следующий выстрел

	if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);

}

void CStenGun::Reload(void)//Перезарядка
{
	DefaultReload(100/*максимальный размер обоймы*/, STEN_RELOAD/*анимация перезарядки*/, 4 /*время перезарядки*/);
}





