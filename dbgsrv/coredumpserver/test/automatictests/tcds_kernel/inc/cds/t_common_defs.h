/**
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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



/**
 @file t_common_defs.h
 @internalTechnology
 @prototype
*/

#ifndef __COMMON_DEFS__
#define __COMMON_DEFS__

/** CDS UID */
const TUid KCDSUid = TUid::Uid(0x10282FE5);

/** UTRACE Formatter UID */
const TUid KUtraceUid = TUid::Uid(0x102836BA);

/** File Writer UID */
const TUid KUidFileWriter= TUid::Uid(0x102831e4);

/** Symbian ELF formatter UID */
const TUid KUidELFFormatter = TUid::Uid(0x10282fe3);

const TUid KUidELFFormatterV2 = TUid::Uid(0x102836bb);

/** Expected Crash File Name */
_LIT(KCrashFileName, "e:\\naibmys");

/** wildcard used to search for the SELF File created for kernel crash*/
_LIT(KCrashWildCard, "naibmys*");

/** Setting Directory Path to E:\\ MMC card */
_LIT(KDir, "e:\\");

/** Thread name of the test driver that crashes */
_LIT(KThreadKernelCrash, "ekern.exe::crashdriver");

/** Thread name of the user application that crashed */
_LIT(KThreadUserCrash, "crash");

/** The Debug Security Server's major version number. */
const TUint KDebugServMajorVersionNumber=2;

/** The Debug Security Server's minor version number. */
const TUint KDebugServMinorVersionNumber=2;

/** The Debug Security Server's patch version number. */
const TUint KDebugServPatchVersionNumber=0;

/** User mode CPSR **/
const TUint32 KCPSRUsrMode = 0x10;

/** 0x1000 ARM REGISTER CPSR **/
const TUint32 KCPSRReg = 0x1000; 

/** M[4:0] mode bits of the CPSR **/
const TUint32 KModeBitCPSR = 0x0F;

/** Trace category to be checked for **/
const TInt KTraceCategory = 200;

/** Maximum Time Out Value to be checked with the SELF File Creation time **/
const TInt KMAXTIMEOUT = 25;

/** INI File parameter for the crashapp */
_LIT(KTe_CrashAppParam,"CrashAppParam");

/** crashapp application used to crash the kernel */
_LIT(KCrashAppFileName,"Z:\\sys\\bin\\crashapp.exe");


_LIT( KPostCrashEventActionPrompt, "Action After Crash:\n  0-None,\n  1-Resume Thread,\n  2-Resume Process,\n  4-Kill Process" );
_LIT(KSELFFileName,"selffile");
_LIT(KFlashDumpFileName,"Z:\\sys\\bin\\flashdump.exe");
_LIT(KTe_FlashDumpParam,"E:\\thedump");
_LIT(KEndOfProcessing, "idle");
_LIT( KFilePathPrompt, "not_important" );
_LIT(KStartOfSELFProc, "-");
_LIT( KDataSegmentPrompt, "Create Data Segments?" );
_LIT( KCodeSegmentPrompt, "Create Code Segments?" );
_LIT( KThreadSegmentPrompt, "Create Thread Segments?" );
_LIT( KTrueFalseOpt, "True,False" );
_LIT( KTrueOpt, "True" );
_LIT( KFalseOpt, "False" );
_LIT( KCreateTraceData, "How much Trace data to capture (Kb)" );




#endif // __CPROCESSCRASHWRAPPER_H__
