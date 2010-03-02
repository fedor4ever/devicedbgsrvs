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

#include "toolsclientsession.h"
#include "toolsclientdefs.h"
#include <f32file.h>
#include "ToolsCmdCodes.h"

// System includes

// User includes

#include "toolsclientutils.h"
// Type definitions

// Constants
const TInt KNumberOfServerStartupAttempts = 2;

// Enumerations

//
// RToolsClientSession (source)
//

//
// RToolsClientSession::RToolsClientSession
//
// Constructor
//
EXPORT_C RToolsClientSession::RToolsClientSession()
{

}

//
//RToolsClientSession::Connect()
//
// Connects the client process to the Tools server, starting the server 
// if it is not already running.
// return KErrNone if successful, otherwise one of the system-wide errors.
//
EXPORT_C TInt RToolsClientSession::Connect()
{
	TInt startupAttempts = KNumberOfServerStartupAttempts;
	for(;;)
	{
		TInt ret = CreateSession(ToolsClientDefs::ServerAndThreadName(), ToolsClientDefs::Version(), KToolsServerAsynchronousSlotCount);
		if (ret != KErrNotFound && ret != KErrServerTerminated)
		{
			return ret;
		}
		
		if	(startupAttempts-- == 0)
		{
			return ret;
		}
		/*
		ret = ToolsClientUtils::StartToolsServer();
		if	(ret != KErrNone && ret != KErrAlreadyExists)
		{
			return ret;
		}
		*/
	}
}

//
// RToolsClientSession::Version()
//
// Provides the version number of the Tools server.
// @return The version number. 
//
EXPORT_C TVersion RToolsClientSession::Version() const
{
	return ToolsClientDefs::Version();
}

// 
// RToolsClientSession::GetConnStatus()
//
// Provides the connection status
//

EXPORT_C TInt RToolsClientSession::GetConnStatus(TConnectionStatus& aConnStatus)
{
    
    TPckg<TConnectionStatus> package(aConnStatus);
    TIpcArgs args(&package);
    return SendReceive(EToolsCmdCodeGetUsbConnStatus, args);
}

// 
// RToolsClientSession::UsbConnNotify()
//
// Asynchronous method to register for USB connection notifications
//
EXPORT_C void RToolsClientSession::UsbConnNotify(TDes8& aDes, TRequestStatus& aStatus)
{
    TIpcArgs args(&aDes);
    SendReceive(EToolsCmdCodeConnNotify, args,aStatus);
}

// 
// RToolsClientSession::UsbConnNotifyCancel()
//
// To cancel the registration for USB connection notifications
//
EXPORT_C TInt RToolsClientSession::UsbConnNotifyCancel()
{
   TIpcArgs args;
   return SendReceive(EToolsCmdCodeConnNotify, args);
}


//
// RToolsClientSession::ShutDownServer()
//
// Closes the server.
// @return KErrNone - if succesful
//         Negative - if failed.
//

EXPORT_C TInt RToolsClientSession::ShutDownServer()
{
	TIpcArgs args;
	return SendReceive(EToolsCmdCodeShutDownServer, args);
}

