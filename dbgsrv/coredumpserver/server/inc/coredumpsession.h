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
//



/**
 @file
 @internalTechnology
 @released
*/

#ifndef CORE_DUMP_SESSION_H
#define CORE_DUMP_SESSION_H

#include <e32std.h>
#include <e32base.h>
#include <s32file.h>
#include <e32property.h>
#include <bsul\inifile.h>
#include <scmdatatypes.h>

#include <rm_debug_api.h>

// Project inclues
#include <crashdata.h>
#include "coredumpserver.h"
#include <coredumpserverapi.h>
#include <optionconfig.h>
#include <crashdatasave.h>
#include "servercrashdatasource.h"
#include "flashdatasource.h"
#include "coretargetobserver.h"

using namespace Debug;
using namespace BSUL;

        enum TPreProcessingActionType {ENoPreAction = 0, ESuspendThread = 0x1, ESuspendProcess = 0x2};
        enum TPostProcessingActionType {ENoPostAction = 0, EResumeThread = 0x1, EResumeProcess = 0x2, EKillProcess = 0x4, EDeleteCrashLog = 0x5};

/**
Core dump server session that handles messages from core dump clients.
@see CCoreDumpServer
@see RCoreDumpSession
*/
class CCoreDumpSession : public CSession2
	{	
	public:		// Construction
		IMPORT_C static CCoreDumpSession* NewL( RSecuritySvrSession &aSecSess );

	public:		// Destruction
		IMPORT_C ~CCoreDumpSession();
		
	public:		// CCoreDumpSession
		IMPORT_C virtual void ServiceL (const RMessage2& aMessage);

	private:
		CCoreDumpSession( RSecuritySvrSession &aSecSess );

    protected:
		void ConstructL();
		
	private:	// CCoreDumpSession
		void PanicClient(const RMessage2& aMessage, TInt aPanic);
		void DispatchMessageL(const RMessage2& aMessage);

		HBufC8 * MarshalListL( TListRequest & req, TAny * list );
		void GetListL( const RMessage2& aMessage );
		void ListInfoL( const RMessage2& aMessage );

        //make sure that those calls leave
		void GetPluginListL(const RMessage2& aMessage);
		void PluginRequestL(const RMessage2& aMessage);

	//	TInt StartDebugSessionL(const RMessage2& aMessage);

		void RefreshPluginListL();

        void ObservationRequestL( const RMessage2 &aMessage );
        void AttachTargetL( const TDesC &aTargetName, const TDesC &aTargetOwnerName );
        void DetachTargetL( const TDesC &aTargetName, const TDesC &aTargetOwnerName );

		TInt GetByPId( const TUint aPid );

		void CreateFormatterL( const TUid loadRequestUid );
		void CreateWriterL( const TUid loadRequestUid );
        void DeleteFormatterL( const TUid freeRequestUid, const TUint index );
        void DeleteWriterL( const TUid freeRequestUid, const TUint index );

        void BindPluginsL(const TUint aIndex, TUint aPair);
		void SetPluginsL( const TUid aFormatterUID, 
							const TUid aWriterUID, 
							const TBool aLoad );

		void GetNumberConfigParametersL( const RMessage2& aMessage );
		void GetConfigParameterL( const RMessage2& aMessage );
		void SetConfigParameterL( const RMessage2& aMessage );
		void SetConfigParameterL( const TInt aIndex, 
								 const TInt32 & aValue, 
								 const TDesC & aDescValue );

		void UpdateConfigParametersL( );


		void LoadConfigFileL( const RMessage2* aMessage = NULL );
		void SaveConfigFileL( const RMessage2* aMessage = NULL );

		
		enum TCoreDumpIniKey
			{
			EIniKeyType,
			EIniKeyIndex,
			EIniKeyValue
			};

		HBufC * GenerateKeyStringLC( TUint aIndex, 
									 TCoreDumpIniKey aKey );

		void RecreateConfigurationL( BSUL::CIniDocument16 * aPersistConfigData );

		void ChangeConfigParamsL( const COptionConfig::TParameterSource aSource,
								  BSUL::CIniDocument16	*	aPersistConfigData,
                                  const TInt aIndex = -1);

		void SaveConfigParamL(  const COptionConfig	 *	aParam, 
								const TPtrC			 &	aSectName,
								const TInt				aSaveIndex,
								BSUL::CIniDocument16 *	aPersistConfigData );

		void OpenIniFileLC( const RMessage2		  *	aMessage, 
							const TBool				aSaving,
							RFs					  & aFsSession,
							HBufC				 *&	aFileName,
							BSUL::CIniDocument16 *&	aPersistConfigData );

		void OpenIniFileLC( const TBool				aSaving,
							RFs					  & aFsSession,
							HBufC				 *&	aFileName,
							BSUL::CIniDocument16 *&	aPersistConfigData );

		void ReadPluginUidFilesL( );
		
//		void ReadSCMConfigListL();
//		void WriteSCMConfigListL();
		//void UpdateSCMConfigParameterL( TConfigItem::TSCMDataType aDataType, const TInt32 & aPriority );
		
		void ValidatePluginL( const TUid aPluginUid ) const;
        void SetCrashEventsL(CTargetObserver &aObserver, TKernelEventAction aAction);
        
        void RefreshCrashListFromFlashL();
        void GetCrashInfoL(TInt aCrashId, TCrashInfo& aCrashInfo);
        void DeleteCrashLogL(const RMessage2& aMessage);
        void DeleteCrashPartitionL();
        void ProcessCrashLogL(const RMessage2& aMessage);
        void ProcessCrashLogAsyncL(const RMessage2& aMessage);
        void CancelProcessCrashLogAsync(const RMessage2& aMessage);
        void UpdateProgressL(const TDesC &aProgress) const;

    public:
        CCrashDataSource* DataSource() const {return iDataSource;};
        CFlashDataSource* FlashDataSource() const {return iFlashDataSource;};
        CCoreDumpFormatter* GetValidFormatter(const TUint aCount);
        TInt32 PreProcessingAction() const;
        TInt32 PostProcessingAction() const;
        void SuspendProcessL(TUint64 aPid);
        void KillProcessL(TUint64 aPid);
        void ResumeProcessL(TUint64 aPid);
        void ResumeThreadL(TUint64 aTid);
        CCoreDumpServer& Server() const;
        virtual void CreateL();

	private:	// Data members
        RSecuritySvrSession &iSecSess;
		
		TUint					iKeyEvent;
		
        /** keeps list of ECOM plugin types */
		RPluginList				iPluginList;

		TUint					iTotalThreadListDescSize;
		RThreadPointerList      iThreadPointerList; 
		HBufC8*					iMarshalledThreadList;
        
		TUint					iTotalProcessListDescSize;
		RProcessPointerList		iProcessPointerList;
		HBufC8*					iMarshalledProcessList;

		TUint					iTotalExecutableListDescSize;
		RExecutablePointerList	iExecutablePointerList;
		HBufC8*					iMarshalledExecutableList;
		
		//this stores the crashes that are in the flash partition
		TUint					iTotalCrashInfoDescSize;
		RCrashInfoPointerList	iCrashList;
		HBufC8*					iMarshalledCrashList;
		
        typedef RPointerArray<CCoreDumpFormatter> RFormatterPointerList;
        
        /** list of formatter plugin instances */
		RFormatterPointerList	iFormatterPlugins;
		
        /** list of formatter plugin info objects coresponding to actual instances*/
        RPluginPointerList      iFormatterInfos;
		HBufC8*					iMarshalledFormatterList;
		TUint					iTotalFormatterListDescSize;

        typedef RPointerArray<CCrashDataSave> RWriterPointerList;
        
        /** list of writer plugin instances */
		RWriterPointerList		iWriterPlugins;
		
        /** list of writer plugin info objects coresponding to actual instances*/
        RPluginPointerList      iWriterInfos;
		HBufC8*					iMarshalledWriterList;
		TUint					iTotalWriterListDescSize;

		CServerCrashDataSource* iDataSource;
		CFlashDataSource* iFlashDataSource;
        CCrashHandler*			iCrashHandler;

        enum TCrashEventTypes 
			{
			ECrashHwExc = 0x1, 
			ECrashKillThread = 0x2, 
			ECrashHwExcAndKillThread = 0x3
			};

		typedef RPointerArray<CTargetObserver>		RObservationList;
		
		RObservationList iObservationList;

    private:
        TInt32 iPreCrashEventAction;
        TInt32 iPostCrashEventAction;
        TInt32 iCrashEventTypes;


		// Configuration parameters for current system. It is the collection
		// of config params from CDS and currently loaded plugins
		typedef RPointerArray<COptionConfig>	RConfigParameterList;
		RConfigParameterList	  iTotalConfigList; //no need to destroy the content

		/** The maximum size of the configuration parameter that was found 
		when the entire list iTotalConfigList was last updated. It is useful
		when allocating descriptor for client/server transfer */
		TInt					iMaxConfigParamSize;

		/** Configuration parameters for CDS. */
		RConfigParameterList     iConfigList;
		
		/** configuration parameters for SCM configuration */
		RConfigParameterList	 iScmConfigList;
		
		

		TUid					iOurSecureID;
		RArray<TUid>			iAllowedPlugins;

		TUint32					iSpare0;
		TUint32					iSpare1;
		TUint32					iSpare2;
		TUint32					iSpare3;
		TUint32					iSpare4;
		TUint32					iSpare5;
		
    private:
    	void PrintScmConfigL();
    	
    	static void CleanupSCMConfiguration(TAny* );
};

#endif	// CORE_DUMP_SESSION_H

