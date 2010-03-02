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



COstMessage* COstMessage::NewL(TDesC8& aMsg)
{
	LOG_MSG("COstMessage::NewL");

	COstMessage* self = new(ELeave) COstMessage();
	CleanupStack::PushL(self);
	self->ConstructL(aMsg);
	CleanupStack::Pop(self);
	return self;
}

COstMessage::COstMessage()
: iMsgBuffer(NULL)
{
	LOG_MSG("COstMessage::COstMessage");


}

COstMessage::~COstMessage()
{
	LOG_MSG("COstMessage::~COstMessage");

	SafeDelete(iMsgBuffer);
}

void COstMessage::ConstructL(TDesC8& aMsg)
{
	LOG_MSG("COstMessage::ConstructL");

	iMsgBuffer = aMsg.AllocL();		
}
