/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Battle

Codaphela Battle is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Battle is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Battle.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/Logger.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "SupportLib/PNGWriter.h"
#include "SupportLib/TrueTypeFontFactory.h"
#include "WindowLib/Canvas.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/JewelTheme.h"
#include "BattleTick.h"
#include "BattleApp.h"


#define BATTLE_SNAPSHOT_VARIABLE			"BATTLE_SNAPSHOT"
#define BATTLE_SNAPSHOT_SECONDS_VARIABLE	"BATTLE_SNAPSHOT_SECONDS"
#define BATTLE_EXIT_VARIABLE				"BATTLE_EXIT"
#define BATTLE_DEMO_VARIABLE				"BATTLE_DEMO"
#define BATTLE_NO_VULKAN_VARIABLE			"BATTLE_NO_VULKAN"

#define BATTLE_KEYS_TEXT					"First ship: A D turn, W thrust, Space S fire.   Second: cursor keys, down and / fire.   1, 2: the computer flies it.   Enter: start again.   V: drawn by "
#define BATTLE_APP_GAP						"  "
#define BATTLE_APP_WIDE_GAP					"        "


//////////////////////////////////////////////////////////////////////////
//
// Before it is set up.
//
//////////////////////////////////////////////////////////////////////////
void CBattleApp::Init(void)
{
	const char*	szSeconds;
	int32		iTicks;

	PreInit();

	mbValid = false;
	mbInputs = false;
	mbExit = false;
	mbSnapshot = getenv(BATTLE_SNAPSHOT_VARIABLE) != NULL;
	miSnapshotTicks = -1;
	if (mbSnapshot)
	{
		szSeconds = getenv(BATTLE_SNAPSHOT_SECONDS_VARIABLE);
		iTicks = (szSeconds != NULL) ? (int32)(atof(szSeconds) / BATTLE_APP_SNAPSHOT_STEP) : 0;
		miSnapshotTicks = (iTicks > BATTLE_APP_SNAPSHOT_TICKS) ? iTicks : BATTLE_APP_SNAPSHOT_TICKS;
	}

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The window, the battle in it and its input.  The devices are described
// by the XML in szInputDirectory; pcNativeInput (the platform's keyboard
// and mouse) is added to them.  Run fails if anything here did.  Call it
// once the app is in the root, so that what is made here is built under
// the root rather than as an unrooted graph.
//
//////////////////////////////////////////////////////////////////////////
bool CBattleApp::Setup(CNativeWindowFactory* pcNativeFactory, CNativeInput* pcNativeInput, const char* szInputDirectory)
{
	Ptr<CFont>				pWhite;
	Ptr<CClearCanvasDraw>	pBackground;

	pBackground = OMalloc<CClearCanvasDraw>(0);  //Coloured once there is a theme.
	mpWindow = OMalloc<CWindow>("Battle", pcNativeFactory, OMalloc<CBattleTick>(this), pBackground, CFT_RGBX, CCO_BGR, CRGB_24bit, ARGB_8bit);
	mpWindow->SetContinuous(true);

	//The font is white and coloured as it is drawn.
	pWhite = ReadTrueTypeFont("Resources/InterVariable.ttf", "Inter 13px", 13.0f, mpWindow->GetColourOrder());
	mbValid = pWhite.IsNotNull();
	if (mbValid)
	{
		mpFontColours = OMalloc<CFontColours>(pWhite);
		mpTheme = CJewelTheme::CreateDarkTheme(mpWindow, mpFontColours);
		pBackground->SetColour(mpTheme->GetColour(TC_Panel));

		mpView = OMalloc<CBattleView>(mpWindow, mpTheme, getenv(BATTLE_NO_VULKAN_VARIABLE) == NULL);
		if (getenv(BATTLE_DEMO_VARIABLE) != NULL)
		{
			mpView->GetGame()->SetComputer(0, true);
			mpView->GetGame()->SetComputer(1, true);
		}
		MakeBars();
		mpStatus = OMalloc<CLabel>(mpWindow, mpFontColours->GetFont(mpTheme->GetColour(TC_Icon)), "");

		mpMain = OMalloc<CGridContainer>(mpWindow);
		mpMain->AddComponent(mpBars, 0, 0, 1, 1, 1, 0);
		mpMain->AddComponent(mpView, 0, 1, 1, 1, 1, 1);
		mpMain->AddComponent(mpStatus, 0, 2, 1, 1, 1, 0);
		mpWindow->SetContainer(mpMain);
		ShowBattle();

		mpInput = OMalloc<CInput>((CNativeInput*)NULL);
		mbValid = mpInput->Read((char*)szInputDirectory);
		mpInput->AddNativeInput(pcNativeInput);
		mpInputSource = OMalloc<CWindowInputSource>(mpInput);
		mcComponentInput.Init(&mpInputSource, &mpWindow);
		mbInputs = true;
		mcComponentInput.AddKeyboards();
		mcComponentInput.AddMice();
		mpWindow->SetComponentInput(&mcComponentInput);

		mpWindow->SetWindowSize(1280, 800);
		mpView->FocusView();
	}
	return mbValid;
}


//////////////////////////////////////////////////////////////////////////
//
// Along the top: for each ship its name and who flies it, then its hull
// and its energy, the hull's bar twice the energy's, with gaps between
// them that are labels of spaces.
//
//////////////////////////////////////////////////////////////////////////
void CBattleApp::MakeBars(void)
{
	Ptr<CFont>	pFont;
	ARGB32		sTrack;
	ARGB32		sHull;
	ARGB32		sEnergy;

	pFont = mpFontColours->GetFont(mpTheme->GetColour(TC_Text));
	sTrack = mpTheme->GetColour(TC_Editor);
	sHull = mpTheme->GetColour(TC_Success);
	sEnergy = mpTheme->GetColour(TC_Accent);

	mpFirstName = OMalloc<CLabel>(mpWindow, pFont, "");
	mpFirstHull = OMalloc<CProgressBar>(mpWindow, sTrack, sHull, BATTLE_APP_BAR_HEIGHT);
	mpFirstEnergy = OMalloc<CProgressBar>(mpWindow, sTrack, sEnergy, BATTLE_APP_BAR_HEIGHT);
	mpSecondName = OMalloc<CLabel>(mpWindow, pFont, "");
	mpSecondHull = OMalloc<CProgressBar>(mpWindow, sTrack, sHull, BATTLE_APP_BAR_HEIGHT);
	mpSecondEnergy = OMalloc<CProgressBar>(mpWindow, sTrack, sEnergy, BATTLE_APP_BAR_HEIGHT);

	mpFirstHull->SetAlignment(CSH_Stretch, CSV_Center);
	mpFirstEnergy->SetAlignment(CSH_Stretch, CSV_Center);
	mpSecondHull->SetAlignment(CSH_Stretch, CSV_Center);
	mpSecondEnergy->SetAlignment(CSH_Stretch, CSV_Center);

	mpBars = OMalloc<CGridContainer>(mpWindow);
	mpBars->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpBars->SetInsets(4, 8, 4, 8);
	mpBars->AddComponent(mpFirstName, 0, 0, 1, 1, 0, 0);
	mpBars->AddComponent(mpFirstHull, 1, 0, 1, 1, 2, 0);
	mpBars->AddComponent(OMalloc<CLabel>(mpWindow, pFont, BATTLE_APP_GAP), 2, 0, 1, 1, 0, 0);
	mpBars->AddComponent(mpFirstEnergy, 3, 0, 1, 1, 1, 0);
	mpBars->AddComponent(OMalloc<CLabel>(mpWindow, pFont, BATTLE_APP_WIDE_GAP), 4, 0, 1, 1, 0, 0);
	mpBars->AddComponent(mpSecondName, 5, 0, 1, 1, 0, 0);
	mpBars->AddComponent(mpSecondHull, 6, 0, 1, 1, 2, 0);
	mpBars->AddComponent(OMalloc<CLabel>(mpWindow, pFont, BATTLE_APP_GAP), 7, 0, 1, 1, 0, 0);
	mpBars->AddComponent(mpSecondEnergy, 8, 0, 1, 1, 1, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleApp::Class(void)
{
	M_Pointer(mpWindow);
	M_Pointer(mpFontColours);
	M_Pointer(mpTheme);
	M_Pointer(mpMain);
	M_Pointer(mpBars);
	M_Pointer(mpView);
	M_Pointer(mpFirstName);
	M_Pointer(mpFirstHull);
	M_Pointer(mpFirstEnergy);
	M_Pointer(mpSecondName);
	M_Pointer(mpSecondHull);
	M_Pointer(mpSecondEnergy);
	M_Pointer(mpStatus);
	M_Pointer(mpInput);
	M_Pointer(mpInputSource);
	U_Data(CComponentInput, mcComponentInput);
	U_Bool(mbValid);
	U_Bool(mbInputs);
	U_Bool(mbExit);
	U_Bool(mbSnapshot);
	U_Int32(miSnapshotTicks);
}


//////////////////////////////////////////////////////////////////////////
//
// Only this object's own members: the window and the input may already
// be freed.
//
//////////////////////////////////////////////////////////////////////////
void CBattleApp::Free(void)
{
	if (mbInputs)
	{
		mcComponentInput.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBattleApp::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBattleApp::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the window until it is closed (if Setup worked), then takes the
// component input out of the window.
//
//////////////////////////////////////////////////////////////////////////
bool CBattleApp::Run(void)
{
	if (mbValid)
	{
		mpWindow->Show();
	}

	if (mbInputs)
	{
		mpWindow->SetComponentInput(NULL);
	}
	return mbValid;
}


//////////////////////////////////////////////////////////////////////////
//
// The window's tick: the battle is moved on by the time that's passed,
// or by a fixed step while a snapshot is waited for, and the bars and
// the status say how it stands.
//
//////////////////////////////////////////////////////////////////////////
void CBattleApp::Tick(Ptr<CWindow> pWindow)
{
	if (mbSnapshot)
	{
		mpView->AdvanceBy(BATTLE_APP_SNAPSHOT_STEP);
	}
	else
	{
		mpView->Advance();
	}
	ShowBattle();

	if (miSnapshotTicks >= 0)
	{
		Snapshot(pWindow);
	}
	if (mbExit)
	{
		pWindow->Stop();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The bars and the labels as the battle now is.  The status says what
// the keys are and what's drawing it, that it's waiting for a key or
// paused when it is, and once it's over who won.
//
//////////////////////////////////////////////////////////////////////////
void CBattleApp::ShowBattle(void)
{
	CBattle*	pcBattle;
	CChars		szStatus;
	int32		iWinner;

	ShowShip(0, mpFirstName, mpFirstHull, mpFirstEnergy);
	ShowShip(1, mpSecondName, mpSecondHull, mpSecondEnergy);

	pcBattle = mpView->GetGame()->GetBattle();
	iWinner = pcBattle->GetWinner();
	szStatus.Init();
	if (mpView->IsPaused())
	{
		szStatus.Append("Paused: click the window to play.");
	}
	else if (iWinner == BATTLE_NO_SHIP)
	{
		if (mpView->GetGame()->IsWaiting())
		{
			szStatus.Append("Press a key to begin.   ");
		}
		szStatus.Append(BATTLE_KEYS_TEXT);
		szStatus.Append(mpView->GetDraw()->IsVulkan() ? "Vulkan." : "the rasteriser.");
	}
	else if (iWinner == BATTLE_DRAWN)
	{
		szStatus.Append("Both ships are destroyed.");
	}
	else
	{
		szStatus.Append("The ");
		szStatus.Append(pcBattle->GetShip(iWinner)->pcType->mszName.Text());
		szStatus.Append(" wins.");
	}
	if ((iWinner != BATTLE_NO_SHIP) && !mpView->IsPaused())
	{
		szStatus.Append("   Enter starts again.");
	}
	SetLabel(mpStatus, szStatus.Text());
	szStatus.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBattleApp::ShowShip(int32 iShip, Ptr<CLabel> pName, Ptr<CProgressBar> pHull, Ptr<CProgressBar> pEnergy)
{
	Ptr<CBattleGame>	pGame;
	SBattleShip*		psShip;
	CChars				szName;

	pGame = mpView->GetGame();
	psShip = pGame->GetBattle()->GetShip(iShip);

	szName.Init(psShip->pcType->mszName.Text());
	szName.Append(pGame->IsComputer(iShip) ? " (computer)  " : " (player)  ");
	SetLabel(pName, szName.Text());
	szName.Kill();

	pHull->SetFraction(psShip->fHull / psShip->pcType->mfHull);
	pEnergy->SetFraction(psShip->fEnergy / psShip->pcType->mfEnergy);
}


//////////////////////////////////////////////////////////////////////////
//
// Only when it says something else, so nothing is laid out again for a
// label that hasn't changed.
//
//////////////////////////////////////////////////////////////////////////
void CBattleApp::SetLabel(Ptr<CLabel> pLabel, const char* szText)
{
	if (StringCompare(pLabel->GetText(), szText) != 0)
	{
		pLabel->SetText(szText);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Counts the ticks down, then saves the window as BATTLE_SNAPSHOT's file.
// With BATTLE_EXIT the window then closes.
//
//////////////////////////////////////////////////////////////////////////
void CBattleApp::Snapshot(Ptr<CWindow> pWindow)
{
	CFileUtil		cFileUtil;
	const char*		szFilename;

	if (miSnapshotTicks == 0)
	{
		szFilename = getenv(BATTLE_SNAPSHOT_VARIABLE);
		cFileUtil.TouchDir(szFilename, true);
		SavePNG(pWindow->GetCanvas()->GetImageOrNull(), (char*)szFilename);
		mbExit = mbExit || (getenv(BATTLE_EXIT_VARIABLE) != NULL);
	}
	miSnapshotTicks--;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InitBattleGlobals(void)
{
	gcLogger.Init();
	TypesInit();
	TypeConverterInit();
	MemoryInit();
	DataIOInit();
	ObjectsInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void KillBattleGlobals(void)
{
	ObjectsFlush();
	ObjectsKill();
	DataIOKill();
	MemoryKill();
	TypeConverterKill();
	TypesKill();
	gcLogger.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The app (its window and input) is held by the root while it runs on
// the host's window factory and input.  szInputDirectory has
// WinInputLib's XML device descriptions.
//
//////////////////////////////////////////////////////////////////////////
bool RunBattleApp(CNativeWindowFactory* pcNativeFactory, CNativeInput* pcNativeInput, const char* szInputDirectory)
{
	Ptr<CBattleApp>	pApp;
	bool			bResult;

	//Validating every object each time one changes is too slow for the text's glyphs.  They are validated once at the end.
	gcObjects.DisableValidation();

	pApp = OMalloc<CBattleApp>();
	ORoot()->Add(pApp);
	bResult = pApp->Setup(pcNativeFactory, pcNativeInput, szInputDirectory);
	bResult = bResult && pApp->Run();
	ORoot()->Remove(pApp);
	pApp = NULL;

	gcObjects.EnableValidation();
	gcObjects.ValidateObjectsConsistency();
	return bResult;
}

