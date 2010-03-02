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

#include "TrkTcbCliSession.h"
#include "TrkTcbClientDefs.h"

// System includes

// User includes
#include "TrkTcbCmdCodes.h"
#include "TrkTcbClientUtils.h"
// Type definitions

// Constants
const TInt KNumberOfServerStartupAttempts = 2;

// Enumerations

//
// RTrkTcbCliSession (source)
//

//
// RTrkTcbCliSession::RTrkTcbCliSession
//
// Constructor
//
RTrkTcbCliSession::RTrkTcbCliSession()
{
}

//
//RTrkTcbCliSession::Connect()
//
// Connects the client process to the TrkTcb server, starting the server 
// if it is not already running.
// return KErrNone if successful, otherwise one of the system-wide errors.
//
TInt RTrkTcbCliSession::Connect()
{
	TInt startupAttempts = KNumberOfServerStartupAttempts;
	for(;;)
	{
		TInt ret = CreateSession(TrkTcbClientDefs::ServerAndThreadName(), TrkTcbClientDefs::Version(), KTrkTcbServerAsynchronousSlotCount);
		if (ret != KErrNotFound && ret != KErrServerTerminated)
		{
			return ret;
		}
		
		if	(startupAttempts-- == 0)
		{
			return ret;
		}
		
		ret = TrkTcbClientUtils::StartTrkTcbServer();
		if	(ret != KErrNone && ret != KErrAlreadyExists)
		{
			return ret;
		}
	}
}

//
// RTrkTcbCliSession::Version()
//
// Provides the version number of the TrkTcb server.
// @return The version number. 
//
TVersion RTrkTcbCliSession::Version() const
{
	return TrkTcbClientDefs::Version();
}

//
// RTrkTcbCliSession::OpenFile()
//
// Opens the specified file in the specified mode.
// @return KErrNone - if succesful
//		   Negative - if failed.
//
TInt RTrkTcbCliSession::OpenFile(const TDesC& aFilePath, TUint aMode, TTime& aModifiedTime)
{
	TPckg<TTime> package(aModifiedTime);
	TInt length = aFilePath.Length();
	TIpcArgs args(length, &aFilePath, aMode, &package);
	
	return SendReceive(ETrkTcbCmdCodeOpenFile, args);
}

//
// RTrkTcbCliSession::ReadFile()
//
// Reads data from the already opened file.
// @return KErrNone - if succesful
//		   Negative - if failed.
//
TInt RTrkTcbCliSession::ReadFile(TUint16 aLength, TDes8& aFileData)
{
	TIpcArgs args(aLength, &aFileData);

	return SendReceive(ETrkTcbCmdCodeReadFile, args);
}

//
// RTrkTcbCliSession::WriteFile()
//
// Writes data into the already opened file.
// @return KErrNone - if succesful
//		   Negative - if failed.
//
TInt RTrkTcbCliSession::WriteFile(const TDesC8& aFileData)
{
	TIpcArgs args(aFileData.Length(), &aFileData);
	
	return SendReceive(ETrkTcbCmdCodeWriteFile, args);
}

//
// RTrkTcbCliSession::CloseFile()
//
// Closes data into the already opened file.
// @return KErrNone - if succesful
//		   Negative - if failed.
//
TInt RTrkTcbCliSession::CloseFile(const TTime& aTime)
{
	TPckg<TTime> package(aTime);
	TIpcArgs args(&package);
	
	return SendReceive(ETrkTcbCmdCodeCloseFile, args);
}

//
// RTrkTcbCliSession::PositionFile()
//
// Changes the current postion in the already opened file
// @return KErrNone - if succesful
//		   Negative - if failed.
//
TInt RTrkTcbCliSession::PositionFile(TSeek aSeek, TInt& aOffset)
{
	TPckg<TSeek>  package(aSeek);
	TIpcArgs args(&package, aOffset);
	
	return SendReceive(ETrkTcbCmdCodePositionFile, args);
}

//
// RTrkTcbCliSession::ShutDownServer()
//
// Closes the server.
// @return KErrNone - if succesful
//		   Negative - if failed.
//
TInt RTrkTcbCliSession::ShutDownServer()
{
	TIpcArgs args;
	return SendReceive(ETrkTcbCmdCodeShutDownServer, args);
}

