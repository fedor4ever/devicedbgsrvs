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
#include "toolsusbportlistener.h" 
                   
// LOCAL FUNCTION PROTOTYPES

// LOCAL CONSTANTS

const TUint KUsbDeviceStateMask				= 0x00ff;
#define KPersonalityPCSuite               113
#define KPersonalityPCSuiteMTP            5

//
// CToolsUsbPortListener::CToolsUsbPortListener
// Constructor
//
CToolsUsbPortListener::CToolsUsbPortListener (MToolsConnectionListener *aConnectionListener) 
		  	  :CToolsCommPortListener(aConnectionListener, CActive::EPriorityStandard)
{
	iUsbDeviceState = EUsbDeviceStateUndefined;
	iConnected = EFalse;
	iPersonalityId = 0;
    
    // For platsim, getting the USB device state is not supported.
    // Infact it panics the server, so don't bother with the USB state for platsim.
#ifndef __PLATSIM__    
    TInt err = iUsbServer.Connect();
    
    err = iUsbServer.GetDeviceState( iUsbDeviceState );
    if (!err)
    	err = iUsbServer.GetCurrentPersonalityId( iPersonalityId );
    
    if (!err && iUsbDeviceState == EUsbDeviceStateConfigured
    		 && iPersonalityId == KPersonalityPCSuiteMTP)
    		 iConnected = ETrue;

    CActiveScheduler::Add( this ); 
#endif

}

//
// CToolsUsbPortListener::~CToolsUsbPortListener
// Destructor
//
CToolsUsbPortListener::~CToolsUsbPortListener()
{
#ifndef __PLATSIM__
    Cancel();   // Ensure that any outstanding requests are cancelled
    Deque();
    iUsbServer.Close();
#endif    
}

//
// CToolsUsbPortListener::ListenL
// Starts listening for usb connection status notifications..
//
void CToolsUsbPortListener::ListenL()
{		
	IssueRequestL();		
}

//
// CToolsUsbPortListener::IsConnectionAvailable
//
TBool CToolsUsbPortListener::IsConnectionAvailable()
{		
#ifndef __PLATSIM__		
	TInt err = iUsbServer.GetDeviceState( iUsbDeviceState );
    if (!err)
    	err = iUsbServer.GetCurrentPersonalityId( iPersonalityId );
    	
    if (!err && iUsbDeviceState == EUsbDeviceStateConfigured
    		 && iPersonalityId == KPersonalityPCSuiteMTP)
    {
		iConnected = ETrue;     
		return ETrue;
    }
#endif      
    return EFalse;
}

//
// CToolsUsbPortListener::IssueRequestL
//
void CToolsUsbPortListener::IssueRequestL()
{		
#ifndef __PLATSIM__		
	if ( iUsbDeviceState == EUsbDeviceStateUndefined )
	{
		User::LeaveIfError( iUsbServer.GetDeviceState( iUsbDeviceState ) );		
	}
	
	iUsbServer.DeviceStateNotification(KUsbDeviceStateMask, iUsbDeviceState, iStatus);
	SetActive();	
#endif		
}

//
// CToolsUsbPortListener::StopListening
// Cancel any pending requests for connection status notifications.
//
void CToolsUsbPortListener::StopListening()
{
	Cancel();	
}

//
// CToolsUsbPortListener::DoCancel
// This method gets called from CActive::Cancel();
//
void CToolsUsbPortListener::DoCancel()
{
#ifndef __PLATSIM__		
	iUsbServer.DeviceStateNotificationCancel();	
#endif	
	iUsbDeviceState = EUsbDeviceStateUndefined;
}

//
// CToolsUsbPortListener::RunL
// This method gets called whenever there is a change in the connection status.
// Right now we are interested only when the cable is disconnected. 
// For all the other notifications, just reissues the request.
//
void CToolsUsbPortListener::RunL()
{
#ifndef __PLATSIM__		
	User::LeaveIfError( iStatus.Int() );

	iUsbServer.GetCurrentPersonalityId( iPersonalityId );
	
    if ( iUsbDeviceState == EUsbDeviceStateUndefined ||
         (iUsbDeviceState == EUsbDeviceStateConfigured && iPersonalityId != KPersonalityPCSuiteMTP) ) 
	{
		iConnectionListener->ConnectionUnAvailable();
		iConnected = EFalse;
	}
	// There is something wrong with USB cable status notifications
	// When the cable is disconnected, first you get a notification 
	// with the device state as configured.
	// So we need to work around using a local status flag
	else if ( iUsbDeviceState == EUsbDeviceStateConfigured &&
			  iPersonalityId == KPersonalityPCSuiteMTP  && 
			  !iConnected) 
	{
		iConnected = ETrue;
		iConnectionListener->ConnectionAvailable();;
	}
		
	IssueRequestL();	
#endif		
}

TConnectionStatus CToolsUsbPortListener::GetUsbConnStatus()
{   
    if(iConnected)
        return EUsbConnected;
    
    return EUsbNotConnected;
}



