// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Example CTestStep derived implementation
//



/**
 @file SELFusersidestrace.cpp
 @internalTechnology
*/
#include "SELFusersidetrace.h"
#include "tcoredumpserverSuiteDefs.h"
#include "tcoredumpserverSuiteServer.h"
#include "optionconfig.h"
#include <coredumpserverapi.h>
#include <e32property.h>
#include <crashdatasave.h>

CSELFUserSideTrace::~CSELFUserSideTrace()
/**
 * Destructor
 */
	{
	}

CSELFUserSideTrace::CSELFUserSideTrace()
/**
 * Constructor
 */
	{
	// **MUST** call SetTestStepName in the constructor as the controlling
	// framework uses the test step name immediately following construction to set
	// up the step's unique logging ID.
	SetTestStepName(KSELFUserSideTrace);
	}

TVerdict CSELFUserSideTrace::doTestStepPreambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	   
	TInt ret = KErrNone;
	ret = iCoreDumpSession.Connect(); 
	if(ret != KErrNone)
		{
		SetTestStepResult( EFail );
		INFO_PRINTF2(_L("Error %d from iCoreDumpSession->Connect()/n"), ret);
		}	
	else
		{
		SetTestStepResult(EPass);
		}
	
	ret = iFsSession.Connect();
	if(ret != KErrNone)
		{
		SetTestStepResult( EFail );
		INFO_PRINTF2(_L("Error %d from iFsSession->Connect()/n"), ret);
		}	
	else
		{
		SetTestStepResult(EPass);
		}

	return TestStepResult();
	}


TVerdict CSELFUserSideTrace::doTestStepL()
/**
 * @return - TVerdict code
 * Override of base class pure virtual
 * Our implementation only gets called if the base class doTestStepPreambleL() did
 * not leave. That being the case, the current test result value will be EPass.
 */
	{

	if (TestStepResult()==EPass)
		{	
		TInt ret = KErrNone;

		__UHEAP_MARK;		
		
		TRAP(ret, ClientAppL());
		if(KErrNone != ret)
			{
			SetTestStepResult(EFail);
			INFO_PRINTF2(_L("Error %d from CSELFStep->ClientAppL()"), ret);
			}		

		__UHEAP_MARKEND;

		}
	return TestStepResult();  
	}

void CSELFUserSideTrace::ClientAppL()
/**
 * @return void
 * This calls each stage of the test
 */
	{
	INFO_PRINTF1(_L("Starting SELF Test Suite") );
	HandleCrashL();
    //CheckParameters();
	}

void CSELFUserSideTrace::HandleCrashL()
/**
 * @return void
 * Tests handling a crash
 */
	{
	iTraceFlag = EFalse;
	if(TestStepResult()==EPass)
		{
		TPtrC crashAppParam;
		TInt tracePresent;
		
		if(!GetStringFromConfig(ConfigSection(), KTe_CrashAppParam, crashAppParam) ||
		   !GetIntFromConfig(ConfigSection(), KTrace, tracePresent))
			{
			INFO_PRINTF1(_L("Failed to get data from ini file"));
			SetTestStepResult(EFail);
			User::Leave(KErrGeneral);
			}
		
		INFO_PRINTF3(_L("The CrashApp Param is %S, Ok so the trace parameter is %d"), &crashAppParam, &tracePresent); // Block end
				
		if(tracePresent == 1)
			iTraceFlag = ETrue; //trace is present
		else
			iTraceFlag = EFalse; //trace not present
			
		INFO_PRINTF2(_L("iTraceFlag %d"), iTraceFlag);
		
		//Start the process that we intend to crash....
		RProcess crashProcess;
		CleanupClosePushL(crashProcess);
		
		TInt ret = crashProcess.Create( KCrashAppFileName, crashAppParam);
		if(ret != KErrNone)
			{
			INFO_PRINTF2(_L("Error %d from RProcess .Create(z:\\sys\\bin\\crashapp.exe)/n"), ret);
			SetTestStepResult(EFail);
			User::Leave(ret);
			}
		
		INFO_PRINTF1(_L("Started userside crash app"));
		
		//Load SELF and Writer Formatter 
		DoCmdLoadPluginsL();
		
		//configure the CDS and File Writer
		DoCmdConfigurePluginsL();
		
		//configure the SELF formatter accordingly for trace/no trace
		ConfigureSELF(iTraceFlag);
		
		// Observe the process and wait for a crash
		TRAP(ret, iCoreDumpSession.ObservationRequestL( KCrashAppFileName, KCrashAppFileName, ETrue) );
		if(ret != KErrNone)
			{
			INFO_PRINTF2(_L("Error %d iCoreDumpSession.ObservationRequestL(z:\\sys\\bin\\crashapp.exe)\n"), ret);
			SetTestStepResult(EFail);
			User::Leave( ret );
			}
		
		//start the crash process
		crashProcess.Resume();
		CleanupStack::PopAndDestroy(&crashProcess); //this is for crashProcess
	
		//Monitor the progress of the crash file being generated...
		MonitorProgressL();
        User::After(5000000);
        
        iFsSession.SetSessionPath(KDir); //set the default path to e:\\ MMC card
    	//Use wildcard utilities to search for the SELF FIle Created
    	TBuf<32> buf;
    	buf = KDir;
    	TUidType uid1;
    	TUidType uid2(uid1.MostDerived());
    	CFindFileByType* obj = new(ELeave) CFindFileByType(iFsSession);
    	CleanupStack::PushL(obj);

    	ret = obj->FindFirst(KCrashWildCard, buf, uid2);
    	if (ret != KErrNone)
    		{
    		ERR_PRINTF2(
    				_L("GenerateElfFileL Error finding the SELF File %d"),
    				ret);
    		SetTestStepResult(EFail);
    		User::Leave(ret);
    		}

    	iSELFFileName = obj->Entry().iName;
    	CleanupStack::PopAndDestroy(); //CFindFileByType
        
    	INFO_PRINTF2(_L("SELF File found: %S"),&iSELFFileName);
    	
    	//test for a valid Trace section
    	DoTestTraceSectionSELFFileL(iTraceFlag);
		
		INFO_PRINTF1(_L("SUCCESS Trace Data Presence/Absence verified!"));
		}
	}

/**
 * Validates the presence of the trace data in the SELF file generated
 */
void CSELFUserSideTrace::DoTestTraceSectionSELFFileL(TBool aFlag)
	{
	
	TInt err = iSELFFile.Open(iFsSession, iSELFFileName, EFileStream|EFileRead);
	if (err != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::ProcessELFFile Error opening the SELF File %d"),
				err);
		SetTestStepResult(EFail);
		User::Leave(err);
		}
	
	//validate the SELF Header Format	
	ValidateHeaderELFFileL();
	
	//validate the Program Header and store information about the string section Sections
	ValidateProgramHeaderELFFileL();

	//validate the trace section to be present
	ValidateTraceSectionELFFileL(aFlag);
	
	//do the necessary clean up 
	CleanupMethod(iSELFFileName);
		
	}

/**
 * Validates the Header Format Elf32_Ehdr for the SELF file 
 */
void CSELFUserSideTrace::ValidateHeaderELFFileL()
	{// Elf32_Ehdr
	INFO_PRINTF1(_L("CSELFUserSideTrace::ValidateHeaderELFFileL validating Elf32_Ehdr"));
	
	TInt sizeofELFHeader = sizeof(Elf32_Ehdr); //size of Elf32_Ehdr structure

	// allocate buffer Elf32_Ehdr
	TUint8* SELFHeader = new TUint8[sizeofELFHeader];
	TPtr8 elfHeaderPointer(SELFHeader, sizeofELFHeader, sizeofELFHeader);
	CleanupStack::PushL(SELFHeader);

	//read the SELF file for Elf32_Ehdr
	TInt ret = iSELFFile.Read(0, elfHeaderPointer, sizeofELFHeader);
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CSELFUserSideTrace::ValidateHeaderELFFile Error reading the SELF Header Elf32_Ehdr %d"),
				ret);
		SetTestStepResult(EFail);
		User::Leave(ret);
		}
	//
	Elf32_Ehdr* eh=(Elf32_Ehdr *)SELFHeader;

	//VALIDATE THE Elf32_Ehdr

	if (eh->e_ident[EI_MAG0] !=0x7f || eh->e_ident[EI_MAG1] != 0x45
			|| eh->e_ident[EI_MAG2] !=0x4c || eh->e_ident[EI_MAG3] != 0x46)
		{
		// EI_MAG0 to EI_MAG3 - A files' first 4 bytes hold a 'magic number', identifying the file as an ELF object file.
		ERR_PRINTF1(_L("Error:  is not a valid ELF file"));
		SetTestStepResult(EFail);
		User::Leave(KErrGeneral);
		}
	if (eh->e_ident[EI_DATA] == 2)
		{
		// ELF Header size should be 52 bytes or converted into Big-Endian system 13312
		if (eh->e_ehsize != 13312)
			{
			ERR_PRINTF1(_L("Error:  ELF Header contains invalid file typ"));
			SetTestStepResult(EFail);
			User::Leave(KErrGeneral);
			}
		// e_ident[EI_DATA] specifies the data encoding of the processor-specific data in the object file.
		ERR_PRINTF1(_L("Error:  Data encoding ELFDATA2MSB (Big-Endian) not supported"));
		SetTestStepResult(EFail);
		User::Leave(KErrGeneral);
		}
	if (eh->e_ehsize != 52)
		{
		// ELF Header size should be 52 bytes
		ERR_PRINTF1(_L("Error:  ELF Header contains invalid file type"));
		SetTestStepResult(EFail);
		User::Leave(KErrGeneral);
		}

	//Number of entries in the program header table
	if ((eh->e_phnum) == 0)
		{
		ERR_PRINTF1(_L("Error:  ELF Header contains zero program headers"));
		SetTestStepResult(EFail);
		User::Leave(KErrGeneral);
		}
	
	//store number of program header entries
	iPHEntries = eh->e_phnum;
	INFO_PRINTF2(_L("Program header entries:%d"), iPHEntries);

	//Offset in bytes from the start of the file to the section header table
	if ((eh->e_phoff) == 0)
		{
		ERR_PRINTF1(_L("Error:  ELF Header contains zero offset to the section header table"));
		SetTestStepResult(EFail);
		User::Leave(KErrGeneral);
		}

	//store program header offset
	iPHOffset = eh->e_phoff;
	INFO_PRINTF2(_L("Program header offsett:%d"), iPHOffset);

	CleanupStack::PopAndDestroy(SELFHeader);
	
	INFO_PRINTF1(_L("Validation of Elf32_Ehdr successful"));
	}

/**
 * Validates the Program Header Entries Elf32_Phdr of the SELF file 
 */
void CSELFUserSideTrace::ValidateProgramHeaderELFFileL()
	{//Elf32_Phdr
	INFO_PRINTF1(_L("CSELFUserSideTrace::ValidateProgramHeaderELFFileL validating the program header Entries"));

	TInt sizeofELFProgramHeader = sizeof(Elf32_Phdr); //size of Elf32_Phdr
	TInt totalsizeofELFProgramHeader = iPHEntries * sizeofELFProgramHeader;

	//allocate buffer Elf32_Phdr
	iSELFPHHeader = new TUint8[totalsizeofELFProgramHeader];
	TPtr8 elfPHHeaderPointer(iSELFPHHeader, totalsizeofELFProgramHeader,
			totalsizeofELFProgramHeader);

	//read the SELF for the whole Program Header Table
	TInt ret = iSELFFile.Read(iPHOffset, elfPHHeaderPointer,
			totalsizeofELFProgramHeader);
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CSELFUserSideTrace::ValidateProgramHeaderELFFileL Error reading the SELF Header Elf32_Phdr %d"),
				ret);
		SetTestStepResult(EFail);
		User::Leave(ret);
		}

	Elf32_Phdr* phdr = (Elf32_Phdr*) iSELFPHHeader;//Elf32_Phdr Program Header

	//Prepare for each of the DHDR buffers 
	TInt sizeofDhdr = sizeof(Sym32_dhdr);

	//loop through the program headers to find out more information on the String section
	for (TInt i = 0; i < iPHEntries; i++)
		{
		//allocate DHDR buffers Sym32_dhdr
		iDHDRBuffer[i] = new TUint8[sizeofDhdr];
		TPtr8 dhdrPointer(iDHDRBuffer[i], sizeofDhdr, sizeofDhdr);
        
		//read SELF file for the Sym32_dhdr
		TInt ret = iSELFFile.Read(phdr[i].p_offset, dhdrPointer, sizeofDhdr);
		if (ret != KErrNone)
			{
			ERR_PRINTF2(
					_L("CSELFUserSideTrace::ValidateHeaderELFFile Error reading the SELF Header Sym32_dhdr %d"),
					ret);
			SetTestStepResult(EFail);
			User::Leave(ret);
			}

		idhdr[i] = (Sym32_dhdr*) iDHDRBuffer[i];//array of Sym32_dhdr store it

		//find information about the String Info section
		if (phdr[i].p_type == PT_NOTE)
			{
			// Intend to store information about the string section ESYM_NOTE_STR
			//the srting section is to be accessed for all sectiosn to be parsed
			if (idhdr[i]->d_type == ESYM_NOTE_STR)
				{
				INFO_PRINTF2(_L("ESYM_NOTE_STR Header offset %d"),
						phdr[i].p_offset);
				INFO_PRINTF2(
						_L("ESYM_NOTE_STR Size of single descriptor element %d "),
						idhdr[i]->d_descrsz);

				iOffsetStringSection = phdr[i].p_offset + sizeof(Sym32_dhdr);
				iSizeofStringSection = idhdr[i]->d_descrsz;

				}//if ESYM_NOTE_STR		
			}//if PT_NOTE
		}//for loop

	}//end of function

/**
 * Validates the Trace Section Sym32_tracedata of the SELF file 
 */
void CSELFUserSideTrace::ValidateTraceSectionELFFileL(TBool aTraceCheck)
	{
	
	INFO_PRINTF1(_L("CSELFUserSideTrace::ValidateTraceSectionELFFileL validating the Trace Section"));
	
	Elf32_Phdr* phdr = (Elf32_Phdr*) iSELFPHHeader;//Elf32_Phdr Program Header
	
	TBool dataPresent = EFalse;
	//loop through the program headers to get information about the Symbian Info section
	for (TInt i = 0; i < iPHEntries; i++)
		{
		if (phdr[i].p_type == PT_NOTE)
			{
			if (idhdr[i]->d_type == ESYM_NOTE_TRACE) //Trace Info
				{
				TInt sizeoftraceinfo = sizeof(Sym32_tracedata);
				INFO_PRINTF2(_L("ESYM_NOTE_TRACE Size of trace section %d"), sizeoftraceinfo);
				INFO_PRINTF2(_L("ESYM_NOTE_TRACE Header offset %d"), phdr[i].p_offset);
				INFO_PRINTF2(_L("ESYM_NOTE_TRACE Size of single descriptor element %d"),
						idhdr[i]->d_descrsz);
				INFO_PRINTF2(_L("ESYM_NOTE_TRACE Number of elements %d"), idhdr[i]->d_elemnum); 

				if (sizeof(Sym32_tracedata) != (idhdr[i]->d_descrsz))
					{
					ERR_PRINTF1(_L("CSELFUserSideTrace::ValidateTraceSectionELFFileL sizeof(Sym32_tracedata) %d is different from descriptor size") );
					SetTestStepResult(EFail);
					User::Leave(KErrGeneral);
					}
				
				TInt offsettotraceinfod = phdr[i].p_offset + sizeof(Sym32_dhdr);
				//allocate buffer for Sym32_tracedata
				TUint8* symbianTraceSection = new TUint8[sizeoftraceinfo];
				CleanupStack::PushL(symbianTraceSection);

				TPtr8 ptrSymbianTraceSection(symbianTraceSection, sizeoftraceinfo,
						sizeoftraceinfo);

				TInt ret = iSELFFile.Read(offsettotraceinfod, ptrSymbianTraceSection, sizeoftraceinfo);
				if (ret != KErrNone)
					{
					ERR_PRINTF2(
							_L("CSELFUserSideTrace::ValidateTraceSectionELFFileL Error reading the SELF Header Sym32_thrdinfod %d"), ret);
					SetTestStepResult(EFail);
					User::Leave(ret);
					}

				Sym32_tracedata *traceInfo =
										(Sym32_tracedata*) symbianTraceSection;
			
				INFO_PRINTF2(_L("Size of trace buffer: %d"), traceInfo->tr_size);
				INFO_PRINTF2(_L("Offset of trace buffer:%d"), traceInfo->tr_data);
				
				//checking for the presence of trace data
				if(traceInfo->tr_size != 0)//trace data present
					{
					dataPresent = ETrue;
					//read the trace buffer
					//unsigned char* data = ADDR(unsigned char, aElfHdr, aTraceData->tr_data);	
					
					RBuf8 tracebufferString;
					tracebufferString.CleanupClosePushL();

					//create trace buffer of size  traceInfo->tr_size
					tracebufferString.CreateL(traceInfo->tr_size);
					
					TInt ret = iSELFFile.Read(traceInfo->tr_data, tracebufferString, traceInfo->tr_size);
					if (ret != KErrNone)
						{
						ERR_PRINTF2(
								_L("CSELFUserSideTrace::ValidateTraceSectionELFFileL Error reading the Trace Information  %d"),
								ret);
						SetTestStepResult(EFail);
						User::Leave(ret);
						}
					
					INFO_PRINTF1(_L("Validate the trace buffer now"));
					ValidateTraceBufferL(tracebufferString);
					
					CleanupStack::PopAndDestroy();//tracebufferString
					CleanupStack::PopAndDestroy();//symbianTraceSection
					break;
					}
				else //expecting trace data to be present otherwise complain
					{
					ERR_PRINTF1(
							_L("CSELFUserSideTrace::ValidateTraceSectionELFFileL Error found trace size to be zero"));
					SetTestStepResult(EFail);
					User::Leave(KErrGeneral);
					
					}
				}			
			}
			
		}

	if(!aTraceCheck && dataPresent )
		{
		ERR_PRINTF1(
				_L("CSELFUserSideTrace::ValidateTraceSectionELFFileL Error found trace section when there shouldnt be one"));
		SetTestStepResult(EFail);
		User::Leave(KErrGeneral);
		}
	if(aTraceCheck  && !dataPresent )
		{
		ERR_PRINTF1(
				_L("CSELFUserSideTrace::ValidateTraceSectionELFFileL Error did not find the trace section"));
		SetTestStepResult(EFail);
		User::Leave(KErrGeneral);
		}
	
	
	}

/**
 * Utility Function to do trace data verification
 */
void CSELFUserSideTrace::ValidateTraceBufferL(const TDesC8& aBuffer)
{
	INFO_PRINTF2(_L("CSELFUserSideTrace::ValidateTraceBufferL: size: %d"), aBuffer.Length());
	
	const TUint8* tracePtr = aBuffer.Ptr();
	TInt traceSize = aBuffer.Length();
	
/*	TUint8 recSize = 0;
	TUint8 flags = 0;
	TUint8 subCategory = 0;*/
	TUint8 category = 0;
	TInt cateogryCount = 0;
	TInt totalCount = 0;

	const TUint8* endPtr = tracePtr+traceSize;
	while(tracePtr < endPtr)
	{ 
		//count of the total number of records
		totalCount++;
		category = 0;
		
/*		
 * for keeping a record of other fields in the Trace		
 		recSize = flags = subCategory = 0;
		recSize = tracePtr[BTrace::ESizeIndex];
		flags = tracePtr[BTrace::EFlagsIndex];
		subCategory = tracePtr[BTrace::ESubCategoryIndex];*/
/*		
 * printing the values
 *      INFO_PRINTF2(_L("recSize: %d"), recSize);
		INFO_PRINTF2(_L("flags: %d"), flags);
		INFO_PRINTF2(_L("subCategory: %d"), subCategory);
		INFO_PRINTF2(_L("category: %d"), category);*/
		
		//read trace header and parse for the Category
		category = tracePtr[BTrace::ECategoryIndex];
		
		//check the parameter Category 
		if(category == KTraceCategory)
			{
			cateogryCount++;//count the number of times the category 200 has appeared
			}
								
		// go to the next trace packet
		tracePtr = BTrace::NextRecord((TAny*)tracePtr);
	}
	
	if(category == 0)
		{// no Category KTraceCategory trace found out
		ERR_PRINTF2(
			_L("CSELFUserSideTrace::ValidateTraceBufferL Category %d trace data NOT found!"), KTraceCategory);
		INFO_PRINTF2(_L("Total Number of trace records found: %d"), totalCount);
		SetTestStepResult(EFail);
		User::Leave(KErrGeneral);
		}
	
	if(category % KTRACENUMBER != 0)
		{//checking for KTRACENUMBER number of KTraceCategory 
		ERR_PRINTF3(	
			_L("CSELFUserSideTrace::ValidateTraceBufferL %d number of Category type %d trace data NOT found!"), KTRACENUMBER, KTraceCategory);
		INFO_PRINTF3(_L("Total number of Category type %d trace data found: %d"), KTraceCategory, cateogryCount);
		INFO_PRINTF2(_L("Total Number of trace records found: %d"), totalCount);
		SetTestStepResult(EFail);
		User::Leave(KErrGeneral);	
		}
	
	INFO_PRINTF3(_L("Total number of Category type %d trace data found: %d"), KTraceCategory, cateogryCount);
	INFO_PRINTF2(_L("Total Number of trace records found: %d"), totalCount);
	INFO_PRINTF1(_L("Validation of Trace Buffer Successful!!!")); 
}

/**
* @return void
* @param aIndex Internal index to the component that owns the object
* @param aUID UID of the component that owns the object
* @param aSource Type of component that owns the object
* @param aType Type of parameter
* @param aPrompt Prompt to present to user 
* @param aNumOptions Number of options that the parameter can be set to. Only applies if type is ETMultiEntryEnum.
* @param aOptions Comma separated list of options. Applies to ETMultiEntryEnum and ETBool
* @param aVal Integer value. Applies to ETInt, ETUInt, ETBool
* @param aStrValue String value. Applies to ETString, ETFileName, ETMultiEntry, ETBool
*/
void CSELFUserSideTrace::DoConfigureL(const TUint32& aIndex, 
		  	     const TUint32& aUID, 
                 const COptionConfig::TParameterSource& aSource, 
                 const COptionConfig::TOptionType& aType, 
                 const TDesC& aPrompt, 
                 const TUint32& aNumOptions,
                 const TDesC& aOptions,
                 const TInt32& aVal, 
                 const TDesC& aStrValue,
                 const TUint aInstance)
	{
	COptionConfig * config;	
	
	config = COptionConfig::NewL( aIndex,
			aUID,
			aSource,
			aType,
			aPrompt,
			aNumOptions,
			aOptions,
			aVal,
			aStrValue);
	
	CleanupStack::PushL(config);
	
    config->Instance(aInstance);

	//Configure now...
	TRAPD(ret, iCoreDumpSession.SetConfigParameterL(*config) );
	if(ret != KErrNone)
		{
		INFO_PRINTF2(_L("Error %d changing param/n"), ret );
		SetTestStepResult(EFail);
		User::Leave(ret);
		}

	CleanupStack::PopAndDestroy(config);
	}

/**
 * @return void
 * This method monitors the RProperty for the crash progress. There are several states of this but we are
 * only interested in the start and finish of these properties
 */
void CSELFUserSideTrace::MonitorProgressL()
	{
	RProperty crashProgress;
	User::LeaveIfError(crashProgress.Attach(KCoreDumpServUid, ECrashProgress));	
	
	TBuf<50> crashProg;			

	TRequestStatus status;
	crashProgress.Subscribe(status);
	User::WaitForRequest(status);
	//Subscribe for next one again...
	crashProgress.Subscribe(status);
	
	//First one should be the start string = "-"
	User::LeaveIfError(crashProgress.Get(crashProg ));		
	if(crashProg != KStartOfSELFProc)
		{
		INFO_PRINTF1(_L("SELF formatter has not started processing the data"));
		}
	
	INFO_PRINTF1(_L("SELF formatters have started processing the data"));
	INFO_PRINTF1(_L("Waiting to be notified of the timeout of the processing. A timeout here is a fail"));
	//Now we wait until its finished
	do
		{			
		User::WaitForRequest(status);		
		crashProgress.Subscribe(status);

		User::LeaveIfError(crashProgress.Get(crashProg ));		
		}
	while(crashProg != KEndOfProcessing);
	
	INFO_PRINTF1(_L("SELF formatter has finished processing the data"));			
	
	}

/**
 * Loads the SELF Formatter and the File Writer plugin
 */
void CSELFUserSideTrace::DoCmdLoadPluginsL()
	{
	INFO_PRINTF1(_L("\nCProcessCrashWrapperloading SELF Formatter and File Writer!!!"));

	TPluginRequest loadSELFReq;
	loadSELFReq.iPluginType = TPluginRequest::EFormatter;
	loadSELFReq.iLoad = ETrue;
	loadSELFReq.iUid = KUidSELFFormatterV2;

	TPluginRequest loadWriterReq;
	loadWriterReq.iPluginType = TPluginRequest::EWriter;
	loadWriterReq.iLoad = ETrue;
	loadWriterReq.iUid = WRITER_UID;

	// loading Symbian ELF formatter
	TRAPD(ret, iCoreDumpSession.PluginRequestL(loadSELFReq));
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CSELFUserSideTrace::DoCmdLoadPluginsL: Could not load SELF plugin!, err:%d\n"),
				ret);
		SetTestStepResult(EFail);
		User::Leave(ret);
		}

	// loading Symbian File writer
	TRAP(ret, iCoreDumpSession.PluginRequestL(loadWriterReq));
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CSELFUserSideTrace::DoCmdLoadPluginsL: Could not load writer plugin!, err:%d\n"),
				ret);
		SetTestStepResult(EFail);
		User::Leave(ret);
		}

	//intend to check for the loaded SELF formatter and File Writer
	RPluginList pluginLists;
	CleanupClosePushL(pluginLists);

	TRAP(ret, iCoreDumpSession.GetPluginListL(pluginLists));

	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CSELFUserSideTrace::DoCmdLoadPluginsL: Could not get plugin list!, err:%d\n"),
				ret);
		SetTestStepResult(EFail);
		User::Leave(ret);
		}

	//Now we look through the list, until we find our plugin, its a fail

	for (TInt i = 0; i < pluginLists.Count(); i++)
		{
		if (pluginLists[i].iUid == KUidSELFFormatterV2 && !(pluginLists[i].iLoaded))
			{
			ERR_PRINTF1(
					_L("SELF Plugin Not loaded !\n"));
			SetTestStepResult(EFail);
			User::Leave(KErrNotFound);
			}
		}

	for (TInt i = 0; i < pluginLists.Count(); i++)
		{
		if (pluginLists[i].iUid == WRITER_UID && !(pluginLists[i].iLoaded))
			{
			ERR_PRINTF1(
								_L("Writer Plugin Not loaded !\n"));
			SetTestStepResult(EFail);
			User::Leave(KErrNotFound);
			}
		}

	INFO_PRINTF1(_L("SELF Formatter and File Writer loaded successfully"));
	CleanupStack::PopAndDestroy();//pluginLists	
	}

/**
 * Configuring the Core Dump Server and the File Writer
 */
void CSELFUserSideTrace::DoCmdConfigurePluginsL()
	{
	INFO_PRINTF1(_L("CSELFUserSideTrace DoCmdConfigurePlugins configuring the Core dump server and File Writer plugin"));

	//Configuring the Core Dump Server
	TRAPD(ret, DoConfigureL(2, CDS_UID.iUid, COptionConfig::ECoreDumpServer,
			COptionConfig::ETUInt, KPostCrashEventActionPrompt, 1, KNullDesC,
			4, KNullDesC, 0));
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CSELFUserSideTrace::DoConfigureL for CDS Error %d changing param"),
				ret);
		SetTestStepResult(EFail);
		User::Leave(ret);
		}

	//Configuring the file writer plugin to have a specified crash file name
	TRAPD(err, DoConfigureL((TInt)(CCrashDataSave::ECoreFilePath),
			WRITER_UID.iUid, COptionConfig::EWriterPlugin,
			COptionConfig::ETFileName, KFilePathPrompt, 1, KNullDesC, 0,
			KCrashFileName, 0));

	if (err != KErrNone)
		{
		ERR_PRINTF2(_L("CSELFUserSideTrace::DoConfigureL for File Writer Error %d changing param"),
				err);
		SetTestStepResult(EFail);
		User::Leave(err);
		}

	}

/**
 * Utility Function to do SELF Configurations
 */
void CSELFUserSideTrace::ConfigureSELF(TBool aTracePresent)
	{
	INFO_PRINTF2(_L("CProcessCrashWrapper::ConfigureSELF confiugring SELF for Trace presence %d"), aTracePresent);

	if(aTracePresent)
		{//configure to have trace segments upto have 10KB data
		DoConfigureL(8, KUidSELFFormatterV2.iUid, COptionConfig::EFormatterPlugin,
				COptionConfig::ETInt,  KCreateTraceData, 
				1, KNullDesC, 10, KNullDesC, 0);
		
		}
	else
		{//configure NOT to have trace segments
		DoConfigureL(8, KUidSELFFormatterV2.iUid, COptionConfig::EFormatterPlugin,
				COptionConfig::ETInt,  KCreateTraceData, 
				1, KNullDesC, 0, KNullDesC, 0);
		}
	
	}

/**
 * Utility Function to do cleanup activities
 */
void CSELFUserSideTrace::CleanupMethod(const TDesC& aDes)
	{
	RBuf fileDelete;
	fileDelete.CreateL(aDes);
	fileDelete.SetMax();
	fileDelete.CleanupClosePushL();
	INFO_PRINTF2(_L("Entered deletion routine for file %S"), &fileDelete);
	
	//close the file
	iSELFFile.Close();
	

	TInt ret = iFsSession.Delete(fileDelete);
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CSELFUserSideTrace::SELF File delete returned error %d"),
				ret);
		SetTestStepResult(EFail);
		User::Leave(ret);
		}

	CleanupStack::PopAndDestroy();//fileDelete
	

	//delete the buffers
	if (iSELFPHHeader)
		{
		delete [] iSELFPHHeader;
		iSELFPHHeader = NULL;
		}

	for (TInt i = 0; i < iPHEntries; i++)
		{
		delete [] iDHDRBuffer[i];
		}	 

	
	INFO_PRINTF1(_L("Cleanup Done"));
	}

/**
 * @return - TVerdict code
 * Override of base class virtual
 */
TVerdict CSELFUserSideTrace::doTestStepPostambleL()
	{
	iCoreDumpSession.Disconnect();
	User::After(7000000); //ensure we give enough time for session to close
	iFsSession.Close();
	return EPass;	
	}




