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

#ifndef __TrkTcbCmdCodes_H__
#define __TrkTcbCmdCodes_H__

// System includes
#include <e32std.h>

// Constants
const TInt KTrkTcbServerMajorVN = 1;
const TInt KTrkTcbServerMinorVN = 0;
const TInt KTrkTcbServerBuildVN = 0;

// asynchronous cmd slot count
const TInt KTrkTcbServerAsynchronousSlotCount	= 0;

//
const TUid KTrkTcbServerUid = { 0x200159DB };
const TUid KTrkAppUid = { 0x200170BB };
const TUid KTrkExeUid = { 0x200159E2 };
const TUid KTrkSrvUid = { 0x200170B7 };

// Literal constants

#if defined(EKA2)
_LIT(KTrkTcbServerName, "!TrkTcbServer");
#else
_LIT(KTrkTcbServerName, "TrkTcbServer");
#endif

_LIT(KTrkTcbServerImageName, "TrkTcbServer");

_LIT(KServerIntiatedSessionPanic, "TRKTCBSERVER");

#define KCapabilityCustomCheck 0

// Enumerations
//
// The cmd-codes used to communicate between the TrkTcb client and TrkTcb server.
//
enum TTrkTcbCmdCode
{
	//Custom check
	ETrkTcbCmdCodeFirst = KCapabilityCustomCheck,				
	ETrkTcbCmdCodeOpenFile,					
	ETrkTcbCmdCodeReadFile,						
	ETrkTcbCmdCodeWriteFile,
	ETrkTcbCmdCodeCloseFile,
	ETrkTcbCmdCodePositionFile,
	ETrkTcbCmdCodeShutDownServer,
	ETrkTcbCmdCodeLast								
};

// Identifies server-initiated panics relating to the client session.
enum TTrkTcbServerInitiatedClientPanic
{
	//This panic occurs when the client requests a copy of any data attached to any 
	//read request, but does not supply enough buffer space to contain the data. 
	ETrkTcbServerInitiatedClientPanicInsufficientRoomForFileData = 1,

	//This panic usually occurs when a client method tries to write to a descriptor 
	//(sometimes asynchronously), and the client-supplied descriptor is not valid.
	ETrkTcbServerInitiatedClientPanicBadDescriptor = 2,


	//This panic occurs when a client tries to perform an invalid operation.
	ETrkTcbServerInitiatedClientPanicInvalidOperation = 3,
};

#endif //__TrkTcbCmdCodes_H__

