/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef __TrkCmdCodes_H__
#define __TrkCmdCodes_H__

// System includes
#include <e32std.h>

// Constants
const TInt KTrkServerMajorVN = 1;
const TInt KTrkServerMinorVN = 0;
const TInt KTrkServerBuildVN = 0;

// asynchronous cmd slot count
const TInt KTrkServerAsynchronousSlotCount	= 8;

//
const TUid KTrkServerUid = { 0x200170B7 };
const TUid KTrkAppUid = { 0x200170BB };
const TUid KTrkExeUid = { 0x200159E2 };

// Literal constants

#if defined(EKA2)
_LIT(KTrkServerName, "!TrkServer");
#else
_LIT(KTrkServerName, "TrkServer");
#endif

_LIT(KTrkServerImageName, "TrkServer");

_LIT(KServerIntiatedSessionPanic, "TRKSERVER");

#define KCapabilityCustomCheck 0

// Enumerations
//
// The cmd-codes used to communicate between the Trk client and Trk server.
//
enum TTrkCmdCode
{
	//Custom check
	ETrkSrvCmdCodeFirst = KCapabilityCustomCheck,
	ETrkSrvGetVersion,
	ETrkSrvCmdConnect,					
	ETrkSrvCmdDisConnect,
	ETrkSrvCmdGetCurrentConnData,
	ETrkSrvCmdSetCurrentConnData,
	ETrkSrvCmdGetDebugConnStatus,
	ETrkSrvDebugConnStatus,
	ETrkSrvDebugConnStatusCancel,
	ETrkSrvCmdGetDebuggingStatus,
	ETrkSrvDebuggingStatus,
	ETrkSrvDebuggingStatusCancel,
	ETrkGetPlugAndPlayType,
	ETrkSetPlugAndPlayType,
	ETrkSrvCmdCodeShutDownServer,
	ETrkCmdCodeLast								
};

// Identifies server-initiated panics relating to the client session.
enum TTrkServerInitiatedClientPanic
{
	//This panic occurs when the client requests a copy of any data attached to any 
	//read request, but does not supply enough buffer space to contain the data. 
	ETrkServerInitiatedClientPanicInsufficientRoomForFileData = 1,

	//This panic usually occurs when a client method tries to write to a descriptor 
	//(sometimes asynchronously), and the client-supplied descriptor is not valid.
	ETrkServerInitiatedClientPanicBadDescriptor = 2,


	//This panic occurs when a client tries to perform an invalid operation.
	ETrkServerInitiatedClientPanicInvalidOperation = 3,
};

class TTRKVersion
{
public:
	TInt iMajor;
	TInt iMinor;
	TInt iMajorAPI;
	TInt iMinorAPI;
	TInt iBuild;
};

#endif //__TrkCmdCodes_H__

