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
#include "player.h"

#include "CElite.h"

LINK_ENTITY_TO_CLASS(weapon_dualies, CElite);
LINK_ENTITY_TO_CLASS(weapon_elite, CElite);

void CElite::Spawn()
{
	pev->classname = MAKE_STRING("weapon_elite"); // hack to allow for old names
	Precache();
	m_iId = WEAPON_ELITE;
	SET_MODEL(ENT(pev), "models/w_9mmhandgun_silenced.mdl");

	m_iDefaultAmmo = GLOCK_DEFAULT_GIVE * 2;

	FallInit(); // get ready to fall down.
}

void CElite::Precache()
{
	PRECACHE_MODEL("models/v_elite.mdl");
	PRECACHE_MODEL("models/v_elite_inv.mdl");
	PRECACHE_MODEL("models/w_9mmhandgun_silenced.mdl");
	PRECACHE_MODEL("models/p_elite.mdl");

	m_iShell = PRECACHE_MODEL("models/shell.mdl"); // brass shell
	m_iClip1 = PRECACHE_MODEL("models/w_9mmclip.mdl"); // empty clip
	left = true;

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");

	PRECACHE_SOUND("weapons/elite_reloadstart.wav");
	PRECACHE_SOUND("weapons/elite_leftclipin.wav");
	PRECACHE_SOUND("weapons/elite_rightclipin.wav");
	PRECACHE_SOUND("weapons/elite_sliderelease.wav");
	PRECACHE_SOUND("weapons/elite_clipout.wav");
	PRECACHE_SOUND("weapons/elite_deploy.wav");

	PRECACHE_SOUND("weapons/pl_gun1.wav");
	PRECACHE_SOUND("weapons/pl_gun2.wav");

	PRECACHE_SOUND("weapons/desert_eagle_sight1.wav");
	PRECACHE_SOUND("weapons/desert_eagle_sight2.wav");
}

bool CElite::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = ELITE_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_ELITE;
	p->iWeight = GLOCK_WEIGHT + 2;

	return true;
}

void CElite::IncrementAmmo(CBasePlayer* pPlayer)
{
	if (pPlayer->GiveAmmo(1, "9mm", _9MM_MAX_CARRY) >= 0)
	{
		EMIT_SOUND(pPlayer->edict(), CHAN_STATIC, "ctf/pow_backpack.wav", 0.5, ATTN_NORM);
	}
}

bool CElite::Deploy()
{
	// pev->body = 1;
	if (m_pPlayer->m_bIsCloaked) {
		return DefaultDeploy("models/v_elite_inv.mdl", "models/p_elite.mdl", ELITE_DRAW, "trip");
	}
	return DefaultDeploy("models/v_elite.mdl", "models/p_elite.mdl", ELITE_DRAW, "trip");
}

void CElite::Holster()
{
	m_fInReload = false; // cancel any reload in progress.

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

#ifndef CLIENT_DLL
	if (m_pSpot)
	{
		m_pSpot->Killed(NULL, GIB_NEVER);
		m_pSpot = NULL;
	}
#endif
}

void CElite::SecondaryAttack()
{
	if (m_pPlayer->m_afButtonLast & IN_ATTACK2)
		return;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(1.2);

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2;
}

void CElite::PrimaryAttack()
{
	GlockFire(0.01, 0.1, true, left);
}

void CElite::GlockFire(float flSpread, float flCycleTime, bool fUseAutoAim, bool isLeftHandGun)
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(0.2);
		}

		return;
	}

	if (m_pPlayer->m_afButtonLast & IN_ATTACK)
	{
#ifndef CLIENT_DLL
		if (m_fSpotActive && m_pSpot)
		{
			m_pSpot->Killed(NULL, GIB_NORMAL);
			m_pSpot = NULL;
		}
#endif
		return;
	}

	UpdateVModel();

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	int flags;

#if defined(CLIENT_WEAPONS)
	flags = UTIL_DefaultPlaybackFlags();
#else
	flags = 0;
#endif

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	// silenced
	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

	// the 3rd entry defines the firing sound path. Make sure to precache first
	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/pl_gun1.wav", 1, ATTN_NORM);
		break;
	case 1:
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/pl_gun2.wav", 1, ATTN_NORM);
		break;
	}

	// Eject the brass
	Vector vecShellVelocity;

	if (isLeftHandGun)
	{
		SendWeaponAnim(ELITE_SHOOT_LEFT1 + RANDOM_FLOAT(0, 4));
		vecShellVelocity = m_pPlayer->pev->velocity + gpGlobals->v_right * RANDOM_FLOAT(100, 200) +
						   gpGlobals->v_up * RANDOM_FLOAT(100, 150) + gpGlobals->v_forward * 25;
		EjectBrass(pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -12 + gpGlobals->v_forward * 20 +
					   gpGlobals->v_right * -8,
			vecShellVelocity, pev->angles.y, m_iShell, TE_BOUNCE_SHELL);
	}
	else
	{
		SendWeaponAnim(ELITE_SHOOT_RIGHT1 + RANDOM_LONG(0, 4));
		vecShellVelocity = m_pPlayer->pev->velocity + gpGlobals->v_right * RANDOM_FLOAT(100, 200) +
						   gpGlobals->v_up * RANDOM_FLOAT(100, 150) + gpGlobals->v_forward * 25;
		EjectBrass(pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -12 + gpGlobals->v_forward * 20 +
					   gpGlobals->v_right * 8,
			vecShellVelocity, pev->angles.y, m_iShell, TE_BOUNCE_SHELL);
	}

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
	vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 8192, BULLET_PLAYER_9MM, 1, 10, m_pPlayer->pev, m_pPlayer->random_seed);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(flCycleTime);
	// m_flNextPrimaryAttack = GetNextAttackDelay(0.15);

	if (0 == m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);

	// flip this flag since we need to fire with the gun in the other hand next
	left = !left;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);

	// Punch the camera to simulate recoil
	m_pPlayer->pev->punchangle.x -= 2;
}


void CElite::Reload()
{
	if (m_pPlayer->ammo_9mm <= 0)
		return;

	Vector vecShellVelocity = m_pPlayer->pev->velocity + gpGlobals->v_right * RANDOM_FLOAT(50, 100) +
							  gpGlobals->v_up * RANDOM_FLOAT(100, 150) + gpGlobals->v_forward * 25;

	bool iResult;

	if (m_iClip == 0)
		iResult = DefaultReload(34, ELITE_RELOAD, 4.5);
	else
		iResult = DefaultReload(34, ELITE_RELOAD, 4.5);

	#ifndef CLIENT_DLL
	if (m_pSpot && m_fSpotActive)
	{
		m_pSpot->Suspend(2.1);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.1;
	}
	#endif

	if (iResult)
	{
		EjectBrass(pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -12 + gpGlobals->v_forward * 20 +
					   gpGlobals->v_right * -8,
			vecShellVelocity, pev->angles.y, m_iClip1, BOUNCE_METAL);
		EjectBrass(pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -12 + gpGlobals->v_forward * 20 +
					   gpGlobals->v_right * 8,
			vecShellVelocity, pev->angles.y, m_iClip1, BOUNCE_METAL);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
	}
}

void CElite::WeaponIdle()
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	UpdateSpot();
	UpdateVModel();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		SendWeaponAnim(ELITE_IDLE);
	}
}

void CElite::UpdateSpot()
{
#ifndef CLIENT_DLL
	// Don't turn on the laser if we're in the middle of a reload.
	if (m_fInReload)
	{
		return;
	}

	if (m_fSpotActive)
	{
		if (!m_pSpot)
		{
			m_pSpot = CLaserSpot::CreateSpot();
			m_pSpot->pev->scale = 0.15;
		}

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecAiming = gpGlobals->v_forward;

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

		UTIL_SetOrigin(m_pSpot->pev, tr.vecEndPos);
	}
#endif
}

void CElite::UpdateVModel() 
{
	if (!m_pPlayer->m_bIsCloaked)
	{
#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_elite.mdl");
#else
		LoadVModel("models/v_elite.mdl", m_pPlayer);
#endif
	}
	else
	{
#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_elite_inv.mdl");
#else
		LoadVModel("models/v_elite_inv.mdl", m_pPlayer);
#endif
	}
}
