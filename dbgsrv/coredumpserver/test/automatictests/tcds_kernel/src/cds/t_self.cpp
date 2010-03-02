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
//  Test wrapper for Symbian ELF testing

/**
 @file
 @internalTechnology
*/

#include <crashdatasave.h>

#include "t_self.h"
#include "t_coredumpserver.h"

CSymbianElfWrapper::CSymbianElfWrapper()
	{}

CSymbianElfWrapper::~CSymbianElfWrapper()
	{
	iCoreDumpSession.Close();
	iFsSession.Close();
	}

/**
 * Two phase constructor for CSymbianElfWrapper
 * @return CSymbianElfWrapper object
 * @leave
 */
CSymbianElfWrapper* CSymbianElfWrapper::NewL()
	{
	RDebug::Printf("\ncreating test oibject");
	CSymbianElfWrapper* ret = new (ELeave) CSymbianElfWrapper();
	CleanupStack::PushL(ret);
	ret->ConstructL();
	CleanupStack::Pop(ret);
	return ret;
	}

/**
 * Safe construction
 * @leave
 */
void CSymbianElfWrapper::ConstructL()
	{
	User::LeaveIfError(iCoreDumpSession.Connect());
	User::LeaveIfError(iFsSession.Connect());
	}

/**
 * Handle a command invoked from the script
 * @param aCommand Is the name of the command for a TBuf
 * @param aSection Is the .ini file section where parameters to the command are located
 * @param aAsyncErrorIndex Is used by the TEF Block framework to handle asynchronous commands.
 * @leaves One of the system wide error codes
 */
TBool CSymbianElfWrapper::DoCommandL(const TTEFFunction& aCommand, const TTEFSectionName& aSection, const TInt aAsyncErrorIndex)
	{
	//First we need to get the parameters from the ini file
	TInt crashId;
	TPtrC crashName;
	TPtrC crashSource;
	TPtrC section;

	TBool valsOk = GetIntFromConfig(aSection, KKeyCrashId, crashId) &&
				   GetStringFromConfig(aSection, KKeyCrashName, crashName) &&
				   GetStringFromConfig(aSection, KKeyCrashSource, crashSource) &&
				   GetStringFromConfig(aSection, KKeyElfSection, section);

	if(!valsOk)
		{
		ERR_PRINTF1(_L("Unable to get the proper config option from ini file. Are you sure its valid?"));
		return EFalse;
		}

	//Look at the command to see what we need to do
	if(KCommandGenerateFile() == aCommand && KSourceSystemCrash() == crashSource)
		{
		GenerateFileFromSystemCrashL(crashName, crashId);
		}
	else if(KCommandFindSection() == aCommand)
		{
		VerifySection(section, crashName, ETrue);
		}
	else if(KCommandVerifyNoSection() == aCommand)
		{
		VerifySection(section, crashName, EFalse);
		}
	else
		{
		ERR_PRINTF1(_L("Unable to get the proper command from script file. Are you sure its valid?"));
		return EFalse;
		}

	return ETrue;
	}

/**
 * Verifies a given section in the elf file
 * @param aSectionToVerify Section to verify
 * @param aCrashName ELF file name
 * @param aExists To check if the section exists and is valid, or not
 */
void CSymbianElfWrapper::VerifySection(TDesC& aSectionToVerify, TDesC& aCrashName, TBool aExists)
	{
	RFile selfFile;

	TInt err = selfFile.Open(iFsSession, aCrashName, EFileStream|EFileRead);
	INFO_PRINTF3(_L("Trying to open crash file at [%S] and got back [%d]"), &aCrashName, err);

	T_SELF_ASSERT_TRUE(err == KErrNone, 0);
	CleanupClosePushL(selfFile);

	CacheElfHeader(selfFile);
	ValidateElfFile(selfFile);

	if(aSectionToVerify == KSectionsVariantData())
		{
		VerifyVariantData(selfFile, aExists);
		}

	CleanupStack::PopAndDestroy();
	}

/**
 * Verifies Variant data in the elf file
 * @param aElfFile File handle to elf file
 */
void CSymbianElfWrapper::VerifyVariantData(RFile& aElfFile, TBool aExists)
	{
	INFO_PRINTF1(_L("Looking at the size of variant data section."));

	TUint sectionOffset = GetSectionOffset(aElfFile, ESYM_NOTE_VARIANT_DATA);
	T_SELF_ASSERT_TRUE(sectionOffset > 0, 0);

	TUint sectionSize = GetSectionSize(aElfFile, ESYM_NOTE_VARIANT_DATA);
	if(aExists)
		{
		T_SELF_ASSERT_TRUE(sectionSize > 0, 0);
		}
	else
		{
		T_SELF_ASSERT_TRUE(sectionSize == 0, 0);
		}

	//Now look at the actual structure
	Sym32_variant_spec_data varData;
	TPtr8 varDataDes((TUint8*)&varData, sizeof(Sym32_variant_spec_data));

	T_SELF_ASSERT_TRUE(aElfFile.Read(sectionOffset, varDataDes) == KErrNone, 0);
	
	INFO_PRINTF1(_L("Looking at the details of variant data structure. Must be > 0"));
	T_SELF_ASSERT_TRUE(varData.es_data > 0, 0);

	if(aExists)
		{
		T_SELF_ASSERT_TRUE(varData.es_size > 0, 0);
		}
	else
		{
		T_SELF_ASSERT_TRUE(varData.es_size == 0, 0);
		}

	INFO_PRINTF2(_L("Variant data lives in the file at [0x%X]"), varData.es_data);
	INFO_PRINTF2(_L("Variant data is [0x%X] in size"), varData.es_size);
	}

/**
 * Looks at elf signature to see if its ok
 * @param aElfFile File to check
 */
void CSymbianElfWrapper::ValidateElfFile(RFile& aElfFile)
	{
	INFO_PRINTF1(_L("Verifying Elf Header..."));
	T_SELF_ASSERT_TRUE(iElfHeader.e_ident[EI_MAG0] == 0x7f, 1);
	T_SELF_ASSERT_TRUE(iElfHeader.e_ident[EI_MAG1] == 0x45, 1);
	T_SELF_ASSERT_TRUE(iElfHeader.e_ident[EI_MAG2] == 0x4c, 1);
	T_SELF_ASSERT_TRUE(iElfHeader.e_ident[EI_MAG3] == 0x46, 1);
	}

/**
 * Caches the ELF header in the object
 * @param aElfFile file to look at
 */
void CSymbianElfWrapper::CacheElfHeader(RFile& aElfFile)
	{
	INFO_PRINTF1(_L("Reading Elf Header..."));
	TUint fileLocation = 0;
	TPtr8 hdrDes((TUint8*)&iElfHeader, sizeof(Elf32_Ehdr));
	T_SELF_ASSERT_TRUE(aElfFile.Read(fileLocation, hdrDes) == KErrNone, 0);
	}

/**
 * Gets the size of a given section in the elf file.
 * @param aElfFile File to look at
 * @param aNoteType Note of interest
 * @return Section size, 0 if not found
 */
TUint CSymbianElfWrapper::GetSectionSize(RFile& aElfFile, ESYM_NOTE_TYPE aNoteType)
	{
	INFO_PRINTF2(_L("Getting section size for section [0x%X]..."), aNoteType);
	TUint32 sectionOffset = GetSectionOffset(aElfFile, aNoteType);

	T_SELF_ASSERT_TRUE_RET(sectionOffset != 0, 0, 0);

	Sym32_dhdr dhdr;
	TPtr8 dhdrDes((TUint8*)&dhdr, sizeof(Sym32_dhdr));

	T_SELF_ASSERT_TRUE_RET(aElfFile.Read(sectionOffset, dhdrDes) == KErrNone, 0, 0);

	return dhdr.d_descrsz;
	}

/**
 * Searches the elf file for a given section and returns the offset of that section
 * @return Section offset - 0 if not found
 */
TUint CSymbianElfWrapper::GetSectionOffset(RFile& aElfFile, ESYM_NOTE_TYPE aNoteType)
	{
	INFO_PRINTF2(_L("Getting section offset for section [0x%X]..."), aNoteType);

	//Look through descriptor notes for this type
	for(TUint j = 0; j < iElfHeader.e_phnum; j++)
		{
		Elf32_Phdr phdr;
		TPtr8 phdrDes((TUint8*)&phdr, sizeof(Elf32_Phdr));
		T_SELF_ASSERT_TRUE_RET(aElfFile.Read(iElfHeader.e_phoff + j*sizeof(Elf32_Phdr), phdrDes) == KErrNone, 0, 0);

		if(phdr.p_type == PT_NOTE)
			{
			Sym32_dhdr dhdr;
			TPtr8 dhdrDes((TUint8*)&dhdr, sizeof(Sym32_dhdr));
			T_SELF_ASSERT_TRUE_RET(aElfFile.Read(phdr.p_offset, dhdrDes) == KErrNone, 0, 0);

			if(dhdr.d_type == aNoteType)
				{
				return phdr.p_offset + sizeof(Sym32_dhdr);
				}
			}
		}

	return 0;
	}

/**
 * Looks at a given system crash and generates a symbian elf file describing it at
 * specified file
 * @param aCrashName Name of ELF file to generate
 * @param aCrashId ID of crash to analyse
 */
void CSymbianElfWrapper::GenerateFileFromSystemCrashL(TDesC& aCrashName, TInt aCrashId)
	{
	INFO_PRINTF1(_L("Generating ELF file from System crash"));

	//Load SELF and filewriter and configure them to dump where required
	TPluginRequest loadSELFReq;
	loadSELFReq.iPluginType = TPluginRequest::EFormatter;
	loadSELFReq.iLoad = ETrue;
	loadSELFReq.iUid = KUidELFFormatterV2;

	TPluginRequest loadWriterReq;
	loadWriterReq.iPluginType = TPluginRequest::EWriter;
	loadWriterReq.iLoad = ETrue;
	loadWriterReq.iUid = KUidFileWriter;

	INFO_PRINTF1(_L("Loading the file writer and symbian elf..."));
	iCoreDumpSession.PluginRequestL(loadWriterReq);
	iCoreDumpSession.PluginRequestL(loadSELFReq);

	COptionConfig * config = COptionConfig::NewL((TInt)(CCrashDataSave::ECoreFilePath),
		   KUidFileWriter.iUid, COptionConfig::EWriterPlugin, COptionConfig::ETFileName, KFilePathPrompt,
			1, KNullDesC, 0, aCrashName);

	CleanupStack::PushL(config);
	config->Instance(0);

	INFO_PRINTF2(_L("Configuring the file writer to dump file at [%S]..."), &aCrashName);
	iCoreDumpSession.SetConfigParameterL(*config);
	CleanupStack::PopAndDestroy(config);

	//Get the crashes from the flash partition
	RCrashInfoPointerList* crashList = new(ELeave)RCrashInfoPointerList;
	TCleanupItem listCleanup(CCoreDumpServerWrapper::CleanupCrashList, (TAny*)crashList);
	CleanupStack::PushL(listCleanup);

	INFO_PRINTF1(_L("Looking at crashes in flash to find ours..."));
	iCoreDumpSession.ListCrashesInFlashL(*crashList);
	T_SELF_ASSERT_TRUE(crashList->Count() != 0, 1);
	T_SELF_ASSERT_TRUE((*crashList)[0]->iCrashId == 0, 1);

	CleanupStack::PopAndDestroy(1);

	TRequestStatus status;
    iCoreDumpSession.ProcessCrashLog(aCrashId, status);
	User::WaitForRequest(status);

	T_SELF_ASSERT_TRUE(KErrNone == status.Int(), 0);
	}

//eof
