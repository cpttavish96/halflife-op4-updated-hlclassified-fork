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

enum KnifeAnim
{
	KNIFE_IDLE1 = 0,
	KNIFE_DRAW,
	KNIFE_HOLSTER,
	KNIFE_ATTACK1,
	KNIFE_ATTACK1MISS,
	KNIFE_ATTACK2,
	KNIFE_ATTACK2HIT,
	KNIFE_ATTACK3,
	KNIFE_ATTACK3HIT,
	KNIFE_IDLE2,
	KNIFE_IDLE3,
	KNIFE_CHARGE,
	KNIFE_STAB
};

class CKnife : public CBasePlayerWeapon
{
private:
	enum SwingMode
	{
		STAB_NONE = 0,
		STAB_START,
		STAB_DOING,
	};

public:
	using BaseClass = CBasePlayerWeapon;

	void Precache() override;

	void Spawn() override;

	bool Deploy() override;

	void Holster() override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void WeaponIdle() override;
	void EXPORT BigSwing();

	bool Swing(const bool bFirst);

	void EXPORT SwingAgain();

	void EXPORT Smack();

	int iItemSlot() override;

	bool GetItemInfo(ItemInfo* p) override;

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return UTIL_DefaultUseDecrement();
#else
		return false;
#endif
	}

	void UpdateVModel();

	float m_flBigSwingStart;
	int m_iSwingMode;
	int m_iSwing;
	TraceResult m_trHit;

private:
	unsigned short m_usKnife;
};
