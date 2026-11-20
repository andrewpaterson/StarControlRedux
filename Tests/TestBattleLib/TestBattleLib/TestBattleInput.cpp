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
#include <string.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "InputLib/Input.h"
#include "InputLib/NativeInput.h"
#include "WindowLib/ComponentInput.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/Window.h"
#include "WindowLib/WindowInputSource.h"
#include "WindowLib/WindowTick.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "BattleLib/BattleView.h"
#include "TestLib/Assert.h"


#define TEST_BATTLE_INPUT_XML	".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "CommonLibraries" _FS_ "Libraries" _FS_ "WinInputLib" _FS_ "xml"

#define TEST_RI_KEY_BREAK		1
#define TEST_RI_KEY_E0			2

//Windows' virtual keys, which a keyboard's events have whatever the host.
#define VK_TEST_ENTER			13
#define VK_TEST_SPACE			32
#define VK_TEST_LEFT			37
#define VK_TEST_UP				38
#define VK_TEST_RIGHT			39
#define VK_TEST_DOWN			40
#define VK_TEST_SLASH			191


//Laid out as SRawKeyboardEvent in WinInputLib, which follows the 'WinRAW Keyboard' format in WindowsFormats.xml.
struct STestBattleRawKeyboard
{
	uint16	usMakeCode;
	uint16	usFlags;
	uint16	usReserved;
	uint16	usVKey;
	uint32	uiMessage;
	uint32	uiExtraInformation;
};


//Stands in for the host's keyboard: one standard keyboard, described by WinInputLib's XML, that's given raw events
//as a real one gives them.
class CTestBattleRawInput : public CNativeInput
{
public:
	CComponentInput*		mpcComponentInput;
	int						miKeyboard;
	STestBattleRawKeyboard	msKeyboard;
	bool					mbKeyboard;

	void Init(CComponentInput* pcComponentInput)
	{
		mpcComponentInput = pcComponentInput;
		miKeyboard = -1;
		mbKeyboard = false;
	}

	void Kill(void) override
	{
	}

	void AddDevicesTo(CInputDevices* pcDevices) override
	{
		CInputDevice*	pcDevice;

		pcDevice = pcDevices->CreateDevice(pcDevices->GetDescription("WinRAW Keyboard: Standard 104 Button Keyboard"));
		pcDevice->GenerateRestState();
		miKeyboard = pcDevice->GetDeviceID();
	}

	void Update(CInputDevices* pcDevices, uint32 uiSequence) override
	{
		if (mbKeyboard)
		{
			pcDevices->GetDevice(miKeyboard)->Process(&msKeyboard, uiSequence);
			mbKeyboard = false;
		}
	}

	void Key(uint16 usVKey, uint16 usFlags)
	{
		memset(&msKeyboard, 0, sizeof(STestBattleRawKeyboard));
		msKeyboard.usVKey = usVKey;
		msKeyboard.usFlags = usFlags;
		mbKeyboard = true;
		mpcComponentInput->Update();
	}

	void Press(uint16 usVKey, bool bExtended = false) { Key(usVKey, bExtended ? TEST_RI_KEY_E0 : 0); }
	void Release(uint16 usVKey, bool bExtended = false) { Key(usVKey, (bExtended ? TEST_RI_KEY_E0 : 0) | TEST_RI_KEY_BREAK); }
};


//Stops the window after a few ticks, by when it's been laid out and drawn.
class CTestBattleInputTick : public CWindowTick
{
CONSTRUCTABLE(CTestBattleInputTick);
DESTRUCTABLE(CTestBattleInputTick);
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
// A battle's keys as a keyboard gives them: the view is put in a window
// and focussed as the app does it, the window is run, and then each key,
// pressed on a standard keyboard read through WinInputLib's XML, does
// what it should.  Nothing is clicked first.
//
//////////////////////////////////////////////////////////////////////////
void TestBattleInputFromKeyboard(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;
	CFileUtil					cFileUtil;
	char						szDirectory[] = "Output" _FS_ "BattleInput";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.TouchDir(szDirectory);

	ObjectsInit();
	{
		Ptr<CWindow>				pWindow;
		Ptr<CGridContainer>			pGrid;
		Ptr<CBattleView>			pView;
		Ptr<CBattleGame>			pGame;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CInput>					pInput;
		Ptr<CWindowInputSource>		pInputSource;
		CComponentInput				cComponentInput;
		CTestBattleRawInput			cRaw;
		CPointer					pNull;
		SBattleControls*			psFirst;
		SBattleControls*			psSecond;
		SBattleShip*				psShip;
		float32						fSpeed;
		int32						i;

		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 320, 240, szDirectory);
		pWindow = OMalloc<CWindow>("Battle Input", &pNativeFactory, OMalloc<CTestBattleInputTick>(), pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pWindow->SetContinuous(true);

		pTheme = OMalloc<CWindowTheme>(Ptr<CFont>());
		pTheme->SetColour(TC_Editor, Set32BitColour((uint8)40, (uint8)44, (uint8)52));
		pView = OMalloc<CBattleView>(pWindow, pTheme, false);
		pView->SetKeySeconds(0.0f);  //To begin with, a control is on only while its key is down.
		pGame = pView->GetGame();
		pGrid = OMalloc<CGridContainer>(pWindow);
		pGrid->AddComponent(pView, 0, 0, 1, 1, 1, 1);
		pWindow->SetContainer(pGrid);

		pInput = OMalloc<CInput>((CNativeInput*)NULL);
		ORoot()->Add(pInput);
		AssertTrue(pInput->Read(TEST_BATTLE_INPUT_XML));
		cRaw.Init(&cComponentInput);
		AssertTrue(pInput->AddNativeInput(&cRaw));
		pInputSource = OMalloc<CWindowInputSource>(pInput);
		cComponentInput.Init(&pInputSource, &pWindow);
		AssertSize(1, cComponentInput.AddKeyboards());
		pWindow->SetComponentInput(&cComponentInput);

		pView->FocusView();
		pWindow->Show();
		AssertTrue(pWindow->GetFocus()->GetFocussedComponent() == pView);

		psFirst = pGame->GetHeld(0);
		psSecond = pGame->GetHeld(1);

		//The player's first key begins the battle.
		AssertTrue(pGame->IsWaiting());
		cRaw.Press('W');
		AssertTrue(psFirst->bThrust);
		AssertFalse(pGame->IsWaiting());
		cRaw.Release('W');
		AssertFalse(psFirst->bThrust);

		cRaw.Press('A');
		AssertTrue(psFirst->bLeft);
		cRaw.Press('D');  //With A still held.
		AssertTrue(psFirst->bRight);
		AssertTrue(psFirst->bLeft);
		cRaw.Release('A');
		AssertFalse(psFirst->bLeft);
		AssertTrue(psFirst->bRight);
		cRaw.Release('D');
		AssertFalse(psFirst->bRight);

		cRaw.Press(VK_TEST_SPACE);
		AssertTrue(psFirst->bPrimary);
		cRaw.Press('W');  //Thrusting while firing.
		AssertTrue(psFirst->bThrust);
		AssertTrue(psFirst->bPrimary);
		cRaw.Release(VK_TEST_SPACE);
		cRaw.Release('W');
		AssertFalse(psFirst->bPrimary);
		AssertFalse(psFirst->bThrust);

		cRaw.Press('S');
		AssertTrue(psFirst->bSecondary);
		cRaw.Release('S');
		AssertFalse(psFirst->bSecondary);
		AssertFalse(psSecond->bLeft || psSecond->bRight || psSecond->bThrust || psSecond->bPrimary || psSecond->bSecondary);

		cRaw.Press(VK_TEST_LEFT, true);
		AssertTrue(psSecond->bLeft);
		cRaw.Release(VK_TEST_LEFT, true);
		AssertFalse(psSecond->bLeft);
		cRaw.Press(VK_TEST_RIGHT, true);
		AssertTrue(psSecond->bRight);
		cRaw.Release(VK_TEST_RIGHT, true);
		AssertFalse(psSecond->bRight);
		cRaw.Press(VK_TEST_UP, true);
		AssertTrue(psSecond->bThrust);
		cRaw.Release(VK_TEST_UP, true);
		AssertFalse(psSecond->bThrust);
		cRaw.Press(VK_TEST_DOWN, true);
		AssertTrue(psSecond->bPrimary);
		cRaw.Release(VK_TEST_DOWN, true);
		AssertFalse(psSecond->bPrimary);
		cRaw.Press(VK_TEST_SLASH);
		AssertTrue(psSecond->bSecondary);
		cRaw.Release(VK_TEST_SLASH);
		AssertFalse(psSecond->bSecondary);

		//As a remote desktop sends a key: pressed and let go in the same instant, before the battle has moved at all.
		//The control stays on for its key seconds of the battle's time, so the ship does thrust.
		pView->SetKeySeconds(BATTLE_VIEW_KEY_SECONDS);
		psShip = pGame->GetBattle()->GetShip(0);
		AssertFloat(0.0f, pGame->GetBattle()->GetSeconds(), 4);
		cRaw.Release('W');
		cRaw.Press('W');
		cRaw.Release('W');
		AssertTrue(psFirst->bThrust);
		pView->AdvanceBy(1.0f / 60.0f);
		AssertTrue(psShip->bThrusting);
		AssertTrue(psShip->sVelocity.Magnitude() > 0.3f);
		AssertTrue(psFirst->bThrust);
		for (i = 0; i < 8; i++)
		{
			pView->AdvanceBy(1.0f / 60.0f);
		}
		AssertFalse(psFirst->bThrust);  //More than its key seconds later.
		fSpeed = psShip->sVelocity.Magnitude();
		AssertTrue(fSpeed < psShip->pcType->mfThrust * (BATTLE_VIEW_KEY_SECONDS + 2.0f / 60.0f));
		pView->AdvanceBy(1.0f / 60.0f);
		AssertFalse(psShip->bThrusting);

		//Sent again every 80 milliseconds, as it is while the key is held at the other end, the control never goes
		//off: after a second the ship has all the speed a second of thrust gives it.
		psShip->sVelocity.Init(0.0f, 0.0f);
		for (i = 0; i < 60; i++)
		{
			if ((i % 5) == 0)
			{
				cRaw.Release('W');
				cRaw.Press('W');
				cRaw.Release('W');
			}
			pView->AdvanceBy(1.0f / 60.0f);
			AssertTrue(psFirst->bThrust);
		}
		AssertTrue(psShip->sVelocity.Magnitude() > psShip->pcType->mfThrust * 0.9f);
		for (i = 0; i < 10; i++)
		{
			pView->AdvanceBy(1.0f / 60.0f);
		}
		AssertFalse(psFirst->bThrust);

		//A key that stays down keeps its control on however long, and it goes off at once when the key comes up
		//after its key seconds.
		cRaw.Press('A');
		for (i = 0; i < 30; i++)
		{
			pView->AdvanceBy(1.0f / 60.0f);
		}
		AssertTrue(psFirst->bLeft);
		cRaw.Release('A');
		AssertFalse(psFirst->bLeft);
		pView->SetKeySeconds(0.0f);
		pGame->Restart();
		pGame->Begin();

		AssertTrue(pGame->IsComputer(1));
		cRaw.Press('2');
		cRaw.Release('2');
		AssertFalse(pGame->IsComputer(1));
		AssertFalse(pGame->IsComputer(0));
		cRaw.Press('1');
		cRaw.Release('1');
		AssertTrue(pGame->IsComputer(0));

		pView->AdvanceBy(0.25f);
		AssertTrue(pGame->GetBattle()->GetSeconds() > 0.2f);
		cRaw.Press(VK_TEST_ENTER);
		cRaw.Release(VK_TEST_ENTER);
		AssertFloat(0.0f, pGame->GetBattle()->GetSeconds(), 4);
		AssertTrue(pGame->IsWaiting());

		//A window that isn't the active one gets no keys, so its battle stands still until it is again.
		cRaw.Press(VK_TEST_ENTER);
		cRaw.Release(VK_TEST_ENTER);
		AssertFalse(pGame->IsWaiting());
		pWindow->Deactivate();
		AssertTrue(pView->IsPaused());
		pView->Advance();
		pView->Advance();
		AssertFloat(0.0f, pGame->GetBattle()->GetSeconds(), 4);
		pWindow->Activate();
		AssertFalse(pView->IsPaused());

		pWindow->SetComponentInput(NULL);
		cComponentInput.Kill();
		pInputSource = NULL;
		ORoot()->Remove(pInput);
		pInput = NULL;
		pGame = NULL;
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
void TestBattleInput(void)
{
	BeginTests();

	TestBattleInputFromKeyboard();

	TestStatistics();
}

