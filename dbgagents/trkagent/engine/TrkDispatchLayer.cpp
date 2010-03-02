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


#include "TrkEngine.h"
#include "TrkDispatchLayer.h"
#include "DateTimeConverter.h"

#ifndef __TEXT_SHELL__
#include "TrkSwInstall.h"
#endif

//
// Macros
//

//
// Version Information
//

#define MAJOR_VERSION_NUMBER	3
#define MINOR_VERSION_NUMBER	2
#define BUILD_NUMBER			4


//
// Support Information
//

#define DS_SUPPORT_MASK_00_07	0x7E	// kDSConnect, kDSDisconnect, kDSReset, kDSVersions, kDSSupportMask, kDSCPUType */
#define DS_SUPPORT_MASK_08_0F	0x00
#define DS_SUPPORT_MASK_10_17	0x4f	// R/W Memory, R/W Registers, FlushCache
#define DS_SUPPORT_MASK_18_1F	0x5f	// kDSContinue, kDSStep, kDSStop, kDSSetBreak, kDSClearBreak
#define DS_SUPPORT_MASK_20_27	0x01	// kDSNotifyFileInput
#define DS_SUPPORT_MASK_28_2F	0x00
#define DS_SUPPORT_MASK_30_37	0x00
#define DS_SUPPORT_MASK_38_3F	0x00
#define DS_SUPPORT_MASK_40_47	0x0f	// OS Create/Delete Item, OS Read/Write Info
#define DS_SUPPORT_MASK_48_4F	0x1f	// OS Read/Write/Open/Close/Position File
#define DS_SUPPORT_MASK_50_57	0x00
#define DS_SUPPORT_MASK_58_5F	0x00
#define DS_SUPPORT_MASK_60_67	0x00
#define DS_SUPPORT_MASK_68_6F	0x00
#define DS_SUPPORT_MASK_70_77	0x00
#define DS_SUPPORT_MASK_78_7F	0x00
#define DS_SUPPORT_MASK_80_87	0x01	// kDSReplyACK
#define DS_SUPPORT_MASK_88_8F	0x00
#define DS_SUPPORT_MASK_90_97	0x11	// kDSNotifyStopped, kDSNotifyStopped2
#define DS_SUPPORT_MASK_98_9F	0x00
#define DS_SUPPORT_MASK_A0_A7	0x03	// kDSOSNotifyCreated, kDSOSNotifyDeleted
#define DS_SUPPORT_MASK_A8_AF	0x00
#define DS_SUPPORT_MASK_B0_B7	0x00
#define DS_SUPPORT_MASK_B8_BF	0x00
#define DS_SUPPORT_MASK_C0_C7	0x00
#define DS_SUPPORT_MASK_C8_CF	0x00
#define DS_SUPPORT_MASK_D0_D7	0x03	// kDSWriteFile, kDSReadFile
#define DS_SUPPORT_MASK_D8_DF	0x00
#define DS_SUPPORT_MASK_E0_E7	0x00
#define DS_SUPPORT_MASK_E8_EF	0x00
#define DS_SUPPORT_MASK_F0_F7	0x00
#define DS_SUPPORT_MASK_F8_FF	0x80	// kDSReplyNAK

#define KProcessKilled 0x4B696C6C // "Kill"

#ifdef EKA2
#define EUSER_LIBPATH _L("Z:\\Sys\\Bin\\euser.dll")
#else
#define EUSER_LIBPATH _L("Z:\\System\\Libs\\euser.dll")
#endif

#ifdef __OEM_TRK__
#define TRK_TYPE_DESCRIPTION _L8("System TRK")
#else
#define TRK_TYPE_DESCRIPTION _L8("Application TRK")
#endif


//
// Swap2
//
// Byte-swaps a 2-byte value.  Used to convert to/from little/big endian.
//
static TUint16 Swap2(TUint16 aSource)
{
	TUint16 dest = 0;
	for (TInt i=0; i<2; i++)
	{
		dest <<= 8;
		dest |= aSource & 0xFF;
		aSource >>= 8;
	}
	
	return dest;
} 

//
// Swap4
//
// Byte-swaps a 4-byte value.  Used to convert to/from little/big endian.
//
static TUint32 Swap4(TUint32 aSource)
{
	TUint32 dest = 0;
	for (TInt i=0; i<4; i++)
	{
		dest <<= 8;
		dest |= aSource & 0xFF;
		aSource >>= 8;
	}
	
	return dest;
} 

//
// Swap4xN
//
//	Byte-swaps N 4-byte values *in place*.  Used to convert to/from little/big endian.
//
void Swap4xN(TUint32 *aSource, TInt aCount)
{
	while (aCount--)
	{
		TUint32 temp = Swap4(*aSource);
		*aSource++ = temp;
	}
} 


//
//
// CExitTrapper implementation
//
//

//
// CExitTrapper constructor
//
// Accepts CTrkDispatchLayer interface and the process ID of the
// process to watch
//
CExitTrapper::CExitTrapper(CTrkDispatchLayer *aDispatch, TUint32 aProcessId)
	: CActive(EPriorityStandard),
	  iProcessId(aProcessId),
	  iDispatch(aDispatch)
{
	CActiveScheduler::Add(this);

	TInt error = iProcess.Open(iProcessId);
	if (KErrNone != error)
		User::Panic(_L("Unable to open process"), __LINE__);
}

//
// CExitTrapper destructor
//
CExitTrapper::~CExitTrapper()
{
	Cancel();
	Deque();
	iProcess.Close();
}

//
// CExitTrapper::Watch
//
// Watch for the process to exit
//
void CExitTrapper::Watch()
{
	iProcess.Logon(iStatus);
	SetActive();
}

//
// CExitTrapper::RunL
//
// Called when the process exits
//
void CExitTrapper::RunL()
{
	// sometimes this is run when the process has yet to exit
	if (iProcess.ExitType() == EExitPending)
	{
		iProcess.LogonCancel(iStatus);
		Watch();
	}
	else
	{
		iDispatch->DoNotifyProcessDiedL(iProcessId, iProcess.ExitReason());
	}
}

//
// CExitTrapper::DoCancel
//
// Stop waiting for this process to exit
//
void CExitTrapper::DoCancel()
{
	iProcess.LogonCancel(iStatus);
}


//
//
// CEventTrapper implementation
//
//

//
// CEventTrapper constructor
//
// Accepts CTrkDispatchLayer interface
//
CEventTrapper::CEventTrapper(CTrkDispatchLayer *aDispatch)
	: CActive(CActive::EPriorityStandard),
	  iDispatch(aDispatch)
{
	CActiveScheduler::Add(this);
}

//
// CEventTrapper destructor
//
CEventTrapper::~CEventTrapper()
{
	Cancel();
	Deque();
}

//
// CEventTrapper::Watch
//
// Start listening for events
//
void CEventTrapper::Watch()
{
	iDispatch->iKernelDriver.GetEvent(iStatus, iEventInfo);
	SetActive();
}

//
// CEventTrapper::RunL
//
// Called when an event occurs
//
void CEventTrapper::RunL()
{
	// determine the type of event and handle accordingly
	switch (iEventInfo.iEventType)
	{
		case SEventInfo::EThreadBreakPoint:
		{
			iDispatch->DoNotifyStoppedL(iEventInfo.iProcessId, iEventInfo.iThreadId, iEventInfo.iCurrentPC, KNullDesC8);
			break;
		}
		case SEventInfo::EThreadException:
		{
			TBuf8<100> buf;
			
			switch(iEventInfo.iExceptionNumber)
			{
				case EExcIntegerDivideByZero:
				{
					buf.Format(_L8("An integer divide by zero exception has occurred."));
					break;
				}
				case EExcIntegerOverflow:
				{
					buf.Format(_L8("An integer overflow exception has occurred."));
					break;
				}
				case EExcBoundsCheck:
				{
					buf.Format(_L8("An bounds check exception has occurred."));
					break;
				}
				case EExcInvalidOpCode:
				{
					buf.Format(_L8("An invalid instruction exception has occurred."));
					break;
				}
				case EExcDoubleFault:
				{
					buf.Format(_L8("A double fault exception has occurred."));
					break;
				}
				case EExcStackFault:
				{
					buf.Format(_L8("A stack fault exception has occurred."));
					break;
				}
				case EExcAccessViolation:
				{
					buf.Format(_L8("An access violation exception has occurred."));
					break;
				}
				case EExcPrivInstruction:
				{
					buf.Format(_L8("A priveledged instruction exception has occurred."));
					break;
				}
				case EExcAlignment:
				{
					buf.Format(_L8("An alignment exception has occurred."));
					break;
				}
				case EExcPageFault:
				{
					buf.Format(_L8("A page fault exception has occurred."));
					break;
				}
				case EExcFloatDenormal:
				case EExcFloatDivideByZero:
				case EExcFloatInexactResult:
				case EExcFloatInvalidOperation:
				case EExcFloatOverflow:
				case EExcFloatStackCheck:
				case EExcFloatUnderflow:
				{
					buf.Format(_L8("A floating point exception has occurred."));
					break;
				}
				case EExcAbort:
				{
					buf.Format(_L8("An abort exception has occurred."));
					break;
				}
				case EExcUserInterrupt:
				{
					buf.Format(_L8("A user interrupt exception has occurred."));
					break;
				}
				case EExcDataAbort:
				{
					buf.Format(_L8("A data abort exception has occurred."));
					break;
				}
				case EExcCodeAbort:
				{
					buf.Format(_L8("A code abort exception has occurred."));
					break;
				}				
				case EExcGeneral:
				case EExcSingleStep:
				case EExcBreakPoint:
				case EExcKill:
				case EExcMaxNumber:
				case EExcInvalidVector:
				default:
				{
					buf.Format(_L8("An unknown exception (%d) has occurred."), iEventInfo.iExceptionNumber);
					break;				
				}
			}
			
			iDispatch->DoNotifyStoppedL(iEventInfo.iProcessId, iEventInfo.iThreadId, iEventInfo.iCurrentPC, buf);
			break;
		}
		case SEventInfo::EThreadPanic:
		{
			TBuf8<100> buf;
//			buf.Format(_L8("Thread 0x%x has panicked.\nCategory: %S\nReason: %d"), iEventInfo.iThreadId, &iEventInfo.iPanicCategory, iEventInfo.iExceptionNumber);
			buf.Format(_L8("Thread 0x%x has panicked. Category: %S; Reason: %d"), iEventInfo.iThreadId, &iEventInfo.iPanicCategory, iEventInfo.iPanicReason);
			
			iDispatch->DoNotifyStoppedL(iEventInfo.iProcessId, iEventInfo.iThreadId, iEventInfo.iCurrentPC, buf, true, iEventInfo.iExceptionNumber);
			break;
		}
		case SEventInfo::EProcessPanic:
		{
			TBuf8<100> buf;
			buf.Format(_L8("Process 0x%x has panicked.\nCategory: %S\nReason: %d"), iEventInfo.iProcessId, &iEventInfo.iPanicCategory, iEventInfo.iExceptionNumber);
			
			iDispatch->DoNotifyStoppedL(iEventInfo.iProcessId, iEventInfo.iThreadId, iEventInfo.iCurrentPC, buf);
			break;
		}
		case SEventInfo::ELibraryLoaded:
		{
			// a library has been loaded.  we get statically linked library loaded notifications
			// the very first time the process is created.  we want to ignore these because we
			// handle static libs in another way.  if the process the library was loaded for is
			// not being debugging, just resume the thread and ignore the event.  if the process
			// is being debugged but it hasn't yet been created, then this must be a static lib
			// and we ignore it.  note that we do not bother resuming the thread since it has not
			// been started yet!

			TBool created = EFalse;
			TBool found = EFalse;
					
			for (TInt i=0; i<iDispatch->iDebugProcessList.Count(); i++)
			{
				if (iDispatch->iDebugProcessList[i]->ProcessId() == iEventInfo.iProcessId)
				{
					created = iDispatch->iDebugProcessList[i]->iReadyForLibraryLoadNotification;
					found = ETrue;
					break;
				}
			}

		#ifndef EKA2
			if (!iEventInfo.iCodeAddress)
			{
				TBuf<KMaxPath> libName;
				libName.Copy(iEventInfo.iFileName);
				
				RLibrary lib;
				TInt err = lib.LoadRomLibrary(libName, KNullDesC);
				if (KErrNone == err)
				{
					iEventInfo.iCodeAddress = (TUint32)lib.EntryPoint();
					lib.Close();
				}
			}
		#endif

			if (!created || !iEventInfo.iCodeAddress || !found)
			{
				iDispatch->iKernelDriver.ResumeThread(iEventInfo.iThreadId);
			}
			else
			{
				iDispatch->DoNotifyLibraryLoadedL(iEventInfo.iFileName, iEventInfo.iProcessId, iEventInfo.iThreadId, iEventInfo.iCodeAddress, iEventInfo.iDataAddress);
			}
			break;
		}
		case SEventInfo::ELibraryUnloaded:
		{
			TBool found = EFalse;
			
			for (TInt i=0; i<iDispatch->iDebugProcessList.Count(); i++)
			{
				if (iDispatch->iDebugProcessList[i]->ProcessId() == iEventInfo.iProcessId)
				{
					found = ETrue;
					break;
				}
			}
			
			if (!found)
			{
				iDispatch->iKernelDriver.ResumeThread(iEventInfo.iThreadId);
			}
			else
			{
				iDispatch->DoNotifyLibraryUnloadedL(iEventInfo.iFileName, iEventInfo.iProcessId, iEventInfo.iThreadId);
			}
			
			break;
		}
		case SEventInfo::EUserTrace:
		{
			iDispatch->DoNotifyUserTraceL(iEventInfo.iTraceData);
			break;
		}
		case SEventInfo::EProcessAdded:
		{
			iDispatch->DoNotifyProcessAddedL(iEventInfo.iFileName, iEventInfo.iProcessId, 
											 iEventInfo.iThreadId, iEventInfo.iUid, 
											 iEventInfo.iCodeAddress, iEventInfo.iDataAddress);
			break;
		}
		case SEventInfo::EUnknown:
		default:
			User::Panic(_L("Unknown event type"), __LINE__);
			break;
	}

	// reset the values
	iEventInfo.Reset();

	// keep listening for more events
	Watch();
}

//
// CEventTrapper::DoCancel
//
// Stop listening for events
//
void CEventTrapper::DoCancel()
{
	iDispatch->iKernelDriver.CancelGetEvent();
}


//
//
// CDebugProcess implementation
//
//

//
// CDebugProcess::NewL
//
// Accepts CTrkDispatchLayer interface and the process ID and main
// thread ID of a process being debugged
//
CDebugProcess* CDebugProcess::NewL(CTrkDispatchLayer *aDispatch, TUint32 aProcessId, TUint32 aMainThreadId)
{
	CDebugProcess* self = new(ELeave) CDebugProcess();
	self->ConstructL(aDispatch, aProcessId, aMainThreadId);
	return self;
}

//
// CDebugProcess::ConstructL
//
// Accepts CTrkDispatchLayer interface and the process ID and main
// thread ID of a process being debugged
//
void CDebugProcess::ConstructL(CTrkDispatchLayer *aDispatch, TUint32 aProcessId, TUint32 aMainThreadId)
{
	iProcessId = aProcessId;
	iMainThreadId = aMainThreadId;
	iReadyForLibraryLoadNotification = EFalse;
	
	// start waiting for this process to exit
	iExitTrapper = new(ELeave) CExitTrapper(aDispatch, aProcessId);
	iExitTrapper->Watch();
}

//
// CDebugProcess destructor
//
CDebugProcess::~CDebugProcess()
{
	SafeDelete(iExitTrapper);
}

#ifndef __TEXT_SHELL__

//
// CPhoneInfo implementation
//
CPhoneInfo::CPhoneInfo():
           CActive(EPriorityStandard),
           iPhoneIdV1Pckg(iPhoneIdV1),
           iDispatchLayer(NULL)
{
    CActiveScheduler::Add(this);
}

CPhoneInfo* CPhoneInfo::NewL()
{
    CPhoneInfo* self = new(ELeave) CPhoneInfo();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
}

void CPhoneInfo::ConstructL()
{
    iTelephony = CTelephony::NewL();
}

CPhoneInfo::~CPhoneInfo() 
{ 
    Cancel();
    Deque();
    SafeDelete(iTelephony);
}

void CPhoneInfo::GetPhoneName(CTrkDispatchLayer* aDispatchLayer)
{
    iDispatchLayer = aDispatchLayer;
    iTelephony->GetPhoneId(iStatus, iPhoneIdV1Pckg);
    SetActive();
}

void CPhoneInfo::RunL()
{   
    if(iStatus == KErrNone)
	{          
        iDispatchLayer->UpdatePhoneNameInfo(iPhoneIdV1.iModel);              
    }
}

void CPhoneInfo::DoCancel()
{
	iTelephony->CancelAsync(CTelephony::EGetPhoneIdCancel);
}
#endif

//
//
// CTrkDispatchLayer implementation
//
//

//
// CTrkDispatchLayer constructor
//
CTrkDispatchLayer::CTrkDispatchLayer()
	: iEngine(NULL),
	  iFramingLayer(NULL),
#ifndef __TEXT_SHELL__
	  iPhoneInfo(NULL),
#endif
	  iFileState(EFileUnknown),
	  iDebugProcessList(1),
	  iProcessList(1),
	  iThreadList(1),
	  iSuspendedThreadList(1),
	  iEventTrapper(NULL),
	  iIsConnected(EFalse),
	  iPhoneNameInfoAvailable(EFalse)
	  
#ifdef __OEM_TRK__	  
	  ,iUseTcbServer(EFalse)
#endif	  
{
}

//
// CTrkDispatchLayer::NewL
//
CTrkDispatchLayer* CTrkDispatchLayer::NewL(CTrkCommPort *aPort, CTrkEngine* aEngine)
{
	CTrkDispatchLayer* self = new(ELeave) CTrkDispatchLayer();
	self->ConstructL(aPort, aEngine);
	return self;
}

//
// CTrkDispatchLayer destructor
//
CTrkDispatchLayer::~CTrkDispatchLayer()
{
	SafeDelete(iFramingLayer);

	SafeDelete(iInputBuffer);
	SafeDelete(iReplyBuffer);
#ifndef __TEXT_SHELL__
	SafeDelete(iPhoneInfo);
#endif
	// just in case a disconnect never happened
	SafeDelete(iEventTrapper);
	
#ifndef __WINS__
	iKernelDriver.Close();
#endif

    CloseTcbServer();
	iProcessList.Close();
	iThreadList.Close();
	iSuspendedThreadList.Close();
	
	iDebugProcessList.ResetAndDestroy();	
	iDebugProcessList.Close();
	//Notify the server that Debugging is ended
    if (iEngine)
        iEngine->DebuggingEnded();  
        
}
//
// CTrkDispathcLayer::CloseTcbServer()
//
void CTrkDispatchLayer::CloseTcbServer()
{
#ifdef __OEM_TRK__  
    //Shutdown the tcb server.
    if (iUseTcbServer)
    {
        iTrkTcbSession.ShutDownServer();
        iTrkTcbSession.Close();
        iUseTcbServer = EFalse;
    }
#endif  

}

//
// CTrkDispatchLayer::ConstructL
//
void CTrkDispatchLayer::ConstructL(CTrkCommPort *aPort, CTrkEngine* aEngine)
{
	iEngine = aEngine;
	
	// create the framing layer interface
	iFramingLayer = new CTrkFramingLayer(aPort);

	// At a minimum, we assume that the host debugger supports 3.3 version of the protocol
	// From 3.0 to 3.3, TRK is backwards compatible. Starting with version 3.4, TRK sends 
	// process created notifications as part of notify created event. Unless the host 
	// debugger sets the supported protocol version to 3.4, we are not going to send this 
	// new notification. Going forward, we should check for the protocol version before 
	// we send new notifications. 
	iHostVersion.iMajor = DS_PROTOCOL_MAJOR_VERSION_3;
	iHostVersion.iMinor = DS_PROTOCOL_MINOR_VERSION_3;
	
	//To find the sw version runnning in the phone
	FindPhoneSWVersion();
		
	// On techview, this functionality is not available for some reason.
	// This needs to be investigated further, but this functionality is not critical for techview platform right now.
#ifdef __S60__
	// To find the name of the phone.
	TRAPD(err1, FindPhoneNameL());
	if (err1 != KErrNone)
#endif	   
	{
	    iPhoneNameInfoAvailable = EFalse;
	}
}

//
// CTrkDispathLayer::FindPhoneSWVersion()
//
// Finds out the software version of the phone
//
void CTrkDispatchLayer::FindPhoneSWVersion()
{
#ifndef __TEXT_SHELL__
    TBuf<KSysUtilVersionTextLength> version;
    if (SysUtil::GetSWVersion( version ) == KErrNone ) 
	{
        iPhoneVersion.Copy(version);
	}                
#endif
}

//
// CTrkDispathLayer::FindPhoneName()
//
// Finds the name of the phone 
//
void CTrkDispatchLayer::FindPhoneNameL()
{
#ifndef __TEXT_SHELL__
    iPhoneInfo = CPhoneInfo::NewL();
    iPhoneInfo->GetPhoneName(this);
#endif             
}

//
// CTrkDispatchLayer::Listen
//
// Start listening to the communications port for messages from the host debugger
//
void CTrkDispatchLayer::Listen()
{
	iFramingLayer->Listen(this);
}

//
// CTrkDispatchLayer::StopListening
//
// Stop listening to the communications port for messages from the host debugger
//
void CTrkDispatchLayer::StopListening()
{
	// just in case the user quit MetroTrk while something was being debugged.
	RProcess process;
	for (TInt i=0; i<iDebugProcessList.Count(); i++)
	{
		if (KErrNone == process.Open(iDebugProcessList[i]->ProcessId()))
		{
		#ifdef EKA2
			process.Kill(KErrNone);
		#else	
			process.Kill(KProcessKilled);
		#endif
			process.Close();
		}
	}

	iFramingLayer->StopListening();
}

//
// CTrkDispatchLayer::GetVersionInfo
//
// Get the current version of MetroTrk
//
void CTrkDispatchLayer::GetVersionInfo(TInt &aMajorVersion, TInt &aMinorVersion, TInt &aMajorAPIVersion, TInt &aMinorAPIVersion, TInt &aBuildNumber)
{
	aMajorVersion = MAJOR_VERSION_NUMBER;
	aMinorVersion = MINOR_VERSION_NUMBER;
	aMajorAPIVersion = DS_PROTOCOL_MAJOR_VERSION;
	aMinorAPIVersion = DS_PROTOCOL_MINOR_VERSION;
	aBuildNumber = BUILD_NUMBER;
}

//
// CTrkDispatchLayer::HandleMsg
//
// Handle a command sent by the host debugger
//
void CTrkDispatchLayer::HandleMsg(const TDesC8& aMsg)
{
	TInt error = KErrNone;

	// reset the input buffer and set it to the new unframed message
	SafeDelete(iInputBuffer);
	iInputBuffer = aMsg.Alloc();

	if (!iInputBuffer)
		User::Panic(_L("Failed to allocate input buffer"), __LINE__);
	
	TRAP(error, DispatchMsgL());

	if (error < 0)
	{
		// its a symbian os error
		error = kDSReplyOsError;
	}
	
	// handle errors raised during command execution
	switch(error)
	{
		// no error and ack was sent by handler
 		case kDSReplyNoError:
 			break;

		// there was a problem with the packet received
		case kDSReplyPacketSizeError:
		case kDSReplyEscapeError:
		case kDSReplyBadFCS:
		case kDSReplyOverflow:
		case kDSReplySequenceMissing:
			iFramingLayer->RespondErr(kDSReplyNAK, error);
			break;
			
		// command and format were OK, but failed for some other reason
 		default:
			iFramingLayer->RespondErr(kDSReplyACK, error);
	}
}

//
// CTrkDispatchLayer::DispatchMsgL
//
// Handle the command sent by the host debugger
//
void CTrkDispatchLayer::DispatchMsgL()
{
	// make sure the input buffer is setup
	User::LeaveIfNull(iInputBuffer);

	// iInputBuffer holds the unframed message from the host
	switch (iInputBuffer->Ptr()[0])
	{
		case kDSPing:
		{
			// nothing to do
			iFramingLayer->RespondOkL(KNullDesC8);
			break;
		}
		case kDSConnect:
		{
			DoConnectL();
			break;
		}
		case kDSDisconnect:
		{
			DoDisconnectL();
			break;
		}
		case kDSVersions:
		{
			DoVersionsL();
			break;
		}
		case kDSVersions2:
		{
			DoVersions2L();
			break;
		}
		case kDSVersions3:
		{
			DoVersions3L();
			break;
		}
		case kDSHostVersions:
		{
			DoHostVersionsL();
			break;
		}
		case kDSSupportMask:
		{
			DoSupportMaskL();
			break;
		}
		case kDSCPUType:
		{
			DoCPUTypeL();
			break;
		}
		case kDSReadMemory:
		{
			DoReadMemoryL();
			break;
		}
		case kDSWriteMemory:
		{
			DoWriteMemoryL();
			break;
		}
		case kDSReadRegisters:
		{
			DoReadRegistersL();
			break;
		}
		case kDSWriteRegisters:
		{
			DoWriteRegistersL();
			break;
		}
		case kDSContinue:
		{
			DoContinueL();
			break;
		}
		case kDSStep:
		{
			DoStepL();
			break;
		}
		case kDSStop:
		{
			DoStopL();
			break;
		}
		case kDSSetBreak:
		{
			DoSetBreakL();
			break;
		}
		case kDSClearBreak:
		{
			DoClearBreakL();
			break;
		}
		case kDSModifyBreakThread:
		{
			DoModifyBreakThreadL();
			break;
		}
		case kDSOSCreateItem:
		{
			DoCreateItemL();
			break;
		}
		case kDSOSDeleteItem:
		{
			DoDeleteItemL();
			break;
		}
		case kDSOSReadInfo:
		{
			DoReadInfoL();
			break;
		}
		case kDSOSWriteInfo:
		{
			DoWriteInfoL();
			break;
		}
		case kDSOSWriteFile:
		{
			DoWriteFileL();
			break;
		}
		case kDSOSReadFile:
		{
			DoReadFileL();
			break;
		}
		case kDSOSOpenFile:
		{
			DoOpenFileL();
			break;
		}	
		case kDSOSCloseFile:
		{
			DoCloseFileL();
			break;
		}
		case kDSOSPositionFile:
		{
			DoPositionFileL();
			break;
		}
		case kDSOSInstallFile:
		{
			DoInstallFileL();
			break;
		}
		case kDSOSInstallFile2:
		{
			DoInstallFile2L();
			break;
		}
		case kDSOSPhoneSWVersion:
	    {		    
		    DoGetPhoneSWVersionL();
			break;
		}
		case kDSOSPhoneName:		    
		{
		    DoGetPhoneNameL();
		    break;
		}
		
		case kDSReplyACK:
		case kDSReplyNAK:
			break;

		case kDSReset:
		case kDSFillMemory:
		case kDSCopyMemory:
		case kDSFlushCache:
		case kDSDownload:
		case kDSNotifyFileInput:			
		default:
		{
			// unsupported command
			iFramingLayer->RespondErr(kDSReplyACK, kDSReplyUnsupportedCommandError);
			break;
		}
	}
}

//
// CTrkDispatchLayer::DoConnectL
//
// The host debugger is starting a new debug session
//
void CTrkDispatchLayer::DoConnectL()
{	
	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSConnect != command)
		User::Leave(kDSReplyError);

	// if we're already connected for some reason, we need to cleanup so we're ready
	// for a new debug session
	if (iIsConnected)
	{
		// reset the sequence ids for the next debug session
		iFramingLayer->ResetSequenceIDs();

		//stop listening for events
		SafeDelete(iEventTrapper);

		// let the kernel side driver cleanup
		iKernelDriver.Close();

		iIsConnected = EFalse;
	}

	// reset the lists so that they are ready for use
	iProcessList.Reset();
	iThreadList.Reset();
	iSuspendedThreadList.Reset();
	iDebugProcessList.ResetAndDestroy();

	TMetroTrkDriverInfo info;
#ifndef EKA2
	RLibrary euser;
	User::LeaveIfError(euser.Load(EUSER_LIBPATH));
	CleanupClosePushL(euser);
	
	info.iPanic1Address = (TUint32)euser.Lookup(812);			// RThread::Panic
	info.iPanic2Address = (TUint32)euser.Lookup(813);			// RProcess::Panic
	info.iException1Address = (TUint32)euser.Lookup(868);		// RThread::RaiseException
	info.iException2Address = (TUint32)euser.Lookup(520);		// User::HandleException
	info.iLibraryLoadedAddress = (TUint32)euser.Lookup(636);	// UserSvr::LibraryLoaded

	// calculate the end address of the user library.  breakpoints and stepping will not
	// be allowed from the start of ROM to the end of the user library since stopping a
	// kernel thread could end up freezing the target
	RFs fs;
	User::LeaveIfError(fs.Connect());
	CleanupClosePushL(fs);

	RFile file;
	User::LeaveIfError(file.Open(fs, EUSER_LIBPATH, EFileStream|EFileRead|EFileShareReadersOnly));
	CleanupClosePushL(file);

	TInt size;
	User::LeaveIfError(file.Size(size));
	
	info.iUserLibraryEnd = (TUint32)euser.EntryPoint() + size;

	CleanupStack::PopAndDestroy(); // file
	CleanupStack::PopAndDestroy(); // fs
	CleanupStack::PopAndDestroy(); // euser
#endif	
	
	// start the kernel side driver
	User::LeaveIfError(iKernelDriver.Open(info));
	
	// start listening for events
	iEventTrapper = new(ELeave) CEventTrapper(this);
	iEventTrapper->Watch();

	iIsConnected = ETrue;	
#ifdef __S60__	
	// now close close crash logger
	CloseCrashLogger();
#endif	
	
	if (iEngine)
		iEngine->DebuggingStarted();
	
	iFramingLayer->RespondOkL(KNullDesC8);
}

//
// CTrkDispatchLayer::DoDisconnectL
//
// The host debugger is stopping the current debug session
//
void CTrkDispatchLayer::DoDisconnectL()
{
	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSDisconnect != command)
		User::Leave(kDSReplyError);

	// make sure we're connected
	if (!iIsConnected)
	{
		User::Leave(KErrCouldNotDisconnect);
	}

	//stop listening for events
	SafeDelete(iEventTrapper);

	iIsConnected = EFalse;

	iFramingLayer->RespondOkL(KNullDesC8);

	// let the kernel side driver cleanup
	iKernelDriver.Close();

	//Shutdown the tcb server.
	CloseTcbServer();
	
	// send the callback to the trk server
	if (iEngine)
		iEngine->DebuggingEnded();	
        
}

//
// CTrkDispatchLayer::DoVersionsL
//
// The host debugger is requesting MetroTrk version information
//
void CTrkDispatchLayer::DoVersionsL()
{
	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSVersions != command)
		User::Leave(kDSReplyError);
	
	TUint8 kernelMajorVersion = MAJOR_VERSION_NUMBER;
	TUint8 kernelMinorVersion = MINOR_VERSION_NUMBER;
	TUint8 protocolMajorVersion = DS_PROTOCOL_MAJOR_VERSION;
	TUint8 protocolMinorVersion = DS_PROTOCOL_MINOR_VERSION;

	AddToReplyBufferL(kernelMajorVersion, true);
	AddToReplyBufferL(kernelMinorVersion);
	AddToReplyBufferL(protocolMajorVersion);
	AddToReplyBufferL(protocolMinorVersion);

	RespondOkL();
}

//
// CTrkDispatchLayer::DoVersionsL
//
// The host debugger is requesting MetroTrk version information
//
void CTrkDispatchLayer::DoVersions2L()
{
	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSVersions2 != command)
		User::Leave(kDSReplyError);
	
	TUint8 kernelMajorVersion = MAJOR_VERSION_NUMBER;
	TUint8 kernelMinorVersion = MINOR_VERSION_NUMBER;
	TUint8 protocolMajorVersion = DS_PROTOCOL_MAJOR_VERSION;
	TUint8 protocolMinorVersion = DS_PROTOCOL_MINOR_VERSION;
	TUint8 buildNumber = BUILD_NUMBER;

	AddToReplyBufferL(kernelMajorVersion, true);
	AddToReplyBufferL(kernelMinorVersion);
	AddToReplyBufferL(protocolMajorVersion);
	AddToReplyBufferL(protocolMinorVersion);
	AddToReplyBufferL(buildNumber);

	RespondOkL();
}

//
// CTrkDispatchLayer::DoVersions3L
//
// The host debugger is requesting MetroTrk version information
//
void CTrkDispatchLayer::DoVersions3L()
{
	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSVersions3 != command)
		User::Leave(kDSReplyError);
	
	TUint8 kernelMajorVersion = MAJOR_VERSION_NUMBER;
	TUint8 kernelMinorVersion = MINOR_VERSION_NUMBER;
	TUint8 protocolMajorVersion = DS_PROTOCOL_MAJOR_VERSION;
	TUint8 protocolMinorVersion = DS_PROTOCOL_MINOR_VERSION;
	TUint8 buildNumber = BUILD_NUMBER;

	AddToReplyBufferL(kernelMajorVersion, true);
	AddToReplyBufferL(kernelMinorVersion);
	AddToReplyBufferL(protocolMajorVersion);
	AddToReplyBufferL(protocolMinorVersion);
	AddToReplyBufferL(buildNumber);
	
	TUint16 len = TRK_TYPE_DESCRIPTION.Length();
	
	AddToReplyBufferL(len);
	AddToReplyBufferL(TRK_TYPE_DESCRIPTION);

	RespondOkL();
}

//
// CTrkDispatchLayer::DoHostVersionsL
//
// The host debugger is sending the supported protocol version
//
void CTrkDispatchLayer::DoHostVersionsL()
{
	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSHostVersions != command)
		User::Leave(kDSReplyError);
	
	GetDataFromBufferL(&iHostVersion.iMajor, 1);
	GetDataFromBufferL(&iHostVersion.iMinor, 1);
	
	iFramingLayer->RespondOkL(KNullDesC8);
}

//
// CTrkDispatchLayer::DoSupportMaskL
//
// The host debugger is requesting MetroTrk capabilities
//
void CTrkDispatchLayer::DoSupportMaskL()
{
	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSSupportMask != command)
		User::Leave(kDSReplyError);
	
	TBuf8<32> supportMask;
	TUint8 level = DS_PROTOCOL_RTOS;
	
	supportMask.Zero();
	supportMask.Append(DS_SUPPORT_MASK_00_07);
	supportMask.Append(DS_SUPPORT_MASK_08_0F);
	supportMask.Append(DS_SUPPORT_MASK_10_17);
	supportMask.Append(DS_SUPPORT_MASK_18_1F);
	supportMask.Append(DS_SUPPORT_MASK_20_27);
	supportMask.Append(DS_SUPPORT_MASK_28_2F);
	supportMask.Append(DS_SUPPORT_MASK_30_37);
	supportMask.Append(DS_SUPPORT_MASK_38_3F);
	supportMask.Append(DS_SUPPORT_MASK_40_47);
	supportMask.Append(DS_SUPPORT_MASK_48_4F);
	supportMask.Append(DS_SUPPORT_MASK_50_57);
	supportMask.Append(DS_SUPPORT_MASK_58_5F);
	supportMask.Append(DS_SUPPORT_MASK_60_67);
	supportMask.Append(DS_SUPPORT_MASK_68_6F);
	supportMask.Append(DS_SUPPORT_MASK_70_77);
	supportMask.Append(DS_SUPPORT_MASK_78_7F);
	supportMask.Append(DS_SUPPORT_MASK_80_87);
	supportMask.Append(DS_SUPPORT_MASK_88_8F);
	supportMask.Append(DS_SUPPORT_MASK_90_97);
	supportMask.Append(DS_SUPPORT_MASK_98_9F);
	supportMask.Append(DS_SUPPORT_MASK_A0_A7);
	supportMask.Append(DS_SUPPORT_MASK_A8_AF);
	supportMask.Append(DS_SUPPORT_MASK_B0_B7);
	supportMask.Append(DS_SUPPORT_MASK_B8_BF);
	supportMask.Append(DS_SUPPORT_MASK_C0_C7);
	supportMask.Append(DS_SUPPORT_MASK_C8_CF);
	supportMask.Append(DS_SUPPORT_MASK_D0_D7);
	supportMask.Append(DS_SUPPORT_MASK_D8_DF);
	supportMask.Append(DS_SUPPORT_MASK_E0_E7);
	supportMask.Append(DS_SUPPORT_MASK_E8_EF);
	supportMask.Append(DS_SUPPORT_MASK_F0_F7);
	supportMask.Append(DS_SUPPORT_MASK_F8_FF);
	
	AddToReplyBufferL(supportMask, true);
	AddToReplyBufferL(level);

	RespondOkL();
}

//
// CTrkDispatchLayer::DoCPUTypeL
//
// The host debugger is requesting cpu information
//
void CTrkDispatchLayer::DoCPUTypeL()
{
	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSCPUType != command)
		User::Leave(kDSReplyError);

	TUint8 cpuMajor = DS_CPU_MAJOR_ARM;
	TUint8 cpuMinor = 0;
	TUint8 bigEndian = iFramingLayer->IsBigEndian() ? 1 : 0;
	TUint8 defaultTypeSize = 4;
	TUint8 fpTypeSize = 0;
	TUint8 extended1TypeSize = 0;
	TUint8 extended2TypeSize = 0;

	AddToReplyBufferL(cpuMajor, true);
	AddToReplyBufferL(cpuMinor);
	AddToReplyBufferL(bigEndian);
	AddToReplyBufferL(defaultTypeSize);
	AddToReplyBufferL(fpTypeSize);
	AddToReplyBufferL(extended1TypeSize);
	AddToReplyBufferL(extended2TypeSize);

	RespondOkL();
}

//
// CTrkDispatchLayer::DoReadMemoryL
//
// Read memory from the target device and return to the host
//
void CTrkDispatchLayer::DoReadMemoryL()
{
	// make sure we're connected
	if (!iIsConnected)
	{
		User::Leave(KErrGeneral);
	}

	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSReadMemory != command)
		User::Leave(kDSReplyError);

 	// get the options
	TUint8 options = 0;
	GetDataFromBufferL(&options, 1);

	if ((options & DS_MSG_MEMORY_EXTENDED) != 0)
		User::Leave(kDSReplyUnsupportedOptionError);

	// get the length to read
	TUint16 length = 0;
	GetDataFromBufferL(&length, 2);

	if (length > DS_MAXREADWRITELENGTH)
		User::Leave(kDSReplyParameterError);
	
	// get the start address
	TUint32 address = 0;
	GetDataFromBufferL(&address, 4);

	// get the process id
	TUint32 processId = 0;
	GetDataFromBufferL(&processId, 4);

	// get the thread id
	TUint32 threadId = 0;
	GetDataFromBufferL(&threadId, 4);

	// allocate enough room for the data
	HBufC8 *data = HBufC8::NewLC(length);
	TPtr8 ptr(data->Des());

	TInt err = iKernelDriver.ReadMemory(threadId, address, length, ptr);
	if (KErrNone != err)
	{
		CleanupStack::PopAndDestroy(data);
		User::Leave(err);
	}
	
	AddToReplyBufferL(length, true);
	AddToReplyBufferL(ptr);

	RespondOkL();

	CleanupStack::PopAndDestroy(data);
}

//
// CTrkDispatchLayer::DoWriteMemoryL
//
// Write memory from the host to the target device
//
void CTrkDispatchLayer::DoWriteMemoryL()
{
	// make sure we're connected
	if (!iIsConnected)
	{
		User::Leave(KErrGeneral);
	}

	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSWriteMemory != command)
		User::Leave(kDSReplyError);

 	// get the options
	TUint8 options = 0;
	GetDataFromBufferL(&options, 1);

	if ((options & DS_MSG_MEMORY_EXTENDED) != 0)
		User::Leave(kDSReplyUnsupportedOptionError);

	// get the length to write
	TUint16 length = 0;
	GetDataFromBufferL(&length, 2);

	if (length > DS_MAXREADWRITELENGTH)
		User::Leave(kDSReplyParameterError);
	
	// get the start address
	TUint32 address = 0;
	GetDataFromBufferL(&address, 4);

	// get the process id
	TUint32 processId = 0;
	GetDataFromBufferL(&processId, 4);

	// get the thread id
	TUint32 threadId = 0;
	GetDataFromBufferL(&threadId, 4);

	User::LeaveIfError(iKernelDriver.WriteMemory(threadId, address, length, *iInputBuffer));
	
	AddToReplyBufferL(length, true);

	RespondOkL();
}

//
// CTrkDispatchLayer::DoReadRegistersL
//
// Read registers from the target device and return to the host
//
void CTrkDispatchLayer::DoReadRegistersL()
{
	// make sure we're connected
	if (!iIsConnected)
	{
		User::Leave(KErrGeneral);
	}

	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSReadRegisters != command)
		User::Leave(kDSReplyError);

	// get the options
	TUint8 options = 0;
	GetDataFromBufferL(&options, 1);

	if (DS_MSG_REGISTERS_TYPE(options) != kDSRegistersDefault)
		User::Leave(kDSReplyUnsupportedOptionError);
		
	// get the first register
	TInt16 firstRegister = 0;
	GetDataFromBufferL(&firstRegister, 2);

	// get the last register
	TInt16 lastRegister = 0;
	GetDataFromBufferL(&lastRegister, 2);

	if ((firstRegister < 0) || (lastRegister > 16))
		User::Leave(kDSReplyInvalidRegisterRange);
	
	// get the process id
	TUint32 processId = 0;
	GetDataFromBufferL(&processId, 4);

	// get the thread id
	TUint32 threadId = 0;
	GetDataFromBufferL(&threadId, 4);

	// allocate enough space to hold the values
	HBufC8 *values = HBufC8::NewLC((lastRegister - firstRegister + 1) * 4);
	TPtr8 ptr(values->Des());
	
	TInt err = iKernelDriver.ReadRegisters(threadId, firstRegister, lastRegister, ptr);
	if (KErrNone != err)
	{
		CleanupStack::PopAndDestroy(values);
		User::Leave(err);
	}
	
	for (int i=0; i<=lastRegister - firstRegister; i++)
		AddToReplyBufferL(*(TUint32 *)&ptr.Ptr()[i*4], i==0 ? true : false);
			
	RespondOkL();

	CleanupStack::PopAndDestroy(values);
}

//
// CTrkDispatchLayer::DoWriteRegistersL
//
// Write registers from the host to the target device
//
void CTrkDispatchLayer::DoWriteRegistersL()
{
	// make sure we're connected
	if (!iIsConnected)
	{
		User::Leave(KErrGeneral);
	}

	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSWriteRegisters != command)
		User::Leave(kDSReplyError);

	// get the options
	TUint8 options = 0;
	GetDataFromBufferL(&options, 1);

	if (DS_MSG_REGISTERS_TYPE(options) != kDSRegistersDefault)
		User::Leave(kDSReplyUnsupportedOptionError);

	// get the first register
	TInt16 firstRegister = 0;
	GetDataFromBufferL(&firstRegister, 2);

	// get the last register
	TInt16 lastRegister = 0;
	GetDataFromBufferL(&lastRegister, 2);

	if ((firstRegister < 0) || (lastRegister > 16))
		User::Leave(kDSReplyInvalidRegisterRange);

	// get the process id
	TUint32 processId = 0;
	GetDataFromBufferL(&processId, 4);

	// get the thread id
	TUint32 threadId = 0;
	GetDataFromBufferL(&threadId, 4);
	
	// MetroTrk register values come in as big endian so we may need to swap them
	if (!iFramingLayer->IsBigEndian())
	{
		Swap4xN((TUint32 *)iInputBuffer->Ptr(), lastRegister-firstRegister + 1);
	}
	
	User::LeaveIfError(iKernelDriver.WriteRegisters(threadId, firstRegister, lastRegister, *iInputBuffer));
	
	iFramingLayer->RespondOkL(KNullDesC8);
}

//
// CTrkDispatchLayer::DoContinueL
//
// Resume thread execution
//
void CTrkDispatchLayer::DoContinueL()
{
	// make sure we're connected
	if (!iIsConnected)
	{
		User::Leave(KErrGeneral);
	}

	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSContinue != command)
		User::Leave(kDSReplyError);

	// get the process id
	TUint32 processId = 0;
	GetDataFromBufferL(&processId, 4);

	// get the thread id
	TUint32 threadId = 0;
	GetDataFromBufferL(&threadId, 4);
	
	// see if this thread is in the the suspended threads list
	// if so, remove it and resume.  otherwise we need to handle
	// the special case of the first time a thread is resumed
	TInt index = iSuspendedThreadList.Find(threadId);
	if (index >= 0)
	{
		iSuspendedThreadList.Remove(index);
		User::LeaveIfError(iKernelDriver.ResumeThread(threadId));
		iFramingLayer->RespondOkL(KNullDesC8);
	}
	else
	{
		// if it's not in the suspended list then this is the first time it has been
		// resumed (the process has just been created).  see if there are statically
		// linked libraries that we need to report to the host debugger.  if so, we
		// don't really want to resume yet.  let the host debugger set any breakpoints
		// in the libraries first.  it will call resume again once it's done.

		// since we never mark it as suspended, we'll actually get here when the host
		// debugger calls resume the second time.  we really don't want to mark it as
		// suspended because then the host debugger will try to draw the thread window
		TBool found = EFalse;
		for (TInt i=0; i<iDebugProcessList.Count(); i++)
		{
			if (iDebugProcessList[i]->ProcessId() == processId)
			{
				found = ETrue;
				// if we're already marked it as ready, just resume and return
				if (iDebugProcessList[i]->iReadyForLibraryLoadNotification)
				{
					User::LeaveIfError(iKernelDriver.ResumeThread(threadId));
					iFramingLayer->RespondOkL(KNullDesC8);		
					return;
				}

				iDebugProcessList[i]->iReadyForLibraryLoadNotification = ETrue;
				break;
			}
		}
		// For RUN mode, we created the process but we didn't put it into the list
		// therefore we do not want to notify the host debugger about static libraries
		// so just resume and return  (see DoCreateExeL)
		// The host debugger will soon disconnect in this mode
		if (!found)
		{
			User::LeaveIfError(iKernelDriver.ResumeThread(threadId));
			iFramingLayer->RespondOkL(KNullDesC8);		
			return;
		}

		// the process has been created and the host debugger is listening for messages
		// see if there are any statically linked libraries for this process.  if so, let
		// the host debugger know about them now. note that the host debugger will send
		// another resume command once it's handled the static libraries.  if there are
		// not static libraries for this process, we really do need to resume the thread.
		
		TInt err = KErrNone;
		TInt i = 0;
		for (i=0; KErrNone==err; i++)
		{
			SEventInfo info;
			info.iProcessId = processId;
			info.iThreadId = threadId;
			err = iKernelDriver.GetStaticLibraryInfo(i, info);
			
			if (KErrNone == err)
			{
				DoNotifyLibraryLoadedL(info.iFileName, info.iProcessId, 0xFFFFFFFF, info.iCodeAddress, info.iDataAddress);
			}
			//else if (0 == i)
			//{
				// there are no static libraries for this process
				//User::LeaveIfError(iKernelDriver.ResumeThread(threadId));			
			//}
		}
		//Always resume the thread instead of asking the host debugger to resume when sending the last static library notification.
		User::LeaveIfError(iKernelDriver.ResumeThread(threadId));			
		iFramingLayer->RespondOkL(KNullDesC8);		
	}
}

//
// CTrkDispatchLayer::DoStepL
//
// Execute one instruction in a thread
//
void CTrkDispatchLayer::DoStepL()
{
	// make sure we're connected
	if (!iIsConnected)
	{
		User::Leave(KErrGeneral);
	}

	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSStep != command)
		User::Leave(kDSReplyError);

	// get the options
	TUint8 options = 0;
	GetDataFromBufferL(&options, 1);
	
	// we only support stepping out of a range of instructions
	if ((kDSStepIntoRange != options) && (kDSStepOverRange != options))
	{
		User::Leave(kDSReplyUnsupportedOptionError);
	}

	// get the range start address
	TUint32 startAddress = 0;
	GetDataFromBufferL(&startAddress, 4);

	// get the range stop address
	TUint32 stopAddress = 0;
	GetDataFromBufferL(&stopAddress, 4);
		
	// get the process id
	TUint32 processId = 0;
	GetDataFromBufferL(&processId, 4);

	// get the thread id
	TUint32 threadId = 0;
	GetDataFromBufferL(&threadId, 4);

	User::LeaveIfError(iKernelDriver.StepRange(threadId, startAddress, stopAddress, (kDSStepIntoRange == options)));

	iFramingLayer->RespondOkL(KNullDesC8);
}

//
// CTrkDispatchLayer::DoStopL
//
// Stop the execution of a thread
//
void CTrkDispatchLayer::DoStopL()
{
	// make sure we're connected
	if (!iIsConnected)
	{
		User::Leave(KErrGeneral);
	}

	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSStop != command)
		User::Leave(kDSReplyError);

	// get the options
	TUint8 options = 0;
	GetDataFromBufferL(&options, 1);
	
	TUint32 processId = 0;
	TUint32 threadId = 0;
	
	switch(options)
	{
		case kDSStopThread:
		{
			// get the process id
			GetDataFromBufferL(&processId, 4);

			// get the thread id
			GetDataFromBufferL(&threadId, 4);
			
			User::LeaveIfError(iKernelDriver.SuspendThread(threadId));
			
			break;
		}
		case kDSStopSystem:
		case kDSStopProcess:
		default:
		{
			User::Leave(kDSReplyUnsupportedOptionError);
			break;
		}
	}

	iFramingLayer->RespondOkL(KNullDesC8);
	
	TBuf8<4> currentPC;
	
	User::LeaveIfError(iKernelDriver.ReadRegisters(threadId, 15, 15, currentPC));
	
	DoNotifyStoppedL(processId, threadId, *(TUint32 *)currentPC.Ptr(), KNullDesC8);
}

//
// CTrkDispatchLayer::DoSetBreakL
//
// Set a breakpoint
//
void CTrkDispatchLayer::DoSetBreakL()
{
	// make sure we're connected
	if (!iIsConnected)
	{
		User::Leave(KErrGeneral);
	}

	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSSetBreak != command)
		User::Leave(kDSReplyError);

	// get the options - unused
	TUint8 options = 0;
	GetDataFromBufferL(&options, 1);

	// get the mode
	TUint8 thumbMode = 0;
	GetDataFromBufferL(&thumbMode, 1);

	// get the address
	TUint32 address = 0;
	GetDataFromBufferL(&address, 4);

	// get the length
	TUint32 length = 0;
	GetDataFromBufferL(&length, 4);

	// get the count
	TUint32 count = 0;
	GetDataFromBufferL(&count, 4);

	// get the process id
	TUint32 processId = 0;
	GetDataFromBufferL(&processId, 4);

	// get the thread id
	TUint32 threadId = 0;
	GetDataFromBufferL(&threadId, 4);
	
	TInt32 breakId = 0;
	User::LeaveIfError(iKernelDriver.SetBreak(processId, threadId, address, thumbMode, breakId));

	// return the id of this breakpoint to the host debugger
	AddToReplyBufferL((TUint32)breakId, true);
	
	RespondOkL();
}

//
// CTrkDispatchLayer::DoClearBreakL
//
// Clear a breakpoint
//
void CTrkDispatchLayer::DoClearBreakL()
{
	// make sure we're connected
	if (!iIsConnected)
	{
		User::Leave(KErrGeneral);
	}

	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);

	if (kDSClearBreak != command)
		User::Leave(kDSReplyError);

	// get the breakpoint id
	TUint32 breakId = 0;
	GetDataFromBufferL(&breakId, 4);

	User::LeaveIfError(iKernelDriver.ClearBreak(breakId));

	iFramingLayer->RespondOkL(KNullDesC8);
}

//
// CTrkDispatchLayer::DoModifyBreakThreadL
//
// Change the thread(s) that a breakpoint is associated with
//
void CTrkDispatchLayer::DoModifyBreakThreadL()
{
	// make sure we're connected
	if (!iIsConnected)
	{
		User::Leave(KErrGeneral);
	}

	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);

	if (kDSModifyBreakThread != command)
		User::Leave(kDSReplyError);

	// get the breakpoint id
	TUint32 breakId = 0;
	GetDataFromBufferL(&breakId, 4);

	// get the thread id
	TUint32 threadId = 0;
	GetDataFromBufferL(&threadId, 4);

	User::LeaveIfError(iKernelDriver.ChangeBreakThread(threadId, breakId));

	iFramingLayer->RespondOkL(KNullDesC8);
}

//
// CTrkDispatchLayer::DoCreateItemL
//
// Create a new OS item on the target device
//
void CTrkDispatchLayer::DoCreateItemL()
{
	// make sure we're connected
	if (!iIsConnected)
	{
		User::Leave(KErrGeneral);
	}

	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSOSCreateItem != command)
		User::Leave(kDSReplyError);

	// get the item type
	TUint16 type = 0;
	GetDataFromBufferL(&type, 2);

	switch(type)
	{
		case kDSOSProcessItem:
		{
			DoCreateProcessL();
			break;
		}
		case kDSOSProcRunItem:
		{
			DoCreateProcessL(ETrue);
			break;
		}
		case kDSOSProcAttachItem:
		{
			DoAttachProcessL(kDSOSProcAttachItem);
			break;
		}
		case kDSOSProcAttach2Item:
		{
			DoAttachProcessL(kDSOSProcAttach2Item);
			break;
		}
		case kDSOSProcAttach3Item:
		{
			DoAttachProcessL(kDSOSProcAttach3Item);
			break;
		}
		case kDSOSThreadItem:
		case kDSOSDLLItem:
		case kDSOSAppItem:
		case kDSOSMemBlockItem:
		case kDSOSThreadAttachItem:
		default:
		{
			User::Leave(kDSReplyUnsupportedOptionError);
			break;
		}
	}
}

//
// CTrkDispatchLayer::DoDeleteItemL
//
// Delete an OS item from the target device
//
void CTrkDispatchLayer::DoDeleteItemL()
{
	// make sure we're connected
	if (!iIsConnected)
	{
		User::Leave(KErrGeneral);
	}

	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSOSDeleteItem != command)
		User::Leave(kDSReplyError);

	// get the item type
	TUint16 type = 0;
	GetDataFromBufferL(&type, 2);

	switch(type)
	{
		case kDSOSProcessItem:
		{
			DoKillProcessL();
			break;
		}
		case kDSOSProcAttachItem:
		{
		    DoDetachProcessL();
			break;
		}
		case kDSOSThreadItem:
		case kDSOSDLLItem:
		case kDSOSAppItem:
		case kDSOSMemBlockItem:
		case kDSOSThreadAttachItem:
		default:
		{
			User::Leave(kDSReplyUnsupportedOptionError);
			break;
		}
	}
}

//
// CTrkDispatchLayer::DoReadInfoL
//
// Read OS information from the target device
//
void CTrkDispatchLayer::DoReadInfoL()
{	
	// make sure we're connected
	if (!iIsConnected)
	{
		User::Leave(KErrGeneral);
	}

	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);

	if (kDSOSReadInfo != command)
		User::Leave(kDSReplyError);

	// get the type of info requested
	TUint16 type = 0;
	GetDataFromBufferL(&type, 2);

	// get the start index
	TUint32 index = 0;
	GetDataFromBufferL(&index, 4);	
	
	switch(type)
	{
		case kDSOSProcessList:
		{
			DoReadProcessListL(index);
			break;
		}
		case kDSOSThreadList:
		{
			DoReadThreadListL(index);
			break;
		}
		case kDSOSDLLInfo:
		{
			TUint16 nameLength = 0;
			GetDataFromBufferL(&nameLength, 2);
			
			// make sure the length of the message is correct
			if (iInputBuffer->Length() != nameLength)
				User::Leave(kDSReplyPacketSizeError);
			
			TBuf8<KMaxFileName> fileName;
			fileName.Copy(*iInputBuffer);
			fileName.ZeroTerminate();

			DoReadLibraryInfoL(fileName);
			break;
		}
		case kDSOSProcessInfo:
		{
			TUint32 uid3 = 0;
			GetDataFromBufferL(&uid3, 4);
			TUint16 nameLength = 0;
			GetDataFromBufferL(&nameLength, 2);
			
			// make sure the length of the message is correct
			if (iInputBuffer->Length() != nameLength)
				User::Leave(kDSReplyPacketSizeError);
			
			TBuf8<KMaxFileName> fileName;
			fileName.Copy(*iInputBuffer);
			fileName.ZeroTerminate();
			
			DoReadProcessInfoL(uid3, fileName);
			break;
		}
		case kDSOSProcessState:
		case kDSOSThreadState:
		case kDSOSDLLList:
		case kDSOSDLLState:
		default:
		{
			User::Leave(kDSReplyUnsupportedOptionError);
			break;
		}
	}
}

//
// CTrkDispatchLayer::DoWriteInfoL
//
// Set options for the Trk
//
void CTrkDispatchLayer::DoWriteInfoL()
{
	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);

	if (kDSOSWriteInfo != command)
		User::Leave(kDSReplyError);

	// get the type of info requested
	TUint16 type = 0;
	GetDataFromBufferL(&type, 2);

	// get the start index
	TUint32 index = 0;
	GetDataFromBufferL(&index, 4);

	// no options supported yet.  might add something for auto-target libraries later
	User::Leave(kDSReplyUnsupportedOptionError);
}

//
// CTrkDispatchLayer::DoOpenFileL
//
// Open or create a file on the target
//
void CTrkDispatchLayer::DoOpenFileL()
{
	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSOSOpenFile != command)
		User::Leave(kDSReplyError);

	// get the file mode(s)
	TUint8 modes = 0;
	GetDataFromBufferL(&modes, 1);

	if ((modes & kDSFileOpenAppend) || (modes & kDSFileOpenCreate))
		modes |= kDSFileOpenWrite;

	TUint16 nameLength = 0;
	GetDataFromBufferL(&nameLength, 2);
	
	// make sure the length of the message is correct
	if (iInputBuffer->Length() != nameLength)
		User::Leave(kDSReplyPacketSizeError);
	
	TBuf<KMaxPath> fullpath;
	fullpath.Copy(*iInputBuffer);
	fullpath.ZeroTerminate();
		
	TUint mode = EFileShareExclusive;

	// get the file mode(s)
	if (modes & kDSFileOpenRead)
		mode |= EFileRead;

	if (modes & kDSFileOpenWrite)
		mode |= EFileWrite;

	if (modes & kDSFileOpenAppend)
		mode |= EFileWrite;

	if (modes & kDSFileOpenCreate)
		mode |= EFileWrite;
	

	//for getting the modification date
	TTime time;
	
	OpenFileL(fullpath, mode, time);
	
	TDateTime dateTime = time.DateTime();

	TDateTimeConverter winTime(dateTime);
	
	TUint32 timestamp = winTime.GetWinTimeDate();

	TUint32 handle = 1;
	TUint8 err = 0;

	AddToReplyBufferL(err, true);
	AddToReplyBufferL(handle);
	AddToReplyBufferL(timestamp);
	
	RespondOkL();
}

//
// CTrkDispatchLayer::OpenFileL
//
// Open a file on the target, if plat security is enabled, uses TrkTcbServer to open the file.
//
void CTrkDispatchLayer::OpenFileL(const TDesC& aFullPath, TUint aMode, TTime& aTime)
{

#ifdef __OEM_TRK__	
	//Check to see if TCB capability is enforced, if so, connect to the tcb server as well.
	if (!iUseTcbServer && PlatSec::IsCapabilityEnforced(ECapabilityTCB))
	{
		if (!iTrkTcbSession.Connect())
		{
			iUseTcbServer = ETrue;
		}	
	}

	if (iUseTcbServer)
	{
		User::LeaveIfError(iTrkTcbSession.OpenFile(aFullPath, aMode, aTime));
	}
	else
#endif	
	{
	#ifndef __OEM_TRK__
		if (IsRestrictedFolder(aFullPath))
			User::Leave(KErrAccessDenied);
	#endif
		// connect to the file server
		User::LeaveIfError(iFs.Connect());

		TInt error = iFs.MkDirAll(aFullPath);

		if ((KErrNone != error) && (KErrAlreadyExists != error))
		{
			iFs.Close();
			User::Leave(error);
		}
		
		error = iFile.Open(iFs, aFullPath, aMode);
		
		if (KErrNone != error)
			User::LeaveIfError(iFile.Replace(iFs, aFullPath, aMode));
		
		User::LeaveIfError(iFile.Modified(aTime));
		iFileState = EFileOpened;
	}
}

//
// CTrkDispatchLayer::DoReadFileL
//
// Read data from a file on the target
//
void CTrkDispatchLayer::DoReadFileL()
{
	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSOSReadFile != command)
		User::Leave(kDSReplyError);

	// make sure the length of the message is correct
	if (iInputBuffer->Length() != 6)
		User::Leave(kDSReplyPacketSizeError);
	
	// remove the handle
	iInputBuffer->Des().Delete(0, 4);

	// get the length
	TUint8 err = 0;
	TUint16 length = 0;
	GetDataFromBufferL(&length, 2);
	
	// allocate a buffer large enough for the data
	HBufC8 *buffer = HBufC8::NewLC(length);
	TPtr8 ptr(buffer->Des());
	
	//	CleanupStack::PushL(buffer);
	
	TInt error = ReadFileL(length, ptr);	
	if (KErrNone != error)
	{
		CleanupStack::PopAndDestroy(buffer);	
		User::Leave(error);
	}
	
	length = buffer->Length();
	
	AddToReplyBufferL(err, true);
	AddToReplyBufferL(length);
//	AddToReplyBufferL(buffer[0], length);
	AddToReplyBufferL(ptr);
	RespondOkL();
	
	CleanupStack::PopAndDestroy(buffer);
}

//
// CTrkDispatchLayer::ReadFileL
//
// Reads data from a file on the target
//
TInt CTrkDispatchLayer::ReadFileL(TUint16 aLength, TPtr8& aData)
{
	TInt error = KErrNone;

#ifdef __OEM_TRK__
	if (iUseTcbServer)
	{
		User::LeaveIfError(iTrkTcbSession.ReadFile(aLength, aData));
	}
	else
#endif	
	{
		iFileState = EFileReading;
		error = iFile.Read(aData);
	}
	return error;
}

//
// CTrkDispatchLayer::DoWriteFileL
//
// Write data to a file on the target
//
void CTrkDispatchLayer::DoWriteFileL()
{
	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSOSWriteFile != command)
		User::Leave(kDSReplyError);
	
	// get the length
	TUint32 handle = 0;
	GetDataFromBufferL(&handle, 4);
	
	if (handle != 1)
		User::Leave(kDSReplyParameterError);

	// get the length
	TUint8 err = 0;
	TUint16 length = 0;
	GetDataFromBufferL(&length, 2);

	// make sure the length of the message is correct
	if (iInputBuffer->Length() != length)
		User::Leave(kDSReplyPacketSizeError);
		
	WriteFileL(*iInputBuffer);
	
	length = iInputBuffer->Length();
	
	AddToReplyBufferL(err, true);
	AddToReplyBufferL(length);
	RespondOkL();
}

//
// CTrkDispatchLayer::WriteFileL
//
// Write data to a file on the target. If plat sec is enabled, uses Trk Tcb Server.
//
void CTrkDispatchLayer::WriteFileL(TDesC8& aData)
{
#ifdef __OEM_TRK__
	if (iUseTcbServer)
	{
		User::LeaveIfError(iTrkTcbSession.WriteFile(aData));
	}
	else
#endif	
	{	
		if (iFileState == EFileOpened)
		{
			iFile.SetSize(0);
			iFileState = EFileWriting;
		}
		User::LeaveIfError(iFile.Write(aData));
		User::LeaveIfError(iFile.Flush());
	}
}

//
// CTrkDispatchLayer::DoPositionFileL
//
// Change the current file position of a file on the target
//
void CTrkDispatchLayer::DoPositionFileL()
{
	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSOSPositionFile != command)
		User::Leave(kDSReplyError);

	// make sure the length of the message is correct
	if (iInputBuffer->Length() != 9)
		User::Leave(kDSReplyPacketSizeError);
	
	// get the positioning mode
	TUint8 mode = 0;
	GetDataFromBufferL(&mode, 1);

	// remove the handle
	iInputBuffer->Des().Delete(0, 4);

	TUint8 err = 0;
	TUint32 offset = 0;
	GetDataFromBufferL(&offset, 4);

	TSeek seek;
	
	// convert mode to native parameter and set the position accordingly
	switch(mode)
	{
		default:
		case kDSFileSeekSet:
		{
			seek = ESeekStart;
			break;
		}
		case kDSFileSeekCur:
		{
			seek = ESeekCurrent;
			break;
		}
		case kDSFileSeekEnd:
		{
			seek = ESeekEnd;
			break;
		}
	}

	PositionFileL(seek, (TInt &)offset);

	AddToReplyBufferL(err, true);
	
	RespondOkL();
}

//
// CTrkDispatchLayer::PositionFileL
//
// Change the current file position of a file on the target. If plat sec is enabled,
// uses Trk Tcb Server.
//
void CTrkDispatchLayer::PositionFileL(TSeek aSeek, TInt& aOffset)
{
#ifdef __OEM_TRK__
	if (iUseTcbServer)
	{
		User::LeaveIfError(iTrkTcbSession.PositionFile(aSeek, aOffset));
	}
	else
#endif	
	{
		User::LeaveIfError(iFile.Seek(aSeek, aOffset));
	}
}


//
// CTrkDispatchLayer::DoCloseFileL
//
// Close a file on the target
//
void CTrkDispatchLayer::DoCloseFileL()
{
	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSOSCloseFile != command)
		User::Leave(kDSReplyError);

	// make sure the length of the message is correct
	if (iInputBuffer->Length() != 8)
		User::Leave(kDSReplyPacketSizeError);

	// remove the handle
	iInputBuffer->Des().Delete(0, 4);
	
	TUint8 err = 0;
	TUint32 timestamp = 0;
	GetDataFromBufferL(&timestamp, 4);
	
	TDateTimeConverter epocTime(timestamp);
	
	CloseFileL(epocTime.GetEpocTimeDate());

	AddToReplyBufferL(err, true);

	RespondOkL();
}

//
// CTrkDispatchLayer::CloseFileL
//
// Close a file on the target. If plat sec is enabled, uses Trk Tcb Server.
//
void CTrkDispatchLayer::CloseFileL(const TTime& aModifiedTime)
{
#ifdef __OEM_TRK__
	if (iUseTcbServer)
	{
		User::LeaveIfError(iTrkTcbSession.CloseFile(aModifiedTime));
	}
	else
#endif	
	{
		User::LeaveIfError(iFile.SetModified(aModifiedTime));
		User::LeaveIfError(iFile.Flush());
		
		iFile.Close();
		iFs.Close();
		iFileState = EFileClosed;
	}
}

//
// CTrkDispatchLayer::DoInstallFileL
//
// Install the application, supplied as a SIS file.
//
void CTrkDispatchLayer::DoInstallFileL()
{
	// make sure we're connected
	if (!iIsConnected)
	{
		User::Leave(KErrGeneral);
	}

#ifndef __TEXT_SHELL__
	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSOSInstallFile != command)
		User::Leave(kDSReplyError);

	// get the installation drive
	TUint8 drive = 'C';
	GetDataFromBufferL(&drive, 1);
	
	TChar installDrive(drive);

	TUint16 nameLength = 0;
	GetDataFromBufferL(&nameLength, 2);
	
	// make sure the length of the message is correct
	if (iInputBuffer->Length() != nameLength)
		User::Leave(kDSReplyPacketSizeError);
	
	TBuf<KMaxPath> fullpath;
	fullpath.Copy(*iInputBuffer);
	fullpath.ZeroTerminate();
			
	TUint8 err = 0;
	// Temporarily disable the lib loaded event before launching the SW Installer.
	// This is necessary when launching the SIS installer since  launching the 
	// installer causes several libraries to be loaded. The driver suspends the 
	// thread thats loaded the library. This suspension might potentially cause 
	// a deadlock as the event handling active object will never get a chance to 
	// run as the Install function below blocks until the installation is completed.
	User::LeaveIfError(iKernelDriver.DisableLibLoadedEvent());
	
	// now launch the installer
	User::LeaveIfError(CTrkSwInstall::SilentInstallL(fullpath, installDrive));

	// now enable the lib loaded event
	iKernelDriver.EnableLibLoadedEvent();
	
	AddToReplyBufferL(err, true);
	RespondOkL();
#else 
	User::LeaveIfError(-1); 
#endif
}

//
// CTrkDispatchLayer::DoInstallFileL
//
// Install the application, supplied as a SIS file.
//
void CTrkDispatchLayer::DoInstallFile2L()
{
	// make sure we're connected
	if (!iIsConnected)
	{
		User::Leave(KErrGeneral);
	}

#ifndef __TEXT_SHELL__
	// make sure we got here from DispatchMsgL
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);
	
	if (kDSOSInstallFile2 != command)
		User::Leave(kDSReplyError);

	TUint16 nameLength = 0;
	GetDataFromBufferL(&nameLength, 2);
	
	// make sure the length of the message is correct
	if (iInputBuffer->Length() != nameLength)
		User::Leave(kDSReplyPacketSizeError);
	
	TBuf<KMaxPath> fullpath;
	fullpath.Copy(*iInputBuffer);
	fullpath.ZeroTerminate();
			
	TUint8 err = 0;
	// Temporarily disable the lib loaded event before launching the SW Installer.
	// This is necessary when launching the SIS installer since  launching the 
	// installer causes several libraries to be loaded. The driver suspends the 
	// thread thats loaded the library. This suspension might potentially cause 
	// a deadlock as the event handling active object will never get a chance to 
	// run as the Install function below blocks until the installation is completed.
	User::LeaveIfError(iKernelDriver.DisableLibLoadedEvent());

	// now launch the installer
	User::LeaveIfError(CTrkSwInstall::Install(fullpath));

	// now enable the lib loaded event
	iKernelDriver.EnableLibLoadedEvent();
	
	AddToReplyBufferL(err);
	RespondOkL();
#else
	User::LeaveIfError(-1);
#endif
}


//
// CTrkDispathLayer::DoGetPhoneSWVesionL()
//
// sends  the software version of the phone to reply buffer
//
void CTrkDispatchLayer::DoGetPhoneSWVersionL()
{
    // make sure we got here from DispatchMsgL
    
	TUint8 command = 0;
	GetDataFromBufferL(&command, 1);

	if (kDSOSPhoneSWVersion != command)
	{
		User::Leave(kDSReplyError);
	}
#ifndef __TEXT_SHELL__
	if (iPhoneVersion.Size()>0 )
	{
		TUint16 versionNameLength = iPhoneVersion.Length();
		AddToReplyBufferL(versionNameLength, ETrue); 
		AddToReplyBufferL(iPhoneVersion);       
		RespondOkL();
	}
	else
#endif	    
	{
		User::Leave(KErrGeneral);
	}    
                
}


//
//CTrkDispathLayer::DoGetPhoneNameL()
//
//sends the phone model name to the reply buffer
//
void CTrkDispatchLayer::DoGetPhoneNameL()
{    
    TUint8 command = 0;
    GetDataFromBufferL(&command, 1);
    
    if (kDSOSPhoneName != command)
    {
    	User::Leave(kDSReplyError);
	}

#ifndef __TEXT_SHELL__
    if (iPhoneNameInfoAvailable)
    {
     	TUint16 phoneModelLen = iPhoneModel.Length();
     	AddToReplyBufferL(phoneModelLen, true);
     	AddToReplyBufferL(iPhoneModel);
        RespondOkL();
    }    
    else
#endif
    {
        User::Leave(KErrGeneral);
    }
}
//
//To Update the phone model name 
//
//callback function from the CPhoneInfo
//
void CTrkDispatchLayer::UpdatePhoneNameInfo(TDesC16& aPhoneModel)
{
#ifndef __TEXT_SHELL__
    iPhoneModel.Copy(aPhoneModel);
    iPhoneNameInfoAvailable = ETrue;    
#endif
}

//
// CTrkDispatchLayer::DoCreateProcessL
//
// Create a new process on the target device
//
void CTrkDispatchLayer::DoCreateProcessL(TBool aRun)
{
	// remove the options - currently unused
	iInputBuffer->Des().Delete(0, 1);

	// get the length of the data
	TUint16 length = 0;
	GetDataFromBufferL(&length, 2);

	// make sure the length of the message is correct
	if (iInputBuffer->Length() != length)
		User::Leave(kDSReplyPacketSizeError);

	// extract the filename and path, command line args, and working directory
	TPtrC8 exec8(iInputBuffer->Ptr());
	TPtrC8 args8(exec8.Ptr() + exec8.Length() + 1);
		
	// convert the filename and args to unicode descriptors
	HBufC* exec = HBufC::NewLC(exec8.Length());
	exec->Des().Copy(exec8);

	HBufC* args = HBufC::NewLC(args8.Length());
	args->Des().Copy(args8);

	// open the file and get the type (app, exe, etc.)
	RFs fs;
	
	// connect to the file server
	User::LeaveIfError(fs.Connect());
	
	TEntry entry;
		
	User::LeaveIfError(fs.Entry(*exec, entry));
	
	fs.Close();

	TCreateProcessData data;
		
	switch(entry[0].iUid)
	{
		case 0x1000007a:
		{
			// EXE
			DoCreateExeL(*exec, *args, data, aRun);
			break;
		}
		default:
			User::Leave(KErrGeneral);
	}

	CleanupStack::PopAndDestroy(args);
	CleanupStack::PopAndDestroy(exec);

	AddToReplyBufferL(data.iProcessId, true);
	AddToReplyBufferL(data.iMainThreadId);
	if (!aRun)
	{
		AddToReplyBufferL(data.iCodeAddr);
		AddToReplyBufferL(data.iDataAddr);
	}
		
	RespondOkL();
}

//
// CTrkDispatchLayer::DoCreateExeL
//
// Create a new executable on the target device
//
void CTrkDispatchLayer::DoCreateExeL(const TDesC& aPath, const TDesC& aArgs, TCreateProcessData& aData, TBool aRun)
{
	RProcess process;
	User::LeaveIfError(process.Create(aPath, aArgs));
	CleanupClosePushL(process);

	aData.iProcessId = process.Id();
	
	HBufC* threadName = HBufC::NewLC(KMaxFullName);

	*threadName = process.Name();
	_LIT(KMainThreadSuffix, "::Main");
	threadName->Des().Append(KMainThreadSuffix);

	// this function should be (indirectly) called when the debuggee
	// has been created but not yet resumed.  So it should have a main
	// thread whose name ends with "::Main", and so the following call
	// should not fail.
	RThread thread;
	User::LeaveIfError(thread.Open(*threadName));
		
	aData.iMainThreadId = thread.Id();
	thread.Close();

	if (!aRun)
	{
		CDebugProcess *proc = CDebugProcess::NewL(this, aData.iProcessId, aData.iMainThreadId);
		iDebugProcessList.Append(proc);

		User::LeaveIfError(iKernelDriver.GetProcessAddresses(aData.iMainThreadId, aData.iCodeAddr, aData.iDataAddr));
	}
		
	CleanupStack::PopAndDestroy(threadName);
	CleanupStack::PopAndDestroy(); // process
}

//
// CTrkDispatchLayer::DoKillProcessL
//
// Kill an existing process
//
void CTrkDispatchLayer::DoKillProcessL()
{
	// get the process id
	TUint32 processId = 0;
	GetDataFromBufferL(&processId, 4);
	
	RProcess process;
	User::LeaveIfError(process.Open(processId));
#ifdef EKA2
	process.Kill(KErrNone);
#else	
	process.Kill(KProcessKilled);
#endif
	process.Close();

	iFramingLayer->RespondOkL(KNullDesC8);	
}

//
// CTrkDispatchLayer::DoAttachProcessL
//
// Create a new process on the target device
//
void CTrkDispatchLayer::DoAttachProcessL(DSOSItemTypes aAttachType)
{
	// remove the options - currently unused
	iInputBuffer->Des().Delete(0, 1);

	// get the length of the data
	TUint32 processId = 0;
	GetDataFromBufferL(&processId, 4);

	RProcess process;
	User::LeaveIfError(process.Open(processId));
	CleanupClosePushL(process);
	
	// do not allow attaching to a system, protected, or the MetroTrk process because
	// if the user were to stop the wrong thread, the whole system could stop
#ifdef EKA2
	if ((TUint)processId == RProcess().Id())
#else
	if (process.System() || process.Protected() || ((TUint)processId == RProcess().Id()))
#endif
	{
		User::Leave(kDSReplyUnsupportedOptionError);
	}

	TMetroTrkTaskInfo threadInfo(processId);
	User::LeaveIfError(iKernelDriver.GetThreadInfo(0, threadInfo));

	
	TUint32 codeAddr;
	TUint32 dataAddr;
	User::LeaveIfError(iKernelDriver.GetProcessAddresses(threadInfo.iId, codeAddr, dataAddr));	
	//this is necessary to get the process died notifications and also any other event for this process
	CDebugProcess *proc = CDebugProcess::NewL(this, processId, threadInfo.iId);
	// For processes that we are attaching, we need to set this flag to true.
	// otherwise library load notifications for this process will be ignored
	proc->iReadyForLibraryLoadNotification = ETrue;
	iDebugProcessList.Append(proc);	
			
	AddToReplyBufferL(threadInfo.iId, true);
	if (aAttachType == kDSOSProcAttach3Item)
	{
		// now get the UID3 for this process
		// and add it to the reply
		TMetroTrkProcUidInfo procUidInfo(processId);
		User::LeaveIfError(iKernelDriver.GetProcUidInfo(procUidInfo));
		AddToReplyBufferL(procUidInfo.iUid3);
	}
	if (aAttachType == kDSOSProcAttach3Item || aAttachType == kDSOSProcAttach2Item)
	{				
		AddToReplyBufferL(codeAddr);
		AddToReplyBufferL(dataAddr);
		
		TBuf8<KMaxFullName> procName;
		procName.Copy(process.Name());
		TUint16 nameLength = procName.Length();

		AddToReplyBufferL(nameLength);
		AddToReplyBufferL(procName);		
	}

	CleanupStack::PopAndDestroy(); // process
	RespondOkL();
}

//
// CTrkDispatchLayer::DoDetachProcessL
//
// To detach the process from the list of processes
//
void CTrkDispatchLayer::DoDetachProcessL()
{
    TUint32 processId = 0;
    GetDataFromBufferL(&processId, 4);
    
    RProcess process;
    User::LeaveIfError(process.Open(processId));
    
	for (TInt i=0 ; i<iDebugProcessList.Count(); i++)
	{
		if(iDebugProcessList[i]->ProcessId()== processId) 
		{
			SafeDelete(iDebugProcessList[i]);
			iDebugProcessList.Remove(i);
		}
	}
    User::LeaveIfError(iKernelDriver.DetachProcess(processId));    

    iFramingLayer->RespondOkL(KNullDesC8);
}


//
// CTrkDispatchLayer::DoReadProcessListL
//
// Return a list of the current processes to the host debugger
//
void CTrkDispatchLayer::DoReadProcessListL(TInt32 aIndex)
{
	// remove the options - unused
	iInputBuffer->Des().Delete(0, 1);

	// remove the filter - unused
	iInputBuffer->Des().Delete(0, 4);

	// an index of zero means we start fresh.  and index other than zero means
	// we were not able to return all of the processes in the last round due to
	// message size limitations, so we need to pick up where we left off
	if (aIndex == 0)
	{
		DoReBuildProcessList();
	}
	
	TInt32 totalCount = iProcessList.Count();
	TInt32 returnedCount = 0;
	TInt32 bytesRemaining = MAXMESSAGESIZE_V2 - 30; // minus the framing bytes
	TInt32 restOfMsgSize = sizeof(TUint32) // process id
						 + sizeof(TUint32) // priority
						 + sizeof(TUint8); // NULL character
	
	for (TInt32 i=aIndex; i<totalCount; i++)
	{
		// make sure there is enough room left in the message
		if (bytesRemaining >= (iProcessList[i].iName.Length() + restOfMsgSize))
		{
			returnedCount++;
			bytesRemaining -= (iProcessList[i].iName.Length() + restOfMsgSize);
		}
		else
			break;
	}

	// add values for returnedCount and totalCount
	AddToReplyBufferL((TUint32)returnedCount, true);
	AddToReplyBufferL((TUint32)totalCount);

	for (TInt32 i=aIndex; i<(aIndex + returnedCount); i++)
	{
		// add this process info to the buffer
		AddToReplyBufferL(iProcessList[i].iId);
		AddToReplyBufferL(iProcessList[i].iPriority);
		AddToReplyBufferL(iProcessList[i].iName);
		
		// host expects the name to be a null terminated string
		AddToReplyBufferL((TUint8)0);
	}
	
	RespondOkL();
}

//
// CTrkDispatchLayer::DoReadThreadListL
//
// Return a list of the current threads for a process to the host debugger
//
void CTrkDispatchLayer::DoReadThreadListL(TInt32 aIndex)
{
	// remove the options - unused
	iInputBuffer->Des().Delete(0, 1);

	// get the process id
	TUint32 processId = 0;
	GetDataFromBufferL(&processId, 4);

	// an index of zero means we start fresh.  and index other than zero means
	// we were not able to return all of the threads in the last round due to
	// message size limitations, so we need to pick up where we left off
	if (aIndex == 0)
	{
		DoReBuildThreadList(processId);
	}
	
	TInt32 totalCount = iThreadList.Count();
	TInt32 returnedCount = 0;
	TInt32 bytesRemaining = MAXMESSAGESIZE_V2 - 30; // minus the framing bytes
	TInt32 restOfMsgSize = sizeof(TUint32) // process id
						 + sizeof(TUint32) // priority
						 + sizeof(TUint8)  // state
						 + sizeof(TUint8); // NULL character
	
	
	for (TInt32 i=aIndex; i<totalCount; i++)
	{
		// make sure there is enough room left in the message
		if (bytesRemaining >= (iThreadList[i].iName.Length() + restOfMsgSize))
		{
			returnedCount++;
			bytesRemaining -= (iThreadList[i].iName.Length() + restOfMsgSize);
		}
		else
			break;
	}

	// add values for returnedCount and totalCount
	AddToReplyBufferL((TUint32)returnedCount, true);
	AddToReplyBufferL((TUint32)totalCount);

	for (TInt32 i=aIndex; i<(aIndex + returnedCount); i++)
	{
		// add this thread info to the buffer
		AddToReplyBufferL(iThreadList[i].iId);
		AddToReplyBufferL(iThreadList[i].iPriority);
		AddToReplyBufferL(IsThreadSuspended(iThreadList[i].iId));		
		AddToReplyBufferL(iThreadList[i].iName);

		// host expects the name to be a null terminated string
		AddToReplyBufferL((TUint8)0);
	}

	RespondOkL();
}

//
// CTrkDispatchLayer::DoReBuildProcessList
//
// Build a list of the current processes
//
void CTrkDispatchLayer::DoReBuildProcessList()
{
	TInt err = KErrNone;

	// reset the process list
	iProcessList.Reset();
	
	// fill up the process list
	for (TInt i=0; KErrNone==err; i++)
	{
		TMetroTrkTaskInfo processInfo(0);
		err = iKernelDriver.GetProcessInfo(i, processInfo);
		
		//Get a Handle to the process
		RProcess proc;
		if(KErrNone == err && KErrNone == proc.Open(TProcessId(processInfo.iId)))
		{			
			//Only display currently running processes
			if(EExitPending == proc.ExitType())
			{
				iProcessList.Append(processInfo);
			}
			proc.Close();
		}
	}
}

//
// CTrkDispatchLayer::DoReBuildThreadList
//
// Build a list of the current threads for a process
//
void CTrkDispatchLayer::DoReBuildThreadList(TUint32 aProcessId)
{
	TInt err = KErrNone;

	// reset the thread list
	iThreadList.Reset();
	
	// fill up the thread list
	for (TInt i=0; KErrNone==err; i++)
	{
		TMetroTrkTaskInfo threadInfo(aProcessId);
		err = iKernelDriver.GetThreadInfo(i, threadInfo);
		
		//Get a Handle to the thread
		RThread thread;
		if(KErrNone == err && KErrNone == thread.Open(TThreadId(threadInfo.iId)))
		{			
			//Only display currently running processes
			if(EExitPending == thread.ExitType())
			{
				iThreadList.Append(threadInfo);
			}
			thread.Close();
		}
	}
}

// CTrkDispatchLayer::DoNotifyStoppedL
//
// Notify the host debugger that a thread has stopped
//
// START_PANIC
//void CTrkDispatchLayer::DoNotifyStoppedL(TUint32 aProcessId, TUint32 aThreadId, TUint32 aCurrentPC, const TDesC8 &aDescription)
void CTrkDispatchLayer::DoNotifyStoppedL(TUint32 aProcessId, TUint32 aThreadId, TUint32 aCurrentPC, const TDesC8 &aDescription, TBool aAddException, const TUint16 aExceptionNumber)
// END_PANIC
{	
	// add this thread to the suspended threads list
	iSuspendedThreadList.Append(aThreadId);

//	TUint8 event = (aAddException==true) ? kDSNotifyStopped2 : kDSNotifyStopped;
	TUint8 event = kDSNotifyStopped;
	TUint16 descLength = aDescription.Length();
	
	AddToReplyBufferL(event, true);
	AddToReplyBufferL(aCurrentPC);
	AddToReplyBufferL(aProcessId);
	AddToReplyBufferL(aThreadId);
	AddToReplyBufferL(descLength);

	if (descLength)
	{
		AddToReplyBufferL(aDescription);

		// host expects the string to be a null terminated string
		AddToReplyBufferL((TUint8)0);
	}
	// START_PANIC
	if (aAddException)
		AddToReplyBufferL(aExceptionNumber);
	// END_PANIC
	
	InformEventL();
}

//
// CTrkDispatchLayer::DoNotifyProcessDiedL
//
// Notify the host debugger that a process has exited
//
void CTrkDispatchLayer::DoNotifyProcessDiedL(TUint32 aProcessId, TInt aExitCode)
{
	// remove this process from our list
	for (TInt i=0; i<iDebugProcessList.Count(); i++)
	{
		if (iDebugProcessList[i]->ProcessId() == aProcessId)
		{
			SafeDelete(iDebugProcessList[i]);
			iDebugProcessList.Remove(i);
		}
	}

	TUint8 event = kDSOSNotifyDeleted;
	TUint16 type = kDSOSProcessItem;

	AddToReplyBufferL(event, true);
	AddToReplyBufferL(type);
	AddToReplyBufferL((TUint32)aExitCode);
	AddToReplyBufferL(aProcessId);

	InformEventL();
}

//
// CTrkDispatchLayer::DoNotifyLibraryLoadedL
//
// Notify the host debugger that a library in now loaded
//
void CTrkDispatchLayer::DoNotifyLibraryLoadedL(TDesC8 &aName, TUint32 aProcessId, TUint32 aThreadId, TUint32 aCodeBaseAddress, TUint32 aDataBaseAddress)
{
	TUint8 event = kDSOSNotifyCreated;
	TUint16 type = kDSOSDLLItem;

	TUint16 nameLength = aName.Length();
	
	AddToReplyBufferL(event, true);
	AddToReplyBufferL(type);
	AddToReplyBufferL(aProcessId);
	AddToReplyBufferL(aThreadId);
	AddToReplyBufferL(aCodeBaseAddress);
	AddToReplyBufferL(aDataBaseAddress);
	AddToReplyBufferL(nameLength);
	AddToReplyBufferL(aName);

	InformEventL();
}

//
// CTrkDispatchLayer::DoNotifyLibraryUnloadedL
//
// Notify the host debugger that a library has been unloaded
//
void CTrkDispatchLayer::DoNotifyLibraryUnloadedL(TDesC8 &aName, TUint32 aProcessId, TUint32 aThreadId)
{
	TUint8 event = kDSOSNotifyDeleted;
	TUint16 type = kDSOSDLLItem;
	
	TUint16 nameLength = aName.Length();
	
	AddToReplyBufferL(event, true);
	AddToReplyBufferL(type);
	AddToReplyBufferL(aProcessId);
	AddToReplyBufferL(aThreadId);
	AddToReplyBufferL(nameLength);
	AddToReplyBufferL(aName);

	InformEventL();
}

//
// CTrkDispatchLayer::GetDataFromBufferL
//
// Notify the host debugger that trace data has been recieved
//
void CTrkDispatchLayer::DoNotifyUserTraceL(TDesC8 &aTrace)
{
	if (iIsConnected)
		iFramingLayer->SendRawMsgL(aTrace);
}

void CTrkDispatchLayer::DoNotifyProcessAddedL(TDesC8 &aName, TUint32 aProcessId, TUint32 aThreadId, TUint32 aUid, TUint32 aCodeBaseAddress, TUint32 aDataBaseAddress)
{
	// check to see if the host supported protocol handles this event, 
	// otherwise just resume the thread. If not, this thread would get suspended indefinitely.
	if (iHostVersion.iMajor < 3 || (iHostVersion.iMajor == 3  && iHostVersion.iMinor <= 3))
	{
		iKernelDriver.ResumeThread(aThreadId);				
	}
	else
	{
		TUint8 event = kDSOSNotifyCreated;
		TUint16 type = kDSOSProcessItem;

		TUint16 nameLength = aName.Length();
		
		AddToReplyBufferL(event, true);
		AddToReplyBufferL(type);
		AddToReplyBufferL(aProcessId);
		AddToReplyBufferL(aThreadId);
		AddToReplyBufferL(aUid);
		AddToReplyBufferL(aCodeBaseAddress);
		AddToReplyBufferL(aDataBaseAddress);
		AddToReplyBufferL(nameLength);
		AddToReplyBufferL(aName);

		InformEventL();
	}
}

//
// CTrkDispatchLayer::DoReadLibraryInfoL()
//
void CTrkDispatchLayer::DoReadLibraryInfoL(TDesC8& aFileName)
{
	TMetroTrkLibInfo libInfo(aFileName.Length(), &aFileName);
	
	TInt err = iKernelDriver.GetLibraryInfo(libInfo);
	
	if (err == KErrNone)
	{
		AddToReplyBufferL(libInfo.iCodeAddress, true);
		AddToReplyBufferL(libInfo.iDataAddress);
		AddToReplyBufferL(libInfo.iAttachProcessId);
		AddToReplyBufferL(libInfo.iAttachThreadId);
		
		RespondOkL();
	}
	else
	{
		User::Leave(err);
	}			
}

//
// CTrkDispatchLayer::DoReadProcessInfoL()
//
void CTrkDispatchLayer::DoReadProcessInfoL(TUint32 aUid, TDesC8& aFileName)
{
	TMetroTrkExeInfo exeInfo(aUid, aFileName.Length(), &aFileName);
	
	TInt err = iKernelDriver.GetExeInfo(exeInfo);
	
	if (err == KErrNone)
	{
		AddToReplyBufferL(exeInfo.iProcessID, true);
		AddToReplyBufferL(exeInfo.iThreadID);
		AddToReplyBufferL(exeInfo.iCodeAddress);
		AddToReplyBufferL(exeInfo.iDataAddress);
		
		RespondOkL();
	}
	else
	{
		User::Leave(err);
	}			
}

//
// CTrkDispatchLayer::GetDataFromBufferL
//
// Get data from the input buffer
//
void CTrkDispatchLayer::GetDataFromBufferL(TAny* aData, TInt aLength)
{
	if (aLength > iInputBuffer->Length())
		User::Leave(kDSReplyPacketSizeError);
		
	if (iFramingLayer->IsBigEndian())
	{
		Mem::Copy(aData, iInputBuffer->Ptr(), aLength);
	}
	else
	{
		TUint8 *p = (TUint8 *)aData;
		for (int i=aLength-1, j=0; i>=0; i--, j++)
			p[j] = iInputBuffer->Ptr()[i];
	}
	
	// now remove it from the buffer
	iInputBuffer->Des().Delete(0, aLength);
}

//
// CTrkDispatchLayer::AddToReplyBufferL
//
// Add data to the buffer which will be sent back to the host as a reply
//
void CTrkDispatchLayer::AddToReplyBufferL(TUint8 aData, TBool aReset)
{
	if (aReset)
	{
		// free the memory associated with the old reply buffer and allocate a new one
		SafeDelete(iReplyBuffer);
		iReplyBuffer = HBufC8::New(sizeof(TUint8));
	}
	else
	{
		// reallocate to make enough room for the new data
		iReplyBuffer = iReplyBuffer->ReAlloc(iReplyBuffer->Length() + sizeof(TUint8));
	}
	
	// make sure the above worked
	if (!iReplyBuffer)
		User::Leave(KErrNoMemory);
	
	iReplyBuffer->Des().Append(aData);
}

//
// CTrkDispatchLayer::AddToReplyBufferL
//
// Add data to the buffer which will be sent back to the host as a reply
//
void CTrkDispatchLayer::AddToReplyBufferL(TUint16 aData, TBool aReset)
{
	TUint16 temp = aData;
	
	if (aReset)
	{
		// free the memory associated with the old reply buffer and allocate a new one
		SafeDelete(iReplyBuffer);
		iReplyBuffer = HBufC8::New(sizeof(TUint16));
	}
	else
	{
		// reallocate to make enough room for the new data
		iReplyBuffer = iReplyBuffer->ReAlloc(iReplyBuffer->Length() + sizeof(TUint16));
	}
	
	// make sure the above worked
	if (!iReplyBuffer)
		User::Leave(KErrNoMemory);
	
	// the host expects all values except for raw data to be returned in big endian format
	if (!iFramingLayer->IsBigEndian())
	{
		temp = Swap2(aData);
	}

	iReplyBuffer->Des().Append((TUint8 *)&temp, sizeof(TUint16));
}

//
// CTrkDispatchLayer::AddToReplyBufferL
//
// Add data to the buffer which will be sent back to the host as a reply
//
void CTrkDispatchLayer::AddToReplyBufferL(TUint32 aData, TBool aReset)
{
	TUint32 temp = aData;
	
	if (aReset)
	{
		// free the memory associated with the old reply buffer and allocate a new one
		SafeDelete(iReplyBuffer);
		iReplyBuffer = HBufC8::New(sizeof(TUint32));
	}
	else
	{
		// reallocate to make enough room for the new data
		iReplyBuffer = iReplyBuffer->ReAlloc(iReplyBuffer->Length() + sizeof(TUint32));
	}
	
	// make sure the above worked
	if (!iReplyBuffer)
		User::Leave(KErrNoMemory);
	
	// the host expects all values except for raw data to be returned in big endian format
	if (!iFramingLayer->IsBigEndian())
	{
		temp = Swap4(aData);
	}

	iReplyBuffer->Des().Append((TUint8 *)&temp, sizeof(TUint32));
}

//
// CTrkDispatchLayer::AddToReplyBufferL
//
// Add data to the buffer which will be sent back to the host as a reply
//
void CTrkDispatchLayer::AddToReplyBufferL(const TDesC8 &aData, TBool aReset)
{
	if (aReset)
	{
		// free the memory associated with the old reply buffer and allocate a new one
		SafeDelete(iReplyBuffer);
		iReplyBuffer = HBufC8::New(aData.Length());
	}
	else
	{
		// reallocate to make enough room for the new data
		iReplyBuffer = iReplyBuffer->ReAlloc(iReplyBuffer->Length() + aData.Length());
	}
	
	// make sure the above worked
	if (!iReplyBuffer)
		User::Leave(KErrNoMemory);
	
	iReplyBuffer->Des().Append(aData);
}

//
// CTrkDispatchLayer::IsThreadSuspended
//
// Determines whether or not a thread is suspended
//
TUint8 CTrkDispatchLayer::IsThreadSuspended(TUint32 aThreadId)
{
	if (iSuspendedThreadList.Find(aThreadId) >= 0)
		return 1;
	
	return 0;
}

//
// CTrkDispatchLayer::IsRestrictedFolder
//
// Check to see if the path is any of the data caged paths like \sys\ or \private\ or \resource\
//
TBool CTrkDispatchLayer::IsRestrictedFolder(const TDesC& aPath)
{
	_LIT(KSYS, "\\sys\\");
	_LIT(KRESOURCE, "\\resource\\");
	_LIT(KPRIVATE, "\\private\\");
	
	if ( (aPath.FindC(KSYS)>=0) || (aPath.FindC(KRESOURCE)>=0) || (aPath.FindC(KPRIVATE)>=0) )
		return ETrue;
	
	return EFalse;
}


TInt CTrkDispatchLayer::CloseCrashLogger()
{
    TInt err = KErrNone;
    
    //The old mobile crash file name is "d_exc_mc.exe" and the new one is "mc_useragent.exe"    
    //This is the string that needs to be passed to the RProcess::Open call to get a handle.
    //The complete process name actually includes the UID info as well.
    //Instead of hard coding the process name, its better to just 
    //search for the process and find it that way.      
    //_LIT16(KCrashLoggerName, "mc_useragent.exe[1020e519]0001");
    _LIT16(KOldCrashLoggerName, "d_exc_mc*");
    _LIT16(KCrashLoggerName, "mc_useragent*");
    
    err = TerminateProcess(KOldCrashLoggerName);
    err = TerminateProcess(KCrashLoggerName);

    return err;
}

TInt CTrkDispatchLayer::TerminateProcess(const TDesC& aProcessName)
{
    TFindProcess find(aProcessName);
    TFullName name; 
        
    TInt err = find.Next(name);
    if (KErrNone == err)
    {   
        RProcess process;
        err = process.Open(find);
    
        if (KErrNone == err)
        {
            process.Kill(KErrNone);
        }
    }
    return err;
}
