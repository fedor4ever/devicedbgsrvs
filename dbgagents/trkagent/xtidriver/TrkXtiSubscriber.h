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


#ifndef __TRKXTISUBSCRIBER_H__
#define __TRKXTISUBSCRIBER_H__

#include <TraceCoreSubscriber.h>
#include <TraceCoreMessageSender.h>

class DTrkXtiChannel;

class DTrkXtiSubscriber : public DTraceCoreSubscriber
{
public:
	DTrkXtiSubscriber();
	~DTrkXtiSubscriber();
	
	TInt Create(DTrkXtiChannel* aChannel, DThread* aClient);

	TInt Send(TDesC8& aMessage);
    void MessageReceived(TTraceMessage& aMessage);
    
    void StoreMessage(const TTraceMessage& aMessage);
    
private:
    
    TTraceMessage iTraceMessage;
	TBool		  iTraceMessageInitialized;
    
   	DThread* iClientThread;
	DTrkXtiChannel* iChannel;
	HBuf8* iHeaderBuf;
};

#endif //__TRKXTISUBSCRIBER_H__
