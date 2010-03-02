/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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


#include "TrkEngineModel.h"
#include "TrkEngine.h"
#ifdef __USE_NEW_DEBUG_API__
#include "trkdispatchlayer2.h"
#else
#include "TrkDispatchLayer.h"
#endif

#ifndef __NOKIA_TEXT_SHELL__
#include "TrkSerialCommPort.h"
#endif

#ifndef __TEXT_SHELL__
#include "TrkBtSocketCommPort.h"
#include "TrkUsbPortListener.h"
#include "TrkDbgTrcCommPort.h"
#endif //__TEXT_SHELL__

#ifdef __OEM_TRK__	  
#include "TrkDccCommPort.h"

#ifdef __ENABLE_XTI__
#include "TrkXtiCommPort.h"
#endif
#endif //__OEM_TRK__


//
//
// CTrkEngine implementation
//
//

//
// CTrkEngine::NewL
//
EXPORT_C CTrkEngine* CTrkEngine::NewL()
{
	CTrkEngine* self = new(ELeave) CTrkEngine;
	self->ConstructL();
	return self;
}

//
// CTrkEngine::NewLC
//
EXPORT_C CTrkEngine* CTrkEngine::NewLC()
{
	CTrkEngine* self = new(ELeave) CTrkEngine;
	CleanupStack::PushL(self);
	
	self->ConstructL();
	
	return self;
}


EXPORT_C CTrkEngine* CTrkEngine::NewL(MTrkEngineCallback* aCallback)
{
	CTrkEngine* self = new(ELeave) CTrkEngine;
	self->ConstructL(aCallback);
	return self;
}

//
// CTrkEngine::NewLC
//
EXPORT_C CTrkEngine* CTrkEngine::NewLC(MTrkEngineCallback* aCallback)
{
	CTrkEngine* self = new(ELeave) CTrkEngine;
	CleanupStack::PushL(self);
	
	self->ConstructL(aCallback);
	
	return self;
}

//
// CTrkEngine destructor
//
CTrkEngine::CTrkEngine()
:iCommPort(NULL),
 iDispatchLayer(NULL),
 iModel(NULL),
 iCommPortListener(NULL),
 iInactivityTimerDisabler(NULL),
 iCallback(NULL),
 iCurrentConnType(ETrkBt)
{
}

//
// CTrkEngine::ConstructL
//

void CTrkEngine::ConstructL(MTrkEngineCallback* aCallback)
{	
    // make sure the kernel side device driver is not already loaded
    User::FreeLogicalDevice(KMetroTrkDriverName);
    // load the kernel side device driver
#ifndef __WINS__
    TInt err = User::LoadLogicalDevice(_L("TrkDriver.ldd"));
    if ((KErrAlreadyExists != err) && (KErrNone != err))
        User::Leave(err);       
#endif

	iModel = CTrkEngineModel::NewL();
	iCallback = aCallback;
}

//
// CTrkEngine destructor
//
EXPORT_C CTrkEngine::~CTrkEngine()
{
    #ifndef __TEXT_SHELL__
    // if USB connection, then stop the cable listener as well.
    if (iCurrentConnType == ETrkUsbDbgTrc)
    {
    	iCommPortListener->StopListening();
    }
	#endif
    
    //Unload the driver stuff
#ifndef __WINS__
    TInt err = User::FreeLogicalDevice(KMetroTrkDriverName);
    if (KErrNone != err)
        User::Panic(_L("FreeLogicalDevice failed"), err);
#endif
    
	SafeDelete(iDispatchLayer);
	SafeDelete(iModel);
	SafeDelete(iCommPortListener);	
	SafeDelete(iInactivityTimerDisabler);
}

//
// CTrkEngine::Start
//
EXPORT_C void CTrkEngine::StartL()
{
	iModel->GetConnData(iCurrentConnData);
	iCurrentConnType = iCurrentConnData.iConnType;
	
	DoStartL();
}

//
// CTrkEngine::Start
//
EXPORT_C void CTrkEngine::StartL(TTrkConnType aConnType)
{
	iModel->GetDefaultConnData(aConnType, iCurrentConnData);
	iCurrentConnType = iCurrentConnData.iConnType;
	
	DoStartL();
}

//
// CTrkEngine::Stop
//
EXPORT_C void CTrkEngine::Stop()
{
	if (iDispatchLayer)
	{
		iDispatchLayer->StopListening();	
		SafeDelete(iDispatchLayer);
		iCommPort = NULL; //port is deleted by the framing layer
	}
}

//
// CTrkEngine::GetVersionInfo
//
EXPORT_C void CTrkEngine::GetVersionInfo(TInt &aMajorVersion, TInt &aMinorVersion, TInt &aMajorAPIVersion, TInt &aMinorAPIVersion, TInt &aBuildNumber)
{
	CTrkDispatchLayer::GetVersionInfo(aMajorVersion, aMinorVersion, aMajorAPIVersion, aMinorAPIVersion, aBuildNumber);
}

//
// CTrkEngine::GetConnectionInfo
//
EXPORT_C void CTrkEngine::GetConnectionInfo(TDes& aMessage)
{
	aMessage = KNullDesC;
	if (iCommPort)
	{
		iCommPort->GetConnectionInfo(aMessage);
	}
}
//
// CTrkEngine::GetConnectionInfo
//
EXPORT_C TTrkConnStatus CTrkEngine::GetConnectionStatus()
{
    if (iCommPort)
    {
        
        return(iCommPort->GetConnectionStatus());
    }
    else
        return ETrkConnectionError;
        
}
//
// CTrkEngine::GetErrorInfo
//
EXPORT_C void CTrkEngine::GetErrorInfo(TDes& aMessage)
{
	if (iCommPort)
	{
		iCommPort->GetErrorInfo(aMessage);
	}
	else
	{
		aMessage = iErrorMessage;
	}
}

//
// CTrkEngine::GetConnectionData
//
EXPORT_C void CTrkEngine::GetConnectionData(TTrkConnData& aConnData)
{
	iModel->GetConnData(aConnData);
}

//
// CTrkEngine::SetConnectionData
//
EXPORT_C TInt CTrkEngine::SetConnectionData(TTrkConnData& aConnData)
{
	return iModel->SetConnData(aConnData);
}

//
// CTrkEngine::IsDebugging
//
EXPORT_C TInt CTrkEngine::IsDebugging()
{
	if (iDispatchLayer)
	{
		return iDispatchLayer->IsDebugging();
	}
	return EFalse;
}

//
// CTrkEngine::GetConnectionType
//
EXPORT_C TTrkConnType CTrkEngine::GetConnectionType()
{
    return iCurrentConnType;
}
//
// CTrkEngine::GetPlugPlaySetting()
//
EXPORT_C TBool CTrkEngine::GetPlugPlaySetting()
{
    TBool PlugPLaySetting;
    PlugPLaySetting = iModel->GetPlugPlaySettingValue();
    return PlugPLaySetting;
    
}
//
// CTrkEngine::DoStartL
//
void CTrkEngine::DoStartL()
{
	CreateCommInterfaceL();

	// ownership of the comm port is passed down to CTrkFramingLayer
	iDispatchLayer = CTrkDispatchLayer::NewL(iCommPort, this);
		
#ifndef __TEXT_SHELL__		
	// start listening for the cable connection.	
	/* not required here as server starts listening
	 
	if (iCurrentConnType == ETrkUsbDbgTrc)
	{
		if (!iCommPortListener) // create the listener only once
			iCommPortListener = new CTrkUsbPortListener(this);

		iCommPortListener->ListenL();
	}
	*/
#endif //__TEXT_SHELL__	

	iCommPort->OpenPortL();
	if (iCommPort->IsConnectionEstablished())
	{	  
		iDispatchLayer->Listen();
	
		if (iCurrentConnData.iConnType == ETrkBt || iCurrentConnData.iConnType == ETrkSerial)
			iCommPort->SendDataL(_L8("Trk for Symbian OS started"));
	}

}

//
// CTrkEngine::CreateCommInterfaceL
//
void CTrkEngine::CreateCommInterfaceL()
{
	iCommPort = NULL;

	switch (iCurrentConnData.iCommType)
	{

	#ifndef __NOKIA_TEXT_SHELL__		
		case ESerialComm:
		{
			iCommPort = CTrkSerialCommPort::NewL(iCurrentConnData, iErrorMessage);
			break;
		}
	#endif
	
	#ifndef __TEXT_SHELL__		
		case EBtSocket:
		{
			iCommPort = CTrkBtSocketCommPort::NewL(iCurrentConnData, iErrorMessage, this);			
			break;
		}
		case EDbgTrcComm:
		{
			iCommPort = CTrkDbgTrcCommPort::NewL(iCurrentConnData, iErrorMessage);
			break;
		}
	#endif //__TEXT_SHELL__
		
	#ifdef __OEM_TRK__	  
		case EDcc:
		{
			iCommPort = CTrkDccCommPort::NewL(iCurrentConnData, iErrorMessage);			
			break;
		}
		
		#ifdef __ENABLE_XTI__
		case EXti:
		{
			iCommPort = CTrkXtiCommPort::NewL(iCurrentConnData, iErrorMessage);			
			break;
		}		
		#endif
	#endif //__OEM_TRK__
	
		default:
		{
			User::Leave(KErrNotFound);
		}
	}
	}
	
/*
 * This is called by when BT socket finds the port and started to listen  
 * 
 */
void CTrkEngine::AsyncConnectionSuccessfulL()
{
    iDispatchLayer->Listen();    

    if (iCurrentConnData.iConnType == ETrkBt || iCurrentConnData.iConnType == ETrkSerial)
        iCommPort->SendDataL(_L8("Trk for Symbian OS started"));
    
    if (iCallback)
			iCallback->OnConnection();
	}

/*
 * This is called when PC rejects the connection
 * 
 */
void CTrkEngine::AsyncConnectionFailed()
{
    iCallback->OnAsyncConnectionFailed();   
}

//
// CTrkEngine::DebuggingStarted
// Called from the dispatch layer to indicate the start of a debug session
//
void CTrkEngine::DebuggingStarted()
{
	//start the inactivity timer
	StartInactivityTimerDisabler();
	
	if (iCallback)
		iCallback->DebuggingStarted();
}

//
// CTrkEngine::DebuggingEnded
// Called from the dispatch layer when a debug session ends.
//
void CTrkEngine::DebuggingEnded()
{	
	if (iCallback)
		iCallback->DebuggingEnded();
	//deactivate inactivity timer
	SafeDelete(iInactivityTimerDisabler);
}

// CTrkEngine::StartInactivityTimerDisabler 
//
void CTrkEngine::StartInactivityTimerDisabler()
{	
	iInactivityTimerDisabler = new(ELeave) CInactivityTimerDisabler;
    iInactivityTimerDisabler->ConstructL();  
    iInactivityTimerDisabler->Activate();
}

//
// DLL Entry point
//
#ifndef EKA2
TInt E32Dll(TDllReason)
{
	return KErrNone;
}
#endif

