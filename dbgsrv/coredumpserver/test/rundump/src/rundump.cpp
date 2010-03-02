// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#include "rundump.h"
#include <e32base.h>
#include <e32std.h>
#include <e32cons.h>			// Console

#include <coredumpserverapi.h>
#include <e32property.h>
#include <crashdatasave.h>



//  Constants

_LIT(KTextConsoleTitle, "Console");
_LIT(KTextFailed, " failed, leave code = %d");
_LIT(KTextPressAnyKey, " [press any key]\n");
_LIT(KCrashAppFileName,"Z:\\sys\\bin\\crashapp.exe");

//  Global Variables

LOCAL_D CConsoleBase* console;  // write all messages to this


//  Local Functions

LOCAL_C void MainL()
	{
	TUid SELF_UID_V2 = { 0x102836bb };
	TUid WRITER_UID = { 0x102831E4 };
	TUid CDS_UID  = { 0x10282FE5 };
	TUid UTRACE_UID = { 0x102836BA };
	TUid DEXC_UID = { 0x102832C5 };
	
	RCoreDumpSession cdsSess;
	CleanupClosePushL(cdsSess);
	cdsSess.Connect();	
	
	//
	// add your program code here, example code below
	//
	console->Write(_L("Starting megadump\n"));
	console->Write(_L("SELF or UTRACE or DEXC? (s/u/d)"));
	TChar inp = console->Getch();	

	//Load plugins	
	TPluginRequest req;
	req.iPluginType = TPluginRequest::EFormatter;
	req.iLoad = ETrue;
	
	switch(inp)
	{
	case('s'):
		{
		req.iUid = SELF_UID_V2;
		console->Write(_L("\nLoading SELF formatter V2\n"));
		break;
		}
	case('u'):
		{
		req.iUid = UTRACE_UID;
		console->Write(_L("\nLoading UTRACE formatter\n"));
		break;
		}
	case('d'):
		{
		req.iUid = DEXC_UID;
		console->Write(_L("\nLoading DEXC Formatter"));
		return;
		}
	default:
		{
		console->Write(_L("\nNot an option"));
		return;
		}
		
	}
	
	TBuf<10> crashAppParam; 
	console->Write(_L("Should crashapp write trace data? (y/n)\n"));
	inp = console->Getch();
	switch(inp)
	{
	case('y'):
		{
		console->Write(_L("Crashapp will write trace data\n"));
		crashAppParam.Append(_L("-d1 -t"));
		break;
		}
	default:
		{
		console->Write(_L("Crashapp will not write any trace data\n"));
		crashAppParam.Append(_L("-d1"));
		break;
		}
	}
	
	//Start the process that we intend to crash....
	RProcess crashProcess;
	CleanupClosePushL(crashProcess);
	crashProcess.Create( KCrashAppFileName, crashAppParam);
	
    cdsSess.PluginRequestL( req );
    
    TPluginRequest req2;
    req2.iUid = WRITER_UID;
    req2.iPluginType = TPluginRequest::EWriter;
    req2.iLoad = ETrue;
	
    console->Write(_L("Loading File writer\n"));
	cdsSess.PluginRequestL( req2 );
	
	_LIT( KPostCrashEventActionPrompt, "Action After Crash:\n  0-None,\n  1-Resume Thread,\n  2-Resume Process,\n  4-Kill Process" );
	_LIT( KFilePathPrompt, "not_important" );
	_LIT( KCrashFileName, "e:\\file");
	
	console->Write(_L("Configuring CDS\n"));
	DoConfigureL(2, CDS_UID.iUid, COptionConfig::ECoreDumpServer, COptionConfig::ETUInt, KPostCrashEventActionPrompt,
			 1, KNullDesC, 4, KNullDesC, 0, cdsSess);
	
	console->Write(_L("Configuring Writer\n"));
	DoConfigureL((TInt)(CCrashDataSave::ECoreFilePath), WRITER_UID.iUid, COptionConfig::EWriterPlugin, COptionConfig::ETFileName,
		    KFilePathPrompt, 1, KNullDesC, 0, KCrashFileName, 0, cdsSess);
	
/*	if(trace)
		{
		console->Write(_L("Configuring Trace\n"));
		DoConfigureL(0, UTRACE_UID.iUid, COptionConfig::EFormatterPlugin, COptionConfig::ETInt,
		    KFilePathPrompt, 1, KNullDesC, 1, KCrashFileName, 0, cdsSess);
		}*/
	
	console->Write(_L("Observing\n"));
	cdsSess.ObservationRequestL( KCrashAppFileName, KCrashAppFileName, ETrue) ;
	
	console->Write(_L("Starting process\n"));
	crashProcess.Resume();
	CleanupStack::PopAndDestroy(&crashProcess);
	CleanupStack::PopAndDestroy(&cdsSess);
	
	console->Write(_L("Any key to exit...."));
	console->Getch();
	
	}

void DoConfigureL(const TUint32& aIndex, 
 	     const TUint32& aUID, 
        const COptionConfig::TParameterSource& aSource, 
        const COptionConfig::TOptionType& aType, 
        const TDesC& aPrompt, 
        const TUint32& aNumOptions,
        const TDesC& aOptions,
        const TInt32& aVal, 
        const TDesC& aStrValue,
        const TUint aInstance,
        const RCoreDumpSession &aSess)
/**
* @return void
* @param aIndex Internal index to the component that owns the object
* @param aUID UID of the component that owns the object
* @param aSource Type of component that owns the object
* @param aType Type of parameter
* @param aPrompt Prompt to present to user 
* @param aNumOptions Number of options that the parameter can be set to. Only applies if type is ETMultiEntryEnum.
* @param aOptions Comma separated list of options. Applies to ETMultiEntryEnum and ETBool
* @param aVal Integer value. Applies to ETInt, ETUInt, ETBool
* @param aStrValue String value. Applies to ETString, ETFileName, ETMultiEntry, ETBool
*/
	{
	COptionConfig * config;	
	
	config = COptionConfig::NewL( aIndex,
		aUID,
		aSource,
		aType,
		aPrompt,
		aNumOptions,
		aOptions,
		aVal,
		aStrValue);
	
	CleanupStack::PushL(config);
	
	config->Instance(aInstance);
	
	//Configure now...
	aSess.SetConfigParameterL(*config);	
	
	CleanupStack::PopAndDestroy(config);
	}


LOCAL_C void DoStartL()
	{
	// Create active scheduler (to run active objects)
	CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
	CleanupStack::PushL(scheduler);
	CActiveScheduler::Install(scheduler);

	TRAPD(err, MainL());
	
	if(err != KErrNone)
		{
		console->Printf(_L("All is not well in the land of megadump. Error = %d\n"), err);
		}

	// Delete active scheduler
	CleanupStack::PopAndDestroy(scheduler);
	}


//  Global Functions

GLDEF_C TInt E32Main()
	{
	// Create cleanup stack
	__UHEAP_MARK;
	CTrapCleanup* cleanup = CTrapCleanup::New();

	// Create output console
	TRAPD(createError, console = Console::NewL(KTextConsoleTitle, TSize(KConsFullScreen,KConsFullScreen)));
	if (createError)
		return createError;

	// Run application code inside TRAP harness, wait keypress when terminated
	TRAPD(mainError, DoStartL());
	if (mainError)
		console->Printf(KTextFailed, mainError);
	console->Printf(KTextPressAnyKey);
	console->Getch();
	
	delete console;
	delete cleanup;
	__UHEAP_MARKEND;
	return KErrNone;
	}



