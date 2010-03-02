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

#ifndef __toolsclientsession_h__
#define __toolsclientsession_h__

// System includes
#include <e32base.h>

enum TConnectionStatus
{
    EUsbNotConnected,
    EUsbConnected
    
};
typedef TPckg<TConnectionStatus> TConnPkg; 

// User includes

// Classes referenced

//The client-side interface to the MetroTrk Tcb server. 
class RToolsClientSession : public RSessionBase
{
	// CONNECT TO SERVER & VERSIONING
	public:										
	    IMPORT_C   RToolsClientSession();
		IMPORT_C TInt Connect();
		IMPORT_C TVersion	Version() const;

	
	public:										
		
		
		IMPORT_C TInt   GetConnStatus(TConnectionStatus& aConnStatus);
		IMPORT_C void   UsbConnNotify(TDes8 &aDes ,TRequestStatus& aStatus);
		IMPORT_C TInt   UsbConnNotifyCancel();
		IMPORT_C TInt   ShutDownServer();

};

#endif //__toolsclientsession_h__
