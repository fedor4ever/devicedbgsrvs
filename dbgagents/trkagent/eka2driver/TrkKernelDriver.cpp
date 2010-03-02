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


#ifdef __WINS__
#error - this driver cannot be built for emulation
#endif

#include <e32def.h>
#include <e32cmn.h>
#include <u32std.h>
#include <kernel.h>
#include <kern_priv.h>
#include <nk_trace.h>
//#include <mmboot.h>
#include <arm.h>
#include <cache.h>
#include <platform.h>
#include <nkern.h>
#include <u32hal.h>

#include "TrkKernelDriver.h"
#include "TrkDriver.h"
#include "TrkEventHandler.h"

#define KTrkAppSecurUid 0x200170BB
#define KTrkExeSecurUid 0x200159E2
#define KTrkSrvSecurUid 0x200170B7

// Uncomment the line below for reading kernel thread registers.
// There is a problem using NKern::Lock before calling UserContextType on old releases like S60 3.0
// So for now, disabling the supporting reading kern thread registers as we are not really supporting
// device driver debugging anyway.
//#define SUPPORT_KERNCONTEXT

//
// Static function definitions
//

static TInt Bitcount(TUint32 val)
{
	TInt nbits;

	for (nbits = 0; val != 0; nbits++)
	{
		val &= val - 1;		// delete rightmost 1-bit in val
	}
	
	return nbits;
}

static TUint8 tolower(TUint8 c)
{
	if (c >= 'A' && c <= 'Z')
		c = c + ('a' - 'A');
	
	return c;
}


static TInt _strnicmp(const TUint8 *s1, const TUint8 *s2, int n)
{	
    int i;
    TUint8 c1, c2;
    for (i=0; i<n; i++)
    {
        c1 = tolower(*s1++);
        c2 = tolower(*s2++);
        if (c1 < c2) return -1;
        if (c1 > c2) return 1;
        if (!c1) return 0;
    }
    return 0;
}

//
//
// DMetroTrkDriverFactory implementation
//
//

//
// DMetroTrkDriverFactory constructor
//
DMetroTrkDriverFactory::DMetroTrkDriverFactory()
{
    iVersion = TVersion(KMajorVersionNumber,KMinorVersionNumber,KBuildVersionNumber);    
}

//
// DMetroTrkDriverFactory::Create
//
TInt DMetroTrkDriverFactory::Create(DLogicalChannelBase*& aChannel)
{
	if (iOpenChannels != 0)
		return KErrInUse; // a channel is already open
	
	aChannel = new DMetroTrkChannel(this);
	
	return aChannel ? KErrNone : KErrNoMemory;
}

//
// DMetroTrkDriverFactory::Install
//
TInt DMetroTrkDriverFactory::Install()
{
    return(SetName(&KMetroTrkDriverName));
}

//
// DMetroTrkDriverFactory::Install
//
void DMetroTrkDriverFactory::GetCaps(TDes8& aDes) const
{
    TCapsMetroTrkDriver b;
    b.iVersion = TVersion(KMajorVersionNumber, KMinorVersionNumber, KBuildVersionNumber);
    
    Kern::InfoCopy(aDes,(TUint8*)&b, sizeof(b));
}


//
//
// DMetroTrkChannel implementation
//
//

//
// DMetroTrkChannel constructor
//
DMetroTrkChannel::DMetroTrkChannel(DLogicalDevice* aLogicalDevice)
	: iExcludedROMAddressStart(ROM_LINEAR_BASE),
      iExcludedROMAddressEnd(0),
   	  iBreakPointList(NUMBER_OF_TEMP_BREAKPOINTS, 0),
  	  iNextBreakId(NUMBER_OF_TEMP_BREAKPOINTS),
      iEventInfo(NULL),
  	  iEventQueue(NUMBER_OF_EVENTS_TO_QUEUE, 0),
  	  iTraceEventQueue(NUMBER_OF_EVENTS_TO_QUEUE, 0),
      iRequestGetEventStatus(NULL),
   	  iPageSize(0x1000),
   	  iNotifyLibLoadedEvent(ETrue),
   	  iMultipleMemModel(EFalse),
   	  iExcInfoValid(EFalse),
   	  iDebugging(ETrue)
{
	LOG_MSG("DMetroTrkChannel::DMetroTrkChannel()");

	iDevice = aLogicalDevice;
	
	iClientThread = &Kern::CurrentThread();
	TInt err = iClientThread->Open();
	
	iBreakPointList.Reset();	
	TBreakEntry emptyTempBreak;
	
	for (TInt i = 0; i < NUMBER_OF_TEMP_BREAKPOINTS; i++)
	{
		emptyTempBreak.iId = i;
		
		if (KErrNone != iBreakPointList.Append(emptyTempBreak))
		{
			LOG_MSG("Error appending blank temp break entry");
		}
	}
	
	SEventInfo emptyEvent;

	for (TInt i=0; i<NUMBER_OF_EVENTS_TO_QUEUE; i++)
	{
		if (KErrNone != iEventQueue.Append(emptyEvent))
		{
			LOG_MSG("Error appending blank event entry");
		}
	}
	
	for (TInt i=0; i<NUMBER_OF_EVENTS_TO_QUEUE; i++)
	{
		if (KErrNone != iTraceEventQueue.Append(emptyEvent))
		{
			LOG_MSG("Error appending blank trace event entry");
		}
	}
		
	TTrkLibName emptyLib;
	for (TInt i=0; i<NUMBER_OF_LIBS_TO_REGISTER; i++)
    {
        if (KErrNone != iLibraryNotifyList.Append(emptyLib))
        {
            LOG_MSG("Error appending blank empty lib entry");
        }
    }
		
	iPageSize = Kern::RoundToPageSize(1);
}

//
// DMetroTrkChannel destructor
//
DMetroTrkChannel::~DMetroTrkChannel()
{
	LOG_MSG("DMetroTrkChannel::~DMetroTrkChannel()");
	
	iDebugging = EFalse;
	
	Kern::SafeClose((DObject*&)iClientThread, NULL);
	
	ClearAllBreakPoints();
	
	// close the breakpoint list and free the memory associated with it
	iBreakPointList.Close();

	// close the event queue and free the memory associated with it
	iEventQueue.Close();
	
	// close the trace event queue and free the memory associated with it
	iTraceEventQueue.Close();
	
	//close the debug process list
	iDebugProcessList.Close();
	
	//close the process notify list
	iProcessNotifyList.Close();
	
	//close the code modifier
	DebugSupport::CloseCodeModifier();

	// PANIC_BACKPORT
	// Resume all the frozen threads with semaphores.
	for(TInt i=0; i<iFrozenThreadSemaphores.Count(); i++)
	{
		NKern::FSSignal(iFrozenThreadSemaphores[i]);
		NKern::ThreadEnterCS();
		delete iFrozenThreadSemaphores[i];
		NKern::ThreadLeaveCS();
		iFrozenThreadSemaphores.Remove(i);		
	}
	//Reset the array and delete the objects that its members point to
	iFrozenThreadSemaphores.ResetAndDestroy();
	// END PANIC_BACKPORT
}

//
// DMetroTrkChannel::DoCreate
//
TInt DMetroTrkChannel::DoCreate(TInt /*aUnit*/, const TDesC* anInfo, const TVersion& aVer)
{
	LOG_MSG("DMetroTrkChannel::DoCreate()");

  	if (!Kern::QueryVersionSupported(TVersion(KMajorVersionNumber, KMinorVersionNumber, KBuildVersionNumber), aVer))
		return KErrNotSupported; 
  	
  	// Do the security check here so that any arbitrary application doesn't make 
  	// use of Trk kernel driver.
  	if (!DoSecurityCheck())
  	{
  		return KErrPermissionDenied;
  	}
  	
  	if (anInfo)
  	{
  		// this is the end address of the user library. 
  		// this doesn't seem to be valid for EKA2.
  		// right now we dont need this for EKA2 since we are not worried
  		// about kernel being stopped as kernel is multithreaded.
  		// just retaining this for future use.		
		TBuf8<32> buf;
		TInt err = Kern::ThreadRawRead(iClientThread, anInfo, &buf, 32);
		if(err != KErrNone)
			return err;
		
		//iExcludedROMAddressEnd = *(TUint32 *)(&(buf.Ptr()[20]));
	}
  	
  	//check whether the memory model is multiple or not.
  	TUint32 memModelAttrib = (TUint32)Kern::HalFunction(EHalGroupKernel, EKernelHalMemModelInfo, NULL, NULL);	
	if ((memModelAttrib & EMemModelTypeMask) == EMemModelTypeMultiple) 
    { 
        //Multiple memory model
        iMultipleMemModel = ETrue;        
    } 

	TUint caps; //ignored for now
	TInt err = DebugSupport::InitialiseCodeModifier(caps, NUMBER_OF_MAX_BREAKPOINTS);
	//if code modifier initializer failed, 
	//return here, since we can't set an breakpoints
	if(err != KErrNone) 
		return err;
		       
	//Setup the driver for receiving client messages
	iDFCQue = NULL;
	TBuf8<KMaxInfoName> trkDFC = _L8("TRK DFC");
	
	err = Kern::DfcQCreate(iDFCQue, 27, &trkDFC);
	if (err == KErrNone)
	{
		SetDfcQ(iDFCQue);
	}
	else
	{
		SetDfcQ(Kern::DfcQue0());
	}
	iMsgQ.Receive();  	
	
	iEventHandler = new DMetroTrkEventHandler;
	if (!iEventHandler)
		return KErrNoMemory;
	err = iEventHandler->Create(iDevice, this, iClientThread);
	if (err != KErrNone)
		return err;
		
	return iEventHandler->Start();
}

//
// DMetroTrkChannel::DoCancel
//
void DMetroTrkChannel::DoCancel(TInt aReqNo)
{
	LOG_MSG("DMetroTrkChannel::DoCancel()");
	
	switch(aReqNo)
	{
		case RMetroTrkDriver::ERequestGetEventCancel:
		{
			Kern::RequestComplete(iClientThread, iRequestGetEventStatus, KErrCancel);
			iEventInfo = NULL;
			iRequestGetEventStatus = 0;
		}
		break;
	}

}

//
// DMetroTrkChannel::DoRequest
//
void DMetroTrkChannel::DoRequest(TInt aReqNo, TRequestStatus* aStatus, TAny* a1, TAny* a2)
{
	LOG_MSG("DMetroTrkChannel::DoRequest()");
	
	switch(aReqNo)
	{
		case RMetroTrkDriver::ERequestGetEvent:
		{
			// check to see if we have any queued up events
			for (TInt i=0; i<NUMBER_OF_EVENTS_TO_QUEUE; i++)
			{
				if (SEventInfo::EUnknown != iEventQueue[i].iEventType)
				{
					LOG_MSG("DoRequest - slot NOT empty");
					// iClientThread is the user side debugger thread, so use it to write the info to it memory
					TInt err = Kern::ThreadRawWrite(iClientThread, a1, (TUint8 *)&iEventQueue[i], sizeof(SEventInfo), iClientThread);
					if (KErrNone != err)
						LOG_MSG2("Error writing event info: %d", err);

					// signal the debugger thread
					Kern::RequestComplete(iClientThread, aStatus, KErrNone);
				
					iEventQueue[i].Reset();
					return;
				}
				
				LOG_MSG("DoRequest - slot empty");
			}

			// check to see if we have any queued up trace events
			for (TInt i=0; i<NUMBER_OF_EVENTS_TO_QUEUE; i++)
			{
				if (SEventInfo::EUnknown != iTraceEventQueue[i].iEventType)
				{
					LOG_MSG("DoRequest - slot NOT empty");
					// iClientThread is the user side debugger thread, so use it to write the info to it memory
					TInt err = Kern::ThreadRawWrite(iClientThread, a1, (TUint8 *)&iTraceEventQueue[i], sizeof(SEventInfo), iClientThread);
					if (KErrNone != err)
						LOG_MSG2("Error writing trace event info: %d", err);

					// signal the debugger thread
					Kern::RequestComplete(iClientThread, aStatus, KErrNone);
				
					iTraceEventQueue[i].Reset();
					return;
				}
				
				LOG_MSG("DoRequest - trace slot empty");
			}
			
			// store the pointer so we can modify it later
			iEventInfo = (SEventInfo *)a1;
			iRequestGetEventStatus = aStatus;
			break;
		}		
		default:
			Kern::RequestComplete(iClientThread, aStatus, KErrNotSupported);
	}

}

//
// DMetroTrkChannel::DoControl
//
TInt DMetroTrkChannel::DoControl(TInt aFunction, TAny* a1, TAny* a2)
{
	LOG_MSG("DMetroTrkChannel::DoControl()");

	LOG_MSG2("DoControl Function %d", aFunction);
	
	TInt err = KErrNone;
	
	switch(aFunction)
	{
		case RMetroTrkDriver::EControlSetBreak:
		{
			err = SetBreak((TUint32)a1, (TMetroTrkBreakInfo*)a2);
			break;
		}
		case RMetroTrkDriver::EControlClearBreak:
		{
			err = DoClearBreak((TInt32)a1);
			break;
		}
		case RMetroTrkDriver::EControlChangeBreakThread:
		{
			err = DoChangeBreakThread((TUint32)a1, (TInt32)a2);
			break;
		}
		case RMetroTrkDriver::EControlSuspendThread:
		{
			err = DoSuspendThread(ThreadFromId((TUint32)a1));
			break;
		}
		case RMetroTrkDriver::EControlResumeThread:
		{			
			err = DoResumeThread(ThreadFromId((TUint32)a1));
			break;
		}
		case RMetroTrkDriver::EControlStepRange:
		{
			err = StepRange(ThreadFromId((TUint32)a1), (TMetroTrkStepInfo*)a2);
			break;
		}
		case RMetroTrkDriver::EControlReadMemory:
		{
			err = ReadMemory(ThreadFromId((TUint32)a1), (TMetroTrkMemoryInfo*)a2);
			break;
		}
		case RMetroTrkDriver::EControlWriteMemory:
		{
			err = WriteMemory(ThreadFromId((TUint32)a1), (TMetroTrkMemoryInfo*)a2);
			break;
		}
		case RMetroTrkDriver::EControlReadRegisters:
		{
			err = ReadRegisters(ThreadFromId((TUint32)a1), (TMetroTrkRegisterInfo*)a2);
			break;
		}
		case RMetroTrkDriver::EControlWriteRegisters:
		{
			err = WriteRegisters(ThreadFromId((TUint32)a1), (TMetroTrkRegisterInfo*)a2);
			break;
		}
		case RMetroTrkDriver::EControlGetProcessInfo:
		{
			err = GetProcessInfo((TInt)a1, (TMetroTrkTaskInfo*)a2);
			break;
		}
		case RMetroTrkDriver::EControlGetThreadInfo:
		{
			err = GetThreadInfo((TInt)a1, (TMetroTrkTaskInfo*)a2);
			break;
		}
		case RMetroTrkDriver::EControlGetProcessAddresses:
		{
			err = GetProcessAddresses(ThreadFromId((TUint32)a1), (TMetroTrkProcessInfo*)a2);
			break;
		}
		case RMetroTrkDriver::EControlGetStaticLibraryInfo:
		{
			err = GetStaticLibraryInfo((TInt)a1, (SEventInfo*)a2);
			break;
		}
		case RMetroTrkDriver::EControlEnableLibLoadedEvent:
		{
			iNotifyLibLoadedEvent = ETrue;
			break;
		}
		case RMetroTrkDriver::EControlDisableLibLoadedEvent:
		{
			iNotifyLibLoadedEvent = EFalse;
			break;
		}
		case RMetroTrkDriver::EControlGetLibraryInfo:
		{
			err = GetLibraryInfo((TMetroTrkLibInfo*)a1);
			break;
		}
		case RMetroTrkDriver::EControlGetExeInfo:
		{
			err = GetExeInfo((TMetroTrkExeInfo*)a1);
			break;
		}
		case RMetroTrkDriver::EControlGetProcUidInfo:
		{	
			err = GetProcUidInfo((TMetroTrkProcUidInfo*)a1);
			break;
		}
		case RMetroTrkDriver::EControlDetachProcess:
		{
		    err = DetachProcess(ProcessFromId((TInt32)a1));
			break;
		}
		default:
		{
			return KErrGeneral;
		}
	}
	
	if (KErrNone != err)
	{
		LOG_MSG2("Error %d from control function", err);
	}
	
	return err;
}

void DMetroTrkChannel::HandleMsg(TMessageBase* aMsg)
{
	LOG_MSG("DMetroTrkChannel::HandleMsg()");
	
	TThreadMessage& m = *(TThreadMessage*)aMsg;
	TInt id = m.iValue;


	if (id == (TInt)ECloseMsg)
	{
		if (iEventHandler)
		{
			iDebugging = EFalse;
			iEventHandler->Stop();
			iEventHandler->Close();
			iEventHandler = NULL;
		}
		m.Complete(KErrNone, EFalse);
		return;
	}

	if (id == KMaxTInt)
	{
		// DoCancel
		DoCancel(m.Int0());
		m.Complete(KErrNone, ETrue); 
		return;
	}

	if (id < 0)
	{
		// DoRequest
		TRequestStatus* pStatus = (TRequestStatus*)m.Ptr0();
		DoRequest(~id, pStatus, m.Ptr1(), m.Ptr2());
		m.Complete(KErrNone, ETrue);
	}
	else
	{
		// DoControl
		TInt err = DoControl(id, m.Ptr0(), m.Ptr1());
		m.Complete(err, ETrue);
	}
}

//
// DMetroTrkChannel::AddProcess
//
void DMetroTrkChannel::AddProcess(DProcess *aProcess, DThread *aThread)
{
	LOG_MSG("DMetroTrkChannel::AddProcess()");
	// check to see if we are still debugging, otherwise just return
	if (!iDebugging) 
		return;

	if (aProcess)
	{		
		if (!aThread)
			LOG_MSG("Creator thread not available");
	
		// check to see if this process is being started by debug agent.
		// If this is the case, we don't need to notify the agent since the debug agent already knows about it.
		if (aThread && aThread->iOwningProcess->iId == iClientThread->iOwningProcess->iId)
			return;		
		
		SEventInfo processEventInfo;
		processEventInfo.iEventType = processEventInfo.EProcessAdded; 
		processEventInfo.iProcessId = aProcess->iId;
		processEventInfo.iFileName.Copy(*aProcess->iName);
		processEventInfo.iUid = aProcess->iUids.iUid[2].iUid; 		

		// Kernel hasn't created teh code segment yet for this process,
		// so queue this event separately and wait for start thread event to notify.
		iProcessNotifyList.Append(processEventInfo);	
	}	
}

//
// DMetroTrkChannel::StartThread
//
void DMetroTrkChannel::StartThread(DThread *aThread)
{
	LOG_MSG("DMetroTrkChannel::StartThread()");
	// check to see if we are still debugging, otherwise just return
	if (!iDebugging) 
		return;
	
	// Using the lib loaded event flag for notifying processes as well.
	// Check if lib loaded event is disabled. Lib loaded event
	// is disabled temporarily in some situations, when the engine
	// is performing some complex operations  like installing sis 
	// files, in which case, suspending the thread for lib loaded 
	// events would end up in a dead lock. This is due  to the fact 
	// that the active object that handles the event notification 
	// wouldn't get a chance to run since the code in TRK engine  
	// which handles the commands from the host debugger is also 
	// done in an active object which runs in the same thread. 
	if (!iNotifyLibLoadedEvent)
		return;
	
	if (aThread && aThread->iOwningProcess)
	{				
		DCodeSeg* codeSeg = aThread->iOwningProcess->iCodeSeg;		
		if (codeSeg)
		{
			TModuleMemoryInfo processMemoryInfo;
			TInt err = codeSeg->GetMemoryInfo(processMemoryInfo, aThread->iOwningProcess);
			if (err == KErrNone)
			{						
				for (TInt i = 0; i < iProcessNotifyList.Count(); i++)
				{
					if (iProcessNotifyList[i].iProcessId == aThread->iOwningProcess->iId)
					{
						// Suspend the thread so that the host debugger can set breakpoints.
						Kern::ThreadSuspend(*aThread, 1); 
						iProcessNotifyList[i].iThreadId = aThread->iId;
						iProcessNotifyList[i].iCodeAddress = processMemoryInfo.iCodeBase;
						iProcessNotifyList[i].iDataAddress = processMemoryInfo.iInitialisedDataBase;
						// Notify the process added event now that we have the code segment for the process
						NotifyEvent(iProcessNotifyList[i]);
						// Now remove from the list
						iProcessNotifyList.Remove(i);
						break;
					}
				}
			}
			else
			{
				LOG_MSG2("Error in getting memory info: %d", err);
			}			
		}
		else
		{
			LOG_MSG2("Invalid code segment found for the started thread: %d", aThread->iId);
		}
	}	
}

//
// DMetroTrkChannel::RemoveProcess
//
void DMetroTrkChannel::RemoveProcess(DProcess *aProcess)
{
	LOG_MSG("DMetroTrkChannel::RemoveProcess()");
	// check to see if we are still debugging, otherwise just return
	if (!iDebugging) 
		return;

	// this is called when a process dies.  we want to mark any breakpoints in this
	// process space as obsolete.  the main reason for this is so we don't return
	// an error when the host debugger tries to clear breakpoints for the process
	
	TUint32 codeAddress = 0;
	TUint32 codeSize = 0;
	
	LOG_MSG2("Process being removed, Name %S", aProcess->iName);
	
	DCodeSeg* codeSeg = aProcess->iCodeSeg;
	
	if (codeSeg)
	{
		TModuleMemoryInfo processMemoryInfo;
		TInt err = codeSeg->GetMemoryInfo(processMemoryInfo, aProcess);
		if (err != KErrNone)
		{
		
			codeAddress = processMemoryInfo.iCodeBase;
			codeSize = processMemoryInfo.iCodeSize;
		}
		else
		{
			LOG_MSG2("Error in getting memory info: %d", err);
		}
		
	}
	
	if (!codeAddress || !codeSize)
	{
		LOG_MSG2("Code segment not available for process %d", aProcess->iId);
		// make sure there is not already a breakpoint at this address
		for (TInt i = 0; i < iDebugProcessList.Count(); i++)
		{
			if (iDebugProcessList[i].iId == aProcess->iId)
			{
				codeAddress = iDebugProcessList[i].iCodeAddress;
				codeSize = iDebugProcessList[i].iCodeSize;
				
				//now remove from the list
				iDebugProcessList.Remove(i);
				break;
			}
		}
	}
	
	if (!codeAddress || !codeSize)
		return;
		
	// first invalidate all breakpoints that were set in the library code
	for (TInt i=0; i<iBreakPointList.Count(); i++)
	{
		if ((iBreakPointList[i].iAddress >= codeAddress) && (iBreakPointList[i].iAddress < (codeAddress + codeSize)))
		{
			LOG_MSG2("Disabling process breakpoint at address %x", iBreakPointList[i].iAddress);
			iBreakPointList[i].iObsoleteLibraryBreakpoint = ETrue;
		}
	}
}


//
// DMetroTrkChannel::AddLibrary
//
void DMetroTrkChannel::AddLibrary(DLibrary *aLibrary, DThread *aThread)
{
	LOG_MSG("DMetroTrkChannel::AddLibrary()");
	
	LOG_MSG2(("Lib loaded: %S"), aLibrary->iName);
	
	// check to see if we are still debugging, otherwise just return
	if (!iDebugging) 
		return;
	
	// Check if lib loaded event is disabled. Lib loaded event
	// is disabled temporarily in some situations, when the engine
	// is performing some complex operations  like installing sis 
	// files, in which case, suspending the thread for lib loaded 
	// events would end up in a dead lock. This is due  to the fact 
	// that the active object that handles the event notification 
	// wouldn't get a chance to run since the code in TRK engine  
	// which handles the commands from the host debugger is also 
	// done in an active object which runs in the same thread. 
	if (!iNotifyLibLoadedEvent)
		return;

	TBool isDebugging = EFalse;	
	for (TInt i = 0; i < iDebugProcessList.Count(); i++)
	{
		if (iDebugProcessList[i].iId == aThread->iOwningProcess->iId)
		{
			isDebugging = ETrue;
			break;
		}
	}
	if (isDebugging == EFalse)
		return;

	if (aThread)
	{
		// make sure this is not the debugger thread
		if ((aThread != iClientThread) && (aThread->iOwningProcess->iId != iClientThread->iOwningProcess->iId))
		{
			SEventInfo info;

			Kern::ThreadSuspend(*aThread, 1); 
			
			info.iEventType = SEventInfo::ELibraryLoaded;
			info.iProcessId = aThread->iOwningProcess->iId;
			info.iThreadId = aThread->iId;
			
			//get the code address
			DCodeSeg* codeSeg = aLibrary->iCodeSeg;
			if (!codeSeg)
			{
				LOG_MSG2("Code segment not available for library %S", aLibrary->iName);
				return;
			}
			
			TModuleMemoryInfo memoryInfo;
			TInt err = codeSeg->GetMemoryInfo(memoryInfo, NULL); //NULL for DProcess should be ok;
			if (err != KErrNone)
			{
				LOG_MSG2("Error in getting memory info: %d", err);
				return;
			}
				
			info.iCodeAddress = memoryInfo.iCodeBase;
			info.iDataAddress = memoryInfo.iInitialisedDataBase;
			
			info.iFileName.Copy(*(aLibrary->iName)); //just the name, without uid info.
						
			// now remove this library if its in our notify list
			for (TInt i =0; i<iLibraryNotifyList.Count(); i++)
			{
				if (!iLibraryNotifyList[i].iEmptySlot &&
				    !_strnicmp(iLibraryNotifyList[i].iName.Ptr(), info.iFileName.Ptr(), info.iFileName.Length()))
				{										
					iLibraryNotifyList[i].iEmptySlot = ETrue;
					break;
				}
			}
			// now check to see if any libs are loaded because of this library load event.
			CheckLibraryNotifyList(info.iProcessId);
			//queue up or complete the event
			NotifyEvent(info);
		}
	
	}
		
}

//
// DMetroTrkChannel::RemoveLibrary
//
void DMetroTrkChannel::RemoveLibrary(DLibrary *aLibrary)
{
	LOG_MSG("DMetroTrkChannel::RemoveLibrary()");
	LOG_MSG2(("Lib unloaded: %S"), aLibrary->iName);
	
	// check to see if we are still debugging, otherwise just return
	if (!iDebugging) 
		return;
	
	// this is called when all handles to this library have been closed.  this can happen when a process dies, or when a dll is
	// unloaded while the process lives on.  in former case, we don't need to notify the host debugger because that process is
	// dying anyway.  for the latter case, we do need to notify the host so it can unload the symbolics, etc.
	
	DThread* aThread = &Kern::CurrentThread();
   
	if ((aThread) &&
       (aThread != iClientThread) && 
       (aThread->iOwningProcess->iId != iClientThread->iOwningProcess->iId))
	{
		//the library gets unloaded only when the mapcount is 0.
		if (aLibrary->iMapCount != 0)
			return;
		
		DCodeSeg* codeSeg = aLibrary->iCodeSeg;
		if (!codeSeg)
		{
			LOG_MSG2("Code segment not available for library %S", aLibrary->iName);
			return;
		}
		
		TModuleMemoryInfo processMemoryInfo;
		TInt err = codeSeg->GetMemoryInfo(processMemoryInfo, NULL); //passing NULL for the DProcess argument should be ok;
		if (err != KErrNone)
		{
			LOG_MSG2("Error in getting memory info: %d", err);
			return;
		}
		
		TUint32 codeAddress = processMemoryInfo.iCodeBase;
		TUint32 codeSize = processMemoryInfo.iCodeSize;
		
		// first invalidate all breakpoints that were set in the library code
		for (TInt i=0; i<iBreakPointList.Count(); i++)
		{
			if ((iBreakPointList[i].iAddress >= codeAddress) && (iBreakPointList[i].iAddress < (codeAddress + codeSize)))
			{
				LOG_MSG2("Disabling library breakpoint at address %x", iBreakPointList[i].iAddress);
				iBreakPointList[i].iObsoleteLibraryBreakpoint = ETrue;
			}
		}

	   	DProcess *process = &Kern::CurrentProcess();
	   	if (process)
	   	{
			RArray<SCodeSegEntry>* dynamicCode = &(process->iDynamicCode);			
			if (dynamicCode)
			{
				for (TInt j=0; j<dynamicCode->Count(); j++)
				{
					if ((*dynamicCode)[j].iLib == aLibrary)
					{
						SEventInfo info;
						
						info.iEventType = SEventInfo::ELibraryUnloaded;
						info.iFileName.Copy(*(aLibrary->iName)); //lib name without uid info
						//info.iFileName.ZeroTerminate();
						info.iProcessId = process->iId;
						info.iThreadId = 0xFFFFFFFF; // don't care!
						
						//queue up or complete the event
						NotifyEvent(info);
					}
				}
			}
		}
	}
}


//
// DMetroTrkChannel::AddCodeSegment
//
void DMetroTrkChannel::AddCodeSegment(DCodeSeg *aCodeSeg, DProcess *aProcess)
{
	LOG_MSG("DMetroTrkChannel::AddCodeSegment()");
	
	// Check if lib loaded event is disabled. Lib loaded event
	// is disabled temporarily in some situations, when the engine
	// is performing some complex operations  like installing sis 
	// files, in which case, suspending the thread for lib loaded 
	// events would end up in a dead lock. This is due  to the fact 
	// that the active object that handles the event notification 
	// wouldn't get a chance to run since the code in TRK engine  
	// which handles the commands from the host debugger is also 
	// done in an active object which runs in the same thread. 
	if (!iNotifyLibLoadedEvent)
		return;	

	TBool isDebugging = EFalse;	
	for (TInt i = 0; i < iDebugProcessList.Count(); i++)
	{
		if (iDebugProcessList[i].iId == aProcess->iId)
		{
			isDebugging = ETrue;
			break;
		}
	}
	if (isDebugging == EFalse)
		return;
		
	if (aCodeSeg)
	{
		const TUint8* ptr = aCodeSeg->iFileName->Ptr();
		if (aCodeSeg->IsDll())
		{
			LOG_MSG2("DLL code segment is loaded: %s", ptr);
			
			DThread* mainThread = aProcess->FirstThread();
			if (mainThread)
			{
				// make sure this is not the debugger thread
				if ((mainThread->iId != iClientThread->iId) && (aProcess->iId != iClientThread->iOwningProcess->iId))
				{
					SEventInfo info;

					//DoSuspendThread(mainThread); 
					Kern::ThreadSuspend(*mainThread, 1);
					
					info.iEventType = SEventInfo::ELibraryLoaded;
					info.iProcessId = aProcess->iId;
					info.iThreadId = mainThread->iId;				
					
					TModuleMemoryInfo memoryInfo;
					TInt err = aCodeSeg->GetMemoryInfo(memoryInfo, NULL); //NULL for DProcess should be ok;
					if (err != KErrNone)
					{
						LOG_MSG2("Error in getting memory info: %d", err);
						return;
					}
						
					info.iCodeAddress = memoryInfo.iCodeBase;
					info.iDataAddress = memoryInfo.iInitialisedDataBase;
					
					info.iFileName.Copy(aCodeSeg->iRootName); //just the name, without uid info.
								
					//queue up or complete the event
					NotifyEvent(info);
				}
		
			}
			else
			{
				LOG_MSG2("Invalid main thread for this process: %d", aProcess->iId);
			}

		}
		else 
		{
			if (aCodeSeg->IsExe())
				LOG_MSG2("EXE code segment is loaded: %s", ptr);		
		}
		
	}	

}

//
// DMetroTrkChannel::RemoveCodeSegment
//
void DMetroTrkChannel::RemoveCodeSegment(DCodeSeg *aCodeSeg, DProcess *aProcess)
{
	LOG_MSG("DMetroTrkChannel::RemoveCodeSegment()");
	
	// We don't do anything right now as we are not using the code segment events.		

}

//
// DMetroTrkChannel::HandleEventKillThread
//
TBool DMetroTrkChannel::HandleEventKillThread(DThread* aThread)
{
	LOG_MSG("DMetroTrkChannel::HandleEventKillThread");
	// check to see if we are still debugging, otherwise just return
	if (!iDebugging) 
		return EFalse;

	if (!aThread)
	{
		LOG_MSG("Invalid thread handle");
		return EFalse;
	}


	DThread* currentThread = &Kern::CurrentThread();
	if (!currentThread)
	{
		LOG_MSG("Error getting current thread");
		return EFalse;
	}

	//Kern::ThreadSuspend(*currentThread, 1); 
	
	if (aThread->iExitType != EExitPanic)
	{
		return EFalse;
	}
		
	SEventInfo info;
	
	info.iProcessId = aThread->iOwningProcess->iId;
	info.iThreadId = aThread->iId;
	info.iCurrentPC = ReadRegister(aThread, 14);//PC_REGISTER); 
	info.iPanicCategory.Copy(aThread->iExitCategory);
	
	info.iExceptionNumber = 100;//aThread->iExitReason;
	info.iPanicReason = aThread->iExitReason;
	info.iEventType = SEventInfo::EThreadPanic;

	// if its not an invalide opcode exception, check to see if we are debugging 
	// the process.
	TBool isDebugging = EFalse;	
	for (TInt i = 0; i < iDebugProcessList.Count(); i++)
	{
		if (iDebugProcessList[i].iId == info.iProcessId)
		{
			isDebugging = ETrue;
			break;
		}
	}
	if (!isDebugging)
		return EFalse;
	
#define DOFREEZE 1
	// PANIC_BACKPORT
#if DOFREEZE
	NKern::ThreadEnterCS();
	NFastSemaphore* sem = new NFastSemaphore();
	NKern::ThreadLeaveCS();
//	sem->iOwningThread = &(Kern::CurrentThread().iNThread);
	sem->iOwningThread = &aThread->iNThread;
	iFrozenThreadSemaphores.Append(sem);
	// First do the notify, then FSWait
	NotifyEvent(info);
	NKern::FSWait(sem);	
	return ETrue;
#else
	NotifyEvent(info);
	LOG_MSG("DMetroTrkChannel::HandleEventKillThread - Panic Do Nothing");
	return EFalse;
#endif
	// END PANIC_BACKPORT	
}

//
// DMetroTrkChannel::HandleSwException
//
TBool DMetroTrkChannel::HandleSwException(TExcType aExcType)
{
	LOG_MSG("DMetroTrkChannel::HandleSwException");

	SEventInfo info;
	
	DThread* currentThread = &Kern::CurrentThread();
	if (!currentThread)
	{
		LOG_MSG("Error getting current thread");
		return EFalse;
	}

	info.iProcessId = currentThread->iOwningProcess->iId;
	info.iThreadId = currentThread->iId;
	info.iCurrentPC = ReadRegister(currentThread, PC_REGISTER);
	info.iExceptionNumber = aExcType;
	info.iEventType = SEventInfo::EThreadException;

	if (info.iExceptionNumber != EExcInvalidOpCode) 
	{
		// if its not an invalide opcode exception, check to see if we are debugging 
		// the process.
		TBool isDebugging = EFalse;	
		for (TInt i = 0; i < iDebugProcessList.Count(); i++)
		{
			if (iDebugProcessList[i].iId == info.iProcessId)
			{
				isDebugging = ETrue;
				break;
			}
		}
		if (!isDebugging)
			return EFalse;
	}	

	NThread nThread = currentThread->iNThread;
	
	if (nThread.iSuspendCount == 0)
		Kern::ThreadSuspend(*currentThread, 1);
	
	HandleException(info, currentThread);
	
	return ETrue;
}

//
// DMetroTrkChannel::HandleHwException
//
TBool DMetroTrkChannel::HandleHwException(TArmExcInfo* aExcInfo)
{
	LOG_MSG("DMetroTrkChannel::HandleHwException()");

	SEventInfo info;
	
	DThread* currentThread = &Kern::CurrentThread();
	
	if (!currentThread)
	{
		LOG_MSG("Error getting current thread");
		return EFalse;
	}

	//store the exception info as its needed to read the context for system threads
	iExcInfoValid = ETrue;
	iCurrentExcInfo = *aExcInfo;

	info.iProcessId = currentThread->iOwningProcess->iId;
	info.iThreadId = currentThread->iId;
	info.iCurrentPC = aExcInfo->iR15;

	switch (aExcInfo->iExcCode)
	{
		case 0:
			info.iExceptionNumber = EExcCodeAbort;
			break;
		case 1:
			info.iExceptionNumber = EExcDataAbort;
			break;
		case 2:
			info.iExceptionNumber = EExcInvalidOpCode;
			break;
		default:
			return EFalse;
		
	}	
	info.iEventType = SEventInfo::EThreadException;
	
	if (info.iExceptionNumber != EExcInvalidOpCode) 
	{
		// if its not an invalide opcode exception, check to see if we are debugging 
		// the process.
		TBool isDebugging = EFalse;	
		for (TInt i = 0; i < iDebugProcessList.Count(); i++)
		{
			if (iDebugProcessList[i].iId == info.iProcessId)
			{
				isDebugging = ETrue;
				break;
			}
		}
		if (!isDebugging)
			return EFalse;
	}	
	
	NThread nThread = currentThread->iNThread;	

	if (nThread.iSuspendCount == 0) //don't suspend the thread if its already suspended.
		Kern::ThreadSuspend(*currentThread, 1);
	
	HandleException(info, currentThread);
	
	return ETrue;
}

//
// DMetroTrkChannel::HandleException
//
void DMetroTrkChannel::HandleException(SEventInfo& aEventInfo, DThread* aCurrentThread)
{
	TInt err = KErrNone;
	// see if it was a breakpoint that stopped the thread
	if (((TInt)2 == aEventInfo.iExceptionNumber) || ((TInt)EExcInvalidOpCode == aEventInfo.iExceptionNumber))
	{
		TUint32 inst = KArmBreakPoint;
		TInt instSize = 4;

		// change these for thumb mode
		if (ReadRegister(aCurrentThread, STATUS_REGISTER) & ECpuThumb)
		{
			inst = KThumbBreakPoint;
			instSize = 2;
		}
		
		TUint32 instruction = 0;
		err = Kern::ThreadRawRead(aCurrentThread, (TUint32 *)aEventInfo.iCurrentPC, (TUint8 *)&instruction, instSize);

		if (KErrNone != err)
			LOG_MSG2("Error reading instruction at currentpc: %d", err);
		
		if (!memcompare((TUint8 *)&inst, instSize, (TUint8 *)&instruction, instSize))
		{
			// the exception was a breakpoint instruction.  see if we have a breakpoint at that address
			for (TInt i=0; i<iBreakPointList.Count(); i++)
			{
				TBreakEntry breakEntry = iBreakPointList[i];
				
				if (breakEntry.iAddress == aEventInfo.iCurrentPC)
				{
					LOG_MSG2("Breakpoint with Id %d has been hit", breakEntry.iId);

					//change the event type to breakpoint type
					aEventInfo.iEventType = SEventInfo::EThreadBreakPoint;
					
				
					// enable any breakpoints we had to disable for this thread
					err = DoEnableDisabledBreak(aEventInfo.iThreadId);
					if (KErrNone != err)
						LOG_MSG2("Error %d enabling disabled breakpoints", err);
						
					// see if this is a temp breakpoint
					if (i < NUMBER_OF_TEMP_BREAKPOINTS)
					{
						// this was a temp breakpoint, so we need to clear it now
						err = DoClearBreak(i);
						if (KErrNone != err)
							LOG_MSG2("Error %d clearing temp breakpoint", err);
						
						// if we're not out of range yet, go ahead and single step again
						// if we are out of range, either continue or report the event depending
						// on the iResumeOnceOutOfRange flag
						if ((aEventInfo.iCurrentPC > breakEntry.iRangeStart) && (aEventInfo.iCurrentPC < breakEntry.iRangeEnd))
						{
							LOG_MSG("PC is still in range, stepping will continue");
							err = DoStepRange(aCurrentThread, breakEntry.iRangeStart, breakEntry.iRangeEnd, breakEntry.iSteppingInto, breakEntry.iResumeOnceOutOfRange, breakEntry.iSteppingInto);
							if (KErrNone != err)
							{
								LOG_MSG2("Error in DoStepRange: %d.  Resuming thread.", err);
								err = DoResumeThread(aCurrentThread);
								if (KErrNone != err)
									LOG_MSG2("Error in DoResumeThread: %d", err);
							}
							return;												
						}
						else
						{
							if (breakEntry.iResumeOnceOutOfRange)
							{
								LOG_MSG("PC is out of range, continuing thread");
								DoResumeThread(aCurrentThread);
								
								return;
							}
						}
					}
					
					// if the breakpoint is thread specific, make sure it's the right thread
					// if not, just continue the thread.  take special care if it's the debugger
					// thread.  if it hits a regular breakpoint, we NEVER want to stop at it.  if
					// it hits a temp breakpoint, we're probably just stepping past a real breakpoint
					// and we do need to handle it.
					if (((breakEntry.iThreadId != aEventInfo.iThreadId) && breakEntry.iThreadSpecific)/*(breakEntry.iThreadId != 0xFFFFFFFF))*/ ||
						((aEventInfo.iProcessId == iClientThread->iOwningProcess->iId) && (breakEntry.iThreadId != aEventInfo.iThreadId)))
					{
						LOG_MSG("breakpoint does not match threadId, calling DoResumeThread");
						err = DoResumeThread(aCurrentThread);
						if (KErrNone != err)
							LOG_MSG2("Error in DoResumeThread: %d", err);

						return;					
					}
					
					//normal user break point, just notify the event
					break;
				}
			}

		}
	}
	
	NotifyEvent(aEventInfo);
}

//
// DMetroTrkChannel::HandleUserTrace
//
TBool DMetroTrkChannel::HandleUserTrace(TText* aStr, TInt aLen)
{
	LOG_MSG("DMetroTrkChannel::HandleUserTrace()");

// handle user trace events only for app trk.
#ifndef __OEM_TRK__ 
	// check to see if we are still debugging, otherwise just return
	if (!iDebugging) 
		return EFalse;
	
	DThread* currentThread = &Kern::CurrentThread();
	
	if (!currentThread)
	{
		LOG_MSG("Error getting current thread");
		return EFalse;
	}

	if (currentThread->iOwningProcess->iId == iClientThread->iOwningProcess->iId)
		return EFalse;

	// only send traces for processes you are debugging	
	TBool isDebugging = EFalse;	
	for (TInt i = 0; i < iDebugProcessList.Count(); i++)
	{
		if (iDebugProcessList[i].iId == currentThread->iOwningProcess->iId)
		{
			isDebugging = ETrue;
			break;
		}
	}
	if (!isDebugging)
		return EFalse;
	
	if (aLen > 0)
	{
		SEventInfo info;

		// This is a temporary solution for grabbing the rdebug printfs.
		// With the current TRK implementation, we don't have a mechanism for 
		// using dynamically allocated buffers between the engine and the driver. 
		// So for now we are limiting the printf strings to 256 bytes.		
		TUint8 traceStr[260];
		info.iTraceDataLen = aLen;
		
		if (info.iTraceDataLen > 256)
			info.iTraceDataLen  = 256;
			
		XTRAPD(r, XT_DEFAULT, kumemget(traceStr, aStr, info.iTraceDataLen));
		
		if (r == KErrNone)
		{
			info.iEventType = SEventInfo::EUserTrace;			
			traceStr[info.iTraceDataLen] = '\r';
			traceStr[info.iTraceDataLen+1] = '\n';
			info.iTraceDataLen += 2; // account for \r and \n.

			info.iTraceData.Copy((TUint8*)traceStr, info.iTraceDataLen);
			NotifyEvent(info, ETrue);										
		}
		else
		{
			return EFalse;
		}
	}
#endif	

	return ETrue;
}


//
// DMetroTrkChannel::SetBreak
//
TInt DMetroTrkChannel::SetBreak(TUint32 aThreadId, TMetroTrkBreakInfo* aBreakInfo)
{
	LOG_MSG("DMetroTrkChannel::SetBreak()");

	TInt err = KErrNone;
	
	if (!aBreakInfo)
		return KErrArgument;
	
	//User side memory is not accessible directly
	TMetroTrkBreakInfo info(0, 0, 0, 0);
	err = Kern::ThreadRawRead(iClientThread, aBreakInfo, (TUint8*)&info, sizeof(TMetroTrkBreakInfo));
	if (err != KErrNone)
		return err;
				
	if (!info.iId) //first check if the iId address is valid
		return KErrArgument;
				
	if (err == KErrNone) 
	{
		TInt32 iId;
		
		err = DoSetBreak(info.iProcessId, aThreadId, info.iAddress, info.iThumbMode, iId);
		
		if (err == KErrNone)
		{
			err = Kern::ThreadRawWrite(iClientThread, (TUint8 *)info.iId, &iId, sizeof(TInt32), iClientThread);
		}
	}
	return err;
}

//
// DMetroTrkChannel::StepRange
//
TInt DMetroTrkChannel::StepRange(DThread* aThread, TMetroTrkStepInfo* aStepInfo)
{
	LOG_MSG("DMetroTrkChannel::StepRange()");

	TInt err = KErrNone;
	
	if (!aStepInfo)
		return KErrArgument;
	
	TMetroTrkStepInfo info(0, 0, 0);
	err = Kern::ThreadRawRead(iClientThread, aStepInfo, (TUint8*)&info, sizeof(TMetroTrkStepInfo));
	
	if (err != KErrNone)
		return err;
	
	err = DoStepRange(aThread, info.iStartAddress, info.iStopAddress, info.iStepInto, EFalse, ETrue);
	
	return err;
}

//
// DMetroTrkChannel::ReadMemory
//
TInt DMetroTrkChannel::ReadMemory(DThread* aThread, TMetroTrkMemoryInfo* aMemoryInfo)
{
	LOG_MSG("DMetroTrkChannel::ReadMemory()");

	TInt err = KErrNone;
	
	if (!aMemoryInfo)
		return KErrArgument;
		
#ifndef __OEM_TRK__
	if (!IsBeingDebugged(aThread))
		return KErrPermissionDenied;
#endif
		
	TMetroTrkMemoryInfo info(0, 0, 0);
	err = Kern::ThreadRawRead(iClientThread, aMemoryInfo, (TUint8*)&info, sizeof(TMetroTrkMemoryInfo));
	if (err != KErrNone)
		return err;
	
	if (!info.iData)
		return KErrArgument;

	TUint8 *data = (TUint8*)Kern::Alloc(info.iLength);
	if (!data)
		return KErrNoMemory;
	
	TPtr8 dataDes(data, info.iLength);
		
	err = DoReadMemory(aThread, info.iAddress, info.iLength, dataDes);
	if (err != KErrNone)
		return err;
	
	err = Kern::ThreadDesWrite(iClientThread, info.iData, dataDes, 0, KChunkShiftBy0, iClientThread);
	
	Kern::Free(data);
	
	return err;
}

//
// DMetroTrkChannel::WriteMemory
//
TInt DMetroTrkChannel::WriteMemory(DThread* aThread, TMetroTrkMemoryInfo* aMemoryInfo)
{
	LOG_MSG("DMetroTrkChannel::WriteMemory()");

	TInt err = KErrNone;
	
	if (!aMemoryInfo)
		return KErrArgument;
	
#ifndef __OEM_TRK__
	if (!IsBeingDebugged(aThread))
		return KErrPermissionDenied;
#endif	
	
	TMetroTrkMemoryInfo info(0, 0, 0);
	err = Kern::ThreadRawRead(iClientThread, aMemoryInfo, (TUint8*)&info, sizeof(TMetroTrkMemoryInfo));
	if (err != KErrNone)
		return err;
	
	if (!info.iData)
		return KErrArgument;
	
	TUint8 *data = (TUint8*)Kern::Alloc(info.iLength);
	if (!data)
		return KErrNoMemory;
	
	TPtr8 dataDes(data, info.iLength);
	
	err = Kern::ThreadDesRead(iClientThread, info.iData, dataDes, 0);
	if (err != KErrNone)
		return err;
							
	err = DoWriteMemory(aThread, info.iAddress, info.iLength, dataDes);
	
	Kern::Free(data);
	
	return err;
}

//
// DMetroTrkChannel::ReadRegisters
//
TInt DMetroTrkChannel::ReadRegisters(DThread* aThread, TMetroTrkRegisterInfo* aRegisterInfo)
{
	LOG_MSG("DMetroTrkChannel::ReadRegisters()");

	TInt err = KErrNone;
	
	if (!aRegisterInfo)
		return KErrArgument;
	
#ifndef __OEM_TRK__	
	if (!IsBeingDebugged(aThread))
		return KErrPermissionDenied;
#endif
	
	TMetroTrkRegisterInfo info(0, 0, 0);
	err = Kern::ThreadRawRead(iClientThread, aRegisterInfo, (TUint8*)&info, sizeof(TMetroTrkRegisterInfo));
	if (err != KErrNone)
		return err;

	if (!info.iValues)
		return KErrArgument;
	
	TUint length = (info.iLastRegister - info.iFirstRegister + 1) * 4;
	TUint8 *values = (TUint8*)Kern::Alloc(length);
	if (!values)
		return KErrNoMemory;

	TPtr8 valuesDes(values, length);
	
	err = DoReadRegisters(aThread, info.iFirstRegister, info.iLastRegister, valuesDes);
	if (err != KErrNone)
		return err;
	
	err = Kern::ThreadDesWrite(iClientThread, info.iValues, valuesDes, 0, KChunkShiftBy0, iClientThread);

	Kern::Free(values);
	
	return err;
}

//
// DMetroTrkChannel::WriteRegisters
//
TInt DMetroTrkChannel::WriteRegisters(DThread* aThread, TMetroTrkRegisterInfo* aRegisterInfo)
{
	LOG_MSG("DMetroTrkChannel::WriteRegisters()");

	TInt err = KErrNone;
	
	if (!aRegisterInfo)
		return KErrArgument;

#ifndef __OEM_TRK__
	if (!IsBeingDebugged(aThread))
		return KErrPermissionDenied;
#endif

	TMetroTrkRegisterInfo info(0, 0, 0);
	err = Kern::ThreadRawRead(iClientThread, aRegisterInfo, (TUint8*)&info, sizeof(TMetroTrkRegisterInfo));
	if (err != KErrNone)
		return err;
		
	if (!info.iValues)
		return KErrArgument;
	
	TUint length = (info.iLastRegister - info.iFirstRegister + 1) * 4;
	
	TUint8 *values = (TUint8*)Kern::Alloc(length);
	if (!values)
		return KErrNoMemory;
	
	TPtr8 valuesDes(values, length);
	
	err = Kern::ThreadDesRead(iClientThread, info.iValues, valuesDes, 0);
	if (err != KErrNone)
		return err;

	err = DoWriteRegisters(aThread, info.iFirstRegister, info.iLastRegister, valuesDes);
	
	Kern::Free(values);
	
	return err;
}

//
// DMetroTrkChannel::GetProcessInfo
//
TInt DMetroTrkChannel::GetProcessInfo(TInt aIndex, TMetroTrkTaskInfo* aTaskInfo)
{
	LOG_MSG("DMetroTrkChannel::GetProcessInfo()");

	TInt err = KErrNone;
	
	if (!aTaskInfo)
		return KErrArgument;

	TMetroTrkTaskInfo info(0);
	
	err = DoGetProcessInfo(aIndex, &info);
	
	if (err != KErrNone)
		return err;
	
	err = Kern::ThreadRawWrite(iClientThread, aTaskInfo, &info, sizeof(TMetroTrkTaskInfo), iClientThread);
	
	return err;
}

//
// DMetroTrkChannel::GetThreadInfo
//
TInt DMetroTrkChannel::GetThreadInfo(TInt aIndex, TMetroTrkTaskInfo* aTaskInfo)
{
	LOG_MSG("DMetroTrkChannel::GetThreadInfo()");

	TInt err = KErrNone;
	
	if (!aTaskInfo)
		return KErrArgument;
	
	TMetroTrkTaskInfo info(0);
	err = Kern::ThreadRawRead(iClientThread, aTaskInfo, (TUint8*)&info, sizeof(TMetroTrkTaskInfo));
	if (err != KErrNone)
		return err;
	
	err = DoGetThreadInfo(aIndex, &info);
	
	if (err != KErrNone)
		return err;
	
	err = Kern::ThreadRawWrite(iClientThread, aTaskInfo, &info, sizeof(TMetroTrkTaskInfo), iClientThread);
	
	return err;
}

//
// DMetroTrkChannel::GetProcessAddresses
//
TInt DMetroTrkChannel::GetProcessAddresses(DThread* aThread, TMetroTrkProcessInfo* aProcessInfo)
{
	LOG_MSG("DMetroTrkChannel::GetProcessAddresses()");

	TInt err = KErrNone;

	if  (!aProcessInfo)
		return KErrArgument;
	
	TMetroTrkProcessInfo info(0, 0);
	err = Kern::ThreadRawRead(iClientThread, aProcessInfo, (TUint8*)&info, sizeof(TMetroTrkProcessInfo));
	
	if (err != KErrNone)
		return err;
		
	if (!info.iCodeAddress || !info.iDataAddress)
		return KErrArgument;
	
	TUint32 codeAddress;
	TUint32 dataAddress;
	
	err = DoGetProcessAddresses(aThread, codeAddress, dataAddress);
	
	if (err != KErrNone)
		return err;
		
	err = Kern::ThreadRawWrite(iClientThread, info.iCodeAddress, (TUint8*)&codeAddress, sizeof(TUint32), iClientThread);
	err = Kern::ThreadRawWrite(iClientThread, info.iDataAddress, (TUint8*)&dataAddress, sizeof(TUint32), iClientThread);

	return err;
}

//
// DMetroTrkChannel::GetStaticLibraryInfo
//
TInt DMetroTrkChannel::GetStaticLibraryInfo(TInt aIndex, SEventInfo* aEventInfo)
{
	LOG_MSG("DMetroTrkChannel::GetStaticLibraryInfo()");

	TInt err = KErrNone;

	if (!aEventInfo)
		return KErrArgument;

	SEventInfo info;

	err = Kern::ThreadRawRead(iClientThread, aEventInfo, (TUint8*)&info, sizeof(SEventInfo));
	
	if (err != KErrNone)
		return err;

	err = DoGetStaticLibraryInfo(aIndex, &info);
	
	if (err != KErrNone)
		return err;
	
	err = Kern::ThreadRawWrite(iClientThread, aEventInfo, &info, sizeof(SEventInfo), iClientThread);

	return err;
}

//
// DMetroTrkChannel::GetLibInfo
//
TInt DMetroTrkChannel::GetLibraryInfo(TMetroTrkLibInfo* aLibInfo)
{
	LOG_MSG("DMetroTrkChannel::GetLibraryInfo()");

	TInt err = KErrNone;
	
	if (!aLibInfo)
		return KErrArgument;

	TMetroTrkLibInfo info(0, 0);

	err = Kern::ThreadRawRead(iClientThread, aLibInfo, (TUint8*)&info, sizeof(TMetroTrkLibInfo));

	if (err != KErrNone)
		return err;

	if (!info.iFileName)
		return KErrArgument;
	
	TUint8 *dllName = (TUint8*)Kern::Alloc(info.iFileNameLength);
	if (!dllName)
		return KErrNoMemory;
	
	TPtr8 dllNameDes(dllName, info.iFileNameLength);
	
	err = Kern::ThreadDesRead(iClientThread, info.iFileName, dllNameDes, 0);
	if (!err)
	{
		err = DoGetLibraryInfo(dllNameDes, &info);
	
		if (!err)		
			err = Kern::ThreadRawWrite(iClientThread, aLibInfo, &info, sizeof(TMetroTrkLibInfo), iClientThread);
		
		// we couldn't find the library info, so add to our list to look for 
		// when a library is loaded or a process is loaded.	
		if (err != KErrNone)
		{
            for (TInt i=0; i<NUMBER_OF_LIBS_TO_REGISTER; i++)
            {                
                if (!iLibraryNotifyList[i].iName.Length() && iLibraryNotifyList[i].iEmptySlot)
		        {
                    iLibraryNotifyList[i].iName.Copy(dllNameDes);
                    iLibraryNotifyList[i].iEmptySlot = EFalse;
                    break;
		        }
		        else if (!iLibraryNotifyList[i].iEmptySlot && !_strnicmp(iLibraryNotifyList[i].iName.Ptr(), dllNameDes.Ptr(), dllNameDes.Length()))
		        {
		        	break;
		        }
            }
		}		
	}
	
	// now free the allocated memory
	Kern::Free(dllName);	

	return err;
}

//
// DMetroTrkChannel::GetExeInfo
//
TInt DMetroTrkChannel::GetExeInfo(TMetroTrkExeInfo* aExeInfo)
{
	LOG_MSG("DMetroTrkChannel::GetExeInfo()");

	TInt err = KErrNone;
	
	if (!aExeInfo)
		return KErrArgument;

	TMetroTrkExeInfo info(0, 0, 0);

	err = Kern::ThreadRawRead(iClientThread, aExeInfo, (TUint8*)&info, sizeof(TMetroTrkExeInfo));

	if (err != KErrNone)
		return err;

	if (!info.iFileName)
		return KErrArgument;
	
	TUint8 *exeName = (TUint8*)Kern::Alloc(info.iFileNameLength);
	if (!exeName)
		return KErrNoMemory;
	
	TPtr8 exeNameDes(exeName, info.iFileNameLength);
	
	err = Kern::ThreadDesRead(iClientThread, info.iFileName, exeNameDes, 0);
	if (KErrNone == err)
	{
			err = DoGetExeInfo(exeNameDes, &info);
		if (KErrNone == err)
		{		
			err = Kern::ThreadRawWrite(iClientThread, aExeInfo, &info, sizeof(TMetroTrkExeInfo), iClientThread);
		}
	}
	
	Kern::Free(exeName);

	return err;
}

//
// DMetroTrkChannel::GetProcUidInfo
//
TInt DMetroTrkChannel::GetProcUidInfo(TMetroTrkProcUidInfo* aProcUidInfo)
{
	LOG_MSG("DMetroTrkChannel::GetProcUidInfo()");

	TInt err = KErrNone;
	
	if (!aProcUidInfo)
		return KErrArgument;

	TMetroTrkProcUidInfo info(0);

	err = Kern::ThreadRawRead(iClientThread, aProcUidInfo, (TUint8*)&info, sizeof(TMetroTrkProcUidInfo));

	if (err != KErrNone)
		return err;
	
	err = DoGetProcUidInfo(&info);
	
	if (err != KErrNone)
		return err;
	
	err = Kern::ThreadRawWrite(iClientThread, aProcUidInfo, &info, sizeof(TMetroTrkProcUidInfo), iClientThread);

	return err;
}

//
//DMetroTrkChannel::DetachProcess
//
TInt DMetroTrkChannel::DetachProcess(DProcess *aProcess)
{
    LOG_MSG("DMetroTrkChannel::DetachProcess()");
    // check to see if we are still debugging, otherwise just return
    if (!iDebugging || !aProcess) 
        return KErrArgument;

    // This is called when a process has been detached.  
    // We want to mark any breakpoints in this process space as clear.      
    TUint32 codeAddress = 0;
    TUint32 codeSize = 0;
    
    LOG_MSG2("Process being Detached, Name %S", aProcess->iName);    

    // make sure there is not already a breakpoint at this address
    for (TInt i = 0; i < iDebugProcessList.Count(); i++)
    {
        if (iDebugProcessList[i].iId == aProcess->iId)
        {
            codeAddress = iDebugProcessList[i].iCodeAddress;
            codeSize = iDebugProcessList[i].iCodeSize;
            //now remove from the list
            iDebugProcessList.Remove(i);
            break;
        }
	}

    if (!codeAddress || !codeSize)
        return KErrArgument;
        
    // first invalidate all breakpoints that were set in the library code
    for (TInt i=0; i<iBreakPointList.Count(); i++)
    {
        if ((iBreakPointList[i].iAddress >= codeAddress) && (iBreakPointList[i].iAddress < (codeAddress + codeSize)))
        {
            LOG_MSG2("Clearing process breakpoint at address %x", iBreakPointList[i].iAddress);
            // clear the break here
            TInt32 err = KErrNone;
            err = DoClearBreak(iBreakPointList[i].iId);
            
            //If not able to clear the break point host debugger  make obselete.
            if (KErrNone != err)
            {
                LOG_MSG2("Clearing process breakpoint at address %x failed", iBreakPointList[i].iAddress);                
            }
        }
	}

    return KErrNone;
}


//
// DMetroTrkChannel::DoSetBreak
//
TInt DMetroTrkChannel::DoSetBreak(const TUint32 aProcessId, const TUint32 aThreadId, const TUint32 aAddress, const TBool aThumbMode, TInt32 &aId)
{
	LOG_MSG("DMetroTrkChannel::DoSetBreak()");

	// do not allow breakpoints in the excluded ROM region
	//if ((aAddress >= iExcludedROMAddressStart) && (aAddress < iExcludedROMAddressEnd))
	//{
	//	return KErrNotSupported;
	//}
	
	// make sure there is not already a breakpoint at this address
	for (TInt i = NUMBER_OF_TEMP_BREAKPOINTS; i < iBreakPointList.Count(); i++)
	{
		if (iBreakPointList[i].iAddress == aAddress)
		{
			return KErrAlreadyExists;
		}
	}
	
	// increment the break id
	aId = iNextBreakId++;	

	// create the new breakpoint entry
	TBreakEntry breakEntry(aId, aThreadId, aAddress, aThumbMode);

	// IDE currently sends non-thread specific breakpoints (threadId == 0xffffffff)
	// if this changes, we will get a real thread ID
	// This check must be done before calling DoEnableBreak as it will replace the 
	// the breakentry threadId with a real one
	if (aThreadId == 0xFFFFFFFF)
	{
	    breakEntry.iThreadSpecific = EFalse;
	    breakEntry.iThreadId = aProcessId+1; //process wide break, just use the main thread id for this process		
	}
	else
	{
		breakEntry.iThreadSpecific = ETrue;
	}
        
	TInt err = DoEnableBreak(breakEntry, ETrue);
	
	if (KErrNone == err)
	{
		ReturnIfError(iBreakPointList.Append(breakEntry));
	}
	
	return err;
}


//
// DMetroTrkChannel::DoEnableBreak
//
TInt DMetroTrkChannel::DoEnableBreak(TBreakEntry &aEntry, TBool aSaveOldInstruction)
{
	LOG_MSG("DMetroTrkChannel::DoEnableBreak()");

	// default to arm mode
	TUint32 inst = KArmBreakPoint;
	TInt instSize = 4;
	
	if (aEntry.iThumbMode)
	{
		LOG_MSG("Thumb Breakpoint");
		inst = KThumbBreakPoint;
		instSize = 2;
	}

	TInt err = KErrNone;
  			
	// Get thread id from the process that we are debugging
	TProcessInfo * proc = NULL;
	TUint32 threadId = NULL;
	
	threadId = aEntry.iThreadId;
	
	if (!threadId || threadId==0xFFFFFFFF) //threadId=0xFFFFFFFF is special case with CW debugger.
	{
		for (TInt i=0; i<iDebugProcessList.Count(); i++)
		{
			proc = &iDebugProcessList[i];
			if ( proc && (proc->iCodeAddress <= aEntry.iAddress) && (aEntry.iAddress <= (proc->iCodeAddress + proc->iCodeSize)))
			{
				threadId = proc->iId+1;
				break;
			}
		}
	}
	
	DThread* threadObj = ThreadFromId(threadId);
	//if we don't have the right thread id for the address, 
	//then try with the thread id of the process that we are debugging 	
	if (!threadObj && iDebugProcessList.Count())
	{
		proc = &iDebugProcessList[0];
		if (proc)
		{
			threadId = proc->iId+1;	
		}
		threadObj = ThreadFromId(threadId);
	}
	
	if (threadObj)
	{		
		if (aSaveOldInstruction)
		{
			TUint32 instruction;

			// read the instruction at the address so we can store it in the break entry for when we clear this breakpoint
			// trap exceptions in case the address is invalid
			XTRAPD(r, XT_DEFAULT, err = TryToReadMemory(threadObj, (TAny *)aEntry.iAddress, (TAny *)&instruction, instSize));

			ReturnIfError((KErrNone == r) ? err : r);

			aEntry.iInstruction.Copy((TUint8 *)&instruction, instSize);
		}

		aEntry.iThreadId = threadId; //set the thread ID here 
		XTRAPD(r, XT_DEFAULT, err = DebugSupport::ModifyCode(threadObj, aEntry.iAddress, instSize, inst, DebugSupport::EBreakpointGlobal));
		err = (DebugSupport::EBreakpointGlobal == r) ? KErrNone : r;
	}
	else
	{
		err = KErrBadHandle;	
	}


	return err;
}

//
// DMetroTrkChannel::DoClearBreak
//
TInt DMetroTrkChannel::DoClearBreak(const TInt32 aId)
{
	LOG_MSG("DMetroTrkChannel::DoClearBreak()");

	// find the break entry matching this id.  note that the breakpoints are already sorted in ascending order by id
	TBreakEntry entry;
	entry.iId = aId;
	TInt index = iBreakPointList.FindInSignedKeyOrder(entry);

	TInt err = KErrNone;
	if (index >= 0)
	{
		// if this breakpoint was set in a library and that library has already been unloaded, don't try to clear it
		if (!iBreakPointList[index].iObsoleteLibraryBreakpoint)
		{
			LOG_MSG2("Clearing breakpoint at address %x", iBreakPointList[index].iAddress);

			DThread* threadObj = ThreadFromId(iBreakPointList[index].iThreadId);
			// In case of multiple memory model, if the thread doesn't exist any more, don't try to clear it.
			// For example it might be in a static library which was unloaded when the thread exited,
			// but we didn't get told so we didn't mark it as iObsoleteLibraryBreakpoint.
			if (threadObj != NULL)
			{
				XTRAPD(r, XT_DEFAULT, err = DebugSupport::RestoreCode(threadObj, iBreakPointList[index].iAddress));
				err = (KErrNone == r) ? err : r;

			}
			else
			{
				err = KErrBadHandle;
			}
		}
		
		if (KErrNone == err)
		{
			// if this is a temp breakpoint, just clear out the values, otherwise remove it from the list
			if (index < NUMBER_OF_TEMP_BREAKPOINTS)
			{
				iBreakPointList[index].Reset();
			}
			else
			{
				iBreakPointList.Remove(index);
			}
		}
		else
		{
			LOG_MSG2("Error clearing breakpoint %d", err);
		}
				
		return err;
	}

	LOG_MSG2("Break Id %d not found", aId);

	return KErrNotFound;
}


//
// DMetroTrkChannel::DoChangeBreakThread
//
TInt DMetroTrkChannel::DoChangeBreakThread(TUint32 aThreadId, TInt32 aId)
{
	LOG_MSG("DMetroTrkChannel::DoChangeBreakThread()");

	// find the break entry matching this id.  note that the breakpoints are already sorted in ascending order by id
	TBreakEntry entry;
	entry.iId = aId;
	TInt index = iBreakPointList.FindInSignedKeyOrder(entry);
	
	if (index >=0)
	{
		// change the thread id for this breakpoint
		iBreakPointList[index].iThreadId = aThreadId;
		return KErrNone;
	}

	return KErrNotFound;
}

//
// DMetroTrkChannel::DoSuspendThread
//
TInt DMetroTrkChannel::DoSuspendThread(DThread *aThread)
{
	LOG_MSG("DMetroTrkChannel::DoSuspendThread()");

	if (!aThread)
	{
		LOG_MSG("Invalid dthread object");
		return KErrArgument;
	}
	
	NThread nThread = aThread->iNThread;
	
	if (nThread.iSuspendCount == 0)
		Kern::ThreadSuspend(*aThread, 1); 

	return KErrNone;
}

//
// DMetroTrkChannel::DoResumeThread
//
TInt DMetroTrkChannel::DoResumeThread(DThread *aThread)
{
	LOG_MSG("DMetroTrkChannel::DoResumeThread()");

	if (!aThread)
		return KErrArgument;

	// get the current PC
	TUint32 currentPC = ReadRegister(aThread, PC_REGISTER);

	// if there is a breakpoint at the current PC, we need to single step past it
	for (TInt i=NUMBER_OF_TEMP_BREAKPOINTS; i<iBreakPointList.Count(); i++)
	{
		if (iBreakPointList[i].iAddress == currentPC)
		{
			return DoStepRange(aThread, currentPC, currentPC+1, ETrue, ETrue);
		}
	}

	// PANIC_BACKPORT
	// if frozen use semaphore
	for(TInt i=0; i<iFrozenThreadSemaphores.Count(); i++)
	{
		if(iFrozenThreadSemaphores[i]->iOwningThread == &aThread->iNThread)
		{
			NKern::FSSignal(iFrozenThreadSemaphores[i]);
			NKern::ThreadEnterCS();
			delete iFrozenThreadSemaphores[i];
			NKern::ThreadLeaveCS();
			iFrozenThreadSemaphores.Remove(i);
			return KErrNone;
		}
	}
	// END PANIC_BACKPORT

	// else use ThreadResume
	Kern::ThreadResume(*aThread);
	
	return KErrNone;
}

//
// DMetroTrkChannel::DoStepRange
//
TInt DMetroTrkChannel::DoStepRange(DThread *aThread, const TUint32 aStartAddress, const TUint32 aStopAddress, TBool aStepInto, TBool aResumeOnceOutOfRange, TBool aUserRequest)
{
	LOG_MSG("DMetroTrkChannel::DoStepRange()");

	if (!aThread)
		return KErrArgument;

	TUint32 startAddress = (aStartAddress & 0x1) ? aStartAddress + 1 : aStartAddress;
	TUint32 stopAddress = (aStopAddress & 0x1) ? aStopAddress + 1 : aStopAddress;

	// don't allow the user to step in the excluded ROM region.  this could be called
	// internally however.  for example, the the special breakpoints we set to handle
	// panics, exceptions, and library loaded events are in the user library, and we
	// will need to step past the breakpoint before continuing the thread.
	//if (aUserRequest && (startAddress >= iExcludedROMAddressStart) && (startAddress < iExcludedROMAddressEnd))
	//{
	//	return KErrNotSupported;
	//}

	// set the temp breakpoint, and disable the breakpoint at the current PC if necessary
	// if its not a user request, and we are just trying to resume from a breakpoint, 
	// then we don't need to check for stubs. The last parameter aUserRequest tells 
	// ModifyBreaksForStep to check for stubs or not. In some cases, the check for stubs 
	// is true even if its not a user request.For example, this is true in cases where 
	// we are doing a step range and the instruction in the range modified PC.
	// in this case, DoStepRange will be called from the exception handler where 
	// we need to check for the stubs for the valid behavior. So truly, we don't need to check 
	// for stubs only when resuming from  a breakpoint.
	ReturnIfError(ModifyBreaksForStep(aThread, startAddress, stopAddress, aStepInto, aResumeOnceOutOfRange, aUserRequest));
	
	// PANIC_BACKPORT
	// if frozen use semaphore
	for(TInt i=0; i<iFrozenThreadSemaphores.Count(); i++)
		{
		if(iFrozenThreadSemaphores[i]->iOwningThread == &aThread->iNThread)
			{
			NKern::FSSignal(iFrozenThreadSemaphores[i]);
			NKern::ThreadEnterCS();
			delete iFrozenThreadSemaphores[i];
			NKern::ThreadLeaveCS();
			iFrozenThreadSemaphores.Remove(i);
			return KErrNone;
			}
		}

	// END PANIC_BACKPORT
	// else use ThreadResume
	Kern::ThreadResume(*aThread);
	
	return KErrNone;
}

//
// DMetroTrkChannel::DoReadMemory
//
TInt DMetroTrkChannel::DoReadMemory(DThread *aThread, const TUint32 aAddress, const TInt16 aLength, TDes8 &aData)
{
	LOG_MSG("DMetroTrkChannel::DoReadMemory()");

	// make sure the parameters are valid
	if (aLength > aData.MaxSize())
		return KErrArgument;

	TInt err = KErrNone;
	
	// trap exceptions in case the address is invalid
	XTRAPD(r, XT_DEFAULT, err = TryToReadMemory(aThread, (TAny *)aAddress, (TAny *)aData.Ptr(), aLength));
	
	err = (KErrNone == r) ? err : r;
	
	if (KErrNone == err)
	{
		aData.SetLength(aLength);
		
		TPtr8 data((TUint8 *)aData.Ptr(), aLength, aLength);
		
		// if we have any breakpoints in this range, put the actual instruction in the buffer
		for (TInt i = NUMBER_OF_TEMP_BREAKPOINTS; i < iBreakPointList.Count(); i++)
		{
			if ((iBreakPointList[i].iAddress >= aAddress) && (iBreakPointList[i].iAddress < (aAddress + aLength)))
			{	
				TInt instSize = (TInt)(iBreakPointList[i].iThumbMode ? 2 : 4);								
				memcpy((TAny*)&data[iBreakPointList[i].iAddress - aAddress], (TAny *)iBreakPointList[i].iInstruction.Ptr(), instSize);
			}
		}
	}

	return err;
}

//
// DMetroTrkChannel::DoWriteMemory
//
TInt DMetroTrkChannel::DoWriteMemory(DThread *aThread, const TUint32 aAddress, const TInt16 aLength, TDes8 &aData)
{
	LOG_MSG("DMetroTrkChannel::DoWriteMemory()");

	// make sure the parameters are valid
	if (aLength > aData.Length())
		return KErrArgument;

	TInt err = KErrNone;
	
	// trap exceptions in case the address is invalid
	XTRAPD(r, XT_DEFAULT,  err = TryToWriteMemory(aThread, (TAny *)aAddress, (TAny *)aData.Ptr(), aLength));
	
	err = (KErrNone == r) ? err : r;

	// reset any breakpoints we may have just overwritten
	if (KErrNone == err)
	{
		TPtr8 data((TUint8 *)aData.Ptr(), aLength, aLength);
		
		for (TInt i = NUMBER_OF_TEMP_BREAKPOINTS; i < iBreakPointList.Count(); i++)
		{
			if ((iBreakPointList[i].iAddress >= aAddress) && (iBreakPointList[i].iAddress < (aAddress + aLength)))
			{
				// default to arm mode
				TUint32 inst = KArmBreakPoint;
				TInt instSize = 4;
				
				if (iBreakPointList[i].iThumbMode)
				{
					inst = KThumbBreakPoint;
					instSize = 2;
				}
				
				iBreakPointList[i].iInstruction.Copy(&data[iBreakPointList[i].iAddress - aAddress], instSize);
				//memcpy((TAny*)iBreakPointList[i].iAddress, (TAny *)&inst, instSize);			
				TryToWriteMemory(aThread, (TAny*)iBreakPointList[i].iAddress, (TAny *)&inst, instSize);			
			}
		}
		
	}
	
	return err;
}

//
// DMetroTrkChannel::DoReadRegisters
//
TInt DMetroTrkChannel::DoReadRegisters(DThread *aThread, const TInt16 aFirstRegister, const TInt16 aLastRegister, TDes8 &aValues)
{
	LOG_MSG("DMetroTrkChannel::DoReadRegisters()");

	// make sure the parameters are valid
	if (!aThread || (aFirstRegister < 0) || (aLastRegister >= (TInt16)(sizeof(TArmRegSet)/sizeof(TArmReg))))
		return KErrArgument;

	// make sure the descriptor is big enough to hold the requested data
	if ((TInt)((aLastRegister - aFirstRegister + 1) * sizeof(TArmReg)) > (aValues.MaxSize()))
		return KErrArgument;
	
	TArmRegSet regSet;
    TUint32 unused;

#ifdef SUPPORT_KERNCONTEXT
	NKern::Lock(); // lock the kernel before callin UserContextType as its required by this function
	NThread nThread = aThread->iNThread;
	NThread::TUserContextType userContextType = nThread.UserContextType();
	NKern::Unlock(); //unlock the kernel now
	
	if (userContextType == NThread::EContextNone || userContextType == NThread::EContextKernel)
	{
		//NKern::ThreadGetSystemContext(&aThread->iNThread, &regSet, unused);
		if (!GetSystemThreadRegisters(&regSet))
			return KErrGeneral;
	}
	else
#endif
	{
		NKern::ThreadGetUserContext(&aThread->iNThread, &regSet, unused);
	}
    
    TArmReg *reg = &regSet.iR0;

	if (!reg)
		return KErrGeneral;
			    
    for (TInt16 i = aFirstRegister; i <= aLastRegister; i++)
    	aValues.Append((TUint8 *)&reg[i], sizeof(TArmReg));

	return KErrNone;
}

//
// DMetroTrkChannel::DoWriteRegisters
//
TInt DMetroTrkChannel::DoWriteRegisters(DThread *aThread, const TInt16 aFirstRegister, const TInt16 aLastRegister, TDesC8 &aValues)
{
	LOG_MSG("DMetroTrkChannel::DoWriteRegisters()");

	// make sure the parameters are valid
	if (!aThread || (aFirstRegister < 0) || (aLastRegister >= (TInt16)(sizeof(TArmRegSet)/sizeof(TArmReg))))
		return KErrArgument;

	// make sure the descriptor is big enough to hold the data to write
	if ((TInt)((aLastRegister - aFirstRegister + 1) * sizeof(TArmReg)) > (aValues.Length()))
		return KErrArgument;

    TArmRegSet regSet;
    TUint32 unused;

	NKern::ThreadGetUserContext(&aThread->iNThread, &regSet, unused);
	
    TArmReg *reg = &regSet.iR0;

    for (TInt16 i = aFirstRegister; i <= aLastRegister; i++)
    {
    	#ifndef __OEM_TRK__	
		if (IsRegisterSecure(i))
			return KErrNotSupported;
		#endif
    	reg[i] = *(TUint32 *)&aValues[(i-aFirstRegister)*sizeof(TArmReg)];
    }
    	
    	
    NKern::ThreadSetUserContext(&aThread->iNThread, &regSet);

	return KErrNone;
}

//
// DMetroTrkChannel::DoGetProcessInfo
//
TInt DMetroTrkChannel::DoGetProcessInfo(const TInt aIndex, TMetroTrkTaskInfo *aInfo)
{
	LOG_MSG("DMetroTrkChannel::DoGetProcessInfo()");
	
	DObjectCon *processes = Kern::Containers()[EProcess];
	if (!processes)
		return KErrGeneral;
	
	TInt err = KErrNone;
	
	NKern::ThreadEnterCS(); // Prevent us from dying or suspending whilst holding a DMutex
	processes->Wait(); // Obtain the container mutex so the list does get changed under us
		
	// make sure the index is valid
	if ((aIndex >= 0) && (aIndex < processes->Count())) // >= because the index is zero based
	{
		DProcess *process = (DProcess *)((*processes)[aIndex]);
		
		if (process)
		{
			process->Name(aInfo->iName);
			aInfo->iId = process->iId;
			aInfo->iPriority = (TUint32)process->iPriority;
		}
		else
		{
			LOG_MSG2("Process %d not found", aIndex);
			err = KErrArgument;
		}
	}
	else
	{
		err = KErrArgument;
	}
	
	processes->Signal();
	NKern::ThreadLeaveCS();
	
	return err;
}

//
// DMetroTrkChannel::DoGetThreadInfo
//
TInt DMetroTrkChannel::DoGetThreadInfo(const TInt aIndex, TMetroTrkTaskInfo *aInfo)
{
	LOG_MSG("DMetroTrkChannel::DoGetThreadInfo()");
		
	DObjectCon *threads = Kern::Containers()[EThread];
	if (!threads)
		return KErrGeneral;	
	
	NKern::ThreadEnterCS(); // Prevent us from dying or suspending whilst holding a DMutex
	threads->Wait(); // Obtain the container mutex so the list does get changed under us
	
	TInt err = KErrNone;
	if ((aIndex >= 0) && (aIndex < threads->Count()))
	{
		DThread *thread = NULL;
		
		TInt threadsForProcessCount = 0;
		TBool found = EFalse;
		
		for (TInt i=0; i<threads->Count(); i++)
		{
			thread = (DThread *)((*threads)[i]);
			
			if (thread && (aInfo->iOtherId == thread->iOwningProcess->iId))
			{
				if (threadsForProcessCount == aIndex)
				{
					thread->Name(aInfo->iName);
					aInfo->iId = thread->iId;
					aInfo->iPriority = (TUint32)thread->iThreadPriority;
					found = ETrue;				
					break;
				}
				threadsForProcessCount++;
			}
		}
		
		if (!found)
		{
			LOG_MSG("Thread for process matching index not found");
			err = KErrArgument;
		}
	
	}
	else
	{		
		err = KErrArgument;
	}
	
	threads->Signal();
	NKern::ThreadLeaveCS();

	return err;
}

//
// DMetroTrkChannel::DoGetProcessAddresses
//
TInt DMetroTrkChannel::DoGetProcessAddresses(DThread *aThread, TUint32 &aCodeAddress, TUint32 &aDataAddress)
{
	LOG_MSG("DMetroTrkChannel::DoGetProcessAddresses()");

	if (!aThread)
		return KErrArgument;
	
#ifndef __OEM_TRK__			
	if (HasManufacturerCaps(aThread))
		return KErrPermissionDenied;
#endif
	
	DProcess *process = (DProcess *)aThread->iOwningProcess;
	
	if (!process)
		return KErrArgument;
	
	DCodeSeg* codeSeg = process->iCodeSeg;
	if (!codeSeg)
		return KErrArgument;
		
	TModuleMemoryInfo processMemoryInfo;
	TInt err = codeSeg->GetMemoryInfo(processMemoryInfo, process);
	if (err != KErrNone)
		return err;
	
	aCodeAddress = processMemoryInfo.iCodeBase;
	aDataAddress = processMemoryInfo.iInitialisedDataBase;
	
	//add this process to the list of processes that we are debugging
	TProcessInfo processInfo(process->iId, aCodeAddress, processMemoryInfo.iCodeSize, aDataAddress);
	iDebugProcessList.Append(processInfo);
		
	return KErrNone;
}

//
// DMetroTrkChannel::DoGetStaticLibraryInfo
//

TInt DMetroTrkChannel::DoGetStaticLibraryInfo(const TInt aIndex, SEventInfo *aInfo)
{
	LOG_MSG("DMetroTrkChannel::DoGetStaticLibraryInfo()");
	
	if (!aInfo)
		return KErrArgument;
		
	DThread *thread = ThreadFromId(aInfo->iThreadId);

	if (!thread)
		return KErrArgument;
	
	DProcess *process = (DProcess *)thread->iOwningProcess;
	

	if (!process)
		return KErrArgument;
	
	
	DCodeSeg *processCodeSeg = process->iCodeSeg;
	
	if (!processCodeSeg)
		return KErrArgument;
	
	int count = processCodeSeg->iDepCount;
	LOG_MSG2("code segment count %d", count);

	
	if (aIndex < 0 || aIndex >= count)
		return KErrArgument;
	
	DCodeSeg **codeSegList = processCodeSeg->iDeps;
	if (!codeSegList)
		return KErrArgument;
	
	DCodeSeg* codeSeg = codeSegList[aIndex];
	if (!codeSeg)
	{
		return KErrArgument;
	}
	if (!codeSeg->IsDll())
	{
		LOG_MSG(" -- code segment is not for a dll");
		return KErrArgument;		
	}
	TModuleMemoryInfo memoryInfo;
	TInt err = codeSeg->GetMemoryInfo(memoryInfo, NULL); //NULL for DProcess should be ok;
	if (err != KErrNone)
	{
		LOG_MSG2("Error in getting TModuleMemoryInfo info: %d", err);
		return KErrArgument;
	}

	aInfo->iEventType = SEventInfo::ELibraryLoaded;	
	aInfo->iFileName.Copy(*(codeSeg->iFileName)); //just the name, without uid info.		
	aInfo->iCodeAddress = memoryInfo.iCodeBase;
	aInfo->iDataAddress = memoryInfo.iInitialisedDataBase;
	//this way, host debugger will not resume the thread, instead engine will resume
	//after going through the list of all static libraries..
	aInfo->iThreadId = 0xFFFFFFFF; 
	
	LOG_MSG2("library name: %S", codeSeg->iFileName);		
	LOG_MSG2("code address: %x", aInfo->iCodeAddress);
	LOG_MSG2("data address: %x", aInfo->iDataAddress);

	return KErrNone;
}


//
// DMetroTrkChannel::DoGetLibraryInfo
//
TInt DMetroTrkChannel::DoGetLibraryInfo(TDesC8 &aDllName, TMetroTrkLibInfo *aInfo)
{
	LOG_MSG("DMetroTrkChannel::DoGetLibraryInfo()");
		
	TInt err = KErrArgument;
	
	if (!aInfo)
		return err;
		
	err = DoGetLibInfoFromCodeSegList(aDllName, aInfo);
					
	return err;
}

//
// DMetroTrkChannel::DoGetExeInfo
//
TInt DMetroTrkChannel::DoGetExeInfo(TDesC8 &aExeName, TMetroTrkExeInfo* aExeInfo)
{
	LOG_MSG("DMetroTrkChannel::DoGetExeInfo()");
	
	DObjectCon *processes = Kern::Containers()[EProcess];
	if (!processes)
		return KErrGeneral;
		
	NKern::ThreadEnterCS(); // Prevent us from dying or suspending whilst holding a DMutex
	processes->Wait(); // Obtain the container mutex so the list does get changed under us
			
	TInt err = KErrNotFound; //set err to KErrNotFound
	for (TInt i=0;  i < processes->Count(); i++) // >= because the index is zero based
	{
		DProcess *process = (DProcess *)((*processes)[i]);		
		if (process && (0x0 != aExeInfo->iUid) && (aExeInfo->iUid == process->iUids.iUid[2].iUid))
		{
			DCodeSeg* codeSeg = process->iCodeSeg;
			DThread* mainThread = process->FirstThread();
			if (codeSeg && mainThread)
			{
				aExeInfo->iProcessID = process->iId;
				aExeInfo->iThreadID = mainThread->iId;

				TModuleMemoryInfo memoryInfo;
				err = codeSeg->GetMemoryInfo(memoryInfo, NULL);
				if (KErrNone == err)
				{
					aExeInfo->iCodeAddress = memoryInfo.iCodeBase;
					aExeInfo->iDataAddress = memoryInfo.iInitialisedDataBase;					
					break;
				}								
			}
		}
	}
		
	processes->Signal();
	NKern::ThreadLeaveCS();
					
	return err;
}


//
// DMetroTrkChannel::DoGetProcUidInfo
//
TInt DMetroTrkChannel::DoGetProcUidInfo(TMetroTrkProcUidInfo* aProcUidInfo)
{
	LOG_MSG("DMetroTrkChannel::DoGetProcUidInfo()");	
	
	TInt err = KErrNotFound;
	DProcess* process = ProcessFromId(aProcUidInfo->iProcessID);	
	if (process)
	{
		aProcUidInfo->iUid1 = process->iUids.iUid[0].iUid;
		aProcUidInfo->iUid2 = process->iUids.iUid[1].iUid;
		aProcUidInfo->iUid3 = process->iUids.iUid[2].iUid;
		aProcUidInfo->iSecurID = process->iS.iSecureId;
		aProcUidInfo->iVendorID = process->iS.iVendorId;
		
		err = KErrNone;
	}								
	return err;
}

//
// DMetroTrkChannel::DoGetLibInfoFromCodeSegList
//
TInt DMetroTrkChannel::DoGetLibInfoFromCodeSegList(TDesC8 &aDllName, TMetroTrkLibInfo *aInfo)
{
	LOG_MSG("DMetroTrkChannel::DoGetLibInfoFromCodeSegList()");
		
	TInt err = KErrArgument;

	//get global code seg list
	SDblQue* codeSegList = Kern::CodeSegList();
	if (!codeSegList)
		return KErrBadHandle;		

	//iterate through the list
	for (SDblQueLink* codeSegPtr = codeSegList->First(); codeSegPtr!=(SDblQueLink*) (codeSegList); codeSegPtr=codeSegPtr->iNext)
	{
		DEpocCodeSeg* codeSeg = (DEpocCodeSeg*)_LOFF(codeSegPtr,DCodeSeg, iLink);	
		if (codeSeg && codeSeg->IsDll())
		{
			if (codeSeg->iFileName) //If this is valid, the rootname will also be valid.
			{
				// some dll names from the code segment list have some characters towards the end.
				// Not sure why.
				// To account for this, we need to compare those strings that are 
				// atleast as big as the dll we are looking for.
				// Also the dll names in the code segment list don't have null terminator and so -1.
				if (codeSeg->iRootName.Length() < aDllName.Length()-1)
					continue;
								
				if (!_strnicmp(codeSeg->iRootName.Ptr(), aDllName.Ptr(), aDllName.Length()-1))
				{
					TModuleMemoryInfo memoryInfo;

					TInt err = codeSeg->GetMemoryInfo(memoryInfo, NULL);
					if (err != KErrNone)
					{
						//there's been an error so return it
						return err;
					}							
					
					aInfo->iCodeAddress = memoryInfo.iCodeBase;
					aInfo->iDataAddress = memoryInfo.iInitialisedDataBase;
					if (codeSeg->iAttachProcess) //not valid if dll is used by multiple processes, so not reliable.
					{
						aInfo->iAttachProcessId = codeSeg->iAttachProcess->iId;
						if (codeSeg->iAttachProcess->FirstThread())
							aInfo->iAttachThreadId = codeSeg->iAttachProcess->FirstThread()->iId;
					}
					
					LOG_MSG2("Code segment found for lib: %s", aDllName.Ptr());
					LOG_MSG2("code address: %x", aInfo->iCodeAddress);
					LOG_MSG2("data address: %x", aInfo->iDataAddress);

					return KErrNone;
				}
			}
		}
	}
	
	LOG_MSG2("Code segment not found for lib: %s", aDllName.Ptr());
	return err;
}

//
// DMetroTrkChannel::DoGetLibInfoFromKernLibContainer
//
TInt DMetroTrkChannel::DoGetLibInfoFromKernLibContainer(TDesC8 &aDllName, TMetroTrkLibInfo *aInfo)
{
	LOG_MSG("DMetroTrkChannel::DoGetLibInfoFromKernLibContainer()");
		
	TInt err = KErrNone;	
		
	DObjectCon *libraries = Kern::Containers()[ELibrary];
	
	if (!libraries)
		return KErrGeneral;	

	NKern::ThreadEnterCS(); // Prevent us from dying or suspending whilst holding a DMutex
	libraries->Wait(); // Obtain the container mutex so the list does get changed under us
		
	for (TInt i=0; i<libraries->Count(); i++)
	{
		DLibrary *library = (DLibrary *)((*libraries)[i]);
		
		if (library)
		{
			TBuf<KMaxPath> libName;
			libName.Copy(*(library->iName)); //just copy the name without the UID info.
						
			if (libName.Length() < aDllName.Length()-1)
				continue;
								
			if (!_strnicmp(libName.Ptr(), aDllName.Ptr(), aDllName.Length()-1))
			{			
				//get the code address
				DCodeSeg* codeSeg = library->iCodeSeg;
				if (codeSeg)
				{
					TModuleMemoryInfo memoryInfo;

					TInt err = codeSeg->GetMemoryInfo(memoryInfo, NULL);
					if (err == KErrNone)
					{
						// there's been an error so return it					
						aInfo->iCodeAddress = memoryInfo.iCodeBase;
						aInfo->iDataAddress = memoryInfo.iInitialisedDataBase;						
						// process id and thread id are in DLibrary object.
					}					
				}
				else
				{
					LOG_MSG2("Code segment not available for library %S", library->iName);
					err = KErrNotFound;
				}				
				break;
			}		
		}										
	}

	libraries->Signal();
	NKern::ThreadLeaveCS();
	
	return err;		
}

//
// DMetroTrkChannel::DoSecurityCheck
//
TBool DMetroTrkChannel::DoSecurityCheck()
{
	DProcess* clientProcess = iClientThread->iOwningProcess;
	if (clientProcess)
	{
		// now we also check to make sure that TRK app has ALLFILES capability as well.
		if (!iClientThread->HasCapability(ECapabilityAllFiles))
			return EFalse;
		
		SSecurityInfo secureInfo = clientProcess->iS;
		if ((secureInfo.iSecureId == KTrkSrvSecurUid) || (secureInfo.iSecureId == KTrkAppSecurUid) || (secureInfo.iSecureId == KTrkExeSecurUid))
		{
			return ETrue;
		}
	}
	return EFalse;
}

//
// DMetroTrkChannel::TryToReadMemory
//
TInt DMetroTrkChannel::TryToReadMemory(DThread *aThread, TAny *aSrc, TAny *aDest, TInt16 aLength)
{
	LOG_MSG("DMetroTrkChannel::TryToReadMemory()");
	
	TInt err = KErrNone;
	// on some targets reading from 0xFFFFFFFF address causes a kernel fault.
	// avoid reading the last 4 bytes in the 32 bit address space.
	TUint32 srcAddr = (TUint32)(aSrc);
	if ((srcAddr >= 0xFFFFFFFC) || (aLength > (0xFFFFFFFC-srcAddr)))
		return KErrAccessDenied;

#ifndef __OEM_TRK__
	if (IsAddressInRom((TUint32)(aSrc)) || IsAddressSecure((TUint32)(aSrc)))
		return KErrNotSupported;
#endif

	//check if we have a valid thread object
	if (!aThread)
		return KErrBadHandle;

	LOG_MSG2("Using Kern::ThreadRawRead to read memory at address %x", aSrc);
	err = Kern::ThreadRawRead(aThread, aSrc, aDest, aLength);	

	return err;
}


//
// DMetroTrkChannel::TryToWriteMemory
//
TInt DMetroTrkChannel::TryToWriteMemory(DThread *aThread, TAny *aDest, TAny *aSrc, TInt16 aLength)
{
	LOG_MSG("DMetroTrkChannel::TryToWriteMemory()");

	TInt err = KErrNone;
	// on some targets writing to 0xFFFFFFFF address causes a kernel fault.
	// avoid writing the last 4 bytes in the 32 bit address space.
	TUint32 destAddr = (TUint32)(aDest);
	if ((destAddr>=0xFFFFFFFC) || (aLength > (0xFFFFFFFC-destAddr)))
		return KErrAccessDenied;

#ifndef __OEM_TRK__
	if (IsAddressInRom((TUint32)(aDest)) || IsAddressSecure((TUint32)(aDest)))
		return KErrNotSupported;
#endif

	//check if we have a valid thread object
	if (!aThread)
		return KErrBadHandle;
		
	LOG_MSG2("Using Kern::ThreadRawWrite to write memory at address %x", (TUint32)aDest);
	err = Kern::ThreadRawWrite(aThread, aDest, aSrc, aLength, iClientThread);		

	return err;
}

//
// DMetroTrkChannel::ReadRegister
//
TInt32 DMetroTrkChannel::ReadRegister(DThread *aThread, TInt aNum)
{
	LOG_MSG("DMetroTrkChannel::ReadRegister()");

	if (!aThread || (aNum < 0) || (aNum >= (TInt16)(sizeof(TArmRegSet)/sizeof(TArmReg))))
	{
		LOG_MSG2("Invalid register number (%d) passed to ReadRegister", aNum);
		return 0;
	}
	
	TArmRegSet regSet;
    TUint32 unused;

#ifdef SUPPORT_KERNCONTEXT	
  	NKern::Lock(); // lock the kernel before callin UserContextType as its required by this function
	NThread nThread = aThread->iNThread;
	NThread::TUserContextType userContextType = nThread.UserContextType();
	NKern::Unlock(); // unlock the kernel now
	
	if (userContextType == NThread::EContextNone || userContextType == NThread::EContextKernel)
	{
		//NKern::ThreadGetSystemContext(&aThread->iNThread, &regSet, unused);
		if (!GetSystemThreadRegisters(&regSet))
			return KErrGeneral;
	}
	else
#endif	
	{
		NKern::ThreadGetUserContext(&aThread->iNThread, &regSet, unused);
	}
    
    TArmReg *reg = &regSet.iR0;
	
	return ((TUint32 *)reg)[aNum];
}

//
// DMetroTrkChannel::ShiftedRegValue
//
TUint32 DMetroTrkChannel::ShiftedRegValue(DThread *aThread, TUint32 aInstruction, TUint32 aCurrentPC, TUint32 aStatusRegister)
{
	LOG_MSG("DMetroTrkChannel::ShiftedRegValue()");

	TUint32 shift = 0;
	if (aInstruction & 0x10)	// bit 4
	{
		shift = (ARM_RS(aInstruction) == PC_REGISTER ? aCurrentPC + 8 : aStatusRegister) & 0xFF;
	}
	else
	{
		shift = ARM_DATA_C(aInstruction);
	}
	
	TInt rm = ARM_RM(aInstruction);
	TUint32 res = (rm == PC_REGISTER ? (aCurrentPC + ((aInstruction & 0x10) ? 12 : 8)) : ReadRegister(aThread, rm));

	switch(ARM_DATA_SHIFT(aInstruction))
	{
		case 0:			// LSL
		{
			res = shift >= 32 ? 0 : res << shift;
			break;
		}
		case 1:			// LSR
		{
			res = shift >= 32 ? 0 : res >> shift;
			break;
		}
		case 2:			// ASR
		{
			if (shift >= 32)
			shift = 31;
			res = ((res & 0x80000000L) ? ~((~res) >> shift) : res >> shift);
			break;
		}
		case 3:			// ROR/RRX
		{
			shift &= 31;
			if (shift == 0)
			{
				res = (res >> 1) | ((aStatusRegister & ARM_CARRY_BIT) ? 0x80000000L : 0);
			}
			else
			{
				res = (res >> shift) | (res << (32 - shift));
			}
			break;
    	}
    }

  	return res & 0xFFFFFFFF;
}


//
// DMetroTrkChannel::ModifyBreaksForStep
//
// Set a temporary breakpoint at the next instruction to be executed after the one at the current PC
// Disable the breakpoint at the current PC if one exists
//
TInt DMetroTrkChannel::ModifyBreaksForStep(DThread *aThread, TUint32 aRangeStart, TUint32 aRangeEnd, TBool aStepInto, TBool aResumeOnceOutOfRange, TBool aCheckForStubs)
{
	LOG_MSG("DMetroTrkChannel::ModifyBreaksForStep()");
	
	if (!aThread)
		return KErrArgument;

	LOG_MSG2("Range Start: %x", aRangeStart);
	LOG_MSG2("Range End: %x", aRangeEnd);

	// get the current PC
	TUint32 currentPC = ReadRegister(aThread, PC_REGISTER);
	LOG_MSG2("Current PC: %x", currentPC);

	// disable breakpoint at the current PC if necessary
	ReturnIfError(DisableBreakAtAddress(currentPC));

	// get the status register
	TUint32 statusRegister = ReadRegister(aThread, STATUS_REGISTER);
	LOG_MSG2("Current SR: %x", statusRegister);

	TBool thumbMode = (statusRegister & ECpuThumb);
	if (thumbMode)
		LOG_MSG("Thumb Mode");

	TInt instSize = thumbMode ? 2 : 4;

	TBool changingModes = EFalse;
	
	TUint32 breakAddress = 0;

	TInt rangeSize = aRangeEnd - currentPC;
	
	const TInt KMaxInstructionBuffer = 80;
	
	// scan the memory and see if any instruction might modify the PC.  if one does,
	// stop scanning and just set a breakpoint at that instruction (with the range set accordingly).
	// if none is found just set the breakpoint after the range.
	// if there is only one instruction there is no need to parse the memory, just execute it
	if ((rangeSize <= KMaxInstructionBuffer) && (rangeSize > instSize))
	{
		LOG_MSG("Scanning range for instructions that might modify the PC");
		
		// set it to the end of the range by default
		breakAddress = aRangeEnd;

		// get the instructions in range
		// we really should be dynamically allocating this memory, but this could be
		// called from another thread, so we can't
		TBuf8<KMaxInstructionBuffer> instructions;
			
		ReturnIfError(DoReadMemory(aThread, currentPC, rangeSize, instructions));
		
		for (TInt i = 0; i < (TInt)rangeSize/instSize; i++)
		{
			if (InstructionModifiesPC(aThread, &instructions[i*instSize], thumbMode, aStepInto))
			{
				breakAddress = currentPC + i*instSize;
				LOG_MSG2("Setting breakpoint at %x inside range", breakAddress);
				break;
			}
		}
	}
	

	TUint32 newRangeEnd = aRangeEnd;

	if ((breakAddress == 0) || (breakAddress == currentPC))
	{
		// either the range consists of a single instruction, or the instruction at the currentPC may modify the PC
		// decode the instruction and see where we need to set the breakpoint
		breakAddress = PCAfterInstructionExecutes(aThread, currentPC, statusRegister, instSize, aStepInto, newRangeEnd, changingModes);
				
		// check to see if this is one of the stubs (found in stubs.s)
		if (aStepInto && aCheckForStubs)
		{
			TBuf8<16> destination;
			TInt err = DoReadMemory(aThread, breakAddress, 16, destination);

			if (KErrNone == err)
			{
				TInt offset = 0;
				
				if (0 == destination.Find(KArm4Stub, sizeof(KArm4Stub)))
				{
					LOG_MSG("Arm4 stub found");
					offset = 8;
				}
				else if ((0 == destination.Find(KArmIStub, sizeof(KArmIStub))) ||
						 (0 == destination.Find(KFastArmIStub, sizeof(KFastArmIStub))))
				{
					LOG_MSG("ArmI stub found");
					offset = 12;
				}
				else if ((0 == destination.Find(KThumbStub, sizeof(KThumbStub))) ||
						 (0 == destination.Find(KFastThumbStub, sizeof(KFastThumbStub))))
				{
					LOG_MSG("Thumb stub found");
					offset = 12;
				}
				else if ((0 == destination.Find(KThumbStub2, sizeof(KThumbStub2))) ||
						 (0 == destination.Find(KFastThumbStub2, sizeof(KFastThumbStub2))))
				{
					LOG_MSG("Thumb stub found");
					offset = 8;
				}
				//check to see if this is the stub generated with RVCT tools. 
				//Look into genstubs.cpp for more details on this stub
				else if(0 == destination.Find(KRvctArm4Stub, sizeof(KRvctArm4Stub)))
				{
					LOG_MSG("RVCT Arm4 stub found");
					offset = 4;
				}

				
				if (offset != 0)
				{
					if (offset == 4)
					{
						breakAddress = *(TUint32 *)&destination[offset];
					}
					else
					{
						err = DoReadMemory(aThread, *(TUint32 *)&destination[offset], 4, destination);
						if (KErrNone == err)
							breakAddress = *(TUint32 *)destination.Ptr();
					}
					
					if (KErrNone == err)
					{						
						if (thumbMode)
						{
							if ((breakAddress & 0x00000001) == 1)
								changingModes = EFalse;
							else
								changingModes = ETrue;
						}
						else
						{
							if ((breakAddress & 0x00000001) == 1)
								changingModes = ETrue;
							else
								changingModes = EFalse;	
						}
						
						breakAddress &= 0xFFFFFFFE;
					}
					else
					{
						LOG_MSG("Error reading destination of stub");
					}
				}
			}
			else
			{
				LOG_MSG("Error reading memory while decoding branch instruction");
			}					
		}
		
		// don't allow the user to step in a function in the excluded ROM region.
		//if ((breakAddress >= iExcludedROMAddressStart) && (breakAddress < iExcludedROMAddressEnd))
		//{
		//	breakAddress = currentPC + instSize;
		//	changingModes = EFalse;
		//}
	}
	
	// see if there is already a breakpoint at this address.  if there is, we do not need to set the temp breakpoint
	for (TInt i=NUMBER_OF_TEMP_BREAKPOINTS; i<iBreakPointList.Count(); i++)
	{
		if (iBreakPointList[i].iAddress == breakAddress)
		{
			return KErrNone;
		}
	}
	
	for (TInt i=0; i<NUMBER_OF_TEMP_BREAKPOINTS; i++)
	{
		if (iBreakPointList[i].iAddress == 0)
		{
			iBreakPointList[i].iThreadId = aThread->iId;
			iBreakPointList[i].iAddress = breakAddress;
			iBreakPointList[i].iThumbMode = (thumbMode && !changingModes) || (!thumbMode && changingModes);
			iBreakPointList[i].iResumeOnceOutOfRange = aResumeOnceOutOfRange;
			iBreakPointList[i].iSteppingInto = aStepInto;
			iBreakPointList[i].iRangeStart = aRangeStart;
			iBreakPointList[i].iRangeEnd = newRangeEnd;
			
			// Temporary breakpoints are always thread specific
			iBreakPointList[i].iThreadSpecific = ETrue;

			LOG_MSG2("Adding temp breakpoint with id: %d", i);
			LOG_MSG2("Adding temp breakpoint with thread id: %d", aThread->iId);

			return DoEnableBreak(iBreakPointList[i], ETrue);			
		}
	}
	
	return KErrNoMemory;
}

//
// DMetroTrkChannel::ClearAllBreakPoints
//
void DMetroTrkChannel::ClearAllBreakPoints()
{
	LOG_MSG("DMetroTrkChannel::ClearAllBreakPoints()");

	TInt err = KErrNone;

	for (TInt i=0; i<iBreakPointList.Count(); i++)
	{		
		if ((iBreakPointList[i].iAddress != 0) && !iBreakPointList[i].iObsoleteLibraryBreakpoint)
		{
			LOG_MSG2("Clearing breakpoint at address %x", iBreakPointList[i].iAddress);
			
			DThread* threadObj = ThreadFromId(iBreakPointList[i].iThreadId);
		
			if (threadObj != NULL)
			{
				XTRAPD(r, XT_DEFAULT, err = DebugSupport::RestoreCode(threadObj, iBreakPointList[i].iAddress));
		
				err = (KErrNone == r) ? err : r;
			}
			else
			{
				err = KErrBadHandle;
			}
				
			if (KErrNone != err)
			{
				LOG_MSG2("Error %d while clearing breakpoint", err);
			}		
		}
	}
	
	iBreakPointList.Reset();	
}


//
// DMetroTrkChannel::DisableBreakAtAddress
//
TInt DMetroTrkChannel::DisableBreakAtAddress(TUint32 aAddress)
{
	LOG_MSG("DMetroTrkChannel::DisableBreakAtAddress()");

	TInt err = KErrNone;
	
	for (TInt i = NUMBER_OF_TEMP_BREAKPOINTS; i < iBreakPointList.Count(); i++)
	{
		if (iBreakPointList[i].iAddress == aAddress)
		{
			iBreakPointList[i].iDisabledForStep = ETrue;
			LOG_MSG2("Disabling breakpoint at address %x", iBreakPointList[i].iAddress);
		
			DThread* threadObj = ThreadFromId(iBreakPointList[i].iThreadId);

			//clear the breakpoint with code modifier
			//code modifier will restore the org instruction and also frees the shadow page if necessary
			if (threadObj != NULL)
			{
				XTRAPD(r, XT_DEFAULT, err = DebugSupport::RestoreCode(threadObj, aAddress));
		
				err = (KErrNone == r) ? err : r;
			}
			else
			{
				err = KErrBadHandle;
			}
			break;
		}
	}
	
	return err;
}

//
// DMetroTrkChannel::InstructionModifiesPC
//
TBool DMetroTrkChannel::InstructionModifiesPC(DThread *aThread, TUint8 *aInstruction, TBool aThumbMode, TBool aStepInto)
{
	LOG_MSG("DMetroTrkChannel::InstructionModifiesPC()");

	if (aThumbMode)
	{
		TUint16 inst = *(TUint16 *)aInstruction;
		
		switch(THUMB_OPCODE(inst))
		{
			case 0x08:
			{
				if (aStepInto && (THUMB_INST_7_15(inst) == 0x08F))
				{
					// BLX(2)
					return ETrue;				
				}
				else if (THUMB_INST_7_15(inst) == 0x08E)
				{
					// BX
					return ETrue;				
				}
				else if (((THUMB_INST_8_15(inst) == 0x46) || (THUMB_INST_8_15(inst) == 0x44)) && ((inst & 0x87) == 0x87))
				{
					// ADD or MOV with PC as the destination
					return ETrue;				
				}
				break;
			}
			case 0x13:
			{
				if (THUMB_INST_8_15(inst) == 0x9F)
				{
					// LDR(4) with the PC as the destination
					return ETrue;				
				}
				break;
			}
			case 0x17:
			{	
				if (THUMB_INST_8_15(inst) == 0xBD)
				{
					// POP with the PC in the list
					return ETrue;				
				}
				break;
			}
			case 0x1A:
			case 0x1B:
			{	
				if (THUMB_INST_8_15(inst) < 0xDE)
				{
					// B(1) conditional branch
					return ETrue;				
				}
				break;
			}
			case 0x1C:
			{	
				// B(2) unconditional branch
				return ETrue;				
			}
			case 0x1D:
			{
				// NOTE: Only return true for the suffix since it's the second instruction that actually does the branch
				if (aStepInto && !(inst & 0x00000001))
				{
					// BLX(1)
					return ETrue;				
				}
				break;
			}
			case 0x1F:
			{
				// NOTE: Only return true for the suffix since it's the second instruction that actually does the branch
				if (aStepInto)
				{
					// BL
					return ETrue;				
				}
				break;
			}
		}
	}
	else
	{
		// Arm mode
		TUint32 inst = *(TUint32 *)aInstruction;

		switch(ARM_OPCODE(inst)) // bits 27-25
		{
			case 0:
			{
				switch((inst & 0x00000010) >> 4) // bit 4
				{
					case 0:
					{
						switch((inst & 0x01800000) >> 23) // bits 24-23
						{
							case 2:
							{
								// move to/from status register.  pc updates not allowed
								// or TST, TEQ, CMP, CMN which don't modify the PC
								break;
							}
							default:
							{
								// Data processing immediate shift
								if (ARM_RD(inst) == PC_REGISTER)
								{
									// destination register is the PC
									if (IsPreviousInstructionMovePCToLR(aThread))
									{
										return aStepInto;
									}
									else
									{
										return ETrue;
									}										
								}
								break;
							}
						}
						break;
					}					
					case 1:
					{
						switch((inst & 0x00000080) >> 7) // bit 7
						{
							case 0:
							{
								switch((inst & 0x01900000) >> 20) // bits 24-23 and bit 20
								{
									case 0x10:
									{
										// from figure 3-3
										switch((inst & 0x000000F0) >> 4) // bits 7-4
										{
											case 1:
											{
												if (((inst & 0x00400000) >> 22) == 0) // bit 22
												{
													// BX
													if (IsPreviousInstructionMovePCToLR(aThread))
													{
														return aStepInto;
													}
													else
													{
														return ETrue;
													}										
												}
												break;
											}
											case 3:
											{
												// BLX
												if (aStepInto)
												{
													return ETrue;
												}
												break;
											}
											default:
											{
												// either doesn't modify the PC or it is illegal to
												break;
											}
										}
										break;
									}
									default:
									{
										// Data processing register shift
										if (((inst & 0x01800000) >> 23) == 2) // bits 24-23
										{
											// TST, TEQ, CMP, CMN don't modify the PC
											return EFalse;
										}
										else if (ARM_RD(inst) == PC_REGISTER)
										{
											// destination register is the PC
											if (IsPreviousInstructionMovePCToLR(aThread))
											{
												return aStepInto;
											}
											else
											{
												return ETrue;
											}										
										}
										break;
									}
								}
								break;
							}
							default:
							{
								// from figure 3-2, updates to the PC illegal
								break;
							}
						}
						break;
					}
				}
				break;
			}
			case 1:
			{
				if (((inst & 0x01800000) >> 23) == 2) // bits 24-23
				{
					// cannot modify the PC
					break;
				}
				else if (ARM_RD(inst) == PC_REGISTER)
				{
					// destination register is the PC
					if (IsPreviousInstructionMovePCToLR(aThread))
					{
						return aStepInto;
					}
					else
					{
						return ETrue;
					}										
				}
				break;
			}
			case 2:
			{
				// load/store immediate offset
				if (ARM_LOAD(inst)) // bit 20
				{
					// loading a register from memory
					if (ARM_RD(inst) == PC_REGISTER)
					{
						// loading the PC register
						if (IsPreviousInstructionMovePCToLR(aThread))
						{
							return aStepInto;
						}
						else
						{
							return ETrue;
						}										
					}
				}	
				break;
			}
			case 3:
			{
				if (((inst & 0xF0000000) != 0xF) && ((inst & 0x00000010) == 0))
				{
					// load/store register offset
					if (ARM_LOAD(inst)) // bit 20
					{
						// loading a register from memory
						if (ARM_RD(inst) == PC_REGISTER)
						{
							// loading the PC register
							if (IsPreviousInstructionMovePCToLR(aThread))
							{
								return aStepInto;
							}
							else
							{
								return ETrue;
							}										
						}
					}	
				}
				break;
			}
			case 4:
			{
				if ((inst & 0xF0000000) != 0xF)
				{
					// load/store multiple
					if (ARM_LOAD(inst)) // bit 20
					{
						// loading a register from memory
						if (((inst & 0x00008000) >> 15))
						{
							// loading the PC register
							return ETrue;
						}
					}					
				}
				break;
			}
			case 5:
			{
				if ((inst & 0xF0000000) == 0xF)
				{
					// BLX
					if (aStepInto)
					{
						return ETrue;
					}
				}
				else
				{
					if ((inst & 0x01000000)) // bit 24
					{
						// BL
						if (aStepInto)
						{
							return ETrue;
						}
					}
					else
					{
						// B
						return ETrue;
					}
				}
				break;
			}
		}	
	}
	
	return EFalse;
}

//
// DMetroTrkChannel::PCAfterInstructionExecutes
//
TUint32 DMetroTrkChannel::PCAfterInstructionExecutes(DThread *aThread, TUint32 aCurrentPC, TUint32 aStatusRegister, TInt aInstSize, TBool aStepInto, TUint32 &aNewRangeEnd, TBool &aChangingModes)
{
	LOG_MSG("DMetroTrkChannel::PCAfterInstructionExecutes()");

	// by default we will set the breakpoint at the next instruction
	TUint32 breakAddress = aCurrentPC + aInstSize;
	
	// get the instruction at the current PC
	TBuf8<4> instruction;
	TInt err = DoReadMemory(aThread, aCurrentPC, aInstSize, instruction); 

	if (KErrNone != err)
	{
		return breakAddress;
	}
	
	if (4 == aInstSize)
	{
		TUint32 inst = *(TUint32 *)instruction.Ptr();
		LOG_MSG2("Current instruction: %x", inst);

		// check the conditions to see if this will actually get executed
		if (IsExecuted(((inst>>28) & 0x0000000F), aStatusRegister)) 
		{
			switch(ARM_OPCODE(inst)) // bits 27-25
			{
				case 0:
				{
					switch((inst & 0x00000010) >> 4) // bit 4
					{
						case 0:
						{
							switch((inst & 0x01800000) >> 23) // bits 24-23
							{
								case 2:
								{
									// move to/from status register.  pc updates not allowed
									// or TST, TEQ, CMP, CMN which don't modify the PC
									break;
								}
								default:
								{
									// Data processing immediate shift
									if (ARM_RD(inst) == PC_REGISTER)
									{
										// destination register is the PC
										if (IsPreviousInstructionMovePCToLR(aThread) && !aStepInto)
										{
											return breakAddress;
										}

										TUint32 rn = aCurrentPC + 8;
										if (ARM_RN(inst) != PC_REGISTER) // bits 19-16
										{
											rn = ReadRegister(aThread, ARM_RN(inst));
										}
										
										TUint32 shifter = ShiftedRegValue(aThread, inst, aCurrentPC, aStatusRegister);
										
										DecodeDataProcessingInstruction(((inst & 0x01E00000) >> 21), rn, shifter, aStatusRegister, breakAddress);
									}
									break;
								}
							}
							break;
						}					
						case 1:
						{
							switch((inst & 0x00000080) >> 7) // bit 7
							{
								case 0:
								{
									switch((inst & 0x01900000) >> 20) // bits 24-23 and bit 20
									{
										case 0x10:
										{
											// from figure 3-3
											switch((inst & 0x000000F0) >> 4) // bits 7-4
											{
												case 1:
												{
													if (((inst & 0x00400000) >> 22) == 0) // bit 22
													{
														// BX
														// this is a strange case.  normally this is used in the epilogue to branch the the link
														// register.  sometimes it is used to call a function, and the LR is stored in the previous
														// instruction.  since what we want to do is different for the two cases when stepping over,
														// we need to read the previous instruction to see what we should do
														if (IsPreviousInstructionMovePCToLR(aThread) && !aStepInto)
														{
															return breakAddress;
														}
														
														breakAddress = ReadRegister(aThread, (inst & 0x0000000F));
														
														if ((breakAddress & 0x00000001) == 1)
														{
															aChangingModes = ETrue;
														}
														
														breakAddress &= 0xFFFFFFFE;
													}
													break;
												}
												case 3:
												{
													// BLX
													if (aStepInto)
													{
														breakAddress = ReadRegister(aThread, (inst & 0x0000000F));

														if ((breakAddress & 0x00000001) == 1)
														{
															aChangingModes = ETrue;
														}
														
														breakAddress &= 0xFFFFFFFE;
													}
													break;
												}
												default:
												{
													// either doesn't modify the PC or it is illegal to
													break;
												}
											}
											break;
										}
										default:
										{
											// Data processing register shift
											if (((inst & 0x01800000) >> 23) == 2) // bits 24-23
											{
												// TST, TEQ, CMP, CMN don't modify the PC
											}
											else if (ARM_RD(inst) == PC_REGISTER)
											{
												// destination register is the PC
												if (IsPreviousInstructionMovePCToLR(aThread) && !aStepInto)
												{
													return breakAddress;
												}

												TUint32 rn = aCurrentPC + 8;
												if (ARM_RN(inst) != PC_REGISTER) // bits 19-16
												{
													rn = ReadRegister(aThread, ARM_RN(inst));
												}
												
												TUint32 shifter = ShiftedRegValue(aThread, inst, aCurrentPC, aStatusRegister);
												
												DecodeDataProcessingInstruction(((inst & 0x01E00000) >> 21), rn, shifter, aStatusRegister, breakAddress);
											}
											break;
										}
									}
									break;
								}
								default:
								{
									// from figure 3-2, updates to the PC illegal
									break;
								}
							}
							break;
						}
					}
					break;
				}
				case 1:
				{
					if (((inst & 0x01800000) >> 23) == 2) // bits 24-23
					{
						// cannot modify the PC
						break;
					}
					else if (ARM_RD(inst) == PC_REGISTER)
					{
						// destination register is the PC
						if (IsPreviousInstructionMovePCToLR(aThread) && !aStepInto)
						{
							return breakAddress;
						}

						TUint32 rn = ReadRegister(aThread, ARM_RN(inst)); // bits 19-16
						TUint32 shifter = ((ARM_DATA_IMM(inst) >> ARM_DATA_ROT(inst)) | (ARM_DATA_IMM(inst) << (32 - ARM_DATA_ROT(inst)))) & 0xffffffff;

						DecodeDataProcessingInstruction(((inst & 0x01E00000) >> 21), rn, shifter, aStatusRegister, breakAddress);
					}
					break;
				}
				case 2:
				{
					// load/store immediate offset
					if (ARM_LOAD(inst)) // bit 20
					{
						// loading a register from memory
						if (ARM_RD(inst) == PC_REGISTER)
						{
							// loading the PC register
							if (IsPreviousInstructionMovePCToLR(aThread) && !aStepInto)
							{
								return breakAddress;
							}

					    	TUint32 base = ReadRegister(aThread, ARM_RN(inst));
					    	TUint32 offset = 0;
					    	
					    	if (ARM_SINGLE_PRE(inst))
					    	{
					    		// Pre-indexing
					    		offset = ARM_SINGLE_IMM(inst);
							    
							    if (ARM_SINGLE_U(inst))
							    {
							    	base += offset;
							    }
							    else
							    {
							    	base -= offset;
								}
							}

							TBuf8<4> destination;
							TInt err = DoReadMemory(aThread, base, 4, destination);
							
							if (KErrNone == err)
							{
								breakAddress = *(TUint32 *)destination.Ptr();
							
								if ((breakAddress & 0x00000001) == 1)
								{
									aChangingModes = ETrue;
								}								
								breakAddress &= 0xFFFFFFFE;
							}
							else
							{
								LOG_MSG("Error reading memory in decoding step instruction");
							}
						}
					}	
					break;
				}
				case 3:
				{
					if (((inst & 0xF0000000) != 0xF) && ((inst & 0x00000010) == 0))
					{
						// load/store register offset
						if (ARM_LOAD(inst)) // bit 20
						{
							// loading a register from memory
							if (ARM_RD(inst) == PC_REGISTER)
							{
								// loading the PC register
								if (IsPreviousInstructionMovePCToLR(aThread) && !aStepInto)
								{
									return breakAddress;
								}

						    	TUint32 base = (ARM_RN(inst) == PC_REGISTER) ? aCurrentPC + 8 : ReadRegister(aThread, ARM_RN(inst));
						    	TUint32 offset = 0;
						    	
						    	if (ARM_SINGLE_PRE(inst))
						    	{
							      	offset = ShiftedRegValue(aThread, inst, aCurrentPC, aStatusRegister);

								    if (ARM_SINGLE_U(inst))
								    {
								    	base += offset;
								    }
								    else
								    {
								    	base -= offset;
									}
								}

								TBuf8<4> destination;
								TInt err = DoReadMemory(aThread, base, 4, destination);
								
								if (KErrNone == err)
								{
									breakAddress = *(TUint32 *)destination.Ptr();
									
									if ((breakAddress & 0x00000001) == 1)
									{
										aChangingModes = ETrue;
									}								
									breakAddress &= 0xFFFFFFFE;
								}
								else
								{
									LOG_MSG("Error reading memory in decoding step instruction");
								}
							}
						}	
					}
					break;
				}
				case 4:
				{
					if ((inst & 0xF0000000) != 0xF)
					{
						// load/store multiple
						if (ARM_LOAD(inst)) // bit 20
						{
							// loading a register from memory
							if (((inst & 0x00008000) >> 15))
							{
								// loading the PC register
								TInt offset = 0;	
								if (ARM_BLOCK_U(inst))
								{
									TUint32 reglist = ARM_BLOCK_REGLIST(inst);
									offset = Bitcount(reglist) * 4 - 4;
									if (ARM_BLOCK_PRE(inst))
										offset += 4;
								}
								else if (ARM_BLOCK_PRE(inst))
								{
									offset = -4;
								}
									
						    	TUint32 temp = ReadRegister(aThread, ARM_RN(inst));
								
								temp += offset;

								TBuf8<4> destination;
								TInt err = DoReadMemory(aThread, temp, 4, destination);
								
								if (KErrNone == err)
								{
									breakAddress = *(TUint32 *)destination.Ptr();
									if ((breakAddress & 0x00000001) == 1)
									{
										aChangingModes = ETrue;
									}
									breakAddress &= 0xFFFFFFFE;
								}
								else
								{
									LOG_MSG("Error reading memory in decoding step instruction");
								}
							}
						}					
					}
					break;
				}
				case 5:
				{
					if ((inst & 0xF0000000) == 0xF)
					{
						// BLX
						if (aStepInto)
						{
							breakAddress = (TUint32)ARM_INSTR_B_DEST(inst, aCurrentPC);

							if ((breakAddress & 0x00000001) == 1)
							{
								aChangingModes = ETrue;
							}
							
							breakAddress &= 0xFFFFFFFE;
						}
					}
					else
					{
						if ((inst & 0x01000000)) // bit 24
						{
							// BL
							if (aStepInto)
							{
								breakAddress = (TUint32)ARM_INSTR_B_DEST(inst, aCurrentPC);
							}
						}
						else
						{
							// B
							breakAddress = (TUint32)ARM_INSTR_B_DEST(inst, aCurrentPC);
						}
					}
					break;
				}
			}	
		}
	}
	else
	{
		// Thumb Mode
		TUint16 inst = *(TUint16 *)instruction.Ptr();
		LOG_MSG2("Current instruction: %x", inst);

		switch(THUMB_OPCODE(inst))
		{
			case 0x08:
			{
				if (aStepInto && (THUMB_INST_7_15(inst) == 0x08F))
				{
					// BLX(2)
					breakAddress = ReadRegister(aThread, ((inst & 0x0078) >> 3));

					if ((breakAddress & 0x00000001) == 0)
					{
						aChangingModes = ETrue;
					}
					
					breakAddress &= 0xFFFFFFFE;
				}
				else if (THUMB_INST_7_15(inst) == 0x08E)
				{
					// BX
					breakAddress = ReadRegister(aThread, ((inst & 0x0078) >> 3));

					if ((breakAddress & 0x00000001) == 0)
					{
						aChangingModes = ETrue;
					}
					
					breakAddress &= 0xFFFFFFFE;
				}
				else if ((THUMB_INST_8_15(inst) == 0x46) && ((inst & 0x87) == 0x87))
				{
					// MOV with PC as the destination
					breakAddress = ReadRegister(aThread, ((inst & 0x0078) >> 3));
				}
				else if ((THUMB_INST_8_15(inst) == 0x44) && ((inst & 0x87) == 0x87))
				{
					// ADD with PC as the destination
					breakAddress = aCurrentPC + ReadRegister(aThread, ((inst & 0x0078) >> 3));
				}
				break;
			}
			case 0x13:
			{
				//This instruction doesn't modify the PC.

				//if (THUMB_INST_8_15(inst) == 0x9F)
				//{
					// LDR(4) with the PC as the destination
				//	breakAddress = ReadRegister(aThread, SP_REGISTER) + (4 * (inst & 0x00FF));
				//}
				break;
			}
			case 0x17:
			{	
				if (THUMB_INST_8_15(inst) == 0xBD)
				{
					// POP with the PC in the list
					TUint32 regList = (inst & 0x00FF);
					TInt offset = ReadRegister(aThread, SP_REGISTER) + (Bitcount(regList) * 4);

					TBuf8<4> destination;
					TInt err = DoReadMemory(aThread, offset, 4, destination);
					
					if (KErrNone == err)
					{
						breakAddress = *(TUint32 *)destination.Ptr();

						if ((breakAddress & 0x00000001) == 0)
						{
							aChangingModes = ETrue;
						}

						breakAddress &= 0xFFFFFFFE;
					}
					else
					{
						LOG_MSG("Error reading memory in decoding step instruction");
					}
				}
				break;
			}
			case 0x1A:
			case 0x1B:
			{	
				if (THUMB_INST_8_15(inst) < 0xDE)
				{
					// B(1) conditional branch
					if (IsExecuted(((inst & 0x0F00) >> 8), aStatusRegister))
					{
						TUint32 offset = ((inst & 0x000000FF) << 1);
						if (offset & 0x00000100)
						{
							offset |= 0xFFFFFF00;
						}
						
						breakAddress = aCurrentPC + 4 + offset;
					}
				}
				break;
			}
			case 0x1C:
			{	
				// B(2) unconditional branch
				TUint32 offset = (inst & 0x000007FF) << 1;
				if (offset & 0x00000800)
				{
					offset |= 0xFFFFF800;
				}
				
				breakAddress = aCurrentPC + 4 + offset;
				break;
			}
			case 0x1D:
			{
				if (aStepInto && !(inst & 0x0001))
				{
					// BLX(1)
					breakAddress = (ReadRegister(aThread, LINK_REGISTER) + ((inst & 0x07FF) << 1));
					if ((breakAddress & 0x00000001) == 0)
					{
						aChangingModes = ETrue;
					}
					
					breakAddress &= 0xFFFFFFFC;
				}
				break;
			}
			case 0x1E:
			{
				// BL/BLX prefix - destination is encoded in this and the next instruction
				aNewRangeEnd += 2;
                // BL Stepping Changes
                if (aStepInto)
                {
                    TBuf8<2> nextInstruction;
                    TInt err = DoReadMemory(aThread, aCurrentPC+2, aInstSize, nextInstruction);
                    if (KErrNone == err)
                    {
                        TUint16 nextInst = *(TUint16*)nextInstruction.Ptr();
                        LOG_MSG2("Next instruction: %x", nextInst);
                        
                        // base new LR from first instruction
                        TUint32 newLR = inst & 0x07FF;
                        newLR <<= 12;
                        if (newLR & (TUint32)(0x0400 << 12))
                        {
                            // sign-extend it
                            newLR |= 0xFF800000;
                        }
                        newLR += aCurrentPC + 4;        // pc is pointing to first instruction + 4
                        TUint32 newPC = newLR + ((nextInst & 0x07FF) << 1);
                        if (THUMB_OPCODE(nextInst) == 0x1D)         // H = 01 changing to ARM (BLX)
                        {
                            newPC &= 0xFFFFFFFC;
                            aChangingModes = ETrue;
                        }
                        
                        breakAddress = newPC;
                    }
                }
                else    // step over
                {
                    breakAddress += 2;
                }
                // End of BL Stepping Changes
				break;
			}
			case 0x1F:
			{
				if (aStepInto)
				{
					// BL
					breakAddress = ReadRegister(aThread, LINK_REGISTER) + ((inst & 0x07FF) << 1);
				}
				break;
			}
		}
	}
	
	return breakAddress;
}


//
// DMetroTrkChannel::DecodeDataProcessingInstruction
//
void DMetroTrkChannel::DecodeDataProcessingInstruction(TUint8 aOpcode, TUint32 aOp1, TUint32 aOp2, TUint32 aStatusRegister, TUint32 &aBreakAddress)
{
	LOG_MSG("DMetroTrkChannel::DecodeDataProcessingInstruction()");

	switch(aOpcode)
	{
		case 0:
		{
			// AND
			aBreakAddress = aOp1 & aOp2;
			break;
		}
		case 1:
		{
			// EOR
			aBreakAddress = aOp1 ^ aOp2;
			break;
		}
		case 2:
		{
			// SUB
			aBreakAddress = aOp1 - aOp2;
			break;
		}
		case 3:
		{
			// RSB
			aBreakAddress = aOp2 - aOp1;
			break;
		}
		case 4:
		{
			// ADD
			aBreakAddress = aOp1 + aOp2;
			break;
		}
		case 5:
		{
			// ADC
			aBreakAddress = aOp1 + aOp2 + (aStatusRegister & ARM_CARRY_BIT) ? 1 : 0;
			break;
		}
		case 6:
		{
			// SBC
			aBreakAddress = aOp1 - aOp2 - (aStatusRegister & ARM_CARRY_BIT) ? 0 : 1;
			break;
		}
		case 7:
		{
			// RSC
			aBreakAddress = aOp2 - aOp1 - (aStatusRegister & ARM_CARRY_BIT) ? 0 : 1;
			break;
		}
		case 12:
		{
			// ORR
			aBreakAddress = aOp1 | aOp2;
			break;
		}
		case 13:
		{
			// MOV
			aBreakAddress = aOp2;
			break;
		}
		case 14:
		{
			// BIC
			aBreakAddress = aOp1 & ~aOp2;
			break;
		}
		case 15:
		{
			// MVN
			aBreakAddress = ~aOp2;
			break;
		}
	}
}

//
// DMetroTrkChannel::IsPreviousInstructionMovePCToLR
//
TBool DMetroTrkChannel::IsPreviousInstructionMovePCToLR(DThread *aThread)
{
	LOG_MSG("DMetroTrkChannel::IsPreviousInstructionMovePCToLR()");

	// there are several types of instructions that modify the PC that aren't
	// designated as linked or non linked branches.  the way gcc generates the
	// code can tell us whether or not these instructions are to be treated as
	// linked branches.  the main cases are bx and any type of mov or load or
	// arithmatic operation that changes the PC.  if these are really just
	// function calls that will return, gcc will generate a mov	lr, pc
	// instruction as the previous instruction.  note that this is just for arm
	// and armi
	
	// get the address of the previous instruction
	TUint32 address = ReadRegister(aThread, PC_REGISTER) - 4;

	TBuf8<4> previousInstruction;
	TInt err = DoReadMemory(aThread, address, 4, previousInstruction);
	if (KErrNone != err)
	{
		LOG_MSG2("Error %d reading memory at address %x", address);
		return EFalse;
	}

	const TUint32 movePCToLRIgnoringCondition = 0x01A0E00F;

	TUint32 inst = *(TUint32 *)previousInstruction.Ptr();
	
	if ((inst & 0x0FFFFFFF) == movePCToLRIgnoringCondition)
	{
		return ETrue;
	}
		
	return EFalse;
}

//
// DMetroTrkChannel::DoEnableDisabledBreak
//
// Restore the breakpoint that was disabled for stepping past it if necessary
//
TInt DMetroTrkChannel::DoEnableDisabledBreak(TUint32 aThreadId)
{
	LOG_MSG("DMetroTrkChannel::DoEnableDisabledBreak()");
		
	for (TInt i = NUMBER_OF_TEMP_BREAKPOINTS; i < iBreakPointList.Count(); i++)
	{
//	    if (iBreakPointList[i].iDisabledForStep && ((iBreakPointList[i].iThreadId == aThreadId) || (iBreakPointList[i].iThreadId == 0xFFFFFFFF)))
	    //  Always re-enable non-Thread Specific breakpoints
		if (iBreakPointList[i].iDisabledForStep && ((iBreakPointList[i].iThreadId == aThreadId) || (iBreakPointList[i].iThreadSpecific == EFalse)))
		{
			LOG_MSG2("Re-enabling breakpoint at address %x", iBreakPointList[i].iAddress);
			iBreakPointList[i].iDisabledForStep = EFalse;
			return DoEnableBreak(iBreakPointList[i], EFalse);
		}		
	}
	
	return KErrNone;
}

//
// DMetroTrkChannel::IsExecuted
//
// Determines whether or not an instruction will be executed
//
TBool DMetroTrkChannel::IsExecuted(TUint8 aCondition ,TUint32 aStatusRegister)
{
	LOG_MSG("DMetroTrkChannel::IsExecuted()");

	TBool N = ((aStatusRegister >> 28) & 0x0000000F) & 0x00000008;
	TBool Z = ((aStatusRegister >> 28) & 0x0000000F) & 0x00000004;
	TBool C = ((aStatusRegister >> 28) & 0x0000000F) & 0x00000002;
	TBool V = ((aStatusRegister >> 28) & 0x0000000F) & 0x00000001;

	switch(aCondition)
	{
		case 0:
			return Z;
		case 1:
			return !Z;
		case 2:
			return C;
		case 3:
			return !C;
		case 4:
			return N;
		case 5:
			return !N;
		case 6:
			return V;
		case 7:
			return !V;
		case 8:
			return (C && !Z);
		case 9:
			return (!C || Z);
		case 10:
			return (N == V);
		case 11:
			return (N != V);
		case 12:
			return ((N == V) && !Z);
		case 13:
			return (Z || (N != V));
		case 14:
		case 15:
			return ETrue;
	}
	
	return EFalse;
}


TBool DMetroTrkChannel::IsAddressInRom(TUint32 aAddress)
{
	LOG_MSG("DMetroTrkChannel::IsAddressInRom()");
	
	TRomHeader romHeader = Epoc::RomHeader();
	
	if ((aAddress >= romHeader.iRomBase ) && (aAddress < (romHeader.iRomBase + romHeader.iRomSize)))
		return ETrue;
	
	return EFalse;
}

TBool DMetroTrkChannel::IsAddressSecure(TUint32 aAddress)
{
	LOG_MSG("DMetroTrkChannel::IsAddressInRom()");
	
	// SHORT TERM FIX...
	// For now, don't allow access to Kernel memory...
	TLinAddr kernDataStartAddr =  iMultipleMemModel ? KSuperPageMultipleLinAddr: KSuperPageMovingLinAddr;
	TLinAddr kernDataEndAddr = iMultipleMemModel ? KKernDataEndMultipleLinAddr: KKernDataEndMovingLinAddr;
	
	if ((aAddress >= kernDataStartAddr) && (aAddress <= kernDataEndAddr))
		return ETrue;
		
	return EFalse;
}

TBool DMetroTrkChannel::IsRegisterSecure(TInt registerIndex)
{	
	LOG_MSG("DMetroTrkChannel::IsRegisterSecure()");
		
	if (registerIndex == SP_REGISTER || registerIndex == LINK_REGISTER || registerIndex == STATUS_REGISTER)
		return ETrue;
		
	return EFalse;
}

//
// DMetroTrkChannel::AllocateShadowPageIfNecessary
//
// Allocate a shadow page if the address is in ROM and no page has been allocated for that range yet
//
TInt DMetroTrkChannel::AllocateShadowPageIfNecessary(TUint32 aAddress, TUint32 &aPageAddress)
{
	LOG_MSG("DMetroTrkChannel::AllocateShadowPageIfNecessary()");

	// if this is in ROM, we need to shadow memory
	TBool inRom = EFalse;
	
	//find out if M::IsRomAddress is available from ekern or any other library
	//ReturnIfError(inRom = M::IsRomAddress((TAny *)aAddress));
	TRomHeader romHeader = Epoc::RomHeader();
	
	if ((aAddress >= romHeader.iRomBase ) && (aAddress < (romHeader.iRomBase + romHeader.iRomSize)))
		inRom = ETrue;

	TInt err = KErrNone;
	
	if (inRom) // && (TSuperPage().iRomConfig[0].iType != KBdbBankTypeRamAsRom))
	{
		// if a shadow page has not already been allocated for this address range, do so now
		TUint32 pageAddress = (aAddress & ~(iPageSize-1));
		
		TBool found = EFalse;
		for (TInt i=0; i<iBreakPointList.Count(); i++)
		{
			if (pageAddress == iBreakPointList[i].iPageAddress)
			{
				LOG_MSG2("Shadow page already allocated at address %x", pageAddress);
				found = ETrue;
			}
		}

		if (!found)
		{
			LOG_MSG2("Allocating shadow page starting at address %x", pageAddress);
			err = Epoc::AllocShadowPage(pageAddress);
			if (KErrNone != err)
			{
				LOG_MSG2("Error %d allocating shadow page", err);
				return KErrGeneral;
			}

			//check to see if this is still necessary
			//if ((ASSPID() & ASSP_ID_MASK) == XSCALE_ASSP_ID)
				//FlushDataCache(); // workaround for cache flush problem on Lubbock/PXA255 base port
		}
		
		// return the page address
		aPageAddress = pageAddress;
	}

	return err;
}

//
// DMetroTrkChannel::FreeShadowPageIfNecessary
//
// Free a shadow page if the address is in ROM and no no other breakpoints are set in that page range
//
TInt DMetroTrkChannel::FreeShadowPageIfNecessary(TUint32 aAddress, TUint32 aPageAddress)
{
	LOG_MSG("DMetroTrkChannel::FreeShadowPageIfNecessary()");

	TInt err = KErrNone;
	
	TBool found = EFalse;

	TBool inRom = EFalse;
	
	//find out if M::IsRomAddress is available from ekern or any other library
	//ReturnIfError(inRom = M::IsRomAddress((TAny *)aAddress));
	TRomHeader romHeader = Epoc::RomHeader();
	
	if ((aAddress >= romHeader.iRomBase ) && (aAddress < (romHeader.iRomBase + romHeader.iRomSize)))
		inRom = ETrue;
	
	if (inRom)// && (TSuperPage().iRomConfig[0].iType != KBdbBankTypeRamAsRom))
	{
		// if there are no other breakpoints in this shadow page, we can free it now
		for (TInt i = 0; i < iBreakPointList.Count(); i++)
		{
			if (aAddress != iBreakPointList[i].iAddress) // ignore the current entry
			{
				if ((iBreakPointList[i].iAddress >= aPageAddress) &&
					(iBreakPointList[i].iAddress < (aPageAddress + iPageSize)))
				{
					LOG_MSG("Still a breakpoint in this shadow page range");
					found = ETrue;
					break;
				}
			}
		}

		if (!found)
		{
			LOG_MSG2("Freeing shadow page starting at address %x", aPageAddress);
			err = Epoc::FreeShadowPage(aPageAddress);
			if (KErrNone != err)
			{
				LOG_MSG2("Error %d freeing shadow page", err);
				return KErrGeneral;
			}
		}
	}

	return err;
}



//
// DMetroTrkChannel::NotifyEvent
//
void DMetroTrkChannel::NotifyEvent(SEventInfo aEventInfo, TBool isTraceEvent)
{
	LOG_MSG("DMetroTrkChannel::NotifyEvent()");
	
	if (iEventInfo)
	{
		LOG_MSG2("Completing event, type: %d", aEventInfo.iEventType);
		
		// iThread is the user side debugger thread, so use it to write the info to it memory
		TInt err = Kern::ThreadRawWrite(iClientThread, iEventInfo, (TUint8 *)&aEventInfo, sizeof(SEventInfo), iClientThread);

		if (KErrNone != err)
			LOG_MSG2("Error writing event info: %d", err);

		// clear this since we've completed the request
		iEventInfo = NULL;
		
		// signal the debugger thread
		Kern::RequestComplete(iClientThread, iRequestGetEventStatus, KErrNone);
	}
	else
	{
		if (isTraceEvent)
		{
			LOG_MSG("Queuing trace event\r\n");

			for (TInt i=0; i<NUMBER_OF_EVENTS_TO_QUEUE; i++)
			{
				if (SEventInfo::EUnknown == iTraceEventQueue[i].iEventType)
				{
					iTraceEventQueue[i] = aEventInfo;
					break;
				}
			}			
		}
		else
		{
			LOG_MSG2("Queuing event, type: %d", aEventInfo.iEventType);
			
			for (TInt i=0; i<NUMBER_OF_EVENTS_TO_QUEUE; i++)
			{
				if (SEventInfo::EUnknown == iEventQueue[i].iEventType)
				{
					iEventQueue[i] = aEventInfo;
					break;
				}
			}			
		}		
	}
}


//
// DMetroTrkChannel::ThreadFromId
//
DThread* DMetroTrkChannel::ThreadFromId(TUint32 aId)
{
	LOG_MSG("DMetroTrkChannel::ThreadFromId()");
	
	NKern::ThreadEnterCS();  // Prevent us from dying or suspending whilst holding a DMutex
	DObjectCon& threads = *Kern::Containers()[EThread];  // Get containing holding threads
	threads.Wait();  // Obtain the container mutex so the list does get changed under us
	
	DThread* thread = Kern::ThreadFromId(aId);
	
	threads.Signal();  // Release the container mutex
	NKern::ThreadLeaveCS();  // End of critical section
	
	return thread;
}

//
// DMetroTrkChannel::ProcessFromId
//
DProcess* DMetroTrkChannel::ProcessFromId(TUint32 aId)
{
	LOG_MSG("DMetroTrkChannel::ProcessFromId()");
	
	NKern::ThreadEnterCS();  // Prevent us from dying or suspending whilst holding a DMutex
	DObjectCon& processes = *Kern::Containers()[EProcess];  // Get containing holding threads
	processes.Wait();  // Obtain the container mutex so the list does get changed under us
	
	DProcess* process = Kern::ProcessFromId(aId);
	
	processes.Signal();  // Release the container mutex
	NKern::ThreadLeaveCS();  // End of critical section
	
	return process;
}

//
// DMetroTrkChannel::GetSystemThreadRegisters
//
TBool DMetroTrkChannel::GetSystemThreadRegisters(TArmRegSet* aArmRegSet)
{
	if (iExcInfoValid)
	{
		aArmRegSet->iR0 = iCurrentExcInfo.iR0;
		aArmRegSet->iR1 = iCurrentExcInfo.iR1;
		aArmRegSet->iR2 = iCurrentExcInfo.iR2;
		aArmRegSet->iR3 = iCurrentExcInfo.iR3;
		aArmRegSet->iR4 = iCurrentExcInfo.iR4;
		aArmRegSet->iR5 = iCurrentExcInfo.iR5;
		aArmRegSet->iR6 = iCurrentExcInfo.iR6;		
		aArmRegSet->iR7 = iCurrentExcInfo.iR7;		
		aArmRegSet->iR8 = iCurrentExcInfo.iR8;		
		aArmRegSet->iR9 = iCurrentExcInfo.iR9;	
		aArmRegSet->iR10 = iCurrentExcInfo.iR10;
		aArmRegSet->iR11 = iCurrentExcInfo.iR11;
		aArmRegSet->iR12 = iCurrentExcInfo.iR12;
		aArmRegSet->iR13 = iCurrentExcInfo.iR13;		
		aArmRegSet->iR14 = iCurrentExcInfo.iR14;		
		aArmRegSet->iR15 = iCurrentExcInfo.iR15;		
		aArmRegSet->iFlags = iCurrentExcInfo.iCpsr;
		
		return ETrue;				
	}
	
	return EFalse;

}

TBool DMetroTrkChannel::HasManufacturerCaps(DThread* aThread)
{
	if (aThread && (aThread->HasCapability(ECapabilityTCB) || 
		    		aThread->HasCapability(ECapabilityDRM) || 
					aThread->HasCapability(ECapabilityAllFiles)))
	{				
		return ETrue;
	}
	return EFalse;
}

TBool DMetroTrkChannel::IsBeingDebugged(const DThread* aThread)
{	
	TBool isDebugging = EFalse;	
	if (aThread)
	{
		for (TInt i = 0; i < iDebugProcessList.Count(); i++)
		{
			if (iDebugProcessList[i].iId == aThread->iOwningProcess->iId)
			{
				isDebugging = ETrue;
				break;
			}
		}
	}
	return isDebugging;
}

void DMetroTrkChannel::CheckLibraryNotifyList(TUint32 aProcessId)
{   
	SDblQue* codeSegList = Kern::CodeSegList();

	for (TInt i=0; i<iLibraryNotifyList.Count(); i++)
	{	
		if (!iLibraryNotifyList[i].iEmptySlot)
		{
			//iterate through the list
			for (SDblQueLink* codeSegPtr = codeSegList->First(); codeSegPtr != (SDblQueLink*)(codeSegList); codeSegPtr = codeSegPtr->iNext)
			{
				DEpocCodeSeg* codeSeg = (DEpocCodeSeg*)_LOFF(codeSegPtr, DCodeSeg, iLink);   
				if (codeSeg && codeSeg->IsDll())
				{
					SEventInfo info;						
					if (!_strnicmp(iLibraryNotifyList[i].iName.Ptr(), codeSeg->iRootName.Ptr(), codeSeg->iRootName.Length()))
					{
						info.iFileName.Copy((codeSeg->iRootName.Ptr())); 
						LOG_MSG2("library name match: %S", &info.iFileName);
						TModuleMemoryInfo memoryInfo;
						TInt err = codeSeg->GetMemoryInfo(memoryInfo, NULL);
						if (err != KErrNone)
				  		{
				       		break;
				   		}
				   		// 
				   		// check the process id from the DCodeSeg instead of using aProcessId
				   		// if the iAttachProcess from DCodeSeg is null, then we should use aProcessId
						info.iProcessId = aProcessId;
						info.iEventType = SEventInfo::ELibraryLoaded; 
						info.iCodeAddress = memoryInfo.iCodeBase;
						info.iDataAddress = memoryInfo.iInitialisedDataBase;
						info.iThreadId = 0xFFFFFFFF; 
						LOG_MSG2("info.iCodeAddress: %x", info.iCodeAddress);
						LOG_MSG2("info.iDataAddress: %x", info.iDataAddress);
						LOG_MSG2("info.iProcessId: %d", info.iProcessId);
						NotifyEvent(info);
						
						// now reset this entry					
						iLibraryNotifyList[i].iEmptySlot = ETrue;
						break;
					}
				}
			}
		}
	}
}

DECLARE_STANDARD_LDD()
{
    return new DMetroTrkDriverFactory;
}
