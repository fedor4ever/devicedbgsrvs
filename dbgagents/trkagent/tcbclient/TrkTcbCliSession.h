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

#ifndef __TrkTcbCliSession_h__
#define __TrkTcbCliSession_h__

// System includes
#include <e32base.h>
#include <f32file.h>
// User includes

// Classes referenced

//The client-side interface to the MetroTrk Tcb server. 
class RTrkTcbCliSession : public RSessionBase
{
	// CONNECT TO SERVER & VERSIONING
	public:										
		 			RTrkTcbCliSession();
		TInt		Connect();
		TVersion	Version() const;

	// TCB CMD SPECIFIC FUNCTIONALITY
	public:										
		TInt	OpenFile(const TDesC& aFilePath, TUint aMode, TTime& aModifiedTime);
		TInt	ReadFile(TUint16 aLength, TDes8& aFileData);
		TInt	WriteFile(const TDesC8& aFileData);
		TInt	CloseFile(const TTime& aTime);
		TInt	PositionFile(TSeek aSeek, TInt& aOffset);
		TInt	ShutDownServer();

};

#endif //__TrkTcbCliSession_h__
