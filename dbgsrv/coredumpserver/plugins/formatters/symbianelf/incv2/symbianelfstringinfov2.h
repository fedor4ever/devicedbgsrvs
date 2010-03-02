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

#ifndef __SYMBIAN_STRING_INFOV2_H__
#define __SYMBIAN_STRING_INFOV2_H__

_LIT8(KSymbianElfCoreDumpVersionV2, "2.0.0");

/**
@internalTechnology
@released

String info table is used by symbian elf formatter to manage the string segment data.
*/
class CStringInfoTableV2 : public CBase
{
public:
	static CStringInfoTableV2* NewL();
	static CStringInfoTableV2* NewLC();

	static CStringInfoTableV2* NewL(const TDesC8& aStrings);
	static CStringInfoTableV2* NewLC(const TDesC8& aStrings);

	~CStringInfoTableV2();

public:
	TUint GetIndex(const TDesC8 &aItem);
	TUint AddStringL(const TDesC8 &aItem);
	TText8* GetAllStrings();
	TUint GetSize();

private:
	void ConstructL();
	void ConstructL(const TDesC8& aStrings);

	CStringInfoTableV2();
    /** internal buffer holding the content of the string table */
	RBuf8 iBuffer;
};

/**
Called when willing to locate the index of the specified text in the internal buffer
@param aItem descriptor holding the text to locate
@return index of the located text in the internall buffer
*/
inline TUint CStringInfoTableV2::GetIndex(const TDesC8 &aItem)
{
	return iBuffer.Find(aItem);
}

/**
Called when required to get the raw content of the string table
@return pointer to the content of the internal buffer
*/
inline TText8* CStringInfoTableV2::GetAllStrings()
{
	return (TText8*)iBuffer.Ptr();
}

/**
Called when required to get the size of the string table
@return size of the internal buffer
*/
inline TUint CStringInfoTableV2::GetSize()
{
	return iBuffer.Length();
}

#endif // __SYMBIAN_STRING_INFOV2_H__
