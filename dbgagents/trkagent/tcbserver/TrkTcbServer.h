/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/


#ifndef __TRKTCBSERVER_H__
#define __TRKTCBSERVER_H__

#include <e32base.h>
#include <f32file.h>
#include <e32cons.h>
#include <hal.h>


//
// class CKeyboardInput
//
// Handles input from the keyboard
//
class CKeyboardInput : public CActive
{
public:

	CKeyboardInput(CConsoleBase* aConsole);
	~CKeyboardInput();

	void ListenToKeyboard();

protected:

	void RunL();
	void DoCancel();

private:

	CConsoleBase* iConsole;
};


//
// class CMetroTrkIO
//
// Handles input and output from/to the console
//
class CMetroTrkIO : public CBase
{
public:

	CMetroTrkIO();
	~CMetroTrkIO();

	void PrintToScreen(TRefByValue<const TDesC> aFmt, ...);
	void Terminating();

private:

	CConsoleBase* iConsole;
	CKeyboardInput* iKeyboardInput;
	TBuf<50> iBuffer;
};

#endif // __TRKTCBSERVER_H__
