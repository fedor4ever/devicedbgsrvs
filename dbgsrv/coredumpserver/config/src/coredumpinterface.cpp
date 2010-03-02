// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Implement the interface to the Core Dump Server
//



/**
 @file
 @internalTechnology
 @released
 @see RCoreDumpSession
*/


// Project includes
#include <coredumpinterface.h>

/**
    Called to start the server (by the RCoreDumpSession::Connect())
    The function is only called if the server is not running.
@return KErrNone if server was successfully started, or one of the other
system wide error codes
*/	
TInt RCoreDumpSession::StartServer()
	{
    LOG_MSG("->RCoreDumpSession::StartServer()\n");
    TInt err;
	// Create the server process
	const TUidType serverUid( KNullUid, KNullUid, KCoreDumpServUid );

	RProcess server;
    _LIT(KServerCommand, "");
	LOG_MSG("RCoreDumpSession::StartServer -> Create()\n");
	err = server.Create( KCoreDumpServerName, KServerCommand, serverUid );
	
    if(err != KErrNone)
        {
        LOG_MSG2("RCoreDumpSession::StartServer - creating server failed! err:%d\n", err);
        return err;
        }

	//Set up Rendezvous so that server thread can signal correct startup
	TRequestStatus serverDiedRequestStatus;
	server.Rendezvous( serverDiedRequestStatus );
	
	// Status flag should still be pending as we haven't resumed the server yet!
	if( serverDiedRequestStatus != KRequestPending )
		{
		LOG_MSG2("RCoreDumpSession::StartServer - rendezvous failed! err:%d\n", serverDiedRequestStatus.Int());
		server.Kill(KErrAbort); 
        server.Close();
        return serverDiedRequestStatus.Int();
		}
	server.Resume();

	LOG_MSG("RCoreDumpSession::StartServer -> User::WaitForRequest()\n");

	// Will block here until server signals starts
    User::WaitForRequest( serverDiedRequestStatus );

    LOG_MSG("RCoreDumpSession::StartServer -> Close()\n");

    // we can't use the 'exit reason' if the server panicked as this
    // is the panic 'reason' and may be '0' which cannot be distinguished from KErrNone
    err = ( server.ExitType()==EExitPanic ) ? KErrGeneral : serverDiedRequestStatus.Int();
	server.Close();
	return err;
	}

    
/**
    Constructor. It initializes the member variables to zero.
*/
EXPORT_C RCoreDumpSession::RCoreDumpSession()
    : iConnected(EFalse), iMaxConfigParamSize(0)
	{
	}


/**
First call that a client must make to use the Core Dump server. This call
starts the server if not already running, and then connects to it.
@return KErrNone if connection was successfully created, or one of the other
system wide error codes
*/
EXPORT_C TInt RCoreDumpSession::Connect()
	{
	LOG_MSG("->RCoreDumpSession::Connect()\n");

    if(iConnected != EFalse)
        {
        LOG_MSG("RCoreDumpSession::Connect - already connected\n"); 
        return KErrNone;
        }

    TInt err;

    for(TInt i = 0; i < KConnectionRetries; i++)
        {
        LOG_MSG("RCoreDumpSession::Connect -> StartServer()\n");
        err = StartServer();
        if( (KErrNone != err) && (KErrAlreadyExists != err) )
            {
            return err;
            }

        LOG_MSG("RCoreDumpSession::Connect - CreateSession()\n");
        err = CreateSession( KCoreDumpServerName, Version(), KTTDefaultMessageSlots );

        if(KErrNone == err)
            {
            LOG_MSG("RCoreDumpSession::Connect - session created successfully\n");
            iConnected = ETrue;
            return KErrNone;
            }
        }
    LOG_MSG("RCoreDumpSession::Connect - too many retries!\n");
	return err;
    }

/**
    Required to be called to close the session with the CoreDumpServer.
    It automatically closes this session handle.
*/
EXPORT_C void RCoreDumpSession::Disconnect()
	{
	iConnected = EFalse;
    RSessionBase::Close();
	}

/**
Provides the version number of the server we require for this API.
@return TVersion object representing the version of the server
*/
TVersion RCoreDumpSession::Version () const
	{
	return TVersion(KCoreDumpServMajorVersionNumber,
					KCoreDumpServMinorVersionNumber,
					KCoreDumpServBuildVersionNumber);
	}

/**
Provides the caller with a list of the currently running processes.
@param aProcessList Reference to the pointer list that is going to be filled
with CProcessInfo objects. The caller takes ownership of the content.
@see RProcessPointerList
*/
EXPORT_C void RCoreDumpSession::GetProcessesL( RProcessPointerList & aProcessList ) const
	{
    LOG_MSG("->RCoreDumpSession::GetProcessesL()\n");

    aProcessList.ResetAndDestroy();

	TListRequest request;
	request.iListType = TListRequest::EProcessList;
	request.iSubId1 = 0;
	request.iSubId2 = 0;
	request.iRequiredDescriptorSize = 0;

	const TUint8 * bufStart = GetListLC(request).Ptr();
	TUint nextObjOffset = 0;
	for(TInt i = 0 ; i < request.iRemaining; i++)
		{
        TPtrC8 nextObj( (bufStart + nextObjOffset), CProcessInfo::MaxSize() );
        CProcessInfo * processInfoPtr = CProcessInfo::NewL( nextObj ); 

        TInt err = aProcessList.Append(processInfoPtr);
        if(err != KErrNone)
            {
            LOG_MSG2("RCoreDumpSession::GetProcessesL - unable to append process info object! err:%d\n", err);
            delete processInfoPtr;
            User::Leave(err);
            }
        nextObjOffset += processInfoPtr->Size();
        }
	CleanupStack::PopAndDestroy(); //GetListLC
    }


/**
    Provides the caller with a list of executables.
@param aExecutableList reference to the pointer list that is going to be filled
with CExecutableInfo objects. The caller takes ownership of the content.
@see RExecutablePointerList
*/
EXPORT_C void RCoreDumpSession::GetExecutablesL( RExecutablePointerList & aExecutableList ) const
	{
    LOG_MSG("->RCoreDumpSession::GetExecutablesL()\n");

    aExecutableList.ResetAndDestroy();

	TListRequest request;
	request.iListType = TListRequest::EExecutableList;
	request.iSubId1 = 0;
	request.iSubId2 = 0;
	request.iRequiredDescriptorSize = 0;

	const TUint8 * bufStart = GetListLC(request).Ptr();
	TUint nextObjOffset = 0;
	for(TInt i = 0 ; i < request.iRemaining; i++)
		{
        TPtrC8 nextObj( (bufStart + nextObjOffset), CExecutableInfo::MaxSize() );
        CExecutableInfo * executableInfoPtr = CExecutableInfo::NewL( nextObj ); 

        TInt err = aExecutableList.Append( executableInfoPtr );
        if(err != KErrNone)
            {
            LOG_MSG2("RCoreDumpSession::GetExecutablesL - unable to append Executable info object! err:%d\n", err);
            delete executableInfoPtr;
            User::Leave(err);
            }
        nextObjOffset += executableInfoPtr->Size();
        }
	CleanupStack::PopAndDestroy(); //GetListLC
    }

/**
Provides the caller with a list of currently running threads.
@param aThreadList reference to the pointer list that is going to be filled
with CThreadInfo objects. The caller takes ownership of the content.
@param aPid Specify this to narrow the list of threads to a particular process.
@see RThreadPointerList
*/
EXPORT_C void RCoreDumpSession::GetThreadsL( RThreadPointerList & aThreadList, const TUint64 aPid ) const
	{
    LOG_MSG("->RCoreDumpSession::GetThreadsL()\n");

    aThreadList.ResetAndDestroy();

	TListRequest request;
	request.iListType = TListRequest::EThreadList;
	request.iSubId1 = aPid;
	request.iSubId2 = 0;
	request.iRequiredDescriptorSize = 0;

	const TUint8 * bufStart = GetListLC(request).Ptr();
	TUint nextObjOffset = 0;

	for(TInt i = 0 ; i < request.iRemaining; i++)
		{
        TPtrC8 nextObj( (bufStart + nextObjOffset), CThreadInfo::MaxSize() );
        CThreadInfo * threadInfoPtr = CThreadInfo::NewL( nextObj ); 
        TInt err = aThreadList.Append(threadInfoPtr);
        if(err != KErrNone)
            {
            LOG_MSG2("RCoreDumpSession::GetThreads - unable to append thread info object! err:%d\n", err);
            delete threadInfoPtr;
            User::Leave(err);
            }
        nextObjOffset += threadInfoPtr->Size();
        }

	CleanupStack::PopAndDestroy(); //GetListLC
    }

/**
@param aFormatterList Reference to the pointer list that is going to be filled
with CPluginInfo objects. The caller takes ownership of the content.
@see RPluginPointerList
*/
EXPORT_C void RCoreDumpSession::GetFormattersL( RPluginPointerList &aFormatterList ) const
	{
    LOG_MSG("->RCoreDumpSession::GetFormattersL()\n");

    aFormatterList.ResetAndDestroy();

	TListRequest request;
	request.iListType = TListRequest::EFormatterList;
	request.iSubId1 = 0;
	request.iSubId2 = 0;
	request.iRequiredDescriptorSize = 0;

	const TUint8 * bufStart = GetListLC(request).Ptr();
	TUint nextObjOffset = 0;
	for(TInt i = 0 ; i < request.iRemaining; i++)
		{
        TPtrC8 nextObj( (bufStart + nextObjOffset), CPluginInfo::MaxSize() );
        CPluginInfo * pluginInfoPtr = CPluginInfo::NewL( nextObj ); 

        TInt err = aFormatterList.Append(pluginInfoPtr);
        if(err != KErrNone)
            {
            LOG_MSG2("RCoreDumpSession::GetPluginsL - unable to append plugin info object! err:%d\n", err);
            delete pluginInfoPtr;
            User::Leave(err);
            }
        nextObjOffset += pluginInfoPtr->Size();
        }
	CleanupStack::PopAndDestroy(); //GetListLC
    }

/**
 * Lists the crashes in the flash partition
 * @param aCrashes Array in which to store the list of crashes
 * @leave one of the OS wide codes
 */
EXPORT_C void RCoreDumpSession::ListCrashesInFlashL(RCrashInfoPointerList& aCrashes) const
	{
	LOG_MSG("->RCoreDumpSession::ListCrashesInFlash()\n");

	aCrashes.ResetAndDestroy();

	TListRequest request;
	request.iListType = TListRequest::ECrashList;
	request.iSubId1 = 0;
	request.iSubId2 = 0;
	request.iRequiredDescriptorSize = 0;

	const TUint8 * bufStart = GetListLC(request).Ptr();
	TUint nextObjOffset = 0;
	for(TInt i = 0 ; i < request.iRemaining; i++)
		{
        TPtrC8 nextObj( (bufStart + nextObjOffset), TCrashInfo::MaxSize() );
        TCrashInfo* crashInf = TCrashInfo::NewL( nextObj ); 

        TInt err = aCrashes.Append(crashInf);
        if(err != KErrNone)
            {
            LOG_MSG2("RCoreDumpSession::ListCrashesInFlash - unable to append plugin info object! err:%d\n", err);
            delete crashInf;
            User::Leave(err);
            }
        nextObjOffset += crashInf->iSize;
        }
	CleanupStack::PopAndDestroy(); //GetListLC
	}

/**
 * Deletes crash log specified from the flash partition
 * @param aCrashId Identifier of crash to delete
 * @leave One of the OS wide codes
 */
EXPORT_C void RCoreDumpSession::DeleteCrashLogL(const TInt aCrashId) const
	{
    LOG_MSG2("->RCoreDumpSession::DeleteCrashLogL(ID = [%d])\n", aCrashId);
    TIpcArgs args(aCrashId);

    TInt err = SendReceive(ECoreDumpDeleteLogRequest, args);
    if(err != KErrNone)
        {
        LOG_MSG2("->RCoreDumpSession::DeleteCrashLogL() - unable to delete log -> err:%d\n", err);
        User::Leave(err);
        }
	}

/**
 * Deletes crash partition from the flash partition
 * @param aCrashId Identifier of crash to delete
 * @leave One of the OS wide codes
 */
EXPORT_C void RCoreDumpSession::DeleteCrashPartitionL() const
	{
    LOG_MSG("->RCoreDumpSession::DeleteCrashPartitionL");

    TInt err = SendReceive(ECoreDumpDeleteCrashPartitionRequest);
    if(err != KErrNone)
        {
        LOG_MSG2("->RCoreDumpSession::DeleteCrashPartitionL() - unable to delete log -> err:%d\n", err);
        User::Leave(err);
        }
	}

/**
 * Processes the crash log in the flash partition corrosponding to the crash ID of this 
 * TCrashInfo object. If this doesnt match a crash in the partition, or the flash cannot be read
 * this will leave with KErrCorrupted
 * @param aCrash The crash to be processed
 * @leave One of the OS wide codes
 */
EXPORT_C void RCoreDumpSession::ProcessCrashLogL(TInt aCrashId)
	{
	LOG_MSG2("->RCoreDumpSession::ProcessCrashLogL(ID = [%d])\n", aCrashId);
	
/*    TPckgBuf<TCrashInfo> requestPckg(aCrashInfo);
    TIpcArgs args(&requestPckg);*/
	TIpcArgs args(aCrashId);

    TInt err = SendReceive(ECoreDumpProcessFlashCrash, args);
    if(err != KErrNone)
        {
        LOG_MSG2("->RCoreDumpSession::ProcessCrashLogL() - unable to process crash log -> err:%d\n", err);
        User::Leave(err);
        }
	}

/**
 * Processes the crash log Asynchronously in the flash partition corresponding to the crash ID of this 
 * TCrashInfo object.
 * @param aCrashId The crash to be processed
 * @param aStatus asynchronous request status object 
 */
EXPORT_C void RCoreDumpSession::ProcessCrashLog(const TInt aCrashId, TRequestStatus &aStatus)
	{
	LOG_MSG2("->RCoreDumpSession::ProcessCrashLog async (ID = [%d])\n", aCrashId);
	
	TIpcArgs args(aCrashId);
    SendReceive(ECoreDumpProcessCrashAsync, args, aStatus);

	}

/**
 * Cancel a previously issued asynchronous RCoreDumpSession::ProcessCrashLog call 
 * @param aCrash The crash to be processed
 * @return Any error which may be returned by RSessionBase::SendReceive()
 */
EXPORT_C TInt RCoreDumpSession::CancelProcessCrashLog(TInt aCrashId)
	{
	LOG_MSG2("->RCoreDumpSession::CancelProcessCrashLog async (ID = [%d])\n", aCrashId);
	
	TIpcArgs args(aCrashId);
    return SendReceive(ECoreDumpCancelProcessCrashAsync, args);

	}


/**
@param aFormatterList Reference to the pointer list that is going to be filled
with CPluginInfo objects. The caller takes ownership of the content.
@see RPluginPointerList
*/
EXPORT_C void RCoreDumpSession::GetWritersL( RPluginPointerList &aWriterList ) const
	{
    LOG_MSG("->RCoreDumpSession::GetWritersL()\n");

    aWriterList.ResetAndDestroy();

	TListRequest request;
	request.iListType = TListRequest::EWriterList;
	request.iSubId1 = 0;
	request.iSubId2 = 0;
	request.iRequiredDescriptorSize = 0;

	const TUint8 * bufStart = GetListLC(request).Ptr();
	TUint nextObjOffset = 0;
	for(TInt i = 0 ; i < request.iRemaining; i++)
		{
        TPtrC8 nextObj( (bufStart + nextObjOffset), CPluginInfo::MaxSize() );
        CPluginInfo * pluginInfoPtr = CPluginInfo::NewL( nextObj ); 

        TInt err = aWriterList.Append(pluginInfoPtr);
        if(err != KErrNone)
            {
            LOG_MSG2("RCoreDumpSession::GetPluginsL - unable to append plugin info object! err:%d\n", err);
            delete pluginInfoPtr;
            User::Leave(err);
            }
        nextObjOffset += pluginInfoPtr->Size();
        }
	CleanupStack::PopAndDestroy(); //GetListLC
    }

/**
    Called by GetProcessesL and GetThreadsL methods to get list information from the CoreDumpServer.
@param aRequest structure defining list request
@return descriptor holding serialized list content or null descriptor if unable to get any data.
*/
const TDesC8 &RCoreDumpSession::GetListLC(const TListRequest &aRequest) const
    {
    LOG_MSG2("->RCoreDumpSession::GetListLC(iType=%d)\n", aRequest.iListType);
	TPckg<TListRequest> requestPckgPtr( aRequest );
	TIpcArgs reqArgs( &requestPckgPtr );

	TInt err = SendReceive( ECoreDumpGetListInfo, reqArgs );

    if(err != KErrNone)
		{
		LOG_MSG2( "RCoreDumpSession::GetListL() - unable to get list info! err:%d\n", err );
		User::Leave(err);
	    }

	HBufC8 *buf = HBufC8::NewMaxL( aRequest.iRequiredDescriptorSize );
	CleanupStack::PushL( buf );

	if( ( aRequest.iRemaining == 0) || ( aRequest.iRequiredDescriptorSize == 0 ) )
		{
		LOG_MSG("RCoreDumpSession::GetListL() - no data to transfer\n");
		return *buf;
        }

    TPtr8 ptr(buf->Des());
	TIpcArgs listArgs( &requestPckgPtr, &ptr );
	err = SendReceive( ECoreDumpGetListData, listArgs );

	if(err != KErrNone) 
		{
		LOG_MSG2("RCoreDumpSession::GetListL() - unable to get list data! err:%d\n", err);
		User::Leave(err);
		}

    return *buf;
    }

/**
    Provides the caller with a list of currently available formatter and 
	writer plugins.
@param aPluginList reference to list to be filled with TPluginInfo structs.
    The caller takes ownership of the content.
@see RPluginList
*/
EXPORT_C void RCoreDumpSession::GetPluginListL(RPluginList &aPluginList) const
    {
    LOG_MSG("RCoreDumpSession::GetPluginListL()\n");
    aPluginList.Reset();

    TPluginInfoBlock *pluginArr = new(ELeave) TPluginInfoBlock;
    CleanupStack::PushL(pluginArr);
	TPtr8 pluginArrPtr( (TUint8*)pluginArr, sizeof(TPluginInfoBlock) );

	TListRequest request;
	request.iIndex = 0; //the one that really matters on the server side
    do
        {
        TPckgBuf<TListRequest> requestPckg(request);
        TIpcArgs reqArgs( &requestPckg , &pluginArrPtr );
        TInt err = SendReceive( ECoreDumpServGetPluginList, reqArgs );
        
        if(err != KErrNone)
            {
            LOG_MSG2("RCoreDumpSession::GetPluginListL - unable to get plugin list! err:%d\n", err);
            User::Leave(err);
            }

        request = requestPckg();
		if(request.iIndex == 0)
			{
			LOG_MSG3("RCoreDumpSession::GetPluginListL - first call got:%d from %d\n",
                    request.iSupplied, request.iSupplied + request.iRemaining);
			request.iIndex = request.iSupplied;
			}
		else
			{
			LOG_MSG3("RCoreDumpSession::GetPluginListL - following call got:%d from %d\n",
                    request.iSupplied, request.iSupplied + request.iRemaining);
			request.iIndex += request.iSupplied;
			}

		for(TInt i = 0; i < request.iSupplied; i++)
			{
	            TPluginInfo pluginData;
				pluginData.iVersion		= pluginArr->plugins[i].iVersion;
				pluginData.iName		= pluginArr->plugins[i].iName;
				pluginData.iUid			= pluginArr->plugins[i].iUid;
				pluginData.iType		= pluginArr->plugins[i].iType;
				pluginData.iDescription = pluginArr->plugins[i].iDescription;
				pluginData.iLoaded		= pluginArr->plugins[i].iLoaded;
				aPluginList.AppendL( pluginData );
			}
        }
    while(request.iRemaining > 0);
    CleanupStack::PopAndDestroy(pluginArr);
    }

/**
Make a request to load/unload a plugin.
@param aPluginRequest Structure defining the plugin request
@see TPluginRequest
*/
EXPORT_C void RCoreDumpSession::PluginRequestL(const TPluginRequest &aPluginRequest ) const
{
    LOG_MSG("->RCoreDumpSession::PluginRequestL()\n");

	TPckgBuf<TPluginRequest> requestPckg(aPluginRequest);
	TIpcArgs reqArgs( &requestPckg );

	LOG_MSG4("RCoreDumpSession::PluginRequestL(): iPluginType=%d, iIndex=%d, iUid=0x%X\n", 
            aPluginRequest.iPluginType, aPluginRequest.iIndex, aPluginRequest.iUid ); 

	TInt err = SendReceive( ECoreDumpPluginRequest, reqArgs );
    if(err != KErrNone)
        {
        LOG_MSG2("->RCoreDumpSession::PluginRequestL() - unable to load plugin! err:%d\n", err);
        User::Leave(err);
        }
}

/**
Request the observation of a thread or process. To observe a process, aTargetName and 
aTargetOwnerName must be the same and be the name of the process.
To observe a thread, aTargetOwnerName is the name of the process and aTargetName is the name
To observe an executable, aTargetOwnerName is the full path to the exe and aTargetName is also the full path to the exe
of the thread. 
@param aTargetName Name of process or thread
@param aTargetOwnerName Name of process 
@param aObserve Set to ETrue to request the observation of the target. EFalse to stop observing the target.
@see GetProcessesL
@see GetThreadsL
*/
EXPORT_C void RCoreDumpSession::ObservationRequestL(const TDesC &aTargetName, const TDesC &aTargetOwnerName, TBool aObserve) const
{
    LOG_MSG("->RCoreDumpSession::ObservationRequestL()\n");
    TIpcArgs args(&aTargetName, &aTargetOwnerName, static_cast<TInt>(aObserve));

    TInt err = SendReceive(ECoreDumpObservationRequest, args);
    if(err != KErrNone)
        {
        LOG_MSG2("->RCoreDumpSession::ObservationRequestL() - unable to [un]observe target! err:%d\n", err);
        User::Leave(err);
        }
}


/**
Return the total list of configuration parameters currently available. 
The list is made up of the parameters from the Core Dump Server and any loaded plugins.
@see COptionConfig
*/
EXPORT_C TInt RCoreDumpSession::GetNumberConfigParametersL() const
	{
	LOG_MSG("->RCoreDumpSession::GetNumberConfigParameters()\n" );
    TInt numParams;
	TPtr8 numParamPtr( (TUint8*) &numParams, sizeof(TInt) );

	TPtr8 maxConfigParamSizePtr( (TUint8*) &iMaxConfigParamSize, sizeof(TInt) );
	TIpcArgs reqArgs( &numParamPtr, &maxConfigParamSizePtr );

	TInt err = SendReceive( ECoreDumpGetNumberConfigParams, reqArgs );

    if(err != KErrNone)
        {
	    LOG_MSG2( "RCoreDumpSession::GetNumberConfigParams() - unable to get params number! err:%d \n", err);
        User::Leave(err); 
        }

	return numParams;
	}
										

/**
Obtain the configuration parameter indexed by aIndex. 
@param aIndex This must be less than the value returned by GetNumberConfigParametersL().
@see COptionConfig
*/
EXPORT_C COptionConfig * RCoreDumpSession::GetConfigParameterL( const TInt aIndex ) const
	{
	LOG_MSG("->RCoreDumpSession::GetConfigParameterL()\n" );
	
	// Allocate the max, rounded up, plus 4 bytes for safety
	HBufC8 *buf = HBufC8::NewMaxL( Align4(iMaxConfigParamSize) );
    CleanupStack::PushL(buf);
    TPtr8 ptr( buf->Des() );
	ptr.FillZ();

	TIpcArgs configArgs( aIndex, &ptr );

	TInt err = SendReceive( ECoreDumpGetConfigParam, configArgs );

	if(err != KErrNone) 
		{
		LOG_MSG3("RCoreDumpSession::GetConfigParameter() - unable to get parameter:%d! err:%d\n", aIndex, err );
        User::Leave(err);
		}

    COptionConfig *option = COptionConfig::NewL(*buf);
    CleanupStack::PopAndDestroy(buf);
    return option;
	}


/**
Change a configuration parameter. 
@param aConfig The modified parameter that will be changed. The index and source are 
checked against those held by the server.
*/
EXPORT_C void RCoreDumpSession::SetConfigParameterL( const COptionConfig &aConfig) const
	{
	LOG_MSG4("->RCoreDumpSession::SetConfigParameterL( aSource=%d, aIndex=%d, aInstance=%d)\n",
             aConfig.Source(), aConfig.Index(), aConfig.Index() );

    TConfigRequest request;
    request.iSource = aConfig.Source();
    request.iIndex = aConfig.Index();
    request.iInstance = aConfig.Instance();

	TPckg<TConfigRequest> requestPckgPtr( request );

	TIpcArgs configArgs( &requestPckgPtr, aConfig.Value(), &aConfig.ValueAsDesc() );
	TInt err = SendReceive( ECoreDumpSetConfigParam, configArgs );

    if(err != KErrNone)
        {
	    LOG_MSG3("RCoreDumpSession::SetConfigParameterL() - unable to set parameter:%d! err:%d\n", aConfig.Index(), err );
        User::Leave(err);
        }
	}


/**
Restore a configuration. Configuration files are created by the Core dump server
when it exits and when the call SaveConfigL is made. The format is private and should be
considered binary. When the Core dump server exits it stores the current configuration 
in its private directory with the name coredumpserver.ini.
@param aLoadPath File that will be read and restored
*/
EXPORT_C void RCoreDumpSession::LoadConfigL( const TDesC & aLoadPath ) const
    {
	LOG_MSG("->RCoreDumpSession::LoadConfig()\n" );

	TIpcArgs configArgs( &aLoadPath );
	TInt err = SendReceive( ECoreDumpLoadConfig, configArgs );
    if(err != KErrNone)
        {
	    LOG_MSG2("RCoreDumpSession::LoadConfigL() - unable to load config! err:%d\n", err );
        User::Leave(err);
        }
    }


/**
Save the current configuration to file. 
@param aSavePath File that will be saved
*/
EXPORT_C void RCoreDumpSession::SaveConfigL( const TDesC & aSavePath ) const
    {
	LOG_MSG("->RCoreDumpSession::SaveConfig()\n" );
	TIpcArgs configArgs( &aSavePath );

	TInt err = SendReceive( ECoreDumpSaveConfig, configArgs );
    if(err != KErrNone)
        {
	    LOG_MSG2("RCoreDumpSession::SaveConfigL() - unable to save config! err:%d\n", err );
        User::Leave(err);
        }
    }

