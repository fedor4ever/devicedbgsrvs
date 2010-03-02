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

#ifndef __TrkTcbSrvSessionEngine_h__
#define __TrkTcbSrvSessionEngine_h__

// System includes
#include <e32base.h>
#include <f32file.h>

// User includes
#include "TrkTcbCmdCodes.h"

// Classes referenced


//
// CTrkTcbSrvSessionEngine (header)
//
//
// The underlying engine used by each session. Manipulates the server-side objects.
//
class CTrkTcbSrvSessionEngine : public CBase
{
	//Static construct and destruct
	public:
		static 	CTrkTcbSrvSessionEngine*	NewL();
				~CTrkTcbSrvSessionEngine();
	
	//private construct
	private:
				CTrkTcbSrvSessionEngine();
		void	ConstructL();

	//methods implementing the server cmds functionality
	public:	
		void	OpenFileL(const TDesC& aFilePath, TUint aMode, TTime& aModifiedTime);
		void	ReadFileL(TDes8& aFileData);
		void	WriteFileL(const TDesC8& aFileData);
		void	CloseFileL(const TTime& aModifiedTime);
		void    PositionFileL(TSeek aSeek, TInt& aOffset);

	//date members
	private:
		enum TFileState
		{
			EFileOpened = 0,
			EFileReading = 1,
			EFileWriting = 2,
			EFileClosed = 3,
			EFileUnknown = -1		
		};
		RFile iFile;
		RFs iFs;
		TFileState iFileState;
		TUint iMode;

};

#endif
