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
#include <math.h>
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Light.h"
#include "BattleGeometry.h"
#include "BattleMeshes.h"
#include "BattleScene.h"


//////////////////////////////////////////////////////////////////////////
//
// The matrix that puts a mesh, which faces along -z, psFromMiddle from
// the middle of the view facing fHeading: fScale its size, and fLength
// its size again along the way it faces, for a beam.
//
//////////////////////////////////////////////////////////////////////////
void BattleSceneMatrix(SFloat4x4* psDest, const SFloat32Vec2* psFromMiddle, float32 fHeading, float32 fScale, float32 fLength)
{
	float32	fSin;
	float32	fCos;

	fSin = sinf(fHeading);
	fCos = cosf(fHeading);

	//Its right, its up and its back, in the scene.
	psDest->x.Init(fCos * fScale, 0.0f, fSin * fScale, 0.0f);
	psDest->y.Init(0.0f, fScale, 0.0f, 0.0f);
	psDest->z.Init(-fSin * fScale * fLength, 0.0f, fCos * fScale * fLength, 0.0f);
	psDest->pos.Init(psFromMiddle->x, 0.0f, -psFromMiddle->y, 1.0f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleInstances::Init(Ptr<CScene> pScene, Ptr<CMeshEditor> pEditor)
{
	PreInit();

	mpScene = pScene;
	mpEditor = pEditor;
	mapInstances.Init();
	muiShown = 0;
	muiPlaced = 0;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleInstances::Class(void)
{
	M_Pointer(mpScene);
	M_Pointer(mpEditor);
	M_Embedded(mapInstances);
	U_Size(muiShown);
	U_Size(muiPlaced);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleInstances::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleInstances::Begin(void)
{
	muiPlaced = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The next instance, in the scene at psWorld.
//
//////////////////////////////////////////////////////////////////////////
void CBattleInstances::Place(SFloat4x4* psWorld)
{
	Ptr<CInstance>		pInstance;
	Ptr<CConnection>	pConnection;

	if (muiPlaced < mapInstances.NumElements())
	{
		pInstance = mapInstances.Get(muiPlaced);
		pConnection = pInstance->mapConnections.Get(0);
	}
	else
	{
		pConnection = OMalloc<CConnection>();
		pInstance = OMalloc<CInstance>();
		pInstance->mszName.Set(mpEditor->mszName.Text());
		pInstance->mapConnections.Add(pConnection);
		pInstance->AddMesh(mpEditor)->mapConnections.Add(pConnection);
		mapInstances.Add(pInstance);
	}
	pConnection->msLocalMatrix = *psWorld;
	pConnection->msWorldMatrix = *psWorld;

	if (muiPlaced >= muiShown)
	{
		mpScene->mapInstances.Add(pInstance);
		muiShown++;
	}
	muiPlaced++;
}


//////////////////////////////////////////////////////////////////////////
//
// Those not placed since Begin leave the scene.
//
//////////////////////////////////////////////////////////////////////////
void CBattleInstances::End(void)
{
	while (muiShown > muiPlaced)
	{
		muiShown--;
		mpScene->mapInstances.Remove(mapInstances.Get(muiShown));
	}
}


size CBattleInstances::NumShown(void) const { return muiShown; }
size CBattleInstances::NumInstances(void) { return mapInstances.NumElements(); }


//////////////////////////////////////////////////////////////////////////
//
// The scene of pcBattle as it is now.  Each ship's mesh faces along -z
// about the ship's middle, and its flame is what's shown behind it while
// it thrusts.
//
//////////////////////////////////////////////////////////////////////////
void CBattleScene::Init(CBattle* pcBattle, Ptr<CMeshEditor> pFirstShip, Ptr<CMeshEditor> pFirstFlame, Ptr<CMeshEditor> pSecondShip, Ptr<CMeshEditor> pSecondFlame)
{
	PreInit();

	mpScene = OMalloc<CScene>();
	mpStars = OMalloc<CBattleInstances>(mpScene, BattleMeshStars());
	mpPlanet = OMalloc<CBattleInstances>(mpScene, BattleMeshPlanet(pcBattle->GetPlanetRadius()));
	mpFirstShip = OMalloc<CBattleInstances>(mpScene, pFirstShip);
	mpFirstFlame = OMalloc<CBattleInstances>(mpScene, pFirstFlame);
	mpSecondShip = OMalloc<CBattleInstances>(mpScene, pSecondShip);
	mpSecondFlame = OMalloc<CBattleInstances>(mpScene, pSecondFlame);
	mpBeams = OMalloc<CBattleInstances>(mpScene, BattleMeshBeam());
	mpBolts = OMalloc<CBattleInstances>(mpScene, BattleMeshBolt());
	mpSeekers = OMalloc<CBattleInstances>(mpScene, BattleMeshSeeker());
	mpHits = OMalloc<CBattleInstances>(mpScene, BattleMeshHit());
	mpBursts = OMalloc<CBattleInstances>(mpScene, BattleMeshBurst());
	mpExplosions = OMalloc<CBattleInstances>(mpScene, BattleMeshExplosion());
	msMiddle = pcBattle->GetPlanet();
	mfHalfAcross = BATTLE_SCENE_MARGIN;
	mfHalfUp = BATTLE_SCENE_MARGIN;
	AddLights();

	PostInit();

	Update(pcBattle);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleScene::Class(void)
{
	M_Pointer(mpScene);
	M_Pointer(mpFirstShip);
	M_Pointer(mpFirstFlame);
	M_Pointer(mpSecondShip);
	M_Pointer(mpSecondFlame);
	M_Pointer(mpBeams);
	M_Pointer(mpBolts);
	M_Pointer(mpSeekers);
	M_Pointer(mpHits);
	M_Pointer(mpBursts);
	M_Pointer(mpExplosions);
	M_Pointer(mpPlanet);
	M_Pointer(mpStars);
	U_2Float32(msMiddle);
	U_Float32(mfHalfAcross);
	U_Float32(mfHalfUp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleScene::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Enough light everywhere that nothing is black against space, and a
// light from above and a little to one side that gives the ships their
// shape.
//
//////////////////////////////////////////////////////////////////////////
void CBattleScene::AddLights(void)
{
	Ptr<CLight>	pLight;
	SFloat4x4	sTurn;
	SFloat4x4	sTilt;

	pLight = OMalloc<CLight>();
	pLight->meType = LT_Ambient;
	pLight->mfIntensity = 0.45f;
	pLight->msColour.Init(1.0f, 1.0f, 1.0f, 1.0f);
	mpScene->mapLights.Add(pLight);

	pLight = OMalloc<CLight>();
	pLight->meType = LT_Directional;
	pLight->mfIntensity = 0.7f;
	pLight->msColour.Init(1.0f, 1.0f, 1.0f, 1.0f);
	pLight->mpConnection = OMalloc<CConnection>();
	Float4x4RotationY(&sTurn, 0.6f);
	Float4x4RotationX(&sTilt, -1.05f);
	Float4x4Multiply(&pLight->mpConnection->msWorldMatrix, &sTilt, &sTurn);
	pLight->mpConnection->msLocalMatrix = pLight->mpConnection->msWorldMatrix;
	mpScene->mapLights.Add(pLight);
}


//////////////////////////////////////////////////////////////////////////
//
// Everything put where the battle has it now.
//
//////////////////////////////////////////////////////////////////////////
void CBattleScene::Update(CBattle* pcBattle)
{
	SFloat32Vec2	sPlanet;
	SFloat32Vec2	sFromMiddle;
	SFloat4x4		sWorld;

	FindMiddle(pcBattle);
	PlaceStars();

	sPlanet = pcBattle->GetPlanet();
	pcBattle->Delta(&sFromMiddle, &msMiddle, &sPlanet);
	BattleSceneMatrix(&sWorld, &sFromMiddle, 0.0f, 1.0f, 1.0f);
	mpPlanet->Begin();
	mpPlanet->Place(&sWorld);
	mpPlanet->End();

	PlaceShips(pcBattle);
	PlaceShots(pcBattle);
	PlaceEffects(pcBattle);
}


//////////////////////////////////////////////////////////////////////////
//
// Half way between the ships by the shorter way, and how far from there
// each is with a margin; on the one that's left when there's only one,
// and where it last was when there's neither.
//
//////////////////////////////////////////////////////////////////////////
void CBattleScene::FindMiddle(CBattle* pcBattle)
{
	SBattleShip*	psFirst;
	SBattleShip*	psSecond;
	SFloat32Vec2	sDelta;

	psFirst = pcBattle->GetShip(0);
	psSecond = pcBattle->GetShip(1);
	mfHalfAcross = BATTLE_SCENE_MARGIN;
	mfHalfUp = BATTLE_SCENE_MARGIN;
	if (psFirst->bAlive && psSecond->bAlive)
	{
		pcBattle->Delta(&sDelta, &psFirst->sPosition, &psSecond->sPosition);
		msMiddle.x = BattleWrap(psFirst->sPosition.x + sDelta.x * 0.5f, pcBattle->GetSize());
		msMiddle.y = BattleWrap(psFirst->sPosition.y + sDelta.y * 0.5f, pcBattle->GetSize());
		mfHalfAcross += fabsf(sDelta.x) * 0.5f;
		mfHalfUp += fabsf(sDelta.y) * 0.5f;
	}
	else if (psFirst->bAlive)
	{
		msMiddle = psFirst->sPosition;
	}
	else if (psSecond->bAlive)
	{
		msMiddle = psSecond->sPosition;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The tiles of stars, as many each way as cover whatever can be seen,
// placed so that the stars stay where they are in the battle as the
// middle of the view moves over them.
//
//////////////////////////////////////////////////////////////////////////
void CBattleScene::PlaceStars(void)
{
	SFloat4x4	sWorld;
	float32		fAcross;
	float32		fDown;
	int32		iAcross;
	int32		iDown;

	fAcross = -BattleWrap(msMiddle.x, BATTLE_STAR_TILE) - BATTLE_STAR_TILE * (float32)(BATTLE_SCENE_STAR_TILES / 2);
	fDown = BattleWrap(msMiddle.y, BATTLE_STAR_TILE) - BATTLE_STAR_TILE * (float32)(BATTLE_SCENE_STAR_TILES / 2 + 1);

	mpStars->Begin();
	for (iDown = 0; iDown < BATTLE_SCENE_STAR_TILES; iDown++)
	{
		for (iAcross = 0; iAcross < BATTLE_SCENE_STAR_TILES; iAcross++)
		{
			sWorld.Identity();
			sWorld.pos.Init(fAcross + BATTLE_STAR_TILE * (float32)iAcross, 0.0f, fDown + BATTLE_STAR_TILE * (float32)iDown, 1.0f);
			mpStars->Place(&sWorld);
		}
	}
	mpStars->End();
}


//////////////////////////////////////////////////////////////////////////
//
// A ship that's been destroyed isn't shown.  Its flame is while it
// thrusts, and its beam while that's on, as long as the beam reaches.
//
//////////////////////////////////////////////////////////////////////////
void CBattleScene::PlaceShips(CBattle* pcBattle)
{
	CBattleInstances*	apcShips[BATTLE_SHIPS];
	CBattleInstances*	apcFlames[BATTLE_SHIPS];
	SBattleShip*		psShip;
	SFloat32Vec2		sFromMiddle;
	SFloat32Vec2		sMuzzle;
	SFloat4x4			sWorld;
	int32				i;

	apcShips[0] = &mpFirstShip;
	apcShips[1] = &mpSecondShip;
	apcFlames[0] = &mpFirstFlame;
	apcFlames[1] = &mpSecondFlame;

	mpBeams->Begin();
	for (i = 0; i < BATTLE_SHIPS; i++)
	{
		psShip = pcBattle->GetShip(i);
		apcShips[i]->Begin();
		apcFlames[i]->Begin();
		if (psShip->bAlive)
		{
			pcBattle->Delta(&sFromMiddle, &msMiddle, &psShip->sPosition);
			BattleSceneMatrix(&sWorld, &sFromMiddle, psShip->fHeading, 1.0f, 1.0f);
			apcShips[i]->Place(&sWorld);
			if (psShip->bThrusting)
			{
				apcFlames[i]->Place(&sWorld);
			}
			if (psShip->bBeam && (psShip->fBeamLength > 0.0f))
			{
				pcBattle->GetMuzzle(i, &psShip->pcType->msPrimary, &sMuzzle);
				pcBattle->Delta(&sFromMiddle, &msMiddle, &sMuzzle);
				BattleSceneMatrix(&sWorld, &sFromMiddle, psShip->fHeading, 1.0f, psShip->fBeamLength);
				mpBeams->Place(&sWorld);
			}
		}
		apcShips[i]->End();
		apcFlames[i]->End();
	}
	mpBeams->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleScene::PlaceShots(CBattle* pcBattle)
{
	SBattleShot*	psShot;
	SFloat32Vec2	sFromMiddle;
	SFloat4x4		sWorld;
	size			i;

	mpBolts->Begin();
	mpSeekers->Begin();
	for (i = 0; i < pcBattle->NumShots(); i++)
	{
		psShot = pcBattle->GetShot(i);
		pcBattle->Delta(&sFromMiddle, &msMiddle, &psShot->sPosition);
		BattleSceneMatrix(&sWorld, &sFromMiddle, psShot->fHeading, 1.0f, 1.0f);
		if (psShot->eKind == BW_Seeker)
		{
			mpSeekers->Place(&sWorld);
		}
		else
		{
			mpBolts->Place(&sWorld);
		}
	}
	mpBolts->End();
	mpSeekers->End();
}


//////////////////////////////////////////////////////////////////////////
//
// Each grows from part of its size to all of it over its life.
//
//////////////////////////////////////////////////////////////////////////
void CBattleScene::PlaceEffects(CBattle* pcBattle)
{
	SBattleEffect*	psEffect;
	SFloat32Vec2	sFromMiddle;
	SFloat4x4		sWorld;
	float32			fGrown;
	size			i;

	mpHits->Begin();
	mpBursts->Begin();
	mpExplosions->Begin();
	for (i = 0; i < pcBattle->NumEffects(); i++)
	{
		psEffect = pcBattle->GetEffect(i);
		fGrown = (psEffect->fLife > 0.0f) ? psEffect->fAge / psEffect->fLife : 1.0f;
		pcBattle->Delta(&sFromMiddle, &msMiddle, &psEffect->sPosition);
		switch (psEffect->eKind)
		{
		case BE_Hit:
			BattleSceneMatrix(&sWorld, &sFromMiddle, 0.0f, psEffect->fRadius * (0.4f + 0.6f * fGrown), 1.0f);
			mpHits->Place(&sWorld);
			break;
		case BE_Burst:
			BattleSceneMatrix(&sWorld, &sFromMiddle, 0.0f, psEffect->fRadius * (0.1f + 0.9f * fGrown), 1.0f);
			mpBursts->Place(&sWorld);
			break;
		case BE_Explosion:
			BattleSceneMatrix(&sWorld, &sFromMiddle, 0.0f, psEffect->fRadius * (0.25f + 0.75f * sqrtf(fGrown)), 1.0f);
			mpExplosions->Place(&sWorld);
			break;
		}
	}
	mpHits->End();
	mpBursts->End();
	mpExplosions->End();
}


//////////////////////////////////////////////////////////////////////////
//
// How many bolts or seekers are in the scene.
//
//////////////////////////////////////////////////////////////////////////
size CBattleScene::NumShown(EBattleWeapon eKind)
{
	if (eKind == BW_Bolt)
	{
		return mpBolts->NumShown();
	}
	if (eKind == BW_Seeker)
	{
		return mpSeekers->NumShown();
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CBattleScene::NumShown(EBattleEffect eKind)
{
	switch (eKind)
	{
	case BE_Hit:
		return mpHits->NumShown();
	case BE_Burst:
		return mpBursts->NumShown();
	case BE_Explosion:
		return mpExplosions->NumShown();
	}
	return 0;
}


Ptr<CScene> CBattleScene::GetScene(void) const { return mpScene; }
SFloat32Vec2 CBattleScene::GetMiddle(void) const { return msMiddle; }
float32 CBattleScene::GetHalfAcross(void) const { return mfHalfAcross; }
float32 CBattleScene::GetHalfUp(void) const { return mfHalfUp; }
size CBattleScene::NumShipsShown(void) { return mpFirstShip->NumShown() + mpSecondShip->NumShown(); }
size CBattleScene::NumFlamesShown(void) { return mpFirstFlame->NumShown() + mpSecondFlame->NumShown(); }
size CBattleScene::NumBeamsShown(void) { return mpBeams->NumShown(); }

