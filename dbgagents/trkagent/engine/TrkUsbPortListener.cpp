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

#include "TrkUsbPortListener.h" 
                   
// LOCAL FUNCTION PROTOTYPES

// LOCAL CONSTANTS

const TUint KTrkUsbDeviceStateMask				= 0x00ff;

#if(defined __SERIES60_30__ || defined __SERIES60_31__ || defined __S60_32__)
#define KPersonalityPCSuite               113
#else
#define KPersonalityPCSuite               5
#endif




//
// CTrkUsbPortListener::CTrkUsbPortListener
// Constructor
//
CTrkUsbPortListener::CTrkUsbPortListener (MTrkUsbConnectionListener *aConnectionListener) 
		  	  :CTrkCommPortListener(aConnectionListener, CActive::EPriorityStandard)
{
	iConnected = EFalse;
	iPersonalityId = 0;

	iUsbDeviceState = EUsbDeviceStateUndefined;
    
    TInt err = iUsbServer.Connect();

    err = iUsbServer.GetDeviceState( iUsbDeviceState );
    if (!err)
    	err = iUsbServer.GetCurrentPersonalityId( iPersonalityId );
    
    if (!err && iUsbDeviceState == EUsbDeviceStateConfigured
    		 && iPersonalityId == KPersonalityPCSuite)
    		 iConnected = ETrue;

    CActiveScheduler::Add( this ); 
}

//
// CTrkUsbPortListener::~CTrkUsbPortListener
// Destructor
//
CTrkUsbPortListener::~CTrkUsbPortListener()
{
    Cancel();   // Ensure that any outstanding requests are cancelled
    Deque();
    iUsbServer.Close();
}

//
// CTrkUsbPortListener::ListenL
// Starts listening for usb connection status notifications..
//
void CTrkUsbPortListener::ListenL()
{		
	IssueRequestL();		
}

//
// CTrkUsbPortListener::IssueRequestL
//
void CTrkUsbPortListener::IssueRequestL()
{		
	if (IsActive()) //already listening, just return...
		return;
		
	if ( iUsbDeviceState == EUsbDeviceStateUndefined )
	{
		User::LeaveIfError( iUsbServer.GetDeviceState( iUsbDeviceState ) );
		
	}
	
	iUsbServer.DeviceStateNotification(KTrkUsbDeviceStateMask, iUsbDeviceState, iStatus);
	SetActive();	
}

//
// CTrkUsbPortListener::StopListening
// Cancel any pending requests for connection status notifications.
//
void CTrkUsbPortListener::StopListening()
{
	Cancel();	
}

//
// CTrkUsbPortListener::DoCancel
// This method gets called from CActive::Cancel();
//
void CTrkUsbPortListener::DoCancel()
{
	iUsbServer.DeviceStateNotificationCancel();	
	iUsbDeviceState = EUsbDeviceStateUndefined;
}

//
// CTrkUsbPortListener::RunL
// This method gets called whenever there is a change in the connection status.
// Right now we are interested only when the cable is disconnected. 
// For all the other notifications, just reissues the request.
//
void CTrkUsbPortListener::RunL()
{
	User::LeaveIfError( iStatus.Int() );

	iUsbServer.GetCurrentPersonalityId( iPersonalityId );
	
  	if ( iUsbDeviceState == EUsbDeviceStateUndefined ||
         (iUsbDeviceState == EUsbDeviceStateConfigured && iPersonalityId != KPersonalityPCSuite) ) 
	{
		iConnectionListener->ConnectionUnAvailable();
		iConnected = EFalse;
	}
	// There is something wrong USB cable status notifications
	// When the cable is disconnected, first you get a notification 
	// with the device state as configured.
	// So we need to work around using a local status flag
	else if ( iUsbDeviceState == EUsbDeviceStateConfigured &&
			  iPersonalityId == KPersonalityPCSuite  && 
			  !iConnected) 
	{
		iConnected = ETrue;
		iConnectionListener->ConnectionAvailable();;
	}		
		IssueRequestL();
}

