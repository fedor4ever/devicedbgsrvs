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

#ifndef TRKCONNSTATELISTENER_H_
#define TRKCONNSTATELISTENER_H_

//  INCLUDES
#include <e32std.h>

#include "trksrvclisession.h"
#include "trkservereventlistener.h"


// DATA TYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

class CTrkConnStateListener: public CTrkServerEventListener
{
    public:  

        CTrkConnStateListener(RTrkSrvCliSession& aTrkSrvSession, MTrkServerEventCallback *aTrkServerEventCallback);    
        ~CTrkConnStateListener();

        // From CTrkCommPortListener
        void ListenL();
        void StopListening();
        
        TBool IsConnected();
        
    protected: 
        // From CActive
        void DoCancel();
        void RunL();        

    private:
    
        void IssueRequestL();

    private:        
        TTrkConnStatus iConnStatus;
        TBuf<KMaxPath> iConnMsg;
        TConnState iConnState;        
};

#endif /* TRKCONNSTATELISTENER_H_ */
