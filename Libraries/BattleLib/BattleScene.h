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
#ifndef __BATTLE_SCENE_H__
#define __BATTLE_SCENE_H__
#include "StandardLib/Array.h"
#include "StandardLib/Object.h"
#include "SupportLib/Scene.h"
#include "Battle.h"


#define BATTLE_SCENE_MARGIN		26.0f	//Seen beyond each ship, so a ship is never at the very edge.
#define BATTLE_SCENE_STAR_TILES	3		//Tiles of stars each way.


//Instances of one mesh that come and go: a battle's bolts, say.  Between Begin and End each Place takes the next
//instance, making one if there isn't one, and puts it in the scene where it's told; End takes out of the scene those
//that weren't placed this time.  They're kept for the next.
class CBattleInstances : public CObject
{
CONSTRUCTABLE(CBattleInstances);
DESTRUCTABLE(CBattleInstances);
protected:
	Ptr<CScene>			mpScene;
	Ptr<CMeshEditor>	mpEditor;
	CArray<CInstance>	mapInstances;
	size				muiShown;  //How many of them, the first ones, are in the scene.
	size				muiPlaced;  //How many have been placed since Begin.

public:
	void	Init(Ptr<CScene> pScene, Ptr<CMeshEditor> pEditor);
	void	Class(void) override;
	void	Free(void) override;

	void	Begin(void);
	void	Place(SFloat4x4* psWorld);
	void	End(void);

	size	NumShown(void) const;
	size	NumInstances(void);
};


//What a battle looks like: a scene with the ships, their flames and beams, what's in flight, the planet and the stars
//behind, which Update puts where the battle has them.
//
//The scene is seen from above and its middle is always the middle of the battle's view: half way between the ships by
//the shorter way, or on the one that's left.  Everything is placed by its shorter way from there, so nothing is ever
//seen torn across an edge, and the stars go by underneath.  GetHalfAcross and GetHalfUp are how far from the middle
//has to be seen for both ships to be in view.
//
//A battle's x and y are the scene's x and -z, and a heading of 0 faces along -z.
class CBattleScene : public CObject
{
CONSTRUCTABLE(CBattleScene);
DESTRUCTABLE(CBattleScene);
protected:
	Ptr<CScene>				mpScene;
	Ptr<CBattleInstances>	mpFirstShip;
	Ptr<CBattleInstances>	mpFirstFlame;
	Ptr<CBattleInstances>	mpSecondShip;
	Ptr<CBattleInstances>	mpSecondFlame;
	Ptr<CBattleInstances>	mpBeams;
	Ptr<CBattleInstances>	mpBolts;
	Ptr<CBattleInstances>	mpSeekers;
	Ptr<CBattleInstances>	mpHits;
	Ptr<CBattleInstances>	mpBursts;
	Ptr<CBattleInstances>	mpExplosions;
	Ptr<CBattleInstances>	mpPlanet;
	Ptr<CBattleInstances>	mpStars;
	SFloat32Vec2			msMiddle;  //Of the view, in the battle.
	float32					mfHalfAcross;
	float32					mfHalfUp;

public:
	void			Init(CBattle* pcBattle, Ptr<CMeshEditor> pFirstShip, Ptr<CMeshEditor> pFirstFlame, Ptr<CMeshEditor> pSecondShip, Ptr<CMeshEditor> pSecondFlame);
	void			Class(void) override;
	void			Free(void) override;

	void			Update(CBattle* pcBattle);

	Ptr<CScene>		GetScene(void) const;
	SFloat32Vec2	GetMiddle(void) const;
	float32			GetHalfAcross(void) const;
	float32			GetHalfUp(void) const;
	size			NumShown(EBattleWeapon eKind);
	size			NumShown(EBattleEffect eKind);
	size			NumShipsShown(void);
	size			NumFlamesShown(void);
	size			NumBeamsShown(void);

protected:
	void			FindMiddle(CBattle* pcBattle);
	void			PlaceShips(CBattle* pcBattle);
	void			PlaceShots(CBattle* pcBattle);
	void			PlaceEffects(CBattle* pcBattle);
	void			PlaceStars(void);
	void			AddLights(void);
};


void	BattleSceneMatrix(SFloat4x4* psDest, const SFloat32Vec2* psFromMiddle, float32 fHeading, float32 fScale, float32 fLength);


#endif // __BATTLE_SCENE_H__

