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

#include "logging.h"
#include "dbgtrcportmgr.h"
#include "ostbaserouter.h"
#include "dbgtrccmdcodes.h"
#include "ostmessage.h"

#include "dbgtrcsrvsession.h"
#include "dbgtrcsrvserver.h"



// Type definitions

// Constants
const TInt KDbgTrcServerTransferBufferExpandSize = 100;
//const TInt KSlot0 = 0;
const TInt KSlot1 = 1;
const TInt KSlot2 = 2;
//const TInt KSlot3 = 3;

// Enumerations

// Classes referenced

// Static functions
//
// Checks for a null'd handle before attempting complete. 
// If handle is null'd then don't complete as this will panic server.
//
void SafeComplete(const RMessagePtr2& aMessage, TInt aCompletionCode)
{
	if(!aMessage.IsNull())
	{
		aMessage.Complete(aCompletionCode);
	}
}


//
// CDbgTrcSrvSession (source)
//

//
// CTrkTcbSrvServer::CDbgTrcSrvSession()
//
// Constructor
//
CDbgTrcSrvSession::CDbgTrcSrvSession(COstBaseRouter* aOstRouter)
: 	iPendingRead(EFalse),
	iPendingWrite(EFalse),
	iPendingReadBufferLength(0),
	iOstRouter(aOstRouter),
	iRecvMessageQueue(1),
	iProtocolIds(1)
{
	LOG_MSG("CDbgTrcSrvSession::CDbgTrcSrvSession");
}

//
// CTrkTcbSrvServer::~CDbgTrcSrvSession()
//
// Destructor
//
CDbgTrcSrvSession::~CDbgTrcSrvSession()
{
	LOG_MSG("CDbgTrcSrvSession::~CDbgTrcSrvSession");
	
	HandleServerDestruction();
	delete iTransferBuffer;
	
	iRecvMessageQueue.ResetAndDestroy();
	iRecvMessageQueue.Close();
	
	iProtocolIds.Reset();
	iProtocolIds.Close();
	
	//get a reference to the server
	CDbgTrcSrvServer* dbgTrcServer = (CDbgTrcSrvServer*)Server();

	//notify the server that the session has been opened
	if (dbgTrcServer != NULL)
		dbgTrcServer->SessionClosed();
}

//
// CTrkTcbSrvServer::ConstructL()
//
// Creates an instance of CDbgTrcSrvSession.
//
void CDbgTrcSrvSession::ConstructL()
{
	LOG_MSG("CDbgTrcSrvSession::ConstructL");
	
	iTransferBuffer = CBufFlat::NewL(KDbgTrcServerTransferBufferExpandSize);
	
	iRecvMessageQueue.Reset();

	iProtocolIds.Reset();
	
}

void CDbgTrcSrvSession::CreateL()
{
	//get a reference to the server
	CDbgTrcSrvServer* dbgTrcServer = (CDbgTrcSrvServer*)Server();
	
	//notify the server that the session has been opened
	if (dbgTrcServer != NULL)
		dbgTrcServer->SessionOpened();
}
//
// CTrkTcbSrvServer::NewL()
//
// Static self construction
//
CDbgTrcSrvSession* CDbgTrcSrvSession::NewL(COstBaseRouter* aOstRouter)
{
	LOG_MSG("CDbgTrcSrvSession::NewL");

	CDbgTrcSrvSession* self = new(ELeave) CDbgTrcSrvSession(aOstRouter);
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
void CDbgTrcSrvSession::HandleServerDestruction()
{
	LOG_MSG("CDbgTrcSrvSession::HandleServerDestruction");
	
	// disconnect here just in case if the client has not disconnected before shutting down.
	DoDisconnect();
}

//
// CTrkTcbSrvServer::ServiceL()
//
// Services requests from a client.
// Called by the IPC framework whenever a client sends a request to the server.
//
void CDbgTrcSrvSession::ServiceL(const RMessage2& aMessage)
{
	LOG_MSG("CDbgTrcSrvSession::ServiceL");

	const TInt cmd = aMessage.Function();	
	switch(cmd)
	{
		case EDbgTrcCmdCodeGetAcmConfig:
			CmdGetAcmConfigL(aMessage);
			break;
		case EDbgTrcCmdCodeSetAcmConfig:
			CmdSetAcmConfigL(aMessage);
			break;
		case EDbgTrcCmdCodeOpen:
			CmdOpenCommPortL(aMessage);
			break;
		case EDbgTrcCmdCodeClose:
			CmdCloseCommPort(aMessage);
			break;
		case EDbgTrcCmdCodeRegisterId:
			CmdRegisterProtocol(aMessage);
			break;
		case EDbgTrcCmdCodeRegisterIds:
			CmdRegisterProtocolIdsL(aMessage);
			break;
		case EDbgTrcCmdCodeUnRegisterId:
			CmdUnRegisterProtocol(aMessage);
			break;
		case EDbgTrcCmdCodeUnRegisterIds:
			CmdUnRegisterProtocolIdsL(aMessage);
			break;
		case EDbgTrcCmdCodeReadMsg:
			CmdReadMsgL(aMessage);
			break;
		case EDbgTrcCmdCodeReadCancel:
			CmdReadCancel(aMessage);			
			break;
		case EDbgTrcCmdCodeWriteMsg:
			CmdWriteMsgL(aMessage);
			break;
		case EDbgTrcCmdCodeWriteCancel:
			CmdWriteCancel(aMessage);			
			break;
		case EDbgTrcCmdDisconnect:
			CmdDisconnect(aMessage);
		default:
			aMessage.Panic(KServerIntiatedSessionPanic, EDbgTrcServerInitiatedClientPanicInvalidOperation);
			break;
	}
				
}


//
// CTrkTcbSrvServer::CmdGetAcmConfigL()
//
// Gets the current port configuration 
//
void CDbgTrcSrvSession::CmdGetAcmConfigL(const RMessage2& aMessage)
{
	LOG_MSG("CDbgTrcSrvSession::CmdGetAcmConfigL");

	const TInt desLength = static_cast<TInt>(aMessage.Int0());

	if (desLength > 0 && desLength == sizeof(TAcmConfigV01))
	{
		TAcmConfig acmConfig;
		iOstRouter->GetPortConfig(acmConfig);
		
		aMessage.WriteL(KSlot1, acmConfig);		
		aMessage.Complete(KErrNone);
	} 
	else
	{
		aMessage.Complete(KErrBadDescriptor);		
	}
}

//
// CTrkTcbSrvServer::CmdSetAcmConfigL()
//
// Sets the port configuration if its not already set
//
void CDbgTrcSrvSession::CmdSetAcmConfigL(const RMessage2& aMessage)
{
	LOG_MSG("CDbgTrcSrvSession::CmdSetAcmConfigL");

	const TInt desLength = static_cast<TInt>(aMessage.Int0());

	if (desLength > 0 && desLength == sizeof(TAcmConfigV01))
	{
		TAcmConfig acmConfig;
		aMessage.ReadL(KSlot1, acmConfig);
		
		TInt err = iOstRouter->SetPortConfig(acmConfig);		
		
		aMessage.Complete(err);
	} 
	else
	{
		aMessage.Complete(KErrBadDescriptor);
	}
}

//
// CTrkTcbSrvServer::CmdOpenCommPort()
//
// Opens the comm port, if its already opened, 
// just increments the number of active connects and returns true
//
//
void CDbgTrcSrvSession::CmdOpenCommPortL(const RMessage2& aMessage)
{
	LOG_MSG("CDbgTrcSrvSession::CmdOpenCommPort");

	TInt err = iOstRouter->OpenCommPortL();
	aMessage.Complete(err);
}

//
// CTrkTcbSrvServer::CmdCloseCommPort()
//
// Closes the comm port, the port is actually closed when the
// number of active connections is 0
//
//
void CDbgTrcSrvSession::CmdCloseCommPort(const RMessage2& aMessage)
{
	LOG_MSG("CDbgTrcSrvSession::CmdCloseCommPort");

	TInt err = iOstRouter->CloseCommPort();
	aMessage.Complete(err);
}

//
// CTrkTcbSrvServer::CmdRegisterProtocol()
//
// Registers the protocol with the id
// This is necessary to be able to read messages
//
//
void CDbgTrcSrvSession::CmdRegisterProtocol(const RMessage2& aMessage)
{
	LOG_MSG("CDbgTrcSrvSession::CmdRegisterProtocol");

	const TOstProtIds aProtocolId = static_cast<TOstProtIds>(aMessage.Int0());
	
	TBool aNeedHeader = static_cast<TBool>(aMessage.Int1());
	
	iOstRouter->RegisterProtocol(aProtocolId, this, aNeedHeader);
	
	iProtocolIds.Append(aProtocolId);	
	
	aMessage.Complete(KErrNone);
}

//
// CTrkTcbSrvServer::CmdUnRegisterProtocol()
//
// Registers the protocol with the id
// This is necessary to be able to read messages
//
//
void CDbgTrcSrvSession::CmdUnRegisterProtocol(const RMessage2& aMessage)
{
	LOG_MSG("CDbgTrcSrvSession::CmdUnRegisterProtocol");

	const TOstProtIds aProtocolId = static_cast<TOstProtIds>(aMessage.Int0());
	
	iOstRouter->UnRegisterProtocol(aProtocolId);
	for (TInt i=0; i<iProtocolIds.Count(); i++)
	{
		if (iProtocolIds[i] == aProtocolId)
		{
			iProtocolIds.Remove(i);
			break;
		}
	}
	
	aMessage.Complete(KErrNone);
}

//
// CTrkTcbSrvServer::CmdRegisterProtocolIds()
//
// Registers the protocol with the id
// This is necessary to be able to read messages
//
//
void CDbgTrcSrvSession::CmdRegisterProtocolIdsL(const RMessage2& aMessage)
{
	LOG_MSG("CDbgTrcSrvSession::CmdRegisterProtocolIdsL");

	TBool aNeedHeader = static_cast<TBool>(aMessage.Int0());
	const TInt numberOfIds = static_cast<TInt>(aMessage.Int1());

	TBuf8<25> protocolIds;
	aMessage.ReadL(KSlot2, protocolIds);
	
	const TUint8* protocolPtr = protocolIds.Ptr();
	for (TInt i=0; i<numberOfIds; i++)
	{
		iOstRouter->RegisterProtocol((TOstProtIds)protocolPtr[i], this, aNeedHeader);
		iProtocolIds.Append((TOstProtIds)protocolPtr[i]);
	}
	
	aMessage.Complete(KErrNone);

}

//
// CTrkTcbSrvServer::CmdUnRegisterProtocol()
//
// Registers the protocol with the id
// This is necessary to be able to read messages
//
//
void CDbgTrcSrvSession::CmdUnRegisterProtocolIdsL(const RMessage2& aMessage)
{
	LOG_MSG("CDbgTrcSrvSession::CmdUnRegisterProtocolIdsL");

	const TInt numberOfIds = static_cast<TInt>(aMessage.Int0());
	
	TBuf8<25> protocolIds;
	aMessage.ReadL(KSlot1, protocolIds);
	
	const TUint8* protocolPtr = protocolIds.Ptr();
	for (TInt i=0; i<numberOfIds; i++)
	{
		iOstRouter->UnRegisterProtocol((TOstProtIds)protocolPtr[i]);
		for (TInt j=0; j<iProtocolIds.Count(); j++)
		{
			if (iProtocolIds[j] == (TOstProtIds)protocolPtr[i])
			{
				iProtocolIds.Remove(j);
				break;
			}
		}
	}
	
	aMessage.Complete(KErrNone);
}


//
// CTrkTcbSrvServer::CmdReadMsgL()
//
// Gets the data for read file request and calls the session engine read file method.
//
void CDbgTrcSrvSession::CmdReadMsgL(const RMessage2& aMessage)
{		
	LOG_MSG("CDbgTrcSrvSession::CmdReadMsg");

	if (iProtocolIds.Count() <= 0)
	{
		// No protocols are registered, just complete request here.
		aMessage.Complete(KErrAccessDenied);
		return;
	}
	
	if (iRecvMessageQueue.Count() > 0)
	{
		LOG_MSG("Message found in recieve queue");

		iPendingReadBufferLength = static_cast<TUint>(aMessage.Int0());		
		
		COstMessage* ostMsg = iRecvMessageQueue[0];				
		HBufC8* msg = ostMsg->iMsgBuffer;
		
		if (msg->Length() <= iPendingReadBufferLength)
		{
			//write the file data into the client descriptor
			aMessage.WriteL(KSlot1, msg->Des());			
			aMessage.Complete(KErrNone);
			
			// now remove the message from the queue
			SafeDelete(iRecvMessageQueue[0]);			
			iRecvMessageQueue.Remove(0);
		}
		else
		{
			aMessage.Complete(KErrBadDescriptor);
		}
	}
	else
	{
		if (iPendingRead)
		{			
			aMessage.Complete(KErrAlreadyExists);
		}
		else
		{
			iBlockedRead = aMessage;
			iPendingReadBufferLength = static_cast<TUint>(aMessage.Int0());		
			
			iPendingRead = ETrue;
			
			// initiate the read.
			iOstRouter->ReadMessage();
		}
	}
}

void CDbgTrcSrvSession::CmdReadCancel(const RMessage2& aMessage)
{
	LOG_MSG("CDbgTrcSrvSession::CmdReadCancel");
	
	if (iPendingRead && iBlockedRead.Handle())
	{
		SafeComplete(iBlockedRead, KErrCancel);
		iPendingRead = EFalse;						
	}
	
	// now complete the read cancel request.	
	SafeComplete(aMessage, KErrNone);
}

void CDbgTrcSrvSession::CmdWriteMsgL(const RMessage2& aMessage)
{
	LOG_MSG("CDbgTrcSrvSession::CmdWriteMsg");

	if (iPendingWrite)
	{
		aMessage.Complete(KErrAlreadyExists);
	}
	else
	{
		const TBool hasHeader = static_cast<TInt>(aMessage.Int0());
		const TInt msgLength = static_cast<TInt>(aMessage.Int1());
	
		if (msgLength > 0)
		{
			if (!hasHeader && iProtocolIds.Count() != 1)
			{
				// For now, if there is no header, then we should have only one protocol id registered by the client.
				// otherwise we don't support writing the message.
				aMessage.Complete(KErrNotSupported);
				return;
			}
			
			HBufC8* msgData = HBufC8::NewLC(msgLength);
			TPtr8 pMsgData(msgData->Des());
			aMessage.ReadL(KSlot2, pMsgData);
			
			iBlockedWrite = aMessage;
			iPendingWrite = ETrue;
			
			if (hasHeader)
				iOstRouter->WriteMessageL(*msgData, this);
			else
				iOstRouter->WriteMessageL(*msgData, this, iProtocolIds[0]);
			//iOstRouter->WriteMessage(*msgData, this);
			

			CleanupStack::PopAndDestroy(msgData);			
		}
		else
		{
			aMessage.Complete(KErrBadDescriptor);
		}		
	}
}

void CDbgTrcSrvSession::CmdWriteCancel(const RMessage2& aMessage)
{
	LOG_MSG("CDbgTrcSrvSession::CmdWriteCancel");

	// Just complete the pending writes, we don't really try to cancel the actual writes themselves.
	if (iPendingWrite && iBlockedWrite.Handle())
	{
		SafeComplete(iBlockedWrite, KErrCancel);
		iPendingWrite = EFalse;;							
	}
	
	// now complete the write cancel message
	aMessage.Complete(KErrNone);
}

void CDbgTrcSrvSession::CmdDisconnect(const RMessage2& aMessage)
{
	LOG_MSG("CDbgTrcSrvSession::CmdDisconnect");
	
	DoDisconnect();
	aMessage.Complete(KErrNone);
}


void CDbgTrcSrvSession::DoDisconnect()
{
	LOG_MSG("CDbgTrcSrvSession::Disconnect");

	for (TInt i=0; i<iProtocolIds.Count(); i++)
	{
		iOstRouter->UnRegisterProtocol(iProtocolIds[i]);				
	}
	iProtocolIds.Reset();

	if (iPendingRead)
	{
		SafeComplete(iBlockedRead, KErrCancel);
		iPendingRead = EFalse;					
	}
	if (iPendingWrite)
	{
		SafeComplete(iBlockedWrite, KErrCancel);
		iPendingWrite = EFalse;				
	}
}

//
// CTrkTcbSrvServer::CmdShutDownServer()
//
// Stops the active scheduler. This way, server process will run to completion.
//
void CDbgTrcSrvSession::MessageAvailableL(TDesC8& aMsg)
{
	LOG_MSG("CDbgTrcSrvSession::MessageAvailable");

	if (iPendingRead && iBlockedRead.Handle())
	{
		// read is pending, complete it now
		if (aMsg.Length() <= iPendingReadBufferLength)
		{
			LOG_MSG("Completing read request");

			//write the file data into the client descriptor
			TRAPD(err, iBlockedRead.WriteL(KSlot1, aMsg))						
			SafeComplete(iBlockedRead, err);	
			
			LOG_MSG("Completed read request");
				
			iPendingRead = EFalse;
			// the message is completed, return from here
			return;
		}
		else
		{
			LOG_MSG("Bad descriptor unable to complete read request");
			SafeComplete(iBlockedRead, KErrBadDescriptor);
			iPendingRead = EFalse;
		}
	}
	else
	{
		LOG_MSG("Invalid read request handle");
	}
	
	LOG_MSG("Queueing up the message");
	//
	// No pending read, so add the message to the recv queue
	// Right now, there is no limit on the recv queue.
	// Need to check if we need to limit the number of messages to be queued up.
	// 
	COstMessage* ostMessage = COstMessage::NewL(aMsg);
	iRecvMessageQueue.Append(ostMessage);					
}



void CDbgTrcSrvSession::WriteComplete(TInt aErrCode)
{
	LOG_MSG("CDbgTrcSrvSession::WriteComplete");

	if (iPendingWrite && iBlockedWrite.Handle())
	{
		LOG_MSG("Completing write request");

		SafeComplete(iBlockedWrite, aErrCode);
		iPendingWrite = EFalse;
		
		LOG_MSG("Completed write request");
	}
	else
	{
		LOG_MSG("Invalid write message handle");
	}
}

