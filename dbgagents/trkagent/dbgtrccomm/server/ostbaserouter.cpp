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


#include <e32debug.h>
#include <e32base.h>
#include <e32cons.h>

#include "logging.h"
#include "dbgtrcportmgr.h"
#include "ostprotregistry.h"
#include "portreader.h"
#include "portwriter.h"
#include "dbgtrcsrvsession.h"
#include "ostmessage.h"
#include "ostbaserouter.h"

// Static functions

//
// Swap2
//
// Byte-swaps a 2-byte value.  Used to convert to/from little/big endian.
//
static TUint16 Swap2(TUint16 aSource)
{
	TUint16 dest = 0;
	for (TInt i=0; i<2; i++)
	{
		dest <<= 8;
		dest |= aSource & 0xFF;
		aSource >>= 8;
	}
	
	return dest;
} 

COstWriteMsgEntry* COstWriteMsgEntry::NewL(TDesC8& aMsg, CDbgTrcSrvSession* aMsgListener)
{
	LOG_MSG("COstWriteMsgEntry::NewL");

	COstWriteMsgEntry* self = new(ELeave) COstWriteMsgEntry(aMsgListener);
	CleanupStack::PushL(self);
	self->ConstructL(aMsg);
	CleanupStack::Pop(self);
	return self;
}

COstWriteMsgEntry::COstWriteMsgEntry(CDbgTrcSrvSession* aMsgListener)
: iMessage(NULL),
  iWriteMsgListener(aMsgListener)
{
	LOG_MSG("COstWriteMsgEntry::COstWriteMsgEntry");


}

COstWriteMsgEntry::~COstWriteMsgEntry()
{
	LOG_MSG("COstWriteMsgEntry::~COstWriteMsgEntry");

	SafeDelete(iMessage);
	iWriteMsgListener = NULL; //don't delete the listener since we don't really own it.
}

void COstWriteMsgEntry::ConstructL(TDesC8& aMsg)
{
	LOG_MSG("COstWriteMsgEntry::ConstructL");

	iMessage = COstMessage::NewL(aMsg);
}


COstBaseRouter*	COstBaseRouter::NewL()
{
	LOG_MSG("COstWriteMsgEntry::NewL");

	COstBaseRouter* self = new(ELeave) COstBaseRouter;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}

COstBaseRouter::COstBaseRouter()
: iPortManager(NULL),
  iProtRegistry(NULL),
  iWriteMsgListener(NULL),
  iWriteMsgQueue(1)  
{
	LOG_MSG("COstBaseRouter::COstBaseRouter");

}

COstBaseRouter::~COstBaseRouter()
{	
	LOG_MSG("COstBaseRouter::~COstBaseRouter");

	SafeDelete(iPortManager);
	SafeDelete(iProtRegistry);
	
	iWriteMsgQueue.ResetAndDestroy();	
	iWriteMsgQueue.Close();
}

void COstBaseRouter::ConstructL()
{
	LOG_MSG("COstBaseRouter::ConstructL");

	iPortManager = CDbgTrcPortMgr::NewL();	
	iProtRegistry = COstProtRegistry::NewL();
	
	// check the target endianness
	TUint32	test;
	TUint8 *byte_alias = (TUint8 *)&test;
	
	// Write a specific 4-byte sequence and then read it as
	// a 32-bit word.  Big-endian systems yield one value and
	// little-endian systems yield another.
	
	byte_alias[ 0 ] = 0x12;
	byte_alias[ 1 ] = 0x34;
	byte_alias[ 2 ] = 0x56;
	byte_alias[ 3 ] = 0x78;
	
	if (test == 0x12345678)
		iIsBigEndian = ETrue;
	else
		iIsBigEndian = EFalse;			
	
	iSendMsgBuffer.Zero();
	iRecvMsgBuffer.Zero();
	
	iWriteMsgQueue.Reset();
}

void COstBaseRouter::RegisterProtocol(const TOstProtIds aProtId, CDbgTrcSrvSession* aProtMsgListener, TBool aNeedHeader)
{
	LOG_MSG("COstBaseRouter::RegisterProtocol");

	iProtRegistry->RegisterProtocol(aProtId, aProtMsgListener, aNeedHeader);
	
	// Initiate the reads once someone registers a protocol.
	//ReadMessage();
}

void COstBaseRouter::UnRegisterProtocol(const TOstProtIds aProtId)
{
	LOG_MSG("COstBaseRouter::UnRegisterProtocol");

	iProtRegistry->UnRegisterProtocol(aProtId);
	
	// if listener count is 0, cancel the reads as well.
	if (!iProtRegistry->GetProtListenerCount())
	{
		CPortReader* reader = iPortManager->GetPortReader();
		if (reader)
		{
			reader->Cancel();
		}
	}	
}


void COstBaseRouter::ReadMessage()
{	
	LOG_MSG("COstBaseRouter::ReadMessage");

	CPortReader* reader = iPortManager->GetPortReader();
	
	if (reader)
		reader->StartRead(this);
}


void COstBaseRouter::WriteMessageL(TDesC8& aMsg, CDbgTrcSrvSession* aProtMsgListener)
{
	LOG_MSG("COstBaseRouter::WriteMessage");
	
	iSendMsgBuffer.Zero();
	iSendMsgBuffer.Append(aMsg);
			
	DoWriteMessageL(aProtMsgListener);
}

void COstBaseRouter::WriteMessageL(TDesC8& aMsg, CDbgTrcSrvSession* aProtMsgListener, TOstProtIds aProtId)
{
	LOG_MSG("COstBaseRouter::WriteMessage");	
	
	// write now, we use a fixed size array for the write message,
	// look below for the commented implementation if necessary in the future.
	iSendMsgBuffer.Zero();

	TUint16 msgLength = aMsg.Length();
	
	TUint8 version = OST_PROTOCOL_VERSION;		
	
	iSendMsgBuffer.Append(&version, sizeof(TUint8));
	iSendMsgBuffer.Append((TUint8*)&aProtId, sizeof(TUint8));
	
	TUint16 temp = msgLength; 
	if (!iIsBigEndian)
		temp = Swap2(msgLength);
		
	iSendMsgBuffer.Append((TUint8*)&temp, sizeof(TUint16));
			
	// now append the msg data
	iSendMsgBuffer.Append(aMsg);
		
	DoWriteMessageL(aProtMsgListener);
}


/*
TInt COstBaseRouter::WriteMessage(TDesC8& aMsg, CDbgTrcSrvSession* aProtMsgListener, TOstProtIds aProtId)
{	
	HBufC8* msgBuffer = HBufC8::New(aMsg.Length() + OST_HEADER_LENGTH);
	
	TUint16 msgLength = aMsg.Length();
	
	TUint8 version = OST_PROTOCOL_VERSION;		
	
	msgBuffer->Des().Append(&version, sizeof(TUint8));
	msgBuffer->Des().Append((TUint8*)&aProtId, sizeof(TUint8));
	
	TUint16 temp = msgLength; 
	if (!iIsBigEndian)
		temp = Swap2(msgLength);
		
	msgBuffer->Des().Append((TUint8*)&temp, sizeof(TUint16));
			
	// now append the msg data
	msgBuffer->Des().Append(aMsg);
	
	
}
*/

void COstBaseRouter::DoWriteMessageL(CDbgTrcSrvSession* aProtMsgListener)
{
	LOG_MSG("COstBaseRouter::DoWriteMessage");

	CPortWriter* writer = iPortManager->GetPortWriter();
	
	if(writer)
	{
		if (writer->IsBusy())
		{
			LOG_MSG("Writer busy, queuing up the write message");

			// we are already writing a message
			// so queue up this one, we only have to queue up one message for each connected client for writing.
			COstWriteMsgEntry* writeMsgEntry = COstWriteMsgEntry::NewL(iSendMsgBuffer, aProtMsgListener);
			iWriteMsgQueue.Append(writeMsgEntry);
		}
		else
		{
			// set the write listener for completing the request when the write is done
			iWriteMsgListener = aProtMsgListener;	
			writer->StartWrite(iSendMsgBuffer, this);
		}
	}
}

void COstBaseRouter::DataAvailable(TDesC8& aMsgData, TUint aLength)
{
	LOG_MSG("COstBaseRouter::DataAvailable");
	LOG_MSG2("Received bytes : %d", aLength);	

	TInt remainingBytes = aMsgData.Length();
	const TUint8* recvMsgDataPtr = aMsgData.Ptr();
	while (remainingBytes > 0)
	{	
		TInt bytesToCopy = remainingBytes;
		TInt spaceAvailable = MAX_BUF_SIZE-iRecvMsgBuffer.Length();
		if (spaceAvailable == 0) //should never happen, if this happens, then we will loose messages.
			return;
			
		if (remainingBytes > spaceAvailable)
		{
			bytesToCopy = spaceAvailable;
		}
		
		remainingBytes -= bytesToCopy;		
		iRecvMsgBuffer.Append(recvMsgDataPtr, bytesToCopy);
		recvMsgDataPtr += bytesToCopy;
							
		LOG_MSG2("Bytes copied: %d", bytesToCopy);
		LOG_MSG2("Remaining bytes : %d", remainingBytes);
					
		//
		// We could possibly get more than one message at a time, 
		// so check until we have atleast as many bytes as the OST header.
		//
		LOG_MSG2("Total bytes in the buffer : %d", iRecvMsgBuffer.Length());
		while (iRecvMsgBuffer.Length() > OST_HEADER_LENGTH)
		{
			LOG_MSG("No of received bytes > OST header length");

			const TUint8* ptr = iRecvMsgBuffer.Ptr();

			TUint8 version = ptr[VERS_FIELD_INDEX];
			TUint8 protId  = ptr[PROTID_FIELD_INDEX];
			
			// There could be some junk bytes in the data.
			// With the latest PCSuite/USB cable drivers, there seems to be some 
			// junk bytes in the USB channel. Also we can only do this simple check 
			// only at the beginning of the packet. The only check we have right now 
			// for the OST base protocol is to check for the version and the protocol id	
			if ( !( (version == OST_PROTOCOL_VERSION || version == OST_VERSION_PROTOCOL_VERSION) &&
				 (protId == EOstProtTrk || protId == EOstProtTraceCore || protId == EOstProtOstSystem || protId == EOstProtTrcActivation) ) )
			{
				LOG_MSG2("Invalid version byte: %x", version);
				LOG_MSG2("Invalid protocol byte: %x", protId);
				RemoveNonOstBytes(iRecvMsgBuffer);
			}
				
			LOG_MSG2("Total bytes in the buffer after removing non ost : %d", iRecvMsgBuffer.Length());					
			if (iRecvMsgBuffer.Length()>OST_HEADER_LENGTH)
			{
				// reset ptr just incase if we have deleted some junk bytes
				ptr = iRecvMsgBuffer.Ptr();
				TUint16 packetLength = 0;
				SwapBytes((TUint8*)&packetLength, &ptr[LENGTH_FIELD_INDEX], 2);
			
				LOG_MSG2("packet length is %d", packetLength);			

				if (iRecvMsgBuffer.Length() >= (packetLength+OST_HEADER_LENGTH))
				{
					LOG_MSG("Got one complete message");

					// got one complete message, complete the message if someone is listening for this message.
					TBool needHeader;
					CDbgTrcSrvSession* protMsgListener = iProtRegistry->GetProtListenerForProtId((TOstProtIds)protId, needHeader);
					
					if (protMsgListener)
					{
						LOG_MSG("Found a listener");

						TPtrC8 completeMessage;
						// check to see if we need to strip the header or not.
						if (needHeader)				
							completeMessage.Set(iRecvMsgBuffer.Ptr(), packetLength+OST_HEADER_LENGTH);
						else
							completeMessage.Set(iRecvMsgBuffer.Ptr()+OST_HEADER_LENGTH, packetLength);
							
						TRAPD(err, protMsgListener->MessageAvailableL(completeMessage));
						if (err != KErrNone)
						    LOG_MSG2("Failed to deliver the message: %d", err);
					}	
				
					LOG_MSG2("Deleting bytes in the recieve buffer: %d", packetLength+OST_HEADER_LENGTH);

					// now remove this message from our buffer as its already given to the listener				
					iRecvMsgBuffer.Delete(0, packetLength+OST_HEADER_LENGTH);
					
					LOG_MSG2("Remaining bytes in the recieve buffer: %d", iRecvMsgBuffer.Length());				
				}
				else
				{
					// We have more bytes than the header but we have an incomplete message.
					// So we break out here.
					break; 
				}
			}	
		}
	}
}

void COstBaseRouter::DataWriteComplete(TInt aErrCode)
{		
	LOG_MSG("COstBaseRouter::DataWriteComplete");

	if (iWriteMsgListener != NULL)
	{
		iWriteMsgListener->WriteComplete(aErrCode);
		iWriteMsgListener = NULL;
	}
	else
	{
		LOG_MSG("Invalid write message listener, should never happen");
	}	
	
	if (iWriteMsgQueue.Count() > 0)
	{
		LOG_MSG("Message found in write queue");
		
		COstMessage* ostMsg = iWriteMsgQueue[0]->iMessage;
		iWriteMsgListener = iWriteMsgQueue[0]->iWriteMsgListener;
		HBufC8* msg = ostMsg->iMsgBuffer;
		
		CPortWriter* writer = iPortManager->GetPortWriter();
		if(writer)
			writer->StartWrite(*msg, this);
		
		// now remove the message from the queue
		SafeDelete(iWriteMsgQueue[0]);			
		iWriteMsgQueue.Remove(0);		
	}
}

void COstBaseRouter::GetPortConfig(TDes8& aConfigDes)
{
	LOG_MSG("COstBaseRouter::GetPortConfig");

	iPortManager->GetPortConfig(aConfigDes);
}

TInt COstBaseRouter::SetPortConfig(TDesC8& aConfigDes)
{
	LOG_MSG("COstBaseRouter::SetPortConfig");

	return iPortManager->SetPortConfig(aConfigDes);
}

TInt COstBaseRouter::OpenCommPortL()
{
	LOG_MSG("COstBaseRouter::OpenCommPort");

	return iPortManager->OpenPortL();
}

TInt COstBaseRouter::CloseCommPort()
{
	LOG_MSG("COstBaseRouter::CloseCommPort");
	
	return iPortManager->ClosePort();
}


void COstBaseRouter::SwapBytes(TUint8* aTrgtData, const TUint8* aSrcData, TInt aLength)
{	
	LOG_MSG("COstBaseRouter::SwapBytes");
	
	if (iIsBigEndian)
	{
		Mem::Copy(aTrgtData, aSrcData, aLength);
	}
	else
	{
		for (int i=aLength-1, j=0; i>=0; i--, j++)
			aTrgtData[j] = aSrcData[i];
	}		
}


void COstBaseRouter::RemoveNonOstBytes(TDes8& aMsgBuffer)
{
	LOG_MSG("COstBaseRouter::RemoveNonOstBytes");	
	// We need to check for non OST bytes only in the beginning of the packet.
	// The only check we have right now for the OST base protocol
	// is to check for the version and the protocol id	
	const TUint8* msgDataPtr = aMsgBuffer.Ptr(); 				
	// look for the version byte..
	// if found, bail out and check for protocol byte
	TUint length = aMsgBuffer.Length();
	LOG_MSG2("Total bytes in the buffer: %d", length);
	for (TInt i=VERS_FIELD_INDEX; i<length; i++)
	{			
		if (msgDataPtr[VERS_FIELD_INDEX] == OST_PROTOCOL_VERSION ||
			msgDataPtr[VERS_FIELD_INDEX] == OST_VERSION_PROTOCOL_VERSION)	
		{
			// If we have more than one byte and the first byte seems to be the version byte
			// then check to see whether the second byte is the protocol byte or not.
			// if not, then delete the two bytes and start looking for version byte in the remaining bytes
			if (aMsgBuffer.Length()>1)
			{
				if (msgDataPtr[PROTID_FIELD_INDEX] == EOstProtTrk || 
					msgDataPtr[PROTID_FIELD_INDEX] == EOstProtTraceCore  ||
					msgDataPtr[PROTID_FIELD_INDEX] == EOstProtOstSystem ||
					msgDataPtr[PROTID_FIELD_INDEX] == EOstProtTrcActivation)				
				{
					LOG_MSG("Found a valid packet");
					break;					
				}					
				else
				{
					// we didn't find the version byte and the protocol byte together
					// so delete the two bytes
					LOG_MSG2("Deleting junk bytes, unexpected version: %x", msgDataPtr[VERS_FIELD_INDEX]);
					LOG_MSG2("Deleting junk bytes, unexpected protocol id: %x", msgDataPtr[PROTID_FIELD_INDEX]);

					aMsgBuffer.Delete(VERS_FIELD_INDEX, 2);
					i++; //deleting more than one byte, so increment one more time here.
				}					
			}
			else
			{
				// Special case handling when there is only one remaining byte 
				// and that byte happens to be the version byte
				// This could be an invalid byte from out of protocol data, just happens to be the version byte.
				// So delete this byte as well just so that we don't treat this byte as the version byte by mistake.
				LOG_MSG2("Deleting one remaining byte, unexpected version: %x", msgDataPtr[VERS_FIELD_INDEX]);
				aMsgBuffer.Delete(VERS_FIELD_INDEX, 1);			
			}
		}
		else
		{
			// if the first byte is not the version byte, delete it
			// it could be out of protocol data, we just need throw away
			LOG_MSG2("Deleting junk byte, unexpected version: %x", msgDataPtr[VERS_FIELD_INDEX]);
			aMsgBuffer.Delete(VERS_FIELD_INDEX, 1);
		}
	}		
}
