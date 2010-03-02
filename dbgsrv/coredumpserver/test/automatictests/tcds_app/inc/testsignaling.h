/**
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
* Test ECrashMediaName RProperty signaling mechanism for application crashes
*/

/**
 @file testsignaling.h
 @internalTechnology
*/

#if (!defined __TEST_SIGNALING_H__)
#define __TEST_SIGNALING_H__

#include <e32property.h>
#include <TestExecuteStepBase.h>
#include "tcoredumpserversuitestepbase.h"
#include "tcoredumpcommon.h"

_LIT(KTestSignaling,"TestSignaling");

class CTestSignaling : public CTe_coredumpserverSuiteStepBase
    {

public:
    CTestSignaling();
    ~CTestSignaling();

    virtual TVerdict doTestStepPreambleL();
    virtual TVerdict doTestStepPostambleL();
    virtual TVerdict doTestStepL();

private:

    void ClientAppL();
    void TestSignalingMechanismL();

    // main methods to check teh signaling mechanism
    void CheckUIDCrashProgressL();
    void CheckCrashMediaNameL();
    void TestCrashMessageBufferL();
    void CheckCancelCrashL();

    void LoadandConfigurePluginsL(const TDesC& aCrashFileName);
    void DoConfigureL( const TUint32& aIndex, const TUint32& aUID,
            const COptionConfig::TParameterSource& aSource,
            const COptionConfig::TOptionType& aType, const TDesC& aPrompt,
            const TUint32& aNumOptions, const TDesC& aOptions,
            const TInt32& aVal, const TDesC& aStrValue, const TUint aInstance);

    void StartCrashAppL(const TDesC& aCrashAppParam);
    void ReadCrashProgressPacketL(TUint& aUID, TDes& aMessageBuffer);
    void ParseMediaNameBufferL(TUint crashUID, TDes& crashMediaNameBuffer, TDes& statusbit);
    void SaveCrashUIDL(TUint aSaveCrashUID);
    void ZeroCrashMediaNameL();
	void TidyFilesL(const TDesC& aFilenameRoot);

 // member variables

    RCoreDumpSession iSess;
    RFs iFs;

    RProperty iCrashProgress;
    RProperty iCrashMediaName;
    RProperty iCrashCancel;

    RBuf iCrashProgressBuffer;
    RBuf iCrashMediaNameBuffer;

    TBuf<KMaxFileName> iCrashFileName;
    // Crash UID for KNumofEntries crashes
    TInt iCrashUID[KNumofEntries];

    TInt iNumberofCrashes;

    TInt iTimestoCrash;

    };

#endif //__TEST_SIGNALING_H__
