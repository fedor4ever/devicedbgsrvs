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


/** Event handler and container for all objects being tracked.  */
#ifndef __TRKEVENTHANDLER_H__
#define __TRKEVENTHANDLER_H__


class DMetroTrkEventHandler : public DKernelEventHandler
{
	public:
		DMetroTrkEventHandler();
		TInt Create(DLogicalDevice* aDevice, DLogicalChannel* aChannel, DThread* aClient);
		~DMetroTrkEventHandler();
		TInt Start();
		TInt Stop();
		
	private:
		static TUint EventHandler(TKernelEvent aEvent, TAny* a1, TAny* a2, TAny* aThis);
		TUint HandleEvent(TKernelEvent aType, TAny* a1, TAny* a2);
		
		TBool HandleEventKillThread(DThread* a1);
		TBool HandleSwException(TExcType aExcType);
		TBool HandleHwException(TArmExcInfo* aExcInfo);
		TBool HandleUserTrace(TText* aStr, TInt aLen);
		
		void AddProcess(DProcess *aProcess, DThread *aThread);
		void StartThread(DThread *aThread);
		void RemoveProcess(DProcess *aProcess);
		void AddLibrary(DLibrary *aLibrary, DThread *aThread);
		void RemoveLibrary(DLibrary *aLibrary);
		void AddCodeSegment(DCodeSeg *aCodeSeg, DProcess *aProcess);
		void RemoveCodeSegment(DCodeSeg *aCodeSeg, DProcess *aProcess);

	private:
		/** Lock serialising calls to event handler */
		DSemaphore* iLock;
		TBool iTracking;
		/** Tracking list (of TTrackedObject).
			Must be accessed only when tracking is disabled or with iLock held.
			Object addresses are used as keys and so must be unique.
		 */
		TInt iOOM;
		TInt iErrorCount;
		TInt iCounters[EEventLimit];
		
		DLogicalDevice* iDevice;	// open reference to LDD for avoiding lifetime issues
		DThread* iClientThread;
		DLogicalChannel* iChannel;
};

#endif //__TRKEVENTHANDLER_H__
