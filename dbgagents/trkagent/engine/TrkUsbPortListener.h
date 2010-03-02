/*
* Copyright (c) 2004-2008 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef     _TRK_USBPORTLISTENER_H_
#define     _TRK_USBPORTLISTENER_H_

//  INCLUDES
#include <e32std.h>
#include <c32comm.h>
#include <usbman.h>

#include "TrkCommPortListener.h"
#include "TrkEngine.h"


// DATA TYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION


class CTrkUsbPortListener: public CTrkCommPortListener
{

    public:  

		CTrkUsbPortListener(MTrkUsbConnectionListener *aConnectionListener);	
        ~CTrkUsbPortListener();

		// From CTrkCommPortListener
        void ListenL();
        void StopListening();

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


#endif      //  _TRK_USBPORTLISTENER_H_


//  End of File
