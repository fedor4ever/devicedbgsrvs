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

#include <e32base.h>
#include <e32debug.h>

#include "symbianelfstringinfo.h"
#include "symbianelfstringinfov2.h"

/**
1st stage construction
@return pointer to the newly created object, caller takes ownership of the object.
*/
CStringInfoTableV2 *CStringInfoTableV2::NewLC()
{
	CStringInfoTableV2 *self = new(ELeave) CStringInfoTableV2();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

/**
1st stage construction
@return pointer to the newly created object, caller takes ownership of the object.
*/
CStringInfoTableV2 *CStringInfoTableV2::NewL()
{
	CStringInfoTableV2 *self = CStringInfoTableV2::NewLC();
	CleanupStack::Pop(self);
	return self;
}

/**
1st stage construction
@return pointer to new object
@param aStrings Buffer containing existing set of NULL terminated strings
@leave One of the system wide codes
*/
CStringInfoTableV2* CStringInfoTableV2::NewL(const TDesC8& aStrings)
{
	CStringInfoTableV2 *self = CStringInfoTableV2::NewLC(aStrings);
	CleanupStack::Pop(self);
	return self;
}

/**
1st stage construction
@return pointer to new object
@param aStrings Buffer containing existing set of NULL terminated strings
@leave One of the system wide codes
*/
CStringInfoTableV2* CStringInfoTableV2::NewLC(const TDesC8& aStrings)
{
	CStringInfoTableV2 *self = new(ELeave) CStringInfoTableV2();
	CleanupStack::PushL(self);
	self->ConstructL(aStrings);
	return self;
}

/**
dtor closes string buffer
*/
CStringInfoTableV2::~CStringInfoTableV2()
{
	iBuffer.Close();
}

/**
ctor nothing really
*/
CStringInfoTableV2::CStringInfoTableV2()
{
}

/**
2nd stage construction.
Creates string buffer and populates it with strings present in every ELF dump file.
@leave one of the system wide error codes
*/
void CStringInfoTableV2::ConstructL()
    {
    //LOG_MSG("->CStringInfoTable::ConstructL()\n");
	iBuffer.CreateL(KEmpty, KEmpty().Length()+1);
	iBuffer.AppendFill((TChar)0, 1);
	iBuffer.ReAllocL(iBuffer.Length()+KSymbian().Length()+1);
	iBuffer.Append(KSymbian);
	iBuffer.AppendFill((TChar)0, 1);
	iBuffer.ReAllocL(iBuffer.Length()+KThread().Length()+1);
	iBuffer.Append(KThread);
	iBuffer.AppendFill((TChar)0, 1);
	iBuffer.ReAllocL(iBuffer.Length()+KProcess().Length()+1);
	iBuffer.Append(KProcess);
	iBuffer.AppendFill((TChar)0, 1);
	iBuffer.ReAllocL(iBuffer.Length()+KExecutable().Length()+1);
	iBuffer.Append(KExecutable);
	iBuffer.AppendFill((TChar)0, 1);
	iBuffer.ReAllocL(iBuffer.Length()+KStr().Length()+1);
	iBuffer.Append(KStr);
	iBuffer.AppendFill((TChar)0, 1);
	iBuffer.ReAllocL(iBuffer.Length()+KSymbianElfCoreDumpVersion().Length()+1);
	iBuffer.Append(KSymbianElfCoreDumpVersion);
	iBuffer.AppendFill((TChar)0, 1);
	}

/**
2nd stage construction.
Creates string buffer and populates it with param
@param aStrings initial buffer
@leave one of the system wide error codes
*/
void CStringInfoTableV2::ConstructL(const TDesC8& aStrings)
	{
	iBuffer.CreateL(aStrings);
	}

/**

@param aItem: The string to be added to internal buffer
@return offset at which the aItem was added to internal buffer
@leave one of the system wide error codes
*/
TUint CStringInfoTableV2::AddStringL(const TDesC8 &aItem)
    {
    //LOG_MSG("->CStringInfoTable::AddStringL()\n");
	TUint index = iBuffer.Length();
	iBuffer.ReAllocL(iBuffer.Length() + aItem.Length() + 1);
	iBuffer.Append(aItem);
	iBuffer.AppendFill((TChar)0, 1);
	return index;
    }
