// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Implements file writer plugin
// Definition of the file writer plugin.
//

/**
 @file
 @internalTechnology
 @released
*/

#ifndef __FILE_WRITER_H__
#define __FILE_WRITER_H__

#include <e32hashtab.h>
#include <crashdatasave.h>

const TUint32 KFileWriterUid = { 0x102831e4 };

/**
@internalTechnology
@released

File writer plugin is loaded by the core dump server and used by formatter plugin to write dump data to a file.
*/
class CFileWriter : public CCrashDataSave
{
public:
	static CFileWriter* NewL();
	static CFileWriter* NewLC();
	~CFileWriter();

public:

	virtual void GetDescription( TDes & aPluginDescription );
	virtual TInt GetNumberConfigParametersL( );
	virtual COptionConfig * GetConfigParameterL( const TInt aIndex );
	virtual void SetConfigParameterL( const TInt aIndex, const TInt32 & aValue, const TDesC & aDescValue );
	virtual const TDesC& GetMediaName();

	// Called by formatter to start the creation of a dump. Can be called several times to create several dumps
	void OpenL( const TDesC& aFileName );
	TInt Open( const TDesC& aFileName );

	// Called by formatter to end the creation of a dump. Open() and Close() must be paired calls
	void CloseL();
	TInt Close();

	// Save aData - binary
	void WriteL( const TDesC8& aData );
	TInt Write( const TDesC8& aData );

	// Save aData - raw
	void WriteL( TAny* aData, TUint aSize );
	TInt Write( TAny* aData, TUint aSize );

    // Save aData at the specific position - binary - need implementation
	void WriteL( TInt aPos, const TDesC8& aData );
	TInt Write( TInt aPos, const TDesC8& aData );

    // Save aData at the specific position - raw
	void WriteL( TInt aPos, TAny* aData, TUint aSize );
	TInt Write( TInt aPos, TAny* aData, TUint aSize );

private:
	void ConstructL();
	CFileWriter();

private:
	HBufC* iDescription;			/** plugin description string */
	RFs	iFs;						/** file server session handle */
	RFile iFile;					/** dump file handle */
	TBuf<KMaxFileName> iFileName;	/** filename to be used */
};

#endif // __FILE_WRITER_H__
