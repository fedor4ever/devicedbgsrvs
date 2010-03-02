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
//  Utility functions to allow us to configure the System Crash Monitor

/**
 @file
 @internalTechnology
*/

#ifndef __SCM_CONFIG_H__
#define __SCM_CONFIG_H__

#include <datawrapper.h>
#include <e32base.h>
#include <coredumpinterface.h>
#include <optionconfig.h>
#include <scmconfigitem.h>

#include "t_common_defs.h"

/**
 * This class is the test wrapper we use to configure the System Crash
 * Monitor
 */
class CSCMConfigurationWrapper : public CDataWrapper
	{
	public:
		~CSCMConfigurationWrapper();
		static CSCMConfigurationWrapper* NewL();

		virtual TBool DoCommandL(const TTEFFunction& aCommand, const TTEFSectionName& aSection, const TInt aAsyncErrorIndex);
		virtual TAny* GetObject() { return NULL; }
		virtual void SetObject(TAny* aObject) { (void)aObject; }

	protected:
		void ConstructL();

	private:
		CSCMConfigurationWrapper();

		void TurnOffConfigOptionL(TConfigItem::TSCMDataType& aParameter);
		void TurnOnConfigOptionL(TConfigItem::TSCMDataType& aParameter);
		void CSCMConfigurationWrapper::SetSCMParameterL(TConfigItem::TSCMDataType& aParameter, const TInt aPriority);

	private:
		RCoreDumpSession iCoreDumpSession;
	};

#endif __SCM_CONFIG_H__

//eof
