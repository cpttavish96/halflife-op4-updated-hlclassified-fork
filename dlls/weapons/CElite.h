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

#pragma once

#include "weapons.h"

 enum CEliteAnims
{
	ELITE_IDLE = 0,
	ELITE_IDLE_LEFTEMPTY,
	ELITE_SHOOT_LEFT1,
	ELITE_SHOOT_LEFT2,
	ELITE_SHOOT_LEFT3,
	ELITE_SHOOT_LEFT4,
	ELITE_SHOOT_LEFT5,
	ELITE_SHOOT_LEFTLAST,
	ELITE_SHOOT_RIGHT1,
	ELITE_SHOOT_RIGHT2,
	ELITE_SHOOT_RIGHT3,
	ELITE_SHOOT_RIGHT4,
	ELITE_SHOOT_RIGHT5,
	ELITE_SHOOT_RIGHTLAST,
	ELITE_RELOAD,
	ELITE_DRAW
 };

class CElite : public CBasePlayerWeapon
 {
 public:
	 void Spawn() override;
	 void Precache() override;
	 int iItemSlot() override { return 2; };
	 bool GetItemInfo(ItemInfo* p) override;

	 void IncrementAmmo(CBasePlayer* pPlayer) override;

	 void PrimaryAttack() override;
	 void SecondaryAttack() override;
	 void GlockFire(float flSpread, float flCycleTime, bool fUseAutoAim, bool left);
	 bool Deploy() override;
	 void Holster() override;
	 void Reload() override;
	 void WeaponIdle() override;
	 void UpdateSpot();
	 void UpdateVModel();

	 CLaserSpot* m_pSpot;
	 bool m_fSpotActive;
	 int m_iTargetRanderamt;

	 bool UseDecrement() override
	 {
#if defined(CLIENT_WEAPONS)
		 return UTIL_DefaultUseDecrement();
#else
		 return false;
#endif
	 }

 private:
	 int m_iShell;
	 int m_iClip1;
	 bool left;


	 unsigned short m_usFireGlock1;
	 unsigned short m_usFireGlock2;
 };