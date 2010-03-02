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

#ifndef __toolscmdcodes_H__
#define __toolscmdcodes_H__

// System includes
#include <e32std.h>

// Constants
const TInt KToolsServerMajorVN = 1;
const TInt KToolsServerMinorVN = 0;
const TInt KToolsServerBuildVN = 0;

// asynchronous cmd slot count
const TInt KToolsServerAsynchronousSlotCount	= 16;

// Uids for platform security

const TUid KToolsClientUid = { 0x200170B4 };
const TUid KToolsServerUid = { 0x200170B5 };
const TUid KTraceServerUid = { 0x200170B6 };
const TUid KTrkServerUid   = { 0x200170B7 };

const TUid KMetroTrkAppUid = { 0x200170BB };
const TUid KMetroTrkExeUid = { 0x200159E2 };



// Literal constants



#if defined(EKA2)
_LIT(KToolsServerName, "!toolsstarterserver");
#else
_LIT(KToolsServerName, "toolsstarterserver");
#endif

_LIT(KToolsServerImageName, "toolsstarterserver");

_LIT(KServerIntiatedSessionPanic, "toolsstarterserver");

#define KCapabilityCustomCheck 0


// Enumerations
//
// The cmd-codes used to communicate between the Tools client and Tools server.
//
enum TToolsCmdCode
{
	//Custom check
	EToolsCmdCodeFirst = KCapabilityCustomCheck,				
	EToolsCmdCodeGetUsbConnStatus,
	EToolsCmdCodeConnNotify,
	EToolCmdCodeConnNotifyCancel,
	EToolsCmdCodeShutDownServer,
	EToolsCmdCodeLast								
};

// Identifies server-initiated panics relating to the client session.
enum TToolsServerInitiatedClientPanic
{
	//This panic occurs when the client requests a copy of any data attached to any 
	//read request, but does not supply enough buffer space to contain the data. 
	EToolsServerInitiatedClientPanicInsufficientRoomForFileData = 1,

	//This panic usually occurs when a client method tries to write to a descriptor 
	//(sometimes asynchronously), and the client-supplied descriptor is not valid.
	EToolsServerInitiatedClientPanicBadDescriptor = 2,


	//This panic occurs when a client tries to perform an invalid operation.
	EToolsServerInitiatedClientPanicInvalidOperation = 3,
};

#endif //__toolscmdCodes_H__

