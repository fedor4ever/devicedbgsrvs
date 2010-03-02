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

#ifndef __TrkSrvCliSession_h__
#define __TrkSrvCliSession_h__

// System includes
#include <e32base.h>
#include <f32file.h>
#include "TrkConnData.h"
// User includes

// Classes referenced
typedef TPckg<TBool> TDebugState;
typedef TPckg<TTrkConnStatus> TConnState;

//class TTrkConnData;


//The client-side interface to the Trk server. 
class RTrkSrvCliSession : public RSessionBase
{
	// CONNECT TO SERVER & VERSIONING
	public:										
		 			RTrkSrvCliSession();
		TInt		ConnectToServer();

	// TRK server CMD SPECIFIC FUNCTIONALITY
	public:										
		TInt	GetTrkVersion(TInt& aMajorVersion, TInt& aMinorVersion, TInt& aMajorAPIVersion, TInt& aMinorAPIVersion, TInt& aBuildNumber);
		TInt	Connect();
		TInt	DisConnect();
		TInt    GetCurrentConnData(TTrkConnData& aConnData);
		TInt    SetCurrentConnData(TTrkConnData& aConnData);
		TInt	GetDebugConnStatus(TTrkConnStatus& aConnStatus, TDes& aConnMsg);
		void    DebugConnStatusNotify(TDes8& aConnStatus, TDes& aConnMsg, TRequestStatus& aStatus);
        TInt    DebugConnStatusNotifyCancel();
		TInt	GetDebuggingStatus(TBool& aDebugging);
		void    DebuggingStatusNotify(TDes8& aDebugging, TRequestStatus& aStatus);
		TInt    DebuggingStatusNotifyCancel();
		TInt    GetPlugAndPlayType(TBool& aPlugandPlay);
		TInt    SetPlugAndPlayType(TBool aPlugandPlay);
		TInt	ShutDownServer();
};

#endif //__TrkSrvCliSession_h__
