/**
* Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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




/**
 @file filewritertestserver.h
 @file
 @internalTechnology
 @released
*/

// __EDIT_ME__ Create your own class definition based on this
#ifndef __FILE_WRITER_TEST_SERVER_H__
#define __FILE_WRITER_TEST_SERVER_H__

#include <TestExecuteServerBase.h>

class CFileWriterTestServer  : public CTestServer
	{
public:
	static CFileWriterTestServer* NewL();
	virtual CTestStep* CreateTestStep(const TDesC& aStepName);
	RFs& Fs(){return iFs;};

private:
	RFs iFs;
	};

#endif // __FILE_WRITER_TEST_SERVER_H__

