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


#include <e32def.h>
#include <e32cmn.h>
#include <arm.h>
#include <kernel.h>
#include <kern_priv.h>
#include <nk_trace.h>

#include "TrkXtiComm.h"

#include "TrkXtiDriver.h"
#include "TrkXtiSubscriber.h"

/**
 * Constructor
 */
DTrkXtiSubscriber::DTrkXtiSubscriber() : 
					iTraceMessageInitialized(FALSE),
					iHeaderBuf(NULL)
{
	LOG_MSG("DTrkXtiSubscriber::DTrkXtiSubscriber()");
}

DTrkXtiSubscriber::~DTrkXtiSubscriber() 
{
	LOG_MSG("DTrkXtiSubscriber::~DTrkXtiSubscriber()");
	// Unsubscribe here
	Unsubscribe(kIntTrkMessageId);
	if (iHeaderBuf)
	{
		delete iHeaderBuf;
		iHeaderBuf = NULL;
	}
	
}

TInt DTrkXtiSubscriber::Create(DTrkXtiChannel* aChannel, DThread* aClient)
{
	LOG_MSG("DTrkXtiSubscriber::Create()");
	
	iChannel = aChannel; //Don't add ref the channel, since channel closes the event handler before it ever gets destroyed.

	TInt err = aClient->Open();
	
	if (err != KErrNone)
	{
		LOG_MSG("DTrkXtiSubscriber::Create() - Thread Opened, error");
		return err;
	}
		
	iClientThread = aClient;

	// subscribe to receive trk messages from TraceCore
	err = Subscribe( kIntTrkMessageId ); //  To do: replace with SOS_MON_TRK_SEND_MESSAGE_REQ 

	if (err != KErrNone)

		return err;
		
	return KErrNone;	

}

void DTrkXtiSubscriber::MessageReceived(TTraceMessage& aMessage)

{ 
	LOG_MSG("DTrkXtiSubscriber::MessageReceived()");

	if (!iTraceMessageInitialized)
	{ 

		iTraceMessage.iHeader = NULL;

		iTraceMessageInitialized = TRUE; 

	}
	
	StoreMessage (aMessage);
	// iTraceMessage.iMessageId = 0x45;  
	LOG_MSG2("iTraceMessage.iHeader->Length() = %d", iTraceMessage.iHeader->Length());
  	LOG_MSG2("aMessage.iHeader->Length() = %d", aMessage.iHeader->Length());
  	LOG_MSG2("aMessage.iData->Length() = %d", aMessage.iData->Length());

	iChannel->DoCompleteRx(*aMessage.iData);

}

TInt DTrkXtiSubscriber::Send(TDesC8& aMessage)
{
	LOG_MSG("DTrkXtiSubscriber::Send()");
	
	if (!iTraceMessageInitialized)
	{
		LOG_MSG("Trace message header not initialized");
		return KErrNotReady;
	}
		
	
	iTraceMessage.iData = &aMessage;
	// Replace this with SOS_MON_TRK_SEND_MESSAGE_RESP when get the new tracecore header
	iTraceMessage.iMessageId = 0x45;
	
	TInt err = KErrNone;
	
	err = iMessageSender->SendMessage(iTraceMessage);
	LOG_MSG2("iMessageSender->SendMessage, error = %d", err);
	LOG_MSG2("iTraceMessage.iHeader->Length() = %d", iTraceMessage.iHeader->Length());
	LOG_MSG2("iTraceMessage.iData->Length() = %d", iTraceMessage.iData->Length());
	
	return err;
}

/**
 * Stores the header from the incoming message
 */
void DTrkXtiSubscriber::StoreMessage( const TTraceMessage& aMessage )
{
    LOG_MSG("DTrkXtiSubscriber::StoreMessage()");
    if (iHeaderBuf == NULL)
    {
        iHeaderBuf = HBuf8::New(aMessage.iHeader->Length( ));
    }
    iHeaderBuf->Copy( *aMessage.iHeader );
    iTraceMessage = aMessage; // Copy message
    iTraceMessage.iHeader = iHeaderBuf; // Assign copy of header
}
