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

// INCLUDE FILES
#include <e32std.h>                      

#include "trksrvclisession.h"
#include "trkdebugstatelistener.h" 
                   
// LOCAL FUNCTION PROTOTYPES

// LOCAL CONSTANTS

//
// CTrkDebugStateListener::CTrkDebugStateListener
// Constructor
//
CTrkDebugStateListener::CTrkDebugStateListener (RTrkSrvCliSession& aTrkSrvSession, MTrkServerEventCallback *aTrkServerEventCallback) 
		  	  :CTrkServerEventListener(aTrkSrvSession, aTrkServerEventCallback, CActive::EPriorityStandard), iDebugState(iDebugging)
{
	iDebugging = EFalse;
	iTrkSrvSession.GetDebuggingStatus(iDebugging);
    CActiveScheduler::Add( this ); 
}

//
// CTrkDebugStateListener::~CTrkDebugStateListener
// Destructor
//
CTrkDebugStateListener::~CTrkDebugStateListener()
{
    Cancel();   // Ensure that any outstanding requests are cancelled
    Deque();
}

//
// CTrkDebugStateListener::ListenL
// Starts listening for usb connection status notifications..
//
void CTrkDebugStateListener::ListenL()
{		
	IssueRequestL();		
}



//
// CTrkDebugStateListener::StopListening
// Cancel any pending requests for connection status notifications.
//
void CTrkDebugStateListener::StopListening()
{
	Cancel();	
}

//
// CTrkDebugStateListener::IssueRequestL
//
TBool CTrkDebugStateListener::IsDebugging()
{       
   iTrkSrvSession.GetDebuggingStatus(iDebugging);
   return iDebugging;
}

//
// CTrkDebugStateListener::IssueRequestL
//
void CTrkDebugStateListener::IssueRequestL()
{       
    if (IsActive()) //already listening, just return...
        return;
    iTrkSrvSession.DebuggingStatusNotify(iDebugState, iStatus);
    
    SetActive();    
}

//
// CTrkDebugStateListener::DoCancel
// This method gets called from CActive::Cancel();
//
void CTrkDebugStateListener::DoCancel()
{
    iTrkSrvSession.DebuggingStatusNotifyCancel();	
}

//
// CTrkDebugStateListener::RunL
// This method gets called whenever there is a change in the debugging status.
//
void CTrkDebugStateListener::RunL()
{
	User::LeaveIfError( iStatus.Int() );
	if (iTrkServerEventCallback)
	    iTrkServerEventCallback->DebugStateChanged(iDebugging);
	
	IssueRequestL();
}

