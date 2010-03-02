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

#ifndef __OstBaseRouter_h__
#define __OstBaseRouter_h__

// System includes
#include <e32base.h>

#include "datalistener.h"

class CDbgTrcPortMgr;
class CDbgTrcSrvSession;
class COstProtRegistry;
class COstMessage;

class COstWriteMsgEntry : public CBase
{
	public:
		static 	COstWriteMsgEntry*	NewL(TDesC8& aMsg, CDbgTrcSrvSession* aMsgListener);
				~COstWriteMsgEntry();
	
	//private construct
	private:
				COstWriteMsgEntry(CDbgTrcSrvSession* aMsgListener);
		void	ConstructL(TDesC8& aMsg);

	public:	
		COstMessage* iMessage;
		CDbgTrcSrvSession* iWriteMsgListener;		
};

class COstBaseRouter : public CBase, MDataListener
{
	public:
		static 	COstBaseRouter*	NewL();
				~COstBaseRouter();
	
	//private construct
	private:
				COstBaseRouter();
		void	ConstructL();		

	public:
	
		void GetPortConfig(TDes8& aConfigDes);
		TInt SetPortConfig(TDesC8& aConfigDes);

		TInt OpenCommPortL();
		TInt CloseCommPort();
		
		void RegisterProtocol(const TOstProtIds aProtId, CDbgTrcSrvSession* aProtMsgListener, TBool aStripHeader);
		void UnRegisterProtocol(const TOstProtIds aProtId);
		
		void ReadMessage();
		void WriteMessageL(TDesC8& aMsg, CDbgTrcSrvSession* aProtMsgListener);
		void WriteMessageL(TDesC8& aMsg, CDbgTrcSrvSession* aProtMsgListener, TOstProtIds aProtId);

		
	public: // from MDataListener
		void DataAvailable(TDesC8& aMsgData, TUint aLength);
		void DataWriteComplete(TInt aErrCode);
		
	private:
		void SwapBytes(TUint8* aTrgtData, const TUint8* aSrcData, TInt aLength);
		void DoWriteMessageL(CDbgTrcSrvSession* aProtMsgListener);
		void RemoveNonOstBytes(TDes8& aMsgBuffer);


	// data members		
	private:
	
		CDbgTrcPortMgr* iPortManager;
		COstProtRegistry* iProtRegistry;
		
		CDbgTrcSrvSession* iWriteMsgListener;
		
		TBuf8<MAX_BUF_SIZE> iRecvMsgBuffer;
		TBuf8<MAX_BUF_SIZE> iSendMsgBuffer;
		
		RPointerArray<COstWriteMsgEntry> iWriteMsgQueue;
				
		TBool iIsBigEndian;		
};

#endif //__OstBaseRouter_h__   

