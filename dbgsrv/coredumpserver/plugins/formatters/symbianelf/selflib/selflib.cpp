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
 * @file selflib.cpp
 * @prototype
 */

#include <f32file.h>

#include "selflib.h"
#include "debuglogging.h"
#include "symbianelfstringinfov2.h"
#include "symbianelfstringinfo.h"

/**
 * First stage construction
 * @return Pointer to new CSELFEditor object
 * @leave One of the System Wide Error Codes
 */
EXPORT_C CSELFEditor* CSELFEditor::NewL(const TDesC& aFilename)
	{
	LOG_ENTRY();

	CSELFEditor* self = CSELFEditor::NewLC(aFilename);
	CleanupStack::Pop();

	return self;
	}

/**
 * First stage construction. Object is left upon cleanup stack
 * @return Pointer to new CSELFEditor object
 * @leave One of the System Wide Error Codes
 */
EXPORT_C CSELFEditor* CSELFEditor::NewLC(const TDesC& aFilename)
	{
	LOG_ENTRY();

	CSELFEditor* self = new(ELeave)CSELFEditor();
	CleanupStack::PushL(self);
	self->ConstructL(aFilename);

	return self;
	}

/**
 * CSELFEditor Desctructor
 */
CSELFEditor::~CSELFEditor()
	{
	LOG_ENTRY();

	delete iRawData;
	delete iStringTable;

	iELFFile.Close();
	iFsSession.Close();
	iNewSegments.Close();
	iProgramHeaders.Close();
	}

/**
 * Second stage safe construction
 * @leave One of the System Wide Error Codes
 */
void CSELFEditor::ConstructL(const TDesC& aFilename)
	{
	LOG_ENTRY();

	User::LeaveIfError(iFsSession.Connect());
	User::LeaveIfError(iELFFile.Open(iFsSession, aFilename, EFileWrite));

	TInt fileSize;
	User::LeaveIfError(iELFFile.Size(fileSize));

	//Check its big enough to at least read the header
	if(fileSize < sizeof(Elf32_Ehdr))
		{
		LOG_MSG("\tCSELFEditor::ConstructL --> File is not big enough to be an ELF file");
		User::Leave(KErrCorrupt);
		}

	CacheELFHeaderL();

	//Validate the ELF file
	if(!IsValidELFFile())
		User::Leave(KErrCorrupt);

	if(!IsELFFileSupported())
		User::Leave(KErrNotSupported);

	iRawData = new(ELeave) CDesC8ArrayFlat(1);

	CacheProgramHeadersL();
	CacheStringTableL();
	}

/**
 * Caches ELF File internally in this object
 * @see GetELFHeader
 * @leave One of the system wide error codes
 */
void CSELFEditor::CacheELFHeaderL()
	{
	//Read the ELF header
	TUint fileLocation = 0;
	TPtr8 hdrDes((TUint8*)&iELFHeader, sizeof(Elf32_Ehdr));
	User::LeaveIfError(iELFFile.Read(fileLocation, hdrDes));
	}

/**
 * CSELFEditor Constructor
 */
CSELFEditor::CSELFEditor()
	{}

/**
 * This ELF library currently only supports a specific subset of the ELF standard.
 * This function makes sure the ELF file supplied is part of that subset. These are the things
 * we do not currently support:
 * 1. 64 Bit ELF
 * 2. ELFDATA2MSB File class (Big Endian)
 * 3. Any sections
 * 4. Any type other than ET_CORE
 *
 * @return TBool indicating if the file is OK to continue with
 */
TBool CSELFEditor::IsELFFileSupported()
	{
	LOG_ENTRY();

	TBool supported = ETrue;
	if(iELFHeader.e_ident[EI_CLASS] != ELFCLASS32)
		{
		LOG_MSG("\tCSELFEditor::IsElfFileSupported() --> Non 32 bit files are not supported");
		supported = EFalse;
		}

	if(iELFHeader.e_ident[EI_DATA] != ELFDATA2LSB)
		{
		LOG_MSG("\tCSELFEditor::IsElfFileSupported() --> Only support Little Endian");
		supported = EFalse;
		}

	if(iELFHeader.e_type != ET_CORE)
		{
		LOG_MSG("\tCSELFEditor::IsElfFileSupported() --> Only support Core Files");
		supported = EFalse;
		}

	if(iELFHeader.e_shoff != 0 || iELFHeader.e_shnum != 0)
		{
		LOG_MSG("\tCSELFEditor::IsElfFileSupported() --> Do not support Sections");
		supported = EFalse;
		}

	return supported;
	}

/**
 * Validates the ELF file by looking at the signature
 * @return Boolean indicating whether or not the file is valid ELF
 */
TBool CSELFEditor::IsValidELFFile()
	{
	LOG_ENTRY();

	//Check signature
	return iELFHeader.e_ident[EI_MAG0] == ELFMAG0 &&
		iELFHeader.e_ident[EI_MAG1] == ELFMAG1 &&
		iELFHeader.e_ident[EI_MAG2] == ELFMAG2 &&
		iELFHeader.e_ident[EI_MAG3] == ELFMAG3;
	}

/**
 * Provides the ELF header of this file
 * @param aELFHeader Holds the ELF header after the call
 */
EXPORT_C void CSELFEditor::GetELFHeader(Elf32_Ehdr& aELFHeader)
	{
	LOG_ENTRY();
	aELFHeader = iELFHeader;
	}

/**
 * Caches the program headers from the file into this object
 * @leave One of the system wide codes
 */
void CSELFEditor::CacheProgramHeadersL()
	{
	LOG_ENTRY();

	Elf32_Phdr progHeader;
	TPtr8 phdrPtr((TUint8*)&progHeader, sizeof(Elf32_Phdr));

	TUint nextPh = iELFHeader.e_phoff;
	for(TUint i = 0; i < iELFHeader.e_phnum; i++)
		{
		User::LeaveIfError(iELFFile.Read(nextPh, phdrPtr));
		iProgramHeaders.AppendL(progHeader);
		nextPh += sizeof(Elf32_Phdr);
		}
	}

/**
 * Inserts variant specific data into the ELF file
 * @param aVarData The binary data to be inserted into the ELF file
 * @leave One of the system wide codes
 */
EXPORT_C void CSELFEditor::InsertVariantSpecificDataL(const TDesC8& aVarData)
	{
	LOG_ENTRY();

	//Store the data in the raw data array, and record its position to associate with the new section
	iRawData->AppendL(aVarData);
	TUint position = iRawData->Count() - 1;

	//Now create the variant sepcific data dhdr and structure
	TSELFSegment newSegment;

	newSegment.iPhdr.p_type = PT_NOTE;
	newSegment.iPhdr.p_offset = 0;    //Fill in when we write the file
	newSegment.iPhdr.p_vaddr = 0;
	newSegment.iPhdr.p_paddr = 0;
	newSegment.iPhdr.p_filesz =0;
	newSegment.iPhdr.p_memsz = 0;
	newSegment.iPhdr.p_flags = 0;
	newSegment.iPhdr.p_align = sizeof(TUint);

	newSegment.iDhdr.d_name = 0;		//Fill in when we write the file (we need the string table)
	newSegment.iDhdr.d_type = ESYM_NOTE_VARIANT_DATA;
	newSegment.iDhdr.d_version = 0;	//Fill in when we write the file (we need the string table)
	newSegment.iDhdr.d_elemnum = 1;
	newSegment.iDhdr.d_descrsz = sizeof(Sym32_variant_spec_data);

	Sym32_variant_spec_data* variantData = (Sym32_variant_spec_data*)(&newSegment.iType);
	variantData->es_data = position;			//This data is in the iRawData until the write
	variantData->es_size = aVarData.Size();

	iNewSegments.AppendL(newSegment);
	++iELFHeader.e_phnum;
	}

/**
 * Commits any changes made in the CSELFEditor to the ELF file
 * @leave One of the system wide codes
 */
EXPORT_C void CSELFEditor::WriteELFUpdatesL()
	{
	LOG_ENTRY();

	//There may be nothing to do
	if(iNewSegments.Count() == 0)
		return;

	//Create tmp file
	TBuf<KMaxFileName> tmpFilename;
	RFile tmpFile;
	User::LeaveIfError(tmpFile.Temp(iFsSession, KTempFileLocation, tmpFilename, EFileWrite));
	CleanupClosePushL(tmpFile);

	//Log file name
	TPtr8 smallName((TUint8*)tmpFilename.Ptr(), 2*tmpFilename.Length(), 2*tmpFilename.Length());
	LOG_MSG2("\tWriteELFUpdatesL() --> Writing new temporary updates to [%S]", &smallName);

	TUint numberofPhdrs = 0;

	//Leave space for elf header plus program header table at start
	TUint spaceToLeave =+ sizeof(Elf32_Ehdr) + (iNewSegments.Count() + iProgramHeaders.Count() ) * sizeof(Elf32_Phdr);
	PutEmptySpaceAtEndOfFileL(tmpFile, spaceToLeave);

	TUint phdrLoc = sizeof(Elf32_Ehdr); //phdr table starts just after ELF header
	TUint dataLoc = spaceToLeave;

	LOG_MSG("WriteELFUpdatesL --> Copying existing program headers to the tmp file");

	//Store new program headers as we go
	RArray<Elf32_Phdr> newProgramHeaders;

	{
	Elf32_Phdr phdr;
	TPtr8 phdrPtr((TUint8*)&phdr, sizeof(Elf32_Phdr), sizeof(Elf32_Phdr));

	for(TUint i = 0; i < iProgramHeaders.Count(); i++)
		{
		phdr = iProgramHeaders[i];

		//Now we put the neccesary data after the phdr table
		if(phdr.p_type == PT_LOAD && phdr.p_filesz != 0 && phdr.p_offset != 0)
			{
			phdr.p_offset = CopyFromELFFileL(phdr.p_offset, phdr.p_filesz, tmpFile, dataLoc);
			dataLoc += phdr.p_filesz;
			}
		else if(phdr.p_type == PT_NOTE)
			{
			//Read the symbian descriptor
			READ_STRUCTL(Sym32_dhdr, symd, iELFFile, phdr.p_offset);

			//TODO: The fact that we need this *MAY* indicate flaws in SELF format. It would be nice
			//to be able to copy a bit that was completely described by the dhdr
			if(symd.d_type == ESYM_NOTE_REG)
				{
				//registers cant just be blindly copied as they contain offsets that need updating
				TUint sizeWritten = 0;
				phdr.p_offset = CopyRegistersL(tmpFile, phdr.p_offset, dataLoc, sizeWritten);
				dataLoc += sizeWritten;
				}
			else if(symd.d_type == ESYM_NOTE_VARIANT_DATA)
				{
				//Variant data cant be blindly copied either as they have offsets too
				TUint sizeWritten = 0;
				phdr.p_offset = CopyVariantDataL(tmpFile, phdr.p_offset, dataLoc, sizeWritten);
				dataLoc += sizeWritten;
				}
			//special case - needs updating so do after everything else
			else if(symd.d_type == ESYM_NOTE_STR)
				{
				continue;
				}
			else
				{
				TUint sizetoCopy = sizeof(Sym32_dhdr) + symd.d_descrsz * symd.d_elemnum;
				phdr.p_offset = CopyFromELFFileL(phdr.p_offset, sizetoCopy, tmpFile, dataLoc);
				dataLoc += sizetoCopy;
				}

			}

		//Write the updated program header
 		User::LeaveIfError(tmpFile.Write(phdrLoc, phdrPtr));
		phdrLoc += sizeof(Elf32_Phdr);
		newProgramHeaders.AppendL(phdr);
		++numberofPhdrs;
		}
	LOG_MSG2("WriteELFUpdatesL --> Copied [%d] existing program headers (minus string table)", numberofPhdrs);
	}

	//Now we add the new segments
	
	//This is for the string table
	TBuf8<KMaxFileName> varDataBuf;
	varDataBuf.Format(_L8("CORE.SYMBIAN.VARDATA"));
	TUint versionIndex = iStringTable->GetIndex(KSymbianElfCoreDumpVersionV2);

	for(TUint i = 0; i < iNewSegments.Count(); i++)
		{
		TSELFSegment segment = iNewSegments[i];

		//Update the program header and the d_hdr
		segment.iPhdr.p_offset = dataLoc;


		TInt index = iStringTable->AddStringL(varDataBuf);
		segment.iDhdr.d_name = (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
		segment.iDhdr.d_version = versionIndex;

		Sym32_variant_spec_data* variantData = (Sym32_variant_spec_data*)(&segment.iType);
		TUint dataIndex = variantData->es_data;
		variantData->es_data = dataLoc + sizeof(Sym32_variant_spec_data) + sizeof(Sym32_dhdr);

		//Write the phdr
		TPtr8 nphdrPtr((TUint8*)&segment.iPhdr, sizeof(Elf32_Phdr), sizeof(Elf32_Phdr));
		User::LeaveIfError(tmpFile.Write(phdrLoc, nphdrPtr));
		newProgramHeaders.AppendL(segment.iPhdr);
		phdrLoc += sizeof(Elf32_Phdr);
		++numberofPhdrs;

		//Write the dhdr
		TPtr8 ndhdrPtr((TUint8*)&segment.iDhdr, sizeof(Sym32_dhdr), sizeof(Sym32_dhdr));
		User::LeaveIfError(tmpFile.Write(dataLoc, ndhdrPtr));
		dataLoc += sizeof(Sym32_dhdr);

		//Write the var struct
		TPtr8 nvarHdr((TUint8*)&segment.iType, sizeof(Sym32_variant_spec_data), sizeof(Sym32_variant_spec_data));
		User::LeaveIfError(tmpFile.Write(dataLoc, nvarHdr));
		dataLoc += sizeof(Sym32_variant_spec_data);

		//Write the data
		User::LeaveIfError(tmpFile.Write(dataLoc, (*iRawData)[dataIndex]));
		dataLoc += variantData->es_size;
		}

	//Write the string info
	Elf32_Phdr sphdr;
	TPtr8 sphdrPtr((TUint8*)&sphdr, sizeof(Elf32_Phdr), sizeof(Elf32_Phdr));

	sphdr.p_type = PT_NOTE;
	sphdr.p_offset = WriteStringInfoL(tmpFile, dataLoc);
	sphdr.p_vaddr = 0;
	sphdr.p_paddr = 0;
	sphdr.p_filesz = 0;
	sphdr.p_memsz = 0;
	sphdr.p_flags = 0;
	sphdr.p_align = 4;

	User::LeaveIfError(tmpFile.Write(phdrLoc, sphdrPtr));
	newProgramHeaders.Append(sphdr);
	phdrLoc += sizeof(Elf32_Phdr);
	++numberofPhdrs;

	//Last thing, is to finally update the ELF header and write it
	iELFHeader.e_phoff = sizeof(Elf32_Ehdr);
	iELFHeader.e_phnum = numberofPhdrs;

	LOG_MSG("\tWriting final ELF Header");
	TPtr8 hdrPtr((TUint8*)&iELFHeader, sizeof(Elf32_Ehdr), sizeof(Elf32_Ehdr));
	User::LeaveIfError(tmpFile.Write(0, hdrPtr));

	tmpFile.Flush();
	CleanupStack::PopAndDestroy(&tmpFile);

	TBuf<KMaxFileName> origFilename;
	iELFFile.FullName(origFilename);
	iELFFile.Close();

	SwapFilesL(tmpFilename, origFilename);

	//Open the file again for further use and reset data structs
	User::LeaveIfError(iELFFile.Open(iFsSession, origFilename, EFileWrite));

	iNewSegments.Reset();
	iProgramHeaders.Close();
	iProgramHeaders = newProgramHeaders;  //Update with latest p hdrs
	}

/**
 * Writes the string info at specified offset
 * This writes the Sym32_dhdr and the string info
 * @param aFile File to write to
 * @param aOffset Position in file to write to. This is updated and finishes at the next free point to
 * write to in this file
 * @leave One of the system wide codes
 * @return Offset in the file where this was written to (the initial value of aOffset)
 */
TUint CSELFEditor::WriteStringInfoL(RFile aFile, TUint& aOffset)
	{
	LOG_ENTRY();

	TUint writeStart = aOffset;

	Sym32_dhdr dhdr;
	TPtr8 dhdrPtr((TUint8*)&dhdr, sizeof(Sym32_dhdr), sizeof(Sym32_dhdr));

	TInt index = iStringTable->GetIndex(KStr);
	dhdr.d_name		= (index == KErrNotFound) ? ESYM_STR_UNDEF : index;
	dhdr.d_descrsz	= iStringTable->GetSize();
	dhdr.d_type		= ESYM_NOTE_STR;
	dhdr.d_version	= iStringTable->GetIndex(KSymbianElfCoreDumpVersionV2);
    dhdr.d_elemnum	= 1;

	//Write the dhdr
	User::LeaveIfError(aFile.Write(aOffset, dhdrPtr));
	aOffset += sizeof(Sym32_dhdr);

	//Write the string info
	TText8* buf = iStringTable->GetAllStrings();
	TUint size = iStringTable->GetSize();

	TPtr8 data((TUint8*)buf, size, size);
	User::LeaveIfError(aFile.Write(aOffset, data));
	aOffset += size;

	return writeStart;
	}

/**
 * Copies the registers to aFile sensibly, updating the offsets accordingly
 * @param aFile Destination file
 * @param aSrcRead Place in src file where d_hdr starts
 * @param aDestWrite Destination to write to
 * @leave One of the system wide codes
 */
TUint CSELFEditor::CopyRegistersL(RFile aFile, TUint aSrcRead, TUint aDestWrite, TUint& aSizeWritten)
	{
	LOG_ENTRY();

	aSizeWritten = 0;
	TUint srcpos = aSrcRead;
	TUint destpos = aDestWrite;

	//Write the dhdr, it wont change
	READ_STRUCTL(Sym32_dhdr, dhdr, iELFFile, srcpos);
	CopyFromELFFileL(srcpos, sizeof(Sym32_dhdr), aFile, destpos);
	srcpos += sizeof(Sym32_dhdr);
	destpos += sizeof(Sym32_dhdr);

	//And the reginfod struct
	READ_STRUCTL(Sym32_reginfod, reginfo, iELFFile, srcpos);
	CopyFromELFFileL(srcpos, sizeof(Sym32_reginfod), aFile, destpos);
	srcpos += sizeof(Sym32_reginfod);
	destpos += sizeof(Sym32_reginfod);

	RBuf8 regVals;
	regVals.CreateL(reginfo.rid_num_registers * sizeof(TUint64));
	regVals.CleanupClosePushL();

	//Now we need to go through the individual register values. They are offset to somewhere and
	//we need to figure out where we will be putting them first
	TUint regoffset = destpos + (reginfo.rid_num_registers * sizeof(Sym32_regdatad) );
	for(TUint i = 0; i < reginfo.rid_num_registers; i++)
		{
		READ_STRUCTL(Sym32_regdatad, regdata, iELFFile, srcpos);
		srcpos += sizeof(Sym32_regdatad);

		//update the offset
		TUint srcOffset = regdata.rd_data;
		regdata.rd_data = regoffset;
		regoffset += sizeof(TUint64);

		//Write it to the file
		TPtr8 dataPtr((TUint8*)&regdata, sizeof(Sym32_regdatad), sizeof(Sym32_regdatad));
		User::LeaveIfError(aFile.Write(destpos, dataPtr));
		destpos += sizeof(Sym32_regdatad);

		//Read the appropriate value
		TUint32 regValue = 0;
		TPtr8 regValuePtr((TUint8*)&regValue, sizeof(TUint64), sizeof(TUint64));

		switch(reginfo.rid_repre)
			{
			case ESYM_REG_8:
				User::LeaveIfError(iELFFile.Read(srcOffset, regValuePtr, sizeof(TUint8)));
				break;
			case ESYM_REG_16:
				User::LeaveIfError(iELFFile.Read(srcOffset, regValuePtr, sizeof(TUint16)));
				break;
			case ESYM_REG_32:
				User::LeaveIfError(iELFFile.Read(srcOffset, regValuePtr, sizeof(TUint32)));
				break;
			case ESYM_REG_64:
				User::LeaveIfError(iELFFile.Read(srcOffset, regValuePtr, sizeof(TUint64)));
				break;
			default:
				regValue = 0;
				break;
			}

		regValuePtr.SetLength(sizeof(TUint64));
		regVals.Append(regValuePtr);
		}

	//Now write the values
	User::LeaveIfError(aFile.Write(destpos, regVals));
	destpos += regVals.Size();

	CleanupStack::PopAndDestroy(&regVals);

	aSizeWritten = destpos - aDestWrite;
	return aDestWrite;
	}

/**
 * Copies the variant data to aFile sensibly, updating the offsets accordingly
 * @param aFile Destination file
 * @param aSrcRead Place in src file where d_hdr starts
 * @param aDestWrite Destination to write to
 * @leave One of the system wide codes
 */
TUint CSELFEditor::CopyVariantDataL(RFile aFile, TUint aSrcRead, TUint aDestWrite, TUint& aSizeWritten)
	{
	LOG_ENTRY();

	aSizeWritten = 0;
	TUint srcpos = aSrcRead;
	TUint destpos = aDestWrite;

	//Write the dhdr, it wont change
	READ_STRUCTL(Sym32_dhdr, dhdr, iELFFile, srcpos);
	CopyFromELFFileL(srcpos, sizeof(Sym32_dhdr), aFile, destpos);
	srcpos += sizeof(Sym32_dhdr);
	destpos += sizeof(Sym32_dhdr);

	//And the var data struct
	READ_STRUCTL(Sym32_variant_spec_data, varData, iELFFile, srcpos);
	varData.es_data = destpos + sizeof(Sym32_variant_spec_data);

	TPtr8 varDataPtr((TUint8*)&varData, sizeof(Sym32_variant_spec_data), sizeof(Sym32_variant_spec_data));
	User::LeaveIfError(aFile.Write(destpos, varDataPtr));
	srcpos += sizeof(Sym32_variant_spec_data);
	destpos += sizeof(Sym32_variant_spec_data);

	//And the data
	CopyFromELFFileL(srcpos, varData.es_size, aFile, destpos);
	srcpos += varData.es_size;
	destpos += varData.es_size;

	aSizeWritten = destpos - aDestWrite;
	return aDestWrite;
	}

/**
 * Copies a part of the ELF file into a destination file
 * @param aStartPoint Point in file to start copying
 * @param aSize Amount to copy
 * @param aDestinationFile Destination file to copy to
 * @param aDestWrite Position in destination to write to
 * @return Place in file where this has been written
 * @leave One of the System wide codes
 */
TUint CSELFEditor::CopyFromELFFileL(TUint aStartPoint, TUint aSize, RFile aDestinationFile, TUint aDestWrite)
	{
	TUint readpos = aStartPoint;
	TUint endpoint = aStartPoint + aSize;
	TUint destpos = aDestWrite;

	//Use free memory/10 for our buffering
	TInt block = 0;
	TInt freemem = User::Available(block);
	TInt maxfilecopysize = freemem/10;

	RBuf8 copybuf;
	copybuf.CreateL(maxfilecopysize);
	copybuf.CleanupClosePushL();

	while(readpos < endpoint)
		{
		TUint readsize = maxfilecopysize;
		if(readpos + readsize > endpoint)
			readsize = endpoint - readpos;

		if(readsize != copybuf.MaxLength())
			copybuf.ReAllocL(readsize);

		User::LeaveIfError(iELFFile.Read(readpos, copybuf, readsize));
		User::LeaveIfError(aDestinationFile.Write(destpos, copybuf));

		copybuf.Zero();

		destpos += readsize;
		readpos += readsize;
		}

	CleanupStack::PopAndDestroy(&copybuf);

	return aDestWrite;
	}

/**
 * Caches the string table in source ELF file into our string info table object
 * @leaves One of the system wide codes
 */
void CSELFEditor::CacheStringTableL()
	{
	LOG_ENTRY();

	for(TUint i = 0; i < iProgramHeaders.Count(); i++)
		{
		Elf32_Phdr phdr = iProgramHeaders[i];

		READ_STRUCTL(Sym32_dhdr, dhdr, iELFFile, phdr.p_offset);
		if(dhdr.d_type == ESYM_NOTE_STR)
			{
			iStringTableDhdr = dhdr;

			RBuf8 stringData;
			stringData.CreateL(dhdr.d_descrsz);
			stringData.CleanupClosePushL();

			User::LeaveIfError(iELFFile.Read(phdr.p_offset + sizeof(Sym32_dhdr), stringData));
			iStringTable = CStringInfoTableV2::NewL(stringData);

			CleanupStack::PopAndDestroy(&stringData);
			return;
			}
		}
	}

/**
 * Utility function to leave space in the file
 * @param aFile
 * @param aSize
 */
void CSELFEditor::PutEmptySpaceAtEndOfFileL(RFile aFile, TUint aSize)
	{
	LOG_ENTRY();

	RBuf8 empty;
	empty.CreateL(aSize);
	empty.CleanupClosePushL();
	empty.SetLength(aSize);

	LOG_MSG2("\tPutEmptySpaceAtEndOfFileL --> Leaving [0x%X] bytes", aSize);
	User::LeaveIfError(aFile.Write(empty));

	CleanupStack::PopAndDestroy(&empty);
	}

/**
 * Swaps 2 files and deletes the original destination
 * @leave One of the system wide codes
 */
void CSELFEditor::SwapFilesL(const TDesC& aSrc, const TDesC& aDest)
	{
	LOG_ENTRY();

	//1. Back up the destination file in case anything fails
	TBuf<KMaxFileName> backupName;
	backupName.Copy(aDest);
	backupName.Append(_L(".bak"));

	CFileMan* fmanager = CFileMan::NewL(iFsSession);
	CleanupStack::PushL(fmanager);
	
	User::LeaveIfError(fmanager->Rename(aDest, backupName));
	
	//2. Rename the src file to the dest file
	TInt err = fmanager->Move(aSrc, aDest);
	if(KErrNone != err)
		{
		//rename failed so restore original file
		TPtr8 dest8((TUint8*)aDest.Ptr(), 2*aDest.Length(), 2*aDest.Length());
		RDebug::Printf("\nFailed to move the tmp file so restoring original file [%S]", &dest8);
		User::LeaveIfError(fmanager->Rename(backupName, aDest));
		User::Leave(err);
		}

	//3. Delete the backup file file
	User::LeaveIfError(fmanager->Delete(backupName));
	CleanupStack::PopAndDestroy(fmanager);
	}
