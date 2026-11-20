/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela TestBattleLib

Codaphela TestBattleLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela TestBattleLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela TestBattleLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "SupportLib/PNGWriter.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/Window.h"
#include "WindowLib/WindowTick.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "BattleLib/BattleGeometry.h"
#include "BattleLib/BattleMeshes.h"
#include "BattleLib/BattleScene.h"
#include "BattleLib/BattleGame.h"
#include "BattleLib/BattleView.h"
#include "TestLib/Assert.h"


#define TEST_BATTLE_VIEW_WIDTH	320
#define TEST_BATTLE_VIEW_HEIGHT	240
#define TEST_BATTLE_VIEW_PICTURE	"BATTLE_VIEW_PICTURE"	//In the environment, a file the drawn view is saved to as a PNG, to look at.


//Stops the window after a few ticks, by when it's been drawn.
class CTestBattleViewTick : public CWindowTick
{
CONSTRUCTABLE(CTestBattleViewTick);
DESTRUCTABLE(CTestBattleViewTick);
protected:
	int32	miTicks;

public:
	void Init(void)
	{
		PreInit();
		CWindowTick::Init();
		miTicks = 0;
		PostInit();
	}

	void Class(void) override
	{
		CWindowTick::Class();
		U_Int32(miTicks);
	}

	void Tick(Ptr<CWindow> pWindow, int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds) override
	{
		miTicks++;
		if (miTicks == 3)
		{
			pWindow->Stop();
		}
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A mesh is put in the scene with the battle's x and y as the scene's x
// and -z, and what was its front (-z) the way its heading faces.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleSceneMatrix(void)
{
	SFloat4x4		sWorld;
	SFloat32Vec2	sFromMiddle;
	SFloat32Vec2	sForward;

	sFromMiddle.Init(3.0f, 5.0f);
	BattleSceneMatrix(&sWorld, &sFromMiddle, 0.0f, 1.0f, 1.0f);
	AssertFloat(1.0f, sWorld.x.x, 5);
	AssertFloat(0.0f, sWorld.x.z, 5);
	AssertFloat(1.0f, sWorld.y.y, 5);
	AssertFloat(0.0f, sWorld.z.x, 5);
	AssertFloat(1.0f, sWorld.z.z, 5);
	AssertFloat(3.0f, sWorld.pos.x, 5);
	AssertFloat(0.0f, sWorld.pos.y, 5);
	AssertFloat(-5.0f, sWorld.pos.z, 5);
	AssertFloat(1.0f, sWorld.pos.w, 5);

	//A quarter turn to the right: its front, minus its z row, is the battle's +x, and its right is down the screen, +z.
	BattleSceneMatrix(&sWorld, &sFromMiddle, BATTLE_PI * 0.5f, 1.0f, 1.0f);
	BattleForward(&sForward, BATTLE_PI * 0.5f);
	AssertFloat(sForward.x, -sWorld.z.x, 5);
	AssertFloat(-sForward.y, -sWorld.z.z, 5);
	AssertFloat(0.0f, sWorld.x.x, 5);
	AssertFloat(1.0f, sWorld.x.z, 5);
	AssertFloat(1.0f, sWorld.Determinant(), 4);  //Turned, not mirrored.

	//Twice the size, and five times that along the way it faces.
	BattleSceneMatrix(&sWorld, &sFromMiddle, 0.0f, 2.0f, 5.0f);
	AssertFloat(2.0f, sWorld.x.x, 5);
	AssertFloat(2.0f, sWorld.y.y, 5);
	AssertFloat(10.0f, sWorld.z.z, 5);
}


//////////////////////////////////////////////////////////////////////////
//
// Every mesh has faces, each with a material, and a ship's outline from
// its mesh is as wide and as long as the mesh is.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleSceneMeshes(void)
{
	ObjectsInit();
	{
		Ptr<CMeshEditor>	apEditors[12];
		Ptr<CMesh>			pMesh;
		CBattleShipType		cType;
		float32				fLeft;
		float32				fRight;
		float32				fFront;
		float32				fBack;
		size				i;
		int					iFace;
		int					iUnset;

		apEditors[0] = BattleMeshKestrel();
		apEditors[1] = BattleMeshKestrelFlame();
		apEditors[2] = BattleMeshBulwark();
		apEditors[3] = BattleMeshBulwarkFlame();
		apEditors[4] = BattleMeshBolt();
		apEditors[5] = BattleMeshSeeker();
		apEditors[6] = BattleMeshBeam();
		apEditors[7] = BattleMeshPlanet(14.0f);
		apEditors[8] = BattleMeshHit();
		apEditors[9] = BattleMeshBurst();
		apEditors[10] = BattleMeshExplosion();
		apEditors[11] = BattleMeshStars();
		for (i = 0; i < 12; i++)
		{
			pMesh = apEditors[i]->GetOutput();
			AssertTrue(pMesh->NumFaces() > 0);
			AssertInt(pMesh->NumFaces() * 3, pMesh->NumCorners());
			AssertTrue(pMesh->mcMaterials.mapMaterials.NumElements() > 0);
			iUnset = 0;
			for (iFace = 0; iFace < pMesh->NumFaces(); iFace++)
			{
				if ((pMesh->mcMaterials.mcFaces.GetValue((size)iFace) < 0) || (pMesh->mcMaterials.mcFaces.GetValue((size)iFace) >= (int)pMesh->mcMaterials.mapMaterials.NumElements()))
				{
					iUnset++;
				}
			}
			AssertInt(0, iUnset);
		}
		AssertInt(BATTLE_STAR_COUNT * 2, apEditors[11]->GetOutput()->NumFaces());
		AssertString("Kestrel", apEditors[0]->mszName.Text());

		BattleShipKestrel(&cType);
		AssertSize(3, cType.muiNumOutline);
		AssertTrue(BattleMeshOutline(apEditors[0], &cType));
		AssertTrue(cType.muiNumOutline > 3);
		fLeft = fRight = fFront = fBack = 0.0f;
		for (i = 0; i < cType.muiNumOutline; i++)
		{
			fLeft = (cType.masOutline[i].x < fLeft) ? cType.masOutline[i].x : fLeft;
			fRight = (cType.masOutline[i].x > fRight) ? cType.masOutline[i].x : fRight;
			fFront = (cType.masOutline[i].y > fFront) ? cType.masOutline[i].y : fFront;
			fBack = (cType.masOutline[i].y < fBack) ? cType.masOutline[i].y : fBack;
		}
		AssertFloat(-3.5f, fLeft, 3);  //Its wing tips,
		AssertFloat(3.5f, fRight, 3);
		AssertFloat(4.6f, fFront, 3);  //its nose, which is in front,
		AssertFloat(-3.2f, fBack, 3);  //and its engine.
		AssertFloat(sqrtf(3.5f * 3.5f + 3.0f * 3.0f), cType.mfRadius, 3);  //A wing tip's back corner is the furthest out.
		cType.Kill();

		BattleShipBulwark(&cType);
		AssertTrue(BattleMeshOutline(apEditors[2], &cType));
		fLeft = fRight = fFront = fBack = 0.0f;
		for (i = 0; i < cType.muiNumOutline; i++)
		{
			fLeft = (cType.masOutline[i].x < fLeft) ? cType.masOutline[i].x : fLeft;
			fRight = (cType.masOutline[i].x > fRight) ? cType.masOutline[i].x : fRight;
			fFront = (cType.masOutline[i].y > fFront) ? cType.masOutline[i].y : fFront;
			fBack = (cType.masOutline[i].y < fBack) ? cType.masOutline[i].y : fBack;
		}
		AssertFloat(-5.8f, fLeft, 3);
		AssertFloat(5.8f, fRight, 3);
		AssertFloat(cType.msPrimary.fMuzzle, fFront, 3);  //Its beam starts at the tip of its emitter.
		AssertFloat(-5.9f, fBack, 3);
		cType.Kill();

		//A mesh that's all in a line has no outline, and the type keeps the one it had.
		BattleShipKestrel(&cType);
		apEditors[0] = OMalloc<CMeshEditor>(OMalloc<CMesh>());
		apEditors[0]->AddCorner(0.0f, 0.0f, 0.0f);
		apEditors[0]->AddCorner(1.0f, 0.0f, 0.0f);
		apEditors[0]->AddCorner(2.0f, 0.0f, 0.0f);
		AssertFalse(BattleMeshOutline(apEditors[0], &cType));
		AssertSize(3, cType.muiNumOutline);
		cType.Kill();

		for (i = 0; i < 12; i++)
		{
			apEditors[i] = NULL;
		}
		pMesh = NULL;
	}
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// The scene shows what the battle has: the ships, a flame while one
// thrusts, the bolts in flight and no more, a beam as long as it reaches,
// and an explosion in place of a ship that's gone.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleSceneFollowsBattle(void)
{
	ObjectsInit();
	{
		Ptr<CBattleGame>	pGame;
		Ptr<CBattleScene>	pScene;
		Ptr<CScene>			pDrawn;
		CBattle*			pcBattle;
		SBattleControls*	psHeld;
		SBattleShip*		psFirst;
		SBattleShip*		psSecond;
		SFloat32Vec2		sMiddle;
		size				uiStill;
		size				uiBolts;

		pGame = OMalloc<CBattleGame>();
		ORoot()->Add(pGame);
		pGame->SetComputer(1, false);
		pScene = pGame->GetScene();
		pDrawn = pScene->GetScene();
		pcBattle = pGame->GetBattle();
		psFirst = pcBattle->GetShip(0);
		psSecond = pcBattle->GetShip(1);
		AssertFalse(pGame->IsComputer(0));
		AssertString("Kestrel", psFirst->pcType->mszName.Text());
		AssertString("Bulwark", psSecond->pcType->mszName.Text());
		AssertTrue(psFirst->pcType->muiNumOutline > 3);  //Its mesh's.

		//The planet, nine tiles of stars and the two ships; the middle is the planet, half way between them.
		uiStill = 1 + BATTLE_SCENE_STAR_TILES * BATTLE_SCENE_STAR_TILES;
		AssertSize(2, pScene->NumShipsShown());
		AssertSize(0, pScene->NumFlamesShown());
		AssertSize(uiStill + 2, pDrawn->mapInstances.NumElements());
		AssertSize(2, pDrawn->mapLights.NumElements());
		sMiddle = pScene->GetMiddle();
		AssertFloat(200.0f, sMiddle.x, 3);
		AssertFloat(200.0f, sMiddle.y, 3);
		AssertFloat(60.0f + BATTLE_SCENE_MARGIN, pScene->GetHalfAcross(), 3);
		AssertFloat(BATTLE_SCENE_MARGIN, pScene->GetHalfUp(), 3);

		//With a player in it the battle waits: nothing moves until it's begun.
		AssertTrue(pGame->IsWaiting());
		psHeld = pGame->GetHeld(0);
		psHeld->bThrust = true;
		pGame->Advance(0.25f);
		AssertFloat(0.0f, pcBattle->GetSeconds(), 4);
		AssertFloat(0.0f, psFirst->sVelocity.Magnitude(), 4);
		AssertSize(0, pScene->NumFlamesShown());
		pGame->Begin();
		AssertFalse(pGame->IsWaiting());

		//Thrusting and firing: a flame, and as many bolts as are in flight.
		psHeld->bPrimary = true;
		pGame->Advance(0.25f);
		AssertFloat(0.25f, pcBattle->GetSeconds(), 3);
		uiBolts = pcBattle->NumShots();
		AssertTrue(uiBolts >= 2);
		AssertSize(1, pScene->NumFlamesShown());
		AssertSize(uiBolts, pScene->NumShown(BW_Bolt));
		AssertSize(0, pScene->NumShown(BW_Seeker));
		AssertSize(uiStill + 2 + 1 + uiBolts, pDrawn->mapInstances.NumElements());

		//Let go: the flame goes at once, and the bolts when they've flown out.
		psHeld->bThrust = false;
		psHeld->bPrimary = false;
		pGame->Advance(0.01f);
		AssertSize(0, pScene->NumFlamesShown());
		pGame->Advance(0.25f);
		pGame->Advance(0.25f);
		pGame->Advance(0.25f);
		pGame->Advance(0.25f);
		AssertSize(0, pcBattle->NumShots());
		AssertSize(0, pScene->NumShown(BW_Bolt));
		AssertSize(uiStill + 2, pDrawn->mapInstances.NumElements());

		//A seeker.
		psHeld->bSecondary = true;
		pGame->Advance(0.02f);
		psHeld->bSecondary = false;
		AssertSize(1, pScene->NumShown(BW_Seeker));
		AssertSize(0, pScene->NumShown(BW_Bolt));

		//The second ship's beam, while it's held, and its burst's ring.
		AssertSize(0, pScene->NumBeamsShown());
		psHeld = pGame->GetHeld(1);
		psHeld->bPrimary = true;
		psHeld->bSecondary = true;
		pGame->Advance(0.02f);
		AssertTrue(psSecond->bBeam);
		AssertSize(1, pScene->NumBeamsShown());
		AssertSize(1, pScene->NumShown(BE_Burst));
		psHeld->bPrimary = false;
		psHeld->bSecondary = false;
		pGame->Advance(0.02f);
		AssertSize(0, pScene->NumBeamsShown());

		//The first ship gone: it isn't shown, its explosion is, and the middle is the ship that's left.
		psFirst->fHull = 0.0f;
		psFirst->bAlive = false;
		pGame->Advance(0.01f);
		AssertSize(1, pScene->NumShipsShown());
		sMiddle = pScene->GetMiddle();
		AssertFloat(psSecond->sPosition.x, sMiddle.x, 3);
		AssertFloat(psSecond->sPosition.y, sMiddle.y, 3);
		AssertFloat(BATTLE_SCENE_MARGIN, pScene->GetHalfAcross(), 3);

		//Started again, both are back, and it waits again.
		pGame->Restart();
		AssertTrue(pGame->IsWaiting());
		pGame->Begin();
		AssertSize(2, pScene->NumShipsShown());
		AssertSize(0, pScene->NumShown(BW_Seeker));
		AssertSize(0, pScene->NumShown(BE_Burst));
		AssertSize(uiStill + 2, pDrawn->mapInstances.NumElements());
		AssertTrue(psFirst->bAlive);

		//Ships either side of an edge are seen together: the middle is on the edge, and they're a little way either side of it.
		psFirst->sPosition.Init(390.0f, 100.0f);
		psSecond->sPosition.Init(20.0f, 100.0f);
		pGame->Advance(0.0f);
		sMiddle = pScene->GetMiddle();
		AssertFloat(5.0f, sMiddle.x, 3);
		AssertFloat(100.0f, sMiddle.y, 3);
		AssertFloat(15.0f + BATTLE_SCENE_MARGIN, pScene->GetHalfAcross(), 3);

		pDrawn = NULL;
		pScene = NULL;
		ORoot()->Remove(pGame);
		pGame = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// With the computer flying both ships a game is won, and started again
// it's being fought again.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleSceneComputerPlays(void)
{
	ObjectsInit();
	{
		Ptr<CBattleGame>	pGame;
		CBattle*			pcBattle;
		int32				i;

		pGame = OMalloc<CBattleGame>();
		ORoot()->Add(pGame);
		AssertTrue(pGame->IsWaiting());
		pGame->SetComputer(0, true);
		AssertTrue(pGame->IsComputer(0));
		AssertTrue(pGame->IsComputer(1));
		AssertFalse(pGame->IsWaiting());  //There's nobody to wait for.
		pcBattle = pGame->GetBattle();

		for (i = 0; (i < 60 * 600) && !pcBattle->IsOver(); i++)
		{
			pGame->Advance(1.0f / 60.0f);
		}
		AssertTrue(pcBattle->IsOver());
		AssertTrue(pGame->GetScene()->NumShipsShown() < 2);

		pGame->Restart();
		AssertFalse(pcBattle->IsOver());
		AssertFalse(pGame->IsWaiting());
		AssertSize(2, pGame->GetScene()->NumShipsShown());

		ORoot()->Remove(pGame);
		pGame = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// How many of the pixels in the square about x, y aren't space.
//
//////////////////////////////////////////////////////////////////////////
static int32 TestBattleViewDrawn(Ptr<CImage> pImage, int32 x, int32 y, int32 iHalf)
{
	uint32	uiPixel;
	int32	iDrawn;
	int32	iX;
	int32	iY;

	iDrawn = 0;
	for (iY = y - iHalf; iY <= y + iHalf; iY++)
	{
		for (iX = x - iHalf; iX <= x + iHalf; iX++)
		{
			memcpy(&uiPixel, (uint8*)pImage->GetData() + ((size)iY * pImage->GetWidth() + (size)iX) * 4, 4);
			if ((uiPixel & 0x00FFFFFF) != (BATTLE_VIEW_BACKGROUND & 0x00FFFFFF))
			{
				iDrawn++;
			}
		}
	}
	return iDrawn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestBattleViewKey(Ptr<CBattleView> pView, char* szName, bool bDown, bool bTaken)
{
	SComponentInputEvent	sEvent;

	memset(&sEvent, 0, sizeof(sEvent));
	sEvent.szName = szName;
	if (bDown)
	{
		pView->KeyDownAction(&sEvent);
	}
	else
	{
		pView->KeyUpAction(&sEvent);
	}
	AssertBool(!bTaken, sEvent.bPass);
}


//////////////////////////////////////////////////////////////////////////
//
// A battle in a window with no screen, drawn by the rasteriser: the
// planet is in the middle with a ship either side, from far enough to
// see both.  Its keys hold a ship's controls, hand a ship to the computer
// and start the battle again, and a key that isn't one of its is passed
// on.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleViewDrawnAndFlown(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "BattleView";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.TouchDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>			pWindow;
		Ptr<CGridContainer>		pGrid;
		Ptr<CBattleView>		pView;
		Ptr<CBattleCanvasDraw>	pDraw;
		Ptr<CBattleGame>		pGame;
		Ptr<CWindowTheme>		pTheme;
		Ptr<CImage>				pImage;
		CPointer				pNull;
		SBattleControls*		psHeld;
		SBattleShip*			psShip;
		float32					fHalfAcross;
		float32					fDistance;
		int32					iShipOffset;
		float32					fBefore;

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, TEST_BATTLE_VIEW_WIDTH, TEST_BATTLE_VIEW_HEIGHT, szDirectory);
		pWindow = OMalloc<CWindow>("Battle View", &pNativeFactory, OMalloc<CTestBattleViewTick>(), pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);

		pTheme = OMalloc<CWindowTheme>(Ptr<CFont>());
		pTheme->SetColour(TC_Editor, Set32BitColour((uint8)40, (uint8)44, (uint8)52));
		pView = OMalloc<CBattleView>(pWindow, pTheme, false);
		pView->SetKeySeconds(0.0f);  //A control is on only while its key is down: TestBattleInput has them kept on.
		pDraw = pView->GetDraw();
		pGame = pView->GetGame();
		pGrid = OMalloc<CGridContainer>(pWindow);
		pGrid->SetAlignment(CSH_Stretch, CSV_Stretch);
		pGrid->AddComponent(pView, 0, 0, 1, 1, 1, 1);
		pWindow->SetContainer(pGrid);
		pWindow->Show();

		pImage = pDraw->GetImage();
		AssertTrue(pImage.IsNotNull());
		AssertInt(TEST_BATTLE_VIEW_WIDTH, (int32)pImage->GetWidth());
		AssertInt(TEST_BATTLE_VIEW_HEIGHT, (int32)pImage->GetHeight());
		if (getenv(TEST_BATTLE_VIEW_PICTURE) != NULL)
		{
			SavePNG(pImage, getenv(TEST_BATTLE_VIEW_PICTURE));
		}

		//From as far off as shows the ships, 60 either side of the middle, with their margin.
		fHalfAcross = 60.0f + BATTLE_SCENE_MARGIN;
		fDistance = fHalfAcross / tanf(TURNTABLE_FOV * 0.5f);
		AssertFloat(fDistance, pDraw->GetDistance(), 2);
		AssertFloat(fDistance, pDraw->DistanceFor(TEST_BATTLE_VIEW_WIDTH, TEST_BATTLE_VIEW_HEIGHT), 2);
		AssertFloat(BATTLE_VIEW_MIN_DISTANCE, pDraw->DistanceFor(0, 0), 2);
		AssertFalse(pDraw->IsVulkan());

		//The planet fills the middle, and each ship is where it should be across the canvas, with space round it.
		AssertInt(49, TestBattleViewDrawn(pImage, TEST_BATTLE_VIEW_WIDTH / 2, TEST_BATTLE_VIEW_HEIGHT / 2, 3));
		iShipOffset = (int32)(60.0f / fHalfAcross * (float32)(TEST_BATTLE_VIEW_WIDTH / 2));
		AssertTrue(TestBattleViewDrawn(pImage, TEST_BATTLE_VIEW_WIDTH / 2 - iShipOffset, TEST_BATTLE_VIEW_HEIGHT / 2, 3) > 8);  //The Kestrel is slim.
		AssertTrue(TestBattleViewDrawn(pImage, TEST_BATTLE_VIEW_WIDTH / 2 + iShipOffset, TEST_BATTLE_VIEW_HEIGHT / 2, 3) > 30);
		AssertTrue(TestBattleViewDrawn(pImage, TEST_BATTLE_VIEW_WIDTH / 2 - iShipOffset / 2, TEST_BATTLE_VIEW_HEIGHT / 2, 3) < 8);  //A star or two at most.
		AssertTrue(TestBattleViewDrawn(pImage, TEST_BATTLE_VIEW_WIDTH / 2 + iShipOffset / 2, TEST_BATTLE_VIEW_HEIGHT / 2, 3) < 8);

		//Told that more has to be seen, the view moves out to it over about a second of the battle's time, not at once.
		pDraw->SetSpan(fHalfAcross * 2.0f, BATTLE_SCENE_MARGIN, 0.1f);
		AssertFloat(fDistance * 2.0f, pDraw->DistanceFor(TEST_BATTLE_VIEW_WIDTH, TEST_BATTLE_VIEW_HEIGHT), 2);
		pDraw->MoveView(TEST_BATTLE_VIEW_WIDTH, TEST_BATTLE_VIEW_HEIGHT);
		AssertFloat(fDistance + fDistance * 0.1f * BATTLE_VIEW_CLOSING, pDraw->GetDistance(), 2);
		pDraw->MoveView(TEST_BATTLE_VIEW_WIDTH, TEST_BATTLE_VIEW_HEIGHT);  //No time has passed since.
		AssertFloat(fDistance + fDistance * 0.1f * BATTLE_VIEW_CLOSING, pDraw->GetDistance(), 2);
		pDraw->SetSpan(fHalfAcross * 2.0f, BATTLE_SCENE_MARGIN, 5.0f);
		pDraw->MoveView(TEST_BATTLE_VIEW_WIDTH, TEST_BATTLE_VIEW_HEIGHT);
		AssertFloat(fDistance * 2.0f, pDraw->GetDistance(), 2);

		//Tall and narrow, it's how far up and down has to be seen that decides.
		pDraw->SetSpan(10.0f, 200.0f, 0.0f);
		AssertFloat(200.0f / tanf(TURNTABLE_FOV * 0.5f) / 2.0f, pDraw->DistanceFor(100, 200), 2);

		//The battle waits for a player.  A key of the ship the computer flies doesn't begin it, nor one that isn't a
		//control; the first ship's does.
		AssertTrue(pGame->IsWaiting());
		AssertFalse(pView->IsPaused());
		pView->AdvanceBy(0.25f);
		AssertFloat(0.0f, pGame->GetBattle()->GetSeconds(), 4);
		TestBattleViewKey(pView, "Cursor Up", true, true);
		TestBattleViewKey(pView, "Cursor Up", false, true);
		TestBattleViewKey(pView, "V", true, true);
		TestBattleViewKey(pView, "V", true, true);
		AssertTrue(pGame->IsWaiting());

		//The first ship's keys.
		psHeld = pGame->GetHeld(0);
		TestBattleViewKey(pView, "W", true, true);
		AssertTrue(psHeld->bThrust);
		AssertFalse(pGame->IsWaiting());
		TestBattleViewKey(pView, "A", true, true);
		AssertTrue(psHeld->bLeft);
		TestBattleViewKey(pView, "D", true, true);
		AssertTrue(psHeld->bRight);
		TestBattleViewKey(pView, "Space", true, true);
		AssertTrue(psHeld->bPrimary);
		TestBattleViewKey(pView, "S", true, true);
		AssertTrue(psHeld->bSecondary);
		TestBattleViewKey(pView, "A", false, true);
		TestBattleViewKey(pView, "D", false, true);
		TestBattleViewKey(pView, "Space", false, true);
		TestBattleViewKey(pView, "S", false, true);
		AssertFalse(psHeld->bLeft);
		AssertFalse(psHeld->bRight);
		AssertFalse(psHeld->bPrimary);
		AssertFalse(psHeld->bSecondary);
		AssertTrue(psHeld->bThrust);
		AssertFalse(pGame->GetHeld(1)->bThrust);

		//Held, the ship flies when the battle is moved on.
		psShip = pGame->GetBattle()->GetShip(0);
		fBefore = psShip->sPosition.y;
		pView->AdvanceBy(0.25f);
		pView->AdvanceBy(0.25f);
		AssertTrue(psShip->sPosition.y > fBefore + 2.0f);
		TestBattleViewKey(pView, "W", false, true);
		AssertFalse(psHeld->bThrust);

		//The second ship's, which do nothing to it while the computer flies it.
		psHeld = pGame->GetHeld(1);
		TestBattleViewKey(pView, "Cursor Left", true, true);
		TestBattleViewKey(pView, "Cursor Right", true, true);
		TestBattleViewKey(pView, "Cursor Up", true, true);
		TestBattleViewKey(pView, "Cursor Down", true, true);
		TestBattleViewKey(pView, "/", true, true);
		AssertTrue(psHeld->bLeft && psHeld->bRight && psHeld->bThrust && psHeld->bPrimary && psHeld->bSecondary);
		TestBattleViewKey(pView, "Cursor Left", false, true);
		TestBattleViewKey(pView, "Cursor Right", false, true);
		TestBattleViewKey(pView, "Cursor Up", false, true);
		TestBattleViewKey(pView, "Cursor Down", false, true);
		TestBattleViewKey(pView, "/", false, true);
		AssertFalse(psHeld->bLeft || psHeld->bRight || psHeld->bThrust || psHeld->bPrimary || psHeld->bSecondary);

		//Who flies what, which changes as the key goes down and not as it comes up.
		AssertFalse(pGame->IsComputer(0));
		AssertTrue(pGame->IsComputer(1));
		TestBattleViewKey(pView, "2", true, true);
		AssertFalse(pGame->IsComputer(1));
		TestBattleViewKey(pView, "2", false, true);
		AssertFalse(pGame->IsComputer(1));
		TestBattleViewKey(pView, "1", true, true);
		AssertTrue(pGame->IsComputer(0));

		//From the beginning, where it waits for the second ship's player now; Enter again begins it.
		AssertTrue(pGame->GetBattle()->GetSeconds() > 0.4f);
		TestBattleViewKey(pView, "Enter", true, true);
		AssertFloat(0.0f, pGame->GetBattle()->GetSeconds(), 4);
		AssertFloat(BATTLE_ARENA_SIZE * BATTLE_START_FIRST, psShip->sPosition.x, 3);
		AssertTrue(pGame->IsWaiting());
		TestBattleViewKey(pView, "Enter", false, true);
		TestBattleViewKey(pView, "Enter", true, true);
		AssertFalse(pGame->IsWaiting());
		pView->AdvanceBy(0.125f);
		AssertTrue(pGame->GetBattle()->GetSeconds() > 0.1f);

		//Not the battle's.
		TestBattleViewKey(pView, "Q", true, false);
		TestBattleViewKey(pView, "Q", false, false);
		TestBattleViewKey(pView, NULL, true, false);

		pImage = NULL;
		pGame = NULL;
		pDraw = NULL;
		pView = NULL;
		pGrid = NULL;
		pTheme = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
	}
	AssertTrue(ORoot()->RemoveAll());
	pNativeFactory = NULL;
	ObjectsFlush();
	ObjectsKill();

	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBattleScene(void)
{
	BeginTests();

	TestBattleSceneMatrix();
	TestBattleSceneMeshes();
	TestBattleSceneFollowsBattle();
	TestBattleSceneComputerPlays();
	TestBattleViewDrawnAndFlown();

	TestStatistics();
}

