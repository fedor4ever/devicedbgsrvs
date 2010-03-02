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


#ifndef     _TOOLSUSBPORTLISTENER_H_
#define     _TOOLSUSBPORTLISTENER_H_

//  INCLUDES
#include <e32std.h>
#include <c32comm.h>
#include <usbman.h>

#include "toolscommportListener.h"
#include "connectionvariables.h"



// DATA TYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION


class CToolsUsbPortListener: public CToolsCommPortListener
{

    public:  

		CToolsUsbPortListener(MToolsConnectionListener *aConnectionListener);	
        ~CToolsUsbPortListener();

		// From CCommPortListener
        void ListenL();
        void StopListening();
        
        TBool IsConnectionAvailable();
        TConnectionStatus GetUsbConnStatus();

    protected: 
    	// From CActive
        void DoCancel();
        void RunL();	    

	private:
	
		void IssueRequestL();

		
    private:    

        RUsb                        iUsbServer;
		TUsbDeviceState				iUsbDeviceState;
		
		TBool iConnected;
		TInt iPersonalityId;
};


#endif      //  _TOOLSUSBPORTLISTENER_H_


//  End of File
