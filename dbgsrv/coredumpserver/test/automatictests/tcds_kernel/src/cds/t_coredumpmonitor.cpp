// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
*/

#include "t_coredumpmonitor.h"

//Names of functions to be tested - referenced from script file
_LIT(KUntrustedAccessToSCMConfig, "UntrustedAccessToSCMConfig");
_LIT(KGatherTraceBuffer, "GatherTraceBuffer");
_LIT(KTrustedAccessToSCMConfig, "TrustedAccessToSCMConfig");
_LIT(KReadDefaultConfigValues, "ReadDefaultConfigValues");
_LIT(KValidateNewestCrash, "ValidateNewestCrash");

/**
 * Constructor for test wrapper
 */
CCoreDumpMonitorWrapper::CCoreDumpMonitorWrapper()
	{
	}

/**
 * Destructor
 */
CCoreDumpMonitorWrapper::~CCoreDumpMonitorWrapper()
	{
	iCoreDumpSession.Close();
	iCrashList.ResetAndDestroy();
	iFsSession.Close();
	}

/**
 * Two phase constructor for CCoreDumpMonitorWrapper
 * @return CCoreDumpMonitorWrapper object
 * @leave
 */
CCoreDumpMonitorWrapper* CCoreDumpMonitorWrapper::NewL()
	{
	CCoreDumpMonitorWrapper* ret = new (ELeave) CCoreDumpMonitorWrapper();
	CleanupStack::PushL(ret);
	ret->ConstructL();
	CleanupStack::Pop(ret);
	return ret;
	}

/**
 * Safe construction
 * @leave
 */
void CCoreDumpMonitorWrapper::ConstructL()
	{
	

	User::LeaveIfError(iCoreDumpSession.Connect());
	User::LeaveIfError(iFsSession.Connect());
	}

/**
 * Assign the object
 * @param aObject TAny* to the object to test
 * @leave
 */
void CCoreDumpMonitorWrapper::SetObjectL(TAny* aObject)
	{}

/**
 * Runs a test preamble
 */
void CCoreDumpMonitorWrapper::PrepareTestL()
	{
	SetBlockResult(EPass);
	INFO_PRINTF1(_L("CCoreDumpMonitorWrapper::Ready to start test"));
	}

/**
 * Handle a command invoked from the script
 * @param aCommand Is the name of the command for a TBuf
 * @param aSection Is the .ini file section where parameters to the command are located
 * @param aAsyncErrorIndex Is used by the TEF Block framework to handle asynchronous commands.
 */
TBool CCoreDumpMonitorWrapper::DoCommandL(const TTEFFunction& aCommand, const TTEFSectionName& aSection, const TInt aAsyncErrorIndex)
	{
	//__UHEAP_MARK;	
	PrepareTestL();

	if (KUntrustedAccessToSCMConfig() == aCommand)
		{
		TestUntrustedAccessToCDSConfigL();
		}
	else if(KGatherTraceBuffer() == aCommand)
		{
		TestTraceBufferAccessL();
		}
	else if(KTrustedAccessToSCMConfig() ==aCommand)
		{
		TestTrustedAccessToCDSConfigL();
		}
	else if (KReadDefaultConfigValues() == aCommand)
		{
		TestReadDefaultConfigValuesL();
		}
else if(KValidateNewestCrash() == aCommand)
		{
		TestValidateNewestCrashL();
		}
	else
		{
		return EFalse;
		}

	//__UHEAP_MARKEND;

	return ETrue;
	}

void CCoreDumpMonitorWrapper::TestTrustedAccessToCDSConfigL()
	{
	INFO_PRINTF1(_L("TestTrustedAccessToCDSConfigL"));
	
	INFO_PRINTF1(_L("Getting the number of config parameters"));
	
	TUint numConfigParams = 0;
	TRAPD(err, numConfigParams = iCoreDumpSession.GetNumberConfigParametersL());
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Unable to get the number of config parameters. Returned %d"), err);		
		SetBlockResult(EFail);
		return;
		}
	
	INFO_PRINTF2(_L("Found %d params"), numConfigParams);
	TBool foundScmConfig = EFalse;
	
	for (TInt i = 0; i < numConfigParams; i++)
		{
		COptionConfig* conf;
		conf = iCoreDumpSession.GetConfigParameterL(i);
		
		if (conf->Source()== COptionConfig::ESCMConfig)
			{
			foundScmConfig = ETrue;
			}
		
		delete conf;
		}
	
	if(!foundScmConfig)
		{
		ERR_PRINTF1(_L("Not Able to retrieve a config param for the SCM"));		
		SetBlockResult(EFail);
		return;
		}
	
	INFO_PRINTF1(_L("Looked at all config parameters and found some SCM params so that is a pass"));
	}

/**
 * Tests that that CDS enforces that untrusted users cannot 
 * access the CDS configuration
 */
void CCoreDumpMonitorWrapper::TestUntrustedAccessToCDSConfigL()
	{
	INFO_PRINTF1(_L("TestUntrustedAccessToCDSConfigL"));
	
	INFO_PRINTF1(_L("Getting the number of config parameters"));
	
	TUint numConfigParams = 0;
	TRAPD(err, numConfigParams = iCoreDumpSession.GetNumberConfigParametersL());
	if(KErrNone != err)
		{
		ERR_PRINTF2(_L("Unable to get the number of config parameters. Returned %d"), err);		
		SetBlockResult(EFail);
		return;
		}
	
	INFO_PRINTF2(_L("Found %d params"), numConfigParams);
	
	for (TInt i = 0; i < numConfigParams; i++)
		{
		COptionConfig* conf;
		conf = iCoreDumpSession.GetConfigParameterL(i);
		
		if (conf->Source()== COptionConfig::ESCMConfig)
			{
			ERR_PRINTF1(_L("Able to retrieve a config param for the SCM"));		
			SetBlockResult(EFail);
			delete conf;
			return;
			}
		
		delete conf;
		}
	
	INFO_PRINTF1(_L("Looked at all config parameters and none were SCM params so that is a pass"));
	}

void CCoreDumpMonitorWrapper::TestTraceBufferAccessL()
	{
	//Step 1: Load the UTRACE formatter and file writter
	INFO_PRINTF1(_L("CCoreDumpMonitorWrapper::TestTraceBufferAccessL loading UTRACE Formatter and File Writer"));

	TPluginRequest loadUtracePlugin;
	loadUtracePlugin.iPluginType = TPluginRequest::EFormatter;
	loadUtracePlugin.iLoad = ETrue;
	loadUtracePlugin.iUid = KUtraceUid;	
	
	TPluginRequest loadWriterReq;
	loadWriterReq.iPluginType = TPluginRequest::EWriter;
	loadWriterReq.iLoad = ETrue;
	loadWriterReq.iUid = KUidFileWriter;
	
	// loading Symbian ELF formatter
	TRAPD(err, iCoreDumpSession.PluginRequestL(loadUtracePlugin));
	if (err != KErrNone)
		{
		ERR_PRINTF2(_L("CCoreDumpMonitorWrapper::TestTraceBufferAccessL() Could not load UTRACE plugin!, err:%d\n"), err);
		SetBlockResult(EFail);
		User::Leave(err);
		}	
	
	// loading Symbian File writer
	TRAP(err, iCoreDumpSession.PluginRequestL(loadWriterReq));
	if (err != KErrNone)
		{
		ERR_PRINTF2(_L("CCoreDumpMonitorWrapper::TestTraceBufferAccessL() Could not load writer plugin!, err:%d\n"), err);
		SetBlockResult(EFail);
		User::Leave(err);
		}
	
	//Configuring the file writer plugin to have a specified crash file name
	TRAP(err, DoConfigureL((TInt)(CCrashDataSave::ECoreFilePath),
			KUidFileWriter.iUid, COptionConfig::EWriterPlugin,
			COptionConfig::ETFileName, KFilePathPrompt, 1, KNullDesC, 0,
			KCrashFileName, 0));

	if (err != KErrNone)
		{
		ERR_PRINTF2(_L("CCoreDumpMonitorWrapper::TestTraceBufferAccessL()() for File Writer Error %d changing param"),
				err);
		SetBlockResult(EFail);
		User::Leave(err);
		}	
	
	//List crashes in flash
	iCrashList.Reset();
	TRAP(err, iCoreDumpSession.ListCrashesInFlashL(iCrashList));
	if (err != KErrNone)
		{
		ERR_PRINTF2(_L("CCoreDumpMonitorWrapper::TestTraceBufferAccessL() Could not read Crashes in the Flash! err:%d\n"), err);
		SetBlockResult(EFail);
		User::Leave(err);
		}	
	
	INFO_PRINTF2(_L("Number of crashes present: %d"),	iCrashList.Count());
	if(iCrashList.Count() == 0)
		{
		ERR_PRINTF1(_L("Failed to find any crashes in log for the analysing"));
		SetBlockResult(EFail);
		User::Leave(err);
		}
	
	//Step 2: Process the crash
	for (TInt i = 0; i < iCrashList.Count(); i++)
		{
		TRAP(err, iCoreDumpSession.ProcessCrashLogL(iCrashList[i]->iCrashId));
		if (err != KErrNone)
			{
			ERR_PRINTF2(_L("CCoreDumpMonitorWrapper::TestTraceBufferAccessL() Could not process crash! err:%d\n"), err);
			SetBlockResult(EFail);
			User::Leave(err);
			}
		}
	
	INFO_PRINTF1(_L("Crashes Processed, Now looking for file generated"));
	
	//Step 3: Check the trace file has been generated
	//Use wildcard utilities to search for the UTRACE FIle Created
	TBuf<32> buf;
	buf = KDir;
	TUidType uid1;
	TUidType uid2(uid1.MostDerived());

	CFindFileByType* obj = new(ELeave) CFindFileByType(iFsSession);
	CleanupStack::PushL(obj);

	err = obj->FindFirst(KCrashWildCard, buf, uid2);
	if (err != KErrNone)
		{
		ERR_PRINTF2(
				_L("CCoreDumpMonitorWrapper::TestTraceBufferAccessL() Error finding the UTRACE File %d"),
				err);
		SetBlockResult(EFail);
		User::Leave(err);
		}
	
	INFO_PRINTF2(_L("The UTRACE File Found is E:\\:%S "), &(obj->Entry().iName));	
	
	CleanupStack::PopAndDestroy(); //CFindFileByType
	
	
	//Future improvements: Check contents of file
	}

/**
 * Tests that that after deletion of the entire partition  
 * SCM config values should correspond to the default build time values
 */
void CCoreDumpMonitorWrapper::TestReadDefaultConfigValuesL()
	{
	INFO_PRINTF1(_L("CCoreDumpMonitorWrapper::TestReadDefaultConfigValuesL Match with Build time SCM config values"));
	
	TInt noConfigParams = 0;
	TInt scmConfigParams = 0;
	// this will ultimateley read the config
    TRAPD(err, noConfigParams = iCoreDumpSession.GetNumberConfigParametersL());
    if(err != KErrNone)
        {
        ERR_PRINTF1(_L("Could not read number of config params"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);       
        }
    else
    	{	
    	INFO_PRINTF2(_L("Found %d config params"), noConfigParams);
    	
		for (TInt i = 0; i < noConfigParams; i++)
			{
			COptionConfig* conf = iCoreDumpSession.GetConfigParameterL(i);
			CleanupStack::PushL(conf);
			
			if (conf->Source()== COptionConfig::ESCMConfig)
				{
				//count number of SCM configurations
				scmConfigParams++;
				TConfigItem::TSCMDataType dataType = (TConfigItem::TSCMDataType) conf->Instance();

				//Default SCM Configuration for TConfigItem::EThreadsUsrStack has priority = 0
				if(dataType == TConfigItem::EThreadsUsrStack && conf->Value() != 0)  
					{
					ERR_PRINTF3(_L("TConfigItem::EThreadsUsrStack did not match the ZERO Priority for config data type %d with priority %d"), dataType, conf->Value());
					SetBlockResult(EFail);
					User::Leave(KErrGeneral);
					}
				//Default SCM Configuration for TConfigItem::EThreadsSvrStack has priority = 0
				else if(dataType == TConfigItem::EThreadsSvrStack && conf->Value() != 0)
					{
					ERR_PRINTF3(_L("TConfigItem::EThreadsSvrStack did not match the ZERO Priority for config data type %d with priority %d"), dataType, conf->Value());
					SetBlockResult(EFail);
					User::Leave(KErrGeneral);
					}
				else if (dataType == TConfigItem::EThreadsSvrStack || dataType == TConfigItem::EThreadsUsrStack)
						{
						INFO_PRINTF3(_L("VERIFIED ZERO priority for config data type %d priority %d"), dataType, conf->Value());
						CleanupStack::PopAndDestroy(conf);
						continue;
						}
 
				//Default SCM Configuration for all has a priority value equal to thier postion in the list + 1
				if(conf->Value() != (dataType + 1) )
					{
					ERR_PRINTF3(_L("Did not match the DEFAULT Configurtion for config data type %d with priority %d"), dataType, conf->Value());
					SetBlockResult(EFail);
					User::Leave(KErrGeneral);			
					}
				
				//happy to print the value
				INFO_PRINTF3(_L("VERIFIED config data type %d has priority %d"), dataType, conf->Value());
					
				}
		
			CleanupStack::PopAndDestroy(conf);//COptionConfig
			}//for loop for noConfigParams
		
		INFO_PRINTF2(_L("Match with DEFAULT config values Successful for %d SCM Configuration Parameters"),scmConfigParams);
    	}//else for no errors GetNumberConfigParametersL
	
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
void CCoreDumpMonitorWrapper::DoConfigureL(const TUint32& aIndex,
		const TUint32& aUID, const COptionConfig::TParameterSource& aSource,
		const COptionConfig::TOptionType& aType, const TDesC& aPrompt,
		const TUint32& aNumOptions, const TDesC& aOptions, const TInt32& aVal,
		const TDesC& aStrValue, const TUint aInstance)
	{
	COptionConfig * config;

	config = COptionConfig::NewL(aIndex, aUID, aSource, aType, aPrompt,
			aNumOptions, aOptions, aVal, aStrValue);

	CleanupStack::PushL(config);

	config->Instance(aInstance);

	//Configure now...
	iCoreDumpSession.SetConfigParameterL(*config);

	CleanupStack::PopAndDestroy(config);
	}


void CCoreDumpMonitorWrapper::TestValidateNewestCrashL()
	{
	
	INFO_PRINTF1(_L("TestValidateNewestCrashL called"));
	
	//Do list all the Crash Information here
	TRAPD(err, iCoreDumpSession.ListCrashesInFlashL(iCrashList));
	if (err != KErrNone)
		{
		ERR_PRINTF2(
				_L("TestValidateNewestCrashL Could not read Crashes in the Flash! err:%d\n"),
				err);
		SetBlockResult(EFail);
		return;
		//User::Leave(err);
		}

	INFO_PRINTF2(_L("TestValidateNewestCrashL Number of crash information present: %d"),
			iCrashList.Count());

	TInt expectedNewestIndex = 0;
	
	if( iCrashList.Count() > 0)
		{
		// must be entries for test to potentially fail	
		TInt newestIndex = 0;
		// start with first entry being newest time
		TUint64 newestTime = iCrashList[0]->iTime;
		
		for (TInt i = 0; i < iCrashList.Count(); i++)
			{		
			INFO_PRINTF3(_L("Crash no: %d, CrashId %d"), i, iCrashList[i]->iCrashId);
			INFO_PRINTF3(_L("Crash no: %d, Type %ld"), i, iCrashList[i]->iType);
			INFO_PRINTF3(_L("Crash no: %d, Time %ld"), i, iCrashList[i]->iTime);
			INFO_PRINTF3(_L("Crash no: %d, Thread ID %ld"), i, iCrashList[i]->iTid);
			
			// crash time held in the header as TUint64 - highest value should be the newset crash
			if(iCrashList[i]->iTime > newestTime)
				{
				newestIndex = i;
				newestTime = iCrashList[i]->iTime;
				}	
			}
			
		INFO_PRINTF2(_L("Newest crash is Crash no: %d"), newestIndex);
		if(newestIndex != expectedNewestIndex)
			{
			ERR_PRINTF3(_L("DoCmdValidateNewestCrashL newest crash was not in expected position expected (%d) actual(%d)")
					, expectedNewestIndex, newestIndex);
			SetBlockResult(EFail);
			//User::Leave(err);
			}			
		}
	
	INFO_PRINTF1(_L("TestValidateNewestCrashL finished"));
	
	}

//eof
