/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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


#ifdef __WINS__
#error - this driver cannot be built for emulation
#endif

#include <e32def.h>
#include <e32cmn.h>
#include <kernel.h>
#include <kern_priv.h>
#include <arm.h>

#include "TrkXtiComm.h"
#include "TrkXtiDriver.h"

#define KTrkAppSecurUid 0x200170BB
#define KTrkExeSecurUid 0x200159E2

//
// Static function definitions
//


//
//
// DTrkXtiDriverFactory implementation
//
//

//
// DTrkXtiDriverFactory constructor
//
DTrkXtiDriverFactory::DTrkXtiDriverFactory()
{
    iVersion = TVersion(KMajorXtiVersionNumber, KMinorXtiVersionNumber, KBuildXtiVersionNumber);    
}

//
// DTrkXtiDriverFactory::Create
//
TInt DTrkXtiDriverFactory::Create(DLogicalChannelBase*& aChannel)
{
	if (iOpenChannels != 0)
		return KErrInUse; // a channel is already open
	
	aChannel = new DTrkXtiChannel(this);
	
	return aChannel ? KErrNone : KErrNoMemory;
}

//
// DTrkXtiDriverFactory::Install
//
TInt DTrkXtiDriverFactory::Install()
{
    return(SetName(&KTrkXtiDriverName));
}

//
// DTrkXtiDriverFactory::Install
//
void DTrkXtiDriverFactory::GetCaps(TDes8& aDes) const
{
    TCapsTrkXtiDriver b;
    b.iVersion = TVersion(KMajorXtiVersionNumber, KMinorXtiVersionNumber, KBuildXtiVersionNumber);
    
    Kern::InfoCopy(aDes,(TUint8*)&b, sizeof(b));
}


//
//
// DTrkXtiChannel implementation
//
//

//
// DTrkXtiChannel constructor
//
DTrkXtiChannel::DTrkXtiChannel(DLogicalDevice* aLogicalDevice)
					: iRxPendingMsg(EFalse),
					  iInterruptId(76)
{
	
	LOG_MSG("DTrkXtiChannel::DTrkXtiChannel()");

	iDevice = aLogicalDevice;
	
	iClientThread = &Kern::CurrentThread();
	TInt err = iClientThread->Open();	
	
	
	LOG_MSG("DTrkXtiChannel::DTrkXtiChannel() End"); 
}

//
// DTrkXtiChannel destructor
//
DTrkXtiChannel::~DTrkXtiChannel()
{
	LOG_MSG("DTrkXtiChannel::~DTrkXtiChannel()");
	
	Kern::SafeClose((DObject*&)iClientThread, NULL);
	
	if (iLock)
		iLock->Close(NULL);	
}

//
// DTrkXtiChannel::DoCreate
//
TInt DTrkXtiChannel::DoCreate(TInt /*aUnit*/, const TDesC* /*anInfo*/, const TVersion& aVer)
{
	LOG_MSG("DTrkXtiChannel::DoCreate()");

  	if (!Kern::QueryVersionSupported(TVersion(KMajorXtiVersionNumber, KMinorXtiVersionNumber, KBuildXtiVersionNumber), aVer))
		return KErrNotSupported; 

	//Setup the driver for receiving client messages
	iDFCQue = NULL;
	TBuf8<KMaxInfoName> xtiDFC = _L8("XTI DFC");;
	
	TInt err = Kern::DfcQCreate(iDFCQue, 27, &xtiDFC);
	if (err == KErrNone)
	{
		SetDfcQ(iDFCQue);
	}
	else
	{
		SetDfcQ(Kern::DfcQue0());
	}

	iMsgQ.Receive();  	
	
	// create subscriber and subscribe to receive trk messages from TraceCore
	iTrkXtiSubscriber = new DTrkXtiSubscriber;
 	if (!iTrkXtiSubscriber)
		return KErrNoMemory;
 	
	err = iTrkXtiSubscriber->Create(this, iClientThread);
	if (err != KErrNone)
		return err;
	
	err = Kern::SemaphoreCreate(iLock, _L("TrkXtiDriverWriteLock"), 1 /* Initial count */);
	if (err != KErrNone)
		return err;

	return KErrNone;
}

//
// DTrkXtiChannel::DoCancel
//
void DTrkXtiChannel::DoCancel(TInt aReqNo)
{
	LOG_MSG("DTrkXtiChannel::DoCancel()");
	
	switch(aReqNo)
	{
		case RTrkXtiDriver::ERequestReadCancel:
		{
			Kern::RequestComplete(iClientThread, iRxRequestStatus, KErrCancel);
			iRxClientBuffer = NULL;
			iRxRequestStatus = NULL;
			iRxClientBufferLength = 0;
		}
		break;
	}

}

//
// DTrkXtiChannel::DoRequest
//
void DTrkXtiChannel::DoRequest(TInt aReqNo, TRequestStatus* aStatus, TAny* a1, TAny* a2)
{
	LOG_MSG("DTrkXtiChannel::DoRequest()");
		
	switch(aReqNo)
	{
		case RTrkXtiDriver::ERequestRead:
		{
			iRxRequestStatus = aStatus;
			iRxClientBuffer = a1;
			iRxClientBufferLength = (TUint32)a2;
			
			if (iRxPendingMsg)
			{
				TInt err = Kern::ThreadDesWrite(iClientThread, iRxClientBuffer, iRxPendingMsgBuffer, 0, KChunkShiftBy0, iClientThread);
				Kern::RequestComplete(iClientThread, iRxRequestStatus, err);
		
				iRxRequestStatus = NULL;
				iRxPendingMsg = EFalse;
			}

			break;
		}		
		default:
			Kern::RequestComplete(iClientThread, aStatus, KErrNotSupported);
	}
}

//
// DTrkXtiChannel::DoControl
//
TInt DTrkXtiChannel::DoControl(TInt aFunction, TAny* a1, TAny* a2)
{
	LOG_MSG("DTrkXtiChannel::DoControl()");
	
	TInt err = KErrNone;
	
	switch(aFunction)
	{
		case RTrkXtiDriver::EControlWrite:
		{
			//lock the Semaphore so that write doesn't happen when a write is already in progress.
			Kern::SemaphoreWait(*iLock);
			err = DoWrite((TUint32)a1, a2);			
			Kern::SemaphoreSignal(*iLock);			
			break;
		}
		default:
		{
			return KErrGeneral;
		}

	}
	if (KErrNone != err)
	{
		LOG_MSG2("Error %d from control function", err);
	}
	return err;
}

//
// DTrkXtiChannel::HandleMsg
//
void DTrkXtiChannel::HandleMsg(TMessageBase* aMsg)
{
	LOG_MSG("DTrkXtiChannel::HandleMsg()");
	
	TThreadMessage& m = *(TThreadMessage*)aMsg;
	TInt id = m.iValue;


	if (id == (TInt)ECloseMsg)
	{
        if (iTrkXtiSubscriber)
        {
            Kern::SemaphoreWait(*iLock);
            delete iTrkXtiSubscriber;
            iTrkXtiSubscriber = NULL;   
            Kern::SemaphoreSignal(*iLock);              
        }
		m.Complete(KErrNone, EFalse);
		return;
	}

	if (id == KMaxTInt)
	{
		// DoCancel
		DoCancel(m.Int0());
		m.Complete(KErrNone, ETrue); 
		return;
	}

	if (id < 0)
	{
		// DoRequest
		TRequestStatus* pStatus = (TRequestStatus*)m.Ptr0();
		DoRequest(~id, pStatus, m.Ptr1(), m.Ptr2());
		m.Complete(KErrNone, ETrue);
	}
	else
	{
		// DoControl
		TInt err = DoControl(id, m.Ptr0(), m.Ptr1());
		m.Complete(err, ETrue);
	}
}

//
// DTrkXtiChannel::DoWrite
//
TInt DTrkXtiChannel::DoWrite(TUint32 aLength, TAny* a2)
{
	LOG_MSG("DTrkXtiChannel::DoWrite()");

	TInt err = KErrNone;

	err = Kern::ThreadDesRead(iClientThread, a2, iTxBuffer, 0, KChunkShiftBy0);

	if (err == KErrNone)
	{	
		err = WriteXtiChannel(iTxBuffer);
	}
	return err;
}

//
// DTrkXtiChannel::WriteXtiChannel
//
// This function writes all the data in the descriptor in one message
TInt DTrkXtiChannel::WriteXtiChannel(TDesC8& aDes)
{
	TInt err = KErrNone;
		err = iTrkXtiSubscriber->Send(aDes);

	return err;
}





//
// DTrkXtiChannel::DoCompleteRx
//
void DTrkXtiChannel::DoCompleteRx(const TDesC8& aMessage)
{
	LOG_MSG("DTrkXtiChannel::DoCompleteRx()");

	// If there is a pending read request, complete the request with this message.
	// Otherwise store the message, ideally we should queue the messages, but having 
	// a large buffer is failing to initialize at the startup.
	// For now, we will just store only one message
	// If for some reason, TRK is waiting on something and doesn't read for a while, 
	// then its possible that we will loose messages. 
	Kern::SemaphoreWait(*iLock);
	if (iRxRequestStatus)
	{
		TInt err = Kern::ThreadDesWrite(iClientThread, iRxClientBuffer, aMessage, 0, KChunkShiftBy0, iClientThread);
		Kern::RequestComplete(iClientThread, iRxRequestStatus, err);
		
		iRxRequestStatus = NULL;
	}
	else if (!iRxPendingMsg)
	{
		iRxPendingMsgBuffer.Copy(aMessage);
		iRxPendingMsg = ETrue;
	}
	Kern::SemaphoreSignal(*iLock);
}

DECLARE_STANDARD_LDD()
{
    return new DTrkXtiDriverFactory;
}
