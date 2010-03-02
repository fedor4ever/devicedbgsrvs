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
 @file DEXECPlugin.cpp
 @internalTechnology
*/
#include "SELFPluginStep.h"
#include "tcoredumpserversuitedefs.h"
#include "DEXCpluginstep.h"

CDEXECPluginStep::~CDEXECPluginStep()
/**
 * Destructor
 */
	{
	}

CDEXECPluginStep::CDEXECPluginStep()
/**
 * Constructor
 */
	{
	// **MUST** call SetTestStepName in the constructor as the controlling
	// framework uses the test step name immediately following construction to set
	// up the step's unique logging ID.
	SetTestStepName(KSELFPluginStep);
	}

TVerdict CDEXECPluginStep::doTestStepPreambleL()
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

TVerdict CDEXECPluginStep::doTestStepL()
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
			INFO_PRINTF2(_L("Error %d from CDEXECPluginStep->ClientAppL()"), ret);
			}
		
		__UHEAP_MARKEND;

		}
	return TestStepResult();  
	}

void CDEXECPluginStep::ClientAppL()
/**
 * @return void
 * This calls each stage of the test
 */
	{
	INFO_PRINTF1(_L("Starting DEXEC Plugin Test Suite") );

    LoadPluginL();
    UnloadPluginL();
    for(TInt i = 1; i < DexcMultipleTries; i++)
        {
        LoadMultiplePluginsL(i);
        UnloadMultiplePluginsL(i);
        }
	}

void CDEXECPluginStep::LoadPluginL()
/**
 * @return void
 * Tests that the plugin can be loaded correctly
 */
	{		
	if (TestStepResult()==EPass)
		{
			
		INFO_PRINTF1(_L("Attempting to load DEXEC Plugin"));
		
		TPluginRequest req;
		req.iPluginType = TPluginRequest::EFormatter;
		req.iLoad = ETrue;
		req.iUid = DEXEC_UID;

		// Should be allowed to load DEXEC
		TRAPD(ret, iSess.PluginRequestL( req ));
		if(ret != KErrNone)
			{
			INFO_PRINTF2(_L("Failed loading plugin with UID 0x%X  -- DEXEC Plugin/n"), req.iUid );
			User::Leave(ret);
			}

		RPluginList plugins;
		CleanupClosePushL(plugins);
       	TRAP(ret, iSess.GetPluginListL(plugins));
		TInt dexecCounter = 0;
		for (TInt i = 0; i < plugins.Count(); i++)
			{
			if (plugins[i].iUid ==  DEXEC_UID && plugins[i].iLoaded)
				{
                dexecCounter++;
				break;
				}
			}
        CleanupStack::PopAndDestroy();
		
        if(dexecCounter != 1)
        {
	        SetTestStepResult(EFail);
			INFO_PRINTF1(_L("Fail - DEXEC plugin has been not marked as loaded"));
            return;
        }

		INFO_PRINTF1(_L("DEXEC Plugin loaded successfully"));
		}	
	}

void CDEXECPluginStep::UnloadPluginL()
/**
 * @return void
 * Tests the plugin can be successfully unloaded
 */
	{
	if(TestStepResult()==EPass)
		{
		INFO_PRINTF1(_L("Attempting to unload DEXEC Plugin"));
		
		TPluginRequest req;
		req.iPluginType = TPluginRequest::EFormatter;
		req.iLoad = EFalse;
		req.iUid = DEXEC_UID;
		
		TRAPD(ret, iSess.PluginRequestL( req ));
		if(ret != KErrNone)
			{
			INFO_PRINTF2(_L("Failed unloading plugin with UID 0x%X  -- DEXEC Plugin"), req.iUid );
			User::Leave(ret);
			}

		RPluginList plugins;
		TRAP(ret, iSess.GetPluginListL(plugins));
		if (ret != KErrNone)
			{
			INFO_PRINTF2(_L("Failed to get plugin list -- DEXEC Plugin. Error: %d"), ret);
			User::Leave(ret);
			}
		
		TInt dexecCounter = 0;
		for (TInt i = 0; i < plugins.Count(); i++)
			{
			if (plugins[i].iUid ==  DEXEC_UID && plugins[i].iLoaded)
				{
                dexecCounter++;
				break;
				}
			}
        plugins.Close();
		
        if(dexecCounter != 0)
        {
	        SetTestStepResult(EFail);
			INFO_PRINTF1(_L("Fail - DEXEC plugin has been not marked as unloaded"));
            return;
        }

		INFO_PRINTF1(_L("DEXEC Plugin unloaded successfully"));
		}		
	}

void CDEXECPluginStep::UnloadPluginL(const TInt aIndex)
/**
 * @return void
 * Tests the plugin can be successfully unloaded
 */
	{
	if(TestStepResult()==EPass)
		{
		INFO_PRINTF1(_L("Attempting to unload DEXEC Plugin"));
		
		TPluginRequest req;
		req.iPluginType = TPluginRequest::EFormatter;
		req.iLoad = EFalse;
		req.iUid = DEXEC_UID;
        req.iIndex = aIndex;
        req.iPair = aIndex;
		
		TRAPD(ret, iSess.PluginRequestL( req ));
		if(ret != KErrNone)
			{
			INFO_PRINTF2(_L("Failed unloading plugin with UID 0x%X  -- DEXEC Plugin"), req.iUid );
			User::Leave(ret);
			}
		
		INFO_PRINTF1(_L("DEXEC Plugin unloaded successfully"));
		}		
	}

void CDEXECPluginStep::LoadMultiplePluginsL(const TInt aCount)
/**
 * @return void
 * This tests behaviour when we attempt to load an already loaded DEXEC plugin.
 * Should only be loaded once, even after calling twice
 */
	{
	if (TestStepResult()==EPass)
		{	
		INFO_PRINTF2(_L("Testing loading the DEXEC plugin multiple times  - %d"), aCount);
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
			INFO_PRINTF2(_L("Failed to get plugin list -- DEXEC Plugin. Error: %d"), ret);
			User::Leave(ret);
			}
		
		TInt dexecCounter = 0;
		for (TInt i = 0; i < plugins.Count(); i++)
			{
			if (plugins[i].iUid ==  DEXEC_UID && plugins[i].iLoaded)
				{
                dexecCounter++;
				break;
				}
			}
		
        if(dexecCounter == 0)
        {
	        SetTestStepResult(EFail);
			INFO_PRINTF1(_L("Fail - DEXEC plugin has been not marked as loaded"));
        }

        RPluginPointerList formatterList;
        TRAP(ret, iSess.GetFormattersL(formatterList));
        dexecCounter = 0;
        for(TInt i = 0; i < formatterList.Count(); i++)
        {
            if(TUid::Uid(formatterList[i]->Uid()) == DEXEC_UID)
            {
            dexecCounter++;
            }
        }
        formatterList.ResetAndDestroy();

		if(dexecCounter != aCount)
			{
			SetTestStepResult(EFail);
			INFO_PRINTF2(_L("Fail - DEXEC plugin has not been found loaded %d times"), aCount);
			}
		else
			{
			INFO_PRINTF1(_L("Loading plugin multiple times is ok"));
			}
		
		CleanupStack::PopAndDestroy();
		}	
	}

void CDEXECPluginStep::UnloadMultiplePluginsL(const TInt aCount)
/**
 * @return void
 * This tests behaviour when we attempt to unload the SELF plugin when it has
 * not been loaded
 */
	{
	if (TestStepResult()==EPass)
		{
		INFO_PRINTF2(_L("Testing unloading the DEXEC plugin multiple times - %d"), aCount);
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
			INFO_PRINTF2(_L("Failed to get plugin list -- DEXEC Plugin. Error: %d"), ret);
			User::Leave(ret);
			}
		
		TInt dexecCounter = aCount;
		for (TInt i = 0; i < plugins.Count(); i++)
			{
			if (plugins[i].iUid ==  DEXEC_UID && plugins[i].iLoaded)
				{
                dexecCounter = 0;
				break;
				}
			}
	
        if(dexecCounter == 0)
        {
			SetTestStepResult(EFail);
			INFO_PRINTF1(_L("Fail - DEXEC plugin has been found marked as loaded"));
        }

        RPluginPointerList formatterList;
        TRAP(ret, iSess.GetFormattersL(formatterList));
        dexecCounter = 0;
        for(TInt i = 0; i < formatterList.Count(); i++)
        {
            if(TUid::Uid(formatterList[i]->Uid()) == DEXEC_UID)
            {
            dexecCounter++;
            }
        }
        formatterList.ResetAndDestroy();

		if(dexecCounter > 0)
			{
			SetTestStepResult(EFail);
			INFO_PRINTF2(_L("Fail - DEXEC %d plugins has been found loaded"), dexecCounter);
			}
		else
			{
			INFO_PRINTF1(_L("Unloading plugin multiple times is ok"));
			}
		
		CleanupStack::PopAndDestroy();
		}
	}

TVerdict CDEXECPluginStep::doTestStepPostambleL()
/**
 * @return - TVerdict code
 * Override of base class virtual
 */
	{
	iSess.Disconnect();
	return EPass;	
	}



