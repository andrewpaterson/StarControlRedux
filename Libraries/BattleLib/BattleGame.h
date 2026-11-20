/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BattleLib

Codaphela BattleLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BattleLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BattleLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __BATTLE_GAME_H__
#define __BATTLE_GAME_H__
#include "StandardLib/Object.h"
#include "BattlePilot.h"
#include "BattleScene.h"


//Who is flying the two ships: what each player is holding, and whether the computer flies it instead.
struct SBattlePlayers
{
	SBattleControls	asHeld[BATTLE_SHIPS];
	bool			abComputer[BATTLE_SHIPS];
};


//A battle with its two ships' types, who flies them and what it looks like.  The first ship is a Kestrel and the second
//a Bulwark, each with the outline its mesh has from above.  To start with a player flies the first and the computer the
//second.
//
//A battle with a player in it waits for the player: nothing moves until Begin, which the first key a player presses
//calls, so the computer can't destroy a ship before anyone has reached the keyboard.  Restart waits again.  With the
//computer flying both there's nobody to wait for.
//
//Advance moves the battle on by the time that's passed, each ship doing what its player holds or what the computer's
//pilot decides, and puts the scene where the battle then is.  Restart begins the battle again with the same players.
class CBattleGame : public CObject
{
CONSTRUCTABLE(CBattleGame);
DESTRUCTABLE(CBattleGame);
protected:
	CBattleShipType		mcFirstType;
	CBattleShipType		mcSecondType;
	CBattle				mcBattle;
	SBattlePlayers		msPlayers;
	bool				mbBegun;  //A player has pressed a key since the battle was started.
	Ptr<CBattleScene>	mpScene;

public:
	void				Init(void);
	void				Class(void) override;
	void				Free(void) override;

	void				Advance(float32 fSeconds);
	void				Restart(void);
	void				Begin(void);
	bool				IsWaiting(void) const;

	SBattleControls*	GetHeld(int32 iShip);
	void				SetComputer(int32 iShip, bool bComputer);
	bool				IsComputer(int32 iShip) const;

	CBattle*			GetBattle(void);
	Ptr<CBattleScene>	GetScene(void) const;
};


#endif // __BATTLE_GAME_H__

