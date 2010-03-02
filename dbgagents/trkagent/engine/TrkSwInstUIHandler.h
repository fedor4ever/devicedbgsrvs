/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/


#ifndef __TrkSWInstUIHandler_H__
#define __TrkSWInstUIHandler_H__


// Epoc Include
#include <swi/msisuihandlers.h>

//Macros
#define KUserNameMaxLength	64
#define KPassword			64


enum KMyErrCode
{
	KErrNothing,
	KErrLanguage,        
	KErrDrive,
	KErrOptions,        
	KErrAbortInstall,
	KErrAbortUnInstall,        
	KErrLeave,
	KErrCannotOverwriteFile,
	KErrError        
};
	

using namespace Swi;

class CTrkSWInstUIHandler : public CBase, public Swi::MUiHandler
{
	public:
		static CTrkSWInstUIHandler* NewL();

		static CTrkSWInstUIHandler* NewL(const TChar);
		
		~CTrkSWInstUIHandler();

		TBool  DisplayInstallL (const CAppInfo& aAppInfo, 
			const CApaMaskedBitmap* aLogo, 
			const RPointerArray <CCertificateInfo> & aCertificates);

		TBool DisplayGrantCapabilitiesL(const CAppInfo& aAppInfo, 
						const TCapabilitySet& aCapabilitySet);
										
		TInt DisplayLanguageL(const CAppInfo& aAppInfo, 
				      const RArray<TLanguage>& aLanguages);
	
		TInt DisplayDriveL(const CAppInfo& aAppInfo, 
				   TInt64 aSize,
				   const RArray<TChar>& aDriveLetters,
				   const RArray<TInt64>& aDriveSpaces);
	
		TBool DisplayUpgradeL(const CAppInfo& aAppInfo, 
				      const CAppInfo& aExistingAppInfo);
	
		TBool DisplayOptionsL(const CAppInfo& aAppInfo,
				      const RPointerArray<TDesC>& aOptions,
				      RArray<TBool>& aSelections);
	
		TBool HandleInstallEventL(const CAppInfo& aAppInfo,
						TInstallEvent aEvent, 
						TInt aValue=0, 
						const TDesC& aDes=KNullDesC);

		void HandleCancellableInstallEventL(const CAppInfo& aAppInfo,
						TInstallCancellableEvent aEvent, 
						MCancelHandler& aCancelHandler,
						TInt aValue=0, 
						const TDesC& aDes=KNullDesC);

		TBool DisplaySecurityWarningL(const CAppInfo& aAppInfo,
						Swi::TSignatureValidationResult aSigValidationResult,
						RPointerArray<CPKIXValidationResultBase>& aPkixResults,
						RPointerArray<CCertificateInfo>& aCertificates,
						TBool aInstallAnyway);
	
		TBool DisplayOcspResultL(const CAppInfo& aAppInfo,
						TRevocationDialogMessage aMessage, 
						RPointerArray<TOCSPOutcome>& aOutcomes, 
						RPointerArray<CCertificateInfo>& aCertificates,
						TBool aWarningOnly);
	
		void DisplayCannotOverwriteFileL(const CAppInfo& aAppInfo,
						 const CAppInfo& aInstalledAppInfo,
						 const TDesC& aFileName);
						 
		TBool DisplayUninstallL(const CAppInfo& aAppInfo);


        TBool DisplayTextL(const CAppInfo& aAppInfo, 
				    TFileTextOption aOption,
				    const TDesC& aText);
	 
		void DisplayErrorL(const CAppInfo& aAppInfo,
				   TErrorDialog aType, 
				   const TDesC& aParam);

		TBool DisplayDependencyBreakL(const CAppInfo& aAppInfo,
					      const RPointerArray<TDesC>& aComponents);
		
		TBool DisplayApplicationsInUseL(const CAppInfo& aAppInfo, const RPointerArray<TDesC>& aAppNames);
	
		TBool DisplayQuestionL(const CAppInfo& aAppInfo, TQuestionDialog aQuestion, const TDesC& aDes=KNullDesC);

		TBool DisplayMissingDependencyL(const CAppInfo& aAppInfo,
										const TDesC& aDependencyName,
										TVersion aWantedVersionFrom,
										TVersion aWantedVersionTo,
										TVersion aInstalledVersion);

		TBool 	iDispInstall;
		TBool	iDispGrantCapabilities;
		TBool	iDispUnInstall;
		TInt	iDispLanguage;
		TInt	iDispDialect;
		TChar	iDispDrive;
		TBool	iDispUpgrade;
		TBool	iDispDependencyBreak;
		TBool	iDispQuestion;
		TBool	iDispSecurityWarning;
		TBool	iDispRevocationQuery;
		TBool	iDispApplicationInUse;
		TBool	iDispTextDialog;
		TInt	iDispOptions;
		TInt	iAppUID;
		
		KMyErrCode 	iErrCode;
		
	private:
		CTrkSWInstUIHandler();
		CTrkSWInstUIHandler(const TChar aInstallDrive);

		void ConstructL();
};

#endif //__TrkSWInstUIHandler_H__

