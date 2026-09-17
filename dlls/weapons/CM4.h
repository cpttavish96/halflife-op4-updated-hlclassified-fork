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

enum M4Anim
{
	M4_LONGIDLE = 0,
	M4_IDLE1,
	M4_LAUNCH,
	M4_RELOAD,
	M4_DEPLOY,
	M4_FIRE1,
	M4_FIRE2,
	M4_FIRE3,
};

class CM4 : public CBasePlayerWeapon
{
public:
#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
#endif

	enum BurstState
	{
		BURST_IDLE = -1,
		BURST_TRIGGER = 0,
		BURST_NUM_SHOTS = 2
	};

	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 3; }
	bool GetItemInfo(ItemInfo* p) override;
	void IncrementAmmo(CBasePlayer* pPlayer) override;

	void M4Fire(float flSpread, float flCycleTime, bool fUseAutoAim);
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Reload() override;
	void WeaponIdle() override;
	float m_flNextAnimTime;
	int m_iShell;

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return UTIL_DefaultUseDecrement();
#else
		return false;
#endif
	}

	void UpdateVModel();

private:
	float m_flNextGrenadeLoad;
	unsigned short m_usMP5;
	unsigned short m_usMP52;
	int m_iBurstState = BURST_IDLE;

public:
	CM4() = default;
};
