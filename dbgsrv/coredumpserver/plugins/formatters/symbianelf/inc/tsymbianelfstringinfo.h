// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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



/**
 @file
 @internalTechnology
 @released
*/

#ifndef __T_SYMBIAN_STRING_INFO_H__
#define __T_SYMBIAN_STRING_INFO_H__

#include <e32test.h>

class CSymbianStringInfoTest : public CBase
{
public:
	static CSymbianStringInfoTest* NewL(const TDesC& aTitle);
	~CSymbianStringInfoTest();
	void TestSymbianStringInfoL();
	
private:
	CSymbianStringInfoTest(const TDesC& aTitle);
	RTest iTest;
};

#endif // __T_SYMBIAN_STRING_INFO_H__
