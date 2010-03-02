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


#include <e32def.h>
#include <e32cmn.h>
#include <arm.h>
#include <kernel.h>
#include <kern_priv.h>
#include <nk_trace.h>

#include "TrkKernelDriver.h"
#include "TrkDriver.h"
#include "TrkEventHandler.h"


DMetroTrkEventHandler::DMetroTrkEventHandler()
	:	DKernelEventHandler(EventHandler, this)
{
	LOG_MSG("DMetroTrkEventHandler::DMetroTrkEventHandler()");

}


TInt DMetroTrkEventHandler::Create(DLogicalDevice* aDevice, DLogicalChannel* aChannel, DThread* aClient)
{
	LOG_MSG("DMetroTrkEventHandler::Create()");

	TInt err;
	err = aDevice->Open();
	if (err != KErrNone)
		return err;
	iDevice = aDevice;
	
	iChannel = aChannel; //Don't add ref the channel, since channel closes the event handler before it ever gets destroyed.

	err = aClient->Open();
	if (err != KErrNone)
		return err;
	iClientThread = aClient;

	// Use a semaphore to protect our data structures from concurrent access.
	err = Kern::SemaphoreCreate(iLock, _L("TrkEventHandlerLock"), 1 /* Initial count */);

	if (err != KErrNone)
		return err;
	
	return Add();
}


DMetroTrkEventHandler::~DMetroTrkEventHandler()
{
	LOG_MSG("DMetroTrkEventHandler::~DMetroTrkEventHandler()");

	if (iLock)
		iLock->Close(NULL);
	
	if (iDevice)
		iDevice->Close(NULL);	
	
	if (iClientThread)
		Kern::SafeClose((DObject*&)iClientThread, NULL);
		
}


TInt DMetroTrkEventHandler::Start()
{
	LOG_MSG("DMetroTrkEventHandler::Start()");

	TInt r = KErrNone;
	iTracking = ETrue;
	return r;
}


TInt DMetroTrkEventHandler::Stop()
{
	LOG_MSG("DMetroTrkEventHandler::Stop()");

	//NKern::ThreadEnterCS();
	//Kern::SemaphoreWait(*iLock);
	iTracking = EFalse;
	//Kern::SemaphoreSignal(*iLock);
	//NKern::ThreadLeaveCS();
	
	return KErrNone;
}


TUint DMetroTrkEventHandler::EventHandler(TKernelEvent aType, TAny* a1, TAny* a2, TAny* aThis)
{
	return ((DMetroTrkEventHandler*)aThis)->HandleEvent(aType, a1, a2);
}

TUint DMetroTrkEventHandler::HandleEvent(TKernelEvent aType, TAny* a1, TAny* a2)
{
	if (iTracking)
	{
		iCounters[aType]++;
		switch (aType)
		{
			case EEventSwExc:
			{
				if (HandleSwException((TExcType)(TInt)a1))
					return (TUint)DKernelEventHandler::EExcHandled;
			}
				
			case EEventHwExc:
			{
				if (HandleHwException((TArmExcInfo*)a1))
					return (TUint)DKernelEventHandler::EExcHandled; 
			}
			case EEventUpdateProcess:
			case EEventAddThread:
			case EEventUpdateThread:
			case EEventRemoveThread:
			case EEventNewChunk:
			case EEventDeleteChunk:				
			//currently we don't handle any of these events;
				break;				
			case EEventAddProcess:
			{			
				AddProcess((DProcess*)a1, (DThread*)a2);
				break;
			}		
			case EEventRemoveProcess:
			{
				RemoveProcess((DProcess*)a1);
				break;
			}
			case EEventStartThread:
			{
				StartThread((DThread*)a1);
				break;
			}
			// According to Symbian, the only way to catch panics in EKA2 is through this event.
			case EEventKillThread:
			{
				if (HandleEventKillThread((DThread*)a1))
					return (TUint)DKernelEventHandler::EExcHandled; 
				break;
			}
			case EEventAddLibrary:
			{
				AddLibrary((DLibrary*)a1, (DThread*)a2);
				break;
			}
				
			case EEventRemoveLibrary:
			{
				RemoveLibrary((DLibrary*)a1);
				break;
			}
			
			case EEventUserTrace:
			{
				HandleUserTrace((TText*)a1, (TInt)a2);
				break;
			}
			case EEventAddCodeSeg:
			{
				//
				// EventAddCodeSeg is not fired for all libs, for example, if a static lib is in ROM,
				// then you don't need get AddCodeSeg event for that lib.
				// 
				//AddCodeSegment((DCodeSeg*)a1, (DProcess*)a2);
				break;
			}			
			case EEventRemoveCodeSeg:
			{
				// RemoveCodeSegment((DCodeSeg*)a1, (DProcess*)a2);
				break;
			}
			default:
				break;
		}
	}
	return DKernelEventHandler::ERunNext;
}

//
// DMetroTrkEventHandler::HandleEventKillThread
//

TBool DMetroTrkEventHandler::HandleEventKillThread(DThread* aThread)
{
	LOG_MSG("DMetroTrkEventHandler::HandleEventKillThread()");
	
	//NKern::ThreadEnterCS();
	//Kern::SemaphoreWait(*iLock);
	
	TBool ret = ((DMetroTrkChannel*)iChannel)->HandleEventKillThread(aThread);
		
	//Kern::SemaphoreSignal(*iLock);
	//NKern::ThreadLeaveCS();
	
	return ret;
}


//
// DMetroTrkEventHandler::HandleSwException
//

TBool DMetroTrkEventHandler::HandleSwException(TExcType aExcType)
{
	LOG_MSG("DMetroTrkEventHandler::HandleSwException()");
	
	NKern::ThreadEnterCS();
	Kern::SemaphoreWait(*iLock);
	
	TBool ret = ((DMetroTrkChannel*)iChannel)->HandleSwException(aExcType);
	
	Kern::SemaphoreSignal(*iLock);
	NKern::ThreadLeaveCS();
	
	return ret;
}

//
// DMetroTrkEventHandler::HandleHwException
//

TBool DMetroTrkEventHandler::HandleHwException(TArmExcInfo* aExcInfo)
{
	LOG_MSG("DMetroTrkEventHandler::HandleHwException()");
	
	TBool ret = EFalse;
		
	NKern::ThreadEnterCS();
	Kern::SemaphoreWait(*iLock);

	if (aExcInfo)
	{
		ret = ((DMetroTrkChannel*)iChannel)->HandleHwException(aExcInfo);
	}
	
	Kern::SemaphoreSignal(*iLock);
	NKern::ThreadLeaveCS();
	
	return ret;
}

//
// DMetroTrkEventHandler::HandleUserTrace
//
TBool DMetroTrkEventHandler::HandleUserTrace(TText* aStr, TInt aLen)
{
	LOG_MSG("DMetroTrkEventHandler::HandleUserTrace()");
	TBool ret = EFalse;

	NKern::ThreadEnterCS();
	Kern::SemaphoreWait(*iLock);
	
	if (aStr && aLen > 0)
	{
		ret = ((DMetroTrkChannel*)iChannel)->HandleUserTrace(aStr, aLen);
	}
	
	Kern::SemaphoreSignal(*iLock);
	NKern::ThreadLeaveCS();
	
	return ret;
}

//
// DMetroTrkEventHandler::AddProcess
//
void DMetroTrkEventHandler::AddProcess(DProcess* aProcess, DThread *aThread)
{
	LOG_MSG("DMetroTrkEventHandler::AddProcess()");
	
	NKern::ThreadEnterCS();
	Kern::SemaphoreWait(*iLock);
	
	if (iChannel && aProcess && aThread)
		((DMetroTrkChannel*)iChannel)->AddProcess(aProcess, aThread);

	Kern::SemaphoreSignal(*iLock);
	NKern::ThreadLeaveCS();
}

//
// DMetroTrkEventHandler::StartThread
//
void DMetroTrkEventHandler::StartThread(DThread *aThread)
{
	LOG_MSG("DMetroTrkEventHandler::StartThread()");
	
	NKern::ThreadEnterCS();
	Kern::SemaphoreWait(*iLock);
	
	if (iChannel && aThread)
		((DMetroTrkChannel*)iChannel)->StartThread(aThread);

	Kern::SemaphoreSignal(*iLock);
	NKern::ThreadLeaveCS();
}
//
// DMetroTrkEventHandler::RemoveProcess
//
void DMetroTrkEventHandler::RemoveProcess(DProcess *aProcess)
{
	LOG_MSG("DMetroTrkEventHandler::RemoveProcess()");

	NKern::ThreadEnterCS();
	Kern::SemaphoreWait(*iLock);
	
	if (iChannel && aProcess)
		((DMetroTrkChannel*)iChannel)->RemoveProcess(aProcess);
		
	Kern::SemaphoreSignal(*iLock);
	NKern::ThreadLeaveCS();
}

//
// DMetroTrkEventHandler::AddLibrary
//
void DMetroTrkEventHandler::AddLibrary(DLibrary *aLibrary, DThread *aThread)
{
	LOG_MSG("DMetroTrkEventHandler::AddLibrary()");
	
	NKern::ThreadEnterCS();
	Kern::SemaphoreWait(*iLock);
	
	if (iChannel && aLibrary)
		((DMetroTrkChannel*)iChannel)->AddLibrary(aLibrary, aThread);

	Kern::SemaphoreSignal(*iLock);
	NKern::ThreadLeaveCS();
}

//
// DMetroTrkEventHandler::RemoveLibrary
//
void DMetroTrkEventHandler::RemoveLibrary(DLibrary *aLibrary)
{
	LOG_MSG("DMetroTrkEventHandler::RemoveLibrary()");

	NKern::ThreadEnterCS();
	Kern::SemaphoreWait(*iLock);
	
	if (iChannel && aLibrary)
		((DMetroTrkChannel*)iChannel)->RemoveLibrary(aLibrary);
	
	Kern::SemaphoreSignal(*iLock);
	NKern::ThreadLeaveCS();
}

//
// DMetroTrkEventHandler::AddCodeSegment
//
void DMetroTrkEventHandler::AddCodeSegment(DCodeSeg *aCodeSeg, DProcess *aProcess)
{
	LOG_MSG("DMetroTrkEventHandler::AddCodeSegment()");

	NKern::ThreadEnterCS();
	Kern::SemaphoreWait(*iLock);
	
	if (iChannel && aCodeSeg && aProcess)
		((DMetroTrkChannel*)iChannel)->AddCodeSegment(aCodeSeg, aProcess);
		
	Kern::SemaphoreSignal(*iLock);
	NKern::ThreadLeaveCS();
}

//
// DMetroTrkEventHandler::RemoveCodeSegment
//
void DMetroTrkEventHandler::RemoveCodeSegment(DCodeSeg *aCodeSeg, DProcess *aProcess)
{
	LOG_MSG("DMetroTrkEventHandler::RemoveCodeSegment()");

	NKern::ThreadEnterCS();
	Kern::SemaphoreWait(*iLock);

	if (iChannel && aCodeSeg && aProcess)
		((DMetroTrkChannel*)iChannel)->RemoveCodeSegment(aCodeSeg, aProcess);
	
	Kern::SemaphoreSignal(*iLock);
	NKern::ThreadLeaveCS();
}
