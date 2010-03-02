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
//



/**
 @file
 @internalTechnology
 @released
*/

#include "corecrashhandler.h"
#include "coredumpsession.h"

CCrashHandler::CCrashHandler(CCoreDumpSession &aCoreSess)
    : CActive(CActive::EPriorityStandard),
    iThreadsRun(0),
    iCoreSess(aCoreSess)
	{
	CActiveScheduler::Add(this); 
	}

CCrashHandler* CCrashHandler::NewL(CCoreDumpSession &aCoreSess)
{
    //LOG_MSG("->CCrashHandler::NewL()");
	CCrashHandler* self = CCrashHandler::NewLC(aCoreSess);;
	CleanupStack::Pop(self);
    return self;
}

CCrashHandler* CCrashHandler::NewLC(CCoreDumpSession &aCoreSess) 
	{
    //LOG_MSG("->CCrashHandler::NewLC()");
	CCrashHandler *self = new (ELeave) CCrashHandler(aCoreSess);;
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

void CCrashHandler::ConstructL()
	{
    //LOG_MSG("->CCrashHandler::ConstructL()");
	iCrashCount = 0;
    iRemainingEvents.ReserveL(EEventsQueueDefaultLength); 
	}

CCrashHandler::~CCrashHandler()
	{
    //LOG_MSG("->CCrashHandler::~CCrashHandler()");
	Cancel();
    iRemainingEvents.Reset();
    iRemainingFlashCrashEvents.Reset();
    iTids.Reset();
	LOG_MSG("<-CCrashHandler::~CCrashHandler()");
	}

void CCrashHandler::DoCancel()
	{
	LOG_MSG("CCrashHandler::DoCancel()");
    iThread.LogonCancel(iStatus);
    iThread.Close();
	}

void CCrashHandler::ResetProperties(const TDesC &aProgress)
{
    LOG_MSG("->CCrashHandler::DefaultProperties\n");
    //restore properties to default values
    TInt err = RProperty::Set(KCoreDumpServUid, ECrashProgress, aProgress);
    if(err != KErrNone)
        {
        LOG_MSG2("CCrashHandler::DefaultProperties - unable to reset 'crash progress'! err:%d\n", err);
        }
    err = RProperty::Set(KCoreDumpServUid, ECancelCrash, EFalse);  
    if(err != KErrNone)
        {
        LOG_MSG2("CCrashHandler::DefaultProperties - unable to reset 'cancel crash'! err:%d\n", err);
        }

    err = RProperty::Set(KCoreDumpServUid, ECrashCount, iCrashCount);  
    if(err != KErrNone)
        {
        LOG_MSG2("CCrashHandler::DefaultProperties - unable to reset 'crash count'! err:%d\n", err);
        }
}

void CCrashHandler::PostProcessL()
{
    TInt32 action = iCoreSess.PostProcessingAction();
    LOG_MSG2("->CCrashHandler::PostProcessL - action:%d", action);
    switch(action)
        {
        case ENoPostAction:
        	{
            //no action
            break;
        	}
        case EResumeThread:
            {            
            if(iParams.iCrashInfo.iCrashSource == TCrashInfo::ELiveCrash)
            	{
	            LOG_MSG2("CCrashHandler::PostProcessL() - resuming crashed thread:%Ld\n", iParams.iCrashInfo.iTid);
	            iCoreSess.ResumeThreadL(iParams.iCrashInfo.iTid);
            	}
            break;
            }

        case EResumeProcess:
            {
            if(iParams.iCrashInfo.iCrashSource == TCrashInfo::ELiveCrash)
            	{            
	            LOG_MSG2("CCrashHandler::PostProcessL() - resuming crashed process:%Ld\n", iParams.iCrashInfo.iPid);
	            iCoreSess.ResumeProcessL(iParams.iCrashInfo.iPid);
            	}
            break;
            }

        case EKillProcess:
            {       
            if(iParams.iCrashInfo.iCrashSource == TCrashInfo::ELiveCrash)
            	{
				LOG_MSG2("CCrashHandler::PostProcessL() - killing crashed process:%Ld\n", iParams.iCrashInfo.iPid);
	            iCoreSess.KillProcessL(iParams.iCrashInfo.iPid);
            	}
            
            break;
            }
        case EDeleteCrashLog:
	        {
            if(iParams.iCrashInfo.iCrashSource == TCrashInfo::ESystemCrash)
            	{	        
            	LOG_MSG2("CCrashHandler::PostProcessL() - deleting crashed process:%Ld\n", iParams.iCrashInfo.iPid);
            	}
            
            break;
	        }
        default:
        	{
            LOG_MSG("CCrashHandler::PostProcessL() - unknown action!\n");
            User::Leave(KErrArgument);
        	}
        }
}
/**
 RunL() completes a previously issued ProcessCrashL call and 
 carry out user specified post processing actions. We trap the calls to 
 resume the thread, resume the process and kill the process since the thread
 or process may no longer be alive, and we would leave without finishing the
 core dump. 
 */
void CCrashHandler::RunL()
	{
    LOG_MSG2("->CCrashHandler::RunL(status:%d)", iStatus.Int());

    if(CrashInProgress())
    {
        LOG_MSG2("CCrashHandler::RunL - thread no:%d finished processing\n", iThreadsRun);
        iThread.Close();
        ResetProperties(_L("clear"));

        if(!(iMessage.IsNull()) && (iParams.iCrashInfo.iCrashSource == TCrashInfo::ESystemCrash) )
        	{
        	LOG_MSG("CCrashHandler::RunL - Completing the Async request After the Thread is Killed\n");
            //we have done with the processing and complete the async request
            iMessage.Complete(iStatus.Int());
        	}

        StartThreadL();
        return;
    }

    //multiple crash event case
    if(iParams.iCrashInfo.iType == TCrashInfo::ECrashException && iParams.iCrashInfo.iCrashSource == TCrashInfo::ELiveCrash)
        {
        //LOG_MSG2("CCrashHandler::RunL - marking thread:%Lu\n", iParams.iCrashInfo.iTid);
        iTids.AppendL(iParams.iCrashInfo.iTid);
        }


    PostProcessL();
    ResetProperties(_L("idle"));

    iCrashCount++;
      
    if(!CrashInProgress() && (iParams.iCrashInfo.iCrashSource == TCrashInfo::ESystemCrash))
    	{
    	if(!iMessage.IsNull())
    		{
        	//When there are no plugins
        	LOG_MSG("CCrashHandler::RunL - Completing the request when there are no plugins\n");
    		iMessage.Complete(iStatus.Int());
    		}
    	}

    if(iRemainingEvents.Count() > 0) 
        {
        ProcessCrashL(TCrashInfo::ELiveCrash);
        }
    
    if(iRemainingFlashCrashEvents.Count() > 0)
    	{
    	ProcessCrashL(TCrashInfo::ESystemCrash);
    	}

	}

// Report any leave to the client if possible.
TInt CCrashHandler::RunError(TInt aError)
	{
	LOG_MSG2("CCrashHandler::RunError(TInt aError=%d)", aError);
    TBuf<63> errorMsg;
    errorMsg.Format(_L("Processing core dump finished abnormally! err:%d"), aError);
    User::InfoPrint(errorMsg);
    
    if(!(iMessage.IsNull()) && (iParams.iCrashInfo.iCrashSource == TCrashInfo::ESystemCrash) )
    	{
    	LOG_MSG("CCrashHandler::RunError - Completing the Async request \n");
        iMessage.Complete(aError);
    	}    
	return KErrNone;
	}


void CCrashHandler::HandleCrashFromFlashL(const TCrashInfo& aCrashInf)
	{
	LOG_MSG("-> CCrashHandler::HandleCrashFromFlashL()");
	
	iRemainingFlashCrashEvents.AppendL(aCrashInf);
	
    if(!CrashInProgress())
        {
	    LOG_MSG("CCrashHandler::HandleCrashFromFlashL() -> ProcessCrashL\n");
        ProcessCrashL(aCrashInf.iCrashSource);
        }
	
	}

void CCrashHandler::HandleCrashFromFlashL(const TCrashInfo& aCrashInf, const RMessage2& aMessage)
	{
	LOG_MSG("-> CCrashHandler::HandleCrashFromFlashL() using the async mechanism");
	
	if(aMessage.IsNull())
		User::Leave(KErrArgument);
	
	iMessage = aMessage;
	
	iRemainingFlashCrashEvents.AppendL(aCrashInf);
	
    if(!CrashInProgress())
        {
	    LOG_MSG("CCrashHandler::HandleCrashFromFlashL() -> ProcessCrashL\n");
        ProcessCrashL(aCrashInf.iCrashSource);
        }
    else
    	{
    	LOG_MSG("CCrashHandler::HandleCrashFromFlashL() -> Crash already in progress\n");
    	}
	
	} 

void CCrashHandler::CancelHandleCrashFromFlash(const TCrashInfo& aCrashInf)
	{
	LOG_MSG("-> CCrashHandler::CancelHandleCrashFromFlashL() using the async mechanism");
	
    //if this request is pending remove the request from the queue
	for(TInt i =0; i<iRemainingFlashCrashEvents.Count(); i++)
		{
		if( (iRemainingFlashCrashEvents[i].iCrashId) == (aCrashInf.iCrashId))
			{
			LOG_MSG("CCrashHandler::CancelHandleCrashFromFlashL() -> Removed Pending Event from the queue\n");
			iRemainingFlashCrashEvents.Remove(i);
			}
		}
	
	}


void CCrashHandler::HandleCrashEventL(const TCrashEventInfo &aCrashEventInfo)
    {    
    LOG_MSG3("CCrashHandler::HandleCrashEventL - type:%d, thread:%Lu\n", aCrashEventInfo.iEventType, aCrashEventInfo.iThreadId);
    //multiple crash events case
    if(aCrashEventInfo.iEventType == EEventsKillThread)
        {
        TInt index = iTids.Find(aCrashEventInfo.iThreadId);
        if(index != KErrNotFound)
            {
            LOG_MSG2("CCrashHandler::HandleCrashEventL - multiple kill event, not handling thread:%Lu\n", aCrashEventInfo.iThreadId);
            iTids.Remove(index);
			iCoreSess.ResumeThreadL(aCrashEventInfo.iThreadId);
            return;
            }
        }


	LOG_MSG2("CCrashHandler::HandleCrashEvent() - preprocessing action:%d\n", iCoreSess.PreProcessingAction());
    if(iCoreSess.PreProcessingAction() == ESuspendProcess)
        {
        LOG_MSG2("CCrashHandler::HandleCrashEventL - suspending crashed process:%Lu\n", aCrashEventInfo.iProcessId);
        iCoreSess.SuspendProcessL(aCrashEventInfo.iProcessId);
        }

    iRemainingEvents.AppendL(aCrashEventInfo); //fifo order, coping the data

    if(!CrashInProgress())
        {
	    LOG_MSG("CCrashHandler::HandleCrashEvent() -> ProcessCrashL\n");
        ProcessCrashL(TCrashInfo::ELiveCrash);
        }
    }

void CCrashHandler::ProcessCrashL(const TCrashInfo::TCrashSource& aType) 
	{	
	switch (aType)
		{
		case TCrashInfo::ELiveCrash :
			{			
			ProcessLiveCrashL();
			break;
			}
		case TCrashInfo::ESystemCrash :
			{
			ProcessSystemCrashL();
			break;
			}
		default:
			{
			LOG_MSG( " CCrashHandler::ProcessCrashL()  --->  ERROR: Unknown crash type" );
			}
		}
	}

/**
 * Handles a crash stored in the system flash
 * @leave
 */
void CCrashHandler::ProcessSystemCrashL()
	{
	LOG_MSG( "->CCrashHandler::ProcessSystemCrashL()\n" );
	//set the crash paramaters
	iParams.iCrashInfo = iRemainingFlashCrashEvents[0];
		
	iRemainingFlashCrashEvents.Remove(0); //remove in fifo order				
	
	iCoreSess.FlashDataSource()->AnalyseCrashL(iParams.iCrashInfo.iCrashId);	
	
	StartThreadL();
	}
	
/**
 * Handles a live crash event
 * @leave
 */
void CCrashHandler::ProcessLiveCrashL()
	{
    LOG_MSG( "->CCrashHandler::ProcessLiveCrashL()\n" );

    TCrashEventInfo &event = iRemainingEvents[0]; //serve in fifo order

    TCrashInfo &crashInfo = iParams.iCrashInfo;

	if( !event.iThreadIdValid )
		{
		LOG_MSG( " ERROR *! - CCrashHandler::ProcessCrashL() - ThreadId marked as invalid\n" );
		User::Leave( KErrCorrupt );
		}

	if( !event.iProcessIdValid )
		{
		LOG_MSG( " ERROR *! - CCrashHandler::ProcessCrashL() - ProcessId marked as invalid\n" );
		User::Leave( KErrCorrupt );
		}

    crashInfo.iTid = event.iThreadId;
    crashInfo.iPid = event.iProcessId;
    crashInfo.iCrashSource = TCrashInfo::ELiveCrash;

    if(event.iEventType == EEventsHwExc)
		{
		crashInfo.iType = TCrashInfo::ECrashException;
		crashInfo.iContext = event.iThreadHwExceptionInfo.iRmdArmExcInfo; //only valid for HwExc
		LOG_MSG2( "  crashInfo.iContext.iFaultAddress=0x%X", crashInfo.iContext.iFaultAddress );
		LOG_MSG2( "  crashInfo.iContext.iFaultStatus=0x%X", crashInfo.iContext.iFaultStatus );
		LOG_MSG2( "  iThreadHwExceptionInfo.iExceptionNumber=0x%X", event.iThreadHwExceptionInfo.iExceptionNumber );
		crashInfo.iExcNumber = (TUint32)event.iThreadHwExceptionInfo.iExceptionNumber; // e32const.h :: TExcType
		}
    else if(event.iEventType == EEventsKillThread)
		{
		crashInfo.iType = TCrashInfo::ECrashKill;
		crashInfo.iReason = event.iThreadKillInfo.iExitReason; //only valid for KillThread
		crashInfo.iExcNumber = (TUint32)event.iThreadKillInfo.iExitType; // e32const.h :: TExitType
		TPtrC8 category( event.iThreadKillInfo.iPanicCategory, event.iThreadKillInfo.iPanicCategoryLength); //only valid for KillThread
		crashInfo.iCategory.Copy(category);
		}
	else
		{
		LOG_MSG2( " ERROR *! - CCrashHandler::ProcessCrashL() - TEventType=%d not supported", event.iEventType );
		User::Leave( KErrNotSupported );
		}
    
    crashInfo.iTime = event.iEventTime.Int64();
    
    iRemainingEvents.Remove(0); //remove in fifo order

    StartThreadL();
    }

void CCrashHandler::StartThreadL()
{
    LOG_MSG2("->CCrashHandler::StartThreadL() iThreadsRun %d\n", iThreadsRun);
    iParams.iFormatter = iCoreSess.GetValidFormatter(iThreadsRun);      

    if(!iParams.iFormatter)
    	{
    	LOG_MSG("->CCrashHandler::StartThreadL() no formatters found\n");
        iThreadsRun = 0;
        RunL();
        return;
    	}
    
    //SELF v1 and DEXC v1 arent supported for system crash
	TBuf<KMaxFileName> pluginDesc;
	iParams.iFormatter->GetDescription(pluginDesc);
	
	if(iParams.iCrashInfo.iCrashSource == TCrashInfo::ESystemCrash && (pluginDesc.Compare(KSelfV1) == 0 || pluginDesc.Compare(KDexcV1) == 0))
		{
		User::Leave(KErrNotSupported);
		}  

    if(iParams.iCrashInfo.iCrashSource == TCrashInfo::ELiveCrash)
    	{
    	iParams.iFormatter->ConfigureDataSourceL(iCoreSess.DataSource());
    	}
    else if(iParams.iCrashInfo.iCrashSource == TCrashInfo::ESystemCrash)
    	{
    	iParams.iFormatter->ConfigureDataSourceL(iCoreSess.FlashDataSource());
    	}
    else
    	{
    	//If we dont know the crash type we cant continue - programming error (dont tell user that though!)
    	__ASSERT_ALWAYS(ETrue, User::Panic(_L("Unknown crash type"), KErrUnknown));
    	}
    

    LOG_MSG("CCrashHandler::StartThreadL - creating the processing thread\n");

    TInt err = iThread.Create(KCrashProcessingThread, CCrashHandler::Processing, KDefaultStackSize, NULL, (TAny*)&iParams);
    if(err != KErrNone)
        {
        LOG_MSG2("CCrashHandler::StartThreadL - unable to create thread! err:%d\n", err);
        User::Leave(err);
        }

    ++iThreadsRun;
    
    iThread.Rendezvous(iStatus);    
    iThread.Resume();
    User::WaitForRequest(iStatus);

    User::LeaveIfError(iStatus.Int()); //something bad happened

    iThread.Logon(iStatus);
    //LOG_MSG("CCrashHandler::ProcessCrash() -> waiting for processing thread to finish\n");
    SetActive(); //wait for thread termination event
}

TInt CCrashHandler::Processing(TAny* aParams)
    {
    //LOG_MSG("-> CCrashHandler::CrashProcessing()");
    RThread::Rendezvous(KErrNone);
//    __UHEAP_MARK;
	TInt err = KErrNoMemory;
    CTrapCleanup* cleanup = CTrapCleanup::New();
	if(cleanup)
		{
	    TCrashThreadParams *params = static_cast<TCrashThreadParams*>(aParams);
        LOG_MSG("CCrashHandler::CrashProcessing() -> formatter.CrashEventL");
        TRAP(err, params->iFormatter->CrashEventL(&params->iCrashInfo)); 
		delete cleanup;
		}
//    __UHEAP_MARKEND;
    LOG_MSG2("CCrashHandler::CrashProcessing - processing thread returns: %d\n", err);
    return err;
    }

//eof

