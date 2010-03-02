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
 @file filewriterteststeps.h
 @file
 @internalTechnology
 @released
*/

#ifndef __FILE_WRITER_TEST_STEPS_H__
#define __FILE_WRITER_TEST_STEPS_H__
#include <TestExecuteStepBase.h>
#include "filewritertestserver.h"

// __EDIT_ME__ - Create your own test step definitions
class CFileWriterTestStep1 : public CTestStep
	{
public:
	CFileWriterTestStep1();
	~CFileWriterTestStep1();
	virtual TVerdict doTestStepPreambleL();
	virtual TVerdict doTestStepPostambleL();
	virtual TVerdict doTestStepL();
private:
	};

class CFileWriterTestStep2 : public CTestStep
	{
public:
	CFileWriterTestStep2();
	~CFileWriterTestStep2();
	virtual TVerdict doTestStepPreambleL();
	virtual TVerdict doTestStepPostambleL();
	virtual TVerdict doTestStepL();
private:

	};


class CFileWriterTestStep3 : public CTestStep
	{
public:
	CFileWriterTestStep3( );
	~CFileWriterTestStep3();
	virtual TVerdict doTestStepPreambleL();
	virtual TVerdict doTestStepPostambleL();
	virtual TVerdict doTestStepL();
private:

	};


class CFileWriterTestStep4 : public CTestStep
	{
public:
	CFileWriterTestStep4();
	~CFileWriterTestStep4();
	virtual TVerdict doTestStepPreambleL();
	virtual TVerdict doTestStepPostambleL();
	virtual TVerdict doTestStepL();
private:

	};


class CFileWriterTestStep5 : public CTestStep
	{
public:
	CFileWriterTestStep5();
	~CFileWriterTestStep5();
	virtual TVerdict doTestStepPreambleL();
	virtual TVerdict doTestStepPostambleL();
	virtual TVerdict doTestStepL();
private:

	};


_LIT(KFileWriterTestStep1,"FileWriterTestStep1");
_LIT(KFileWriterTestStep2,"FileWriterTestStep2");
_LIT(KFileWriterTestStep3,"FileWriterTestStep3");
_LIT(KFileWriterTestStep4,"FileWriterTestStep4");
_LIT(KFileWriterTestStep5,"FileWriterTestStep5");


#endif // __FILE_WRITER_TEST_STEPS_H__
