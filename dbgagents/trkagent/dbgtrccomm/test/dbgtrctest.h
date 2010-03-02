/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef __DbgTrcTest_H__
#define __DbgTrcTest_H__

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
// class CConsoleIO
//
// Handles input and output from/to the console
//
class CConsoleIO : public CBase
{
public:

	CConsoleIO();
	~CConsoleIO();

	void PrintToScreen(TRefByValue<const TDesC> aFmt, ...);
	void Terminating();

private:

	CConsoleBase* iConsole;
	CKeyboardInput* iKeyboardInput;
	TBuf<50> iBuffer;
};

#endif // __DbgTrcTest_H__

