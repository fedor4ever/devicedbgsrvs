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

#ifndef TOOLSCONNECTIONLISTENER_H_
#define TOOLSCONNECTIONLISTENER_H_

#include <toolsclientsession.h>
#include "TrkUsbConnectionListener.h"


class CToolsConnectionListener:public CActive
{
    public:
        
        static CToolsConnectionListener* NewL(MTrkUsbConnectionListener* aListener);
        void   ConstructL(MTrkUsbConnectionListener* aListener );
        void   StartListening();
        void   StopListening();
        
        ~CToolsConnectionListener();
        
        
    private:
        
        CToolsConnectionListener();
        
    protected:
        
        void RunL();        
        void DoCancel();
        
    private:
        
        RToolsClientSession iToolSession;
        TConnectionStatus iConnStatus;
        TConnPkg iPackage;      
        MTrkUsbConnectionListener* iConnectionListener;
    
};





#endif /* USBCONNECTIONLISTENER_H_ */
