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
//

/**
 @file
 @internalTechnology
*/

#include "t_scm_config.h"

_LIT(KConfigureOn, "ConfigureOn");
_LIT(KConfigureOff, "ConfigureOff");
_LIT(KKeyValue, "value");

CSCMConfigurationWrapper::CSCMConfigurationWrapper()
	{}

CSCMConfigurationWrapper::~CSCMConfigurationWrapper()
	{
	iCoreDumpSession.Close();
	}

/**
 * Two phase constructor for CSCMConfigurationWrapper
 * @return CSCMConfigurationWrapper object
 * @leave
 */
CSCMConfigurationWrapper* CSCMConfigurationWrapper::NewL()
	{
	CSCMConfigurationWrapper* ret = new (ELeave) CSCMConfigurationWrapper();
	CleanupStack::PushL(ret);
	ret->ConstructL();
	CleanupStack::Pop(ret);
	return ret;
	}

/**
 * Safe construction
 * @leave
 */
void CSCMConfigurationWrapper::ConstructL()
	{
	User::LeaveIfError(iCoreDumpSession.Connect());
	}

/**
 * Handle a command invoked from the script
 * @param aCommand Is the name of the command for a TBuf
 * @param aSection Is the .ini file section where parameters to the command are located
 * @param aAsyncErrorIndex Is used by the TEF Block framework to handle asynchronous commands.
 * @leaves One of the system wide error codes
 */
TBool CSCMConfigurationWrapper::DoCommandL(const TTEFFunction& aCommand, const TTEFSectionName& aSection, const TInt aAsyncErrorIndex)
	{
	//Get the enum value from the ini file
	TConfigItem::TSCMDataType type;
	TInt enumValue;

	if(GetIntFromConfig(aSection,KKeyValue, enumValue))
		{
		type = (TConfigItem::TSCMDataType)enumValue;
		}
	else
		{
		ERR_PRINTF1(_L("Unable to get the proper config option from ini file. Are you sure its valid?"));
		return EFalse;
		}

	//Look at the command to see what we need to do
	if(KConfigureOn() == aCommand)
		{
		TurnOnConfigOptionL(type);
		}
	else if(KConfigureOff() == aCommand)
		{
		TurnOffConfigOptionL(type);
		}
	else
		{
		ERR_PRINTF1(_L("Unable to get the proper command from script file. Are you sure its valid?"));
		return EFalse;
		}

	return ETrue;
	}

/**
 * This method allows the test wrapper to configure the SCM to not dump one of the
 * possible options
 * @param aParameter The param we wish to set
 * @leaves One of the system wide error codes
 */
void CSCMConfigurationWrapper::TurnOffConfigOptionL(TConfigItem::TSCMDataType& aParameter)
	{
	TInt zeroPriority = 0;
	SetSCMParameterL(aParameter, zeroPriority);
	}

/**
 * This method allows the test wrapper to configure the SCM to dump one of the
 * possible options
 * @param aParameter The param we wish to set
 * @leaves One of the system wide error codes
 */
void CSCMConfigurationWrapper::TurnOnConfigOptionL(TConfigItem::TSCMDataType& aParameter)
	{
	TInt highestPriority = 255;
	SetSCMParameterL(aParameter, highestPriority);
	}

/**
 * Sets an SCM parameter to a given priority
 * @param aParameter The SCM parameter we wish to change
 * @param aPriority The new priority
 * @leaves One of the System wide codes
 */
void CSCMConfigurationWrapper::SetSCMParameterL(TConfigItem::TSCMDataType& aParameter, const TInt aPriority)
	{
	//Unfortunately, there is no "get by instance" method (just a get by index - which we dont know
	TInt numConfigParams = iCoreDumpSession.GetNumberConfigParametersL();
	for(TUint cnt = 0; cnt < numConfigParams; cnt++)
		{
		COptionConfig* conf = iCoreDumpSession.GetConfigParameterL(cnt);
		CleanupStack::PushL(conf);

		if (conf->Source()== COptionConfig::ESCMConfig && conf->Instance() == aParameter)
			{
			//Found the one of interest. Set config to desired value
			INFO_PRINTF3(_L("Setting config value %d to be %d"), aParameter, aPriority);

			conf->Value(aPriority);
			iCoreDumpSession.SetConfigParameterL(*conf);
			}
		CleanupStack::PopAndDestroy(conf);
		}
	}

//eof
