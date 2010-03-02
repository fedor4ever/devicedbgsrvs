// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Defines D_EXC formatter plugin class
//



/**
 @file
 @internalTechnology
 @released
*/

#ifndef D_EXC_FORMATTER_H
#define D_EXC_FORMATTER_H

#include <e32std.h>
#include <e32property.h>
#include <rm_debug_api.h> //registers info

#include <formatterapi.h>
#include <optionconfig.h>
#include <crashdatasource.h>
#include <crashdatasave.h>
#include "tdexcformatter.h" //test friendship
#include <coredumpserverapi.h> //property - UID



const TUint32 KDEXCFormatterUid = { 0x102832C5 };

const TInt KMaxLineLength = KMaxFullName + 32; //compatibility with legacy D_EXC implementation
_LIT(KPluginDescription, "D_EXC formatter plugin description");

class COptionConfig;
class CThreadInfo;

/**
@internalTechnology 
@released 
DEXC formatter plugin is loaded by the core dump server and notified about the crash event.
Then it dupms crash data in the same format as the legacy d_exc crash dump utility.
@see CCrashDataSave
*/
class CDexcFormatter : public CCoreDumpFormatter
{
        static void CleanupThreadList(TAny *aArray);
        static void CleanupCodeSegList(TAny *aArray);
        friend void CDexcFormatterTest::TestSuiteL(TSuite aSuite) const; //test harness
        enum { KBufSize = 1024 }; //mem read buf size
        enum { KDexcFileNameLength = 26 }; //d_exc_<tid>.[stk|txt]

	public:
		static CDexcFormatter* NewL();
		static CDexcFormatter* NewLC();
        ~CDexcFormatter();

	public:
		virtual TInt GetNumberConfigParametersL( );
		virtual COptionConfig * GetConfigParameterL( const TInt aIndex );
		virtual void SetConfigParameterL( const TInt aIndex, const TInt32 & aValue, const TDesC & aDescValue );

	    virtual void ConfigureDataSaveL(CCrashDataSave* aDataSave);
	    virtual void ConfigureDataSourceL(CCrashDataSource* aDataSource);
	    virtual void GetDescription(TDes& aPluginDescription);
        virtual void CrashEventL(TCrashInfo* aCrashInfo);
 
    private:
        void ConstructL();
        void DumpLogL(const CThreadInfo& aThread);
        void DumpPanicInfoL(const TCrashInfo& aInfo);
        void DumpExcInfoL(const TCrashInfo& aCrash);
        void DumpRegistersL(const TUint64 aTid);
        void DumpCodeSegsL(const TUint64 aPid) ;
        void DumpStackL(const TUint64 aTid, const CThreadInfo& aInfo);
        void UpdateCrashProgressL(const TDesC &aProgress) const;
        TInt PickRegisters(const RRegisterList& aAllRegs, RRegisterList& aReqRegs,
                           const TFunctionalityRegister* const aReqIds) const;
        const CThreadInfo& GetCrashedThreadInfoLC(const TCrashInfo &aCrashInfo) const;

        const TDesC& DumpFileNameLC(TUint64 aTid, const TDesC &aExtension);

    private:
        /** Buffer holding each dump line */
	    TBuf8<KMaxLineLength> iLine;

        /** reserved for the future */
        TInt iSpare;

        /** Pointer to writer plugin created and managed by core dump server, provides API to store the dump information*/
        CCrashDataSave* iDataSave;

        /** Pointer to data source object created and managed by core dump server, provides API to gather necessary info about the crash */ 
        CCrashDataSource* iDataSource; 

        /** Dump stack information flag, updated on parameter change */
        TBool iDumpStack; 

        /** Dump stack in ASCII format flag, updated on parameter change */
        TBool iAsciiStack;

        /** Available values of DEXC formatter plugin parameter index */
		enum TDexcParams
		    {
            /** dump stack */
            EDumpStack,
            /** dump stack in ASCII */
            EAsciiStack,
            /** guard */
            EDexcParamsLast
            };

        /** DEXC formatter configuration parameter list */
		RConfigParameterList iConfigList;
};
#endif
