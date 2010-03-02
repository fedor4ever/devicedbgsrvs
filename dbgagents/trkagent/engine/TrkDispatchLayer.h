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


#ifndef __TRKDISPATCHLAYER_H__
#define __TRKDISPATCHLAYER_H__

#include <f32file.h>
#include "TrkFramingLayer.h"
#include "TrkKernelDriver.h"


#ifndef __TEXT_SHELL__
#include <sysutil.h> 
#include <etel3rdparty.h>
#endif

#ifdef __OEM_TRK__
#include "TrkTcbCliSession.h"
#endif



//
// Forward declarations
//
class CTrkDispatchLayer;
class CTrkEngine;


//
// class CExitTrapper
//
// Watches to see when a process exists
//
class CExitTrapper : public CActive
{
public:

	CExitTrapper(CTrkDispatchLayer *aDispatch, TUint32 aProcessId);
	~CExitTrapper();

	void Watch();

protected:

	void RunL();
	void DoCancel();

private:

	RProcess iProcess;
	TUint32 iProcessId;
	CTrkDispatchLayer *iDispatch;
};


//
// class CEventTrapper
//
// Listens for events like breakpoints, exceptions, panic, and library loads
//
class CEventTrapper : public CActive
{
public:

	CEventTrapper(CTrkDispatchLayer *aDispatch);
	~CEventTrapper();
	void Watch();

protected:

	void RunL();
	void DoCancel();

private:

	SEventInfo iEventInfo;
	CTrkDispatchLayer *iDispatch;
};


//
// class CDebugProcess
//
// Represents a process being debugged by the user
//
class CDebugProcess : public CBase
{
public:

	static CDebugProcess* NewL(CTrkDispatchLayer *aDispatch, TUint32 aProcessId, TUint32 aMainThreadId);
	~CDebugProcess();

	void ConstructL(CTrkDispatchLayer *aDispatch, TUint32 aProcessId, TUint32 aMainThreadId);

	TUint32 ProcessId() { return iProcessId; }

public:
	
	TBool iReadyForLibraryLoadNotification;

private:
	
	TUint32 iProcessId;
	TUint32 iMainThreadId;
	CExitTrapper *iExitTrapper;
};

#ifndef __TEXT_SHELL__
//
//class CPhoneInfo
//
//This class is used  to find the name of the Phone for eg: For Nokia 5800, the phone name is 5800 Music Express
//
class CPhoneInfo : public CActive
{
private:    
    
    CTelephony::TPhoneIdV1 iPhoneIdV1;
    CTelephony::TPhoneIdV1Pckg iPhoneIdV1Pckg;
    CTelephony* iTelephony;
    CPhoneInfo();
    void ConstructL();

public:
    static CPhoneInfo* NewL();    
    void GetPhoneName(CTrkDispatchLayer* aDispatchLayer);
    ~CPhoneInfo();

private:
    
    void RunL();
    void DoCancel();
    
private:
    CTrkDispatchLayer* iDispatchLayer;
 
};

#endif

//
// class TCreateProcessData
//
// Container class for information related to the creation of a new process
//
class TCreateProcessData
{
public:

	inline TCreateProcessData()
				: iProcessId(0),
				  iMainThreadId(0),
				  iEntryAddr(0xFFFFFFFF),
				  iCodeAddr(0xFFFFFFFF),
				  iDataAddr(0xFFFFFFFF),
				  iBssAddr(0xFFFFFFFF) {};
	
public:

	TUint32 iProcessId;
	TUint32 iMainThreadId;
	TUint32 iEntryAddr;
	TUint32 iCodeAddr;
	TUint32 iDataAddr;
	TUint32 iBssAddr;
};

//
// class TProtocolVersion
//
// Container class for the protocol version
//
class TProtocolVersion
{
public:
	inline TProtocolVersion() { };
	inline TProtocolVersion(TUint8 aMajor, TUint8 aMinor)
				: iMajor(aMajor), 
				  iMinor(aMinor) { };
public:
	TUint8 iMajor;
	TUint8 iMinor;	
};

//
// class CTrkDispatchLayer
//
// Handles messages between the kernel side driver and the framing layer
//
class CTrkDispatchLayer : public CBase
{
public:

	static CTrkDispatchLayer* NewL(CTrkCommPort *aPort, CTrkEngine* aEngine);
	~CTrkDispatchLayer();

	void Listen();
	void StopListening();
	static void GetVersionInfo(TInt &aMajorVersion, TInt &aMinorVersion, TInt &aMajorAPIVersion, TInt &aMinorAPIVersion, TInt &aBuildNumber);
	void HandleMsg(const TDesC8& aMsg);
	
	TBool IsDebugging() { return iIsConnected; }
	
	void UpdatePhoneNameInfo(TDesC16& aPhoneModel);
	
private:

	CTrkDispatchLayer();
	void ConstructL(CTrkCommPort *aPort, CTrkEngine* aEngine);

	void FindPhoneSWVersion();
	void FindPhoneNameL();

	void DispatchMsgL();
	void DoConnectL();
	void DoDisconnectL();
	void DoVersionsL();
	void DoVersions2L();
	void DoVersions3L();
	void DoHostVersionsL();
	void DoSupportMaskL();
	void DoCPUTypeL();
	void DoReadMemoryL();
	void DoWriteMemoryL();
	void DoReadRegistersL();
	void DoWriteRegistersL();
	void DoContinueL();
	void DoStepL();
	void DoStopL();
	void DoSetBreakL();
	void DoClearBreakL();
	void DoModifyBreakThreadL();
	void DoCreateItemL();
	void DoDeleteItemL();
	void DoReadInfoL();
	void DoWriteInfoL();

	void DoOpenFileL();
	void OpenFileL(const TDesC& aFullPath, TUint aMode, TTime& aTime);
	void DoReadFileL();
	TInt ReadFileL(TUint16 aLength, TPtr8& aData);

	void DoWriteFileL();
	void WriteFileL(TDesC8& aData);

	void DoPositionFileL();
	void PositionFileL(TSeek aSeek, TInt& aOffset);

	void DoCloseFileL();
	void CloseFileL(const TTime& aModifiedTime);

	void DoInstallFileL();
	void DoInstallFile2L();
	
	void DoGetPhoneSWVersionL();
	void DoGetPhoneNameL();
	
	void DoCreateProcessL(TBool aRun=EFalse);
	void DoCreateExeL(const TDesC& aPath, const TDesC& aArgs, TCreateProcessData& aData, TBool aRun=EFalse);
	void DoKillProcessL();

	void DoAttachProcessL(DSOSItemTypes aAttachType=kDSOSProcAttachItem);
	void DoDetachProcessL();
	
	void DoReadProcessListL(TInt32 aIndex);
	void DoReadThreadListL(TInt32 aIndex);
	void DoReBuildProcessList();
	void DoReBuildThreadList(TUint32 aProcessid);
	
	void DoNotifyStoppedL(TUint32 aProcessId, TUint32 aThreadId, TUint32 aCurrentPC, const TDesC8 &aDescription, TBool aAddException=false, const TUint16 aExceptionNumber=0);
	void DoNotifyProcessDiedL(TUint32 aProcessId, TInt aExitCode);
	void DoNotifyLibraryLoadedL(TDesC8 &aName, TUint32 aProcessId, TUint32 aThreadId, TUint32 aCodeBaseAddress, TUint32 aDataBaseAddress);
	void DoNotifyLibraryUnloadedL(TDesC8 &aName, TUint32 aProcessId, TUint32 aThreadId);
	void DoNotifyUserTraceL(TDesC8 &aTrace);
	void DoNotifyProcessAddedL(TDesC8 &aName, TUint32 aProcessId, TUint32 aThreadId, TUint32 aUid, TUint32 aCodeBaseAddress, TUint32 aDataBaseAddress);
	void DoReadLibraryInfoL(TDesC8& aFileName);
	void DoReadProcessInfoL(TUint32 aUid, TDesC8& aFileName);


	void GetDataFromBufferL(TAny *aData, TInt aLength);
	void AddToReplyBufferL(TUint8 aData, TBool aReset = false);
	void AddToReplyBufferL(TUint16 aData, TBool aReset = false);
	void AddToReplyBufferL(TUint32 aData, TBool aReset = false);
	void AddToReplyBufferL(const TDesC8 &aData, TBool aReset = false);


	void RespondOkL() 	{ if (iReplyBuffer) iFramingLayer->RespondOkL(*iReplyBuffer); };
	void InformEventL() { if (iReplyBuffer) iFramingLayer->InformEventL(*iReplyBuffer); };
	
	
	TUint8 IsThreadSuspended(TUint32 aThreadId);
	TBool IsRestrictedFolder(const TDesC& aPath);
	TInt CloseCrashLogger();
	TInt TerminateProcess(const TDesC& aProcessName);
	
	void CloseTcbServer();
private:

	CTrkEngine* iEngine;
	CTrkFramingLayer *iFramingLayer;
#ifndef __TEXT_SHELL__
	CPhoneInfo* iPhoneInfo;
#endif

	HBufC8 *iInputBuffer;
	HBufC8 *iReplyBuffer;

#ifndef __TEXT_SHELL__
	TBuf8 <KSysUtilVersionTextLength> iPhoneVersion;
    TBuf8 <CTelephony::KPhoneModelIdSize> iPhoneModel;
#endif
    
    enum TFileState
	{
		EFileOpened = 0,
		EFileReading = 1,
		EFileWriting = 2,
		EFileClosed = 3,
		EFileUnknown = -1		
	};
	RFile iFile;
	RFs iFs;
	TFileState iFileState;

	RPointerArray<CDebugProcess> iDebugProcessList;
	
	RArray<TMetroTrkTaskInfo> iProcessList;
	RArray<TMetroTrkTaskInfo> iThreadList;
	
	RArray<TUint> iSuspendedThreadList;
	
	CEventTrapper *iEventTrapper;
		
	RMetroTrkDriver iKernelDriver;	
	TBool iIsConnected;
	
	TBool iPhoneNameInfoAvailable; 

#ifdef __OEM_TRK__	
	RTrkTcbCliSession iTrkTcbSession;
	TBool iUseTcbServer;
#endif
	TProtocolVersion iHostVersion;

	friend class CEventTrapper;
	friend class CExitTrapper;
};

#endif // __TRKDISPATCHLAYER_H__
