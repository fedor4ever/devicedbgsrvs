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


#include <e32base.h>
#include <e32cons.h>

#include "logging.h"
#include "ostprotregistry.h"

COstProtRegistry*	COstProtRegistry::NewL()
{
	LOG_MSG("COstProtRegistry::NewL");
	
	COstProtRegistry* self = new(ELeave) COstProtRegistry;
	return self;
}

COstProtRegistry::COstProtRegistry()
{
	LOG_MSG("COstProtRegistry::COstProtRegistry");

	iProtListenerList.Reset();
}

COstProtRegistry::~COstProtRegistry()
{
	LOG_MSG("COstProtRegistry::~COstProtRegistry");

	iProtListenerList.Close();
}

void COstProtRegistry::RegisterProtocol(const TOstProtIds aProtId, CDbgTrcSrvSession* aProtMsgListener, TBool aStripHeader)
{
	LOG_MSG("COstProtRegistry::RegisterProtocol");

	TProtRegistryEntry protEntry(aProtId, aProtMsgListener, aStripHeader);
	
	iProtListenerList.Append(protEntry);	
}


void COstProtRegistry::UnRegisterProtocol(const TOstProtIds aProtId)
{
	LOG_MSG("COstProtRegistry::UnRegisterProtocol");

	// remove this listener from our list
	for (TInt i=0; i<iProtListenerList.Count(); i++)
	{
		if (iProtListenerList[i].iProtId == aProtId)
		{
			iProtListenerList.Remove(i);			
			break;
		}
	}
}

TBool COstProtRegistry::IsHeaderNeedForProtId(TOstProtIds aProtId)
{
	LOG_MSG("COstProtRegistry::IsHeaderNeedForProtId");

	for (TInt i=0; i<iProtListenerList.Count(); i++)
	{
		if (iProtListenerList[i].iProtId == aProtId)
		{			
			return iProtListenerList[i].iNeedHeader;
		}
	}
	return EFalse;
}

CDbgTrcSrvSession* COstProtRegistry::GetProtListenerForProtId(const TOstProtIds aProtId, TBool& aNeedHeader)
{
	LOG_MSG("COstProtRegistry::GetProtListenerForProtId");

	// remove this listener from our list
	for (TInt i=0; i<iProtListenerList.Count(); i++)
	{
		if (iProtListenerList[i].iProtId == aProtId)
		{
			aNeedHeader = iProtListenerList[i].iNeedHeader;
			return iProtListenerList[i].iProtListener;
		}
	}
	return NULL;
}

TInt COstProtRegistry::GetProtListenerCount()
{
	LOG_MSG("COstProtRegistry::GetProtListenerCount");

	return iProtListenerList.Count();
}
