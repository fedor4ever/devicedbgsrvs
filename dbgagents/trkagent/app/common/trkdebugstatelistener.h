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


#ifndef     _TRKDEBUGSTATELISTENER_H_
#define     _TRKDEBUGSTATELISTENER_H_

//  INCLUDES
#include <e32std.h>
#include "trksrvclisession.h"
#include "trkservereventlistener.h"


// DATA TYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

class CTrkDebugStateListener: public CTrkServerEventListener
{
    public:  

		CTrkDebugStateListener(RTrkSrvCliSession& aTrkSrvSession, MTrkServerEventCallback *aTrkServerEventCallback);	
        ~CTrkDebugStateListener();

		// From CTrkCommPortListener
        void ListenL();
        void StopListening();
        
        TBool IsDebugging();
        
    protected: 
    	// From CActive
        void DoCancel();
        void RunL();	    

	private:
	
		void IssueRequestL();

    private:		
		TBool iDebugging;
		TDebugState iDebugState;		
};


#endif      //  _TRKDEBUGSTATELISTENER_H_


//  End of File
