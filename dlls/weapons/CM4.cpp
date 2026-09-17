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
#include "soundent.h"
#include "gamerules.h"
#include "UserMessages.h"

#include "CM4.h"

LINK_ENTITY_TO_CLASS(weapon_m4, CM4);


//=========================================================
//=========================================================
void CM4::Spawn()
{
	pev->classname = MAKE_STRING("weapon_m4"); // hack to allow for old names
	Precache();
	SET_MODEL(ENT(pev), "models/w_m4.mdl");
	m_iId = WEAPON_M4;

	m_iDefaultAmmo = MP5_DEFAULT_GIVE;

	m_flNextGrenadeLoad = gpGlobals->time;

	FallInit(); // get ready to fall down.
}


void CM4::Precache()
{
	PRECACHE_MODEL("models/v_m4.mdl");
	PRECACHE_MODEL("models/v_m4_inv.mdl");
	PRECACHE_MODEL("models/w_m4.mdl");
	PRECACHE_MODEL("models/p_m4.mdl");

	m_iShell = PRECACHE_MODEL("models/saw_shell.mdl"); // brass shellTE_MODEL

	PRECACHE_MODEL("models/grenade.mdl"); // grenade

	m_iClipMdl = PRECACHE_MODEL("models/w_m4clip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/f_wep_draw_7.wav");
	PRECACHE_SOUND("weapons/f_wep_draw_8.wav");
	PRECACHE_SOUND("weapons/w_m4_magout.wav");
	PRECACHE_SOUND("weapons/w_m4_magin.wav");
	PRECACHE_SOUND("weapons/f_wep_mow_hit_2.wav");

	PRECACHE_SOUND("weapons/m41.wav");		// M to the 4
	PRECACHE_SOUND("weapons/m42.wav");		// M to the 4
	PRECACHE_SOUND("weapons/m43.wav");		// M to the 4

	PRECACHE_SOUND("weapons/glauncher.wav");
	PRECACHE_SOUND("weapons/glauncher2.wav");

	PRECACHE_SOUND("weapons/357_cock1.wav");
}

bool CM4::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556";
	p->iMaxAmmo1 = M249_MAX_CARRY;
	p->pszAmmo2 = "ARgrenades";
	p->iMaxAmmo2 = M203_GRENADE_MAX_CARRY;
	p->iMaxClip = M249_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 3;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_M4;
	p->iWeight = MP5_WEIGHT * 2;

	return true;
}

void CM4::IncrementAmmo(CBasePlayer* pPlayer)
{
	if (pPlayer->GiveAmmo(1, "556", M249_MAX_CARRY) >= 0)
	{
		EMIT_SOUND(pPlayer->edict(), CHAN_STATIC, "ctf/pow_backpack.wav", 0.5, ATTN_NORM);
	}

	if (m_flNextGrenadeLoad < gpGlobals->time)
	{
		pPlayer->GiveAmmo(1, "ARgrenades", M203_GRENADE_MAX_CARRY);
		m_flNextGrenadeLoad = gpGlobals->time + 10;
	}
}

bool CM4::Deploy()
{
	if (m_pPlayer->m_bIsCloaked)
		return DefaultDeploy("models/v_m4_inv.mdl", "models/p_m4.mdl", M4_DEPLOY, "mp5");
	return DefaultDeploy("models/v_m4.mdl", "models/p_m4.mdl", M4_DEPLOY, "mp5");
}

void CM4::UpdateVModel()
{
	if (!m_pPlayer->m_bIsCloaked)
	{
#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_m4.mdl");
#else
		LoadVModel("models/v_m4.mdl", m_pPlayer);
#endif
	}
	else
	{
#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_m4_inv.mdl");
#else
		LoadVModel("models/v_m4_inv.mdl", m_pPlayer);
#endif
	}
}

void CM4::PrimaryAttack()
{
	/*if (m_pPlayer->m_afButtonLast & IN_ATTACK)
	{
		return;
	}
	
	if (m_iBurstState > BURST_IDLE)
	{
		return;
	}

	m_iBurstState = BURST_TRIGGER;*/

	UpdateVModel();

	M4Fire(0.025, 0.13, true);
}

void CM4::M4Fire(float flSpread, float flCycleTime, bool fUseAutoAim)
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

	// non-silenced
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	switch (RANDOM_LONG(0, 2))
	{
	case 0:
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/m41.wav", 1, ATTN_NORM);
		break;
	case 1:
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/m42.wav", 1, ATTN_NORM);
		break;
	default:
	case 2:
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/m43.wav", 1, ATTN_NORM);
		break;
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
	switch (RANDOM_LONG(0, 2))
	{
	case 0:
		SendWeaponAnim(M4_FIRE1);
		break;
	case 1:
		SendWeaponAnim(M4_FIRE2);
		break;

	default:
	case 2:
		SendWeaponAnim(M4_FIRE3);
		break;
	}

	// Punch the camera to simulate recoil
	m_pPlayer->pev->punchangle.x -= 1.05;
}

void CM4::SecondaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
	{
		PlayEmptySound();
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;
	m_pPlayer->m_flStopExtraSoundTime = UTIL_WeaponTimeBase() + 0.2;

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	// we don't add in player velocity anymore.
	CGrenade::ShootContact(m_pPlayer->pev,
		m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16,
		gpGlobals->v_forward * 800);

	int flags;
#if defined(CLIENT_WEAPONS)
	flags = UTIL_DefaultPlaybackFlags();
#else
	flags = 0;
#endif
	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/glauncher.wav", 1, ATTN_NORM);
		break;
	case 1:
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/glauncher2.wav", 1, ATTN_NORM);
		break;
	}

	SendWeaponAnim(M4_LAUNCH);

	// Punch the camera to simulate recoil
	m_pPlayer->pev->punchangle.x -= 8;

	m_flNextPrimaryAttack = GetNextAttackDelay(1);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5; // idle pretty soon after shooting.

	if (0 == m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
}

void CM4::Reload()
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		const bool bResult = DefaultReload(M249_MAX_CLIP, M4_RELOAD, 1.5);

		/*if (bResult)
		{
			Vector vecShellVelocity = m_pPlayer->pev->velocity + gpGlobals->v_right * RANDOM_FLOAT(50, 100) +
									  gpGlobals->v_up * RANDOM_FLOAT(100, 150) + gpGlobals->v_forward * 25;
			EjectBrass(pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -12 + gpGlobals->v_forward * 20 +
						   gpGlobals->v_right * 8,
				vecShellVelocity, pev->angles.y, m_iClipMdl, BOUNCE_METAL);
		}*/

		if (bResult)
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10.0, 15.0);
		}
	}
}


void CM4::WeaponIdle()
{
	ResetEmptySound();
	UpdateVModel();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_iBurstState > BURST_IDLE)
	{
		if (m_iBurstState >= BURST_NUM_SHOTS)
		{
			m_iBurstState = BURST_IDLE;
		}
		else
		{
			m_iBurstState++;
			if (m_iClip > 0)
			{
				M4Fire(0.1, 0.2, false);
			}
		}
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2f;
		return;
	}

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_iClip == 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
		{
			Reload();
		}
		else
		{
			int iAnim;
			switch (RANDOM_LONG(0, 1))
			{
			case 0:
				iAnim = M4_LONGIDLE;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 49.0 / 16;
				break;

			default:
			case 1:
				iAnim = M4_IDLE1;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 16.0;
				break;
			}

			SendWeaponAnim(iAnim);
		}
	}
}
