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
#include <e32base.h>
#include <e32cons.h>
#include <hal.h>
#include <iniparser.h>

// Epoc Includes
#include <swi/launcher.h>
// S60 specific headers
#ifdef __S60__
#include <SWInstApi.h>
#endif

#include "TrkSwInstall.h"
#include "TrkSwInstUIHandler.h"

#define SafeDelete(x) { if (x) delete x; x = NULL; }

CTrkSwInstall::CTrkSwInstall()
{
}

TInt CTrkSwInstall::Install(const TDesC& aFileName)
{		
#ifdef __S60__
	TInt error = KErrNone;
	
	SwiUI::RSWInstLauncher swInstLauncher;
	
	error = swInstLauncher.Connect();
	if (!error)
	{
		error = swInstLauncher.Install(aFileName);
		
		swInstLauncher.Close();
	}
	
	return error;
#else
	return KErrNotSupported;	
#endif	
}

TInt CTrkSwInstall::SilentInstallL(const TDesC& aFileName, const TChar aInstallDrive)
{
	TInt error = KErrNone;

	TFileName fileName(aFileName);
	
	fileName.PtrZ();	
	
	Swi::CInstallPrefs* iInstallPrefs = Swi::CInstallPrefs::NewL();
	
	CTrkSWInstUIHandler* iUIHandler = CTrkSWInstUIHandler::NewL(aInstallDrive);
	
	error = Swi::Launcher::Install(*iUIHandler, fileName, *iInstallPrefs);

	if ((KErrNone == error) && (KErrNothing != iUIHandler->iErrCode))
		error = KErrGeneral;

	SafeDelete(iUIHandler);
	SafeDelete(iInstallPrefs);
	
	return error;
}

TInt CTrkSwInstall::UninstallL(const TDesC& aUid)
{
	
	TInt error = KErrNone;

	TLex conv(aUid);
	
	TUint32 id;
	
	error = conv.Val(id,EHex);
	
	if (error != KErrNone)
	{
		return error;
	}
		
	TUid uid = TUid::Uid(id);
		
	
	CTrkSWInstUIHandler* iUIHandler = CTrkSWInstUIHandler::NewL();

	error = Swi::Launcher::Uninstall(*iUIHandler, uid);
	
	SafeDelete(iUIHandler);

	return error;
}

