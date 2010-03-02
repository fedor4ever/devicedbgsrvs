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
// INCLUDE FILES
#include <e32std.h> 
#include <e32base.h>    
#include "logging.h"
#include "toolsusbportlistener.h"
#include "toolslaunchmgr.h" 

#define SafeDelete(x) { if (x) delete x; x = NULL; }

_LIT(KTRKSERVER, "trkserver.exe");
//_LIT(KTRACESERVER, "traceserver.exe");

_LIT(KCMDLINECONNTYPEUSB, "-usb");
_LIT(KCMDLINECONNTYPEXTI, "-xti");

                   
// LOCAL FUNCTION PROTOTYPES

// LOCAL CONSTANTS



//
// CToolsLaunchMgr::NewL
//
CToolsLaunchMgr* CToolsLaunchMgr::NewL()
{
	CToolsLaunchMgr* self = new(ELeave) CToolsLaunchMgr;
	self->ConstructL();
	return self;
}

//
// CTrkEngine::NewLC
//
CToolsLaunchMgr* CToolsLaunchMgr::NewLC()
{
	CToolsLaunchMgr* self = new(ELeave) CToolsLaunchMgr;
	CleanupStack::PushL(self);
	
	self->ConstructL();
	
	return self;
}


//
// CToolsLaunchMgr::ConstructL
// Constructor
//
void CToolsLaunchMgr::ConstructL() 
{
	iUsbPortListener = new CToolsUsbPortListener(this);	
	TRAPD(err, iUsbPortListener->ListenL());
	if (err != KErrNone)
	    LOG_MSG("Failed to start the USB port listener");
	
	// Adding TRKAPP to the launch list
	CToolsProcess* toolsProcess = CToolsProcess::NewL(KTRKSERVER, KCMDLINECONNTYPEUSB, KCMDLINECONNTYPEXTI);
	iToolsProcessList.Append(toolsProcess);

	// Adding TRACESWITCH app
	// For now, we are disabling the autostart of traceserver from toolsstarter
	// When traceserver connects to the tracecore ost ldd at bootup time, 
	// there is a problem for getting the data from the ldd.
	// Until this problem is fixed, traceswitch needs to be started after the bootup
	// for plug-n-play trace support.
	//_LIT(KNOXTI, "");
	
	//toolsProcess = CToolsProcess::NewL(KTRACESERVER, KCMDLINECONNTYPEUSB, KNOXTI);
	//iToolsProcessList.Append(toolsProcess);
}

//
// CToolsLaunchMgr::CToolsLaunchMgr
// Constructor
//
CToolsLaunchMgr::CToolsLaunchMgr()
			: iUsbPortListener(NULL),
			  iToolsProcessList(1),
			  iToolListenerList(1)
{
	
}

//
// CToolsLaunchMgr::~CToolsLaunchMgr
// Destructor
//
CToolsLaunchMgr::~CToolsLaunchMgr()
{
	SafeDelete(iUsbPortListener);
	iToolsProcessList.ResetAndDestroy();
	iToolsProcessList.Close();
	iToolListenerList.ResetAndDestroy();
	iToolListenerList.Close();
}

//
// CToolsLaunchMgr::Launch
//
void CToolsLaunchMgr::Launch() 
{
    if (iUsbPortListener->IsConnectionAvailable())
	{
		LaunchTools(EUsb);
	}
	else
	{
		LaunchTools(EXti);
	}
}

//
// CToolsLaunchMgr::ConnectionAvailable
//
void CToolsLaunchMgr::ConnectionAvailable() 
{
    
    for (TInt i=0; i<iToolListenerList.Count(); i++)
    {
         iToolListenerList[i]->ConnStatusChanged(EUsbConnected);        
        
    }
    
}

//
// CToolsLaunchMgr::LaunchTools
//
void CToolsLaunchMgr::LaunchTools(TCmdLineConnType aConnType) 
{ 
    // remove this process from our list
	for (TInt i=0; i<iToolsProcessList.Count(); i++)
	{
		TInt err = iToolsProcessList[i]->Start(aConnType);
		
		if (err != KErrNone)
		{
		   LOG_MSG2("Failed to start the process: %d", err);
		}
	}
}

//
// CToolsLaunchMgr::ConnectionAvailable
//
void CToolsLaunchMgr::ConnectionUnAvailable() 
{    
    for (TInt i=0; i<iToolListenerList.Count(); i++)
    {
        iToolListenerList[i]->ConnStatusChanged(EUsbNotConnected);        
       
    }
}

//
// CToolsLaunchMgr::StopTools
//
void CToolsLaunchMgr::StopTools() 
{
	// remove this process from our list
	for (TInt i=0; i<iToolsProcessList.Count(); i++)
	{
		iToolsProcessList[i]->Stop();
	} 
}
//
//  CToolsLaunchMgr::GetUsbConnStatus()
//
//  To Find out the usb connection status  
//
TConnectionStatus CToolsLaunchMgr::GetUsbConnStatus()
{
    return (iUsbPortListener->GetUsbConnStatus());
    
}
//
//  CToolsLaunchMgr::RegisterListener()
//
//  To register the cable connection events  
//

void CToolsLaunchMgr::RegisterListener(MConnStateListener* aListener)
{
    iToolListenerList.Append(aListener);
}

//
//  CToolsLaunchMgr::UnregisterListener()
//
//  To unregister the cable connection events  
//
void CToolsLaunchMgr::UnregisterListener(MConnStateListener* aListener)
{
    for (TInt i=0; i<iToolListenerList.Count(); i++)
    {
        if(iToolListenerList[i] == aListener)
        {
            iToolListenerList.Remove(i);
        }        
       
    }
            
}
//
//  CToolsLaunchMgr::CloseCrashLogger()
//
//  To close the mobile crash logger
//

TInt CToolsLaunchMgr::CloseCrashLogger()
{
    TInt err = KErrNone;
    
    //The old mobile crash file name is "d_exc_mc.exe" and the new one is "mc_useragent.exe"    
    //This is the string that needs to be passed to the RProcess::Open call to get a handle.
    //The complete process name actually includes the UID info as well.
    //Instead of hard coding the process name, its better to just 
    //search for the process and find it that way.      
    //_LIT16(KCrashLoggerName, "mc_useragent.exe[1020e519]0001");
    _LIT16(KOldCrashLoggerName, "d_exc_mc*");
    _LIT16(KCrashLoggerName, "mc_useragent*");
    
    err = TerminateProcess(KOldCrashLoggerName);
    err = TerminateProcess(KCrashLoggerName);

    return err;
}

//
//  CToolsLaunchMgr::TerminateProcess
//
//  Terminates the given process 
//
TInt CToolsLaunchMgr::TerminateProcess(const TDesC& aProcessName)
{
    _LIT16(KCrashLoggerClMsg, "Closing crash logger");
    
    TFindProcess find(aProcessName);
    TFullName name; 
        
    TInt err = find.Next(name);
    if (KErrNone == err)
    {   
        RProcess process;
        err = process.Open(find);
    
        if (KErrNone == err)
        {
            User::InfoPrint(KCrashLoggerClMsg);
            process.Kill(KErrNone);
        }
    }
    return err;
}
