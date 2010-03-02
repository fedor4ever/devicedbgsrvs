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
// Defines UTRACE formatter plugin class
//



/**
 @file
 @internalTechnology
 @released
*/

#ifndef UTRACE_FORMATTER_H
#define UTRACE_FORMATTER_H

#include <e32std.h>
#include <e32property.h>
#include <rm_debug_api.h> //registers info

#include <formatterapi.h>
#include <optionconfig.h>
#include <crashdatasource.h>
#include <crashdatasave.h>
#include <coredumpserverapi.h> //property - UID

const TUint32 KUtraceFormatterUID = { 0x102836BA   };

_LIT(KPluginDescription, "UTrace formatter plugin");
_LIT(KUTraceCrashStart, "started utrace formatting");
_LIT(KUTraceCrashFinish, "done");

/**
@internalTechnology 
@released 
DEXC formatter plugin is loaded by the core dump server and notified about the crash event.
Then it dupms crash data in the same format as the legacy d_exc crash dump utility.
@see CCrashDataSave
*/
class CUtraceFormatter : public CCoreDumpFormatter
{
	enum { KUTraceDumpFileLength = 23 };

	public:
		static CUtraceFormatter* NewL();
		static CUtraceFormatter* NewLC();
		~CUtraceFormatter();
		
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
    	TUint HelpWriteTraceBufferL(TUint aTraceAvailable, TUint aTraceWanted, TUint aOffset);
    	TUint HelpWriteTraceBufferWithTraceFrameWorkL(TUint aTraceWanted, TUint aOffset);    	
    	
    private:
    	
	    /** Pointer to writer plugin created and managed by core dump server, provides API to store the dump information*/
	    CCrashDataSave* iDataSave;
	    
	    /** Pointer to data source object created and managed by core dump server, provides API to gather necessary info about the crash */ 
        CCrashDataSource* iDataSource; 
        
        /** Values for utrace configuration */
        enum TUTraceParams
        {
        /** Amount of trace to dump */
        ETraceSize,
        
        /** Gaurd */
        ETraceGuard
        };
        
        const TDesC& DumpFileNameLC(TUint64 aStamp);
         
        void UpdateCrashProgressL( const TDesC &aProgress );
        
     public:
     
        /** UTRACE formatter configuration parameter list */
		RConfigParameterList iConfigList;   
    

};
#endif
