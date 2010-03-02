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

#include "trksrvcmdcodes.h"
#include "trksrvclientutils.h"
#include "trksrvclientdefs.h"
#include "trksrvclisession.h"

// Type definitions

// Constants
const TInt KNumberOfServerStartupAttempts = 4;

// Enumerations

//
// RTrkSrvCliSession (source)
//

//
// RTrkSrvCliSession::RTrkSrvCliSession
//
// Constructor
//
RTrkSrvCliSession::RTrkSrvCliSession()
{
}

//
//RTrkSrvCliSession::ConnectToServer()
//
// Connects the client process to the TrkSrv server, starting the server 
// if it is not already running.
// return KErrNone if successful, otherwise one of the system-wide errors.
//
TInt RTrkSrvCliSession::ConnectToServer()
{
	TInt startupAttempts = KNumberOfServerStartupAttempts;
	for(;;)
	{
		TInt ret = CreateSession(TrkSrvClientDefs::ServerAndThreadName(), TrkSrvClientDefs::Version(), KTrkServerAsynchronousSlotCount);
		if (ret != KErrNotFound && ret != KErrServerTerminated)
		{
			return ret;
		}
		
		if	(startupAttempts-- == 0)
		{
			return ret;
		}
		
		ret = TrkSrvClientUtils::StartTrkServer();
		if	(ret != KErrNone && ret != KErrAlreadyExists)
		{
			return ret;
		}
		
	}
}

//
// RTrkSrvCliSession::GetTrkVersion()
//
// Provides the version number of the Trk server.
// @return The version number. 
//
TInt RTrkSrvCliSession::GetTrkVersion(TInt& aMajorVersion, TInt& aMinorVersion, TInt& aMajorAPIVersion, TInt& aMinorAPIVersion, TInt& aBuildNumber)
{
    TTRKVersion trkVersion; 
    TPckg<TTRKVersion> trkVersionPkg(trkVersion);
	TInt err = SendReceive(ETrkSrvGetVersion, TIpcArgs(&trkVersionPkg));
	if (KErrNone == err)
	{
        aMajorVersion = trkVersion.iMajor;
        aMinorVersion = trkVersion.iMinor;
        aMajorAPIVersion = trkVersion.iMajorAPI;
        aMinorAPIVersion = trkVersion.iMinorAPI;
        aBuildNumber = trkVersion.iBuild;
	}

	return err;
}

//
// RTrkSrvCliSession::Connect()
//
//
TInt RTrkSrvCliSession::Connect()
{	
	return SendReceive(ETrkSrvCmdConnect);
}

//
// RTrkSrvCliSession::ReadFile()
//
TInt RTrkSrvCliSession::DisConnect()
{
	return SendReceive(ETrkSrvCmdDisConnect);
}

//
// RTrkSrvCliSession::GetCurrentConnData()
//
TInt RTrkSrvCliSession::GetCurrentConnData(TTrkConnData& aConnData)
{
    TPckg<TTrkConnData> package(aConnData);
    TIpcArgs args(&package);
    return SendReceive(ETrkSrvCmdGetCurrentConnData, args);
}

//
// RTrkSrvCliSession::SetCurrentConnData()
//
TInt RTrkSrvCliSession::SetCurrentConnData(TTrkConnData& aConnData)
{
    TPckg<TTrkConnData> package(aConnData);
    TIpcArgs args(&package);
    return SendReceive(ETrkSrvCmdSetCurrentConnData, args);
}

//
// RTrkSrvCliSession::GetDebugConnStatus()
//
TInt RTrkSrvCliSession::GetDebugConnStatus(TTrkConnStatus& aConnStatus, TDes& aConnMsg)
{    
    TPckg<TTrkConnStatus> package(aConnStatus);
	TIpcArgs args(&package, aConnMsg.Length(), &aConnMsg);
	
	return SendReceive(ETrkSrvCmdGetDebugConnStatus, args);
}

//
// RTrkSrvCliSession::DebugConnStatusNotify()
//
void RTrkSrvCliSession::DebugConnStatusNotify(TDes8& aConnStatus, TDes& aConnMsg, TRequestStatus& aStatus)
{
    SendReceive(ETrkSrvDebugConnStatus, TIpcArgs(&aConnStatus, &aConnMsg), aStatus);
}

//
// RTrkSrvCliSession::DebugConnStatusNotifyCancel()
//
TInt RTrkSrvCliSession::DebugConnStatusNotifyCancel()
{
    return SendReceive(ETrkSrvDebugConnStatusCancel);
}

//
// RTrkSrvCliSession::GetDebugStatus()
//
TInt RTrkSrvCliSession::GetDebuggingStatus(TBool& aDebugging)
{
	TPckg<TBool> isDebugging(aDebugging);
	return SendReceive(ETrkSrvCmdGetDebuggingStatus, TIpcArgs(&isDebugging));
}

//
// RTrkSrvCliSession::DebuggingStatusNotify()
//
void RTrkSrvCliSession::DebuggingStatusNotify(TDes8& aDebugging, TRequestStatus& aStatus)
{
    SendReceive(ETrkSrvDebuggingStatus, TIpcArgs(&aDebugging), aStatus);   
}

//
// RTrkSrvCliSession::DebuggingStatusNotifyCancel()
//
TInt RTrkSrvCliSession::DebuggingStatusNotifyCancel()
{
    return SendReceive(ETrkSrvDebuggingStatusCancel);
}

TInt RTrkSrvCliSession::GetPlugAndPlayType(TBool& aPlugandPlay)
{
    TPckg<TBool> package(aPlugandPlay);
    return SendReceive(ETrkGetPlugAndPlayType, TIpcArgs(&package));
    
}

TInt RTrkSrvCliSession::SetPlugAndPlayType(TBool aPlugandPlay)
{   
    TPckg<TBool> package(aPlugandPlay);
    return SendReceive(ETrkSetPlugAndPlayType, TIpcArgs(&package));
}

//
// RTrkSrvCliSession::ShutDownServer()
//
// Closes the server.
// @return KErrNone - if succesful
//		   Negative - if failed.
//
TInt RTrkSrvCliSession::ShutDownServer()
{
	return SendReceive(ETrkSrvCmdCodeShutDownServer);
}

