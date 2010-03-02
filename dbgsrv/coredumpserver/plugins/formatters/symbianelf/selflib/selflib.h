// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
//	Classes that allow the editing and creation of Symbian ELF files
//

/**
 * @file
 * @publishedAll
 * @prototype
 */

#ifndef __SELF_LIB_H__
#define __SELF_LIB_H__

#include <e32base.h>
#include <e32debug.h>
#include <f32file.h>
#include <elfdefs.h>
#include <badesca.h>
#include <symbianelfdefs.h>

_LIT(KTempFileLocation, "C:\\"); //TODO: Investigate system drive

/** Reads a struct from a file
 *	@param t Type to read
 *	@param s the structure
 *	@param f RFile to read from
 *	@param r Point in the file from which to read
 * */
#define READ_STRUCTL(t, s, f, r)\
	t s;\
	{\
	TPtr8 ptr((TUint8*)&s, sizeof(s), sizeof(s));\
	User::LeaveIfError(f.Read(r, ptr));\
	}

/**
 * This represents a Symbian Note segment in an ELF file
 * It consits of a program header, descriptor header and
 * the Note specific segment.
 */
struct TSELFSegment
	{
	Elf32_Phdr iPhdr;
	Sym32_dhdr iDhdr;

	union TSegmentType
		{
		Sym32_variant_spec_data iVarData;
		//Other section types can live here - currently only support variant specific data
		};

	TSegmentType iType;
	};

class CStringInfoTableV2;

/**
 * This class is the interface to allow editing of SELF files
 */
class CSELFEditor : public CBase
	{
	public:
		IMPORT_C static CSELFEditor* NewL(const TDesC& aFilename);
		IMPORT_C static CSELFEditor* NewLC(const TDesC& aFilename);

		IMPORT_C void GetELFHeader(Elf32_Ehdr& aELFHeader);
		IMPORT_C void InsertVariantSpecificDataL(const TDesC8& aVarData);
		IMPORT_C void WriteELFUpdatesL();

		virtual ~CSELFEditor();

	private:
		CSELFEditor();
		void ConstructL(const TDesC& aFilename);
		TBool IsValidELFFile();
		TBool IsELFFileSupported();

		TUint CopyFromELFFileL(TUint aStartPoint, TUint aSize, RFile aDestinationFile, TUint aDestWrite = 0);
		TUint CopyRegistersL(RFile aFile, TUint aSrcRead, TUint aDestWrite, TUint& aSizeWritten);
		TUint CopyVariantDataL(RFile aFile, TUint aSrcRead, TUint aDestWrite, TUint& aSizeWritten);
		void PutEmptySpaceAtEndOfFileL(RFile aFile, TUint aSize);

		void CacheStringTableL();
		void CacheProgramHeadersL();
		void CacheELFHeaderL();

		TUint WriteStringInfoL(RFile aFile, TUint& aOffset);
		void SwapFilesL(const TDesC& aSrc, const TDesC& aDest);

	private:
		RFs iFsSession;
		RFile iELFFile;
		RArray<TSELFSegment> iNewSegments;
		RArray<Elf32_Phdr> iProgramHeaders;

		Elf32_Ehdr iELFHeader;
		Sym32_dhdr iStringTableDhdr;
		CStringInfoTableV2* iStringTable;

		//Flat array of descriptors for data to be inserted to ELF
		CDesC8Array* iRawData;

	};

#endif
