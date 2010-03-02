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
// core_dump_session_calls.cpp
//



/**
 @file
 @internalTechnology
 @released
*/

#include <crashlogwalker.h>
#include <scmconfigitem.h>
#include "coredumpsession.h"

using namespace Debug;

// Creates and returns a heap descriptor which holds contents of list
HBufC8* CCoreDumpSession::MarshalListL( TListRequest & req, TAny * list )
	{

	LOG_MSG("->CCoreDumpSession::MarshalListL()\n" );

	// Create a dynamic flat buffer to hold this object's member data	
	// Can only use CBufFlat due to Store supporting CBufFlat and CBufSeg
	// See Symbian OS guide » System libraries » Using Store » Streaming » Templated stream operators

	const TInt KExpandSize = 128; // "Granularity" of dynamic buffer

	CBufFlat* buf = CBufFlat::NewL( KExpandSize );
	CleanupStack::PushL( buf );

	RBufWriteStream stream( *buf ); // Stream over the buffer
	CleanupClosePushL( stream );

	for( TInt i = 0; i < req.iRemaining; i ++ )
		{		
		switch( req.iListType )
			{
			case TListRequest::EThreadList:
				{
				//LOG_MSG2( "  -> ((*threadPointerList)[%d])->ExternalizeL( )\n", i );
				iThreadPointerList[i]->ExternalizeL( stream, buf );
				//LOG_MSG2( "  buf->Size()=%d\n", buf->Size() );
				break;
				}
			case TListRequest::EProcessList:
				{
				//LOG_MSG2( "  -> ((*processPointerList)[%d])->ExternalizeL( )\n", i );
				iProcessPointerList[i]->ExternalizeL( stream, buf );
				//LOG_MSG2( "  buf->Size()=%d\n", buf->Size() );
				break;
				}

			case TListRequest::EExecutableList:
				{
				//LOG_MSG2( "  -> ((*processPointerList)[%d])->ExternalizeL( )\n", i );
				iExecutablePointerList[i]->ExternalizeL( stream, buf );
				//LOG_MSG2( "  buf->Size()=%d\n", buf->Size() );
				break;
				}
            case TListRequest::EFormatterList:
                {
				LOG_MSG2( "  -> ((*formatterInfos)[%d])->ExternalizeL( )\n", i );
				iFormatterInfos[i]->ExternalizeL( stream, buf );
				//LOG_MSG2( "  buf->Size()=%d\n", buf->Size() );
				break;
                }
            case TListRequest::EWriterList:
                {
				LOG_MSG2( "  -> ((*writerInfos)[%d])->ExternalizeL( )\n", i );
				iWriterInfos[i]->ExternalizeL( stream, buf );
				//LOG_MSG2( "  buf->Size()=%d\n", buf->Size() );
				break;
                }
            case TListRequest::ECrashList:
            	{
            	LOG_MSG2( "  -> ((*iCrashList)[%d])->ExternalizeL( )\n", i );
            	iCrashList[i]->ExternalizeL(stream, buf);
            	//LOG_MSG2( "  buf->Size()=%d\n", buf->Size() );
            	break;
            	}
			default:
				{
				LOG_MSG("CCoreDumpSession::MarshalListL : Received unknown list type");				
				User::Leave( KErrNotSupported );
				}
			}
		}

	stream.CommitL();

	TInt bufSize = buf->Size();

	// Create a heap descriptor from the buffer
	HBufC8* des = HBufC8::NewL( buf->Size() );
	TPtr8 ptr( des->Des() );

	buf->Read( 0, ptr, buf->Size() );
	
	CleanupStack::PopAndDestroy( &stream );
	CleanupStack::PopAndDestroy( buf ); 
	
	return des;
	}


void CCoreDumpSession::GetListL( const RMessage2& aMessage )
	{
	LOG_MSG( "CCoreDumpSession::GetListL()\n" );

	TListRequest listRequest; 
	HBufC8 *listDes = NULL;

	TPtr8 listReqPtr( (TUint8 *)&listRequest, sizeof(TListRequest) );

	aMessage.ReadL( 0, listReqPtr );

	switch( listRequest.iListType )
		{
		case TListRequest::EThreadList :
			{
			if( listRequest.iRequiredDescriptorSize != iTotalThreadListDescSize )
				{
				LOG_MSG( "  ERROR !* listReq.iRequiredDescriptorSize != iTotalThreadListDescSize\n" );
				User::Leave( KErrBadDescriptor );
				}
			listDes = iMarshalledThreadList;
			break;
			}
		case TListRequest::EProcessList :
			{
			if( listRequest.iRequiredDescriptorSize != iTotalProcessListDescSize )
				{
				LOG_MSG( "  ERROR !* listReq.iRequiredDescriptorSize != iTotalProcessListDescSize\n" );
				User::Leave( KErrBadDescriptor );
				}
			listDes = iMarshalledProcessList;
			break;
			}
		case TListRequest::EExecutableList :
			{
			if( listRequest.iRequiredDescriptorSize != iTotalExecutableListDescSize )
				{
				LOG_MSG( "  ERROR !* listReq.iRequiredDescriptorSize != iTotalExecutableListDescSize\n" );
				User::Leave( KErrBadDescriptor );
				}
			listDes = iMarshalledExecutableList;
			break;
			}
		case TListRequest::EFormatterList :
			{
			if( listRequest.iRequiredDescriptorSize != iTotalFormatterListDescSize )
				{
				LOG_MSG( "  ERROR !* listReq.iRequiredDescriptorSize != iTotalFormatterListDescSize\n" );
				User::Leave( KErrBadDescriptor );
				}
			listDes = iMarshalledFormatterList;
			break;
			}
		case TListRequest::EWriterList :
			{
			if( listRequest.iRequiredDescriptorSize != iTotalWriterListDescSize )
				{
				LOG_MSG( "  ERROR !* listReq.iRequiredDescriptorSize != iTotalWriterListDescSize\n" );
				User::Leave( KErrBadDescriptor );
				}
			listDes = iMarshalledWriterList;
			break;
			}
		case TListRequest::ECrashList :
			{
			if( listRequest.iRequiredDescriptorSize != iTotalCrashInfoDescSize )
				{
				LOG_MSG( "  ERROR !* listReq.iRequiredDescriptorSize != iTotalWriterListDescSize\n" );
				User::Leave( KErrBadDescriptor );
				}
			listDes = iMarshalledCrashList;
			break;
			}
		default :
			{
			LOG_MSG2( " ERROR !* : CCoreDumpSession::ListInfoL() : invalid TListRequestType %d\n", 	listRequest.iListType );
			User::Leave( KErrArgument );
			}
		}

	if ( listDes == NULL )
		{

		LOG_MSG( "  ERROR !* MarshalData returned null buf\n" );
		User::Leave( KErrBadDescriptor );

		}
	else
		{

		//LOG_MSG( "  -> TPtr8 lsitPtr( listDes->Des() )\n" );
		TPtr8 listPtr( listDes->Des() );
		//LOG_MSG2( "  listPtr->Size()=%d\n", listPtr.Size() );

		if( listPtr.Size() == 0 )
			{
			LOG_MSG( "  ERROR !* : if( lsitPtr.Size() == 0 )\n" );
			User::Leave( KErrBadDescriptor );
			}

		//LOG_MSG( "  -> WriteL( 1, listPtr )\n" );
		aMessage.WriteL( 1, listPtr );

		}

	}


void CCoreDumpSession::ListInfoL( const RMessage2& aMessage )
	{
    LOG_MSG( "CCoreDumpSession::ListInfoL()\n" );

	TListRequest listRequest; 
	TUint64 procId;
	
	TPtr8 listReqPtr( (TUint8 *)&listRequest, sizeof(TListRequest) );

	aMessage.ReadL( 0, listReqPtr );

	switch( listRequest.iListType )
		{
		    case TListRequest::EThreadList :
				{
				procId = MAKE_TUINT64( listRequest.iSubId2, listRequest.iSubId1 );				
				iDataSource->GetThreadListL( procId, iThreadPointerList, iTotalThreadListDescSize );
	
				listRequest.iRemaining = iThreadPointerList.Count();
	
	            //mark those that we observe
	            for(TInt i = 0; i < iObservationList.Count(); i++)
	                {
	                for(TInt j = 0; j < iThreadPointerList.Count(); j++)
	                    {
	                    if(iObservationList[i]->HasThread(iThreadPointerList[j]->Name()))
	                        {
	                        iThreadPointerList[j]->Observed(ETrue); 
	                        }
	                    }
	                }
	
				if( NULL != iMarshalledThreadList )
					{
					delete iMarshalledThreadList;
					}
	
				//LOG_MSG( " EThreadList -> MarshalDataL()\n" );
				iMarshalledThreadList = MarshalListL( listRequest, (TAny*) &iThreadPointerList );
				listRequest.iRequiredDescriptorSize = iMarshalledThreadList->Des().Size();
				iTotalThreadListDescSize = listRequest.iRequiredDescriptorSize;
								
				break;
				}
		case TListRequest::EProcessList :
            {
			iDataSource->GetProcessListL( iProcessPointerList, iTotalProcessListDescSize );

			listRequest.iRemaining = iProcessPointerList.Count();

            //mark those that we observe
            for(TInt i = 0; i < iObservationList.Count(); i++)
                {
                for(TInt j = 0; j < iProcessPointerList.Count(); j++)
                    {
                    if( (iObservationList[i]->TargetName() == iProcessPointerList[j]->Name()) && //attached to it
                        (iObservationList[i]->ThreadCount() == 0) ) //and no threads on the list 
                        {
                        iProcessPointerList[j]->Observed(ETrue); 
                        }
                    }
                }

			if( NULL != iMarshalledProcessList )
				{
				delete iMarshalledProcessList;
				}

			//LOG_MSG( " EProcessList -> MarshalDataL()\n" );
			iMarshalledProcessList = MarshalListL( listRequest, (TAny*) &iProcessPointerList );
			listRequest.iRequiredDescriptorSize = iMarshalledProcessList->Des().Size();
			iTotalProcessListDescSize = listRequest.iRequiredDescriptorSize;
						
			break;
			}			
		case TListRequest::EExecutableList :
            {
			iDataSource->GetExecutableListL( iExecutablePointerList, iTotalExecutableListDescSize );

			listRequest.iRemaining = iExecutablePointerList.Count();
			//LOG_MSG2( "  listRequest.iRemaining=%d\n", listRequest.iRemaining );

            //mark those that we observe
            for(TInt i = 0; i < iObservationList.Count(); i++)
                {
                for(TInt j = 0; j < iExecutablePointerList.Count(); j++)
                    {
                    if( iObservationList[i]->TargetName() == iExecutablePointerList[j]->Name() )//attached to it
                        {
                        iExecutablePointerList[j]->Observed(ETrue); 
                        }
                    }
                }

			if( NULL != iMarshalledExecutableList )
				{
				delete iMarshalledExecutableList;
				}

			//LOG_MSG( " EExecutableList -> MarshalDataL()\n" );
			iMarshalledExecutableList = MarshalListL( listRequest, (TAny*) &iExecutablePointerList );
			listRequest.iRequiredDescriptorSize = iMarshalledExecutableList->Des().Size();
			iTotalExecutableListDescSize = listRequest.iRequiredDescriptorSize;
			
			break;
			}		
		case TListRequest::EFormatterList :
            {
			listRequest.iRemaining = iFormatterInfos.Count();

            if(iMarshalledFormatterList)
                {
                delete iMarshalledFormatterList;
                }

            iMarshalledFormatterList = MarshalListL( listRequest, (TAny*) &iFormatterInfos );
			listRequest.iRequiredDescriptorSize = iMarshalledFormatterList->Des().Size();
			iTotalFormatterListDescSize = listRequest.iRequiredDescriptorSize;
			
            break;
            }
		case TListRequest::EWriterList :
            {
			listRequest.iRemaining = iWriterInfos.Count();

            if(iMarshalledWriterList)
                {
                delete iMarshalledWriterList;
                }

            iMarshalledWriterList = MarshalListL( listRequest, (TAny*) &iWriterInfos );
			listRequest.iRequiredDescriptorSize = iMarshalledWriterList->Des().Size();
			iTotalWriterListDescSize = listRequest.iRequiredDescriptorSize;
			
            break;
            }
		case TListRequest::ECrashList :
			{
			//Dealing with listing crashes from flash. Refresh the list and then
			//serialise the list onto iMarshalledCrashList
			
			RefreshCrashListFromFlashL();			
			listRequest.iRemaining = iCrashList.Count();
			
			if(iMarshalledCrashList)
				{
				delete iMarshalledCrashList;
				iMarshalledCrashList = NULL;
				}
			
			iMarshalledCrashList = MarshalListL(listRequest, (TAny*)&iCrashList);
			listRequest.iRequiredDescriptorSize = iMarshalledCrashList->Des().Size();
			iTotalCrashInfoDescSize = listRequest.iRequiredDescriptorSize;
			
			break;
			}
		default :
			{
			LOG_MSG2( " ERROR !* : CCoreDumpSession::ListInfoL() : invalid TListRequestType %d\n", listRequest.iListType );			
			User::Leave( KErrArgument );
			}
		}

	aMessage.WriteL( 0, listReqPtr );

	}

/**
 * This method looks at the crash logs in the flash partition
 * and caches them
 */
void CCoreDumpSession::RefreshCrashListFromFlashL()
	{	
	LOG_MSG("CCoreDumpSession::RefreshCrashListFromFlashL()");
	
	TBuf8<Debug::KMaxCoreHeaderSize> buf;	
	iCrashList.ResetAndDestroy();
	
	//Start looking where the config ends if it exists
	TInt crashLogPos = 0;
	SCMConfiguration* config = FlashDataSource()->GetSCMConfigFromFlashL();
	if(config)
		{
		crashLogPos = config->GetSize();
		}
	
	delete config;
	
	do
		{
		LOG_MSG3("Looking for crash at [%d]   [0x%X]", crashLogPos, crashLogPos);

		//Read in the 2 headers and the context, while we are in the crash log		
		TInt err = iSecSess.ReadCrashLog(crashLogPos, buf, Debug::KMaxCoreHeaderSize);
		if(err != KErrNone)
			{
			if(err == KErrPermissionDenied)
				{
				User::Leave(err);
				}
			
			//We have reached the end of the crash partition
			return;
			}
		
		TCrashLogWalker wlk(buf);
					
		if(KErrNone == wlk.ReadLogHeader(0))
			{
			LOG_MSG("Found a valid crash");
			const TCrashInfoHeader hdr = wlk.GetCrashHeader();
			TCrashInfo* inf = TCrashInfo::NewL(hdr);
			inf->iCrashSource = TCrashInfo::ESystemCrash;
			inf->iContext = wlk.GetCrashContext();
			
			iCrashList.Append(inf);
			
			//Increment this so it goes up by the log size AND then enough to align it to a flash block
			TUint32 flashPadding = 0x20000 - (crashLogPos + hdr.iLogSize)%0x20000;
			crashLogPos += (hdr.iLogSize + flashPadding);
			}
		else
			{			
			//otherwise we dont have any more crashes			
			return;
			}
		
		}
	while(true);		
	
	}


/**
 * Processes the crash log in the flash partition corrosponding to the crash ID of the parameter 
 * passed through. If this doesnt match a crash in the partition, or the flash cannot be read 
 * this will leave with KErrCorrupted
 * @param aMessage
 * @leave one of the OS wide codes
 */
void CCoreDumpSession::ProcessCrashLogL(const RMessage2& aMessage)
	{
	TInt crashId = aMessage.Int0();	
	LOG_MSG2("->CCoreDumpSession::ProcessCrashLogL(ID = [%d])\n",  crashId);
	
	RefreshCrashListFromFlashL();
	
	for(TInt cnt = 0; cnt < iCrashList.Count(); cnt++)
		{
		if(iCrashList[cnt]->iCrashId == crashId)
			{
			RDebug::Printf("Going to the crash handler");
			iCrashHandler->HandleCrashFromFlashL(*(iCrashList[cnt]));
			}
		}
	}

/**
 * Processes the crash log in the flash partition Asynchronously corrosponding to the crash ID of the parameter 
 * passed through. It refreshes the crash list and then calls the crash handler. Leaves with standard leave codes 
 * when the underlying methods leave.  
 * @param aMessage
 * @leave one of the OS wide codes
 */
void CCoreDumpSession::ProcessCrashLogAsyncL(const RMessage2& aMessage)
	{
	TInt crashId = aMessage.Int0();	
	LOG_MSG2("->CCoreDumpSession::ProcessCrashLogAsyncL(ID = [%d])\n",  crashId);
	
	RefreshCrashListFromFlashL();
	
	for(TInt cnt = 0; cnt < iCrashList.Count(); cnt++)
		{
		if(iCrashList[cnt]->iCrashId == crashId)
			{
			LOG_MSG("Going to the crash handler using the async mechanism");
			iCrashHandler->HandleCrashFromFlashL(*(iCrashList[cnt]), aMessage);
			}
		}
	}

/**
 * Cancels asynchronous processing of the crash log in the flash partition  
 * @param aMessage
 */
void CCoreDumpSession::CancelProcessCrashLogAsync(const RMessage2& aMessage)
	{
	TInt crashId = aMessage.Int0();	
	LOG_MSG2("->CCoreDumpSession::CancelProcessCrashLogAsyncL(ID = [%d])\n",  crashId);
	
	for(TInt cnt = 0; cnt < iCrashList.Count(); cnt++)
		{
		if(iCrashList[cnt]->iCrashId == crashId)
			{
			LOG_MSG("Going to the cancel async crash handler processing");
			iCrashHandler->CancelHandleCrashFromFlash(*(iCrashList[cnt]));
			}
		}
	}

/**
 * From a crash ID gets a TCrashInfo
 * @param aCrashId crash ID to search for
 * @param aCrashInfo the crash info for supplied crash ID
 * @leave one of the OS wide codes
 */
void CCoreDumpSession::GetCrashInfoL(TInt aCrashId, TCrashInfo& aCrashInfo)
	{
	for(TInt i = 0; i<iCrashList.Count(); i++)
		{
		if(iCrashList[i]->iCrashId == aCrashId)
			{
			aCrashInfo = *(iCrashList[i]);
			return;
			}
		}
	
	User::Leave(KErrNotFound);
	}

/**
    Retrives the 'cancel crash' property value and checks if required to abort the crash dump. Updates 'crash progress' property. 

@param aProgress descriptor with the crash progress value.
@leave KErrAbort if crash cancel property has been set
 */
void CCoreDumpSession::UpdateProgressL(const TDesC &aProgress) const
    {
    LOG_MSG("->CCoreDumpSession::UpdateProgressL()\n");
    TInt cancelCrash = EFalse;
    TInt err = RProperty::Get(KCoreDumpServUid, ECrashProgress, cancelCrash);
    if(err != KErrNone)
        {
        LOG_MSG2("CCoreDumpSession::UpdateProgressL - unable to retrive 'ECrashProgress' value! err:%d\n", err);
        }

    if(cancelCrash)
        {
        LOG_MSG("CCoreDumpSession::UpdateProgressL - aborting dump in progress\n");
        User::Leave(KErrNotReady);
        }

    err = RProperty::Set(KCoreDumpServUid, ECrashProgress, aProgress);
    if(err != KErrNone)
        {
        LOG_MSG2("CCoreDumpSession::UpdateProgressL - unable to retrive 'crash progress' value! err:%d\n", err);
        }
#ifdef DEBUG 
    User::After(1000000);
#endif
  }

/**
 * This method removes a crash log from the flash partition
 * @param aMessage
 * @leave one of the OS wide codes
 */
void CCoreDumpSession::DeleteCrashLogL(const RMessage2& aMessage) 
	{
	User::Leave(KErrNotSupported);
	}

void CCoreDumpSession::DeleteCrashPartitionL()
	{
	LOG_MSG("CCoreDumpSession::DeleteCrashPartitionL()");
	
	UpdateProgressL(_L("Erasing Entire Log"));

	//Read in config first
	SCMConfiguration* config = FlashDataSource()->GetSCMConfigFromFlashL();
	TCleanupItem scmCleanup(CCoreDumpSession::CleanupSCMConfiguration, (TAny*)config);
	if(config)
		CleanupStack::PushL(scmCleanup);
	
	TInt err = iSecSess.EraseCrashFlashPartition();	
	User::LeaveIfError(err);	
	
	if(config)
		{
		//Now we rewrite the config
		RBuf8 data;	
		data.CreateL(config->GetSize());
		data.SetLength(config->GetSize());
		data.CleanupClosePushL();
			
		TByteStreamWriter writer(const_cast<TUint8*>(data.Ptr()), EFalse);	
		config->Serialize(writer);
			
		TUint32 written = 0;
		User::LeaveIfError(iSecSess.WriteCrashConfig(0, data, written));
		
		CleanupStack::PopAndDestroy(2);//data and scmCleanup
		}
	
	RefreshCrashListFromFlashL();	

	UpdateProgressL(_L("Idle"));
	}

void CCoreDumpSession::CreateWriterL( const TUid loadRequestUid )
	{
	LOG_MSG("->CCoreDumpSession::CreateWriterL()\n" );

	ValidatePluginL( loadRequestUid );

    if(iPluginList.Count() == 0)
        {
        RefreshPluginListL();
        }

    CPluginInfo *info = NULL;
    for(TInt i = 0; i < iPluginList.Count(); i++)
        {
        if(iPluginList[i].iUid == loadRequestUid)
            {
                info = CPluginInfo::NewL(iPluginList[i].iName,
                                         loadRequestUid.iUid,
                                         iPluginList[i].iVersion,
                                         TPluginRequest::EWriter);        
                CleanupStack::PushL(info);
                break;
            }
        }

    if(!info)
        {
        LOG_MSG2("CCoreDumpSession::CreateWriterL - unable to find writer uid:%d on plugin type list!\n", loadRequestUid);
        User::Leave(KErrArgument); 
        }

    iWriterInfos.AppendL(info);
    CleanupStack::Pop(info);
    
	CCrashDataSave *writer = CCrashDataSave::NewL( (TUid)( loadRequestUid ) );

    CleanupStack::PushL(writer);
    iWriterPlugins.AppendL(writer);
    CleanupStack::Pop(writer);

    LOG_MSG3("CCoreDumpSession::CreateFormatter fmt:%d, wrt:%d\n", iFormatterPlugins.Count(), iWriterPlugins.Count());
    //for compatibility, first pair is bound by default
    if( (iFormatterPlugins.Count() == 1) && (iWriterPlugins.Count() == 1) )
        {
	        LOG_MSG("CCoreDumpSession::CreateFormatter binding default plugin pair\n" );
            iFormatterInfos[0]->Pair(0);
            iFormatterPlugins[0]->ConfigureDataSaveL(iWriterPlugins[0]);
        }
	}

void CCoreDumpSession::CreateFormatterL( const TUid loadRequestUid )
	{	
	LOG_MSG( "->CCoreDumpSession::CreateFormatter()\n" );

	ValidatePluginL( loadRequestUid );
	
	if(iPluginList.Count() == 0)
		{		
		RefreshPluginListL();
		}

    CPluginInfo *info = NULL;
    for(TInt i = 0; i < iPluginList.Count(); i++)
        {    	
        if(iPluginList[i].iUid == loadRequestUid)
            {
                info = CPluginInfo::NewL(iPluginList[i].iName,
                                         loadRequestUid.iUid,
                                         iPluginList[i].iVersion,
                                         TPluginRequest::EFormatter);        
                CleanupStack::PushL(info);
                break;
            }
        }

    if(!info)
        {
        LOG_MSG2("CCoreDumpSession::CreateFormatterL - unable to find formatter uid:%X on plugin type list!\n", loadRequestUid);
        User::Leave(KErrArgument);
        }

    iFormatterInfos.AppendL(info);
    CleanupStack::Pop(info);

	CCoreDumpFormatter *formatter = CCoreDumpFormatter::NewL( (TUid)( loadRequestUid ) );

    CleanupStack::PushL(formatter);
    iFormatterPlugins.AppendL(formatter);
    CleanupStack::Pop(formatter);

    LOG_MSG3("CCoreDumpSession::CreateFormatter fmt:%d, wrt:%d\n", iFormatterPlugins.Count(), iWriterPlugins.Count());
    //for compatibility, first pair is bound by default
    if( (iFormatterPlugins.Count() == 1) && (iWriterPlugins.Count() == 1) )
        {
	        LOG_MSG("CCoreDumpSession::CreateFormatter binding default plugin pair\n" );
            iFormatterInfos[0]->Pair(0);
            iFormatterPlugins[0]->ConfigureDataSaveL(iWriterPlugins[0]);
        }
	}

void CCoreDumpSession::DeleteFormatterL( const TUid aFreeRequestUid, const TUint aIndex )
{
    LOG_MSG("->CCoreDumpSession::DeleteFormatterL()\n");
    if( (iFormatterInfos.Count() > aIndex) && (iFormatterInfos[aIndex]->Uid() == aFreeRequestUid.iUid) )
    {
        delete iFormatterPlugins[aIndex];
        iFormatterPlugins.Remove(aIndex);

        delete iFormatterInfos[aIndex];
        iFormatterInfos.Remove(aIndex);
    }
    else
    {
        User::Leave(KErrArgument);
    }
}

void CCoreDumpSession::DeleteWriterL( const TUid aFreeRequestUid, const TUint aIndex )
{
    LOG_MSG("->CCoreDumpSession::DeleteWriterL()\n");
    if( (iWriterInfos.Count() > aIndex) && (iWriterInfos[aIndex]->Uid() == aFreeRequestUid.iUid) )
    {
        for(TInt i = 0; i < iFormatterInfos.Count(); i++)
        {
            if(iFormatterInfos[i]->Pair() == aIndex)
            {
                iFormatterInfos[i]->Pair(KMaxTUint32);
            }
        }

        delete iWriterPlugins[aIndex];
        iWriterPlugins.Remove(aIndex);

        delete iWriterInfos[aIndex];
        iWriterInfos.Remove(aIndex);
    }
    else
    {
        User::Leave(KErrArgument);
    }
}

void CCoreDumpSession::PluginRequestL(const RMessage2& aMessage)
{
	LOG_MSG("->CCoreDumpSession::PluginRequestL()\n");

	TPluginRequest request;
	TPtr8 pluginReqPtr((TUint8 *)&request, sizeof(TPluginRequest));
	aMessage.ReadL(0, pluginReqPtr);
	
    if(request.iUid.iUid != 0) //(un)load a plugin
    { 
        if(request.iPluginType == TPluginRequest::EFormatter)
        {
            if(request.iLoad)
            {
                CreateFormatterL(request.iUid);
            }
            else
            {
                //index mapping compatibility  - one plugin at a time case
                TUint index = (request.iIndex == request.iPair) ? request.iIndex : 0;
                DeleteFormatterL(request.iUid, index);
            }
        }
        else if(request.iPluginType == TPluginRequest::EWriter)
        {
            if(request.iLoad)
            {
                CreateWriterL(request.iUid);
            }
            else
            {
                //index mapping compatibility - one plugin at a time case
                TUint index = (request.iIndex == request.iPair) ? request.iIndex : 0;
                DeleteWriterL(request.iUid, index);
            }
        }
        else
        {
            LOG_MSG2("CCoreDumpSession::PluginRequestL() - invalid plugin type:%d!\n", request.iPluginType);
            User::Leave(KErrNotSupported);
        }
	    UpdateConfigParametersL();
    }
    else //bind a plugin pair
    {
        BindPluginsL(request.iIndex, request.iPair);
    }
}

void CCoreDumpSession::BindPluginsL(const TUint aFormatterIndex, const TUint aWriterIndex)
{
   LOG_MSG3("->CCoreDumpSession::BindPluginsL(fmt=%d, wrt=%d)\n", aFormatterIndex, aWriterIndex);
   if( (aFormatterIndex < iFormatterInfos.Count()) && (aWriterIndex < iWriterInfos.Count()) )
   {
        iFormatterInfos[aFormatterIndex]->Pair(aWriterIndex);
        iFormatterPlugins[aFormatterIndex]->ConfigureDataSaveL(iWriterPlugins[aWriterIndex]);
   }
   else
   {
        LOG_MSG5("CCoreDumpSession::BindPluginsL -> fmtIndex:%d/%d wrtIndex:%d/%d\n", aFormatterIndex, iFormatterInfos.Count(),
                                                                                     aWriterIndex, iWriterInfos.Count());
        User::Leave(KErrNotFound);
   }
}

CCoreDumpFormatter* CCoreDumpSession::GetValidFormatter(const TUint aCount)
{
    LOG_MSG2("->CCoreDumpSession::ValidFormatter(aCount=%d)", aCount);
    TUint counter = 0;

    for(TInt i = 0; i < iFormatterInfos.Count(); i++)
    {
        LOG_MSG3("CCoreDumpSession::ValidFormatter() - fmt[%d]->pair=%d", i, iFormatterInfos[i]->Pair());
        if( (iFormatterInfos[i]->Pair() < iWriterInfos.Count()) && ( counter++ == aCount))
        {
            LOG_MSG2("CCoreDumpSession::ValidFormatter() - found valid formatter=%d", iFormatterPlugins[i]);
            return iFormatterPlugins[i];
        }
    }
    return NULL;
}

static void CleanupEComArray(TAny* aArray)
{
	(static_cast<RImplInfoPtrArray*> (aArray))->ResetAndDestroy();
	(static_cast<RImplInfoPtrArray*> (aArray))->Close();
}

void CCoreDumpSession::RefreshPluginListL()
{
	LOG_MSG("->CCoreDumpSession::RefreshPluginList()\n");

	iPluginList.Close();
	TPluginInfo ref;

	RImplInfoPtrArray infoArray;
	TCleanupItem cleanup(CleanupEComArray, &infoArray);
	CleanupStack::PushL(cleanup);
    
	CCoreDumpFormatter::ListAllImplementationsL( infoArray );

	for ( TInt i = 0; i < infoArray.Count(); i++ )
		{
		ref.iVersion = infoArray[i]->Version();
		ref.iName = infoArray[i]->DisplayName();
		ref.iUid = infoArray[i]->ImplementationUid();
		ref.iType = TPluginRequest::EFormatter;

        ref.iLoaded = EFalse; //iLoaded on this list means we have an instance of that plugin
        for(TInt j = 0; j < iFormatterInfos.Count(); j++)
        {
            if(iFormatterInfos[j]->Uid() == ref.iUid.iUid)
            {
                ref.iLoaded = ETrue;
                break;
            }
        }

		iPluginList.AppendL(ref);
		}
	
	infoArray.ResetAndDestroy();

	CCrashDataSave::ListAllImplementationsL( infoArray );

	for ( TInt i = 0; i < infoArray.Count(); i++ )
		{
		ref.iVersion = infoArray[i]->Version();
		ref.iName = infoArray[i]->DisplayName();
		ref.iUid = infoArray[i]->ImplementationUid();
		ref.iType = TPluginRequest::EWriter;

        ref.iLoaded = EFalse; //iLoaded on this list means we have an instance of that plugin
        for(TInt j = 0; j < iWriterInfos.Count(); j++)
        {
            if(iWriterInfos[j]->Uid() == ref.iUid.iUid)
            {
                ref.iLoaded = ETrue;
                break;
            }
        }

		iPluginList.AppendL(ref);
		}

	CleanupStack::PopAndDestroy();
}

void CCoreDumpSession::GetPluginListL(const RMessage2& aMessage)
	{
	//LOG_MSG("->CCoreDumpSession::GetPluginList()\n");

	TListRequest listRequest; 

	TPtr8 pluginReqPtr((TUint8 *)&listRequest, sizeof(TListRequest));

	//read target debug app address and size from debug agent
	aMessage.ReadL(0, pluginReqPtr);

	TPluginInfoBlock * plugins = new (ELeave) TPluginInfoBlock;
    CleanupStack::PushL(plugins);

	if( 0 == listRequest.iIndex )
		{
		iPluginList.Reset();
		RefreshPluginListL();
		}

	TUint toSupply = iPluginList.Count() - listRequest.iIndex;
	if ( toSupply > KNumPluginDetails )
		{
		toSupply = KNumPluginDetails;
		}

	for( TUint index = 0; index < toSupply; index ++ )
		{
		plugins->plugins[index].iVersion		= iPluginList[index+listRequest.iIndex].iVersion;
		plugins->plugins[index].iName			= iPluginList[index+listRequest.iIndex].iName;
		plugins->plugins[index].iDescription	= iPluginList[index+listRequest.iIndex].iDescription;
		plugins->plugins[index].iUid			= iPluginList[index+listRequest.iIndex].iUid;
		plugins->plugins[index].iType			= iPluginList[index+listRequest.iIndex].iType;
		plugins->plugins[index].iLoaded			= iPluginList[index+listRequest.iIndex].iLoaded;
		//RDebug::Print( _L("  loaded[%d]=%d\n"), index, plugins->plugins[index].iLoaded );
		}

	listRequest.iSupplied = toSupply;
	listRequest.iRemaining = iPluginList.Count() - (listRequest.iIndex + listRequest.iSupplied);

	//RDebug::Print( _L("  supplying=%d, remaining=%d\n"), toSupply, listRequest.iRemaining );

	aMessage.WriteL(0, pluginReqPtr);
	TPtr8 pluginArrPtr( (TUint8 *)plugins, sizeof(TPluginInfoBlock) );
	pluginArrPtr.Set( (TUint8 *)plugins, sizeof(TPluginInfoBlock), sizeof(TPluginInfoBlock) );

	aMessage.WriteL(1, pluginArrPtr);
    CleanupStack::PopAndDestroy(plugins);
	}

void CCoreDumpSession::ObservationRequestL( const RMessage2& aMessage )
    {
	//RDebug::Print(_L("->CCoreDumpSession::ObservationRequestL()\n"));
    TInt deslen;
    deslen = aMessage.GetDesLengthL(0);
	if (deslen == 0)
		{
		// Zero-length target name supplied
		User::Leave(KErrArgument);
		}

	RBuf targetName;
	targetName.CleanupClosePushL();
	targetName.CreateL(deslen);
	// Read the target name into targetName
	aMessage.ReadL(0, targetName);

    deslen = aMessage.GetDesLengthL(1);
	if (deslen == 0)
		{
		// Zero-length target owner name supplied
		User::Leave(KErrArgument);
		}

	RBuf targetOwnerName;
	targetOwnerName.CleanupClosePushL();
	targetOwnerName.CreateL(deslen);
	// Read the target owner name into targetOwnerName
	aMessage.ReadL(1, targetOwnerName);

    TBool observe = static_cast<TBool>(aMessage.Int2());
    if(observe)
        {
        LOG_MSG("CCoreDumpSession::ObservationRequestL - KAttach\n" );
        AttachTargetL(targetName, targetOwnerName);
        }
    else
        {
        LOG_MSG("CCoreDumpSession::ObservationRequestL - KDetach\n");
        DetachTargetL(targetName, targetOwnerName);
        }

    CleanupStack::PopAndDestroy(&targetOwnerName);
    CleanupStack::PopAndDestroy(&targetName);
    }

void CCoreDumpSession::AttachTargetL( const TDesC &aTargetName, const TDesC &aTargetOwnerName )
    {
	//LOG_MSG("->CoreDumpSession::AttachTargetL()\n");
    TBool found = EFalse;

    for(TInt i = 0; i < iObservationList.Count(); ++i)
        {
        if(iObservationList[i]->TargetName() == aTargetOwnerName)
            {
            if(aTargetName != aTargetOwnerName) //attach thread
                {
                iObservationList[i]->AddThreadL(aTargetName);
                //LOG_MSG("CoreDumpSession::AttachTargetL - thread added\n");
                }
            else
                {
                LOG_MSG("CoreDumpSession::AttachTargetL - process added\n");
                iObservationList[i]->ClearThreads();
                } 
            found = ETrue;
            break;
            }
        }

    if(!found)
        {
        //RDebug::Printf( "CoreDumpSession::AttachTargetL - creating new observer", &aTargetOwnerName );
        CTargetObserver *observer = CTargetObserver::NewLC( iSecSess, *iCrashHandler, aTargetOwnerName ); 
        User::LeaveIfError( iObservationList.Insert(observer, 0) );
        
        if(aTargetName != aTargetOwnerName) //attach thread
            {
            observer->AddThreadL(aTargetName);
            //LOG_MSG("CoreDumpSession::AttachTargetL - thread added\n");
            }

        if( (iPreCrashEventAction & ESuspendThread) || 
			(iPreCrashEventAction & ESuspendProcess) )
			{
			LOG_MSG("CCoreDumpSession::AttachTargetL() ->SetCrashEventsL(EActionSuspend)\n" );
            SetCrashEventsL(*observer, EActionSuspend);
			}
        else
			{
			LOG_MSG("CCoreDumpSession::AttachTargetL() ->SetCrashEventsL(EActionContinue)\n" );
            SetCrashEventsL(*observer, EActionContinue);
			}

        observer->Observe();

	    //LOG_MSG("CoreDumpSession::AttachTargetL - observer started\n");
        CleanupStack::Pop(); //observer is now in the iObservationList 
        }
    }

void CCoreDumpSession::SetCrashEventsL(CTargetObserver &aObserver, TKernelEventAction aAction)
{
	LOG_MSG2("CCoreDumpSession::SetCrashEventsL(aAction=%d)\n", aAction);

    if(iCrashEventTypes & ECrashHwExc)
		{
		LOG_MSG2("  aObserver.SetCrashEventL(EEventsHwExc, aAction=%d)\n", aAction);
        aObserver.SetCrashEventL(EEventsHwExc, aAction);
		}
    else
		{
		LOG_MSG("  aObserver.SetCrashEventL(EEventsHwExc, EActionIgnore)\n");
        aObserver.SetCrashEventL(EEventsHwExc, EActionIgnore);
		}

    if(iCrashEventTypes & ECrashKillThread)
		{
		LOG_MSG2("  aObserver.SetCrashEventL(EEventsKillThread, aAction=%d)\n", aAction);
        aObserver.SetCrashEventL(EEventsKillThread, aAction);
		}
    else
		{
		LOG_MSG("  aObserver.SetCrashEventL(EEventsKillThread, EActionIgnore)\n");
        aObserver.SetCrashEventL(EEventsKillThread, EActionIgnore);
		}
}

void CCoreDumpSession::DetachTargetL( const TDesC &aTargetName, const TDesC &aTargetOwnerName )
    {
	//LOG_MSG("->CoreDumpSession::DetachTargetL()\n");
    TBool found = EFalse;

    RBuf ton;
    ton.CreateL(aTargetName);
    char *tonp = (char*) ton.Collapse().PtrZ();
    LOG_MSG2("CCoreDumpSession::DetachTargetL(%s)\n", tonp);
    ton.Close();

    RBuf otn;
    TInt count = iObservationList.Count();
    for(TInt i = 0; i < count; ++i)
        {
        otn.CreateL(iObservationList[i]->TargetName());
        char* otnp = (char*) otn.Collapse().PtrZ();
        LOG_MSG3("CCoreDumpSession::DetachTargetL - iObservationList[%d]->TargetName=%s\n", i, otnp);
        otn.Close();
        if (iObservationList[i]->TargetName() == aTargetOwnerName)
            {
            if(aTargetName != aTargetOwnerName) //detach thread
                {
                iObservationList[i]->DelThreadL(aTargetName);
                LOG_MSG("CoreDumpSession::DetachTargetL - thread removed\n");
                }
            if(iObservationList[i]->ThreadCount() == 0) //no more threads to observe or detach process
                {
                LOG_MSG("CoreDumpSession::DetachTargetL - destroying observer\n");
                delete iObservationList[i]; 
                iObservationList.Remove(i);
                }
            found = ETrue;
            break;
            }
        }

    if(!found)
        {
        LOG_MSG("CoreDumpSession::DetachTargetL - target not found\n");
        User::Leave(KErrNotFound);
        }
    }

void CCoreDumpSession::UpdateConfigParametersL()
{
	LOG_MSG( "->CCoreDumpSession::UpdateConfigParametersL()\n" );

    COptionConfig *config = NULL;
    iTotalConfigList.Reset();
    TUint configParamSize;
    
    TInt paramNum = iConfigList.Count();
	for(TInt i = 0; i < paramNum; i++ )
		{
        config = iConfigList[i];
        
        if(!config)
            {
            User::Leave(KErrBadHandle);
            }

        configParamSize = config->Size();
        if(configParamSize > iMaxConfigParamSize)
            {
            iMaxConfigParamSize = configParamSize ;
            }
        iTotalConfigList.AppendL(config);
		}


    for(TInt i = 0; i < iFormatterPlugins.Count(); i++)
        {
        paramNum = iFormatterPlugins[i]->GetNumberConfigParametersL();
        for(TInt index = 0; index < paramNum; index++)
            {
            config = iFormatterPlugins[i]->GetConfigParameterL(index);
            if(!config)
                {
                User::Leave(KErrBadHandle);
                }

            configParamSize = config->Size();
            if(configParamSize > iMaxConfigParamSize)
                {
                iMaxConfigParamSize = configParamSize ;
                }

            config->Instance(i); 
            iTotalConfigList.AppendL(config);
            }
        }

    for(TInt i = 0; i < iWriterPlugins.Count(); i++)
        {
        paramNum = iWriterPlugins[i]->GetNumberConfigParametersL();
        for(TInt index = 0; index < paramNum; index++)
            {
            config = iWriterPlugins[i]->GetConfigParameterL(index);
            if(!config)
                {
                User::Leave(KErrBadHandle);
                }

            configParamSize = config->Size();
            if(configParamSize > iMaxConfigParamSize)
                {
                iMaxConfigParamSize = configParamSize ;
                }

            config->Instance(i); 
            iTotalConfigList.AppendL(config);
            }
        }
    
    
	TRAPD(err, iFlashDataSource->ReadSCMConfigL(iScmConfigList));
	if(err != KErrNone)
		{
		LOG_MSG("Unable to retrieve config params for the SCM: [%d]");
		return;
		}
	
    for(TInt i=0;i<iScmConfigList.Count();i++)
    	{
    	iTotalConfigList.AppendL(iScmConfigList[i]);
    	} 
	}

/**
 * This logs the current SCM Config to serial 
 */
void CCoreDumpSession::PrintScmConfigL()
	{	
    for(TInt i=0;i<iScmConfigList.Count();i++)
    	{
    	COptionConfig* optCon = iScmConfigList[i];
    	LOG_DES(optCon->Prompt());
    	LOG_MSG2("\t\tHas a value of %d", optCon->Value());
    	} 
	}

// Gather all conifg params from CDS, formatter and iWriter, 
// collent into an array and set num to the number of configs avaliable
void CCoreDumpSession::GetNumberConfigParametersL( const RMessage2& aMessage )
	{
	//LOG_MSG( "->CCoreDumpSession::GetNumberConfigParametersL()\n" );
	 UpdateConfigParametersL( );

	TInt numConfigParams = iTotalConfigList.Count();
	//LOG_MSG2( "  numConfigParams=%d", numConfigParams );

	TPtr8 numParamPtr( (TUint8 *) & numConfigParams, sizeof(TInt) );
	// Else it does not get passed back
	numParamPtr.Set( (TUint8 *)& numConfigParams, sizeof(TInt), sizeof(TInt) ); 

	TPtr8 maxParamSizePtr( (TUint8 *) & iMaxConfigParamSize, sizeof(TInt) );
	// Else it does not get passed back
	maxParamSizePtr.Set( (TUint8 *)& iMaxConfigParamSize, sizeof(TInt), sizeof(TInt) ); 

	/*
	RDebug::Printf( " CCoreDumpSession::GetNumberConfigParameters(): iConfigList.Count()=%d, passing back nParams=%d, iMaxConfigParamSize=%d \n", 
		iConfigList.Count(), numConfigParams, iMaxConfigParamSize );
	*/

	aMessage.WriteL( 0, numParamPtr );

	aMessage.WriteL( 1, maxParamSizePtr );
	}

void CCoreDumpSession::GetConfigParameterL( const RMessage2& aMessage )
	{
//	LOG_MSG( "->CCoreDumpSession::GetConfigParameterL()\n" );


	if(	iTotalConfigList.Count() == 0 )
		{
		// The user has never called GetNumberConfigParametersL(), 
		// so update the parameters first.
		UpdateConfigParametersL( );
		}

	if( iTotalConfigList.Count() == 0 )
		{
		RDebug::Printf( "  ERROR !* : Have not gathered the configuration parameters\n" );
		User::Leave( KErrBadHandle );
		}

	TInt configIndex = aMessage.Int0();
	//RDebug::Printf( "  configIndex=%d\n", configIndex );

	if ( ( configIndex < 0 ) || ( configIndex >= iTotalConfigList.Count() ) )
		{
		RDebug::Printf( "  ERROR !* : config Index out of bounds\n" );
		User::Leave( KErrBadHandle );
		}

	COptionConfig * config = iTotalConfigList[configIndex];
	
	HBufC8 * configDes = config->MarshalDataL();
	CleanupStack::PushL( configDes );
	
	if ( configDes == NULL )
		{
		RDebug::Printf( "  ERROR !* MarshalData returned null buf\n" );
		User::Leave( KErrBadDescriptor );
		}
	else
		{
		TPtr8 configPtr( configDes->Des() );
		if( configPtr.Size() == 0 )
			{
			RDebug::Printf( "  ERROR !* : if( configPtr.Size() == 0 )\n" );
			User::Leave( KErrBadDescriptor );
			}

		aMessage.WriteL( 1, configPtr );
		}

	CleanupStack::PopAndDestroy( configDes );
	}

void CCoreDumpSession::SetConfigParameterL( const RMessage2& aMessage )
	{
	LOG_MSG( "->CCoreDumpSession::SetConfigParameterL()\n" );

	TConfigRequest configRequest; 
	
	TPtr8 configReqPtr( (TUint8 *)&configRequest, sizeof(TConfigRequest) );
	aMessage.ReadL( 0, configReqPtr );

	TInt32 value = aMessage.Int1();

	HBufC *valueDesc = NULL;
	TInt valueDesLen = aMessage.GetDesLength( 2 );
	if( valueDesLen > 0 )
		{
		valueDesc = HBufC::NewL( valueDesLen );
		CleanupStack::PushL( valueDesc );
		TPtr ptr( valueDesc->Des() );
		aMessage.ReadL( 2, ptr );
		}
	else
		{
		valueDesc = KNullDesC().AllocL();
		CleanupStack::PushL( valueDesc );
		}

	switch(configRequest.iSource)
		{
		case COptionConfig::ECoreDumpServer:

			SetConfigParameterL( configRequest.iIndex, value, *valueDesc );
			break;

		case COptionConfig::EFormatterPlugin:
            {
                if(configRequest.iInstance < iFormatterPlugins.Count())
                    {
                    iFormatterPlugins[configRequest.iInstance]->SetConfigParameterL(configRequest.iIndex, value, *valueDesc); 
                    break;
                    }
			break;
            }
		case COptionConfig::EWriterPlugin:
            {	
                if(configRequest.iInstance < iWriterPlugins.Count())
                    {
                    iWriterPlugins[configRequest.iInstance]->SetConfigParameterL(configRequest.iIndex, value, *valueDesc); 
                    break;
                    }
			break;
            }
		case COptionConfig::ESCMConfig:
			{
							
    		LOG_MSG4("(CCoreDumpSession::SetConfigParameterL) Updating config param instance %d  value %d index =%d"
    				, configRequest.iInstance , value, configRequest.iIndex);
    		iFlashDataSource->ModifySCMConfigItemL(configRequest.iInstance, value);
            break;
            }
            
		default:
            User::Leave(KErrArgument);
		}

	    CleanupStack::PopAndDestroy( valueDesc );
	}


TInt32 CCoreDumpSession::PreProcessingAction() const
	{
    return iConfigList[EPreCrashEventAction]->Value();
	}

TInt32 CCoreDumpSession::PostProcessingAction() const
	{
    return iConfigList[EPostCrashEventAction]->Value();
	}

void CCoreDumpSession::SetConfigParameterL( const TInt aIndex, 
										 const TInt32 & aValue, 
										 const TDesC & aDescValue )
	{
	LOG_MSG3( "->CCoreDumpSession::SetConfigParameterL( aIndex=%d, aValue=0x%X )\n", aIndex, aValue );

	if ( ( aIndex < 0 ) || ( aIndex >= iConfigList.Count() ) )
		{
		LOG_MSG2( "  ERROR !* : config Index out of bounds, max=%d\n", iConfigList.Count() - 1 );
		User::Leave( KErrBadHandle );
		}

	//Match the parameter passed in to one of our own parameters via the index
	COptionConfig &config = *iConfigList[aIndex];

	if( config.Index() != aIndex )
		{
		RDebug::Printf( "  ERROR !* : config Index %d does not match internal index %d\n", aIndex, config.Index() );
		User::Leave( KErrBadHandle );
		}

	if( aIndex == (TInt)EPreCrashEventAction )
	    {
		switch( aValue )
			{
			case ENoPreAction:
			case ESuspendThread:
			case ESuspendProcess:

				LOG_MSG2("  Changing EPreCrashEventAction to =%d\n", aValue );

				iPreCrashEventAction = aValue;

                for(TInt i = 0; i < iObservationList.Count(); i++)
                    {
                    if( (iPreCrashEventAction & ESuspendThread) || 
						(iPreCrashEventAction & ESuspendProcess) )
						{
						LOG_MSG2("  SetCrashEventsL(*iObservationList[%d], EActionSuspend)\n", i );
                        SetCrashEventsL(*iObservationList[i], EActionSuspend);
						}
                    else
						{
						LOG_MSG2("  SetCrashEventsL(*iObservationList[%d], EActionContinue);\n", i );
                        SetCrashEventsL(*iObservationList[i], EActionContinue);
						}
                    }
                
				break;
			default:
				RDebug::Printf(" ERROR !* EPreCrashEventAction cannot be %d\n", aValue );
				User::Leave(KErrArgument);
			}
	    }
	else if( aIndex == (TInt)EPostCrashEventAction )
	    {
		switch( aValue )
			{
			case ENoPostAction:
			case EResumeThread:
			case EResumeProcess:
			case EKillProcess:
				iPostCrashEventAction = aValue;
				break;
			default:
				RDebug::Printf(" ERROR !* EPostCrashEventAction cannot be %d\n", aValue );
				User::Leave(KErrArgument);
			}
	    }
    else if( aIndex == (TInt)ECrashEventTypes )
        {
		switch( aValue )
			{
			case ECrashHwExc:
			case ECrashKillThread:
			case ECrashHwExcAndKillThread:

				LOG_MSG2("  Changing trigger ECrashEventTypes to =%d\n", aValue );
				iCrashEventTypes = aValue;

				for(TInt i = 0; i < iObservationList.Count(); i++)
                    {
                    if( (iPreCrashEventAction & ESuspendThread) || 
						(iPreCrashEventAction & ESuspendProcess) )
                        SetCrashEventsL(*iObservationList[i], EActionSuspend);
                    else
                        SetCrashEventsL(*iObservationList[i], EActionContinue);
                    }

				break;
			default:
				RDebug::Printf( " ERROR !* ECrashEventTypes  cannot be %d\n", aValue );
				User::Leave(KErrArgument);
			}
        }
	else
		{
		RDebug::Printf(
			"CCoreDumpSession::SetConfigParameterL() : parameter with index %d does is not valid\n", 
				aIndex );
		User::Leave(KErrArgument);
		}

	// Change our internal values
	config.Value( aValue );
	//LOG_MSG( "  config->ValueL( aDescValue )\n" );
	config.ValueL( aDescValue );

	}



/**
Save one parameter for a specific section. 
@param aParam COptionConfig pointer to save
@param aSectName Section name in the file to save this parameter to
@param aSaveIndex This parameter is only used to create key strings that are unique.
@param aPersistConfigData Object used to save the configuration to. 
*/
void CCoreDumpSession::SaveConfigParamL( const COptionConfig * aParam, 
									   const TPtrC	&	aSectName,
									   const TInt		aSaveIndex,
									   CIniDocument16 * aPersistConfigData )
	{

	LOG_MSG2( "CCoreDumpSession::SaveConfigParamL( aSaveIndex=%d )\n", aSaveIndex );

	HBufC * iniStringKey;
	TInt err;

	// Save the Configuration Parameter Value
		{
		iniStringKey = GenerateKeyStringLC( aSaveIndex, EIniKeyValue );
		TInt keyIntValue = aParam->Value();

		// Size of character buffer required to represent a 32bit 
		// interger as decimal should be 10.
		TBuf<16> intValueBuf; 
				
		switch( aParam->Type() )
			{
			case COptionConfig::ETInt:
			case COptionConfig::ETUInt:
			case COptionConfig::ETBool:

				LOG_MSG2( "  -> SetKey( int value=%d)\n", keyIntValue );
				intValueBuf.AppendNum( keyIntValue );
				err = aPersistConfigData->SetKey( aSectName, iniStringKey->Des(), intValueBuf );
				
                /*
				const TDesC & paramV = intValueBuf;
				RBuf valueV;
				valueV.Create( paramV );
				char* cl = (char*) valueV.Collapse().PtrZ();
				RDebug::Printf("  saving integer value=%s\n", cl );
                */

				break;

			case COptionConfig::ETString:
			case COptionConfig::ETFileName:
			case COptionConfig::ETMultiEntryEnum:
			case COptionConfig::ETSingleEntryEnum:
				{
				const TDesC & paramValue = aParam->ValueAsDesc();

				if( paramValue == KNullDesC )
					{
					LOG_MSG( "  Cannot save parameter value that is NULL\n" );	
					CleanupStack::PopAndDestroy( iniStringKey );
					return;
					}
                /*
				RBuf value;
				value.Create( paramValue );
				char* cl = (char*) value.Collapse().PtrZ();
				RDebug::Printf("  saving string value=%s\n", cl );
                */

				LOG_MSG( "  -> SetKey( string value )\n" );
				err = aPersistConfigData->SetKey( aSectName, iniStringKey->Des(), paramValue );
				}
				break;
			default:
				{
				// unknown type, Leave since the file could be corrupt or malformed
				LOG_MSG2( "  Invalid COptionConfig parameter type %d\n", aParam->Type() );
				CleanupStack::PopAndDestroy( iniStringKey );
				User::Leave( KErrCorrupt );
				}
			}// switch (COptionConfig::TOptionType)

		LOG_MSG( "  CleanupStack::PopAndDestroy( iniStringKey )" );
		CleanupStack::PopAndDestroy( iniStringKey );
		}



	// Save the Configuration Parameter Index
		{
		iniStringKey = GenerateKeyStringLC( aSaveIndex, EIniKeyIndex );

		TInt keyIndex = aParam->Index();
		TBuf<4> intIndexBuf;
		LOG_MSG3( "  -> keyIndex%d = parameter index %d\n", aSaveIndex, keyIndex );
		intIndexBuf.AppendNum( keyIndex );

		err = aPersistConfigData->SetKey( aSectName, iniStringKey->Des(), intIndexBuf );

		CleanupStack::PopAndDestroy( iniStringKey );
		
		if( KErrNone != err )
			{
			LOG_MSG2( "  ERROR !* aPersistConfigData->AddValue() returned %d\n", err );
			}
		User::LeaveIfError( err );
		}


	// Save the Configuration Parameter Type
		{
		iniStringKey = GenerateKeyStringLC( aSaveIndex, EIniKeyType );

		TInt keyType = aParam->Type();
		TBuf<8> intTypeBuf;
		LOG_MSG3( "  -> KeyType%d = parameter type %d\n", aSaveIndex, keyType );
		intTypeBuf.AppendNum( keyType );

		err = aPersistConfigData->SetKey( aSectName, iniStringKey->Des(), intTypeBuf );

		CleanupStack::PopAndDestroy( iniStringKey );
		if( KErrNone != err )
			{
			LOG_MSG2( "  ERROR !* aPersistConfigData->AddValue() returned %d\n", err );
			}
		User::LeaveIfError( err );
		}
	}



_LIT( KPluginExtension, ".plugin" );
#define KPluginUIDCharsPerLine 8


_LIT( KCDSConfig,				"CoreDumpServerConfiguration"			 );
_LIT( KWriterConfig,			"WriterConfiguration"					 );
_LIT( KFormatterConfig,			"FormatterConfiguration"				 );
_LIT( KObservedConfig,			"ObservedConfiguration"					 );

_LIT( KDefaultIniFile, "coredumpserver.ini" );
_LIT( KUID, "UID" );
_LIT( KPluginPair, "Pair" );





/**
Method to parse the set of files that contain the UIDs of the allowed plugins. 
The files are read our private directory in all drives. Thus installed .plugin
files are also checked. 

The plugin files must have the extension ".plugin". The content must be in 
ASCII-8. The format must be one UID per line, with no empty lines. 
The EOL can be CR+LF or just CR.
*/
void CCoreDumpSession::ReadPluginUidFilesL( )
	{

	RFs	fsSession;
	CleanupClosePushL( fsSession );
	User::LeaveIfError( fsSession.Connect() );

	//RProcess thisProc;
	
	TDriveList driveList;
    TVolumeInfo volInfo;

    User::LeaveIfError( fsSession.DriveList( driveList ) );

	HBufC* sessionPath = HBufC::NewLC( KMaxFileName );
	TPtr privPath( sessionPath->Des() );

	RBuf8 allowedPluginUidStr;
	allowedPluginUidStr.CleanupClosePushL();
	allowedPluginUidStr.CreateL( KPluginUIDCharsPerLine );

	RFile pluginFile;
	CleanupClosePushL( pluginFile );

    for( TInt driveNumber = EDriveA; driveNumber <= EDriveZ; driveNumber++ )
    {
        if( ( !driveList[driveNumber] ) || 
			( KErrNone != fsSession.Volume( volInfo, driveNumber) ) ||
			( volInfo.iDrive.iType==EMediaNotPresent ) )
            {
            continue;
            }

		// Get a list of the files in the private directory of drive <driveNumber> 

		fsSession.SetSessionToPrivate( driveNumber );
		User::LeaveIfError( fsSession.SessionPath( privPath ) );
		CDir * fileList = NULL;

		// KEntryAttNormal gets only files (not dirs, hidden or system files)
		TInt ret = fsSession.GetDir( privPath, KEntryAttNormal, ESortByName, fileList );
		if( ( KErrNone != ret ) || ( !fileList ) )
			{
	if( fileList )
				delete fileList;
			continue;
			}

		CleanupStack::PushL( fileList );

		for( TInt i = 0; i < fileList->Count(); i++ )
			{
			TEntry fileEntry = (*fileList)[i];
			TParse fileNameParser;
			fileNameParser.Set( fileEntry.iName, NULL, NULL );
			if( fileNameParser.Ext() != KPluginExtension )
				{
				continue;
				}

            /*
			RBuf8 pluginFilePrint;
			pluginFilePrint.CreateL( fileEntry.iName.Length() + 1 );
			pluginFilePrint.Copy( fileEntry.iName );
			char* cl = (char*) pluginFilePrint.PtrZ();
			LOG_MSG2("    Reading .plugin file %s\n", cl );
            */

			// File extension is .plugin, so add the list of UIDs contained inside 
			// the file to our list of allowed plugins
			//
			ret = pluginFile.Open( fsSession, fileEntry.iName, EFileRead );
			if( KErrNone != ret )
				{
				LOG_MSG("  Error openning file. Ignoring this file\n" );
				continue;
				}

			while( KErrNone == ret )
				{
				// Read the UID characters on a line. Exactly 8 * 1 byte non-UNICODE chars per line
				ret = pluginFile.Read( allowedPluginUidStr, KPluginUIDCharsPerLine );
				if( ( KErrNone != ret ) || (allowedPluginUidStr.Length() != KPluginUIDCharsPerLine ) )
					{
					// Reached the end of file, so go to the next file
					break;
					}

				TLex8 uidLex( allowedPluginUidStr );
				TUint pluginUidValue = 0;
				uidLex.Val( pluginUidValue, EHex );
				TUid allowedUid = TUid::Uid( pluginUidValue );
				iAllowedPlugins.AppendL( allowedUid );

				// Now skip the end of line character/s CR 0x0D and possibly LF 0x0A
				ret = pluginFile.Read( allowedPluginUidStr, 1 );
				if( ( KErrNone == ret ) && (allowedPluginUidStr.Length() == 1) )
					{
					//Could have read the CR or LF
					if( 0x0D == *(allowedPluginUidStr.Ptr()) )
						{
						// CR should be followed by LF, so read that too
						ret = pluginFile.Read( allowedPluginUidStr, 1 );
						}
					}

				if( allowedPluginUidStr.Length() == 0 )
					{
					ret = KErrEof;
					}

				} // while have read 8 bytes

			pluginFile.Close();
			} // for each file in file list

		CleanupStack::PopAndDestroy( fileList );
		}
	
	CleanupStack::PopAndDestroy( &pluginFile );
	CleanupStack::PopAndDestroy( &allowedPluginUidStr );
	CleanupStack::PopAndDestroy( sessionPath );
	CleanupStack::PopAndDestroy( &fsSession );

	if( 0 == iAllowedPlugins.Count() )
		{
		LOG_MSG( "  Warning : No plugins will be allowed\n" );
		}
	else
		{
		for( TInt i=0; i < iAllowedPlugins.Count(); i++ )
			{
			LOG_MSG2( "  Plugin 0x%X allowed\n", iAllowedPlugins[i] );
			}
		}

	}

void CCoreDumpSession::ValidatePluginL( const TUid aPluginUid ) const
	{

	LOG_MSG2( "->CCoreDumpSession::ValidatePlugin(aPluginId=0x%X)\n", aPluginUid.iUid );

	for( TInt i = 0; i < iAllowedPlugins.Count(); i++ )
		{
		if( iAllowedPlugins[i] == aPluginUid )
			{
			LOG_MSG( "  Plugin Allowed\n" );
			return;
			}
		}
	LOG_MSG( "  Plugin Not Allowed\n" );
    User::Leave(KErrPermissionDenied);
	}

/**
@pre aFsSession FileSystem session must have been connected to and still be active
@post aFileName and aPersistConfigData pushed onto the stack
*/
void CCoreDumpSession::OpenIniFileLC( const RMessage2	* aMessage, 
									const TBool		  aSaving,
									RFs				& aFsSession,
									HBufC		   *& aFileName,
									CIniDocument16 *& aPersistConfigData )
	{


	LOG_MSG( "CCoreDumpSession::OpenIniFileLC(RMessage2)\n" );

	TInt ret;

	TInt iniFileDesLen = 0;

	if( aMessage )
		{
		LOG_MSG( " RMessage2 != NULL\n" );
		iniFileDesLen = aMessage->GetDesLength( 0 );
		if( iniFileDesLen > 0 )
			{
			//LOG_MSG( "  User supplied ini file name\n" );
			aFileName = HBufC::NewL( iniFileDesLen );

			//LOG_MSG( "  CleanupStack::PushL( aFileName )\n" );
			CleanupStack::PushL( aFileName );

			TPtr ptr( aFileName->Des() );
			aMessage->ReadL( 0, ptr );
			if( !aSaving && !aFsSession.IsValidName( ptr ) )
				{
				// If we are loading and the file and path are incorrect
				//LOG_MSG( "  IsValidName() == EFalse\n" );
				User::Leave( KErrPathNotFound );
				}
			}
		}

	if( 0 == iniFileDesLen )
		{
		aFsSession.SetSessionToPrivate( EDriveE );

		RBuf privPath;
		privPath.CleanupClosePushL();
		privPath.CreateL( KMaxFileName );
		
		aFsSession.SessionPath( privPath );

		privPath.Append( KDefaultIniFile );
		aFileName = HBufC::NewL( privPath.Length() );
		*aFileName = privPath;// Copy name
        
		CleanupStack::PopAndDestroy( &privPath );

		CleanupStack::PushL( aFileName );

		if( aSaving )
			{
			//LOG_MSG( " aFsSession.CreatePrivatePath( EDriveE )\n" );
			ret = aFsSession.CreatePrivatePath( EDriveE );
			if( ( KErrNone  != ret ) && ( KErrAlreadyExists != ret ) )
				{
				LOG_MSG2( " aFsSession.CreatePrivatePath( EDriveE ) returned ERROR %d\n", ret );
				User::Leave( ret );
				}
			}
		}

	if( aSaving  ) 
		{
		// delete existing file since CIniDocument16::NewL expects a correctly formed 
		// file or no file at all. An empty file makes the NewL Leave
		//
		//LOG_MSG( "  -> aFsSession.Delete( aFileName->Des() )\n");
		aFsSession.Delete( aFileName->Des() );
		}
    else
    {
        TEntry entry;
        User::LeaveIfError(aFsSession.Entry(aFileName->Des(), entry));
    }
	//LOG_MSG( "  ->CIniDocument16::NewL( aFsSession, aFileName->Des() )\n");
	TRAPD(err, aPersistConfigData = CIniDocument16::NewL( aFsSession, aFileName->Des() ));

	if( KErrNone != err )
		{
		LOG_MSG2( "  <- CIniDocument16::NewL( ) left with %d\n", err );
		}

	CleanupStack::PushL( aPersistConfigData );
	}



_LIT( KIniFileCommentTitle,	"CoreDumpServerSettingsFile" );
_LIT( KIniFileCommentSymbian, "Symbian_Software_Ltd" );
_LIT( KIniFileCommentSymbianYear, "2007" );
_LIT( KIniFileCommentVersionString,	"Version" );
_LIT( KIniFileCommentVersionNumber,	"1.0.0" );


/*
* Client request to load system configuration file
*/
void CCoreDumpSession::SaveConfigFileL( const RMessage2* aMessage )
	{
	
	LOG_MSG( "CCoreDumpSession::SaveConfigFileL(RMessage2)\n");

	RFs	fsSession;
	CleanupClosePushL( fsSession );
	User::LeaveIfError( fsSession.Connect() );


	HBufC * fileName = NULL;
	CIniDocument16 * persistConfigData = NULL;

	OpenIniFileLC( aMessage, ETrue, fsSession, fileName, persistConfigData );

	// persistConfigData is on top of the stack, and under it is fileName

	TInt ret;
  	TPtrC uidString;
	uidString.Set( KUID );
	TPtrC sectName;

	sectName.Set( KIniFileCommentTitle );
	ret = persistConfigData->AddSection( sectName );
	User::LeaveIfError( ret );

	ret = persistConfigData->SetKey( sectName, KIniFileCommentSymbian, KIniFileCommentSymbianYear );
	User::LeaveIfError( ret );

	ret = persistConfigData->SetKey( sectName, KIniFileCommentVersionString, KIniFileCommentVersionNumber );
	User::LeaveIfError( ret );

	
	sectName.Set( KCDSConfig );
	//LOG_MSG( "    persistConfigData->AddSection( KCDSConfig )\n" );
	ret = persistConfigData->AddSection( sectName );
	//LOG_MSG2( "    AddSection( KCDSConfig ) returned %d", ret );
	User::LeaveIfError( ret );

	RBuf cdsUidNumberString;
    cdsUidNumberString.CleanupClosePushL();
	cdsUidNumberString.CreateL( 8 );
	cdsUidNumberString.AppendNumFixedWidth( (TUint)iOurSecureID.iUid, EHex, 8 );  // Width = 8 chars

	//LOG_MSG( "    persistConfigData->SetKey( KCDSConfig, uidString, cdsUidNumberString )\n" );
	ret = persistConfigData->SetKey( sectName, uidString, cdsUidNumberString );
	User::LeaveIfError( ret );

	for( TInt i = 0; i < iConfigList.Count(); i++ )
		{
		SaveConfigParamL( iConfigList[i], sectName, i, persistConfigData );
		}
    CleanupStack::PopAndDestroy(&cdsUidNumberString);	

	if( iObservationList.Count() > 0 )
		{

		//LOG_MSG( "  Saving Observed section\n");

		// There is no point in saving the id of a thread since the id is not predictable 
		// and could be used in another boot session. Thus only save the names
		// EIniKeyValue holds Target Name EIniKeyType holds TargetOwnerName
		// Thus a process will have both the same, so we can differentiate them
		HBufC * iniStringTargetValue;
		HBufC * iniStringOwnerType;

		TPtrC observationSection;
		observationSection.Set( KObservedConfig );
		//LOG_MSG( "  persistConfigData->AddSection( KObservedConfig )\n" );
		ret = persistConfigData->AddSection( observationSection );
		User::LeaveIfError( ret );

		TInt observeKey = 0;

		for( TInt i = 0; i < iObservationList.Count(); i ++ )
			{
            CTargetObserver *observer = iObservationList[i];
            TInt count = observer->ThreadCount();
            TInt j = 0;
            do
                {
                // Key string for target name
                iniStringTargetValue = GenerateKeyStringLC( observeKey, EIniKeyValue );
                TPtrC targetName;
                if(count == 0) //whole process, no threads on list => targetName == targetOwnerName
                    {
                    targetName.Set(observer->TargetName());
                    }
                else
                    {
                    targetName.Set(observer->Thread(j));
                    }

                //LOG_MSG2( " iPersistConfigData->SetKey( KObservedConfig, target.Length()=%d )\n", targetName.Length() );
                ret = persistConfigData->SetKey( observationSection, 
                                                 iniStringTargetValue->Des(), 
                                                 targetName );

                CleanupStack::PopAndDestroy( iniStringTargetValue );
                if( KErrNone != ret )
                    {
                    LOG_MSG2( "  ERROR !* iPersistConfigData->AddValue() for observed target returned %d\n", ret );
                    User::Leave( ret );
                    }

                // Key string for target owner name
                iniStringOwnerType = GenerateKeyStringLC( observeKey, EIniKeyType ); 
                TPtrC targetOwnerName( observer->TargetName() );

                LOG_MSG2( " iPersistConfigData->SetKey( KObservedConfig, targetOwner.Length()=%d )\n", targetOwnerName.Length() );
                ret = persistConfigData->SetKey( observationSection, 
                                                 iniStringOwnerType->Des(), 
                                                 targetOwnerName );

                CleanupStack::PopAndDestroy( iniStringOwnerType );
                if( KErrNone != ret )
                    {
                    LOG_MSG2( "  ERROR !* iPersistConfigData->AddValue() for observed target owner returned %d\n", ret );
                    User::Leave( ret );
                    }

                j++;
				observeKey ++;

                } while(j < count);

			} //for(i)
		}

    TBuf<63> sectionName;


    for(TInt i = 0; i < iFormatterPlugins.Count(); i++)
    {
        sectionName = KFormatterConfig;
        sectionName.AppendFormat(_L("%d"), i);

		ret = persistConfigData->AddSection( sectionName );
        User::LeaveIfError(ret);


   		// Add a key "UID" with a value equal to the UID of the writer, so we can load it again
		RBuf uidNumberString;
        uidNumberString.CleanupClosePushL();
		uidNumberString.CreateL( 8 );
		uidNumberString.AppendNumFixedWidth( (TUint)iFormatterInfos[i]->Uid(), EHex, 8 ); // Width = 8 chars
		ret = persistConfigData->SetKey( sectionName, uidString, uidNumberString );
		User::LeaveIfError( ret );
        CleanupStack::PopAndDestroy(&uidNumberString);

        TPtrC pairString;
        pairString.Set( KPluginPair );

        //Add pair key with value equal to the paired writer plugin
		RBuf pairNumberString;
        pairNumberString.CleanupClosePushL();
		pairNumberString.CreateL( 8 );
		pairNumberString.AppendNumFixedWidth( (TUint)iFormatterInfos[i]->Pair(), EHex, 8 ); // Width = 8 chars
		ret = persistConfigData->SetKey( sectionName, pairString, pairNumberString );
		User::LeaveIfError( ret );
        CleanupStack::PopAndDestroy(&pairNumberString);

   		for( TInt j = 0; j < iTotalConfigList.Count(); j++ )
			{
			if( iTotalConfigList[j] && (COptionConfig::EFormatterPlugin == iTotalConfigList[j]->Source()) && (i == iTotalConfigList[j]->Instance()))
				{
				SaveConfigParamL( iTotalConfigList[j], sectionName, iTotalConfigList[j]->Index(), persistConfigData );
				}
			}
    }

    for(TInt i = 0; i < iWriterPlugins.Count(); i++)
    {
        sectionName = KWriterConfig;
        sectionName.AppendFormat(_L("%d"), i);

		ret = persistConfigData->AddSection( sectionName );
        User::LeaveIfError(ret);

   		// Add a key "UID" with a value equal to the UID of the writer, so we can load it again
		RBuf uidNumberString;
        uidNumberString.CleanupClosePushL();
		uidNumberString.CreateL( 8 );
		uidNumberString.AppendNumFixedWidth( (TUint)iWriterInfos[i]->Uid(), EHex, 8 ); // Width = 8 chars
		ret = persistConfigData->SetKey( sectionName, uidString, uidNumberString );
		User::LeaveIfError( ret );
        CleanupStack::PopAndDestroy(&uidNumberString);

   		for( TInt j = 0; j < iTotalConfigList.Count(); j++ )
			{
			if( iTotalConfigList[j] && (COptionConfig::EWriterPlugin == iTotalConfigList[j]->Source()) && (i == iTotalConfigList[j]->Instance()))
				{
				SaveConfigParamL( iTotalConfigList[j], sectionName, iTotalConfigList[j]->Index(), persistConfigData );
				}
			}
    }

	if( NULL == persistConfigData )
		{
		LOG_MSG( "  persistConfigData == NULL\n" );
		}

	// Now save the config to file
	ret = persistConfigData->Externalise( fileName->Des() );
	//LOG_MSG2( "  <- iPersistConfigData->Externalise() returned %d\n", ret );

	//LOG_MSG( "  CleanupStack::PopAndDestroy( iPersistConfigData )" );
	CleanupStack::PopAndDestroy( persistConfigData );

	//LOG_MSG( "  CleanupStack::PopAndDestroy( fileName )" );
	CleanupStack::PopAndDestroy( fileName );

	//LOG_MSG( "  ->CleanupStack::PopAndDestroy( fsSession )" );
	CleanupStack::PopAndDestroy( & fsSession );
	//LOG_MSG( "  <-CleanupStack::PopAndDestroy( fsSession )" );

	}

/*
* Client request to load system configuration file
*/
void CCoreDumpSession::LoadConfigFileL( const RMessage2* aMessage )
	{
	

	LOG_MSG( "CCoreDumpSession::LoadConfigFileL(RMessage2)" );

	RFs	fsSession;
	//LOG_MSG( "  User::LeaveIfError( fsSession.Connect() )\n" );
	User::LeaveIfError( fsSession.Connect() );
	CleanupClosePushL( fsSession );

	TPtrC16 value;

	HBufC * fileName = NULL;
	CIniDocument16 * persistConfigData = NULL;

	//LOG_MSG( "   -> CIniDocument16 * iPersistConfigData = OpenIniFileLC()" );
	OpenIniFileLC( aMessage, EFalse, fsSession, fileName, persistConfigData );

	//LOG_MSG( "   -> RecreateConfigurationL( iPersistConfigData )\n" );
	RecreateConfigurationL( persistConfigData );

	//LOG_MSG( "   -> CleanupStack::PopAndDestroy( iPersistConfigData )" );
	CleanupStack::PopAndDestroy( persistConfigData );
	//LOG_MSG( "   -> CleanupStack::PopAndDestroy( fileName )" );
	CleanupStack::PopAndDestroy( fileName );
	//LOG_MSG( "   -> CleanupStack::PopAndDestroy( & fsSession )" );
	CleanupStack::PopAndDestroy( & fsSession );
	//LOG_MSG( "   returning from CCoreDumpSession::LoadConfigFileL(RMessage2)" );
	}


/**
Method used to load and unload the formatter and writer plugins as a pair.
Also configures the data source and data save for the formatter.
If unloading the UIDs are still checked.
*/
void CCoreDumpSession::SetPluginsL( const TUid aFormatterUID, 
								  const TUid aWriterUID, 
								  const TBool aLoad )
	{
	ValidatePluginL( aFormatterUID );

	ValidatePluginL( aWriterUID );

    /*
	if( iFormatter )
		{
		delete iFormatter;
		iFormatterUid = TUid::Uid(0);
		}

	if( iWriter )
		{
		delete iWriter;
		iWriterUid = TUid::Uid(0);
		}

	if( !aLoad )
		{
		return;
		}

	// We push the plugins onto the cleanup stack since if we leave here we 
	// want them destroyed so that there is always a valid pair or none are loaded
	iWriter = CCrashDataSave::NewL( aWriterUID );
	CleanupStack::PushL( iWriter );

	iFormatter = CCoreDumpFormatter::NewL( aFormatterUID );
	CleanupStack::PushL( iFormatter );

	iFormatter->ConfigureDataSourceL( iDataSource );
	iFormatter->ConfigureDataSaveL( iWriter );

	iFormatterUid = aFormatterUID;
	iWriterUid = aWriterUID;

	// We can pop the plugins
	CleanupStack::Pop( 2, iWriter );

    */
	}



/**
Look through the iPersistConfigData object (initialised from the ini file)
and attempt to recreate the state described by the object.
This means 
 - unloading current plugins, 
 - loading new plugins, 
 - updating the configuration list by calling UpdateConfigParametersL()
 - recreating the configuration for section (CDS, formatter, writer )

We have to be lax about the errors since we do not want to stop the 
loading of a configuration if there is a non-fatal error. This applies to 
loading of the plugins.
*/
void CCoreDumpSession::RecreateConfigurationL( CIniDocument16 * iPersistConfigData )
	{

	// for each section in ini file, 
	//  read section's SID
	//  load plugins
	//  for each (type, index, value) triplet, 
	//    create a COptionConfig 
	//    append to aIniFileConfigList
	//


	TInt ret;
	TBool foundCdsSection = EFalse;
	TBool foundFormatterSection = EFalse;
	TBool foundWriterSection = EFalse;

	TPtrC uidString;
	uidString.Set( KUID );

	// Recreate CDS section
		{
		TPtrC cdsUidString;
		//LOG_MSG("  iPersistConfigData->GetKeyValue( KCDSConfig, uidString, cdsUidString )\n" );
		ret = iPersistConfigData->GetKeyValue( KCDSConfig, uidString, cdsUidString );
		//LOG_MSG2("  iPersistConfigData->GetKeyValue( ) returned %d\n", ret );

		if( KErrNone == ret )
			{
			foundCdsSection = ETrue;
			}
		else
			{
			// It an error for there to be no saved CDS configuration 
			User::Leave( ret );
			}

		if( foundCdsSection )
			{
			TLex cdsUidLex( cdsUidString );
			TUint cdsUidValue;
			cdsUidLex.Val( cdsUidValue, EHex );
			TUid savedCdsUid = TUid::Uid( cdsUidValue );
			//LOG_MSG2("  Found CDS Config Section with UID 0x%X\n", cdsUidValue );
			//LOG_MSG2("  Current UID is 0x%X\n", iOurSecureID.iUid );
			if( savedCdsUid != iOurSecureID )
				{
				// The UID for the CDS does not match our current CDS SID.
				// This could arise if the config file was created by a previous version 
				// of the CDS, but more likely we are reading a corrupt file.
				// Thus the safe option is to leave
				LOG_MSG2( "  ERROR !* Read CDS UID from ini file as 0x%X, which is not same as our current value\n",
					cdsUidValue );
				User::Leave( KErrCorrupt );
				}
			}//foundCdsSection
		}

    TBuf<63> sectionName;


    foundWriterSection = -1;
    for(TInt i = 0;;i++)
    {
        sectionName = KWriterConfig;
        sectionName.AppendFormat(_L("%d"), i);

		TPtrC writerUidString;
		ret = iPersistConfigData->GetKeyValue( sectionName, uidString, writerUidString );

   		TUint writerUidValue;
		if( KErrNone == ret )
			{
            foundWriterSection++;
			TLex writerUidLex( writerUidString );
			writerUidLex.Val( writerUidValue, EHex );
			CreateWriterL( TUid::Uid( writerUidValue ) );
			}
		else
			{
                break;
			}

    } 


    foundFormatterSection = -1;
    for(TInt i = 0;;i++)
    {
        sectionName = KFormatterConfig;
        sectionName.AppendFormat(_L("%d"), i);

		TPtrC formatterUidString;
		ret = iPersistConfigData->GetKeyValue( sectionName, uidString, formatterUidString );

   		TUint formatterUidValue;
		if( KErrNone == ret )
			{
			foundFormatterSection++;
			TLex fromatterUidLex( formatterUidString );
			fromatterUidLex.Val( formatterUidValue, EHex );
			CreateFormatterL( TUid::Uid( formatterUidValue ) );

            TPtrC pairString;
            pairString.Set( KPluginPair );
            TPtrC formatterPairString;
            ret = iPersistConfigData->GetKeyValue(sectionName, pairString, formatterPairString);
            if(ret == KErrNone)
                {
			    TLex fromatterPairLex( formatterPairString );
                TUint formatterPairValue;
			    fromatterPairLex.Val( formatterPairValue, EHex );
                BindPluginsL(i, formatterPairValue);
                }
            }
		else
			{
                break;
			}

    }

    
	// Now that the old plugins are unloaded (if a new one was specified in the ini file),
	// and the new ones loaded, reconstruct the list of configuration parameters. 
	// We will then change them with the ini file values that we are about to read.
	// For plugins that have not been unloaded (because there was no mention in the init file)
	// we will simply ask for the config params again, and we should get the current values.

	UpdateConfigParametersL();

	if( foundCdsSection )
		{
		//LOG_MSG( "  -> ChangeConfigParamsL( COptionConfig::ECoreDumpServer\n" );
		ChangeConfigParamsL( COptionConfig::ECoreDumpServer, iPersistConfigData );
		}

    for(TInt i = 0; i <= foundFormatterSection; i++)
		{
		//LOG_MSG( "  -> ChangeConfigParamsL( COptionConfig::EFormatterPlugin\n" );
		ChangeConfigParamsL( COptionConfig::EFormatterPlugin, iPersistConfigData, foundFormatterSection );
		}

    for(TInt i = 0; i <= foundWriterSection; i++)
		{
		//LOG_MSG( "  -> ChangeConfigParamsL( COptionConfig::EWriterPlugin\n" );
		ChangeConfigParamsL( COptionConfig::EWriterPlugin, iPersistConfigData, foundWriterSection );
		}

	// Now rebuild the Observed list from the file
		{

		//LOG_MSG( "  About to Rebuild Observed Target List from Ini File\n" );
		
		HBufC * iniTargetValue;
		HBufC * iniOwnerType;
		TInt observeIndex = 0;
		TBool targetFound = ETrue;
		while( targetFound )
			{

			// Each value obtained via EIniKeyIndex from the file is the CConfigOption index 
			// and is used to tie the saved to the live index.

			//LOG_MSG2("  GenerateKeyStringLC( EIniKeyValue =%d)\n", observeIndex );
			iniTargetValue = GenerateKeyStringLC( observeIndex, EIniKeyValue );

			TPtrC observeTargetValue;

			//LOG_MSG("  iPersistConfigData->GetKeyValue( KObservedConfig, targetName )\n" );
			ret = iPersistConfigData->GetKeyValue( KObservedConfig, 
													iniTargetValue->Des(), 
													observeTargetValue );
			//LOG_MSG2("  iPersistConfigData->GetKeyValue( ) returned %d\n", ret );

			CleanupStack::PopAndDestroy( iniTargetValue );
		
			if( KErrNone != ret )
				{
				LOG_MSG2( "  Could not find a target : FindVar(EIniKeyValue) index %d returned false\n", 
					observeIndex );
				break;
				}

			//LOG_MSG2("  GenerateKeyStringLC( EIniKeyType =%d)\n", observeIndex );
			iniOwnerType = GenerateKeyStringLC( observeIndex, EIniKeyType );

			TPtrC observeTargetOwner;

			/*
			LOG_MSG3( "  ->FindVar( KObservedConfig, target owner %d. iniOwnerType.Length()=%d\n", 
				observeIndex, iniOwnerType->Des().Length() );
			*/

			ret = iPersistConfigData->GetKeyValue( KObservedConfig, 
													iniOwnerType->Des(), 
													observeTargetOwner );
			//LOG_MSG2("  iPersistConfigData->GetKeyValue( ) returned %d\n", ret );

			CleanupStack::PopAndDestroy( iniOwnerType );

			if( KErrNone != ret )
				{
				LOG_MSG2( "  Could not find a target owner: FindVar(EIniKeyValue) index %d returned false\n", 
					observeIndex );
				break;
				}

			//LOG_MSG("  Creating observation request\n" );

            AttachTargetL(observeTargetValue, observeTargetOwner);

			observeIndex++;
			
			}// while targetFound

		} // Now rebuild the Observed list from the file

	//LOG_MSG( "  <- returning from RecreateConfigurationL()\n" );

	}


/**
Method that reads the saved configuration and attempts to recreate that state.
For a given section of the ini file (indicated by the parameter aSource), 
recreate each parameter from a triplet of settings (index, type, value) 
and then apply the setting by calling SetConfigParameterL().

@param aSource Section of the config file to read and update state for. 
@param aPersistConfigData Pointer to CIniDocument16 object that contains 
the parameters to restore

@see CIniDocument16 
*/
void CCoreDumpSession::ChangeConfigParamsL( const COptionConfig::TParameterSource aSource,
										  CIniDocument16 * aPersistConfigData, const TInt aInstance)
	{

	//LOG_MSG2( "ChangeConfigParamsL( source=%d )\n", aSource );

	TPtrC sectName;
	TInt ret = KErrNone;

	switch(  aSource )
		{
		case COptionConfig::ECoreDumpServer:
			sectName.Set( KCDSConfig );
			break;
		case COptionConfig::EFormatterPlugin:
			sectName.Set( KFormatterConfig );
			break;
		case COptionConfig::EWriterPlugin:
			sectName.Set( KWriterConfig );
			break;
		default:
			User::Leave( KErrArgument );
		}


	HBufC * iniStringKey;
	TUint index = 0;

	// Loop to recreate all the parameters for a given section and then 
	// call SetConfigParameterL() for each parameter
	// As soon as one of the FindVar fails, we assume that there are no more entries for
	// this section, and thus give up.

	// Parameter index
	TInt keyIndex;

	// Parameter type
	TInt keyType;

	// Parameter integer value
	TInt keyIntValue;
	
	TLex paramLex;

	while( KErrNone == ret )
		{

		keyIndex = ELastParam;
		keyType = -1;
		keyIntValue = 0;
		TPtrC keyStringValue;

		// Get parameter index
			{
			// Each value obtained via EIniKeyIndex from the file is the CConfigOption index 
			// and is used to tie the saved to the live index.
			//LOG_MSG2("  ->GenerateKeyStringLC( index=%d, EIniKeyIndex )\n", index );
			iniStringKey = GenerateKeyStringLC( index, EIniKeyIndex );
			TPtrC keyIndexString;

			//LOG_MSG("  ->aPersistConfigData->GetKeyValue( EIniKeyIndex )\n" );
			ret = aPersistConfigData->GetKeyValue( sectName, iniStringKey->Des(), keyIndexString );
			CleanupStack::PopAndDestroy( iniStringKey );
			if( KErrNone != ret )
				{
				LOG_MSG3("  aPersistConfigData->GetKeyValue( EIniKeyIndex ) returned %d for index %d\n", ret, index );
				break;
				}
			
			paramLex.Assign( keyIndexString );
			keyIndex = 0;
			paramLex.Val( keyIndex );
			//LOG_MSG2( "  found EIniKeyIndex =%d\n", keyIndex );
			}


		// Get parameter type
			{

			//LOG_MSG2("  ->GenerateKeyStringLC( index=%d, EIniKeyType )\n", index );
			iniStringKey = GenerateKeyStringLC( index, EIniKeyType );
			TPtrC keyTypeString;
			
			//LOG_MSG("  ->aPersistConfigData->GetKeyValue( EIniKeyType )\n" );
			ret = aPersistConfigData->GetKeyValue( sectName, iniStringKey->Des(), keyTypeString );
			CleanupStack::PopAndDestroy( iniStringKey );
			if( KErrNone != ret )
				{
				LOG_MSG3("  aPersistConfigData->GetKeyValue( EIniKeyType ) returned %d for index %d\n", ret, index );
				break;
				}

			paramLex.Assign( keyTypeString );
			paramLex.Val( keyType );
			//LOG_MSG2( "  found EIniKeyType =%d\n", keyType );
			}

		// Get parameter value
			{

			//LOG_MSG2("  ->GenerateKeyStringLC( index=%d, EIniKeyValue )\n", index );
			iniStringKey = GenerateKeyStringLC( index, EIniKeyValue );

			//LOG_MSG( "  aPersistConfigData->GetKeyValue( EIniKeyValue )\n" );
			ret = aPersistConfigData->GetKeyValue( sectName, iniStringKey->Des(), keyStringValue );
			
			//LOG_MSG2( "  aPersistConfigData->GetKeyValue( EIniKeyValue ) returned string with length=%d\n", keyStringValue.Length() );
			CleanupStack::PopAndDestroy( iniStringKey );
			if( KErrNone != ret )
				{
				LOG_MSG3("  aPersistConfigData->GetKeyValue( EIniKeyValue ) returned %d for index %d\n", ret, index );
				break;
				}

			switch( (COptionConfig::TOptionType) keyType )
				{
				case COptionConfig::ETInt:
				case COptionConfig::ETUInt:
				case COptionConfig::ETBool:
					paramLex.Assign( keyStringValue );
					paramLex.Val( keyIntValue );
					keyStringValue.Set( KNullDesC );
					break;
				}
			}


			/*
			{
			if( -1 != keyIntValue )
				{
				LOG_MSG2( "  found EIniKeyValue as Int =%d\n", keyIntValue );
				}

			RBuf value;
			value.Create( keyStringValue );
			char* cl = (char*) value.Collapse().PtrZ();
			RDebug::Printf("  found EIniKeyValue as String =%s\n", cl );
			}
			*/

		LOG_MSG3( "  SetConfigParameterL( value=%d, string length=%d\n", keyIntValue, keyStringValue.Length() );

		switch(  aSource )
			{

			case COptionConfig::ECoreDumpServer:
				SetConfigParameterL( keyIndex, keyIntValue, keyStringValue );
				break;

			case COptionConfig::EFormatterPlugin:

				if( (iFormatterPlugins.Count() > 0 ) && (iFormatterPlugins.Count() < aInstance) )
					{
					iFormatterPlugins[aInstance]->SetConfigParameterL( keyIndex, keyIntValue, keyStringValue );
					}
				else
					{
					LOG_MSG2("CCoreDumpSession::ChangeConfigParamsL() : formatter:%d not loaded\n", aInstance);
					User::Leave( KErrArgument );
					}
				break;

			case COptionConfig::EWriterPlugin:

				if( (iWriterPlugins.Count() > 0 ) && (iWriterPlugins.Count() < aInstance) )
					{
					iWriterPlugins[aInstance]->SetConfigParameterL( keyIndex, keyIntValue, keyStringValue );
					}
				else
					{
					LOG_MSG2("CCoreDumpSession::ChangeConfigParamsL() : writer:%d not loaded\n", aInstance);
					User::Leave( KErrArgument );
					}
				break;

			default:
				User::Leave( KErrArgument );
			}
		
		index ++;
		}//while
	}


_LIT(KKey,"Key");
_LIT(KType,"Type");
_LIT(KIndex,"Index");
_LIT(KValue,"Value");

/**
This method allocates a string of two concatenated components. The first component is based on 
the parameter aKey. The second component is the string representation of aIndex.
Thus it maps an enum to a string and then it appends the index. The string is left on the 
cleanup stack and it is the responsibility of the caller to remove it from the cleanup stack.
*/
HBufC * CCoreDumpSession::GenerateKeyStringLC( TUint aIndex, 
										 CCoreDumpSession::TCoreDumpIniKey aKey )
	{

	HBufC * keyString = HBufC::NewL( 20 );
	CleanupStack::PushL( keyString );

	TPtr ptr = keyString->Des();
	ptr.Append( KKey );

	switch( aKey )
		{
		case EIniKeyType:
			ptr.Append( KType );
			break;

		case EIniKeyIndex:
			ptr.Append( KIndex );
			break;

		case EIniKeyValue:
			ptr.Append( KValue );
			break;
		}

	ptr.AppendNum( aIndex );
	return keyString;
	}

void CCoreDumpSession::SuspendProcessL(TUint64 aPid)
    {
    LOG_MSG("->CCoreDumpSession::SuspendProcessL()\n");
    iDataSource->GetThreadListL( aPid, iThreadPointerList, iTotalThreadListDescSize );
    TInt err = KErrNone;
    for(TInt i = 0; i < iThreadPointerList.Count(); i++)
        {
        LOG_MSG2("CCoreDumpSession::SuspenProcessL - suspending thread:%Ld\n", iThreadPointerList[i]->Id());
        err = iSecSess.SuspendThread(iThreadPointerList[i]->Id());
            if( (err != KErrNone) && (err != KErrAlreadyExists) )
            {
            LOG_MSG2("CCoreDumpSession::SuspendProcessL - unable to suspend thread! err:%d\n", err); 
            User::LeaveIfError(err);
            }
        } 
    }

void CCoreDumpSession::ResumeThreadL(TUint64 aTid)
{
    LOG_MSG2("->CCoreDumpSession::ResumeThreadL(aTid=%Ld)\n", aTid);

    TInt err = iSecSess.ResumeThread(TThreadId(aTid));
    if(err != KErrNone)
        {
        LOG_MSG2("CCoreDumpSession::ResumeThreadL - unable to resume thread! err:%d\n", err); 
        User::LeaveIfError(err);
        }
}

/**
Called to resume all the threads of the specified process. CCoreCrashHandler executes it as a post-processing action.
@param aPid ID of process to be resumed
@leave err one of the system wide error codes
@see SuspendProcessL
*/
void CCoreDumpSession::ResumeProcessL(TUint64 aPid)
	{
    LOG_MSG2("->CCoreDumpSession::ResumeProcessL(aPid=%Ld)\n", aPid);

    iDataSource->GetThreadListL( aPid, iThreadPointerList, iTotalThreadListDescSize );

    TInt err = KErrNone;
    //first resume child threads
    TInt looperr = KErrNone;
    for(TInt j = 1; j < iThreadPointerList.Count(); j++)
        {
        LOG_MSG2("CCoreDumpSession::ResumeProcessL - resuming thread:%Ld\n", iThreadPointerList[j]->Id());
        looperr = iSecSess.ResumeThread( iThreadPointerList[j]->Id() );
        if( (looperr != KErrNone) && (looperr != KErrNotFound) )
            {
            err = looperr;
            LOG_MSG2("CCoreDumpSession::ResumeProcessL - unable to resume thread! err:%d\n", err);
            }
        }

    looperr = KErrNone;
    //then resume the main thread
    if(iThreadPointerList.Count())
        {
        LOG_MSG2("CCoreDumpSession::ResumeProcessL - resuming main thread:%Ld\n", iThreadPointerList[0]->Id());
        looperr = iSecSess.ResumeThread(iThreadPointerList[0]->Id());
        }

    if( (looperr != KErrNone) && (looperr != KErrNotFound) )
        {
        err = looperr;
        LOG_MSG2("CCoreDumpSession::ResumeProcessL - unable to resume main thread! err:%d\n", err); 
        }

    User::LeaveIfError(err);
	}


void CCoreDumpSession::KillProcessL(TUint64 aPid)
    {
    LOG_MSG2("->CCoreDumpSession::KillProcessL(aPid=%Ld)\n", aPid);
    RProcess target;
    //LOG_MSG("CCoreDumpSession::KillProcessL - opening crashed process handle\n"); 
    TInt err = target.Open(aPid);
    if(err != KErrNone)
    {
        LOG_MSG2("CCoreDumpSession::KillProcessL - unable to open process handle! err:%d\n", err); 
        User::Leave(err);
    }

    CleanupClosePushL(target);

    TInt i;
    for(i = 0; i < iObservationList.Count(); i++)
        {
        if(iObservationList[i]->TargetName().CompareF(target.FileName()) == 0) break;
        }

    ResumeProcessL(aPid);

    if(i == iObservationList.Count())
        {
        LOG_MSG("CCoreDumpSession::KillProcessL - unable to find process observer!\n");
        User::Leave(KErrNotFound);
        }

	LOG_MSG("  -> SetCrashEventsL(*iObservationList[i], EActionIgnore)\n" ); 
    SetCrashEventsL(*iObservationList[i], EActionIgnore);

	LOG_MSG("  -> target.Terminate(KErrAbort)\n" ); 
    target.Terminate(KErrAbort);
	
	TRequestStatus logonStatus;
	LOG_MSG("  -> target.Logon( logonStatus );\n" ); 
	target.Logon( logonStatus );
	LOG_MSG("  <- target.Logon( logonStatus );\n" ); 

	CleanupStack::PopAndDestroy(&target); //target

	User::WaitForRequest( logonStatus );
	LOG_MSG("  <- User::WaitForRequest( logonStatus );\n" ); 
	

    if( (iPreCrashEventAction & ESuspendThread) || 
		(iPreCrashEventAction & ESuspendProcess) )
        SetCrashEventsL(*iObservationList[i], EActionSuspend);
    else
        SetCrashEventsL(*iObservationList[i], EActionContinue);
    }

/**
 * Cleanup item implementation for SCMConfiguration
*/
void CCoreDumpSession::CleanupSCMConfiguration(TAny* aScmConfig)
	{
	SCMConfiguration* config = static_cast<SCMConfiguration*> (aScmConfig);
	delete config;
	config = NULL;
	}


//void CCoreDumpSession::ReadSCMConfigListL()
//	{	
//	// read the config data from the flash data source	
//	}
//
//void CCoreDumpSession::WriteSCMConfigListL()
//	{	
//	// read the config data from the flash data source	
//	iFlashDataSource->WriteSCMConfigL();	
//	}
//

