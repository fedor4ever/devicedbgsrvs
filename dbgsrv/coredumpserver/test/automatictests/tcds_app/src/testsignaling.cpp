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
// Example CTestStep derived implementation
//

/**
 @file testsignaling.cpp
 @internalTechnology
*/

#include <crashdatasave.h>
#include <s32file.h>
#include <bautils.h>

#include "testsignaling.h"
#include "tcoredumpserversuitedefs.h"
#include "tcoredumpserverSuiteServer.h"

/* Null UID set for ECrashProgress which does not have UID */
const TUint KNULLUID = 999;

/**
 * Constructor
 */
CTestSignaling::CTestSignaling()

    {
    // **MUST** call SetTestStepName in the constructor as the controlling
    // framework uses the test step name immediately following construction to set
    // up the step's unique logging ID.
    SetTestStepName (KTestSignaling);
    }

/**
 * Destructor
 */
CTestSignaling::~CTestSignaling()
    {
    }

/**
 * @return - TVerdict code
 * Override of base class virtual
 * establishing connection to core dump server and file server
 * Property handles created and also the corresponding buffers
 */
TVerdict CTestSignaling::doTestStepPreambleL()
    {
    TInt ret = KErrNone;
    SetTestStepResult (EPass);

    ret = iSess.Connect ();
    if ( ret != KErrNone)
        {
        SetTestStepResult ( EFail);
        ERR_PRINTF2 (_L ("Error %d from iSess->Connect()/n"), ret);
        User::Leave(ret);
        }

    ret = iFs.Connect ();
    if ( ret != KErrNone)
        {
        SetTestStepResult ( EFail);
        ERR_PRINTF2 (_L ("Error %d from iFs->Connect()/n"), ret);
        User::Leave(ret);
        }

    ret = iCrashProgress.Attach(KCoreDumpServUid, ECrashProgress);
    if ( ret != KErrNone)
        {
        SetTestStepResult ( EFail);
        ERR_PRINTF2 (_L ("Error %d from iCrashProgress.Attach()/n"), ret);
        User::Leave(ret);
        }

    ret = iCrashMediaName.Attach(KCoreDumpServUid, ECrashMediaName);
    if ( ret != KErrNone)
        {
        SetTestStepResult ( EFail);
        ERR_PRINTF2 (_L ("Error %d from iCrashMediaName.Attach()/n"), ret);
        User::Leave(ret);
        }

    ret = iCrashCancel.Attach(KCoreDumpServUid, ECancelCrash);
    if ( ret != KErrNone)
        {
        SetTestStepResult ( EFail);
        ERR_PRINTF2 (_L ("Error %d from iCrashCancel.Attach()/n"), ret);
        User::Leave(ret);
        }

    ret = iCrashProgressBuffer.Create(KMaxCrashProgressBuffer);
    if ( ret != KErrNone)
        {
        SetTestStepResult ( EFail);
        ERR_PRINTF2 (_L ("Error %d from iCrashProgressBuffer.Create()/n"), ret);
        User::Leave(ret);
        }

    ret = iCrashMediaNameBuffer.Create(KNumofEntries * KMaxEntryLength);
    if ( ret != KErrNone)
        {
        SetTestStepResult ( EFail);
        ERR_PRINTF2 (_L ("Error %d from iCrashMediaNameBuffer.Create()/n"), ret);
        User::Leave(ret);
        }

    return TestStepResult();
    }

/**
 * @return - TVerdict code
 * Override of base class virtual
 * releasing the sessions to the servers
 * closing the RProperty handles and buffers
 */
TVerdict CTestSignaling::doTestStepPostambleL()
    {
    iSess.Disconnect();
    User::After(10000000); //ensure we give enough time for session to close
    iFs.Close();

    iCrashProgress.Close();
    iCrashMediaName.Close();
    iCrashCancel.Close();

    iCrashProgressBuffer.Close();
    iCrashMediaNameBuffer.Close();

    return EPass;
    }

/**
 * @return - TVerdict code
 * Override of base class pure virtual
 * Our implementation only gets called if the base class doTestStepPreambleL() did
 * not leave. That being the case, the current test result value will be EPass.
 */
TVerdict CTestSignaling::doTestStepL()
    {
    if(TestStepResult() == EPass)
        {
        TInt ret = KErrNone;

        __UHEAP_MARK;

        TRAP (ret, ClientAppL ());
        if (KErrNone != ret)
            {
            SetTestStepResult (EFail);
            ERR_PRINTF2 (
                    _L ("Error %d from CTestFormatterUserSideStep->ClientAppL()"),
                    ret);
            User::Leave(ret);
            }

        __UHEAP_MARKEND;

        }
    return TestStepResult ();
    }

/**
 * @return void
 * This calls each stage of the test
 */
void CTestSignaling::ClientAppL()
    {

    INFO_PRINTF1 (_L ("Starting Test Signaling Mechanism Test Suite"));
    TestSignalingMechanismL();

    }

/**
 * @return void
 * Tests the signaling mechanism through RProperty updates
 * checks for the crash progress and crash media name property updates
 */
void CTestSignaling::TestSignalingMechanismL()
    {
    if (TestStepResult() == EPass)
        {
        //reading configuration from the ini file
        TPtrC crashAppParam;
        TPtrC crashFileName;
        TPtrC testNumberofCrashes;

        if(!GetStringFromConfig(ConfigSection(), KTe_CrashAppParam, crashAppParam) ||
           !GetStringFromConfig(ConfigSection(), KTe_CrashFileName, crashFileName) ||
           !GetStringFromConfig(ConfigSection(), KNumberofCrashes, testNumberofCrashes))
            {
            ERR_PRINTF1(_L("Failed to get data from the ini file"));
            SetTestStepResult(EFail);
            User::Leave(KErrGeneral);
            }

        //Convert number of crashes
        TLex luther(testNumberofCrashes);
        User::LeaveIfError(luther.Val(iTimestoCrash));

        if(iTimestoCrash < KNumofEntries)
            {
            ERR_PRINTF3(_L ("Expecting the number of crashes %d to be more than %d"), iTimestoCrash, KNumofEntries);
            SetTestStepResult(EFail);
            User::Leave(KErrGeneral);
            }

        INFO_PRINTF2(_L("Number of crashes to happen %d"), iTimestoCrash);

        iCrashFileName.Copy(crashFileName);

        // setting the crash media name to zero
        // ** caution this is just done for testing **
        ZeroCrashMediaNameL();

        // Load the SELF Formatter and File Writer
        LoadandConfigurePluginsL(crashFileName);

        //lets do the first crash
        //start the application that would crash and observe it
        StartCrashAppL(crashAppParam);

        // check for the presence of UID in the crash progress messages
        CheckUIDCrashProgressL();
        iNumberofCrashes++;

        // breather -- TODO: Why is this here? Document or remove
        User::After(1000000);

        // create more crashes to create iTimestoCrash - 1 crashes
        // and check the ECrashMediaName buffer content
        for (TInt i = 0; i < (iTimestoCrash - 1); i++ )
            {

            StartCrashAppL(crashAppParam);

            //CheckUIDCrashProgressL();
            CheckCrashMediaNameL();

			// TODO: Why is this here? Document or remove
            User::After(1000000);
            }

        // now we have done multiple crashes
        // and we would like to investigate the crash media name buffer
        TestCrashMessageBufferL();

        //now going to test the cancel crash scenario
        StartCrashAppL(crashAppParam);

        CheckCancelCrashL();

		TidyFilesL(crashFileName);

        INFO_PRINTF1 (_L ("\nSuccessfully tested the signaling mechanism! Adios!\n"));

        }

    }

/**
 * Deletes the crash files generated by this test to prevent corruption
 * of future test
 * @param aFilenameRoot root of filename to remove (we will remove aFilenameRoot*)
 * @leave One of the system wide error codes
 */
void CTestSignaling::TidyFilesL(const TDesC& aFilenameRoot)
	{
	_LIT(KWildCardMatch, "*");

	TBuf<KMaxFileName> file;
	file.Append(aFilenameRoot);
	file.Append(KWildCardMatch);

	BaflUtils::DeleteFile(iFs, file);
	}

/**
 * @return void
 * @param parameters for crashapp/exe
 * Starts the crashapp.exe and observe for the crash via CDS
 */
void CTestSignaling::StartCrashAppL(const TDesC& aCrashAppParam)
    {

    INFO_PRINTF2(_L ("Executing crashapp.exe %S \n"), &aCrashAppParam);

    //Start the process that we intend to crash....
    RProcess crashProcess;
    CleanupClosePushL (crashProcess);

    TInt ret = crashProcess.Create(KCrashAppFileName, aCrashAppParam);
    if ( ret != KErrNone)
        {
        ERR_PRINTF2 (
             _L ("Error %d from RProcess.Create(z:\\sys\\bin\\crashapp.exe)/n"),
             ret);
        SetTestStepResult (EFail);
        User::Leave(ret);
        }

    // Observe the process and wait for a crash
    TRAP (ret, iSess.ObservationRequestL( KCrashAppFileName, KCrashAppFileName, ETrue));
    if ( ret != KErrNone)
      {
      ERR_PRINTF2 (
              _L ("Error %d iSess.ObservationRequestL(z:\\sys\\bin\\crashapp.exe)\n"),
              ret);
      SetTestStepResult(EFail);
      User::Leave(ret);
      }

    //start the crash process
    crashProcess.Resume ();
    CleanupStack::PopAndDestroy (&crashProcess);
    }

/**
 * @return void
 * Check for the presence of UID in the crash progress messages
 * also check for messages if they have the same crash UID
*/
void CTestSignaling::CheckUIDCrashProgressL()
    {
    INFO_PRINTF1(_L ("Subscribing and Reading ECrashProgress Messages"));

    TUint uid = 0;
    TUint saveduid = KNULLUID;
    TRequestStatus status;
    TInt i = 0;

    iCrashProgressBuffer.Zero();
    User::LeaveIfError(iCrashProgress.Get(iCrashProgressBuffer));

    // read the crash UID and the crash progress message present
    ReadCrashProgressPacketL(uid, iCrashProgressBuffer);

    if(uid != KNULLUID)
        {
        ERR_PRINTF1(_L("First crashprogress message should no crash uid in it"));
        SetTestStepResult(EFail);
        User::Leave(KErrGeneral);
        }

    INFO_PRINTF2(_L ("CrashProgress Messages %S"), &iCrashProgressBuffer);

    //Now we read the crash progress messages until the processing is finished
    do
        {
        i++;

        //start the subscription to see change in the property ECrashProgress
        iCrashProgress.Subscribe(status);
        User::WaitForRequest (status);
        User::LeaveIfError(iCrashProgress.Get(iCrashProgressBuffer));

        INFO_PRINTF2(_L ("CrashProgress Messages %S"), &iCrashProgressBuffer);

        // read the crash UID and the crash progress message present
        ReadCrashProgressPacketL(uid, iCrashProgressBuffer);

        // check for first message to be "Start"
        if ( (1 == i) &&  (iCrashProgressBuffer.CompareF(_L("Start")) != 0) )
            {
            // the first message has to be Start
            ERR_PRINTF2(_L("First crashprogress message %S should have Start in it"), &iCrashProgressBuffer);
            SetTestStepResult(EFail);
            User::Leave(KErrGeneral);
            }
        else if ( (1 == i) &&  (iCrashProgressBuffer.CompareF(_L("Start")) == 0) )
            {
            // save the crash UID for future checks
            if (uid != KNULLUID)
                SaveCrashUIDL(uid);
            }
        else if( (saveduid != KNULLUID) && (uid != KNULLUID) )
            {
            // chec if all the messages do have the same crash UID
            if(saveduid != uid)
                {
                ERR_PRINTF3(_L("Mismatch in the UID values saveduid %S does not match with uid %S"), saveduid, uid);
                SetTestStepResult(EFail);
                User::Leave(KErrGeneral);
                }
            }

        saveduid = uid;
        }
    while(iCrashProgressBuffer != KEndOfProcessing);

    iCrashProgressBuffer.Zero();

    INFO_PRINTF1(_L ("Succesful Read all the ECrashProgress Messages"));
    }

/**
 * @return void
 * Check for the the crash media name messages
 * parses the values and validates the crash media name and the status bit
*/
void CTestSignaling::CheckCrashMediaNameL()
    {
    INFO_PRINTF1(_L ("Validate the Crash Media Name Messages"));

    TUint crashUID = 0;
    TBuf<KMaxStatusLength> statusbit;
    TRequestStatus status;

    do
        {
        // subscribe to the Crash Progress Property
        // intention is to be notified of the crash processing start
        iCrashProgress.Subscribe(status);
        User::WaitForRequest(status);

        User::LeaveIfError(iCrashProgress.Get(iCrashProgressBuffer));
        // extract the crash UID and the crash progress message
        ReadCrashProgressPacketL(crashUID, iCrashProgressBuffer);

        if(iCrashProgressBuffer.CompareF(_L("Start")) == 0)
            {
            // now read the crash media name which should be updated
            // no need to subscribe for ECrashMediaName as it has already been updated
            User::LeaveIfError(iCrashMediaName.Get(iCrashMediaNameBuffer));
            INFO_PRINTF2(_L ("Crash Media Name Messages %S"), &iCrashMediaNameBuffer);

            // read the crash media name entry for the particular crash uid
            ParseMediaNameBufferL(crashUID, iCrashMediaNameBuffer, statusbit);

            //check if the crash status bit "I" i.e it is still In Progress
            if(statusbit.CompareF(_L("I")) != 0)
                {
                ERR_PRINTF2(_L("Incorrect status bit %S expecting it to be I"), &statusbit);
                SetTestStepResult(EFail);
                User::Leave(KErrGeneral);
                }

            // check if the CrashFile name is valid name
            iCrashFileName.Append(_L("*"));
            if (iCrashMediaNameBuffer.MatchF(iCrashFileName) == KErrNotFound)
                {
                ERR_PRINTF3(_L("Crash Media Name %S does not match with %S"), &iCrashFileName, &iCrashMediaNameBuffer);
                SetTestStepResult(EFail);
                User::Leave(KErrGeneral);
                }

            // happy to save the crash UID for future checks
            SaveCrashUIDL(crashUID);

            }

        }
    while(iCrashProgressBuffer != KEndOfProcessing);

    //now check the status bit message
    User::LeaveIfError(iCrashMediaName.Get(iCrashMediaNameBuffer));
    INFO_PRINTF2(_L ("Crash Media Name Messages %S"), &iCrashMediaNameBuffer);
    // parse the crash media name message
    ParseMediaNameBufferL(iCrashUID[iNumberofCrashes - 1], iCrashMediaNameBuffer, statusbit);

    //check if the crash status bit "D" i.e the crash processing is now done
    if(statusbit.CompareF(_L("D")) != 0)
        {
        ERR_PRINTF2(_L("Incorrect status bit %S supposed to be D"), &statusbit);
        SetTestStepResult(EFail);
        User::Leave(KErrGeneral);
        }

    INFO_PRINTF1(_L ("Successful checked the Crash Media Name Messages"));
    }

/**
 * @return void
 * Check for the the crash media name messages after multiple crashes has happened
 * expects that the number of crashes is more than the KNumofEntries
 * parses the crash media name buffer and validates the status bit
 * also checks if we have all the latest crashes in the crash media buffer
*/
void CTestSignaling::TestCrashMessageBufferL()
    {
    INFO_PRINTF1(_L ("Test the final Crash Media Name Message Buffer"));

    // read the crash media name buffer
    iCrashMediaNameBuffer.Zero();
    User::LeaveIfError(iCrashMediaName.Get(iCrashMediaNameBuffer));
    INFO_PRINTF2(_L("Final Crash Media name Buffer %S"), &iCrashMediaNameBuffer);

    TInt numofEntries = iCrashMediaNameBuffer.Length() / KMaxEntryLength;
    // expects that the number of crashes is greater than  KNumofEntries
    if (numofEntries != KNumofEntries)
        {
        ERR_PRINTF2(_L("%d Entries is not equal to the max length"), numofEntries);
        SetTestStepResult(EFail);
        User::Leave(KErrGeneral);
        }

    // check for the buffer consistency
    if( (iCrashMediaNameBuffer.Length() % KMaxEntryLength) != 0  )
        {
        ERR_PRINTF2(_L("Buffer corrupt iCrashMediaNameBuffer.Length() %d"), iCrashMediaNameBuffer.Length());
        SetTestStepResult(EFail);
        User::Leave(KErrGeneral);
        }

    // run through the buffer
    for(TInt i = 0; i < numofEntries; i++ )
        {
        // check for the status bit
        // used as a bit to check on the validity of the entry
        TPtrC statusptr = iCrashMediaNameBuffer.Mid((i * KMaxEntryLength), KMaxStatusLength);

        if ((statusptr.CompareF(_L("D")) != 0) )
          {
          ERR_PRINTF2(_L("Entry Number %d does not seem to have complete status"), i);
          SetTestStepResult(EFail);
          User::Leave(KErrGeneral);
          }

        // now check if we have all the latest crashes by checking on the crash UID
        // check for UID and match with the saved UID values
        // we have been saving the crash UID in the right fashion always keeping the latest crashes
        TPtrC uidptr = iCrashMediaNameBuffer.Mid(((i * KMaxEntryLength) + KMaxStatusLength), KMaxUIDLength);

        TUint uid = 0;
        TLex lexuid(uidptr);
        User::LeaveIfError(lexuid.Val(uid));

        if ( uid != iCrashUID[i])
            {
            ERR_PRINTF3(_L("Entry Number %d does not match with iCrashUID %d"), uid, iCrashUID[i]);
            SetTestStepResult(EFail);
            User::Leave(KErrGeneral);
            }

        //check for the terminating character '/'
        TPtrC endptr = iCrashMediaNameBuffer.Mid((((i + 1) * KMaxEntryLength) - KMaxEndLength), KMaxEndLength);

        if(endptr.CompareF(_L("\\")) != 0 )
            {
            ERR_PRINTF3(_L("Entry Number %d does not have the right termination character instead has %S"), i, &endptr);
            SetTestStepResult(EFail);
            User::Leave(KErrGeneral);
            }

        }

    INFO_PRINTF1(_L ("Successfully tested the final Crash Media Name Message Buffer"));
    }

void  CTestSignaling::CheckCancelCrashL()
    {
    INFO_PRINTF1(_L ("Test the Cancel status message update in the Crash Media Buffer"));

    TUint crashUID = 0;
    TBuf<KMaxStatusLength> statusbit;
    TRequestStatus status;

    do
        {
        // subscribe to the Crash Progress Property
        // intention is to be notified of the crash processing start
        iCrashProgress.Subscribe(status);
        User::WaitForRequest(status);

        User::LeaveIfError(iCrashProgress.Get(iCrashProgressBuffer));
        // extract the crash UID and the crash progress message
        ReadCrashProgressPacketL(crashUID, iCrashProgressBuffer);

        if(iCrashProgressBuffer.CompareF(_L("Start")) == 0)
            {
            // now that the crash processing has started
            // issue a cancel crash command
            iCrashCancel.Set(ETrue);

            }

        }
    while(iCrashProgressBuffer != KEndOfProcessing);

    // once the processing is done
    // read the crash media buffer to see
    // if we have the latest crash has the cancelled status bit
    User::LeaveIfError(iCrashMediaName.Get(iCrashMediaNameBuffer));

    // read the last crash entry as it has the latest crash
    // check for the status bit to have "C" signifying the processing was cancelled
    TPtrC statusptr = iCrashMediaNameBuffer.Mid(((KNumofEntries - 1) * KMaxEntryLength), KMaxStatusLength);

    if( statusptr.CompareF(_L("C")) != 0)
        {
        ERR_PRINTF2(_L("The entry does not seem to be valid status bit %S"), &statusptr);
        SetTestStepResult(EFail);
        User::Leave(KErrGeneral);
        }

    INFO_PRINTF1(_L ("Completed Successfully Cancel status message update in the Crash Media Buffer"));
    }
/**
 * @return void
 * @param aCrashUID crash UID used to find the matching crash in the entries
 * @param aCrashMediaNameBuffer RProperty message buffer
 * @param aStatusBit status bit of the crash media name entry
 * Reads the CrashMediaName property value extracts the crash UID
 * and the crash media name and the status bit
 * The function runs through all the buffer entries
 * but this is not needed as it is guranteed that the last entry has the latest crash information
 * Here is how the buffer entries looks like
 * there are KNumofEntries number of entries each of KMaxEntryLength
 * |Status|         MediaName         |UID |END|
 *   1              128 + 20           10   1   number of characters = KMaxEntryLength = 160

 * Status:      D stands for Done
 *              I stands for In progress
 *              C stands for Cancel
 * UID:         lower 32 bit of crash time
 * MediaName:   medianame + 64 bit time appended to it
 * END:         end character \
 */
void CTestSignaling::ParseMediaNameBufferL(TUint aCrashUID, TDes& aCrashMediaNameBuffer, TDes& aStatusBit)
    {

    TBool matchfound = EFalse;
    TInt numofEntries = aCrashMediaNameBuffer.Length() / KMaxEntryLength;
    // check the number of entries are not more than KNumofEntries
    if (numofEntries > KNumofEntries)
        {
        ERR_PRINTF2(_L("entries %d more than the max length"), numofEntries);
        SetTestStepResult(EFail);
        User::Leave(KErrGeneral);
        }

    // check for the buffer consistency the each should be of length KMaxEntryLength
    if( (iCrashMediaNameBuffer.Length() % KMaxEntryLength) != 0  )
        {
        ERR_PRINTF2(_L("Buffer corrupt iCrashMediaNameBuffer.Length() %d"), iCrashMediaNameBuffer.Length());
        SetTestStepResult(EFail);
        User::Leave(KErrGeneral);
        }

    // now read through all the entries and check for consistency
    for(TInt i = 0; i < numofEntries; i++ )
        {
        // read the status bit and check for validity
        TPtrC statusptr = aCrashMediaNameBuffer.Mid((i * KMaxEntryLength), KMaxStatusLength);

        if ((statusptr.CompareF(_L("D")) == 0) || (statusptr.CompareF(_L("I")) == 0) || (statusptr.CompareF(_L("C")) == 0) )
            {
            // read the crash UID and see if we can find a match
            TPtrC uidptr = aCrashMediaNameBuffer.Mid(((i * KMaxEntryLength) + KMaxStatusLength), KMaxUIDLength);

            TUint uid = 0;
            TLex lexuid(uidptr);
            User::LeaveIfError(lexuid.Val(uid));

            //check if the last the crash entry is the latest one
            if (uid == aCrashUID)
                {
                if( (i+1) != numofEntries)
                    {
                    ERR_PRINTF2(_L("The last entry does not have the last crash information %d"), i);
                    SetTestStepResult(EFail);
                    User::Leave(KErrGeneral);
                    }

                TPtrC crashmediaptr = aCrashMediaNameBuffer.Mid(((i * KMaxEntryLength) + KMaxStatusLength + KMaxUIDLength), KMaxMediaLength);

                //update the crash media name
                aCrashMediaNameBuffer.Copy(crashmediaptr);
                //update the status
                aStatusBit.Copy(statusptr);

                // we have found a macthing UID
                matchfound = ETrue;
                }

            }
        else
            {
            ERR_PRINTF2(_L("The entry does not seem to be valid status bit %S"), &statusptr);
            SetTestStepResult(EFail);
            User::Leave(KErrGeneral);
            }

        }

    // if we dont find it match flag it as error
    if(!matchfound)
        {
        ERR_PRINTF2(_L("The entry not found with the right UID %d"), aCrashUID);
        SetTestStepResult(EFail);
        User::Leave(KErrGeneral);
        }

    }

/**
 * @return void
 * @param aUID carsh UID
 * @param aMessageBuffer RProperty message for crashprogress
 * Reads the CrashProgress property value extracts the crash UID
 * and the crash progress message
 */
void CTestSignaling::ReadCrashProgressPacketL(TUint& aUID, TDes& aMessageBuffer)
    {
    if ( (aMessageBuffer.CompareF(_L("")) == 0) || (aMessageBuffer.CompareF(_L("clear")) == 0) || (aMessageBuffer.CompareF(_L("idle")) == 0) || (aMessageBuffer.CompareF(_L("Cancel")) == 0))
        {
        //signifying that these messages does not have a UID
        aUID = KNULLUID;
        }
    else
        {
        //INFO_PRINTF2(_L("aMessageBuffer %S"), &aMessageBuffer);
        //process the UID
        RBuf uidbuf;
        TInt ret = KErrNone;
        ret = uidbuf.Create(aMessageBuffer, KMaxUIDLength);
        if (ret != KErrNone)
            {
            ERR_PRINTF2(_L("Error %d rbuf create for UID /n"), ret);
            SetTestStepResult (EFail);
            User::Leave(ret);
            }

        TLex lexuid(uidbuf);
        ret = lexuid.Val(aUID);
        if (ret != KErrNone)
            {
            ERR_PRINTF2(_L("Error %d lex uid /n"), ret);
            SetTestStepResult (EFail);
            User::Leave(ret);
            }
        uidbuf.Close();

        // stripping the UID and the crash progress message
        TPtr message = aMessageBuffer.MidTPtr(KMaxUIDLength);
        aMessageBuffer.Copy(message);
        }
    }

/**
 * @return void
 * Utility function to save the crash UID present in the crash progress messages
*/
void CTestSignaling::SaveCrashUIDL(TUint aSaveCrashUID)
    {
    if (iNumberofCrashes < KNumofEntries)
        {
        iCrashUID[iNumberofCrashes] = aSaveCrashUID;
        iNumberofCrashes++;
        }
    else
        {
        for (TInt i = 0; i < (KNumofEntries - 1); i++)
            {
                iCrashUID[i] = iCrashUID[i+1];
            }

        iCrashUID[iNumberofCrashes-1] = aSaveCrashUID;
        }

    if(iNumberofCrashes > iNumberofCrashes)
        {
        ERR_PRINTF2(_L("Mismatch in crash uid storing %d"), iNumberofCrashes);
        SetTestStepResult(EFail);
        User::Leave(KErrGeneral);
        }
    }
/**
 * @return void
 * sets the CrashMediaName property to zero
 * this is done solely for testing purposes
 * clients of CDS are not expected to modify the property
 */
void CTestSignaling::ZeroCrashMediaNameL()
    {

    iCrashMediaNameBuffer.Zero();
    User::LeaveIfError(iCrashMediaName.Set(iCrashMediaNameBuffer));
    // setting this resets the crash UID array to zero
    iNumberofCrashes = 0;

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
void CTestSignaling::DoConfigureL(const TUint32& aIndex, const TUint32& aUID,
        const COptionConfig::TParameterSource& aSource,
        const COptionConfig::TOptionType& aType, const TDesC& aPrompt,
        const TUint32& aNumOptions, const TDesC& aOptions, const TInt32& aVal,
        const TDesC& aStrValue, const TUint aInstance)

    {
    COptionConfig * config;

    config = COptionConfig::NewL ( aIndex, aUID, aSource, aType, aPrompt,
            aNumOptions, aOptions, aVal, aStrValue);

    CleanupStack::PushL (config);

    config->Instance (aInstance);

    //Configure now...
    TRAPD (ret, iSess.SetConfigParameterL (*config));
    if ( ret != KErrNone)
        {
        ERR_PRINTF2(_L("Error %d changing parameter/n"), ret);
        SetTestStepResult (EFail);
        User::Leave(ret);
        }

    CleanupStack::PopAndDestroy (config);
    }

/**
 * @return void
 * Utility function to load the SELF Formatter and File Writer
 * Configures the file writer and the CDS
 */
void CTestSignaling::LoadandConfigurePluginsL(const TDesC& aCrashFileName)
    {
    INFO_PRINTF1 (_L ("Attempting to load SELF Formatter V2 and File Writer"));

    // SELF Formatter V2
    TPluginRequest SELFreq;
    SELFreq.iPluginType = TPluginRequest::EFormatter;
    SELFreq.iLoad = ETrue;
    SELFreq.iUid = KUidSELFFormatterV2;

    TRAPD(ret, iSess.PluginRequestL(SELFreq));
    if ( ret != KErrNone)
        {
        ERR_PRINTF2 (_L ("Failed to load SELF Formatter V2 plugin, error = %d"), ret);
        SetTestStepResult (EFail);
        User::Leave(ret);
        }

    // File Writer
    TPluginRequest writerreq;
    writerreq.iUid = WRITER_UID;
    writerreq.iPluginType = TPluginRequest::EWriter;
    writerreq.iLoad = ETrue;

    TRAP(ret, iSess.PluginRequestL(writerreq ));
    if(ret != KErrNone)
        {
        ERR_PRINTF2(_L("Failed to load writer plugin, error = %d"), ret);
        SetTestStepResult(EFail);
        User::Leave(ret);
        }

    INFO_PRINTF1(_L("Successfully loaded SELF Formatter V2 and File Writer"));

    //Now we configure CDS and writer
    _LIT( KFilePathPrompt, "not_important" );

    DoConfigureL(2, CDS_UID.iUid, COptionConfig::ECoreDumpServer, COptionConfig::ETUInt, KPostCrashEventActionPrompt,
                 1, KNullDesC, 4, KNullDesC, 0);

    DoConfigureL((TInt)(CCrashDataSave::ECoreFilePath), WRITER_UID.iUid, COptionConfig::EWriterPlugin, COptionConfig::ETFileName,
                        KFilePathPrompt, 1, KNullDesC, 0, aCrashFileName, 0);

    INFO_PRINTF1(_L("Successfully configured File Writer and CDS"));

    }
