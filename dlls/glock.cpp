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
#include "player.h"

LINK_ENTITY_TO_CLASS(weapon_glock, CGlock);
LINK_ENTITY_TO_CLASS(weapon_9mmhandgun, CGlock);

void CGlock::Spawn()
{
	pev->classname = MAKE_STRING("weapon_9mmhandgun"); // hack to allow for old names
	Precache();
	m_iId = WEAPON_GLOCK;
	if (m_bIsSilenced)
		SET_MODEL(ENT(pev), "models/w_9mmhandgun_silenced.mdl");
	else
		SET_MODEL(ENT(pev), "models/w_9mmhandgun.mdl");

	m_iDefaultAmmo = GLOCK_DEFAULT_GIVE;

	FallInit(); // get ready to fall down.
}


void CGlock::Precache()
{
	PRECACHE_MODEL("models/v_9mmhandgun.mdl");
	PRECACHE_MODEL("models/v_9mmhandgun_silenced.mdl");
	PRECACHE_MODEL("models/v_9mmhandgun_inv.mdl");
	PRECACHE_MODEL("models/v_9mmhandgun_silenced_inv.mdl");
	PRECACHE_MODEL("models/w_9mmhandgun.mdl");
	PRECACHE_MODEL("models/w_9mmhandgun_silenced.mdl");
	PRECACHE_MODEL("models/p_9mmhandgun.mdl");
	PRECACHE_MODEL("models/p_9mmhandgun_silenced.mdl");

	m_iShell = PRECACHE_MODEL("models/shell.mdl"); // brass shell
	m_iClipMdl = PRECACHE_MODEL("models/w_9mmclip.mdl"); // empty clip
	m_bIsSilenced = true;

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");

	PRECACHE_SOUND("weapons/pl_gun1.wav"); //silenced handgun
	PRECACHE_SOUND("weapons/pl_gun2.wav"); //silenced handgun
	PRECACHE_SOUND("weapons/pl_gun3.wav"); //handgun

	PRECACHE_SOUND("weapons/usp_silencer_on.wav");
	PRECACHE_SOUND("weapons/usp_silencer_off.wav");

	PRECACHE_SOUND("weapons/9mm_clipout.wav");
	PRECACHE_SOUND("weapons/9mm_draw.wav");
	PRECACHE_SOUND("weapons/9mm_clip.wav");
	PRECACHE_SOUND("weapons/9mm_in.wav");
	PRECACHE_SOUND("weapons/9mm_cock1.wav");
	PRECACHE_SOUND("weapons/9mm_cock2.wav");	

	m_usFireGlock1 = PRECACHE_EVENT(1, "events/glock1.sc");
	m_usFireGlock2 = PRECACHE_EVENT(1, "events/glock2.sc");
}

bool CGlock::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GLOCK_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_GLOCK;
	p->iWeight = GLOCK_WEIGHT;

	return true;
}

void CGlock::IncrementAmmo(CBasePlayer* pPlayer)
{
	if (pPlayer->GiveAmmo(1, "9mm", _9MM_MAX_CARRY) >= 0)
	{
		EMIT_SOUND(pPlayer->edict(), CHAN_STATIC, "ctf/pow_backpack.wav", 0.5, ATTN_NORM);
	}
}

bool CGlock::Deploy()
{
	m_bIsHolstered = false;

	if (m_bIsSilenced) {
		if (m_pPlayer->m_bIsCloaked)
			return DefaultDeploy("models/v_9mmhandgun_silenced_inv.mdl", "models/p_9mmhandgun_silenced.mdl", GLOCK_DRAW, "onehanded");
		return DefaultDeploy("models/v_9mmhandgun_silenced.mdl", "models/p_9mmhandgun_silenced.mdl", GLOCK_DRAW, "onehanded");
	}

	if (m_pPlayer->m_bIsCloaked)
		return DefaultDeploy("models/v_9mmhandgun_inv.mdl", "models/p_9mmhandgun.mdl", GLOCK_DRAW, "onehanded");
	return DefaultDeploy("models/v_9mmhandgun.mdl", "models/p_9mmhandgun.mdl", GLOCK_DRAW, "onehanded");
}

void CGlock::UpdateVModel()
{
	if (!m_pPlayer->m_bIsCloaked)
	{
		if (m_bIsSilenced)
		{
#ifndef CLIENT_DLL
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_9mmhandgun_silenced.mdl");
#else
			LoadVModel("models/v_9mmhandgun_silenced.mdl", m_pPlayer);
#endif
		}
		else 
		{
#ifndef CLIENT_DLL
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_9mmhandgun.mdl");
#else
			LoadVModel("models/v_9mmhandgun.mdl", m_pPlayer);
#endif
		}
	}
	else
	{
		if (m_bIsSilenced)
		{
#ifndef CLIENT_DLL
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_9mmhandgun_silenced_inv.mdl");
#else
			LoadVModel("models/v_9mmhandgun_silenced_inv.mdl", m_pPlayer);
#endif
		}
		else
		{
#ifndef CLIENT_DLL
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_9mmhandgun_inv.mdl");
#else
			LoadVModel("models/v_9mmhandgun_inv.mdl", m_pPlayer);
#endif
		}
	}
}

void CGlock::Holster()
{
	m_fInReload = false; // cancel any reload in progress.

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim(GLOCK_HOLSTER);
	m_bIsHolstered = true;
}

void CGlock::SecondaryAttack()
{
	if (m_pPlayer->m_afButtonLast & IN_ATTACK2 || m_bIsHolstered)
		return;

	Holster();
	UpdateVModel();

	if (m_bIsSilenced)
	{
		if (!m_pPlayer->m_bIsCloaked)
		{
#ifndef CLIENT_DLL
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_9mmhandgun.mdl");
#else
			LoadVModel("models/v_9mmhandgun.mdl", m_pPlayer);
#endif
		}
		else
		{
#ifndef CLIENT_DLL
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_9mmhandgun_inv.mdl");
#else
			LoadVModel("models/v_9mmhandgun_inv.mdl", m_pPlayer);
#endif
		}
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_9mmhandgun.mdl");
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/usp_silencer_off.wav", 1, ATTN_NORM);
		m_bIsSilenced = false;
	}
	else
	{
		if (!m_pPlayer->m_bIsCloaked)
		{
#ifndef CLIENT_DLL
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_9mmhandgun_silenced.mdl");
#else
			LoadVModel("models/v_9mmhandgun_silenced.mdl", m_pPlayer);
#endif
		}
		else
		{
#ifndef CLIENT_DLL
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_9mmhandgun_silenced_inv.mdl");
#else
			LoadVModel("models/v_9mmhandgun_silenced_inv.mdl", m_pPlayer);
#endif
		}
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_9mmhandgun_silenced.mdl");
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/usp_silencer_on.wav", 1, ATTN_NORM);
		m_bIsSilenced = true;
	}
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(2.0);
}

void CGlock::PrimaryAttack()
{
	if (m_bIsSilenced) {
		GlockFire(0.015, 0.1, true);
	}
	else {
		GlockFire(0.020, 0.32, false);
	}
}

void CGlock::GlockFire(float flSpread, float flCycleTime, bool fUseAutoAim)
{
	if (m_pPlayer->m_afButtonLast & IN_ATTACK && fUseAutoAim)
		return;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(0.2);
		}

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
	if (m_bIsSilenced)
	{
		m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
		switch (RANDOM_LONG(0, 1))
		{
		case 0:
			EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/pl_gun1.wav", 1, ATTN_NORM);
			break;
		case 1:
			EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/pl_gun2.wav", 1, ATTN_NORM);
			break;
		}
	}
	else
	{
		// non-silenced
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/pl_gun3.wav", 1, ATTN_NORM);
	}

	// Eject the brass
	Vector vecShellVelocity = m_pPlayer->pev->velocity + gpGlobals->v_right * RANDOM_FLOAT(100, 200) +
					   gpGlobals->v_up * RANDOM_FLOAT(100, 150) + gpGlobals->v_forward * 25;
	EjectBrass(pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -12 + gpGlobals->v_forward * 20 +
				   gpGlobals->v_right * 8,
		vecShellVelocity, pev->angles.y, m_iShell, TE_BOUNCE_SHELL);

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

	if (0 == m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);

    // Play view model animation and firing sound
	SendWeaponAnim(GLOCK_SHOOT);

    // Punch the camera to simulate recoil
    m_pPlayer->pev->punchangle.x -= 2;
}


void CGlock::Reload()
{
	if (m_pPlayer->ammo_9mm <= 0)
		return;

	bool iResult;
	Vector vecShellVelocity = m_pPlayer->pev->velocity + gpGlobals->v_right * RANDOM_FLOAT(50, 100) +
							  gpGlobals->v_up * RANDOM_FLOAT(100, 150) + gpGlobals->v_forward * 25;

	if (m_iClip == 0) {
		iResult = DefaultReload(17, GLOCK_RELOAD, 2.7);
		if (iResult)
		{
			EjectBrass(pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -12 + gpGlobals->v_forward * 20 +
						   gpGlobals->v_right * 8,
				vecShellVelocity, pev->angles.y, m_iClipMdl, BOUNCE_METAL);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
		}
	}
	else
		iResult = DefaultReload(17, GLOCK_RELOAD_TACTICAL, 2.2);
}



void CGlock::WeaponIdle()
{
	ResetEmptySound();
	UpdateVModel();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_bIsHolstered)
		Deploy();

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.0, 1.0);

		if (flRand <= 0.3 + 0 * 0.75)
		{
			iAnim = GLOCK_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 49.0 / 16;
		}
		else if (flRand <= 0.6 + 0 * 0.875)
		{
			iAnim = GLOCK_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 16.0;
		}
		else
		{
			iAnim = GLOCK_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
		}
		SendWeaponAnim(iAnim);
	}
}


class CGlockAmmo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_9mmclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBaseEntity* pOther) override
	{
		if (pOther->GiveAmmo(AMMO_GLOCKCLIP_GIVE, "9mm", _9MM_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_glockclip, CGlockAmmo);
LINK_ENTITY_TO_CLASS(ammo_9mmclip, CGlockAmmo);
