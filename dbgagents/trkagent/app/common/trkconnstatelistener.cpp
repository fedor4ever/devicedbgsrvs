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
#include "trkconnstatelistener.h" 
                   
// LOCAL FUNCTION PROTOTYPES

// LOCAL CONSTANTS

//
// CTrkConnStateListener::CTrkConnStateListener
// Constructor
//
CTrkConnStateListener::CTrkConnStateListener (RTrkSrvCliSession& aTrkSrvSession, MTrkServerEventCallback *aTrkServerEventCallback) 
              :CTrkServerEventListener(aTrkSrvSession, aTrkServerEventCallback, CActive::EPriorityStandard), iConnMsg(KNullDesC), iConnState(iConnStatus)
{
    iConnStatus = ETrkNotConnected;
    iTrkSrvSession.GetDebugConnStatus(iConnStatus, iConnMsg);
    CActiveScheduler::Add( this ); 
}

//
// CTrkConnStateListener::~CTrkConnStateListener
// Destructor
//
CTrkConnStateListener::~CTrkConnStateListener()
{
    Cancel();   // Ensure that any outstanding requests are cancelled
    Deque();
}

//
// CTrkConnStateListener::ListenL
// Starts listening for usb connection status notifications..
//
void CTrkConnStateListener::ListenL()
{       
    IssueRequestL();        
}



//
// CTrkConnStateListener::StopListening
// Cancel any pending requests for connection status notifications.
//
void CTrkConnStateListener::StopListening()
{
    Cancel();   
}

//
// CTrkConnStateListener::IssueRequestL
//
TBool CTrkConnStateListener::IsConnected()
{       
   iTrkSrvSession.GetDebugConnStatus(iConnStatus, iConnMsg);
   return (iConnStatus==ETrkConnected);
}

//
// CTrkConnStateListener::IssueRequestL
//
void CTrkConnStateListener::IssueRequestL()
{       
    if (IsActive()) //already listening, just return...
        return;
    iTrkSrvSession.DebugConnStatusNotify(iConnState, iConnMsg, iStatus);
    
    SetActive();    
}

//
// CTrkConnStateListener::DoCancel
// This method gets called from CActive::Cancel();
//
void CTrkConnStateListener::DoCancel()
{
    iTrkSrvSession.DebugConnStatusNotifyCancel();   
}

//
// CTrkConnStateListener::RunL
// This method gets called whenever there is a change in the connection status.
//
void CTrkConnStateListener::RunL()
{
    User::LeaveIfError( iStatus.Int() );
    if (iTrkServerEventCallback)
        iTrkServerEventCallback->ConnectionStateChanged(iConnStatus);
    
    IssueRequestL();
}
