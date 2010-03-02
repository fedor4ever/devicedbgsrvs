/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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

#include "TrkDccComm.h"
#include "TrkDccDriver.h"

//This macro turns on DCC3 mode, this is always turned on.
//To test DCC1 mode, comment this out and make sure to configure T32 to use DCC1 as well.
#define DCC3

//currenly both start and end bytes for the packet are the same
#define PKT_STRT_BYTE 0x7E
#define PKT_END_BYTE 0x7E

#define KTrkAppSecurUid 0x200170BB
#define KTrkExeSecurUid 0x200159E2
//
// Static function definitions
//


//
//
// DTrkDccDriverFactory implementation
//
//

//
// DTrkDccDriverFactory constructor
//
DTrkDccDriverFactory::DTrkDccDriverFactory()
{
    iVersion = TVersion(KMajorDccVersionNumber, KMinorDccVersionNumber, KBuildDccVersionNumber);    
}

//
// DTrkDccDriverFactory::Create
//
TInt DTrkDccDriverFactory::Create(DLogicalChannelBase*& aChannel)
{
	if (iOpenChannels != 0)
		return KErrInUse; // a channel is already open
	
	aChannel = new DTrkDccChannel(this);
	
	return aChannel ? KErrNone : KErrNoMemory;
}

//
// DTrkDccDriverFactory::Install
//
TInt DTrkDccDriverFactory::Install()
{
    return(SetName(&KTrkDccDriverName));
}

//
// DTrkDccDriverFactory::Install
//
void DTrkDccDriverFactory::GetCaps(TDes8& aDes) const
{
    TCapsTrkDccDriver b;
    b.iVersion = TVersion(KMajorDccVersionNumber, KMinorDccVersionNumber, KBuildDccVersionNumber);
    
    Kern::InfoCopy(aDes,(TUint8*)&b, sizeof(b));
}


//
//
// DTrkDccChannel implementation
//
//

//
// DTrkDccChannel constructor
//
DTrkDccChannel::DTrkDccChannel(DLogicalDevice* aLogicalDevice)
				: iRxTimer(DTrkDccChannel::RxTimerCallBack, this),
				  iRxBufSize(MAXMESSAGESIZE*2),
				  iRxGetIndx(0),
				  iRxPutIndx(0),
				  iRxCompleteDfc(DTrkDccChannel::RxTimerDfc, this, 2),
				  iInterruptId(76),
				  iPacketStarted(EFalse),
				  iPacketEnded(EFalse)


{
	LOG_MSG("DTrkDccChannel::DTrkDccChannel()");

	iDevice = aLogicalDevice;
	
	iClientThread = &Kern::CurrentThread();
	TInt err = iClientThread->Open();	
	
	iRxTimeOut = NKern::TimerTicks(1); 
}

//
// DTrkDccChannel destructor
//
DTrkDccChannel::~DTrkDccChannel()
{
	LOG_MSG("DTrkDccChannel::~DTrkDccChannel()");

	iRxTimer.Cancel();
	
	if (iRxBuffer)
	{
		delete iRxBuffer;
		iRxBuffer = NULL;
	}
	if (iLock)
		iLock->Close(NULL);

	Kern::SafeClose((DObject*&)iClientThread, NULL);	
}

//
// DTrkDccChannel::DoCreate
//
TInt DTrkDccChannel::DoCreate(TInt /*aUnit*/, const TDesC* /*anInfo*/, const TVersion& aVer)
{
	LOG_MSG("DTrkDccChannel::DoCreate()");

  	if (!Kern::QueryVersionSupported(TVersion(KMajorDccVersionNumber, KMinorDccVersionNumber, KBuildDccVersionNumber), aVer))
		return KErrNotSupported; 

	//Setup the driver for receiving client messages
	iDFCQue = NULL;
	TBuf8<KMaxInfoName> dccDFC = _L8("DCC DFC");;
	
	TInt err = Kern::DfcQCreate(iDFCQue, 27, &dccDFC);
	if (err == KErrNone)
	{
		SetDfcQ(iDFCQue);
	}
	else
	{
		SetDfcQ(Kern::DfcQue0());
	}

	iMsgQ.Receive();  	
	
	iRxCompleteDfc.SetDfcQ(Kern::TimerDfcQ());
	
	iRxBuffer=(TUint8*)Kern::Alloc(iRxBufSize);

	err = Kern::SemaphoreCreate(iLock, _L("TrkDccDriverWriteLock"), 1 /* Initial count */);
	if (err != KErrNone)
		return err;

	return KErrNone;
}

//
// DTrkDccChannel::DoCancel
//
void DTrkDccChannel::DoCancel(TInt aReqNo)
{
	LOG_MSG("DTrkDccChannel::DoCancel()");
	
	switch(aReqNo)
	{
		case RTrkDccDriver::ERequestReadCancel:
		{
			Kern::RequestComplete(iClientThread, iRxRequestStatus, KErrCancel);
			iRxClientBuffer = NULL;
			iRxRequestStatus = NULL;
			iRxClientBufferLength = 0;
			
			iRxTimer.Cancel();
		}
		break;
	}

}

//
// DTrkDccChannel::DoRequest
//
void DTrkDccChannel::DoRequest(TInt aReqNo, TRequestStatus* aStatus, TAny* a1, TAny* a2)
{
	LOG_MSG("DTrkDccChannel::DoRequest()");
		
	switch(aReqNo)
	{
		case RTrkDccDriver::ERequestRead:
		{
			iRxRequestStatus = aStatus;
			iRxClientBuffer = a1;
			iRxClientBufferLength = (TUint32)a2;
			//start the polling timer...
			iRxTimer.OneShot(iRxTimeOut);
			break;
		}		
		default:
			Kern::RequestComplete(iClientThread, aStatus, KErrNotSupported);
	}
}

//
// DTrkDccChannel::DoControl
//
TInt DTrkDccChannel::DoControl(TInt aFunction, TAny* a1, TAny* a2)
{
	LOG_MSG("DTrkDccChannel::DoControl()");
	
	TInt err = KErrNone;
	
	switch(aFunction)
	{
		case RTrkDccDriver::EControlWrite:
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
// DTrkDccChannel::HandleMsg
//
void DTrkDccChannel::HandleMsg(TMessageBase* aMsg)
{
	LOG_MSG("DTrkDccChannel::HandleMsg()");
	
	TThreadMessage& m = *(TThreadMessage*)aMsg;
	TInt id = m.iValue;


	if (id == (TInt)ECloseMsg)
	{
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
// DTrkDccChannel::DoWrite
//
TInt DTrkDccChannel::DoWrite(TUint32 aLength, TAny* a2)
{
	LOG_MSG("DTrkDccChannel::DoWrite()");

	TInt err = KErrNone;

	err = Kern::ThreadDesRead(iClientThread, a2, iTxBuffer, 0, KChunkShiftBy0);

	if (err == KErrNone)
	{	
		err = WriteDccChannel(iTxBuffer);
	}
	return err;
}

//
// DTrkDccChannel::DoRead
//
TInt DTrkDccChannel::DoRead()
{	
	TInt err = KErrNone;
	
	ReadDccChannel();

	if (iRxPutIndx != iRxGetIndx)
	{
		if (iPacketEnded)
		{
			iPacketStarted = EFalse;
			iPacketEnded = EFalse;
		}
		//there is some data, complete the read request if there is one pending
		DoCompleteRx();
		
		if (iRxPutIndx == iRxGetIndx)
		{
			iRxPutIndx = iRxGetIndx = 0;
		}
	}
	else
	{
		err = -1;
		//start the timer again since we didn't complete the request
		iRxTimer.OneShot(iRxTimeOut);
	}
	
	return err;			
}

//
// DTrkDccChannel::WriteDccChannel
//
// This function writes all the data in the descriptor word by word.
// DCC3 mode is used for transferring the data to the DCC channel.
// With DCC3, the number of bytes in the word is encoded in its MSB.
// DCC3 mode can be turned of by commenting the DCC3 macro at the 
// beginning of this file.
//
TInt DTrkDccChannel::WriteDccChannel(const TDesC8& aDes)
{
	TInt err = KErrNone;
	
	const TText8* pStart = aDes.Ptr();
	const TText8* pEnd = pStart + aDes.Length();
	if (pStart != pEnd)
	{
		while (pStart < pEnd)
		{			
			TUint retries = 10;

		#ifdef DCC3
			TUint32 data = 0;
			//for DCC3 support
			if ((pEnd-pStart) >= 3)
			{
				data = ((TUint32)(*pStart++)) | ((TUint32)(*pStart++))<<8 | ((TUint32)(*pStart++))<<16 | (0x02<<24);
			}
			else if ((pEnd-pStart) == 2)
			{
				data = ((TUint32)(*pStart++)) | ((TUint32)(*pStart++))<<8 |  (0x01<<24);
			}
			else if ((pEnd-pStart) == 1)
			{
				data = (TUint32)(*pStart++);
			}
		#endif
			
			while (retries--)
			{
			#ifdef DCC3
				err = Arm::DebugOutJTAG(data);		
			#else
				err = Arm::DebugOutJTAG(*pStart);		
			#endif
				
				//if we get timeout error, try three times sending the same byte
				if (err == KErrTimedOut)
				{
					LOG_MSG("Write timed out, trying again\n");
					continue;
				}
				else
					break;
			}
			if (err)
			{
				LOG_MSG2("WriteDccChannel() failed %d", err);
				return err;
			}
			
		#ifndef DCC3
			pStart++;
		#endif
		
		}
	}
	return err;
}

//
// DTrkDccChannel::ReadDccChannel
//
// Strictly reads packet by packets since TRK expects the host debugger 
// to wait for the response for the cmd sent. 
// This function will return if 
// 	- the packet is completely read
//  - if out of band data, i.e. data not conforming to TRK protocol packets
//	- if an error other than KErrNotReady happens in while in the middle of 
//    reading a packet.
// Also the function uses DCC3 mode, where the no of bytes sent in the word
// is encoded in the MSB and the actual data bytes in the remaining bytes.
// Like for sending three bytes, the word will be 0x02CCBBAA.
// For sending three bytes, the MSB should be 2.
// For two bytes, the MSB should be 1.
// For one bytes, the MSB should be 0.
//
void DTrkDccChannel::ReadDccChannel()
{
	TInt err = KErrNone;
	TInt err1 = KErrNone;
	
	TUint i = iRxPutIndx;
	TUint32 data;
	
	err = err1 = Arm::DebugInJTAG(data);	
	while (KErrNone == err)
	{	
		if (KErrNone == err1)
		{	
			TUint8 nBytes = 0;
			nBytes = TUint8(data>>24);
			
			for (int j=0; j<=nBytes; j++)
			{									
				iRxBuffer[i] = (TUint8)(data>>(j*8));	
				if (iRxBuffer[i] == PKT_STRT_BYTE)	
				{
					if (!iPacketStarted)
					{
						iPacketStarted = ETrue;
					}
					else
					{
						iPacketEnded = ETrue;
					}
				}
				i++; //increment i here so that put index can be appriopriately set later 
			}
			
			if (i == iRxBufSize)
			{
				i = 0;
				break;
			}
			//if cmd packet ended, send the data to the engine.
			if (iPacketEnded)
			{
				break;
			}
		}

		//read again to see if there is data already available
		err = err1 = Arm::DebugInJTAG(data);	
		
		//if the packet is started, try to read the whole packet
		//so set the err value to KErrNone so that we can continue reading
		if ((err == KErrNotReady) && iPacketStarted && !iPacketEnded)
			err = KErrNone; 				
			
		if (err != KErrNone && err != KErrNotReady)
			LOG_MSG2("Read Error %d\n", err);
	}

	iRxPutIndx=i;	
}

//
// DTrkDccChannel::RxTimerCallBack
//
void DTrkDccChannel::RxTimerCallBack(TAny* aPtr)
{
	// called from ISR when timer completes
	DTrkDccChannel *pChannel = (DTrkDccChannel*)aPtr;
	pChannel->iRxCompleteDfc.Add();
}

//
// DTrkDccChannel::RxTimerDfc
//
void DTrkDccChannel::RxTimerDfc(TAny* aPtr)
{
	DTrkDccChannel *pChannel = (DTrkDccChannel*)aPtr;	
	pChannel->DoRead();
}

//
// DTrkDccChannel::DoCompleteRx
//
void DTrkDccChannel::DoCompleteRx()
{
	LOG_MSG("DTrkDccChannel::DoCompleteRx()");

	if (iRxRequestStatus)
	{
		TInt avail = 0;
		if (iRxPutIndx > iRxGetIndx) 
			avail = iRxPutIndx-iRxGetIndx;
		else
			avail = iRxBufSize-iRxGetIndx;
			
		TInt len = Min(avail, iRxClientBufferLength);
		
		TPtrC8 des(iRxBuffer+iRxGetIndx, len);
		
		TInt err = Kern::ThreadDesWrite(iClientThread, iRxClientBuffer, des, 0, KChunkShiftBy0, iClientThread);

		Kern::RequestComplete(iClientThread, iRxRequestStatus, err);
		
		iRxRequestStatus = NULL;
		iRxGetIndx += len;
	}
			
	if (iRxGetIndx >= iRxBufSize)
		iRxGetIndx = 0;
}


DECLARE_STANDARD_LDD()
{
    return new DTrkDccDriverFactory;
}
