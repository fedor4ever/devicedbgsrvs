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
 @file t_process_crash.cpp
 @internalTechnology
*/

#include <bautils.h>

#include "t_process_crash.h"

using namespace Debug;

//Names of functions to be tested - referenced from script file
_LIT(KProcessCrashNewL, "NewL");
_LIT(KReadCrashInfo, "ReadCrashInfo");
_LIT(KDeleteAllCrash, "DeleteAllCrashes");
_LIT(KLoadPlugins, "LoadPlugins");
_LIT(KUnLoadPlugins, "UnLoadPlugins");
_LIT(KConfigurePlugins, "ConfigurePlugins");
_LIT(KConfigureSCMZeroPriority, "ConfigureSCMZeroPriority");
_LIT(KCheckMultipleCrashes, "CheckMultipleCrashes");
_LIT(KValidateSingleELFFile, "ValidateSingleELFFile");
_LIT(KValidateMultipleSELFFile, "ValidateMultipleSELFFile");
_LIT(KValidateSelfCPUId, "ValidateSelfCPUID");
_LIT(KValidateHeapSELFFile, "ValidateHeapSELFFile");
_LIT(KValidateTraceSELFFile, "ValidateTraceSELFFile");
_LIT(KPerformanceMeasureSELFFile, "PerformanceMeasureSELFFile");
_LIT(KValidateCorruptCrash, "ValidateCorruptCrash");
_LIT(KValidateAsyncProcessCrashLog, "ValidateAsyncProcessCrashLog");
_LIT(KValidateAsyncProcessAndLiveCrash, "ValidateAsyncProcessAndLiveCrash");

//list of exception to check for
enum TCheckExecption
	{
	ETestDataAbort = 24
	};

//Max String Read out from the SELF file String scetion
const TInt KMaxStringLength = 200;

//Minimum amount of crashes we need for multi crash tests
const TInt KMinimumAcceptableCrashNumber = 2;

// crash type
enum TCrashType
	{ECrashException, ECrashKill};

/**
 * Constructor for test wrapper
 */
CProcessCrashWrapper::CProcessCrashWrapper()
	{
	}

/**
 * Destructor
 */
CProcessCrashWrapper::~CProcessCrashWrapper()
	{
	iCrashList.ResetAndDestroy();
	iCoreDumpSession.Disconnect();
	iFsSession.Close();
	iThreadCrashed.Close();
	}

/**
 * Two phase constructor for CProcessCrashWrapper
 * @return CProcessCrashWrapper object
 * @leave
 */
CProcessCrashWrapper* CProcessCrashWrapper::NewL()
	{
	CProcessCrashWrapper* ret = new (ELeave) CProcessCrashWrapper();
	CleanupStack::PushL(ret);
	ret->ConstructL();
	CleanupStack::Pop(ret);
	return ret;
	}

/**
 * Safe construction
 * @leave
 */
void CProcessCrashWrapper::ConstructL()
	{
	User::LeaveIfError(iFsSession.Connect());
	iFsSession.SetSessionPath(KDir);
	}

/**
 * Assign the object
 *
 * @param aObject TAny* to the object to test
 * @leave
 */
void CProcessCrashWrapper::SetObjectL(TAny* aObject)
	{}

/**
 * Runs a test preamble
 */
void CProcessCrashWrapper::PrepareTestL()
	{
	SetBlockResult(EPass);

	//Delete any crash files from previous tests
	TBuf<KMaxFileName> name;
	name.Append(KDir);
	name.Append(KCrashWildCard);
	BaflUtils::DeleteFile(iFsSession, name);
	}

/**
 * Handle a command invoked from the script
 *
 * @param aCommand Is the name of the command
 *   for a TBuf
 * @param aSection Is the .ini file section where parameters to the command
 *   are located
 * @param aAsyncErrorIndex Is used by the TEF Block framework to handle
 *   asynchronous commands.
 */

TBool CProcessCrashWrapper::DoCommandL(const TTEFFunction& aCommand,
		const TTEFSectionName& aSection, const TInt aAsyncErrorIndex)
	{

	//__UHEAP_MARK;

	PrepareTestL();

	if (KProcessCrashNewL() == aCommand)
		{
		DoCmdNewL();
		}
	else if (KReadCrashInfo() == aCommand)
		{
		DoCmdReadCrashInfoL();
		}
	else if (KCheckMultipleCrashes() == aCommand)
		{
		DoCmdCheckMultipleCrashesL();
		}
	else if (KDeleteAllCrash() == aCommand)
		{
		DoCmdDeleteAllCrashL();
		}
	else if (KLoadPlugins() == aCommand)
		{
		DoCmdLoadPluginsL();
		}
	else if (KUnLoadPlugins() == aCommand)
		{
		DoCmdUnLoadPluginsL();
		}
	else if (KConfigurePlugins() == aCommand)
		{
		DoCmdConfigurePluginsL(KCrashFileName);
		}
	else if (KConfigureSCMZeroPriority() == aCommand)
		{
		DoCmdConfigureSCMZeroPriorityL();
		}
	else if (KValidateSingleELFFile() == aCommand)
		{
		DoCmdValidateSingleELFFileL();
		}
	else if (KValidateMultipleSELFFile() == aCommand)
		{
		DoCmdValidateMultipleELFFileL();
		}
	else if (KValidateSelfCPUId() == aCommand)
		{
		DoCmdValidateCpuIDL();
		}
	else if (KValidateHeapSELFFile() == aCommand)
		{
		DoCmdValidateHeapSELFFileL();
		}
	else if (KValidateTraceSELFFile() == aCommand)
		{
		DoCmdValidateTraceSELFFileL();
		}
	else if (KPerformanceMeasureSELFFile() == aCommand)
		{
		DoCmdPerformanceMeasureSELFFileL();
		}
	else if (KValidateCorruptCrash() == aCommand)
		{
		DoCmdValidateCorruptCrashL();
		}
	else if (KValidateAsyncProcessCrashLog() == aCommand)
		{
		DoCmdValidateAsyncProcessCrashLogL();
		}
	else if (KValidateAsyncProcessAndLiveCrash() == aCommand)
		{
		DoCmdValidateAsyncProcessAndLiveCrashCrashLogL(aSection);
		}
	else
		{
		INFO_PRINTF1(_L("NOT ** Inside Command"));
		return EFalse;
		}

	//__UHEAP_MARKEND;

	return ETrue;
	}

/**
 * The initailization function wherein the sessions to Core Dump Server is established
 */
void CProcessCrashWrapper::DoCmdNewL()
	{
	INFO_PRINTF1(_L("CProcessCrashWrapper::DoCmdNewL"));

	TInt err = iCoreDumpSession.Connect();
	if (err != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::DoCmdNewL: iCoreDumpSession->Connect() failed!, err:%d\n"),
				err);
		SetBlockResult(EFail);
		User::Leave(err);
		}

	}

/**
 * Reads all the crash information present in the flash
 */
void CProcessCrashWrapper::DoCmdReadCrashInfoL()
	{
	INFO_PRINTF1(_L("CProcessCrashWrapper:: DoCmdReadCrashInfoL\n"));

	//Do list all the Crash Information here
	TRAPD(err, iCoreDumpSession.ListCrashesInFlashL(iCrashList));
	if (err != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::DoCmdReadCrashInfoL: Could not read Crashes in the Flash! err:%d\n"),
				err);
		SetBlockResult(EFail);
		User::Leave(err);
		}

	INFO_PRINTF2(_L("Number of crash information present: %d"),
			iCrashList.Count());

	for (TInt i = 0; i < iCrashList.Count(); i++)
		{
		INFO_PRINTF3(_L("Crash no: %d, CrashId %d"), i, iCrashList[i]->iCrashId);
		INFO_PRINTF3(_L("Crash no: %d, Type %ld"), i, iCrashList[i]->iType);
		INFO_PRINTF3(_L("Crash no: %d, Time %ld"), i, iCrashList[i]->iTime);
		INFO_PRINTF3(_L("Crash no: %d, Thread ID %ld"), i, iCrashList[i]->iTid);
		}
	}

/**
 * Test case specific to DT-coredump-monitor-010
 * Reads all the crash information present
 * Checks for exactly four crashed present
 * Process the crash without any plugins
 */
void CProcessCrashWrapper::DoCmdCheckMultipleCrashesL()
	{
	INFO_PRINTF1(_L("CProcessCrashWrapper:: DoCmdCheckMultipleCrashesL"));

	//reads all the crash information present in the flash
	DoCmdReadCrashInfoL();

	//check for exactly four crashes present in the flash
	if (iCrashList.Count() < KMinimumAcceptableCrashNumber)
		{
		ERR_PRINTF1(_L("CProcessCrashWrapper::DoCmdCheckMultipleCrashesL Could not find multiple crashes"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}

	INFO_PRINTF1(_L("Verified that there are multiple crashes present"));
	INFO_PRINTF1(_L("CProcessCrashWrapper:: Procesing the crashes!!!"));

	for (TInt i = 0; i < iCrashList.Count(); i++)
		{
		//process teh crash log for each of the crashes present
		TRAPD(error, iCoreDumpSession.ProcessCrashLogL(iCrashList[i]->iCrashId));
		if (error != KErrNone)
			{
			ERR_PRINTF2(
					_L("CProcessCrashWrapper::ProcessCrashLogL: Could not process crash! err:%d\n"),
					error);
			SetBlockResult(EFail);
			User::Leave(error);
			}
		}
	}

/**
 * Deletes all crashes in the flash
 * Always called after Reading all crash information present
 * by CProcessCrashWrapper::DoCmdReadCrashInfoL()
 */
void CProcessCrashWrapper::DoCmdDeleteAllCrashL()
	{
	INFO_PRINTF1(_L("CProcessCrashWrapper:: DoCmdDeleteAllCrashL deleting all the crashes"));

	TRAPD(err, iCoreDumpSession.DeleteCrashPartitionL());
	if (err != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::DoCmdDeleteAllCrashL: Could not delete crash! err:%d\n"),
				err);
		SetBlockResult(EFail);
		User::Leave(err);
		}

	INFO_PRINTF1(_L("Successfully deleted all the crashes"));
	}

/**
 * Loads the SELF Formatter and the File Writer plugin
 */
void CProcessCrashWrapper::DoCmdLoadPluginsL()
	{
	INFO_PRINTF1(_L("CProcessCrashWrapperloading SELF Formatter and File Writer!!!"));

	TPluginRequest loadSELFReq;
	loadSELFReq.iPluginType = TPluginRequest::EFormatter;
	loadSELFReq.iLoad = ETrue;
	loadSELFReq.iUid = KUidELFFormatterV2;

	TPluginRequest loadWriterReq;
	loadWriterReq.iPluginType = TPluginRequest::EWriter;
	loadWriterReq.iLoad = ETrue;
	loadWriterReq.iUid = KUidFileWriter;

	// loading Symbian ELF formatter
	TRAPD(ret, iCoreDumpSession.PluginRequestL(loadSELFReq));
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::DoCmdLoadPluginsL: Could not load SELF plugin!, err:%d\n"),
				ret);
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	// loading Symbian File writer
	TRAP(ret, iCoreDumpSession.PluginRequestL(loadWriterReq));
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::DoCmdLoadPluginsL: Could not load writer plugin!, err:%d\n"),
				ret);
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	//intend to check for the loaded SELF formatter and File Writer
	RPluginList pluginLists;
	CleanupClosePushL(pluginLists);

	TRAP(ret, iCoreDumpSession.GetPluginListL(pluginLists));

	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::DoCmdLoadPluginsL: Could not get plugin list!, err:%d\n"),
				ret);
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	//Now we look through the list, until we find our plugin, its a fail

	for (TInt i = 0; i < pluginLists.Count(); i++)
		{
		if (pluginLists[i].iUid == KUidELFFormatterV2 && !(pluginLists[i].iLoaded))
			{
			ERR_PRINTF1(
					_L("SELF Plugin Not loaded !\n"));
			SetBlockResult(EFail);
			User::Leave(KErrNotFound);
			}
		}

	for (TInt i = 0; i < pluginLists.Count(); i++)
		{
		if (pluginLists[i].iUid == KUidFileWriter && !(pluginLists[i].iLoaded))
			{
			ERR_PRINTF1(
								_L("Writer Plugin Not loaded !\n"));
			SetBlockResult(EFail);
			User::Leave(KErrNotFound);
			}
		}

	INFO_PRINTF1(_L("SELF Formatter and File Writer loaded successfully \n"));
	CleanupStack::PopAndDestroy();//pluginLists
	}

/**
 * UnLoads the SELF Formatter and the File Writer plugin
 */
void CProcessCrashWrapper::DoCmdUnLoadPluginsL()
	{
	INFO_PRINTF1(_L("CProcessCrashWrapper DoCmdUnLoadPluginsL unloading SELF Formatter and File Writer"));

	TPluginRequest unloadSELFReq;
	unloadSELFReq.iPluginType = TPluginRequest::EFormatter;
	unloadSELFReq.iLoad = EFalse;
	unloadSELFReq.iUid = KUidELFFormatterV2;

	TPluginRequest unloadWriterReq;
	unloadWriterReq.iPluginType = TPluginRequest::EWriter;
	unloadWriterReq.iLoad = EFalse;
	unloadWriterReq.iUid = KUidFileWriter;

	TRAPD(ret, iCoreDumpSession.PluginRequestL(unloadSELFReq));
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::DoCmdLoadPluginsL: Could not unload SELF plugin!, err:%d\n"),
				ret);
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	TRAPD(err, iCoreDumpSession.PluginRequestL(unloadWriterReq));
	if (err != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::DoCmdLoadPluginsL: Could not unload writer plugin!, err:%d\n"),
				err);
		SetBlockResult(EFail);
		User::Leave(err);
		}

	INFO_PRINTF1(_L("Successful unloading SELF Formatter and File Writer"));
	}

/**
 * Configuring the SCM to have minimal information the flash
 * Done inorder to incorporate more crashes in the flash
 */
void CProcessCrashWrapper::DoCmdConfigureSCMZeroPriorityL()
	{
	INFO_PRINTF1(_L("CProcessCrashWrapper::DoCmdConfigureSCMZeroPriority setting all SCM configurations to zero priority"));

	TInt noConfigParams = 0;
	// this will ultimateley read the config
    TRAPD(err, noConfigParams = iCoreDumpSession.GetNumberConfigParametersL());
    if(err != KErrNone)
        {
        ERR_PRINTF1(_L("COULD not read number of config params"));
		User::Leave(KErrGeneral);
		SetBlockResult(EFail);
        }
    else
    	{

    	INFO_PRINTF2(_L("CProcessCrashWrapper::DoCmdConfigureSCMZeroPriority found %d config params"), noConfigParams);
		for (TInt i = 0; i < noConfigParams; i++)
			{
			TInt priority = 1;//setting priority to one for all

			COptionConfig* conf = iCoreDumpSession.GetConfigParameterL(i);
			CleanupStack::PushL(conf);

			if (conf->Source()== COptionConfig::ESCMConfig)
				{
				TConfigItem::TSCMDataType dataType = (TConfigItem::TSCMDataType) conf->Instance();

//				INFO_PRINTF3(_L("CProcessCrashWrapper::DoCmdConfigureSCMZeroPriority found config data type %d priority %d")
//						, dataType, conf->Value());

				if(dataType == TConfigItem::EKernelHeap || dataType == TConfigItem::EThreadsUsrStack || dataType == TConfigItem::EThreadsSvrStack )
					{
					priority = 0;
					INFO_PRINTF3(_L("CProcessCrashWrapper::DoCmdConfigureSCMZeroPriority modifying data type %d with priority %d")
							, dataType, priority);
					}

				// set the value
				conf->Value(priority);
				TRAP(err, iCoreDumpSession.SetConfigParameterL(*conf));
				if (err != KErrNone)
					{
					ERR_PRINTF3(
							_L("CProcessCrashWrapper::DoCmdConfigureSCMZeroPriority for SCM Configurations failed error code is %d changing param for run no: %d"),
							err, i);
					SetBlockResult(EFail);
					User::Leave(err);
					}
				}

			CleanupStack::PopAndDestroy(conf);
			}

    	}

	INFO_PRINTF1(_L("SCM Configuration successful"));
	}

/**
 * Tests that the SELF plugin can handle invalid configuration
 */
void CProcessCrashWrapper::DoCmdConfigureSELFPluginWithInvalidnessL()
	{
	INFO_PRINTF1(_L("CProcessCrashWrapper::DoCmdConfigureSELFPluginWithInvalidnessL configuring the Core dump server and File Writer plugin"));

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
		CleanupStack::PushL(conf);

		if(conf->Source() == COptionConfig::EFormatterPlugin)
			{
			//Try to configure with the old invalidity
			switch(conf->Type())
				{
				}
			}

		CleanupStack::PopAndDestroy(conf);

		}

	}

/**
 * Configuring the Core Dump Server and the File Writer
 * @param aSELFFileName is the SELF File Name configuring the File Writer plugin
 */
void CProcessCrashWrapper::DoCmdConfigurePluginsL(const TDesC& aSELFFileName)
	{
	INFO_PRINTF1(_L("CProcessCrashWrapper DoCmdConfigurePlugins configuring the Core dump server and File Writer plugin"));

	//Configuring the Core Dump Server
	TRAPD(ret, DoConfigureL(2, KCDSUid.iUid, COptionConfig::ECoreDumpServer,
			COptionConfig::ETUInt, KPostCrashEventActionPrompt, 1, KNullDesC,
			4, KNullDesC, 0));
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::DoConfigureL for CDS Error %d changing param"),
				ret);
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	//Configuring the file writer plugin to have a specified crash file name
	TRAPD(err, DoConfigureL((TInt)(CCrashDataSave::ECoreFilePath),
			KUidFileWriter.iUid, COptionConfig::EWriterPlugin,
			COptionConfig::ETFileName, KFilePathPrompt, 1, KNullDesC, 0,
			aSELFFileName, 0));

	if (err != KErrNone)
		{
		ERR_PRINTF2(_L("CProcessCrashWrapper::DoConfigureL for File Writer Error %d changing param"),
				err);
		SetBlockResult(EFail);
		User::Leave(err);
		}

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
void CProcessCrashWrapper::DoConfigureL(const TUint32& aIndex,
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

void CProcessCrashWrapper::DoCmdValidateCpuIDL()
	{
	LOG_MSG(_L("CProcessCrashWrapper::DoCmdValidateCpuIDL()"));

	//expects one crash information to be present in the flash otherwise complain
	if (iCrashList.Count() != 1)
		{
		ERR_PRINTF1(_L("More than one crash information present"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}

	GenerateElfFileL(iCrashList[0]->iCrashId);

	//open the SELF File for processing
	TInt err = iSELFFile.Open(iFsSession, iSELFFileName, EFileStream|EFileRead);
	if (err != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::ProcessELFFile Error opening the SELF File %d"),
				err);
		SetBlockResult(EFail);
		User::Leave(err);
		}

	//Parse start of ELF file
	ValidateHeaderELFFileL();
	ValidateProgramHeaderELFFileL();
	ValidateSymInfoSectionELFFileL();

	//Validate the CPU Id
	RDebug::Printf("Looking at CPU ID");
	ValidateThreadInfoSectionELFFileL(ETrue, KThreadKernelCrash, ETrue);

	TBuf<KMaxFileName> fileName;
	iSELFFile.Name(fileName);

	INFO_PRINTF2(_L("Finished with %S So im sharking it"), &fileName);
	iSELFFile.Close();
	iFsSession.Delete(fileName);
	}
/**
 * Generate the SELF File and search for the generated file
 * aCrashId  = crash id
 * aTiming (default value is False) determines the timing measurement
 */
void CProcessCrashWrapper::GenerateElfFileL(TUint aCrashId, TBool aTiming)
	{
	if(aTiming) HelpStartTestTimer();

	//process the crash with configured SELF formatter and file writer
	TRAPD(ret, iCoreDumpSession.ProcessCrashLogL(aCrashId));
	if (ret != KErrNone)
		{
		ERR_PRINTF2(_L("CProcessCrashWrapper::ProcessCrashLogL for CDS Error %d"),
				ret);
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	MonitorProgressL();

	if(aTiming) HelpStopTestTimer();

	INFO_PRINTF1(_L("ProcessCrashLogL Successful"));

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
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	iSELFFileName = obj->Entry().iName;

	CleanupStack::PopAndDestroy(); //CFindFileByType
	INFO_PRINTF2(_L("The SELF File Found is E:\\%S "), &iSELFFileName);
	}

/**
 * Generate the SELF File and Validate the Symbian INFO and executable sections
 */
void CProcessCrashWrapper::DoCmdValidateSingleELFFileL()
	{
	INFO_PRINTF1(_L("CProcessCrashWrapper DoCmdSingleValidateELFFileL"));

	//expects one crash information to be present in the flash otherwise complain
	if (iCrashList.Count() != 1)
		{
		ERR_PRINTF1(_L("More than one crash information present"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}

	//generate the SELF File by calling ProcessCrashLogL
	GenerateElfFileL(iCrashList[0]->iCrashId);

	//open the SELF File for processing
	TInt err = iSELFFile.Open(iFsSession, iSELFFileName, EFileStream|EFileRead);
	if (err != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::ProcessELFFile Error opening the SELF File %d"),
				err);
		SetBlockResult(EFail);
		User::Leave(err);
		}

	//validate the SELF Header Format
	ValidateHeaderELFFileL();

	//validate the Program Header and store information about the string section Sections
	ValidateProgramHeaderELFFileL();

	//validate the Symbian Info section
	ValidateSymInfoSectionELFFileL();

	//validate Thread name that crashed
	ValidateThreadInfoSectionELFFileL(EFalse, KThreadKernelCrash, ETrue);

	//do the necessary clean up
	CleanupMethod(iSELFFileName);
	}

/**
 * Process the crash log using the asynchrnous API ProcessCrashLog
 */
void CProcessCrashWrapper::DoCmdValidateAsyncProcessCrashLogL()
	{

	INFO_PRINTF1(_L("CProcessCrashWrapper DoCmdValidateAsyncProcessCrashLogL"));

	//expects one crash information to be present in the flash otherwise complain
	if (iCrashList.Count() != 1)
		{
		ERR_PRINTF1(_L("More than one crash information present"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}

	//create the Active Object to process the crash log asynchrnously
	CAsyncProcessCrash* asyncProcessCrash = CAsyncProcessCrash::NewL(this);
	CleanupStack::PushL(asyncProcessCrash);

	asyncProcessCrash->IssueProcessCrashRequest(iCrashList[0]->iCrashId);

	CActiveScheduler::Start();// Doesn’t return until it is explicitly

	RDebug::Printf("After CActiveScheduler::Start");
	CleanupStack::PopAndDestroy();
	INFO_PRINTF1(_L("DoCmdValidateAsyncProcessCrashLogL Successful with processing the crash log asynchrnouly"));

	}

/**
 * Process the crash log using the asynchrnous API ProcessCrashLog whilst a user side crash
 */
void CProcessCrashWrapper::DoCmdValidateAsyncProcessAndLiveCrashCrashLogL(const TDesC& aSection)
	{

	INFO_PRINTF1(_L("CProcessCrashWrapper DoCmdValidateAsyncProcessAndLiveCrashCrashLogL"));

	//expects one crash information to be present in the flash otherwise complain
	if (iCrashList.Count() != 1)
		{
		ERR_PRINTF1(_L("More than one crash information present"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}

	//generate a live crash using the test application crashapp.exe
	GenerateLiveCrashL(aSection);

	//start processing the crash log in the flash
	TRequestStatus asyncSystemCrash;
    iCoreDumpSession.ProcessCrashLog( (iCrashList[0]->iCrashId), asyncSystemCrash);
	User::WaitForRequest(asyncSystemCrash);

	//This user after is to ensure the live crash has completed before analysis
    User::After(3000000);

    //process the two SELF files created
    DoProcessSELFLiveandKernelL();

	}

/**
 * Generates a user side crash using the crashapp
 */
void CProcessCrashWrapper::GenerateLiveCrashL(const TDesC& aSection)
	{

	TPtrC crashAppParam;
	    //read from the INI for crashapp paramemters
	if(!GetStringFromConfig(aSection, KTe_CrashAppParam, crashAppParam))
		{
		ERR_PRINTF1(_L("Failed to get data from ini file"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
	    }

	INFO_PRINTF2(_L("The CrashApp Parameters are: crashapp %S, "), &crashAppParam); // print it to check
	//Start the process that we intend to crash....
	RProcess crashProcess;
	CleanupClosePushL(crashProcess);

	TInt ret = crashProcess.Create( KCrashAppFileName, crashAppParam);
	if(ret != KErrNone)
		{
		INFO_PRINTF2(_L("Error %d from RProcess .Create(z:\\sys\\bin\\crashapp.exe)/n"), ret);
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	INFO_PRINTF1(_L("Started userside crash app"));

	// Observe the process and wait for a crash
	TRAP(ret, iCoreDumpSession.ObservationRequestL( KCrashAppFileName, KCrashAppFileName, ETrue) );
	if(ret != KErrNone)
		{
		INFO_PRINTF2(_L("Error %d iCoreDumpSession.ObservationRequestL(z:\\sys\\bin\\crashapp.exe)\n"), ret);
		SetBlockResult(EFail);
		User::Leave( ret );
		}

	//start the crash process which should go ahead a create a crash
	crashProcess.Resume();
	CleanupStack::PopAndDestroy(&crashProcess); //this is for crashProcess

	}

/**
 * Complete the processing the SELF file generated after the async ProcessCrashLog
 * called from RunL
 */
void CProcessCrashWrapper::ProcessSELFFileCreatedL()
	{

	INFO_PRINTF1(_L("CProcessCrashWrapper ProcessSELFFileCreatedL called after RunL"));

	//Use wildcard utilities to search for the SELF FIle Created
	TBuf<32> buf;
	buf = KDir;
	TUidType uid1;
	TUidType uid2(uid1.MostDerived());

	CFindFileByType* obj = new(ELeave) CFindFileByType(iFsSession);
	CleanupStack::PushL(obj);

	TInt ret = obj->FindFirst(KCrashWildCard, buf, uid2);
	if (ret != KErrNone)
		{
		ERR_PRINTF2(_L("GenerateElfFileL Error finding the SELF File %d"),
				ret);
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	iSELFFileName = obj->Entry().iName;
	CleanupStack::PopAndDestroy();//CFindFileByType* obj

	INFO_PRINTF2(_L("The SELF File Found is E:\\%S "), &iSELFFileName);

	//open the SELF File for processing
	TInt err = iSELFFile.Open(iFsSession, iSELFFileName, EFileStream|EFileRead);
	if (err != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::ProcessSELFFileCreatedL Error opening the SELF File %d"),
				err);
		SetBlockResult(EFail);
		User::Leave(err);
		}

	//validate the SELF Header Format
	ValidateHeaderELFFileL();

	//validate the Program Header and store information about the string section Sections
	ValidateProgramHeaderELFFileL();

	//validate the Symbian Info section
	ValidateSymInfoSectionELFFileL();

	//validate Thread name that crashed
	ValidateThreadInfoSectionELFFileL(EFalse, KThreadKernelCrash, ETrue);

	//do the necessary clean up
	CleanupMethod(iSELFFileName);

	}

/**
 * Process the two SELF files created for user and kernel side crashes
 */
void CProcessCrashWrapper::DoProcessSELFLiveandKernelL()
	{
	INFO_PRINTF1(_L("CProcessCrashWrapper DoProcessSELFLiveandKernel about process user and kernel crashes"));

	TBool systemcrash = EFalse;

	//Use wildcard utilities to search for the two SELF File Created
	TBuf<32> buf;
	buf = KDir;
	TUidType uid1;
	TUidType uid2(uid1.MostDerived());

	CFindFileByType* obj = new(ELeave) CFindFileByType(iFsSession);
	CleanupStack::PushL(obj);

	TInt ret = obj->FindFirst(KCrashWildCard, buf, uid2);
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("DoProcessSELFLiveandKernelL Error finding the SELF File %d"),
				ret);
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	iSELFFileName = obj->Entry().iName;
	INFO_PRINTF2(_L("The SELF File Found is E:\\%S "), &iSELFFileName);

    ret = iSELFFile.Open(iFsSession, iSELFFileName, EFileStream|EFileRead);
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("DoProcessSELFLiveandKernelL Error opening the SELF File %d"),
				ret);
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	//validate the SELF Header Format
	ValidateHeaderELFFileL();

	//validate the Program Header and store information about the string section Sections
	ValidateProgramHeaderELFFileL();

	//validate the Symbian Info section
	ValidateSymInfoSectionELFFileL();

	//validate the Register Info Section also verifies the CPSR to check on the type of the crash
	ValidateRegisterInfoSectionELFFileL();

	if(iSystemCrash)//if it is a System Crash
		{
		//checking this SELF file for the kernel crash
		ValidateThreadInfoSectionELFFileL(EFalse, KThreadKernelCrash, ETrue);
		}
	else
		{
		//checking this one for user crash
		ValidateThreadInfoSectionELFFileL(EFalse, KThreadUserCrash, ETrue);
		}

	systemcrash = iSystemCrash;
	//do the necessary clean up
	CleanupMethod(iSELFFileName);

	//Find the next SELF file
	ret = obj->FindNext();

	if (ret != KErrNone)
		{
		ERR_PRINTF1(
				_L("DoProcessSELFLiveandKernelL Expecting Two SELF files to be created"));
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	iSELFFileName = obj->Entry().iName;
	INFO_PRINTF2(_L("The SELF File Found is E:\\%S "), &iSELFFileName);

	CleanupStack::PopAndDestroy(); //CFindFileByType

	//open the SELF File for processing
	ret = iSELFFile.Open(iFsSession, iSELFFileName, EFileStream|EFileRead);
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("DoProcessSELFLiveandKernelL Error opening the SELF File %d"),
				ret);
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	//validate the SELF Header Format
	ValidateHeaderELFFileL();

	//validate the Program Header and store information about the string section Sections
	ValidateProgramHeaderELFFileL();

	//validate the Symbian Info section
	ValidateSymInfoSectionELFFileL();

	//validate the Register Info Section also verifies the CPSR to check on the type of the crash
	ValidateRegisterInfoSectionELFFileL();

	if(iSystemCrash == systemcrash)
		{
		ERR_PRINTF1( _L("Expecting to have two different types of crashes one User side and the other one as System Crash"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}

	if(iSystemCrash)//if it is a System Crash
		{
		//checking this SELF file for the kernel crash
		ValidateThreadInfoSectionELFFileL(EFalse, KThreadKernelCrash, ETrue);
		}
	else
		{
		//checking this one for user crash
		ValidateThreadInfoSectionELFFileL(EFalse, KThreadUserCrash, ETrue);
		}

	//do the necessary clean up
	CleanupMethod(iSELFFileName);

	}
/**
 * Generate multiple SELF File and Validate the Symbian INFO and executable sections
 */
void CProcessCrashWrapper::DoCmdValidateMultipleELFFileL()
	{
	INFO_PRINTF1(_L("CProcessCrashWrapper::DoCmdValidateMultipleELFFileL parsing multiple SELF files"));

	//expects MULTIPLECRASHNUMBERCHECK to be present in the flash otherwise complain
	if (iCrashList.Count() < KMinimumAcceptableCrashNumber)
		{
		ERR_PRINTF1(_L("Did not find multiple crashes"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}

	for (TInt i = 0; i < iCrashList.Count(); i++)
		{
		//generate the SELF File(s) by calling ProcessCrashLogL
		GenerateElfFileL(iCrashList[i]->iCrashId);

		TInt err = iSELFFile.Open(iFsSession, iSELFFileName, EFileStream|EFileRead);
		if (err != KErrNone)
			{
			ERR_PRINTF2(
					_L("CProcessCrashWrapper::ProcessELFFile Error opening the SELF File %d"),
					err);
			SetBlockResult(EFail);
			User::Leave(err);
			}

		//validate the SELF Header Format
		ValidateHeaderELFFileL();

		//validate the Program Header and store information about the string section Sections
		ValidateProgramHeaderELFFileL();

		//validate the Symbian Info section
		ValidateSymInfoSectionELFFileL();

		//validate Thread name that crashed
		ValidateThreadInfoSectionELFFileL(EFalse, KThreadKernelCrash, ETrue);

		//do the necessary clean up
		CleanupMethod(iSELFFileName);

		}

/*
 * code to check on multiple files in the directory and storing them
 * commented for future reference
 * RBuf selfname;
	selfname.CreateL(obj->Entry().iName);
	selfname.SetMax();

	TBufC<100> selfnamebuffer = obj->Entry().iName;
	HBufC* selfname = selfnamebuffer.AllocL();

	iArray.Append(selfname);

	INFO_PRINTF2(_L("The SELF File Found is E:\\%S "), &iArray[0]);

	for (TInt i = 0; i < iCrashList.Count() - 1; i++)
		{

		ret = obj->FindNext();
		if (ret != KErrNone)
			{
			ERR_PRINTF2(
					_L("CProcessCrashWrapper::CFindFileByType Error finding the SELF File %d"),
					ret);
			SetBlockResult(EFail);
			User::Leave(ret);
			}

		RBuf selfname2;
		selfname2.CreateL(obj->Entry().iName);
		selfname2.SetMax();

		TBufC<100> selfnamebuffer2 = obj->Entry().iName;
		HBufC* selfname2 = selfnamebuffer2.AllocL();

		iArray.Append(selfname2);// no close

		INFO_PRINTF2(_L("The SELF File Found is E:\\%S "), &iArray[i]);
	*/

	}

/**
 * Validates the presence of the heap data in the SELF file
 */
void CProcessCrashWrapper::DoCmdValidateHeapSELFFileL()
	{
	INFO_PRINTF1(_L("\nDoCmdValidateHeapSELFFileL intend to validate the Heap Section of SELF File"));

	//expects one crash information to be present in the flash otherwise complain
	if (iCrashList.Count() != 1)
		{
		ERR_PRINTF1(_L("More than one crash information present"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}

	/*
	 * POSITIVE TEST CASE
	 * Test for the positive case let the Heap Section be present
	 */

	//configuring the SELF Formatter to have the date segment
    ConfigureSELF(ETrue, EHeap);

	//generate the SELF File by calling ProcessCrashLogL
	GenerateElfFileL(iCrashList[0]->iCrashId);

	//open the SELF File for processing
	TInt err = iSELFFile.Open(iFsSession, iSELFFileName, EFileStream|EFileRead);
	if (err != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::ProcessELFFile Error opening the SELF File %d"),
				err);
		SetBlockResult(EFail);
		User::Leave(err);
		}

	//validate the SELF Header Format
	ValidateHeaderELFFileL();

	//validate the Program Header and store information about the string section Sections
	ValidateProgramHeaderELFFileL();

	//validate the Symbian Info section
	ValidateSymInfoSectionELFFileL();

	//validate Thread name that crashed
	ValidateThreadInfoSectionELFFileL(EFalse, KThreadKernelCrash, ETrue);

	//validate the heap section to be present
	ValidateHeapSectionELFFileL(ETrue);

	//clean up of the generated SELF file
	iSELFFile.Close();
    TInt ret = iFsSession.Delete(iSELFFileName);
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::SELF File delete returned error %d"),
				ret);
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	INFO_PRINTF1(_L("Validation of the Positive Heap Test Case Successful!!!\n\n"));

	/*
	 * NEGATIVE TEST CASE
	 * Test for the negative test case let the Heap Section be NOT present
	 */

	//configuring the SELF NOT to have data segment/heap
	ConfigureSELF(EFalse, EHeap);

	//generate the SELF File by calling ProcessCrashLogL
	GenerateElfFileL(iCrashList[0]->iCrashId);

	//open the SELF File for processing
	err = iSELFFile.Open(iFsSession, iSELFFileName, EFileStream|EFileRead);
	if (err != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::ProcessELFFile Error opening the SELF File %d"),
				err);
		SetBlockResult(EFail);
		User::Leave(err);
		}

	//validate the SELF Header Format
	ValidateHeaderELFFileL();

	//validate the Program Header and store information about the string section Sections
	ValidateProgramHeaderELFFileL();

	//validate the Symbian Info section
	ValidateSymInfoSectionELFFileL();

	//validate Thread name that crashed
	ValidateThreadInfoSectionELFFileL(EFalse, KThreadKernelCrash, ETrue);

	//validate the heap section NOT to be present this time
	ValidateHeapSectionELFFileL(EFalse);

	//do necessary clean up
	CleanupMethod(iSELFFileName);

	INFO_PRINTF1(_L("Validation of the Negative Heap Test Case Successful!!!\n\n"));
	}

/**
 * Validates the presence of the trace data in the SELF file
 */
void CProcessCrashWrapper::DoCmdValidateTraceSELFFileL()
	{
	INFO_PRINTF1(_L("\nDoCmdValidateTraceSELFFileL intend to validate the Trace Section of SELF File"));

	//expects one crash information to be present in the flash otherwise complain
	if (iCrashList.Count() != 1)
		{
		ERR_PRINTF1(_L("More than one crash information present"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}

	/*
	 * POSITIVE TEST CASE
	 * Test for the positive case let the Trace Section be present
	 */

	//configuring the SELF Formatter to have the trace segment
    ConfigureSELF(ETrue, ETrace);

    //generate the SELF File by calling ProcessCrashLogL
    GenerateElfFileL(iCrashList[0]->iCrashId);

	//open the SELF File for processing
	TInt err = iSELFFile.Open(iFsSession, iSELFFileName, EFileStream|EFileRead);
	if (err != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::ProcessELFFile Error opening the SELF File %d"),
				err);
		SetBlockResult(EFail);
		User::Leave(err);
		}

	//validate the SELF Header Format
	ValidateHeaderELFFileL();

	//validate the Program Header and store information about the string section Sections
	ValidateProgramHeaderELFFileL();

	//validate the Symbian Info section
	ValidateSymInfoSectionELFFileL();

	//validate Thread name that crashed
	ValidateThreadInfoSectionELFFileL(EFalse, KThreadKernelCrash, ETrue);

	//validate the trace section to be present
	ValidateTraceSectionELFFileL(ETrue);

	//clean up of the generated SELF file
	iSELFFile.Close();
	TInt ret = iFsSession.Delete(iSELFFileName);
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::SELF File delete returned error %d"),
				ret);
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	INFO_PRINTF1(_L("Validation of the Positive Test Case Successful!!!\n\n"));

	/*
	 * NEGATIVE TEST CASE
	 * Test for the negative test case let the Trace Section be NOT present
	 */

	//configuring the SELF NOT to have trace segment
	ConfigureSELF(EFalse, ETrace);

    //generate the SELF File by calling ProcessCrashLogL
    GenerateElfFileL(iCrashList[0]->iCrashId);

	//open the SELF File for processing
	err = iSELFFile.Open(iFsSession, iSELFFileName, EFileStream|EFileRead);
	if (err != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::ProcessELFFile Error opening the SELF File %d"),
				err);
		SetBlockResult(EFail);
		User::Leave(err);
		}

	//validate the SELF Header Format
	ValidateHeaderELFFileL();

	//validate the Program Header and store information about the string section Sections
	ValidateProgramHeaderELFFileL();

	//validate the Symbian Info section
	ValidateSymInfoSectionELFFileL();

	//validate Thread name that crashed
	ValidateThreadInfoSectionELFFileL(EFalse, KThreadKernelCrash, ETrue);

	//validate the heap section NOT to be present this time
	ValidateTraceSectionELFFileL(EFalse);

	//do necessary clean up
	CleanupMethod(iSELFFileName);

	INFO_PRINTF1(_L("Validation of the Negative Test Case Successful!!!\n\n"));

	}

/**
 * Benchmark test of SELF files
 */
void CProcessCrashWrapper::DoCmdPerformanceMeasureSELFFileL()
	{
	INFO_PRINTF1(_L("CProcessCrashWrapper::DoCmdPerformanceMeasureSELFFileL performing Performance measurement on various SELF file(s)"));

	//expects one crash information to be present in the flash otherwise complain
	if (iCrashList.Count() != 1)
		{
		ERR_PRINTF1(_L("More than one crash information present"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}

	/*
	 * WITH OR WITHOUT HEAP DATA
	 */

	//configuring SELF to have the Heap section
	ConfigureSELF(ETrue, EHeap);

	//Generate the SELF File keeping a record of the time
	GenerateElfFileL(iCrashList[0]->iCrashId , ETrue);

	INFO_PRINTF1(_L("SELF File has Heap Data"));
	ValidatePerformanceELFFile();

	//configuring SELF NOT to have the Heap section
	ConfigureSELF(EFalse, EHeap);

	//Generate the SELF File keeping a record of the time
	GenerateElfFileL(iCrashList[0]->iCrashId , ETrue);

	INFO_PRINTF1(_L("SELF File has NO Heap Data"));
	ValidatePerformanceELFFile();

	/*
	 * WITH OR WITHOUT CODE SEGEMENT
	 */

	//configuring SELF to have the Code Segment
	ConfigureSELF(ETrue, ECode);
	//putting back the heap data as well
	ConfigureSELF(ETrue, EHeap);

	//Generate the SELF File keeping a record of the time
	GenerateElfFileL(iCrashList[0]->iCrashId , ETrue);

	INFO_PRINTF1(_L("SELF File has Code Segement"));
	ValidatePerformanceELFFile();

	//configuring SELF NOT to have the Code segment
	ConfigureSELF(EFalse, ECode);

	//Generate the SELF File keeping a record of the time
	GenerateElfFileL(iCrashList[0]->iCrashId , ETrue);

	INFO_PRINTF1(_L("SELF File has NO Code Segement"));
	ValidatePerformanceELFFile();

	/*
	 * WITH OR WITHOUT THREAD SEGEMENT
	 */

	//configuring SELF to have the Thread Segment
	ConfigureSELF(ETrue, EThread);
	//putting back the code segement as well
	ConfigureSELF(ETrue, ECode);

	//Generate the SELF File keeping a record of the time
	GenerateElfFileL(iCrashList[0]->iCrashId , ETrue);

	INFO_PRINTF1(_L("SELF File has Thread Segement"));
	ValidatePerformanceELFFile();

	//configuring SELF NOT to have the Thread segment
	ConfigureSELF(EFalse, EThread);

	//Generate the SELF File keeping a record of the time
	GenerateElfFileL(iCrashList[0]->iCrashId , ETrue);

	INFO_PRINTF1(_L("SELF File has NO Thread Segement"));
	ValidatePerformanceELFFile();

	INFO_PRINTF1(_L("Benchmarking of SELF files successful"));
	}

/**
 * Validates the Corrupted Crash Information
 * Part of neagtive test on ProcessCrashLog
 */
void CProcessCrashWrapper::DoCmdValidateCorruptCrashL()
	{
	LOG_MSG(_L("CProcessCrashWrapper::DoCmdValidateCorruptCrashL() validating the Corrupted Crash"));

	//reads all the crash information present in the flash
	DoCmdReadCrashInfoL();

	//expects one crash information to be present in the flash otherwise complain
	if (iCrashList.Count() != 1)
		{
		ERR_PRINTF1(_L("More than one crash information present"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}

	INFO_PRINTF1(_L("Able to read crash log now start processing the crash"));

	TRAPD(error, iCoreDumpSession.ProcessCrashLogL(iCrashList[0]->iCrashId));
	if (error == KErrCorrupt)
		{
		INFO_PRINTF2(_L("Success!!! The crash information is corrupted returns error %d\n"), error);
		}
	else
		{
		ERR_PRINTF2(
				_L("Returns error %d which is NOT  KErrCorrupt\n"),
				error);
		SetBlockResult(EFail);
		User::Leave(error);
		}
	}

/**
 * Validates the Header Format Elf32_Ehdr for the SELF file
 */
void CProcessCrashWrapper::ValidateHeaderELFFileL()
	{// Elf32_Ehdr
	INFO_PRINTF1(_L("CProcessCrashWrapper::ValidateHeaderELFFileL validating Elf32_Ehdr"));
	RDebug::Printf("Validating the ELF file");

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
				_L("CProcessCrashWrapper::ValidateHeaderELFFile Error reading the SELF Header Elf32_Ehdr %d"),
				ret);
		SetBlockResult(EFail);
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
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}
	if (eh->e_ident[EI_DATA] == 2)
		{
		// ELF Header size should be 52 bytes or converted into Big-Endian system 13312
		if (eh->e_ehsize != 13312)
			{
			ERR_PRINTF1(_L("Error:  ELF Header contains invalid file typ"));
			SetBlockResult(EFail);
			User::Leave(KErrGeneral);
			}
		// e_ident[EI_DATA] specifies the data encoding of the processor-specific data in the object file.
		ERR_PRINTF1(_L("Error:  Data encoding ELFDATA2MSB (Big-Endian) not supported"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}
	if (eh->e_ehsize != 52)
		{
		// ELF Header size should be 52 bytes
		ERR_PRINTF1(_L("Error:  ELF Header contains invalid file type"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}

	//Number of entries in the program header table
	if ((eh->e_phnum) == 0)
		{
		ERR_PRINTF1(_L("Error:  ELF Header contains zero program headers"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}

	//store number of program header entries
	iPHEntries = eh->e_phnum;
	INFO_PRINTF2(_L("Program header entries:%d"), iPHEntries);

	//Offset in bytes from the start of the file to the section header table
	if ((eh->e_phoff) == 0)
		{
		ERR_PRINTF1(_L("Error:  ELF Header contains zero offset to the section header table"));
		SetBlockResult(EFail);
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
void CProcessCrashWrapper::ValidateProgramHeaderELFFileL()
	{//Elf32_Phdr
	INFO_PRINTF1(_L("CProcessCrashWrapper::ValidateProgramHeaderELFFileL validating the program header Entries"));

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
				_L("CProcessCrashWrapper::ValidateProgramHeaderELFFileL Error reading the SELF Header Elf32_Phdr %d"),
				ret);
		SetBlockResult(EFail);
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
					_L("CProcessCrashWrapper::ValidateHeaderELFFile Error reading the SELF Header Sym32_dhdr %d"),
					ret);
			SetBlockResult(EFail);
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
 * Validates the Symbian Info Section Sym32_syminfod of the SELF file
 */
void CProcessCrashWrapper::ValidateSymInfoSectionELFFileL()
	{//Sym32_syminfod
	INFO_PRINTF1(_L("CProcessCrashWrapper::ValidateSymInfoSection validating the Symbian Info Section"));

	Elf32_Phdr* phdr = (Elf32_Phdr*) iSELFPHHeader;//Elf32_Phdr Program Header

	//loop through the program headers to get information about the Symbian Info section
	for (TInt i = 0; i < iPHEntries; i++)
		{
		if (phdr[i].p_type == PT_NOTE)
			{
			if (idhdr[i]->d_type == ESYM_NOTE_SYM) //SYMBIAN INFO SEGMENT
				{
				if (sizeof(Sym32_syminfod) != (idhdr[i]->d_descrsz))
					{
					ERR_PRINTF1(_L("CProcessCrashWrapper::ValidateProgramHeaderELFFileL sizeof(Sym32_syminfod)is different from descriptor size") );
					SetBlockResult(EFail);
					User::Leave(KErrGeneral);
					}
				TInt sizeofsyminfo = sizeof(Sym32_syminfod);
				INFO_PRINTF2(_L("ESYM_NOTE_SYM Header offset %d"), phdr[i].p_offset);
				INFO_PRINTF2(_L("ESYM_NOTE_SYM Size of single descriptor element %d"),
						idhdr[i]->d_descrsz);

				TInt offsettosyminfod = phdr[i].p_offset + sizeof(Sym32_dhdr);

				//allocate buffer for Sym32_syminfod
				TUint8* symbianInfoSection = new TUint8[sizeofsyminfo];
				CleanupStack::PushL(symbianInfoSection);

				TPtr8 ptrSymbianInfoSection(symbianInfoSection, sizeofsyminfo,
						sizeofsyminfo);

				TInt ret = iSELFFile.Read(offsettosyminfod, ptrSymbianInfoSection, sizeofsyminfo);
				if (ret != KErrNone)
					{
					ERR_PRINTF2(
							_L("CProcessCrashWrapper::ValidateProgramHeaderELFFileL Error reading the SELF Header Sym32_syminfod %d"), ret);
					SetBlockResult(EFail);
					User::Leave(ret);
					}
				Sym32_syminfod *syminfod = (Sym32_syminfod*) symbianInfoSection; //Symbian Info Section Sym32_syminfod

				// Validation Phase of the Symbian Info section
				INFO_PRINTF2(_L("Executable Crc32 %X"), syminfod->sd_execid.exec_crc);
				INFO_PRINTF2(_L("Crashed Thread %ld"), syminfod->sd_thread_id);
				INFO_PRINTF2(_L("Owning Process %ld"), syminfod->sd_proc_id);

				iCrashedThreadId = syminfod->sd_thread_id;
				if (ECrashException == syminfod->sd_exit_type)
					{
					INFO_PRINTF2(_L("Hardware Exception Number:%d"),
							syminfod->sd_exit_reason);
					ValidateExceptionL(syminfod->sd_exit_reason, ETestDataAbort, EFalse); //print the exception string
					}
				else
					if (ECrashKill == syminfod->sd_exit_type)
						{
						INFO_PRINTF2(_L("Exit Type\t\t\t:%d"),
								syminfod->sd_exit_reason);
						switch (syminfod->sd_exit_reason)
							{
							case 0:
								INFO_PRINTF1(_L(":EExitKill\n"));
								break;
							case 1:
								INFO_PRINTF1(_L(":EExitTerminate\n"));
								break;
							case 2:
								INFO_PRINTF1(_L(":EExitPanic\n"));
								break;
							case 3:
								INFO_PRINTF1(_L(":EExitPending\n"));
								break;
							default:
								INFO_PRINTF1(_L(":Unknown\n"));
								break;
							}
						}
					else
						{
						INFO_PRINTF1(_L("\tUnknown Crash Type\n"));
						}

				CleanupStack::PopAndDestroy(symbianInfoSection);

				}//if ESYM_NOTE_SYM

			if (idhdr[i]->d_type == ESYM_NOTE_EXEC) //EXECUTABLE INFO SEGMENT
				{
				if (sizeof(Sym32_execinfod) != (idhdr[i]->d_descrsz))
					{
					ERR_PRINTF1(_L("CProcessCrashWrapper::ValidateProgramHeaderELFFileL sizeof(Sym32_execinfod))is different from descriptor size") );
					User::Leave(KErrGeneral);
					}

				TInt sizeofsymexecinfo = sizeof(Sym32_execinfod);
				INFO_PRINTF2(_L("ESYM_NOTE_EXEC Header offset %d"),
						phdr[i].p_offset);
				INFO_PRINTF2(
						_L("ESYM_NOTE_EXEC Size of single descriptor element %d"),
						idhdr[i]->d_descrsz);

				//Sym32_execinfod *execinfod = ADDR(Sym32_execinfod,eh,data+sizeof(Sym32_dhdr));
				TInt offsettosymexecinfod = phdr[i].p_offset
						+ sizeof(Sym32_dhdr);

				TUint8* symbianExecInfoSection = new TUint8[sizeofsymexecinfo];
				CleanupStack::PushL(symbianExecInfoSection);

				TPtr8 ptrSymbianExecInfoSection(symbianExecInfoSection,
						sizeofsymexecinfo, sizeofsymexecinfo);

				TInt ret = iSELFFile.Read(offsettosymexecinfod,
						ptrSymbianExecInfoSection, sizeofsymexecinfo);
				if (ret != KErrNone)
					{
					ERR_PRINTF2(
							_L("CProcessCrashWrapper::ValidateProgramHeaderELFFileL Error reading the SELF Header Sym32_execinfod %d"),
							ret);
					User::Leave(ret);
					}
				Sym32_execinfod *execinfod =
						(Sym32_execinfod*) symbianExecInfoSection; //Symbian Info Section Sym32_syminfod

				TBufC<30> nameToCheck(KCrashAppFileName);

				ValidateStringL(EValExecutable, execinfod->ed_name, nameToCheck, EFalse);

				CleanupStack::PopAndDestroy();
				}//if ESYM_NOTE_EXEC

			}//if PT_NOTE

		}//for loop PHEntries
	INFO_PRINTF1(_L("Sym_Info validated"));
	}

/**
 * Validates the Symbian Thread Section Sym32_thrdinfod of the SELF file
 * @param aValidateCpuId If this is true, this test will fail if all the CPU Id's are not the same
 */
void CProcessCrashWrapper::ValidateThreadInfoSectionELFFileL(TBool aValidateCpuId, const TDesC& aThreadCrashed, TBool aCheck)
	{
	INFO_PRINTF1(_L("Validating the Symbian Thread Info Section"));

	Elf32_Phdr* phdr = (Elf32_Phdr*) iSELFPHHeader;//Elf32_Phdr Program Header
	TInt cpuId = -1;

	//loop through the program headers to get information about the Symbian Info section
	for (TInt i = 0; i < iPHEntries; i++)
		{
		if (phdr[i].p_type == PT_NOTE)
			{
			if (idhdr[i]->d_type == ESYM_NOTE_THRD) //SYMBIAN INFO SEGMENT
				{
				if (sizeof(Sym32_thrdinfod) != (idhdr[i]->d_descrsz))
					{
					ERR_PRINTF1(_L("CProcessCrashWrapper::ValidateThreadInfoSectionELFFileL sizeof(Sym32_thrdinfod) %d is different from descriptor size") );
					SetBlockResult(EFail);
					User::Leave(KErrGeneral);
					}

				TInt sizeofthreadinfo = sizeof(Sym32_thrdinfod);
				INFO_PRINTF2(_L("ESYM_NOTE_THRD Header offset %d"), phdr[i].p_offset);
				INFO_PRINTF2(_L("ESYM_NOTE_THRD Size of single descriptor element %d"),
						idhdr[i]->d_descrsz);
				INFO_PRINTF2(_L("ESYM_NOTE_THRD Number of elements %d"), idhdr[i]->d_elemnum);

				TInt offsettothreadinfod = phdr[i].p_offset + sizeof(Sym32_dhdr);
				TInt newoffsettothreadinfod = offsettothreadinfod;

				//allocate buffer for Sym32_thrdinfod
				TUint8* symbianThreadSection = new TUint8[sizeofthreadinfo];
				CleanupStack::PushL(symbianThreadSection);

				TPtr8 ptrSymbianThreadSection(symbianThreadSection, sizeofthreadinfo,
						sizeofthreadinfo);

				TInt ret = iSELFFile.Read(offsettothreadinfod, ptrSymbianThreadSection, sizeofthreadinfo);
				if (ret != KErrNone)
					{
					ERR_PRINTF2(
							_L("CProcessCrashWrapper::ValidateThreadInfoSectionELFFileL Error reading the SELF Header Sym32_thrdinfod %d"), ret);
					SetBlockResult(EFail);
					User::Leave(ret);
					}
				Sym32_thrdinfod *thrdinfod = (Sym32_thrdinfod*) symbianThreadSection; //Symbian Info Section Sym32_syminfod

				for(int thrIdx = 0; thrIdx < idhdr[i]->d_elemnum; thrIdx++ )
					{
					//valid checking fo thread id

					if (iCrashedThreadId == thrdinfod->td_id)
						{
						INFO_PRINTF2(_L("Owning process %ld"), thrdinfod->td_owning_process);
						INFO_PRINTF2(_L("Thread Priority %d"), thrdinfod->td_priority);
						INFO_PRINTF2(_L("Heap Adress 0x%X"), thrdinfod->td_heap_add);
						INFO_PRINTF2(_L("Heap Size %d"), thrdinfod->td_heap_sz);
						//storing the heap address and the Heap Siz for future verification
						iHeapBase = thrdinfod->td_heap_add;
						iHeapSize = thrdinfod->td_heap_sz;

						ValidateStringL(EValThread, thrdinfod->td_name, aThreadCrashed, aCheck);

						}

					if(aValidateCpuId)
						{
						//If its the first, set the CPU ID to be this
						if(cpuId < 0)
							{
							cpuId = thrdinfod->td_last_cpu_id;
							}
						else
							{
							//Otherwise, the current CPU ID should be the same as the previous
							if(thrdinfod->td_last_cpu_id != cpuId)
								{
								ERR_PRINTF3(_L("CProcessCrashWrapper::ValidateThreadInfoSectionELFFileL CPU ID Not as expected. Current is not the same as previous. Current = [%d] Previous = [%d]"), thrdinfod->td_last_cpu_id, cpuId);
								SetBlockResult(EFail);
								User::Leave(KErrCorrupt);
								}
							cpuId = thrdinfod->td_last_cpu_id;
							}
						}

					newoffsettothreadinfod = newoffsettothreadinfod + sizeofthreadinfo;

					ret = iSELFFile.Read(newoffsettothreadinfod, ptrSymbianThreadSection, sizeofthreadinfo);
					if (ret != KErrNone)
						{
						ERR_PRINTF2(
								_L("CProcessCrashWrapper::ValidateThreadInfoSectionELFFileL Error reading the SELF Header Sym32_thrdinfod %d"), ret);
						SetBlockResult(EFail);
						User::Leave(ret);
						}

					thrdinfod = (Sym32_thrdinfod*) symbianThreadSection;

					}

				CleanupStack::PopAndDestroy();
				}//if ESYM_NOTE_THRD

			}//if PT_NOTE

		}//for PHEntries

	}

/**
 * Validates the Register Info Section
 * Reads the CPSR value to determine the type of the crash (user/system)
 */
void CProcessCrashWrapper::ValidateRegisterInfoSectionELFFileL()
	{

	INFO_PRINTF1(_L("CProcessCrashWrapper::ValidateRegisterInfoSectionELFFileL validating the regsiter Section"));

	TBool matchfound = EFalse;
	Elf32_Phdr* phdr = (Elf32_Phdr*) iSELFPHHeader;//Elf32_Phdr Program Header

	//loop through the program headers to get information about the Symbian Info section
	for (TInt i = 0; (i < iPHEntries) && !matchfound; i++)
		{
		if (phdr[i].p_type == PT_NOTE)
			{
			if (idhdr[i]->d_type == ESYM_NOTE_REG) //SYMBIAN REGISTER INFO SEGEMENT
				{

				TInt sizeofreginfo = sizeof(Sym32_reginfod);
				TInt offsettoreginfod = phdr[i].p_offset + sizeof(Sym32_dhdr);

				//allocate buffer for Sym32_reginfod
				TUint8* symbianRegSection = new TUint8[sizeofreginfo];
				CleanupStack::PushL(symbianRegSection);

				TPtr8 ptrSymbianRegSection(symbianRegSection, sizeofreginfo,
						sizeofreginfo);

				TInt ret = iSELFFile.Read(offsettoreginfod, ptrSymbianRegSection, sizeofreginfo);
				if (ret != KErrNone)
					{
					ERR_PRINTF2(
							_L("CProcessCrashWrapper::ValidateRegisterInfoSectionELFFileL Error reading the SELF Header Sym32_thrdinfod %d"), ret);
					SetBlockResult(EFail);
					User::Leave(ret);
					}
				Sym32_reginfod *reginfod = (Sym32_reginfod*) symbianRegSection; //Symbian Register Info Section Sym32_reginfod

				if(reginfod->rid_thread_id == iCrashedThreadId)
					{

					INFO_PRINTF2(_L("Crashed Thread id %ld"), iCrashedThreadId);
					INFO_PRINTF2(_L("Thread ID %ld"), reginfod->rid_thread_id);

					TInt offsettoregdatad = offsettoreginfod + sizeofreginfo;

					for( TInt i = 0; (i < reginfod->rid_num_registers) && !matchfound; i++ )
						{
						//Sym32_regdatad immediately following the Register Info descriptor header
						TInt sizeofregdatad = sizeof (Sym32_regdatad);
						offsettoregdatad = offsettoregdatad + sizeofreginfo;

						TUint8* symbianRegDatad = new TUint8[sizeofregdatad];
						CleanupStack::PushL(symbianRegDatad);

						TPtr8 ptrSymbianRegDatad(symbianRegDatad, sizeofregdatad,
								sizeofregdatad);

						TInt ret = iSELFFile.Read(offsettoregdatad, ptrSymbianRegDatad, sizeofregdatad);
						if (ret != KErrNone)
							{
							ERR_PRINTF2(
									_L("CProcessCrashWrapper::ValidateRegisterInfoSectionELFFileL Error reading the SELF Header Sym32_thrdinfod %d"), ret);
							SetBlockResult(EFail);
							User::Leave(ret);
							}

						Sym32_regdatad *regdatad = (Sym32_regdatad*) symbianRegDatad; //Symbian Register Info Section Sym32_regdatad

						if( (ESYM_REG_CORE == reginfod->rid_class) && (ESYM_REG_32 == reginfod->rid_repre) && (KCPSRReg == regdatad->rd_id) )
							{
							//we have found the CPSR value for the crashed thread, now read the register value
							TInt sizeofELF32Word = sizeof(Elf32_Word);
							TUint8* rd_data_cpsr = new TUint8[sizeofELF32Word];
							CleanupStack::PushL(rd_data_cpsr);

							TPtr8 ptrSymbianRdDataCpsr(rd_data_cpsr, sizeofELF32Word,
									sizeofELF32Word);

							TInt ret = iSELFFile.Read(regdatad->rd_data , ptrSymbianRdDataCpsr, sizeofELF32Word);
							if (ret != KErrNone)
								{
								ERR_PRINTF2(
									_L("CProcessCrashWrapper::ValidateRegisterInfoSectionELFFileL Error reading the SELF Header Sym32_thrdinfod %d"), ret);
								SetBlockResult(EFail);
								User::Leave(ret);
								}

							Elf32_Word * val32 = (Elf32_Word*) rd_data_cpsr;

							TUint cpsrval = *val32;

							if(cpsrval & KModeBitCPSR) //checking on the M[4:0] mode bits of the CPSR
								{
								INFO_PRINTF2(_L("System Crash CPSR  value: 0x%X\n"), cpsrval );
								iSystemCrash = ETrue; //system crash
								}
							else
								{
								INFO_PRINTF2(_L("User Side Crash CPSR  value: 0x%X\n"), cpsrval );
								iSystemCrash = EFalse; //user crash
								}

							matchfound = ETrue;

							CleanupStack::PopAndDestroy();//rd_data_cpsr CPSR data value
							}

						CleanupStack::PopAndDestroy();//Sym32_regdatad symbianRegDatad
						}// for loop reginfod->rid_num_registers

					}//if iCrashedThreadId

				CleanupStack::PopAndDestroy();

				}
			}
		}

	}

/**
 * Validates the Heap Section of the SELF file
 */
void CProcessCrashWrapper::ValidateHeapSectionELFFileL(TBool aHeapCheck)
	{

	INFO_PRINTF1(_L("CProcessCrashWrapper::ValidateHeapSectionELFFileL validating the Heap Section"));

	Elf32_Phdr* phdr = (Elf32_Phdr*) iSELFPHHeader;//Elf32_Phdr Program Header

	TBool matchfound = EFalse;
	//loop through the program headers to get information about the Symbian Info section
	for (TInt i = 0; i < iPHEntries; i++)
		{
		if (phdr[i].p_type == PT_LOAD)
			{
			if(phdr[i].p_vaddr == iHeapBase && phdr[i].p_filesz == iHeapSize )
				{
				if(phdr[i].p_filesz == 0) //if the heap size is zero just complain
					{
					ERR_PRINTF1(
							_L("CProcessCrashWrapper::ValidateHeapSectionELFFileL Error Heap Size found to be zero"));
					SetBlockResult(EFail);
					User::Leave(KErrGeneral);
					}
				matchfound = ETrue;
				INFO_PRINTF2(_L("Heap section found at PHDR offset %d"), phdr[i].p_offset);
				break;
				}

			}
		}
	if(!aHeapCheck && matchfound )
		{
		ERR_PRINTF1(
				_L("CProcessCrashWrapper::ValidateHeapSectionELFFileL Error found heap section when there shouldnt be one"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}
	if(aHeapCheck  && !matchfound )
		{
		ERR_PRINTF1(
				_L("CProcessCrashWrapper::ValidateHeapSectionELFFileL Error did not find the heap section"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}

	}

/**
 * Validates the Trace Section Sym32_tracedata of the SELF file
 */
void CProcessCrashWrapper::ValidateTraceSectionELFFileL(TBool aTraceCheck)
	{

	INFO_PRINTF1(_L("CProcessCrashWrapper::ValidateTraceSectionELFFileL validating the Trace Section"));

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
					ERR_PRINTF1(_L("CProcessCrashWrapper::ValidateTraceSectionELFFileL sizeof(Sym32_tracedata) %d is different from descriptor size") );
					SetBlockResult(EFail);
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
							_L("CProcessCrashWrapper::ValidateTraceSectionELFFileL Error reading the SELF Header Sym32_thrdinfod %d"), ret);
					SetBlockResult(EFail);
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

					tracebufferString.CreateL(traceInfo->tr_size);//READING 200 characters assuming strings to be not more than 200 characters

					TInt ret = iSELFFile.Read(traceInfo->tr_data, tracebufferString, traceInfo->tr_size);
					if (ret != KErrNone)
						{
						ERR_PRINTF2(
								_L("CProcessCrashWrapper::ValidateTraceSectionELFFileL Error reading the Trace Information  %d"),
								ret);
						SetBlockResult(EFail);
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
							_L("CProcessCrashWrapper::ValidateTraceSectionELFFileL Error found trace size to be zero"));
					SetBlockResult(EFail);
					User::Leave(KErrGeneral);

					}
				}
			}

		}

	if(!aTraceCheck && dataPresent )
		{
		ERR_PRINTF1(
				_L("CProcessCrashWrapper::ValidateTraceSectionELFFileL Error found trace section when there shouldnt be one"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}
	if(aTraceCheck  && !dataPresent )
		{
		ERR_PRINTF1(
				_L("CProcessCrashWrapper::ValidateTraceSectionELFFileL Error did not find the trace section"));
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}

	}

/**
 * @return void
 * This method monitors the RProperty for the crash progress. There are several states of this but we are
 * only interested in the start and finish of these properties
 */
void CProcessCrashWrapper::MonitorProgressL()
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
 * Utility Function to print and validate the exception type
 * @param aException exception number
 * @param acheckException exception to check for
 * @param aCheck whether to validate the exception or not
 */
void CProcessCrashWrapper::ValidateExceptionL(TInt aException, TInt acheckException, TBool aCheck)
	{
	INFO_PRINTF2(_L("Hardware Exception of type: %d"), aException);
	switch (aException)
		{
		case 0:
			INFO_PRINTF1(_L(":EExcGeneral"));
			break;
		case 1:
			INFO_PRINTF1(_L(":EExcIntegerDivideByZero"));
			break;
		case 2:
			INFO_PRINTF1(_L(":EExcSingleStep"));
			break;
		case 3:
			INFO_PRINTF1(_L(":EExcBreakPoint"));
			break;
		case 4:
			INFO_PRINTF1(_L(":EExcIntegerOverflow"));
			break;
		case 5:
			INFO_PRINTF1(_L(":EExcBoundsCheck"));
			break;
		case 6:
			INFO_PRINTF1(_L(":EExcInvalidOpCode"));
			break;
		case 7:
			INFO_PRINTF1(_L(":EExcDoubleFault"));
			break;
		case 8:
			INFO_PRINTF1(_L(":EExcStackFault"));
			break;
		case 9:
			INFO_PRINTF1(_L(":EExcAccessViolation"));
			break;
		case 10:
			INFO_PRINTF1(_L(":EExcPrivInstruction"));
			break;
		case 11:
			INFO_PRINTF1(_L(":EExcAlignment"));
			break;
		case 12:
			INFO_PRINTF1(_L(":EExcPageFault"));
			break;
		case 13:
			INFO_PRINTF1(_L(":EExcFloatDenormal"));
			break;
		case 14:
			INFO_PRINTF1(_L(":EExcFloatDivideByZero"));
			break;
		case 15:
			INFO_PRINTF1(_L(":EExcFloatInexactResult"));
			break;
		case 16:
			INFO_PRINTF1(_L(":EExcFloatInvalidOperation"));
			break;
		case 17:
			INFO_PRINTF1(_L(":EExcFloatOverflow"));
			break;
		case 18:
			INFO_PRINTF1(_L(":EExcFloatStackCheck"));
			break;
		case 19:
			INFO_PRINTF1(_L(":EExcFloatUnderflow"));
			break;
		case 20:
			INFO_PRINTF1(_L(":EExcAbort"));
			break;
		case 21:
			INFO_PRINTF1(_L(":EExcKill"));
			break;
		case 22:
			INFO_PRINTF1(_L(":EExcUserInterrupt"));
			break;
		case 23:
			INFO_PRINTF1(_L(":EExcDataAbort"));
			break;
		case 24:
			INFO_PRINTF1(_L(":EExcCodeAbort"));
			break;
		case 25:
			INFO_PRINTF1(_L(":EExcMaxNumber"));
			break;
		case 26:
			INFO_PRINTF1(_L(":EExcInvalidVector"));
			break;
		default:
			ERR_PRINTF1(_L("Unknown Exception"));
			break;
		}

		if(aCheck)
			{
		    if( aException != acheckException )
		    	{
		    	ERR_PRINTF1(
						_L("CProcessCrashWrapper::ValidateExceptionL is not the valid type"));
				SetBlockResult(EFail);
				User::Leave(KErrGeneral);
		    	}

		    INFO_PRINTF1(_L("Hardware Exception matched successful!"));
			}

	}
/**
 * Utility Function to print and validate a string
 * Reads the String section and searches for a NULL character to find the right string
 * @param aIndex index to the string section
 * @param aStingToMatch string to be matched
 * @param aCheck whether to validate the string or not
 */
void CProcessCrashWrapper::ValidateStringL(TValidateString aType, TInt aIndex, const TDesC& aStingToMatch, TBool aCheck)
	{
	INFO_PRINTF2(_L("CProcessCrashWrapper::ValidateStringL Index %d"), aIndex);

	RBuf8 bufferString;
	bufferString.CleanupClosePushL();
	bufferString.CreateL(KMaxStringLength);//Reading 200 characters assuming strings to be not more than 200 characters

	//going to the string in the string node
	TInt offsetString = iOffsetStringSection + aIndex;
	TInt ret = iSELFFile.Read(offsetString, bufferString, KMaxStringLength);
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::PrintStringL Error reading the String Information  %d"),
				ret);
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	//trying to find a NULL terminating character
	TInt nullPos = -1;
	TInt loop = 0;
	while ((nullPos = bufferString.Locate(NULL)) > KErrNotFound)
		{
		loop++;
		if (loop > KMaxStringLength)
			{
			ERR_PRINTF1(_L("CProcessCrashWrapper::ValidateStringL Error reading NULL character from the string"));
			SetBlockResult(EFail);
			User::Leave(KErrGeneral);
			}
		else
			{
			//found the NULL character exit the loop
			INFO_PRINTF2(_L("ValidateStringL postion of NULL character %d"), nullPos);
			break;
			}
		}

	CleanupStack::PopAndDestroy(); //bufferString

	//create a new actual string to hold the value we want
	RBuf8 printString;
	printString.CreateL(nullPos+2);
	printString.CleanupClosePushL();

	ret = iSELFFile.Read(offsetString, printString, nullPos);
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::ValidateStringL Error reading reading the String Information %d"),
				ret);
		SetBlockResult(EFail);
		User::Leave(ret);
		}

	//create a NULL terminated string for printing on serial output
	char* clPrompt = (char*) printString.PtrZ();
	RDebug::Printf("String printed %s", clPrompt);

	RBuf bigString;
	bigString.CreateL(KMaxFileName);
	bigString.CleanupClosePushL();

	bigString.Copy(printString); //8 to 16 bit conversion

	if(aType == EValExecutable)
		{
		INFO_PRINTF2(_L("Crashed Executable name: %S"), &bigString);

		}

	else if(aType == EValThread)
		{
		if(!aCheck) //NOT to check on the thread name
			{
			//just happy printing the name of the thread
			INFO_PRINTF2(_L("Crashed Thread name: %S"), &bigString);
			}
		else
			{
			// check with the thread name passed
			RBuf stringtoCheck;
			stringtoCheck.Create(aStingToMatch);
			stringtoCheck.CleanupClosePushL();

			bigString.SetLength(stringtoCheck.Length()); //reducing the length

			INFO_PRINTF2(_L("Crashed Thread name Verified : %S"), &bigString);

			if(bigString.Compare(stringtoCheck) != 0)
				{
				ERR_PRINTF1(
						_L("CProcessCrashWrapper::ValidateStringL Application name does not match "));
				SetBlockResult(EFail);
				User::Leave(KErrGeneral);
				}

			INFO_PRINTF1( _L("CProcessCrashWrapper::ValidateStringL Crashed Application Match Successful ! "));

			CleanupStack::PopAndDestroy();//stringtoCheck

			}

		}
	CleanupStack::PopAndDestroy();//bigString
	CleanupStack::PopAndDestroy(); //printString

	}

/**
 * Utility Function to do trace data verification
 */
void CProcessCrashWrapper::ValidateTraceBufferL(const TDesC8& aBuffer)
{
	INFO_PRINTF2(_L("CDataTranslator::ValidateTraceBufferL: size: %d"), aBuffer.Length());

	const TUint8* tracePtr = aBuffer.Ptr();
	TInt traceSize = aBuffer.Length();

/*	TUint8 recSize = 0;
	TUint8 flags = 0;*/
	TUint8 category = 0;
/*	TUint8 subCategory = 0;*/
	TInt count = 0;

	const TUint8* endPtr = tracePtr+traceSize;
	while(tracePtr < endPtr)
	{
		count++;
		category = 0;

		/*recSize = flags = subCategory = 0;
		recSize = tracePtr[BTrace::ESizeIndex];
		flags = tracePtr[BTrace::EFlagsIndex];
		subCategory = tracePtr[BTrace::ESubCategoryIndex];*/

		//read trace header
		category = tracePtr[BTrace::ECategoryIndex];

		//check the parameter Category
		if(category != KTraceCategory)
			{
			ERR_PRINTF1(
					_L("CProcessCrashWrapper::ValidateTraceBufferL Category 200 did not match with the trace buffer"));
			SetBlockResult(EFail);
			User::Leave(KErrGeneral);
			}

		tracePtr = BTrace::NextRecord((TAny*)tracePtr);
	}

	INFO_PRINTF2(_L("Number of trace packets found: %d"), count);
}

/**
 * Reports performance metrics from all the tests
 */
void CProcessCrashWrapper::ValidatePerformanceELFFile()
	{

	TInt ticks = HelpGetTestTicks();
	TInt nkTicksPerSecond = HelpTicksPerSecond();

	TInt actualtimeinseconds = ticks/nkTicksPerSecond;

	INFO_PRINTF3(_L("SELF File Creation E:\\%S took: %d seconds\n") , &iSELFFileName, ticks/nkTicksPerSecond);

	if (actualtimeinseconds > KMAXTIMEOUT)
		{
		ERR_PRINTF2(
				_L("SELF File Creation had exceeded TIME OUT of %d secs "), KMAXTIMEOUT);
		SetBlockResult(EFail);
		User::Leave(KErrGeneral);
		}

	//clean up routine for the file created
	TInt ret = iFsSession.Delete(iSELFFileName);
	if (ret != KErrNone)
		{
		ERR_PRINTF2(
				_L("CProcessCrashWrapper::SELF File delete returned error %d"),
				ret);
		SetBlockResult(EFail);
		User::Leave(ret);
		}
	iStartTick = iStopTick = 0;//reset the timer ticks

	}
/**
 * Utility Function to do SELF Configurations
 */
void CProcessCrashWrapper::ConfigureSELF(TBool aPresent, TInt aSegment)
	{

	INFO_PRINTF3(_L("CProcessCrashWrapper::ConfigureSELF confiugring SELF for segment %d to present %d"), aSegment, aPresent);

	if(EHeap == aSegment)
		{//heap segment is configured through the data segemnts in the SELF file
		if(aPresent)
			{//configure to have data segments
			DoConfigureL(2, KUidELFFormatterV2.iUid, COptionConfig::EFormatterPlugin,
					COptionConfig::ETBool, KDataSegmentPrompt,
					1, KTrueFalseOpt, 1, KTrueOpt, 0);

			}
		else
			{//configure NOT to have data segments
			DoConfigureL(2, KUidELFFormatterV2.iUid, COptionConfig::EFormatterPlugin,
					COptionConfig::ETBool, KDataSegmentPrompt,
					1, KTrueFalseOpt, 0, KFalseOpt, 0);
			}
		}

    if(ETrace == aSegment)
    	{//trace segment configured here
    	if(aPresent)
			{//configure to have trace segments upto have 10KB data
			DoConfigureL(8, KUidELFFormatterV2.iUid, COptionConfig::EFormatterPlugin,
					COptionConfig::ETInt,  KCreateTraceData,
					1, KNullDesC, 10, KNullDesC, 0);

			}
		else
			{//configure NOT to have trace segments
			DoConfigureL(8, KUidELFFormatterV2.iUid, COptionConfig::EFormatterPlugin,
					COptionConfig::ETInt,  KCreateTraceData,
					1, KNullDesC, 0, KNullDesC, 0);
			}
    	}

    if(ECode == aSegment)
    	{//code segment configured here
    	if(aPresent)
    		{//configure to have Code Segements
			DoConfigureL(3, KUidELFFormatterV2.iUid, COptionConfig::EFormatterPlugin,
					COptionConfig::ETBool, KCodeSegmentPrompt,
					1, KTrueFalseOpt, 1, KTrueOpt, 0);
    		}
    	else
    		{//configure NOT to have Code Segments
			DoConfigureL(3, KUidELFFormatterV2.iUid, COptionConfig::EFormatterPlugin,
					COptionConfig::ETBool, KCodeSegmentPrompt,
					1, KTrueFalseOpt, 0, KFalseOpt, 0);
    		}
    	}

    if(EThread == aSegment)
    	{//thread segment configured here
    	if(aPresent)
    		{//configure to have Thread Segments
			DoConfigureL(4, KUidELFFormatterV2.iUid, COptionConfig::EFormatterPlugin,
					COptionConfig::ETBool, KThreadSegmentPrompt,
					1, KTrueFalseOpt, 1, KTrueOpt, 0);
    		}
    	else
    		{//configure NOT to have Thread Segments
			DoConfigureL(4, KUidELFFormatterV2.iUid, COptionConfig::EFormatterPlugin,
					COptionConfig::ETBool, KThreadSegmentPrompt,
					1, KTrueFalseOpt, 0, KFalseOpt, 0);
    		}
    	}

	}

/**
 * Utility Function to do cleanup activities
 */
void CProcessCrashWrapper::CleanupMethod(const TDesC& aDes)
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
				_L("CProcessCrashWrapper::SELF File delete returned error %d"),
				ret);
		SetBlockResult(EFail);
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

	INFO_PRINTF1(_L("Cleanup Done of the SELF File\n"));
	}

/**
 * Returns the number of nanokernel ticks in one second
 * @return Number of nanokernel ticks. 0 if unsuccesful
 */
TInt CProcessCrashWrapper::HelpTicksPerSecond(void)
	{
	TInt nanokernel_tick_period;
	HAL::Get(HAL::ENanoTickPeriod, nanokernel_tick_period);

	ASSERT(nanokernel_tick_period != 0);

	static const TInt KOneMillion = 1000000;

	return KOneMillion/nanokernel_tick_period;
	}

/**
 *Prints the error condition for TEF log/output
 */
void CProcessCrashWrapper::PrintErrorCondition(TInt aError)
	{
	//print out the error and put the result as failure
	ERR_PRINTF2(_L("Oops!!! error %d"),aError);
	SetBlockResult(EFail);
	}
/*
 * class CAsyncProcessCrash Implementation
 */

/**
 * Two phase constructor for CAsyncProcessCrash
 * @return CAsyncProcessCrash object
 * @leave
 */
CAsyncProcessCrash* CAsyncProcessCrash::NewL(CProcessCrashWrapper* aProcessCrashWrapper)
	{
	CAsyncProcessCrash* ret = new (ELeave) CAsyncProcessCrash(aProcessCrashWrapper);
	CleanupStack::PushL(ret);
	ret->ConstructL();
	CleanupStack::Pop(ret);
	return ret;
	}

/**
 * Safe construction
 * @leave
 */
void CAsyncProcessCrash::ConstructL()
	{
	}

/*
 * Constructor for the CActive derived object
 */
CAsyncProcessCrash::CAsyncProcessCrash(CProcessCrashWrapper* aProcessCrashWrapper)
	: CActive(CActive::EPriorityStandard),
	  iProcessCrashWrapper(aProcessCrashWrapper),
	  iCrashId(0)
	{
	CActiveScheduler::Add(this);
	}

/*
 * Function that issues request to the Core Dump Server
 * @param aCrashId is the crash id
 * to process crash log
 */
void CAsyncProcessCrash::IssueProcessCrashRequest(TUint aCrashId)
	{
	RDebug::Printf("CAsyncProcessCrash::IssueProcessCrashRequest");

	iCrashId = aCrashId;

	ASSERT(!IsActive());

	// Actually issue the request
	iProcessCrashWrapper->iCoreDumpSession.ProcessCrashLog( iCrashId, iStatus);

	// Tell the framework that a request is pending but
	// do it after the async request in case it Leaves
	SetActive();
	}

/*
 * RunL from CActive when the asynchronous request completes
 *
 */
void CAsyncProcessCrash::RunL()
	{
	RDebug::Printf("CAsyncProcessCrash::RunL %d", iStatus.Int());
	// that errors are handled in RunError()
	User::LeaveIfError(iStatus.Int());

	// request-handling code
  	iProcessCrashWrapper->ProcessSELFFileCreatedL();

  	CActiveScheduler::Stop();
	}

/*
 * RunError from CActive called when RunL leaves
 */
TInt CAsyncProcessCrash::RunError(TInt aError)
	{
	RDebug::Printf("CAsyncProcessCrash::RunError");

	iProcessCrashWrapper->PrintErrorCondition(aError);

	CActiveScheduler::Stop();

	return KErrNone; // error handled
	}

/*
 * Destructor for clean up purposes
 */
CAsyncProcessCrash::~CAsyncProcessCrash()
	{
	RDebug::Printf("CAsyncProcessCrash::~CAsyncProcessCrash");
	//cancel any outstanding request
	Cancel();

	}

/*
 * Destructor for clean up purposes
 */
void CAsyncProcessCrash::DoCancel()
	{
	RDebug::Printf("CAsyncProcessCrash::DoCancel");
	//cancel the processing the crashlog
	iProcessCrashWrapper->iCoreDumpSession.CancelProcessCrashLog(iCrashId);
	}

//eof
