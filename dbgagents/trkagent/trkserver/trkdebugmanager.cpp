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

#include "trkdebugmgrcallbacks.h"
#include "trkdebugmanager.h"
#ifndef __TEXT_SHELL__
#include "DebugNotifier.h"
#endif
// System includes
#include <e32debug.h>

//Constants
#ifndef __TEXT_SHELL__
_LIT(KLibraryName, "DebugNotifier.dll");
#endif

#define SafeDelete(x) { if (x) delete x; x = NULL; }

//
// CTrkDebugManager (source)
//

//
// CTrkDebugManager::CTrkDebugManager()
//
// Constructor
//
CTrkDebugManager::CTrkDebugManager() : iTrkEngine(NULL), iConnStatus(ETrkNotConnected), iDebugging(EFalse), iPlugAndPlay(ETrue),
                                       iDebugMgrCallback(NULL)
#ifndef __TEXT_SHELL__                                       
, iToolsConnectionListener(NULL)
#endif
{
    
}

//
// CTrkDebugManager::~CTrkDebugManager()
//
// Destructor
//
CTrkDebugManager::~CTrkDebugManager()
{
    StopListening();
#ifndef __TEXT_SHELL__
    SafeDelete(iToolsConnectionListener);
#endif
    SafeDelete(iTrkEngine);
}

//
// CTrkDebugManager::ConstructL()
//
// Second level constructor
//
void CTrkDebugManager::ConstructL()
{	
	iTrkEngine = CTrkEngine::NewL(this);
#ifndef __TEXT_SHELL__
	TRAPD(err,iToolsConnectionListener = CToolsConnectionListener::NewL(this));
	if(!err)
	iToolsConnectionListener->StartListening();
#endif
}

//
// CTrkDebugManager::NewL()
//
// Creates an instance of CTrkDebugManager
//
CTrkDebugManager* CTrkDebugManager::NewL()
{
	CTrkDebugManager* self = new(ELeave) CTrkDebugManager();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}


//
// CTrkDebugManager::NewLC()
//
// Creates an instance of CTrkDebugManager
//
CTrkDebugManager* CTrkDebugManager::NewLC()
{
	CTrkDebugManager* self = new(ELeave) CTrkDebugManager();
	CleanupStack::PushL(self);
	self->ConstructL();
	
	return self;
}
//
// CTrkDebugManager::StartDebugging()
//
// This method is to start debugging when the connection changes and also to handle command line arguments
//
void CTrkDebugManager::StartDebuggingL(TTrkConnType aConnType)
{   
    TRAPD(error, iTrkEngine->StartL(aConnType));
    
    if (error != KErrNone)
    {
        iTrkEngine->GetErrorInfo(iConnMsg);
        iConnStatus = ETrkNotConnected;
        iTrkEngine->Stop();
    }
    else
    {
        iTrkEngine->GetConnectionInfo(iConnMsg);
        iConnStatus = ETrkConnected;
    }
    User::Leave(error);
}
//
// CTrkSrvScheduler::StartDebuggingL()
//
// This method is to start debugging with the bluetooth connection
//
void CTrkDebugManager::StartDebuggingL()
{	
	TRAPD(error, iTrkEngine->StartL());

	if (error != KErrNone)
	{
		iTrkEngine->GetErrorInfo(iConnMsg);
		iConnStatus = ETrkNotConnected;
		iTrkEngine->Stop();
	}
	else
	{
		iTrkEngine->GetConnectionInfo(iConnMsg);
		iConnStatus = iTrkEngine->GetConnectionStatus();
	}
}
//
// CTrkSrvScheduler::StopDebugging()
//
// This method is to stop the debugging
//
void CTrkDebugManager::StopDebugging()
{   
	iTrkEngine->Stop();
	iConnStatus = ETrkNotConnected;
	iConnMsg = KNullDesC;
}
//
// CTrkSrvScheduler::StopDebugging()
//
// To get the version info of TRK
//
void CTrkDebugManager::GetVersion(TTRKVersion& aVersion)
{
	iTrkEngine->GetVersionInfo(aVersion.iMajor, aVersion.iMinor, aVersion.iMajorAPI, aVersion.iMinorAPI, aVersion.iBuild);
}
//
// CTrkSrvScheduler::GetConnectionMsg()
//
// To find out the connection msg 
//
void CTrkDebugManager::GetConnectionMsg(TDes& aConnStatusMsg)
{
    aConnStatusMsg = iConnMsg;        
}

//
// CTrkDebugManager::ConnectToDebuggerL()
//
void CTrkDebugManager::ConnectToDebuggerL()
{	
    StartDebuggingL();
}

//
// CTrkSrvScheduler::DisConnect()
//
void CTrkDebugManager::DisConnect()
{
    StopDebugging();
}

//
// CTrkDebugManager::GetCurrentConnData()
//
void CTrkDebugManager::GetCurrentConnData(TTrkConnData& aConnData)
{
    iTrkEngine->GetConnectionData(aConnData);
}

//
//CTrkDebugManager::SetCurrentConnData()
//
void CTrkDebugManager::SetCurrentConnData(TTrkConnData& aConnData)
{
    iTrkEngine->SetConnectionData(aConnData);
}
//
// CTrkDebugManager::IsDebugging()
// returns whether debugging started or not
//
TBool CTrkDebugManager::IsDebugging()
{
    iDebugging = iTrkEngine->IsDebugging();
    return iDebugging;
}
//
// CTrkDebugManager::GetPlugAndPlayOption()
// returns whether the plug and play is set or not
//
TBool CTrkDebugManager::GetPlugAndPlayOption()
{   
    return iPlugAndPlay;
    
}
//
// CTrkDebugManager::SetPlugAndPlayOption()
// To set the plug and play option 
//
void CTrkDebugManager::SetPlugAndPlayOption(TBool aPlugAndPlay)
{
    iPlugAndPlay = aPlugAndPlay;
    
}
// ----------------------------------------------------
// CTrkDebugManager::OnConnection
// Called when a connection is opened, right now nothing is done
// as connection is always established from the user side.
// This call would be useful when we add for connecting automatically 
// when a USB cable is plugged in while TRK is running.
// ----------------------------------------------------
//
void CTrkDebugManager::OnConnection()
{
    iConnStatus = ETrkConnected;
    iTrkEngine->GetConnectionInfo(iConnMsg);
    if (iDebugMgrCallback)
         iDebugMgrCallback->DebugConnStatusChanged(iConnStatus);
}

// ----------------------------------------------------
// CTrkDebugManager::OnCloseConnection
// Called when a connection is closed, update any of the UI clients.
// ----------------------------------------------------
//
void CTrkDebugManager::OnCloseConnection()
{   
    iConnStatus = ETrkNotConnected;
    if (iDebugMgrCallback)
          iDebugMgrCallback->DebugConnStatusChanged(iConnStatus);
}

//
// ----------------------------------------------------
// CTrkDebugManager::DebuggingStarted
// Called when a debug session is started.
// ----------------------------------------------------
//
void CTrkDebugManager::DebuggingStarted()
{
    iDebugging = ETrue;
    if (iDebugMgrCallback)
        iDebugMgrCallback->DebuggingStatusChanged(iDebugging);
}

//
// ----------------------------------------------------
// CTrkDebugManager::DebuggingEnded
// Called when a debug session is ended.
// ----------------------------------------------------
//
void CTrkDebugManager::DebuggingEnded()
{
    iDebugging = EFalse;
    if (iDebugMgrCallback)
        iDebugMgrCallback->DebuggingStatusChanged(iDebugging);
}

//
//----------------------------------------------------
//CTrkDebugManager::OnAsyncConnectionFailed()
//Called when bluetooth connection falied
//----------------------------------------------------
//
void CTrkDebugManager::OnAsyncConnectionFailed()
{
    
    iConnStatus = ETrkConnectionError;
    iTrkEngine->GetErrorInfo(iConnMsg);
    if (iDebugMgrCallback)
          iDebugMgrCallback->DebugConnStatusChanged(iConnStatus);
    
}

//
//----------------------------------------------------
// CTrkDebugManager::ConnectionAvailable()
// Called when the usb connection available
//----------------------------------------------------
//
void CTrkDebugManager::ConnectionAvailable()
{   
    iPlugAndPlay = iTrkEngine->GetPlugPlaySetting(); 
  
    if(!iDebugging && iPlugAndPlay)
    {   
       StopDebugging();
       TRAPD(error, StartDebuggingL(ETrkUsbDbgTrc));
        if (!error)
        {   
            TRAP(error, NotifyTheUserL());           
        } 
            if (iDebugMgrCallback)
                iDebugMgrCallback->DebugConnStatusChanged(iConnStatus);
        }       
}

//
//----------------------------------------------------
// CTrkDebugManager::ConnectionUnAvailable()
// Called when the usb cable is disconnected
//----------------------------------------------------
//
void CTrkDebugManager::ConnectionUnAvailable()
{   
    TTrkConnType connType = iTrkEngine->GetConnectionType();
    if(connType == ETrkUsbDbgTrc)
    {
        StopDebugging();
        TRAPD(error, StartDebuggingL(ETrkXti));
        if (!error)
        {   
            // Nothing to do for now                       
        }
        if (iDebugMgrCallback)
            iDebugMgrCallback->DebugConnStatusChanged(iConnStatus);
    }    
}

//
//----------------------------------------------------
// CTrkDebugManager::NotifyTheUser()
// To notify the user that debug services have started
//----------------------------------------------------
//
void CTrkDebugManager::NotifyTheUserL()
{
#ifndef __TEXT_SHELL__
    RLibrary library;
    // Dynamically load DLL
    TInt err = library.Load(KLibraryName);
    if(!err)
    {
        TLibraryFunction entry=library.Lookup(1);
        IDebugNotifier* notify =(IDebugNotifier*)entry();
        CleanupStack::PushL(notify);
        notify->NotifyTheUserL();
        CleanupStack::PopAndDestroy();
        // Finished with the DLL
        library.Close();
    }
#endif    
}
//
//----------------------------------------------------
// CTrkDebugManager::StopListening()
// To stop listening to cable connections
//----------------------------------------------------
//
void CTrkDebugManager::StopListening()
{    
#ifndef __TEXT_SHELL__
    if(iToolsConnectionListener)
    iToolsConnectionListener->StopListening();
#endif
}
