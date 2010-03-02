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

#include "toolssrvsession.h"
#include "toolssrvserver.h"
// System includes

//User Includes
#include "ToolsCmdCodes.h"

// Type definitions

// Constants
const TInt KToolsServerTransferBufferExpandSize = 100;
const TInt KSlot0 = 0;

// Enumerations

// Classes referenced


// Static functions
//
// Checks for a null'd handle before attempting complete. 
// If handle is null'd then don't complete as this will panic server.
//
void SafeComplete(const RMessagePtr2& aMessage, TInt aCompletionCode)
{
    if(!aMessage.IsNull())
    {
        aMessage.Complete(aCompletionCode);
    }
}



//
// CToolsSrvSession (source)
//

//
// CToolsSrvServer::CToolsSrvSession()
//
// Constructor
//
CToolsSrvSession::CToolsSrvSession()
:   iPendingRead(EFalse),
    iLaunchManager(NULL)
{
}

//
// CToolsSrvServer::~CToolsSrvSession()
//
// Destructor
//
CToolsSrvSession::~CToolsSrvSession()
{
	HandleServerDestruction();
	delete iTransferBuffer;
	//get a reference to the server
	CToolsSrvServer* toolsserver = (CToolsSrvServer*)Server();
	
	//notify the server that the session has been opened
	if (toolsserver != NULL)
	    toolsserver->SessionClosed();
	if(iLaunchManager)
	    iLaunchManager->UnregisterListener(this);
}

//
// CToolsSrvServer::ConstructL()
//
// Creates an instance of CToolsSrvSession.
//
void CToolsSrvSession::ConstructL()
{
    iTransferBuffer = CBufFlat::NewL(KToolsServerTransferBufferExpandSize);
}

//
//  CToolsSrvSession::CreateL()
//
void CToolsSrvSession::CreateL()
{
    //get a reference to the server
    CToolsSrvServer* toolsserver = (CToolsSrvServer*)Server();
    
    //notify the server that the session has been opened
    if (toolsserver != NULL)
    {
        toolsserver->SessionOpened();
        iLaunchManager = toolsserver->GetLaunchInterface();
    }
  
}
//
// CToolsSrvServer::NewL()
//
// Static self construction
//
CToolsSrvSession* CToolsSrvSession::NewL()
{
	CToolsSrvSession* self = new(ELeave) CToolsSrvSession();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}

//
// CToolsSrvServer::HandleServerDestruction()
//
// Called by the server's destructor. We need to be told that the server is
// being destroyed.
//
void CToolsSrvSession::HandleServerDestruction()
{
}

//
// CToolsSrvServer::ServiceL()
//
// Services requests from a client.
// Called by the IPC framework whenever a client sends a request to the server.
//
void CToolsSrvSession::ServiceL(const RMessage2& aMessage)
{

    const TInt cmd = aMessage.Function();

	switch(cmd)
	{
		case EToolsCmdCodeGetUsbConnStatus:
			 CmdGetUsbConnStatus(aMessage);
			break;
		case EToolsCmdCodeConnNotify:
			CmdConnNofify(aMessage);
			break;
		case EToolCmdCodeConnNotifyCancel:
			CmdConnNotifyCancel(aMessage);
			break;		
		case EToolsCmdCodeShutDownServer:
			CmdShutDownServer();			
			break;
	
		default:
			aMessage.Panic(KServerIntiatedSessionPanic, EToolsServerInitiatedClientPanicInvalidOperation);
			break;
	}
		
}

//
//  CToolsSrvSession::CmdGetUsbConnStatus      
//
//  Command implementation to find out the usb connection status
//
void CToolsSrvSession::CmdGetUsbConnStatus(const RMessage2& aMessage)
{
    TConnectionStatus status = iLaunchManager->GetUsbConnStatus();
    TPckg<TConnectionStatus> package(status);
    TRAPD(err,aMessage.Write(KSlot0,package)); 
    if (err != KErrNone)
    {
        aMessage.Panic(_L("ToolsServer"),2);
    }
    SafeComplete(aMessage,KErrNone);
}
//
//  CToolsSrvSession::CmdConnNofify      
//
//  To register for usb cable connection notifications
//
void CToolsSrvSession::CmdConnNofify(const RMessage2& aMessage)
{   
    if (iPendingRead)
    {   
        SafeComplete(aMessage, KErrAlreadyExists);
    }
    else
    {
        iLaunchManager->RegisterListener(this);
        iBlockedRead = aMessage;
        iPendingRead = ETrue;     
        
    }   
}
//
//  CToolsSrvSession::CmdConnNotifyCancel      
//
//  To unregister for usb cable connection notifications
//
void CToolsSrvSession::CmdConnNotifyCancel(const RMessage2& aMessage)
{
    if ( iPendingRead && iBlockedRead.Handle())
    {
        SafeComplete(iBlockedRead, KErrCancel);
        iPendingRead = EFalse;
        iLaunchManager->UnregisterListener(this);
    }
    SafeComplete(aMessage, KErrNone);
}

//
//  CToolsSrvSession::ConnStatusChanged
//
//  Called when the usb cable connection changes
//
void CToolsSrvSession::ConnStatusChanged(TConnectionStatus aStatus)
{
    if(iPendingRead && iBlockedRead.Handle())
    {
        iPendingRead = EFalse;
        TConnectionStatus status = aStatus;
        TPckg<TConnectionStatus> package(status);
        TRAPD(err, iBlockedRead.WriteL(KSlot0, package));  
        if ( err != KErrNone )
        {
            iBlockedRead.Panic(_L("ToolsServer"),2);
        }
        SafeComplete(iBlockedRead,KErrNone);
    }
       
}
//
// CToolsSrvServer::CmdShutDownServer()
//
// Stops the active scheduler. This way, server process will run to completion.
//
void CToolsSrvSession::CmdShutDownServer()
{
	CActiveScheduler::Stop();
}
