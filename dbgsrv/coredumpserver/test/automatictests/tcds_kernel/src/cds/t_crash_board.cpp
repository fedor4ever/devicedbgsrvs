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
 @file t_crash_board.cpp
 @internalTechnology
*/

#include <coredumpinterface.h>
#include "t_crash_board.h"

//Names of functions to be tested - referenced from script file
_LIT(KCrashBoard, "CrashBoard");
_LIT(KCleanFlashPartition, "CleanFlashPartition");
_LIT(KCleanEntireFlashPartition, "CleanEntireFlashPartition");


/**
 * Constructor for test wrapper
 */
CCrashBoardWrapper::CCrashBoardWrapper()
	{
	}

/**
 * Destructor
 */
CCrashBoardWrapper::~CCrashBoardWrapper()
	{
    }

/**
 * Two phase constructor for CCrashBoardWrapper
 * @return CCrashBoardWrapper object
 * @leave
 */
CCrashBoardWrapper* CCrashBoardWrapper::NewL()
	{
	CCrashBoardWrapper* ret = new (ELeave) CCrashBoardWrapper();
	CleanupStack::PushL(ret);
	ret->ConstructL();
	CleanupStack::Pop(ret);
	return ret;
	}

/**
 * Safe construction
 * @leave
 */
void CCrashBoardWrapper::ConstructL()
	{
	}

/**
 * Assign the object
 *
 * @param aObject TAny* to the object to test
 * @leave
 */
void CCrashBoardWrapper::SetObjectL(TAny* aObject)
	{
    INFO_PRINTF1(_L("GETS CALLED CCrashBoardWrapper::SetObjectL"));
	}

/**
 * Runs a test preamble
 */
void CCrashBoardWrapper::PrepareTestL()
	{
	SetBlockResult(EPass);

	INFO_PRINTF1(_L("Ready to start test"));
	}

/**
 * Process command to see what test to run
 */
TBool CCrashBoardWrapper::DoCommandL(const TTEFFunction& aCommand, const TTEFSectionName& aSection, const TInt aAsyncErrorIndex)
	{
	__UHEAP_MARK;
	
	PrepareTestL();

	if (KCrashBoard() == aCommand)
		{
		INFO_PRINTF1(_L("CCrashBoardWrapper::DoCommandL: Inside Command KCrashBoard"));
		DoCmdCrashBoardL(aSection);
		}
	else if (KCleanFlashPartition() == aCommand)
		{
		INFO_PRINTF1(_L("CCrashBoardWrapper::DoCommandL: Inside Command KCleanFlashPartition"));
		DoCmdCleanCrashPartition();
		}
	else if (KCleanEntireFlashPartition() == aCommand)
		{
		INFO_PRINTF1(_L("CCrashBoardWrapper::DoCommandL: Inside Command KCleanEntireFlashPartition"));
		DoCmdCleanEntireCrashPartitionL();
		}
	else
		{
        INFO_PRINTF1(_L("CCrashBoardWrapper::DoCommandL: NOT ** Inside Any Command"));
		return EFalse;
		}
	RDebug::Printf("Returnng from DoCommandL");
	__UHEAP_MARKEND;

	return ETrue;
	}

void CCrashBoardWrapper::DoCmdCleanCrashPartition()
	{
	INFO_PRINTF1(_L("CCrashBoardWrapper::DoCmdCleanCrashPartition - Deleting entire crash flash partition"));
	
	RCoreDumpSession session;
	session.Connect();
	CleanupClosePushL(session);
	
	session.DeleteCrashPartitionL();
	
	INFO_PRINTF1(_L("CCrashBoardWrapper::DoCmdCleanCrashPartition - Deleting entire crash flash partition succesful"));
	
	CleanupStack::PopAndDestroy(&session);
	}

/**
 * This tests deletes the entire crash flash partition using the DSS
 * @leave
 */
void CCrashBoardWrapper::DoCmdCleanEntireCrashPartitionL()
	{
	INFO_PRINTF1(_L("CCrashBoardWrapper::DoCmdCleanEntireCrashPartitionL - Deleting entire crash flash partition using the DSS"));
	TVersion secVers(Debug::KDebugServMajorVersionNumber, Debug::KDebugServMinorVersionNumber, Debug::KDebugServPatchVersionNumber);
	User::LeaveIfError(iSecSess.Connect(secVers));
	
	INFO_PRINTF1(_L("Erasing the flash EraseCrashFlashPartition using DSS"));
	iSecSess.EraseCrashFlashPartition();
	
	INFO_PRINTF1(_L("Erasing the flash EraseCrashFlashPartition using DSS Successful"));
	iSecSess.Close();
	
	}
/**
 * This tests that we can succesfully construct
 * @leave
 */
void CCrashBoardWrapper::DoCmdCrashBoardL(const TDesC& aSection)
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

	RProcess crashProcess;
	CleanupClosePushL(crashProcess);

	TInt ret = crashProcess.Create( KCrashAppFileName, crashAppParam);
	if(ret != KErrNone)
		{
		ERR_PRINTF2( _L("Failed to create process CRASHAPP ret=%d\n"), ret );
		SetBlockResult(EFail);
		User::Leave(ret);
		}
    
    //resume the crashapp process
    crashProcess.Resume();

    CleanupStack::PopAndDestroy(&crashProcess); //RProcess crashProcess

	INFO_PRINTF1(_L("DoCmdCrashL DONE"));
	}
//eof

