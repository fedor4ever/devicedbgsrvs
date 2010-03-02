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

#include "trksrvsession.h"

// System includes

//User Includes
#include "trkdebugmgrcallbacks.h"
#include "trksrvcmdcodes.h"

// Type definitions

// Constants
const TInt KTrkServerTransferBufferExpandSize = 100;
const TInt KSlot0 = 0;
const TInt KSlot2 = 2;


// Enumerations

// Classes referenced

// Static functions
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
// CTrkSrvSession (source)
//

//
// CTrkSrvSession::CTrkSrvSession()
//
// Constructor
//
CTrkSrvSession::CTrkSrvSession(CTrkDebugManager* aTrkDebugMgr) : iTrkDebugMgr(aTrkDebugMgr)
{
}

//
// CTrkSrvSession::~CTrkSrvSession()
//
// Destructor
//
CTrkSrvSession::~CTrkSrvSession()
{
	HandleServerDestruction();
	delete iTransferBuffer;
}

//
// CTrkSrvSession::ConstructL()
//
// Creates an instance of CTrkSrvSession.
//
void CTrkSrvSession::ConstructL()
{
	iTransferBuffer = CBufFlat::NewL(KTrkServerTransferBufferExpandSize);
	iTrkDebugMgr->SetDebugMgrCallback(this);
}

//
// CTrkSrvServer::NewL()
//
// Static self construction
//
CTrkSrvSession* CTrkSrvSession::NewL(CTrkDebugManager* aTrkDebugMgr)
{
	CTrkSrvSession* self = new(ELeave) CTrkSrvSession(aTrkDebugMgr);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}

//
// CTrkSrvSession::HandleServerDestruction()
//
// Called by the server's destructor. We need to be told that the server is
// being destroyed.
//
void CTrkSrvSession::HandleServerDestruction()
{

}

//
// CTrkSrvSession::ServiceL()
//
// Services requests from a client.
// Called by the IPC framework whenever a client sends a request to the server.
//
void CTrkSrvSession::ServiceL(const RMessage2& aMessage)
{
	const TInt cmd = aMessage.Function();
	
	switch (cmd)
	{
		case ETrkSrvGetVersion:
			CmdGetVersionL(aMessage);
			break;
		case ETrkSrvCmdConnect:
			CmdConnectL(aMessage);
			break;
		case ETrkSrvCmdDisConnect:
			CmdDisConnectL(aMessage);
			break;
		case ETrkSrvCmdGetCurrentConnData:
		    CmdGetCurrentConnDataL(aMessage);
		    break;
		case ETrkSrvCmdSetCurrentConnData:
		    CmdSetCurrentConnDataL(aMessage);
		    break;
		case ETrkSrvCmdGetDebugConnStatus:
			CmdGetDebugConnStatusL(aMessage);
			break;
		case ETrkSrvDebugConnStatus:
			ConnStatusNotifyL(aMessage);
			break;
		case ETrkSrvDebugConnStatusCancel:
		    ConnStatusNotifyCancel(aMessage);
		    break;
		case ETrkSrvCmdGetDebuggingStatus:
			CmdGetDebugStatusL(aMessage);
			break;
		case ETrkSrvDebuggingStatus:
			DebugStatusNotifyL(aMessage);				
			break;
		case ETrkSrvDebuggingStatusCancel:
		    DebugStatusNotifyCancel(aMessage);
		    break;
		case ETrkGetPlugAndPlayType:
		    GetPlugAndPlayTypeL(aMessage);
		    break;
		case ETrkSetPlugAndPlayType:
		    SetPlugAndPlayTypeL(aMessage);
		    break;
		case ETrkSrvCmdCodeShutDownServer:
		    CmdShutDownServer(aMessage);
			break;
		default:
		    aMessage.Panic(KServerIntiatedSessionPanic, ETrkServerInitiatedClientPanicInvalidOperation);
			break;
	}
}

//
// CTrkSrvSession::CmdGetVersionL()
//
// Gets the TRK version information
//
void CTrkSrvSession::CmdGetVersionL(const RMessage2& aMessage)
{
	TTRKVersion version;
	iTrkDebugMgr->GetVersion(version);
	
	TPckgC<TTRKVersion> package(version);
	TRAPD(err, aMessage.WriteL(KSlot0, package));
	if (err != KErrNone)
	{
        aMessage.Panic(_L("TrkServer"), 2);
	}
	aMessage.Complete(KErrNone);
}

//
// CTrkSrvSession::CmdConnectL()
//
void CTrkSrvSession::CmdConnectL(const RMessage2& aMessage)
{
	TRAPD(err,iTrkDebugMgr->ConnectToDebuggerL());
	if (err != KErrNone)
	{
        aMessage.Panic(_L("TrkServer"), 3);
	}
	aMessage.Complete(KErrNone);
}

//
// CTrkSrvSession::CmdDisconnectL()
//
void CTrkSrvSession::CmdDisConnectL(const RMessage2& aMessage)
{
	iTrkDebugMgr->DisConnect();	
	aMessage.Complete(KErrNone);
}

//
// CTrkSrvSession::CmdGetConnStatusL()
//
void CTrkSrvSession::CmdGetCurrentConnDataL(const RMessage2& aMessage)
{
    TTrkConnData connData;  
    iTrkDebugMgr->GetCurrentConnData(connData);
    
    TPckgC<TTrkConnData> package(connData);      
    TRAPD(err, aMessage.WriteL(KSlot0, package));    
    if (err != KErrNone)
    {
        aMessage.Panic(_L("TrkServer"), 2);
    }
    aMessage.Complete(KErrNone);
}

//
// CTrkSrvSession::CmdGetConnStatusL()
//
void CTrkSrvSession::CmdSetCurrentConnDataL(const RMessage2& aMessage)
{   
    TTrkConnData connData;

    TPckg<TTrkConnData> package(connData);
    TRAPD(err, aMessage.ReadL(KSlot0, package));
    if (err != KErrNone)
    {
        aMessage.Panic(_L("TrkServer"), 2);
    }
    TPckgBuf<TBool> connected(iTrkDebugMgr->IsConnected());
    TBuf<KMaxPath> connStatus;
    iTrkDebugMgr->SetCurrentConnData(connData);       
    
    aMessage.Complete(KErrNone);
}

//
// CTrkSrvSession::CmdGetConnStatusL()
//
void CTrkSrvSession::CmdGetDebugConnStatusL(const RMessage2& aMessage)
{	
    TPckgBuf<TTrkConnStatus> package(iTrkDebugMgr->GetConnectionStatus());
    TBuf<KMaxPath> connMsg;
	iTrkDebugMgr->GetConnectionMsg(connMsg);
		
	TRAPD(err, aMessage.WriteL(KSlot0, package));
	TRAPD(err1, aMessage.WriteL(KSlot2, connMsg));
	if (err || err1)
	{
        aMessage.Panic(_L("TrkServer"), 2);
	 }
	aMessage.Complete(KErrNone);   
}

//
// CTrkSrvSession::ConnStatusNotifyL()
//
void CTrkSrvSession::ConnStatusNotifyL(const RMessage2& aMessage)
{
    if (!iPendingConnState)
    {
        iBlockedConnState = aMessage;       
        iPendingConnState = ETrue;
    }
    else
    {
        aMessage.Complete(KErrAlreadyExists);
    }
}

//
//CTrkSrvSession::ConnStatusNotifyCancel()
//
void CTrkSrvSession::ConnStatusNotifyCancel(const RMessage2& aMessage)
{
    if (iPendingConnState && iBlockedConnState.Handle())
    {
       SafeComplete(iBlockedConnState, KErrCancel);
       iPendingConnState = EFalse;        
    }
    SafeComplete(aMessage, KErrNone);
    iTrkDebugMgr->SetDebugMgrCallback(NULL);
}

//
// CTrkSrvSession::CmdGetDebugStatusL()
//
void CTrkSrvSession::CmdGetDebugStatusL(const RMessage2& aMessage)
{
    TPckgBuf<TBool> debugging(iTrkDebugMgr->IsDebugging());        
    TRAPD(err, aMessage.WriteL(KSlot0, debugging));
    
    if (err != KErrNone)
    {
        aMessage.Panic(_L("TrkServer"), 2);
    }
    aMessage.Complete(KErrNone);
}

//
// CTrkSrvSession::DebugStatusNotifyL()
//
void CTrkSrvSession::DebugStatusNotifyL(const RMessage2& aMessage)
{	
    if (!iPendingDebugState)
    {
        iBlockedDebugState = aMessage;       
        iPendingDebugState = ETrue;
    }
    else
    {
        aMessage.Complete(KErrAlreadyExists);
    }
}

//
// CTrkSrvSession::DebugStatusNotifyCancel()
//
void CTrkSrvSession::DebugStatusNotifyCancel(const RMessage2& aMessage)
{
    if (iPendingDebugState && iBlockedDebugState.Handle())
    {
       SafeComplete(iBlockedDebugState, KErrCancel);
       iPendingDebugState = EFalse;        
    }
    SafeComplete(aMessage, KErrNone);
}
//
// CTrkSrvSession::CmdShutDownServer()
//
// Stops the active scheduler. This way, server process will run to completion.
//
void CTrkSrvSession::CmdShutDownServer(const RMessage2& aMessage)
{
    CActiveScheduler::Stop();
    aMessage.Complete(KErrNone);
}

//
// CTrkSrvSession::DebugConnStatusChanged()
//
// It is called when the debug connection status has been found changed
//
void CTrkSrvSession::DebugConnStatusChanged(TTrkConnStatus& aConnStatus)
{
    if (iPendingConnState && iBlockedConnState.Handle())
    {
        TPckg<TTrkConnStatus> package(aConnStatus);
        //write the file data into the client descriptor
        TRAPD(err, iBlockedConnState.WriteL(KSlot0, package));                      
        if (err != KErrNone)
        {
            iBlockedConnState.Panic(_L("TrkServer"), 2);
        }
        SafeComplete(iBlockedConnState, err);    
    }
    iPendingConnState = EFalse;
}

//
// CTrkSrvSession::DebugConnStatusChanged()
//
// Called when the debugging status is changed
//
void CTrkSrvSession::DebuggingStatusChanged(TBool& aDebugging)
{
    if (iPendingDebugState && iBlockedDebugState.Handle())
    {
        TPckg<TBool> package(aDebugging);
        //write the file data into the client descriptor
        TRAPD(err, iBlockedDebugState.WriteL(KSlot0, package));                      
        if (err != KErrNone)
        {
            iBlockedDebugState.Panic(_L("TrkServer"), 2);
        }
        SafeComplete(iBlockedDebugState, err);    
    }
    iPendingDebugState = EFalse;
}

//
// CTrkSrvSession::GetPlugAndPlayType()
//
// To know whether plug and play is set or not
//
void CTrkSrvSession::GetPlugAndPlayTypeL(const RMessage2& aMessage)
{
    TPckg<TBool> plugandPlay(iTrkDebugMgr->GetPlugAndPlayOption());
        
    TRAPD(err, aMessage.WriteL(KSlot0, plugandPlay));
    if (err!= KErrNone)
    {
        aMessage.Panic(_L("TrkServer"), 2);
    }
    aMessage.Complete(KErrNone);  
}

//
// CTrkSrvSession::SetPlugAndPlayType()
//
// To set the plug and play 
//
void CTrkSrvSession::SetPlugAndPlayTypeL(const RMessage2& aMessage)
{   
    TBool plugandPlay;
    TPckg<TBool> package(plugandPlay);
    TRAPD(err, aMessage.ReadL(KSlot0, package));
    if (err!= KErrNone)
    {
        aMessage.Panic(_L("TrkServer"), 2);
    }
    iTrkDebugMgr->SetPlugAndPlayOption(plugandPlay);
    aMessage.Complete(KErrNone);
}

