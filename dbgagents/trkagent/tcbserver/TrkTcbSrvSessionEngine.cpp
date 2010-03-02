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

#include "TrkTcbSrvSessionEngine.h"

// System includes
#include <e32debug.h>

// User includes

// Type definitions

// Constants

// Enumerations

// Classes referenced


//
// CTrkTcbSrvSessionEngine (source)
//

//
// CTrkTcbSrvScheduler::CTrkTcbSrvSessionEngine()
//
// Constructor
//
CTrkTcbSrvSessionEngine::CTrkTcbSrvSessionEngine() : iFileState(EFileUnknown)
{
}

//
// CTrkTcbSrvScheduler::~CTrkTcbSrvSessionEngine()
//
// Destructor
//
CTrkTcbSrvSessionEngine::~CTrkTcbSrvSessionEngine()
{
}

//
// CTrkTcbSrvScheduler::ConstructL()
//
// Second level constructor
//
void CTrkTcbSrvSessionEngine::ConstructL()
{
	//nothing right now
}

//
// CTrkTcbSrvScheduler::NewL()
//
// Creates an instance of CTrkTcbSrvSessionEngine
//
CTrkTcbSrvSessionEngine* CTrkTcbSrvSessionEngine::NewL()
{
	CTrkTcbSrvSessionEngine* self = new(ELeave) CTrkTcbSrvSessionEngine();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}

//
// CTrkTcbSrvScheduler::OpenFileL()
//
// Opens the specified file
//
void CTrkTcbSrvSessionEngine::OpenFileL(const TDesC& aFilePath, TUint aMode, TTime& aModifiedTime)
{	
	//just for testing
	TBuf8<KMaxPath> fileName;
	fileName.Copy(aFilePath);

	iMode = aMode; //copy  the mode, for properly closing the file.
	
	User::LeaveIfError(iFs.Connect());

	TInt error = iFs.MkDirAll(aFilePath);

	if ((KErrNone != error) && (KErrAlreadyExists != error))
	{
		iFs.Close();
		User::Leave(error);
	}
	
	error = iFile.Open(iFs, aFilePath, aMode);
	if (KErrNone != error)
		User::LeaveIfError(iFile.Replace(iFs, aFilePath, aMode));

	// get the modification date
	User::LeaveIfError(iFile.Modified(aModifiedTime));
	iFileState = EFileOpened;
}

//
// CTrkTcbSrvScheduler::ReadFileL()
//
// Reads from the already opened file
//
void CTrkTcbSrvSessionEngine::ReadFileL(TDes8& aFileData)
{
	iFileState = EFileReading;
	User::LeaveIfError(iFile.Read(aFileData));
}

//
// CTrkTcbSrvScheduler::WriteFileL()
//
// Writes to an already opened file
//
void CTrkTcbSrvSessionEngine::WriteFileL(const TDesC8& aFileData)
{
	if (iFileState == EFileOpened)
	{
		iFile.SetSize(0);
		iFileState = EFileWriting;
	}
	User::LeaveIfError(iFile.Write(aFileData));	
	User::LeaveIfError(iFile.Flush());
}

//
// CTrkTcbSrvScheduler::CloseFileL()
//
// Closes an already opened file
//
void CTrkTcbSrvSessionEngine::CloseFileL(const TTime& aModifiedTime)
{
	//set the modification time only if the file is opened in write mode
	if (iMode & EFileWrite)
	{
		User::LeaveIfError(iFile.SetModified(aModifiedTime));
		User::LeaveIfError(iFile.Flush());
	}
	
	iFile.Close();
	iFs.Close();
	iFileState = EFileClosed;
}

//
// CTrkTcbSrvScheduler::PositionFileL()
//
// Changes the positions in an already opened file
//
void CTrkTcbSrvSessionEngine::PositionFileL(TSeek aSeek, TInt& aOffset)
{
	User::LeaveIfError(iFile.Seek(aSeek, aOffset));
}


