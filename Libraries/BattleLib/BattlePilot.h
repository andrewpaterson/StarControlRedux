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
#ifndef __BATTLE_PILOT_H__
#define __BATTLE_PILOT_H__
#include "Battle.h"


#define BATTLE_PILOT_AIM			0.10f	//Radians off its aim within which a pilot fires a bolt or a beam,
#define BATTLE_PILOT_TURN			0.03f	//and within which it stops turning.
#define BATTLE_PILOT_PLANET_MARGIN	10.0f	//How near the planet's surface a pilot lets its ship pass.
#define BATTLE_PILOT_LOOK_AHEAD		2.5f	//Seconds of its drift a pilot looks along for the planet.
#define BATTLE_PILOT_SHOT_MARGIN		3.0f	//How near the planet's surface a pilot will shoot past.
#define BATTLE_PILOT_REACH_SHARE		0.7f	//Of how far its primary weapon reaches, how near a pilot likes to be.
#define BATTLE_PILOT_ARRIVE_SPEED	6.0f	//How fast a pilot is still closing when it gets there,
#define BATTLE_PILOT_ARRIVE_RATE	0.7f	//and how much faster for each unit further off it is.
#define BATTLE_PILOT_SPEED_SLACK	3.0f	//How far from the speed it wants a pilot lets itself be before it thrusts.


//A computer's pilot.  BattlePilot decides what the ship does next from the battle as it is, and remembers nothing, so the
//same battle gets the same answer.
//
//In reach of the other ship it turns to face where that will be when a shot gets there, and fires when it's facing
//there.  Out of reach it flies to where it would be in reach: it thrusts towards the speed it wants, slower the nearer it
//is, which is how it slows down as well as how it gets going.  A seeker is sent when there's energy to spare for it, and
//a burst is set off when the other ship or one of its shots is within reach.
//
//The planet comes first.  A ship drifting at it thrusts aside, one too near thrusts straight out, and with the planet
//between the ships it flies round by the shorter way and holds its fire.
void	BattlePilot(CBattle* pcBattle, int32 iShip, SBattleControls* psDest);


#endif // __BATTLE_PILOT_H__

