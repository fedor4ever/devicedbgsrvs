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

#ifndef __DbgTrcCmdCodes_H__
#define __DbgTrcCmdCodes_H__

// System includes
#include <e32std.h>

// Constants
const TInt KDbgTrcServerMajorVN = 1;
const TInt KDbgTrcServerMinorVN = 0;
const TInt KDbgTrcServerBuildVN = 0;

// asynchronous cmd slot count
const TInt KDbgTrcServerAsynchronousSlotCount	= 16;

//
const TUid KDbgTrcServerUid = { 0x200170BE };


// Literal constants

#if defined(EKA2)
_LIT(KDbgTrcServerName, "!UsbOstRouter");
#else
_LIT(KDbgTrcServerName, "UsbOstRouter");
#endif

_LIT(KDbgTrcServerImageName, "usbostrouter");

_LIT(KServerIntiatedSessionPanic, "USBOSTROUTER");

#define KCapabilityCustomCheck 0

// Enumerations
//
// The cmd-codes used to communicate between the DbgTrc client and DbgTrc server.
//
enum TDbgTrcCmdCode
{
	//Custom check
	EDbgTrcCmdCodeFirst = KCapabilityCustomCheck,
	EDbgTrcCmdConnect,
	EDbgTrcCmdDisconnect,	
	EDbgTrcCmdCodeGetAcmConfig,
	EDbgTrcCmdCodeSetAcmConfig,
	EDbgTrcCmdCodeOpen,					
	EDbgTrcCmdCodeClose,						
	EDbgTrcCmdCodeRegisterId,
	EDbgTrcCmdCodeRegisterIds,
	EDbgTrcCmdCodeUnRegisterId,
	EDbgTrcCmdCodeUnRegisterIds,
	EDbgTrcCmdCodeReadMsg,
	EDbgTrcCmdCodeReadCancel,
	EDbgTrcCmdCodeWriteMsg,
	EDbgTrcCmdCodeWriteCancel,
	EDbgTrcCmdCodeLast								
};

// Identifies server-initiated panics relating to the client session.
enum TDbgTrcServerInitiatedClientPanic
{

	//This panic occurs when a client tries to perform an invalid operation.
	EDbgTrcServerInitiatedClientPanicInvalidOperation = 1,

	//This panic usually occurs when a client method tries to write to a descriptor 
	//(sometimes asynchronously), and the client-supplied descriptor is not valid.
	EDbgTrcServerInitiatedClientPanicBadDescriptor = 2,

};

#endif //__DbgTrcCmdCodes_H__

