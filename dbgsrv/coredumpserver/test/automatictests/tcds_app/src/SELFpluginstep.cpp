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
 @file SELFStep.cpp
 @internalTechnology
*/
#include "SELFpluginstep.h"
#include "tcoredumpserversuitedefs.h"

CSELFPluginStep::~CSELFPluginStep()
/**
 * Destructor
 */
	{
	}

CSELFPluginStep::CSELFPluginStep()
/**
 * Constructor
 */
	{
	// **MUST** call SetTestStepName in the constructor as the controlling
	// framework uses the test step name immediately following construction to set
	// up the step's unique logging ID.
	SetTestStepName(KSELFPluginStep);
	}

TVerdict CSELFPluginStep::doTestStepPreambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	   
	TInt ret = KErrNone;
	ret = iSess.Connect(); 
	if(ret != KErrNone)
		{
		SetTestStepResult( EFail );
		INFO_PRINTF2(_L("Error %d from iSess->Connect()/n"), ret);
		}	
	else
		{
		SetTestStepResult(EPass);
		}

	return TestStepResult();
	}


TVerdict CSELFPluginStep::doTestStepL()
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

void CSELFPluginStep::ClientAppL()
/**
 * @return void
 * This calls each stage of the test
 */
	{
	INFO_PRINTF1(_L("Starting SELF Plugin Test Suite") );
	LoadPluginL();
	UnloadPluginL();
    for(TInt i = 1; i < SelfMultipleTries; i++)
        {
        LoadMultiplePluginsL(i);
        UnloadMultiplePluginsL(i);
        }

	}

void CSELFPluginStep::LoadPluginL()
/**
 * @return void
 * Tests that the plugin can be loaded correctly
 */
	{		
	if (TestStepResult()==EPass)
		{
			
		INFO_PRINTF1(_L("Attempting to load SELF Plugin"));
		
		TPluginRequest req;
		req.iPluginType = TPluginRequest::EFormatter;
		req.iLoad = ETrue;
		req.iUid = SELF_UID;

		// Should be allowed to load Symbian ELF
		TRAPD(ret, iSess.PluginRequestL( req ));
		if(ret != KErrNone)
			{
			INFO_PRINTF2(_L("Failed loading plugin with UID 0x%X  -- SELF Plugin/n"), req.iUid );
			User::Leave(ret);
			}
		
		//Now we need to look in the plugin list and make sure the SELF plugin isnt there
		RPluginList plugins;
		CleanupClosePushL(plugins);
		
		TRAP(ret, iSess.GetPluginListL(plugins));
		if (ret != KErrNone)
			{
			INFO_PRINTF2(_L("Failed to get plugin list -- SELF Plugin. Error: %d"), ret);
			User::Leave(ret);
			}
		
		//Now we look through the list, until we find our plugin, its a fail
		SetTestStepResult(EFail);
		
		for (TInt i = 0; i < plugins.Count(); i++)
			{
			if (plugins[i].iUid ==  SELF_UID && plugins[i].iLoaded)
				{
				SetTestStepResult(EPass);
				INFO_PRINTF1(_L("SELF Plugin loaded successfully"));
				CleanupStack::PopAndDestroy();//take plugins off cleanupstack
				return;
				}
			}
		
		INFO_PRINTF1(_L("SELF Plugin failed to load successfully"));
		CleanupStack::PopAndDestroy();

		}	
	}

void CSELFPluginStep::UnloadPluginL()
/**
 * @return void
 * Tests the plugin can be successfully unloaded
 */
	{
	if(TestStepResult()==EPass)
		{
		INFO_PRINTF1(_L("Attempting to unload SELF Plugin"));
		
		TPluginRequest req;
		req.iPluginType = TPluginRequest::EFormatter;
		req.iLoad = EFalse;
		req.iUid = SELF_UID;
		
		TRAPD(ret, iSess.PluginRequestL( req ));
		if(ret != KErrNone)
			{
			INFO_PRINTF2(_L("Failed unloading plugin with UID 0x%X  -- SELF Plugin"), req.iUid );
			User::Leave(ret);
			}
		
		//Now we need to look in the plugin list and make sure the SELF plugin isnt there
		RPluginList plugins;
		CleanupClosePushL(plugins);
		
		TRAP(ret, iSess.GetPluginListL(plugins));
		if (ret != KErrNone)
			{
			INFO_PRINTF2(_L("Failed to get plugin list -- SELF Plugin. Error: %d"), ret);
			User::Leave(ret);
			}
		
		//Now we look through the list, until we find our plugin, its a fail
		SetTestStepResult(EFail);
		
		for (TInt i = 0; i < plugins.Count(); i++)
			{
			if (plugins[i].iUid ==  SELF_UID && !plugins[i].iLoaded)
				{
				SetTestStepResult(EPass);
				INFO_PRINTF1(_L("SELF Plugin unloaded successfully"));
				CleanupStack::PopAndDestroy();
				return;
				}
			}
		
		INFO_PRINTF1(_L("SELF Plugin failed to unload successfully"));
		CleanupStack::PopAndDestroy();	
		}		
	}

void CSELFPluginStep::UnloadPluginL(const TInt aIndex)
/**
 * @return void
 * Tests the plugin can be successfully unloaded
 */
	{
	if(TestStepResult()==EPass)
		{
		INFO_PRINTF2(_L("Attempting to unload SELF Plugin. Index = [%d]"), aIndex);
		
		TPluginRequest req;
		req.iPluginType = TPluginRequest::EFormatter;
		req.iLoad = EFalse;
		req.iUid = SELF_UID;
		
		TRAPD(ret, iSess.PluginRequestL( req ));
		if(ret != KErrNone)
			{
			INFO_PRINTF2(_L("Failed unloading plugin with UID 0x%X  -- SELF Plugin"), req.iUid );
			User::Leave(ret);
			}
		
		INFO_PRINTF1(_L("SELF Plugin unloaded successfully"));
		}		
	}

void CSELFPluginStep::LoadMultiplePluginsL(const TInt aCount)
/**
 * @return void
 * This tests behaviour when we attempt to load an already loaded SELF plugin.
 * Should only be loaded once, even after calling twice
 */
	{
	if (TestStepResult()==EPass)
		{	
		INFO_PRINTF1(_L("Testing loading the SELF plugin multiple times"));
        for(TInt i = 0; i < aCount; i++)
            {
		    LoadPluginL();	
            }
		
		//Now we need to look in the plugin list and make sure the DEXEC plugin isnt there multiple times
		RPluginList plugins;
		CleanupClosePushL(plugins);
		
		TRAPD(ret, iSess.GetPluginListL(plugins));
		if (ret != KErrNone)
			{
			INFO_PRINTF2(_L("Failed to get plugin list -- SELF Plugin. Error: %d"), ret);
			User::Leave(ret);
			}
		
		TInt selfCounter = 0;
		for (TInt i = 0; i < plugins.Count(); i++)
			{
			if (plugins[i].iUid ==  SELF_UID && plugins[i].iLoaded)
				{
                selfCounter++;
				break;
				}
			}
		
        if(selfCounter == 0)
        {
	        SetTestStepResult(EFail);
			INFO_PRINTF1(_L("Fail - SELF plugin has been not marked as loaded"));
        }

        RPluginPointerList formatterList;
        TRAP(ret, iSess.GetFormattersL(formatterList));
        selfCounter = 0;
        for(TInt i = 0; i < formatterList.Count(); i++)
        {
            if(TUid::Uid(formatterList[i]->Uid()) == SELF_UID)
            {
            selfCounter++;
            }
        }
        formatterList.ResetAndDestroy();

		if(selfCounter != aCount)
			{
			SetTestStepResult(EFail);
			INFO_PRINTF2(_L("Fail - SELF plugin has not been found loaded %d times"), aCount);
			}
		else
			{
			INFO_PRINTF1(_L("Loading plugin multiple times is ok"));
			}
		
		CleanupStack::PopAndDestroy();
		}	
	}

void CSELFPluginStep::UnloadMultiplePluginsL(const TInt aCount)
/**
 * @return void
 * This tests behaviour when we attempt to unload the SELF plugin when it has
 * not been loaded
 */
   	{
	if (TestStepResult()==EPass)
		{
		INFO_PRINTF2(_L("Testing unloading the SELF plugin multiple times - %d"), aCount);
        for(TInt i = aCount; i > 0; i--)
            {
		    UnloadPluginL(i - 1);	
            }
		
		//Now we need to look in the plugin list and make sure the SELF plugin isnt there
		RPluginList plugins;
		CleanupClosePushL(plugins);
		
		TRAPD(ret, iSess.GetPluginListL(plugins));
		if (ret != KErrNone)
			{
			INFO_PRINTF2(_L("Failed to get plugin list -- SELF Plugin. Error: %d"), ret);
			User::Leave(ret);
			}
		
		TInt selfCounter = aCount;
		for (TInt i = 0; i < plugins.Count(); i++)
			{
			if (plugins[i].iUid ==  SELF_UID && plugins[i].iLoaded)
				{
                selfCounter = 0;
				break;
				}
			}
	
        if(selfCounter == 0)
        {
			SetTestStepResult(EFail);
			INFO_PRINTF1(_L("Fail - SELF plugin has been found marked as loaded"));
        }

        RPluginPointerList formatterList;
        TRAP(ret, iSess.GetFormattersL(formatterList));
        selfCounter = 0;
        for(TInt i = 0; i < formatterList.Count(); i++)
        {
            if(TUid::Uid(formatterList[i]->Uid()) == SELF_UID)
            {
            selfCounter++;
            }
        }
        formatterList.ResetAndDestroy();

		if(selfCounter > 0)
			{
			SetTestStepResult(EFail);
			INFO_PRINTF2(_L("Fail - SELF %d plugins has been found loaded"), selfCounter);
			}
		else
			{
			INFO_PRINTF1(_L("Unloading plugin multiple times is ok"));
			}
		
		CleanupStack::PopAndDestroy();
		}
	}

TVerdict CSELFPluginStep::doTestStepPostambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	iSess.Disconnect();
	User::After(10000000); //ensure we give enough time for session to close
	return EPass;	
	}
