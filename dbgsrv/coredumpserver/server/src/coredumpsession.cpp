// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// Implements the session functinality of the Core Dump Server
//



/**
 @file
 @internalTechnology
 @released
*/
#include <scmconfigitem.h>

#include "coredumpsession.h"

/**
C++ constructor
@param aSession RSecuritySvrSession session parameter
*/
EXPORT_C CCoreDumpSession::CCoreDumpSession( RSecuritySvrSession &aSession )
	: iSecSess( aSession )
	{
	LOG_MSG("->CCoreDumpSession::CCoreDumpSession()\n");
	// No implementation required
	}

/**
C++ Destructor. Saves the current configuration to the standard configuration file
Calls REComSession::FinalClose() to finish the ECOM session.
Disconnect from the Core Dump Server by calling its SessionClosed() method.
*/
EXPORT_C CCoreDumpSession::~CCoreDumpSession()
	{

	LOG_MSG("CCoreDumpSession::~CCoreDumpSession()\n" );

	TRAPD( err, SaveConfigFileL() );
	if( KErrNone != err )
		{
		_LIT(KSaveConfigFileErrorMsg ,"Warning :Core Dump Server Could Not Save Configuration File.");
		User::InfoPrint( KSaveConfigFileErrorMsg );
		}

	iPluginList.Reset();
	LOG_MSG("<-iPluginList.Reset()" );

	iThreadPointerList.ResetAndDestroy();
	LOG_MSG("<-iThreadPointerList.ResetAndDestroy()" );

	iProcessPointerList.ResetAndDestroy();
	LOG_MSG("<-iProcessPointerList.ResetAndDestroy()" );

	iExecutablePointerList.ResetAndDestroy();
	LOG_MSG("<-iProcessPointerList.ResetAndDestroy()" );

	iObservationList.ResetAndDestroy();
	LOG_MSG("<-iObservationList.ResetAndDestroy()" );

    iConfigList.ResetAndDestroy();
	LOG_MSG("<-iConfigList.ResetAndDestroy()" );
	
	
	iScmConfigList.ResetAndDestroy();

    iTotalConfigList.Reset();

    iAllowedPlugins.Reset();

    if(iDataSource)
        {
		LOG_MSG("->delete iDataSource" );
        delete iDataSource;
        iDataSource = NULL;
        }
    
    if(iFlashDataSource)
    	{
		LOG_MSG("->delete iFlashDataSource" );
        delete iFlashDataSource;
        iFlashDataSource = NULL;    	
    	}

    if( iCrashHandler )
        {
		LOG_MSG("->delete iCrashHandler" );
        delete iCrashHandler;
        iCrashHandler = NULL;
        }

    iFormatterPlugins.ResetAndDestroy();
    iFormatterInfos.ResetAndDestroy();
    if(iMarshalledFormatterList)
    {
        delete iMarshalledFormatterList;
    }

    iWriterPlugins.ResetAndDestroy();
    iWriterInfos.ResetAndDestroy();
    if(iMarshalledWriterList)
    {
        delete iMarshalledWriterList;
    }


	LOG_MSG( "->RProperty::Delete(KCoreDumpServUid, ECancelCrash)" );
    err = RProperty::Delete(KCoreDumpServUid, ECancelCrash);
    if(err != KErrNone)
        {
        LOG_MSG2("CCoreDumpSession::~CCoreDumpSession - unable to delete 'cancel crash' property! err:%d\n", err);
        }

	LOG_MSG("->RProperty::Delete(KCoreDumpServUid, ECrashProgress)" );
    err = RProperty::Delete(KCoreDumpServUid, ECrashProgress);
    if(err != KErrNone)
        {
        LOG_MSG2("CCoreDumpSession::~CCoreDumpSession - unable to delete 'crash progress' property! err:%d\n", err);
        }

	
    err = RProperty::Delete(KCoreDumpServUid, ECrashCount);
    if(err != KErrNone)
        {
        LOG_MSG2("CCoreDumpSession::~CCoreDumpSession - unable to delete 'crash count' property! err:%d\n", err);
        }
    
    err = RProperty::Delete(KCoreDumpServUid, ECrashMediaName);
    if(err != KErrNone)
        {
        LOG_MSG2("CCoreDumpSession::~CCoreDumpSession - unable to delete 'crash media name' property! err:%d\n", err);
        }

    if(iMarshalledProcessList != NULL)
        {
		LOG_MSG("->delete iMarshalledProcessList;\n" );
        delete iMarshalledProcessList;
        }

    if(iMarshalledExecutableList != NULL)
        {
		LOG_MSG("->delete iMarshalledExecutableList;\n" );
        delete iMarshalledExecutableList;
        }

    if(iMarshalledThreadList != NULL)
        {
		LOG_MSG( "->delete iMarshalledThreadList;\n" );
        delete iMarshalledThreadList;
        }
    
    iCrashList.ResetAndDestroy();
    if(iMarshalledCrashList != NULL)
    	{
		LOG_MSG( "->delete iMarshalledCrashList;\n" );
        delete iMarshalledCrashList;   	
    	}
    
	LOG_MSG("REComSession::FinalClose()" );
    REComSession::FinalClose();

	Server().SessionClosed();

	LOG_MSG("<-CCoreDumpSession::~CCoreDumpSession()" );

	}

#define KObservationListGranularity (20)

/**
* Symbian OS 2nd stage construction
*/
void CCoreDumpSession::ConstructL()
	{
	LOG_MSG( "CCoreDumpSession::ConstructL()\n" );

    iMarshalledThreadList = NULL;
	iTotalThreadListDescSize = 0;
    iMarshalledProcessList = NULL;
	iTotalProcessListDescSize = 0;
    iMarshalledExecutableList = NULL;
	iTotalExecutableListDescSize = 0;
    iMarshalledFormatterList = NULL;
	iTotalFormatterListDescSize = 0;
    iMarshalledWriterList = NULL;
	iTotalWriterListDescSize = 0;

	iObservationList.ReserveL(KObservationListGranularity);

    static _LIT_SECURITY_POLICY_PASS(KAllowAllPolicy); //do we want to limit it?? DOS attack via CancelCrash

    //UI sets to ETrue, formatter gets, crash handler resets
    TInt err = RProperty::Define(KCoreDumpServUid, ECancelCrash, RProperty::EInt, KAllowAllPolicy, KAllowAllPolicy);
    if( (err != KErrAlreadyExists) && (err != KErrNone) )
        { 
        LOG_MSG2("CCoreDumpSession::ConstrucL - unable to create 'cancel crash' property! err:%d\n", err);
        User::LeaveIfError(err);
        }

    //UI gets and displays, formatter sets to update the progress, crash handler resets
    err = RProperty::Define(KCoreDumpServUid, ECrashProgress, RProperty::EText, KAllowAllPolicy, KAllowAllPolicy);
    if( (err != KErrAlreadyExists) && (err != KErrNone) )
        {
        LOG_MSG2("CCoreDumpSession::ConstrucL - unable to create 'crash progress' property! err:%d\n", err);
        User::LeaveIfError(err);
        }

    //UI gets crash count
    err = RProperty::Define(KCoreDumpServUid, ECrashCount, RProperty::EInt, KAllowAllPolicy, KAllowAllPolicy);
    if( (err != KErrAlreadyExists) && (err != KErrNone) )
        {
        LOG_MSG2("CCoreDumpSession::ConstrucL - unable to create 'crash count' property! err:%d\n", err);
        User::LeaveIfError(err);
        }
    
    //UI gets the crash media name
    err = RProperty::Define(KCoreDumpServUid, ECrashMediaName, RProperty::ELargeByteArray, KAllowAllPolicy, KAllowAllPolicy, RProperty::KMaxLargePropertySize);
    if( (err != KErrAlreadyExists) && (err != KErrNone) )
        {
        LOG_MSG2("CCoreDumpSession::ConstrucL - unable to create 'crash media name' property! err:%d\n", err);
        User::LeaveIfError(err);
        }

    
    iCrashHandler = CCrashHandler::NewL(*this);

	iConfigList.ReserveL( (TInt)ELastParam );
	
	iScmConfigList.ReserveL((TInt)Debug::TConfigItem::ELast);
	
	

	// This list will contain all the configuration parameters for the entire system,
	// but we do not know how many there are. At least hold our own params.

	//LOG_MSG( "  -> new (ELeave) RConfigParameterList()\n" );

	iTotalConfigList.ReserveL( (TInt)ELastParam );

	COptionConfig * config;

	//LOG_MSG( "  -> NewL KSuspendOwningProcsPrompt\n" );

	RProcess thisProc;
	iOurSecureID = thisProc.SecureId();
	TUint32 ourSecureID = iOurSecureID.iUid;
	//LOG_MSG2( "  thisProc.SecureId()=0x%X\n", ourSecureID );

    
	// This is stored as a bitfield
	_LIT( KCrashedEventTypePrompt, "Select Crash Trigger:\n  1-HW Exception,\n  2-Thread Kill,\n  3-Both" );
	iCrashEventTypes = ECrashHwExcAndKillThread;
	config = COptionConfig::NewL(  (TInt)ECrashEventTypes,
									ourSecureID,
									COptionConfig::ECoreDumpServer,
									COptionConfig::ETUInt, 
									KCrashedEventTypePrompt, 
									1,
									KNullDesC,
									iCrashEventTypes,
									KNullDesC );
    
	iConfigList.AppendL( config );


	_LIT( KPreCrashEventActionPrompt, "Action On Crash:\n  0-None,\n  1-Suspend Thread,\n  2-Suspend Process" );
	iPreCrashEventAction = ESuspendThread;
	config = COptionConfig::NewL(  (TInt)EPreCrashEventAction,
									ourSecureID,
									COptionConfig::ECoreDumpServer,
									COptionConfig::ETUInt, 
									KPreCrashEventActionPrompt, 
									1,
									KNullDesC,
									iPreCrashEventAction,
									KNullDesC );
	iConfigList.AppendL( config );
    

	_LIT( KPostCrashEventActionPrompt, "Action After Crash:\n  0-None,\n  1-Resume Thread,\n  2-Resume Process,\n  4-Kill Process" );
	iPostCrashEventAction = EResumeThread;
	config = COptionConfig::NewL(  (TInt)EPostCrashEventAction,
									ourSecureID,
									COptionConfig::ECoreDumpServer,
									COptionConfig::ETUInt, 
									KPostCrashEventActionPrompt, 
									1,
									KNullDesC,
									iPostCrashEventAction,
									KNullDesC );

	iConfigList.AppendL( config );

	iDataSource = CServerCrashDataSource::NewL(iSecSess);
	iFlashDataSource = CFlashDataSource::NewL(iSecSess);

	ReadPluginUidFilesL();	
    }

void CCoreDumpSession::CreateL()
{
    LOG_MSG("->CCoreDumpSession::CreateL()\n");
	Server().SessionOpened();
}

/**
* Symbian OS 1st stage construction called by CCoreDumpServer::NewSessionL()
*/
EXPORT_C CCoreDumpSession* CCoreDumpSession::NewL( RSecuritySvrSession &aSession )
	{
	LOG_MSG( "CCoreDumpSession::NewL()\n" );
	CCoreDumpSession* self = new (ELeave) CCoreDumpSession( aSession );
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

CCoreDumpServer& CCoreDumpSession::Server() const
{
    LOG_MSG("->CCoreDumpServer::Server()\n");
    return *static_cast<CCoreDumpServer*>(const_cast<CServer2*>(CSession2::Server()));
}

/*
* Called each time a client request message is received
*/
EXPORT_C void CCoreDumpSession::ServiceL(const RMessage2& aMessage)
	{
	// Any security checks on a connected client can be made here using the aMessage parameter.
	// aMessage is the service request message from the client thread.
    TInt err = KErrNone;

	TRAP(err, DispatchMessageL(aMessage));

	if ((aMessage.Function()) != ECoreDumpProcessCrashAsync)
		{//ECoreDumpProcessFlashCrashAsync is an async request so we complete the request when we are done
		aMessage.Complete(err);
		}
    	
	}

//
_LIT(KSessionPanic,"CCoreDumpSession panic");
/*
* Called when the client should be panicked
*/
void CCoreDumpSession::PanicClient(const RMessage2& aMessage, TInt aPanic)
	{
	aMessage.Panic(KSessionPanic, aPanic);
	}


/*
* Called by ServiceL() when a request message is received, calls the appropriate service
*/
void CCoreDumpSession::DispatchMessageL(const RMessage2& aMessage)
	{
	// Call the required service given by the request message's function number
	switch(aMessage.Function())
		{
		case ECoreDumpServGetPluginList:
			{
			LOG_MSG( "CCoreDumpSession::DispatchMessageL() ECoreDumpServerGetPluginList\n" );
			GetPluginListL(aMessage);
			break;
			}
		case ECoreDumpPluginRequest:
			{
			LOG_MSG("CCoreDumpSession::DispatchMessageL() ECoreDumpPluginRequest\n");
            if(iCrashHandler->CrashInProgress())
                {
                LOG_MSG( "CCoreDumpSession::DispatchMessageL - unable to handle because of crash in progress\n" );
                User::Leave(KErrServerBusy);
                }
            else	
            	{
			    PluginRequestL( aMessage );
            	}
			break;
			}
		case ECoreDumpGetListInfo:
			{
			LOG_MSG("CCoreDumpSession::DispatchMessageL() ECoreDumpListInfo\n");
			ListInfoL( aMessage );
			break;
			}
		case ECoreDumpGetListData:
			{
			LOG_MSG("CCoreDumpSession::DispatchMessageL() ECoreDumpGetList\n");
			GetListL( aMessage );
			break;
			}
		case ECoreDumpGetNumberConfigParams :
			{
			LOG_MSG( "CCoreDumpSession::DispatchMessageL() ECoreDumpGetNumberConfigParams\n" );
			GetNumberConfigParametersL( aMessage );
			break;
			}
		case ECoreDumpGetConfigParam :
			{
			LOG_MSG( "CCoreDumpSession::DispatchMessageL() ECoreDumpGetConfigParam\n" );
			GetConfigParameterL( aMessage );
			break;
			}
		case ECoreDumpSetConfigParam :
			{
			LOG_MSG( "CCoreDumpSession::DispatchMessageL() ECoreDumpSetConfigParam\n" );
            if(iCrashHandler->CrashInProgress())
                {
                LOG_MSG( "CCoreDumpSession::DispatchMessageL - unable to handle because of crash in progress\n");
                User::Leave(KErrServerBusy);
                }
            else	
            	{
			    SetConfigParameterL( aMessage );
            	}
			break;
			}
        case ECoreDumpObservationRequest:
        	{
			LOG_MSG( "CCoreDumpSession::DispatchMessageL() ECoreDumpObservationRequest\n" );
            ObservationRequestL( aMessage );
            break;
        	}
		case ECoreDumpLoadConfig :
			{
			LOG_MSG( "CCoreDumpSession::DispatchMessageL() ECoreDumpLoadConfig\n" );
			LoadConfigFileL( &aMessage );
			break;
			}
		case ECoreDumpSaveConfig :
			{
			LOG_MSG( "CCoreDumpSession::DispatchMessageL() ECoreDumpSaveConfig\n" );
			SaveConfigFileL( &aMessage );
			break;
			}
		case ECoreDumpDeleteCrashPartitionRequest:
			{
			LOG_MSG( "CCoreDumpSession::DispatchMessageL() ECoreDumpDeleteCrashPartitionRequest\n" );
			DeleteCrashPartitionL();
			break;
			}
		case ECoreDumpDeleteLogRequest:
			{
			LOG_MSG( "CCoreDumpSession::DispatchMessageL() ECoreDumpDeleteLogRequest\n" );
			//DeleteCrashLogL(aMessage);
			User::Leave(KErrNotSupported);
			break;
			}
		case ECoreDumpProcessFlashCrash:
			{
			LOG_MSG( "CCoreDumpSession::DispatchMessageL() ECoreDumpProcessFlashCrash\n" );
			ProcessCrashLogL(aMessage);
			break;
			}
		case ECoreDumpProcessCrashAsync:
			{
			LOG_MSG( "CCoreDumpSession::DispatchMessageL() ECoreDumpProcessFlashCrashAsync\n" );
			ProcessCrashLogAsyncL(aMessage);
			break;			
			}
		case ECoreDumpCancelProcessCrashAsync:
			{
			LOG_MSG( "CCoreDumpSession::DispatchMessageL() ECoreDumpCancelProcessFlashCrashAsync\n" );
			CancelProcessCrashLogAsync(aMessage);
			break;			
			}
		default:
			{
			// Unknown function number - panic the client
			PanicClient(aMessage, EBadRequest);
			}
		}
	}


