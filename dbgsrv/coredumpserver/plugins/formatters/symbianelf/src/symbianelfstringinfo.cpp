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
#include "symbianelfstringinfo.h"

#include <e32debug.h>

/**
1st stage construction
@return pointer to the newly created object, caller takes ownership of the object.
*/
CStringInfoTable *CStringInfoTable::NewLC()
{
	CStringInfoTable *self = new(ELeave) CStringInfoTable();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

/**
1st stage construction
@return pointer to the newly created object, caller takes ownership of the object.
*/
CStringInfoTable *CStringInfoTable::NewL()
{
	CStringInfoTable *self = CStringInfoTable::NewLC();
	CleanupStack::Pop(self);
	return self;
}
/**
dtor closes string buffer
*/
CStringInfoTable::~CStringInfoTable()
{
	iBuffer.Close();
}

/**
ctor nothing really
*/
CStringInfoTable::CStringInfoTable()
{
}

/**
2nd stage construction.
Creates string buffer and populates it with strings present in every ELF dump file.
@leave one of the system wide error codes
*/
void CStringInfoTable::ConstructL()
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

@param aItem: The string to be added to internal buffer
@return offset at which the aItem was added to internal buffer
@leave one of the system wide error codes
*/
TUint CStringInfoTable::AddStringL(const TDesC8 &aItem)
    {
    //LOG_MSG("->CStringInfoTable::AddStringL()\n");
	TUint index = iBuffer.Length();
	iBuffer.ReAllocL(iBuffer.Length() + aItem.Length() + 1);
	iBuffer.Append(aItem);
	iBuffer.AppendFill((TChar)0, 1);
	return index;
    }

