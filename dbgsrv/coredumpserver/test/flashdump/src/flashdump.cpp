// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#include "flashdump.h"
#include "debuglogging.h"

/**
 * 1st stage construction
 * @return pointer to the newly created object, caller takes ownership of the object.
 */
CCrashFlashDump* CCrashFlashDump::NewL()
	{
	LOG_MSG("->CFlashDump::NewL()->\n");
	CCrashFlashDump* self = CCrashFlashDump::NewLC();
	CleanupStack::Pop();
	return self;
	}

/**
 * 1st stage construction
 * @return pointer to the newly created object, caller takes ownership of the object. 
 */
CCrashFlashDump* CCrashFlashDump::NewLC()
	{
	LOG_MSG("->CFlashDump::NewLC()->\n");
	CCrashFlashDump* self = new(ELeave)CCrashFlashDump();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

/**
 * Constructor
 */
CCrashFlashDump::CCrashFlashDump()
	{
	}

/**
 * 2nd stage construction
 */
void CCrashFlashDump::ConstructL()
	{
	LOG_MSG("CCrashFlashDump::ConstructL");
	
	User::LeaveIfError(iFs.Connect());
	User::LeaveIfError(iSecSess.Connect(KSecurityServerVersion));
	}

/**
 * Opens a file to dump the flash to
 * @param aFileName name of file to open for the flash dump
 * @leave one of the OS wide codes
 */
void CCrashFlashDump::OpenFileL(const TDesC& aFileName)
	{
	LOG_MSG2("CCrashFlashDump::OpenFileL --> [%S]", &aFileName);
	
	iFile.Close();	
	User::LeaveIfError(iFile.Create(iFs, aFileName, EFileWrite | EFileRead));
	}

/**
 * This goes through the crash flash partition and dumps it to the file
 * @leave one of the os wide codes
 */
void CCrashFlashDump::DumpFlashToFileL()
	{
	LOG_MSG("CCrashFlashDump::DumpFlashToFile");
	
	//We know on a H4 the flash 
	TInt remaining = KCrashFlashPartitionSize;
	
	//We shall dump 10k at a time
	RBuf8 tmpBuf;
	TUint bufSize = 0x2800;
	TUint sizeDumped = 0; 
	
	User::LeaveIfError(tmpBuf.Create(bufSize));
	tmpBuf.CleanupClosePushL();
	
	while(remaining > 0)
		{
		LOG_MSG3("Reading [%d] bytes from crash flash position [0x%X]", bufSize, sizeDumped);
		User::LeaveIfError(iSecSess.ReadCrashLog(sizeDumped, tmpBuf, bufSize));
		User::LeaveIfError(iFile.Write(tmpBuf));
		
		sizeDumped += bufSize;
		remaining -= bufSize;
		
		bufSize = (bufSize > remaining) ? remaining : bufSize;
		}						
	
	CleanupStack::PopAndDestroy();
	}

/**
 * destructor
 */
CCrashFlashDump::~CCrashFlashDump()
	{
	iSecSess.Close();
	iFile.Close();
	}

//eof
