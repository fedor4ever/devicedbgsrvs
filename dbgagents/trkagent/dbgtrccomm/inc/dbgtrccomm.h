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

#ifndef __RDbgTrcComm_h__
#define __RDbgTrcComm_h__

// System includes
#include <e32base.h>

// User includes
#include "ostprotdefs.h"

#define SafeDelete(x) { if (x) delete x; x = NULL; }


/**
 Enumeration of baud rates in bits per second for ACM port configuration.
 * e.g EBaud1152000 is for 115200Bps data rate  
 */
enum TBaudRates
	{
	EBaud50,
	EBaud75,
	EBaud110,
	EBaud134,
	EBaud150,
	EBaud300,
	EBaud600,
	EBaud1200,
	EBaud1800,
	EBaud2000,
	EBaud2400,
	EBaud3600,
	EBaud4800,
	EBaud7200,
	EBaud9600,
	EBaud19200,
	EBaud38400,
	EBaud57600,
	EBaud115200,
	EBaud230400,
	EBaud460800,
	EBaud576000,
	EBaud1152000,
	EBaud4000000,
	EBaud921600,
	EBaudAutobaud=0x40000000,
	EBaudSpecial=0x80000000,
	};

class TAcmConfigV01
    {
public:
	TUint iPortNumber; // port number for ACM port is always 1 with S60 devices, atleast when using the PC Suite drivers.
	TBaudRates iBaudRate;
    };
typedef TPckgBuf< TAcmConfigV01> TAcmConfig;


// Classes referenced

//The client-side interface to the dbg trc comms server. 
class RDbgTrcComm : public RSessionBase
{
	// CONNECT TO SERVER & VERSIONING
	public:										
		IMPORT_C			RDbgTrcComm();
		IMPORT_C TInt		Connect();
		IMPORT_C TInt		Disconnect();
		IMPORT_C TVersion	Version() const;

	// CMD SPECIFIC FUNCTIONALITY
	public:										
		IMPORT_C TInt	GetAcmConfig(TDes8& aConfig);
		IMPORT_C TInt	SetAcmConfig(TDesC8& aConfig);
		
		IMPORT_C TInt	Open();
		IMPORT_C TInt	Close();
		IMPORT_C TInt	RegisterProtocolID(const TOstProtIds aId, TBool aNeedHeader);
		IMPORT_C TInt   RegisterProtocolIDs(const TOstProtIds* aIds, const TUint aNumberofIds, TBool aNeedHeader);
		IMPORT_C TInt	UnRegisterProtocolID(const TOstProtIds aId);
		IMPORT_C TInt   UnRegisterProtocolIDs(const TOstProtIds* aIds, const TUint aNumberofIds);

		IMPORT_C void 	ReadMessage(TRequestStatus& aStatus, TDes8& aDes);
		IMPORT_C TInt 	ReadCancel();
		IMPORT_C void 	WriteMessage(TRequestStatus& aStatus, const TDesC8& aDes, TBool aHasHeader=EFalse);
		IMPORT_C TInt 	WriteCancel();
};

#endif //__RDbgTrcComm_h__
