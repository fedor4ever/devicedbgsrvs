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

#ifndef __OstProtRegistry_h__
#define __OstProtRegistry_h__

// System includes
#include <e32base.h>
#include "ostprotdefs.h"

class CDbgTrcSrvSession;

//
// class TProtRegistryEntry
//
class TProtRegistryEntry
{
public:

	inline TProtRegistryEntry(TOstProtIds aProtId, CDbgTrcSrvSession* aProtListener, TBool aNeedHeader)
				: iProtId(aProtId),
				  iProtListener(aProtListener),
				  iNeedHeader(aNeedHeader) { }
public:

	TOstProtIds iProtId;
	CDbgTrcSrvSession* iProtListener;
	TBool iNeedHeader;	
};

class COstProtRegistry : public CBase
{
	public:
		static 	COstProtRegistry*	NewL();
				~COstProtRegistry();
	
	//private construct
	private:
				COstProtRegistry();

	public:
	
		void RegisterProtocol(const TOstProtIds aProtId, CDbgTrcSrvSession* aProtMsgListener, TBool aStripHeader);
		void UnRegisterProtocol(const TOstProtIds aProtId);
		
		TBool IsHeaderNeedForProtId(TOstProtIds aProtId);
		CDbgTrcSrvSession* GetProtListenerForProtId(const TOstProtIds aProtId, TBool& aNeedHeader);
		
		TInt GetProtListenerCount();

	// data members		
	private:
		RArray<TProtRegistryEntry> iProtListenerList;
};

#endif //__OstProtRegistry_h__
