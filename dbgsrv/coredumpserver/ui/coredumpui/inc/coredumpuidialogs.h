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

#ifndef CoreDumpUI_DIALOGS_H
#define CoreDumpUI_DIALOGS_H

#include <eikdialg.h>
#include <coredumpinterface.h>
#include <optionconfig.h>

class CCoreDumpDialogBase : public CEikDialog
	{
public:
	// constructor
	CCoreDumpDialogBase(RCoreDumpSession &aCoreDumpSession);
	virtual ~CCoreDumpDialogBase();
public:
	void DisplayErrorL(TDesC& aMessage, TInt aErrorNumber);
	void DisplayErrorL(TDesC& aMessage);
	
protected:
    RCoreDumpSession &iCoreDumpSession;  
	RPointerArray<COptionConfig> iConfs; 

	};


class CConfigurePluginDialog : public CCoreDumpDialogBase
	{
public:
	// constructor
	CConfigurePluginDialog(TUid aUid, RCoreDumpSession &aCoreDumpSession, TInt aInstance);
	~CConfigurePluginDialog();

private:
	void PreLayoutDynInitL();
	void PostLayoutDynInitL();
	TBool OkToExitL(TInt aKeycode);
	void UpdateDialogDataL(void);
	void UpdatePriorityText(void);
	void UpdateProcessId(void);

private:
	TUid iUid;
	TInt iInstance;
	};


#endif 
