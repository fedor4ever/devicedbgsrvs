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

#include "dbgtrccomm.h"
#include "dbgtrcclidefs.h"

// System includes

// User includes
#include "dbgtrccmdcodes.h"
#include "dbgtrccliutils.h"
// Type definitions

// Constants
const TInt KNumberOfServerStartupAttempts = 2;

// Enumerations

//
// RDbgTrcComm (source)
//

//
// RDbgTrcComm::RDbgTrcComm
//
// Constructor
//
EXPORT_C RDbgTrcComm::RDbgTrcComm()
{
}

//
// RDbgTrcComm::Connect()
//
// Connects the client process to the DbgTrc server, starting the server 
// if it is not already running.
// return KErrNone if successful, otherwise one of the system-wide errors.
//
EXPORT_C TInt RDbgTrcComm::Connect()
{
	TInt startupAttempts = KNumberOfServerStartupAttempts;
	for(;;)
	{
		TInt ret = CreateSession(DbgTrcCliDefs::ServerAndThreadName(), DbgTrcCliDefs::Version(), KDbgTrcServerAsynchronousSlotCount);
		if (ret != KErrNotFound && ret != KErrServerTerminated)
		{
			return ret;
		}
		
		if	(startupAttempts-- == 0)
		{
			return ret;
		}
		
		ret = DbgTrcCliUtils::StartDbgTrcServer();
		if	(ret != KErrNone && ret != KErrAlreadyExists)
		{
			return ret;
		}
	}
}


//
//RDbgTrcComm::Disconnect()
//
// Connects the client process to the DbgTrc server, starting the server 
// if it is not already running.
// return KErrNone if successful, otherwise one of the system-wide errors.
//
EXPORT_C TInt RDbgTrcComm::Disconnect()
{	
	// let the session object that we are disconnecting.
	TInt error = SendReceive(EDbgTrcCmdDisconnect);
	
	// needs to close the session somehow, there is no close session right now in RSessionBase.	
	// Either we need to switch to RSubSessionBase or use HandleBase close to see if it closes the session.
	return 	error;
}


//
// RDbgTrcComm::Version()
//
// Provides the version number of the DbgTrc server.
// @return The version number. 
//
EXPORT_C TVersion RDbgTrcComm::Version() const
{
	return DbgTrcCliDefs::Version();
}

//
// RDbgTrcComm::GetAcmConfig(TDes8& aConfig)
//
// Get the current configuration for the ACM port.
// This would be either the default ACM port settings or 
// port settings set by another client if the port is already opened.
// @return KErrNone - if succesful
//		   Negative - if failed.
//
EXPORT_C TInt RDbgTrcComm::GetAcmConfig(TDes8& aConfig)
{
	TIpcArgs args(aConfig.Length(), &aConfig);
	
	return SendReceive(EDbgTrcCmdCodeGetAcmConfig, args);
}

//
// RDbgTrcComm::SetAcmConfig(TDes8& aConfig)
//
// Set the current configuration for the ACM port.
// If the port is already configured by another client, 
// then the function would return KErrInUse, meaning its already set.
//
// @return KErrNone - if succesful
//		   Negative - if failed.
//
EXPORT_C TInt RDbgTrcComm::SetAcmConfig(TDesC8& aConfig)
{
	TIpcArgs args(aConfig.Length(), &aConfig);
	
	return SendReceive(EDbgTrcCmdCodeSetAcmConfig, args);
}


//
// RDbgTrcComm::Open()
//
// Reads data from the already opened file.
// @return KErrNone - if succesful
//		   Negative - if failed.
//
EXPORT_C TInt RDbgTrcComm::Open()
{
	return SendReceive(EDbgTrcCmdCodeOpen);
}

//
// RDbgTrcComm::WriteFile()
//
// Writes data into the already opened file.
// @return KErrNone - if succesful
//		   Negative - if failed.
//
EXPORT_C TInt RDbgTrcComm::Close()
{
	TInt err = SendReceive(EDbgTrcCmdCodeClose);
	
	RHandleBase::Close();	
	return err;
}

//
// RDbgTrcComm::RegisterProtocolID()
//
// Registers the protocol id with the dbgtrc server.
// Only protocol id per session.
// @return KErrNone - if succesful
//		   Negative - if failed.
//
EXPORT_C TInt RDbgTrcComm::RegisterProtocolID(const TOstProtIds aId, TBool aNeedHeader)
{	
	TIpcArgs args(aId, aNeedHeader);

	return SendReceive(EDbgTrcCmdCodeRegisterId, args);
}

//
// RDbgTrcComm::RegisterProtocolIDs()
//
// Registers the protocol ids with the dbgtrc server.
// Only protocol id per session.
// @return KErrNone - if succesful
//		   Negative - if failed.
//
EXPORT_C TInt RDbgTrcComm::RegisterProtocolIDs(const TOstProtIds* aIds, const TUint aNumberofIds, TBool aNeedHeader)
{			
	if (!aIds || aNumberofIds <= 0)
		return KErrArgument;
	
	TPtr8 ids((TUint8*)aIds, aNumberofIds);
	
	TIpcArgs args(aNeedHeader, ids.Length(), &ids);
	 		
	return SendReceive(EDbgTrcCmdCodeRegisterIds, args);			
}

//
// RDbgTrcComm::UnRegisterProtocolID()
//
// Unregisters the protocol id with the dbgtrc server.
// This is probably not necessary, the only case 
// where it might be necessary is when one client wants to recieve two types of messages.
// @return KErrNone - if succesful
//		   Negative - if failed.
//
EXPORT_C TInt RDbgTrcComm::UnRegisterProtocolID(const TOstProtIds aId)
{	
	TIpcArgs args(aId);

	return SendReceive(EDbgTrcCmdCodeUnRegisterId, args);
}

//
// RDbgTrcComm::UnRegisterProtocolIDs()
//
// Unregisters the protocol ids with the dbgtrc server.
// This is probably not necessary, the only case 
// where it might be necessary is when one client wants to recieve two types of messages.
// @return KErrNone - if succesful
//		   Negative - if failed.
//
EXPORT_C TInt RDbgTrcComm::UnRegisterProtocolIDs(const TOstProtIds* aIds, const TUint aNumberofIds)
{			
	if (!aIds || aNumberofIds <= 0)
		return KErrArgument;
	
	TPtr8 ids((TUint8*)aIds, aNumberofIds);
	
	TIpcArgs args(ids.Length(), &ids);
	 		
	return SendReceive(EDbgTrcCmdCodeUnRegisterIds, args);			
}

//
// RDbgTrcComm::ReadMessage(TRequestStatus& aStatus, TDes8& aDes)
//
// Reads the message from the message queue for this session if there is one.
// Otherwise queues up the request.
//
// @return KErrNone - if succesful
//		   Negative - if failed.
//
EXPORT_C void RDbgTrcComm::ReadMessage(TRequestStatus& aStatus, TDes8& aDes)
{
	TIpcArgs args(aDes.MaxLength(), &aDes);
	
	SendReceive(EDbgTrcCmdCodeReadMsg, args, aStatus);	
}

//
// RDbgTrcComm::ReadCancel()
//
// Reads the message from the message queue for this session if there is one.
// Otherwise queues up the request.
//
// @return KErrNone - if succesful
//		   Negative - if failed.
//
EXPORT_C TInt RDbgTrcComm::ReadCancel()
{
	return SendReceive(EDbgTrcCmdCodeReadCancel);	
}


//
// RDbgTrcComm::WriteMessage(TRequestStatus& aStatus, TDes8& aDes, TBool aHasHeader)
//
//
// @return KErrNone - if succesful
//		   Negative - if failed.
//
EXPORT_C void RDbgTrcComm::WriteMessage(TRequestStatus& aStatus, const TDesC8& aDes, TBool aHasHeader)
{
	TIpcArgs args(aHasHeader, aDes.Length(), &aDes);
	
	SendReceive(EDbgTrcCmdCodeWriteMsg, args, aStatus);	
}

//
// RDbgTrcComm::WriteCancel()
//
//
// @return KErrNone - if succesful
//		   Negative - if failed.
//
EXPORT_C TInt RDbgTrcComm::WriteCancel()
{
	return SendReceive(EDbgTrcCmdCodeWriteCancel);	
}
