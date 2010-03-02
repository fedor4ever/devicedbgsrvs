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


#ifndef __TRKKERNELDRIVER_H__
#define __TRKKERNELDRIVER_H__

//
// class TCapsMetroTrkDriver
//
class TCapsMetroTrkDriver
{
public:
	TVersion	iVersion;
};


//
// class SEventInfo
//
class SEventInfo
{
public:

	enum TEventType
	{
		EUnknown = -1,
	    EThreadBreakPoint,
	    EThreadException,
	    EThreadPanic,
	    EProcessPanic,
	    ELibraryLoaded,
	    ELibraryUnloaded,
	    EUserTrace,
	    EProcessAdded
	};

	inline SEventInfo() { Reset(); };

	inline void Reset() { iProcessId = 0;
						  iThreadId = 0;
						  iCurrentPC = 0;
						  iExceptionNumber = 0;
						  iFileName.FillZ();
						  iPanicCategory.FillZ();
						  iCodeAddress = 0;
						  iDataAddress = 0;
						  iEventType = EUnknown; 
						  iTraceData.FillZ();
						  iTraceDataLen = 0; 
						  iPanicReason = 0; 
						  iUid = 0; 
						  };
public:

    TUint32 				iProcessId;
    TUint32 				iThreadId;
    TUint32					iCurrentPC;
    TInt					iExceptionNumber; // from TExcType
    TBuf8<KMaxName>			iFileName;
    TBuf8<KMaxName>			iPanicCategory;
	TUint32					iCodeAddress;
	TUint32					iDataAddress;
    TEventType				iEventType;
    TBuf8<260>				iTraceData;
    TInt					iTraceDataLen;
    // START_PANIC
    TInt					iPanicReason;
    // END_PANIC
    TUint32 				iUid;
};


//
// class TMetroTrkBreakInfo
//
class TMetroTrkBreakInfo
{
public:

	inline TMetroTrkBreakInfo(const TUint32 aAddress, const TBool aThumbMode, TInt32 *aId, const TUint32 aProcessId)
				: iAddress(aAddress),
				  iThumbMode(aThumbMode),
				  iId(aId),
				  iProcessId(aProcessId) {};

public:

	TUint32 iAddress;
	TBool iThumbMode;
	TInt32* iId;
	TUint32 iProcessId;
};


//
// class TMetroTrkMemoryInfo
//
class TMetroTrkMemoryInfo
{
public:

	inline TMetroTrkMemoryInfo(const TUint32 aAddress, const TInt32 aLength, TDesC8 *aData)
				: iAddress(aAddress),
				  iLength(aLength),
				  iData(aData) {};
	
public:

	TUint32 iAddress;
	TInt16	iLength;
	TDesC8*	iData;
};


//
// class TMetroTrkRegisterInfo
//
class TMetroTrkRegisterInfo
{
public:

	inline TMetroTrkRegisterInfo(const TInt16 aFirstRegister, const TInt16 aLastRegister, TDesC8 *aValues)
				: iFirstRegister(aFirstRegister),
				  iLastRegister(aLastRegister),
				  iValues(aValues) {};
	
public:

	TInt16	iFirstRegister;
	TInt16	iLastRegister;
	TDesC8*	iValues;
};


//
// class TMetroTrkTaskInfo
//
class TMetroTrkTaskInfo
{
public:

	inline TMetroTrkTaskInfo(TUint32 aOtherId)
				: iId(0),
				  iOtherId(aOtherId),
				  iPriority(0) { iName.FillZ(); };

public:

	TUint32 iId;
	TUint32 iOtherId;
	TUint32 iPriority;	
	TBuf8<KMaxName> iName;
};


//
// class TMetroTrkStepInfo
//
class TMetroTrkStepInfo
{
public:

	inline TMetroTrkStepInfo(const TUint32 aStartAddress, const TUint32 aStopAddress, const TBool aStepInto)
				: iStartAddress(aStartAddress),
				  iStopAddress(aStopAddress),
				  iStepInto(aStepInto) {};

public:

	TUint32 iStartAddress;
	TUint32 iStopAddress;
	TBool iStepInto;
};


//
// class TMetroTrkDriverInfo
//
class TMetroTrkDriverInfo
{
public:

	TUint32 iPanic1Address;
	TUint32 iPanic2Address;
	TUint32 iException1Address;
	TUint32 iException2Address;
	TUint32 iLibraryLoadedAddress;
	TUint32 iUserLibraryEnd;
};


//
// class TMetroTrkProcessInfo
//
class TMetroTrkProcessInfo
{
public:

	inline TMetroTrkProcessInfo(TUint32 *aCodeAddress, TUint32 *aDataAddress)
				: iCodeAddress(aCodeAddress),
				  iDataAddress(aDataAddress) {};

public:

	TUint32* iCodeAddress;
	TUint32* iDataAddress;
};

class TMetroTrkLibInfo
{
public:

	inline TMetroTrkLibInfo(TUint32 aLength, TDesC8* aFileName)
				: iCodeAddress(0xFFFFFFFF),
				  iDataAddress(0xFFFFFFFF),
				  iAttachProcessId(0),
				  iAttachThreadId(0),
				  iFileNameLength(aLength),
				  iFileName(aFileName)
				  {};
	
public:
	//TBuf8<KMaxFileName> iFileName;
	TUint32 iCodeAddress;
	TUint32 iDataAddress;
	TUint32 iAttachProcessId;
	TUint32 iAttachThreadId;
	TUint32 iFileNameLength;
	TDesC8* iFileName;		
};

class TMetroTrkExeInfo
{
public:

	inline TMetroTrkExeInfo(TUint32 aUid, TUint32 aLength, TDesC8* aFileName)
				: iProcessID(0),
				  iThreadID(0),
				  iUid(aUid),
				  iCodeAddress(0xFFFFFFFF),
				  iDataAddress(0xFFFFFFFF),
				  iFileNameLength(aLength),
				  iFileName(aFileName)
				  {};
	
public:
	TUint32 iProcessID;
	TUint32 iThreadID;
	TUint32 iUid;
	TUint32 iCodeAddress;
	TUint32 iDataAddress;
	TUint32 iFileNameLength;
	TDesC8* iFileName;		
};

class TMetroTrkProcUidInfo
{
public:
	inline TMetroTrkProcUidInfo(TUint32 aProcID)
				: iProcessID(aProcID),				  
				  iUid1(0),
				  iUid2(0),
				  iUid3(0),
				  iSecurID(0),
				  iVendorID(0)
				  {};
	
public:
	TUint32 iProcessID;
	TUint32 iUid1;
	TUint32 iUid2;
	TUint32 iUid3;
	TUint32 iSecurID;
	TUint32 iVendorID;
};

class TTrkLibName
{
	public:
		inline TTrkLibName() { iName.FillZ(); iEmptySlot = ETrue;};
		inline TTrkLibName(TDesC8 aName) { iName.Copy(aName); iEmptySlot = EFalse; };

		TBuf8<KMaxLibraryName> iName;
		TBool iEmptySlot;
};

//
// class RMetroTrkDriver
//
class RMetroTrkDriver : public RBusLogicalChannel
{
public:

	enum TControl
	{
		EControlSetBreak = 0,
		EControlClearBreak,
		EControlChangeBreakThread,
		EControlSuspendThread,
		EControlResumeThread,
		EControlStepRange,
		EControlReadMemory,
		EControlWriteMemory,
		EControlReadRegisters,
		EControlWriteRegisters,
		EControlGetProcessInfo,
		EControlGetThreadInfo,
		EControlGetProcessAddresses,
		EControlGetStaticLibraryInfo,
		EControlEnableLibLoadedEvent,
		EControlDisableLibLoadedEvent,
		EControlGetLibraryInfo,
		EControlGetExeInfo,
		EControlGetProcUidInfo,
		EControlDetachProcess
	};
	
	enum TRequest
	{
		ERequestGetEvent=0x0, ERequestGetEventCancel=0x1
	};	
		
public:

	inline TInt Open(const TMetroTrkDriverInfo aDriverInfo);
	inline TInt	SetBreak(const TUint32 aProcessId, const TUint32 aThreadId, const TUint32 aAddr, const TBool aThumbMode, TInt32 &aId);
	inline TInt	ClearBreak(const TInt32 aId);
	inline TInt	ChangeBreakThread(const TUint32 aThreadId, const TInt32 aId);
	inline TInt	SuspendThread(const TUint32 aThreadId);
	inline TInt	ResumeThread(const TUint32 aThreadId);
	inline TInt	StepRange(const TUint32 aThreadId, const TUint32 aStartAddress, const TUint32 aStopAddress, TBool aStepInto);
	inline TInt ReadMemory(const TUint32 aThreadId, const TUint32 aAddress, const TInt16 aLength, TDes8 &aData);
	inline TInt WriteMemory(const TUint32 aThreadId, const TUint32 aAddress, const TInt16 aLength, TDesC8 &aData);
	inline TInt ReadRegisters(const TUint32 aThreadId, const TInt32 aFirstRegister, const TInt32 aLastRegister, TDes8 &aValues);
	inline TInt WriteRegisters(const TUint32 aThreadId, const TInt32 aFirstRegister, const TInt32 aLastRegister, TDesC8 &aValues);
	inline void GetEvent(TRequestStatus &aStatus, SEventInfo &aEventInfo);
	inline void CancelGetEvent();
	inline TInt GetProcessInfo(const TInt aIndex, TMetroTrkTaskInfo &aInfo);
	inline TInt GetThreadInfo(const TInt aIndex, TMetroTrkTaskInfo &aInfo);
	inline TInt GetProcessAddresses(const TUint32 aThreadId, TUint32 &aCodeAddress, TUint32 &aDataAddress);
	inline TInt GetStaticLibraryInfo(const TInt aIndex, SEventInfo &aInfo);
	inline TInt EnableLibLoadedEvent();
	inline TInt DisableLibLoadedEvent();
	inline TInt GetLibraryInfo(TMetroTrkLibInfo &aInfo);
	inline TInt GetExeInfo(TMetroTrkExeInfo &aInfo);
	inline TInt GetProcUidInfo(TMetroTrkProcUidInfo &aInfo);
	inline TInt DetachProcess(const TUint32 &aProcessId);

};


_LIT(KMetroTrkDriverName,"Trk Driver");

// Version information
const TInt KMajorVersionNumber=2;
const TInt KMinorVersionNumber=0;
const TInt KBuildVersionNumber=3;


inline TInt RMetroTrkDriver::Open(const TMetroTrkDriverInfo aDriverInfo)
{
	TBuf8<32> buf;
	buf.Append((TUint8*)&aDriverInfo.iPanic1Address, 4);
	buf.Append((TUint8*)&aDriverInfo.iPanic2Address, 4);
	buf.Append((TUint8*)&aDriverInfo.iException1Address, 4);
	buf.Append((TUint8*)&aDriverInfo.iException2Address, 4);
	buf.Append((TUint8*)&aDriverInfo.iLibraryLoadedAddress, 4);
	buf.Append((TUint8*)&aDriverInfo.iUserLibraryEnd, 4);
	
	#ifdef EKA2
	return DoCreate(KMetroTrkDriverName, TVersion(KMajorVersionNumber, KMinorVersionNumber, KBuildVersionNumber), KNullUnit, NULL, &buf);
	#else
	return DoCreate(KMetroTrkDriverName, TVersion(KMajorVersionNumber, KMinorVersionNumber, KBuildVersionNumber), NULL, KNullUnit, NULL, &buf);
	#endif
}

inline TInt RMetroTrkDriver::SetBreak(const TUint32 aProcessId, const TUint32 aThreadId, const TUint32 aAddr, const TBool aThumbMode, TInt32 &aId)
{
	TMetroTrkBreakInfo info(aAddr, aThumbMode, &aId, aProcessId);
	return DoSvControl(EControlSetBreak, reinterpret_cast<TAny*>(aThreadId), (TAny*)&info);
}

inline TInt RMetroTrkDriver::ClearBreak(const TInt32 aId)
{
	return DoSvControl(EControlClearBreak, reinterpret_cast<TAny*>(aId));
}

inline TInt RMetroTrkDriver::ChangeBreakThread(const TUint32 aThreadId, const TInt32 aId)
{
	return DoControl(EControlChangeBreakThread, reinterpret_cast<TAny*>(aThreadId), reinterpret_cast<TAny*>(aId));
}

inline TInt RMetroTrkDriver::SuspendThread(const TUint32 aThreadId)
{
	return DoControl(EControlSuspendThread, reinterpret_cast<TAny*>(aThreadId));
}

inline TInt RMetroTrkDriver::ResumeThread(const TUint32 aThreadId)
{
	return DoSvControl(EControlResumeThread, reinterpret_cast<TAny*>(aThreadId));
}

inline TInt RMetroTrkDriver::StepRange(const TUint32 aThreadId, const TUint32 aStartAddress, const TUint32 aStopAddress, TBool aStepInto)
{
	TMetroTrkStepInfo info(aStartAddress, aStopAddress, aStepInto);
	return DoSvControl(EControlStepRange, reinterpret_cast<TAny*>(aThreadId), (TAny*)&info);
}

inline TInt RMetroTrkDriver::ReadMemory(const TUint32 aThreadId, const TUint32 aAddress, const TInt16 aLength, TDes8 &aData)
{
	TMetroTrkMemoryInfo info(aAddress, aLength, &aData);
	return DoControl(EControlReadMemory, reinterpret_cast<TAny*>(aThreadId), (TAny*)&info);
}

inline TInt RMetroTrkDriver::WriteMemory(const TUint32 aThreadId, const TUint32 aAddress, const TInt16 aLength, TDesC8 &aData)
{
	TMetroTrkMemoryInfo info(aAddress, aLength, &aData);
	return DoControl(EControlWriteMemory, reinterpret_cast<TAny*>(aThreadId), (TAny*)&info);
}

inline TInt RMetroTrkDriver::ReadRegisters(const TUint32 aThreadId, const TInt32 aFirstRegister, const TInt32 aLastRegister, TDes8 &aValues)
{
	TMetroTrkRegisterInfo info(aFirstRegister, aLastRegister, &aValues);
	return DoControl(EControlReadRegisters, reinterpret_cast<TAny*>(aThreadId), (TAny*)&info);
}

inline TInt RMetroTrkDriver::WriteRegisters(const TUint32 aThreadId, const TInt32 aFirstRegister, const TInt32 aLastRegister, TDesC8 &aValues)
{
	TMetroTrkRegisterInfo info(aFirstRegister, aLastRegister, &aValues);
	return DoControl(EControlWriteRegisters, reinterpret_cast<TAny*>(aThreadId), (TAny*)&info);
}

inline void RMetroTrkDriver::GetEvent(TRequestStatus &aStatus, SEventInfo &aEventInfo)
{
	DoRequest(ERequestGetEvent, aStatus, (TAny*)&aEventInfo);
}

inline void RMetroTrkDriver::CancelGetEvent()
{
	DoCancel(ERequestGetEventCancel);
}

inline TInt RMetroTrkDriver::GetProcessInfo(const TInt aIndex, TMetroTrkTaskInfo &aInfo)
{
	return DoControl(EControlGetProcessInfo, reinterpret_cast<TAny*>(aIndex), (TAny*)&aInfo);
}

inline TInt RMetroTrkDriver::GetThreadInfo(const TInt aIndex, TMetroTrkTaskInfo &aInfo)
{
	return DoControl(EControlGetThreadInfo, reinterpret_cast<TAny*>(aIndex), (TAny*)&aInfo);
}

inline TInt RMetroTrkDriver::GetProcessAddresses(const TUint32 aThreadId, TUint32 &aCodeAddress, TUint32 &aDataAddress)
{
	TMetroTrkProcessInfo info(&aCodeAddress, &aDataAddress);
	return DoControl(EControlGetProcessAddresses, reinterpret_cast<TAny*>(aThreadId), (TAny*)&info);
}

inline TInt RMetroTrkDriver::GetStaticLibraryInfo(const TInt aIndex, SEventInfo &aInfo)
{
	return DoControl(EControlGetStaticLibraryInfo, reinterpret_cast<TAny*>(aIndex), (TAny*)&aInfo);
}

inline TInt RMetroTrkDriver::EnableLibLoadedEvent()
{
	return DoControl(EControlEnableLibLoadedEvent);
}

inline TInt RMetroTrkDriver::DisableLibLoadedEvent()
{
	return DoControl(EControlDisableLibLoadedEvent);
}

inline TInt RMetroTrkDriver::GetLibraryInfo(TMetroTrkLibInfo &aInfo)
{
	return DoControl(EControlGetLibraryInfo, (TAny*)&aInfo);
}

inline TInt RMetroTrkDriver::GetExeInfo(TMetroTrkExeInfo &aInfo)
{
	return DoControl(EControlGetExeInfo, (TAny*)&aInfo);
}

inline TInt RMetroTrkDriver::GetProcUidInfo(TMetroTrkProcUidInfo &aInfo)
{
	return DoControl(EControlGetProcUidInfo, (TAny*)&aInfo);
}

inline TInt RMetroTrkDriver::DetachProcess(const TUint32 &aProcessId)
{
    return DoControl(EControlDetachProcess, (TAny*)&aProcessId);
}
#endif // __TRKKERNELDRIVER_H__
