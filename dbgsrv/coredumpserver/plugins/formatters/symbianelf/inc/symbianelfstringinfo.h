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
// Implements handling of the string info segment in Elf core dump
// Definition of the string info table.
//



/**
 @file
 @internalTechnology
 @released
*/

#ifndef __SYMBIAN_STRING_INFO_H__
#define __SYMBIAN_STRING_INFO_H__

_LIT8(KEmpty, "");
_LIT8(KSymbian, "CORE.SYMBIAN");
_LIT8(KThread, "CORE.SYMBIAN.THREAD");
_LIT8(KProcess, "CORE.SYMBIAN.PROCESS");
_LIT8(KExecutable, "CORE.SYMBIAN.EXECUTABLE");
_LIT8(KStr, "CORE.SYMBIAN.STR");
_LIT8(KSymbianElfCoreDumpVersion, "1.0.0");
_LIT8(KSymbianTraceVersion, "1.0.0");

/**
@internalTechnology
@released

String info table is used by symbian elf formatter to manage the string segment data.
*/
class CStringInfoTable : public CBase
{
public:
	static CStringInfoTable *NewL();
	static CStringInfoTable *NewLC();
	~CStringInfoTable();

public:
	TUint GetIndex(const TDesC8 &aItem);
	TUint AddStringL(const TDesC8 &aItem);
	TText8* GetAllStrings();
	TUint GetSize();

private:
	void ConstructL();
	CStringInfoTable();
    /** internal buffer holding the content of the string table */
	RBuf8 iBuffer;
};

/**
Called when willing to locate the index of the specified text in the internal buffer
@param aItem descriptor holding the text to locate
@return index of the located text in the internall buffer
*/
inline TUint CStringInfoTable::GetIndex(const TDesC8 &aItem)
{
	return iBuffer.Find(aItem);
}

/**
Called when required to get the raw content of the string table
@return pointer to the content of the internal buffer
*/
inline TText8* CStringInfoTable::GetAllStrings()
{
	return (TText8*)iBuffer.Ptr();
}

/**
Called when required to get the size of the string table
@return size of the internal buffer
*/
inline TUint CStringInfoTable::GetSize()
{
	return iBuffer.Length();
}

#endif // __SYMBIAN_STRING_INFO_H__
