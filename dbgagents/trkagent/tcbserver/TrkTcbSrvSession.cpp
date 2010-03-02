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

#include "TrkTcbSrvSession.h"

// System includes

//User Includes
#include "TrkTcbCmdCodes.h"
#include "TrkTcbSrvSessionEngine.h"
// Type definitions

// Constants
const TInt KTrkTcbServerTransferBufferExpandSize = 100;
const TInt KSlot0 = 0;
const TInt KSlot1 = 1;
//const TInt KSlot2 = 2;
const TInt KSlot3 = 3;

// Enumerations

// Classes referenced


//
// CTrkTcbSrvSession (source)
//

//
// CTrkTcbSrvServer::CTrkTcbSrvSession()
//
// Constructor
//
CTrkTcbSrvSession::CTrkTcbSrvSession()
{
}

//
// CTrkTcbSrvServer::~CTrkTcbSrvSession()
//
// Destructor
//
CTrkTcbSrvSession::~CTrkTcbSrvSession()
{
	HandleServerDestruction();
	delete iTransferBuffer;
}

//
// CTrkTcbSrvServer::ConstructL()
//
// Creates an instance of CTrkTcbSrvSession.
//
void CTrkTcbSrvSession::ConstructL()
{
	iTransferBuffer = CBufFlat::NewL(KTrkTcbServerTransferBufferExpandSize);
	iSessionEngine = CTrkTcbSrvSessionEngine::NewL();
}

//
// CTrkTcbSrvServer::NewL()
//
// Static self construction
//
CTrkTcbSrvSession* CTrkTcbSrvSession::NewL()
{
	CTrkTcbSrvSession* self = new(ELeave) CTrkTcbSrvSession();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}

//
// CTrkTcbSrvServer::HandleServerDestruction()
//
// Called by the server's destructor. We need to be told that the server is
// being destroyed.
//
void CTrkTcbSrvSession::HandleServerDestruction()
{
	delete iSessionEngine;
	iSessionEngine = NULL;
}

//
// CTrkTcbSrvServer::ServiceL()
//
// Services requests from a client.
// Called by the IPC framework whenever a client sends a request to the server.
//
void CTrkTcbSrvSession::ServiceL(const RMessage2& aMessage)
{
	// Leave's are caught by CASSrvServer::RunError
	iMessage = &aMessage;
	
	const TBool completeMessage = DoServiceL();
	
	//Right now there are no asynchronous message completion.
	//So if the message is not completed, then there is an error and needs to be reported
	//to the client.
	if	(completeMessage)
		aMessage.Complete(KErrNone);
	else
		aMessage.Complete(KErrGeneral);
		
	iMessage=NULL;
}

//
// CTrkTcbSrvServer::ServiceL()
//
// Services the requests.
//
TBool CTrkTcbSrvSession::DoServiceL()
{
	TBool completeMessage = EFalse;
	const TInt cmd = Message().Function();
	//
	switch(cmd)
	{
		case ETrkTcbCmdCodeOpenFile:
			completeMessage = CmdOpenFileL();
			break;
		case ETrkTcbCmdCodeReadFile:
			completeMessage = CmdReadFileL();
			break;
		case ETrkTcbCmdCodeWriteFile:
			completeMessage = CmdWriteFileL();
			break;
		case ETrkTcbCmdCodeCloseFile:
			completeMessage = CmdCloseFileL();
			break;
		case ETrkTcbCmdCodePositionFile:
			completeMessage = CmdPositionFileL();
			break;
		case ETrkTcbCmdCodeShutDownServer:
			completeMessage = CmdShutDownServer();			
			break;
	
		default:
			Message().Panic(KServerIntiatedSessionPanic, ETrkTcbServerInitiatedClientPanicInvalidOperation);
			break;
	}
		
	return completeMessage;
}

//
// CTrkTcbSrvServer::CmdOpenFileL()
//
// Gets the data for open file request and calls the session engine open file method.
//
TBool CTrkTcbSrvSession::CmdOpenFileL()
{
	const TInt fileNameLength = static_cast<TInt>(Message().Int0());

	if (fileNameLength > 0)
	{
		HBufC* filePath = HBufC::NewLC(fileNameLength);
		TPtr pFilePath(filePath->Des());
		Message().ReadL(KSlot1, pFilePath);
	
		const TUint mode = static_cast<TUint>(Message().Int2());
		
		TTime modifiedTime;
		iSessionEngine->OpenFileL(filePath->Des(), mode, modifiedTime);
		
		TPckgC<TTime> package(modifiedTime);
		Message().WriteL(KSlot3, package);

		CleanupStack::PopAndDestroy(filePath);
		
		return ETrue;
	}
	return EFalse;
}

//
// CTrkTcbSrvServer::CmdReadFileL()
//
// Gets the data for read file request and calls the session engine read file method.
//
TBool CTrkTcbSrvSession::CmdReadFileL()
{
	const TInt dataLength = static_cast<TInt>(Message().Int0());
	
	if (dataLength > 0)
	{
		HBufC8* fileData = HBufC8::NewLC(dataLength);
		TPtr8 ptr(fileData->Des());
		
		iSessionEngine->ReadFileL(ptr);
		
		//write the file data into the client descriptor
		Message().WriteL(KSlot1, ptr);
	
		CleanupStack::PopAndDestroy(fileData);
	}

	return ETrue;
}

//
// CTrkTcbSrvServer::CmdWriteFileL()
//
// Gets the data for write file request and calls the session engine write file method.
//
TBool CTrkTcbSrvSession::CmdWriteFileL()
{
	const TInt fileDataLength = static_cast<TInt>(Message().Int0());
	
	if (fileDataLength > 0)
	{
		HBufC8* fileData = HBufC8::NewLC(fileDataLength);
		TPtr8 pFileData(fileData->Des());
		Message().ReadL(KSlot1, pFileData);
		
		iSessionEngine->WriteFileL(*fileData);

		CleanupStack::PopAndDestroy(fileData);
		
		return ETrue;
	}
	return EFalse;
}

//
// CTrkTcbSrvServer::CmdCloseFileL()
//
// Gets the data for close file request and calls the session engine close file method.
//
TBool CTrkTcbSrvSession::CmdCloseFileL()
{
	TTime modificationTime;
	
	TPckg<TTime> package(modificationTime);
	Message().ReadL(KSlot0, package);
	
	iSessionEngine->CloseFileL(modificationTime);
	
	return ETrue;
}

//
// CTrkTcbSrvServer::CmdPositionFileL()
//
// Gets the data for position file request and calls the session engine position file method.
//
TBool CTrkTcbSrvSession::CmdPositionFileL()
{
	TSeek seek;
	
	TPckg<TSeek> seekPckg(seek);
	Message().ReadL(KSlot0, seekPckg);
	
	TInt offset = static_cast<TInt>(Message().Int1());;

	iSessionEngine->PositionFileL(seek, offset);

	return ETrue;
}

//
// CTrkTcbSrvServer::CmdShutDownServer()
//
// Stops the active scheduler. This way, server process will run to completion.
//
TBool CTrkTcbSrvSession::CmdShutDownServer()
{
	CActiveScheduler::Stop();
	
	return ETrue;
}
