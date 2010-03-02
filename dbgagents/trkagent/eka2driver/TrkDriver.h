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


#ifndef __TRKDRIVER_H__
#define __TRKDRIVER_H__

// Debug messages
#ifdef _DEBUG
#define LOG_MSG(x) __KTRACE_OPT(KDEBUGGER, Kern::Printf(x))
#define LOG_MSG2(x, y) __KTRACE_OPT(KDEBUGGER, Kern::Printf(x, y))
#else
#define LOG_MSG(x)
#define LOG_MSG2(x, y)
#endif


//
// Macros
//
const TUint32 KArmBreakPoint = 0xE7F123F4;
const TUint16 KThumbBreakPoint = 0xDE56;

// From mmboot.h header
const TLinAddr	KDataSectionEnd			=0x40000000u;
const TLinAddr	KRomLinearBase			=0xF8000000u;

const TLinAddr	KSuperPageMovingLinAddr		=0x60000000u;
const TLinAddr	KSuperPageMultipleLinAddr	=0xC0000000u;
const TLinAddr	KKernDataEndMovingLinAddr	=0x655FFFFFu;//0xF3FFFFFFu;
const TLinAddr	KKernDataEndMultipleLinAddr	=0xFFEFFFFFu;

#define NUMBER_OF_TEMP_BREAKPOINTS 10
#define NUMBER_OF_EVENTS_TO_QUEUE 50
#define NUMBER_OF_LIBS_TO_REGISTER 100
#define ROM_LINEAR_BASE KRomLinearBase

//for multiple memory model
#define NUMBER_OF_MAX_BREAKPOINTS 100

// Result checking
#define ReturnIfError(x) { TInt y = x; if (KErrNone != y) return y; }

// Register definitions
#define SP_REGISTER			13
#define LINK_REGISTER		14
#define PC_REGISTER			15
#define STATUS_REGISTER		16

// ARM instruction bitmasks
#define ARM_OPCODE(x)		(((TUint32)(x) & 0x0E000000) >> 25)

// Generic instruction defines
#define ARM_RM(x)				((TUint32)(x) & 0x0000000F)			// bit 0- 4
#define ARM_RS(x)				(((TUint32)(x) & 0x00000F00) >> 8)	// bit 8-11
#define ARM_RD(x)				(((TUint32)(x) & 0x0000F000) >> 12)	// bit 12-15
#define ARM_RN(x)				(((TUint32)(x) & 0x000F0000) >> 16)	// bit 16-19
#define ARM_LOAD(x)				(((TUint32)(x) & 0x00100000) >> 20)	// bit 20

// Data processing instruction defines
#define ARM_DATA_SHIFT(x)		(((TUint32)(x) & 0x00000060) >> 5) 	// bit 5- 6
#define ARM_DATA_C(x)			(((TUint32)(x) & 0x00000F80) >> 7) 	// bit 7-11
#define ARM_DATA_IMM(x)			((TUint32)(x) & 0x000000FF)			// bit 0-7
#define ARM_DATA_ROT(x)			(((TUint32)(x) & 0x00000F00) >> 8) 	// bit 8-11

// Single date transfer instruction defines
#define ARM_SINGLE_IMM(x)		((TUint32)(x) & 0x00000FFF)			// bit 0-11
#define ARM_SINGLE_BYTE(x)		(((TUint32)(x) & 0x00400000) >> 22)	// bit 22
#define ARM_SINGLE_U(x)			(((TUint32)(x) & 0x00800000) >> 23)	// bit 23
#define ARM_SINGLE_PRE(x)		(((TUint32)(x) & 0x01000000) >> 24)	// bit 24

// Block data transfer instruction defines
#define ARM_BLOCK_REGLIST(x)	((TUint32)(x) & 0x0000FFFF)		// bit 0-15
#define ARM_BLOCK_U(x)			(((TUint32)(x) & 0x00800000) >> 23)	// bit 23
#define ARM_BLOCK_PRE(x)		(((TUint32)(x) & 0x01000000) >> 24)	// bit 24

// Branch instruction defines
#define ARM_B_ADDR(x)			((x & 0x00800000) ? ((TUint32)(x) & 0x00FFFFFF | 0xFF000000) : (TUint32)(x) & 0x00FFFFFF)
#define ARM_INSTR_B_DEST(x,a)	(ARM_B_ADDR(x) << 2) + ((TUint32)(a) + 8)

#define ARM_CARRY_BIT			0x20000000	// bit 30


// Thumb instruction bitmasks
#define THUMB_OPCODE(x)		(((TUint16)(x) & 0xF800) >> 11)
#define THUMB_INST_7_15(x)	(((TUint16)(x) & 0xFF80) >> 7)
#define THUMB_INST_8_15(x)	(((TUint16)(x) & 0xFF00) >> 8)


const TUint8 KArm4Stub[8] = { 0x00, 0xC0, 0x9F, 0xE5, 0x00, 0xF0, 0x9C, 0xE5 };
const TUint8 KArmIStub[12] = { 0x04, 0xC0, 0x9F, 0xE5, 0x00, 0xC0, 0x9C, 0xE5, 0x1C, 0xFF, 0x2F, 0xE1 };
const TUint8 KFastArmIStub[12] = { 0x04, 0xC0, 0x9F, 0xE5, 0x1C, 0xFF, 0x2F, 0xE1, 0x00, 0x00, 0x00, 0x00 };
const TUint8 KThumbStub[12] = { 0x40, 0xB4, 0x02, 0x4E, 0x36, 0x68, 0xB4, 0x46, 0x40, 0xBC, 0x60, 0x47 };
const TUint8 KThumbStub2[8] = { 0x01, 0x4B, 0x1B, 0x68, 0x18, 0x47, 0xC0, 0x46 };
const TUint8 KFastThumbStub[12] = { 0x40, 0xB4, 0x02, 0x4E, 0xB4, 0x46, 0x40, 0xBC, 0x60, 0x47, 0xC0, 0x46 };
const TUint8 KFastThumbStub2[8] = { 0x01, 0x4B, 0x18, 0x47, 0xC0, 0x46, 0xC0, 0x46 };

const unsigned char KRvctArm4Stub[4] = { 0x04, 0xF0, 0x1F, 0xE5 };

//
// class TBreakEntry
//
class TBreakEntry
{
public:

	inline TBreakEntry() { Reset(); };

	inline TBreakEntry(TUint32 aId, TUint32 aThreadId, TUint32 aAddress, TBool aThumbMode)
			: iId(aId),
			  iThreadId(aThreadId),
			  iAddress(aAddress),
			  iThumbMode(aThumbMode)
	{
		 iInstruction.FillZ(4);
		 iPageAddress = 0;
		 iDisabledForStep = EFalse;
		 iObsoleteLibraryBreakpoint = EFalse;
		 iResumeOnceOutOfRange = EFalse;
		 iSteppingInto = EFalse;
		 iRangeStart = 0;
		 iRangeEnd = 0;
		 iThreadSpecific = EFalse;
	};
	
	inline void Reset()
	{
		 iThreadId = 0;
		 iAddress = 0;
		 iThumbMode = EFalse;
		 iInstruction.FillZ(4);
		 iPageAddress = 0;
		 iDisabledForStep = EFalse;
		 iObsoleteLibraryBreakpoint = EFalse;
		 iResumeOnceOutOfRange = EFalse;
		 iSteppingInto = EFalse;
		 iRangeStart = 0;
		 iRangeEnd = 0;
	};

public:
			
	TInt32		iId;
	TUint32		iThreadId;
	TUint32		iAddress;
	TBool		iThumbMode;
	TBuf8<4>	iInstruction;
	TUint32		iPageAddress;
	TBool		iDisabledForStep;
	TBool		iObsoleteLibraryBreakpoint;
	TBool		iResumeOnceOutOfRange;
	TBool		iSteppingInto;
	TUint32		iRangeStart;
	TUint32		iRangeEnd;
	TBool		iThreadSpecific;
};

class TProcessInfo
{
	public:

		inline TProcessInfo() { Reset(); }

		inline TProcessInfo(TUint32 aId, TUint32 aCodeAddress, TUint32 aCodeSize, TUint32 aDataAddress)
				: iId(aId),
				  iCodeAddress(aCodeAddress),
				  iCodeSize(aCodeSize),
				  iDataAddress(aDataAddress) { }
		
		inline void Reset()
		{
			 iId = 0;
			 iCodeAddress = 0;
			 iCodeSize = 0;
			 iDataAddress = 0;
		}

	public:
		TUint32 iId;
		TUint32 iCodeAddress;
		TUint32 iCodeSize;
		TUint32 iDataAddress;
};


//
// class DMetroTrkDriverFactory
//
class DMetroTrkDriverFactory : public DLogicalDevice
{
public:

	DMetroTrkDriverFactory();
	virtual TInt Install();
	virtual void GetCaps(TDes8& aDes) const;
	virtual TInt Create(DLogicalChannelBase*& aChannel);
};

class DMetroTrkEventHandler;
//
// DMetroTrkChannel
//
class DMetroTrkChannel : public DLogicalChannel
{
public:

	DMetroTrkChannel(DLogicalDevice* aLogicalDevice);
	~DMetroTrkChannel();

	virtual TInt DoCreate(TInt aUnit, const TDesC* anInfo, const TVersion& aVer);	
	virtual void HandleMsg(TMessageBase* aMsg);
	
	//called from the event handler
	void AddProcess(DProcess *aProcess, DThread* aThread);
	void StartThread(DThread *aThread);
	void RemoveProcess(DProcess *aProcess);
	void AddLibrary(DLibrary *aLibrary, DThread *aThread);
	void RemoveLibrary(DLibrary *aLibrary);
	void AddCodeSegment(DCodeSeg *aCodeSeg, DProcess *aProcess);
	void RemoveCodeSegment(DCodeSeg *aCodeSeg, DProcess *aProcess);
	TBool HandleEventKillThread(DThread* aThread);
	TBool HandleSwException(TExcType aExcType);
	TBool HandleHwException(TArmExcInfo* aExcInfo);
	TBool HandleUserTrace(TText* aStr, TInt aLen);
	
protected:
	virtual void DoCancel(TInt aReqNo);
	virtual void DoRequest(TInt aReqNo, TRequestStatus* aStatus, TAny* a1, TAny* a2);
	virtual TInt DoControl(TInt aFunction, TAny *a1, TAny *a2);
	
private:
	void HandleException(SEventInfo& aEventInfo, DThread* aCurrentThread);

	TInt SetBreak(TUint32 aThreadId, TMetroTrkBreakInfo* aBreakInfo);
	TInt StepRange(DThread* aThread, TMetroTrkStepInfo* aStepInfo);
	TInt ReadMemory(DThread* aThread, TMetroTrkMemoryInfo* aMemoryInfo);
	TInt WriteMemory(DThread* aThread, TMetroTrkMemoryInfo* aMemoryInfo);
	TInt ReadRegisters(DThread* aThread, TMetroTrkRegisterInfo* aRegisterInfo);
	TInt WriteRegisters(DThread* aThread, TMetroTrkRegisterInfo* aRegisterInfo);
	TInt GetProcessInfo(TInt aIndex, TMetroTrkTaskInfo* aTaskInfo);
	TInt GetThreadInfo(TInt aIndex, TMetroTrkTaskInfo* aTaskInfo);
	TInt GetProcessAddresses(DThread* aThread, TMetroTrkProcessInfo* aProcessInfo);
	TInt GetStaticLibraryInfo(TInt aIndex, SEventInfo* aEventInfo);
	TInt GetLibraryInfo(TMetroTrkLibInfo* aLibInfo);
	TInt GetExeInfo(TMetroTrkExeInfo* aExeInfo);
	TInt GetProcUidInfo(TMetroTrkProcUidInfo* aProcUidInfo);
	TInt DetachProcess(DProcess* aProcess);

	TInt DoSetBreak(const TUint32 aProcessId, const TUint32 aThreadId, const TUint32 aAddress, const TBool aThumbMode, TInt32 &aId);
	TInt DoEnableBreak(TBreakEntry &aEntry, TBool aSaveOldInstruction);
	TInt DoClearBreak(const TInt32 aId);
	TInt DoChangeBreakThread(TUint32 aThreadId, TInt32 aId);
	TInt DoSuspendThread(DThread *aThread);
	TInt DoResumeThread(DThread *aThread);
	TInt DoStepRange(DThread *aThread, const TUint32 aStartAddress, const TUint32 aStopAddress, TBool aStepInto, TBool aResumeOnceOutOfRange, TBool aUserRequest = EFalse);
	TInt DoReadMemory(DThread *aThread, const TUint32 aAddress, const TInt16 aLength, TDes8 &aData);
	TInt DoWriteMemory(DThread *aThread, const TUint32 aAddress, const TInt16 aLength, TDes8 &aData);
	TInt DoReadRegisters(DThread *aThread, const TInt16 aFirstRegister, const TInt16 aLastRegister, TDes8 &aValues);
	TInt DoWriteRegisters(DThread *aThread, const TInt16 aFirstRegister, const TInt16 aLastRegister, TDesC8 &aValues);
	TInt DoGetProcessInfo(const TInt aIndex, TMetroTrkTaskInfo *aInfo);
	TInt DoGetThreadInfo(const TInt aIndex, TMetroTrkTaskInfo *aInfo);
	TInt DoGetProcessAddresses(DThread *aThread, TUint32 &aCodeAddress, TUint32 &aDataAddress);
	TInt DoGetStaticLibraryInfo(const TInt aIndex, SEventInfo *aInfo);
	TInt DoGetLibraryInfo(TDesC8 &aDllName, TMetroTrkLibInfo* aLibInfo);
	TInt DoGetExeInfo(TDesC8 &aExeName, TMetroTrkExeInfo* aExeInfo);
	TInt DoGetProcUidInfo(TMetroTrkProcUidInfo* aProcUidInfo);

	TBool DoSecurityCheck();
	TInt TryToReadMemory(DThread *aThread, TAny *aSrc, TAny *aDest, TInt16 aLength);
	TInt TryToWriteMemory(DThread *aThread, TAny *aDest, TAny *aSrc, TInt16 aLength);
	TInt32 ReadRegister(DThread *aThread, TInt aNum);
	TInt ModifyBreaksForStep(DThread *aThread, TUint32 aRangeStart, TUint32 aRangeEnd, TBool aStepInto, TBool aResumeOnceOutOfRange, TBool aCheckForStubs);
	TBool IsExecuted(TUint8 aCondition, TUint32 aStatusRegister);
	void ClearAllBreakPoints();
	TInt DisableBreakAtAddress(TUint32 aAddress);
	TInt DoEnableDisabledBreak(TUint32 aThreadId);
	TUint32 ShiftedRegValue(DThread *aThread, TUint32 aInstruction, TUint32 aCurrentPC, TUint32 aStatusRegister);
	TBool InstructionModifiesPC(DThread *aThread, TUint8 *aInstruction, TBool aThumbMode, TBool aStepInto);
	TUint32 PCAfterInstructionExecutes(DThread *aThread, TUint32 aCurrentPC, TUint32 aStatusRegister, TInt aInstSize, TBool aStepInto, TUint32 &aNewRangeEnd, TBool &aChangingModes);
	void DecodeDataProcessingInstruction(TUint8 aOpcode, TUint32 aOp1, TUint32 aOp2, TUint32 aStatusRegister, TUint32 &aBreakAddress);
	TBool IsPreviousInstructionMovePCToLR(DThread *aThread);
	
	TBool IsAddressInRom(TUint32 aAddress);
	TBool IsAddressSecure(TUint32 aAddress);
	TBool IsRegisterSecure(TInt registerIndex);

	TInt AllocateShadowPageIfNecessary(TUint32 aAddress, TUint32 &aPageAddress);
	TInt FreeShadowPageIfNecessary(TUint32 aAddress, TUint32 aPageAddress);

	void NotifyEvent(SEventInfo aEventInfo, TBool isTraceEvent=EFalse);
	DThread* ThreadFromId(TUint32 aId);
	DProcess* ProcessFromId(TUint32 aId);
	TBool GetSystemThreadRegisters(TArmRegSet* aArmRegSet);
	TInt DoGetLibInfoFromCodeSegList(TDesC8 &aDllName, TMetroTrkLibInfo *aInfo);
	TInt DoGetLibInfoFromKernLibContainer(TDesC8 &aDllName, TMetroTrkLibInfo *aInfo);

	TBool HasManufacturerCaps(DThread* aThread);
	TBool IsBeingDebugged(const DThread* aThread);

	void CheckLibraryNotifyList(TUint32 aProcessId);

private:
	DThread* iClientThread;
	DMetroTrkEventHandler* iEventHandler;
	
	TUint32 iExcludedROMAddressStart;
	TUint32 iExcludedROMAddressEnd;
	
	RArray<TBreakEntry> iBreakPointList;
	TInt iNextBreakId;
	
	SEventInfo *iEventInfo;
	RArray<SEventInfo> iEventQueue;
	RArray<SEventInfo> iTraceEventQueue;
	TRequestStatus* iRequestGetEventStatus;

	TUint32 iPageSize;
	
	TBool  iNotifyLibLoadedEvent;
	TBool  iMultipleMemModel;
	
	RArray<TProcessInfo> iDebugProcessList; //processes that we are debugging
	RArray<SEventInfo> iProcessNotifyList; 
	
	TDfcQue* iDFCQue;

	TArmExcInfo iCurrentExcInfo;
	TBool iExcInfoValid;
	TBool iDebugging;
	// PANIC_BACKPORT
	RPointerArray<NFastSemaphore> iFrozenThreadSemaphores;
	// END PANIC_BACKPORT
	RArray<TTrkLibName> iLibraryNotifyList;

};

#endif //__TRKDRIVER_H__
