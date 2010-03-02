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


#ifndef __TRKDCCDRIVER_H__
#define __TRKDCCDRIVER_H__

// Debug messages
#ifdef _DEBUG
#define LOG_MSG(x) Kern::Printf(x)
#define LOG_MSG2(x, y) Kern::Printf(x, y)
#else
#define LOG_MSG(x)
#define LOG_MSG2(x, y)
#endif


//
// Macros
//
#define MAXMESSAGESIZE	0x0880

//
// class DTrkDccDriverFactory
//
class DTrkDccDriverFactory : public DLogicalDevice
{
public:

	DTrkDccDriverFactory();
	virtual TInt Install();
	virtual void GetCaps(TDes8& aDes) const;
	virtual TInt Create(DLogicalChannelBase*& aChannel);
};

//
// DTrkDccChannel
//
class DTrkDccChannel : public DLogicalChannel
{
public:

	DTrkDccChannel(DLogicalDevice* aLogicalDevice);
	~DTrkDccChannel();

	virtual TInt DoCreate(TInt aUnit, const TDesC* anInfo, const TVersion& aVer);	
	virtual void HandleMsg(TMessageBase* aMsg);
	
	
protected:
	virtual void DoCancel(TInt aReqNo);
	virtual void DoRequest(TInt aReqNo, TRequestStatus* aStatus, TAny* a1, TAny* a2);
	virtual TInt DoControl(TInt aFunction, TAny *a1, TAny *a2);
	
	
private:
	TInt DoWrite(TUint32 aLength, TAny* a2);
	TInt DoRead();

	TInt WriteDccChannel(const TDesC8& aDes);
	void ReadDccChannel();

	void DoCompleteRx();
	
	static void RxTimerDfc(TAny* aPtr);
	static void RxTimerCallBack(TAny* aPtr);
		
private:
	DThread* iClientThread;
	
	TRequestStatus* iRequestGetEventStatus;

	NTimer iRxTimer;
	
	TBuf8<MAXMESSAGESIZE> iTxBuffer;
	//TBuf8<MAXMESSAGESIZE*2> iRxBuffer;
	TUint8 *iRxBuffer;
	TInt iRxBufSize;
	
	TRequestStatus* iRxRequestStatus;
	TAny* iRxClientBuffer;
	TUint32 iRxClientBufferLength;
	TInt iRxGetIndx, iRxPutIndx;
	
	TDfc iRxCompleteDfc;

	TInt iInterruptId;
	TInt iRxTimeOut;
	TUint32 iPageSize;
	
	TBool iPacketStarted;
	TBool iPacketEnded;
	
	/** Lock serialising calls to event handler */
	DSemaphore* iLock;
	
	TDfcQue* iDFCQue;

};

#endif //__TRKDCCDRIVER_H__
