/**
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* This file define all the common values thoughout your test project
*
*/



/**
 @file Te_coredumpserverSuiteDefs.h
 @internalTechnology
*/
#if (!defined __TE_COREDUMPSERVER_SUITEDEFS_H__)
#define __TE_COREDUMPSERVER_SUITEDEFS_H__

#include <rm_debug_api.h>

using namespace Debug;

static const TUid SELF_UID = TUid::Uid(0x10282FE3);  // For UID allocations see http://smglinx.intra/twiki/bin/view/PTD/CoreToolsDebugCoreDump
static const TUid CDS_UID  = TUid::Uid(0x10282FE5);
static const TUid DEXEC_UID = TUid::Uid(0x102832C5);
static const TUid WRITER_UID = TUid::Uid(0x102831E4);
static const TUid UTRACE_UID = TUid::Uid(0x102836BA);
static const TUid KUidSELFFormatterV2 = TUid::Uid(0x102836bb);

// Please modify below value with your project and must match with your configuration ini file which is required to be modified as well
_LIT(KTe_coredumpserverSuiteString,"TheString");
_LIT(KTe_coredumpserverSuiteInt,"TheInt");
_LIT(KTe_coredumpserverSuiteBool,"TheBool");
_LIT(KCrashAppFileName,"Z:\\sys\\bin\\crashapp.exe");
_LIT(KCrashAppFileNameNonDebugBit,"Z:\\sys\\bin\\crashapp_nodebugbit.exe");
_LIT( KPostCrashEventActionPrompt, "Action After Crash:\n  0-None,\n  1-Resume Thread,\n  2-Resume Process,\n  4-Kill Process" );

_LIT(KTe_CrashAppParam,"CrashAppParam");
_LIT(KTe_CrashFileName,"CrashFileName");
_LIT(KWriterToUse, "WriterToUse");
_LIT(KTrace, "Trace");
_LIT(KNumberofCrashes, "CrashNumber");


/** Expected Crash File Name */
_LIT(KCrashFileName, "e:\\naibmys");
/** String to be match when read from the INI file */
_LIT(KTraceData, "Trace");
/** wildcrad used to search for the File created */
_LIT(KCrashWildCard, "naibmys*");
/** Setting Directory Path to E:\\ MMC card */
_LIT(KDir, "e:\\");
_LIT( KFilePathPrompt, "not_important" );

const TVersion securityServerVersion( 
		KDebugServMajorVersionNumber, 
		KDebugServMinorVersionNumber, 
		KDebugServPatchVersionNumber );

//These need to corrospond to those being set in the SELF formatter
_LIT(KStartOfSELFProc, "-");
_LIT(KStartOfUTraceProc, "started utrace formatting");

_LIT(KStartOfDEXECProc, "crash dump started");
_LIT(KEndOfProcessing, "idle");

// For test step panics
_LIT(KTe_coredumpserverSuitePanic,"Te_coredumpserverSuite");

#endif
