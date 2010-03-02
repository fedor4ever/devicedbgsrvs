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
#include "dbgtrccomm.h"
#include "ostmessage.h"


COstWriteMsgEntry* COstWriteMsgEntry::NewL(TDesC8& aMsg, CDbgTrcSrvSession* aMsgListener)
{
	LOG_MSG("COstWriteMsgEntry::NewL");

	COstWriteMsgEntry* self = new(ELeave) COstWriteMsgEntry();
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
}

void COstWriteMsgEntry::ConstructL(TDesC8& aMsg)
{
	LOG_MSG("COstWriteMsgEntry::ConstructL");

	iMessage = new COstMessage(aMsg);
}
