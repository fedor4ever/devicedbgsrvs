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


#include "toolsconnectionlistener.h"
#include <e32debug.h>

//
//----------------------------------------------------
// CToolsConnectionListener::NewL()
// To create the instance
//----------------------------------------------------
//
CToolsConnectionListener* CToolsConnectionListener :: NewL(MTrkUsbConnectionListener* aListener)
{
    CToolsConnectionListener* self = new (ELeave) CToolsConnectionListener();
    CleanupStack::PushL(self);
    self->ConstructL(aListener);
    CleanupStack::Pop(self);
    return self;   
}
//
//----------------------------------------------------
// CToolsConnectionListener::ConstructL()
// 2nd phase constructor
//----------------------------------------------------
//
void CToolsConnectionListener::ConstructL(MTrkUsbConnectionListener* aListener)
{
    User::LeaveIfError( iToolSession.Connect() );
    iConnectionListener = aListener;
}
//
//----------------------------------------------------
// CToolsConnectionListener::CToolsConnectionListener()
// 1st phase constructor
//----------------------------------------------------
//
CToolsConnectionListener::CToolsConnectionListener()
    : CActive(EPriorityStandard),
    iPackage(iConnStatus),
    iConnectionListener(NULL)
   
{   
    CActiveScheduler::Add(this);
}
//
//----------------------------------------------------
// CToolsConnectionListener::~CToolsConnectionListener()
// Desctructor
//----------------------------------------------------
//
CToolsConnectionListener::~CToolsConnectionListener() 
{   
    Cancel();
    iToolSession.Close();
}
//
//----------------------------------------------------
// CToolsConnectionListener::StartListening()
// To issue the asynchronous request
//----------------------------------------------------
//

void CToolsConnectionListener::StartListening()
{
    iToolSession.UsbConnNotify(iPackage, iStatus);    
    SetActive();
}
//
//----------------------------------------------------
// CToolsConnectionListener::StartListening()
// To stop the asynchronous request
//----------------------------------------------------
//
void CToolsConnectionListener::StopListening()
{
    DoCancel();  
    
}
//
//----------------------------------------------------
// CToolsConnectionListener::RunL()
// RunL implementation
//----------------------------------------------------
//
void CToolsConnectionListener::RunL()
{
    if(iStatus==KErrNone)
    {
        switch(iConnStatus)
        {   
            case EUsbConnected:
            {  
                if(iConnectionListener)
                    iConnectionListener->ConnectionAvailable();
                break;
            }
            case EUsbNotConnected:
            {
                if(iConnectionListener)
                    iConnectionListener->ConnectionUnAvailable();
                break;
            }
                
            default:
                break;
            
         }
     }
    
    StartListening();
       
}

//
//----------------------------------------------------
// CToolsConnectionListener::DoCancel()
// 
//----------------------------------------------------
//

void CToolsConnectionListener::DoCancel()
{
    iToolSession.UsbConnNotifyCancel();
}

