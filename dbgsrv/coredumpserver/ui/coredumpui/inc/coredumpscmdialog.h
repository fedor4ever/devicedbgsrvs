// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef __COREDUMPSCMDIALOG_H_INCLUDED__
#define __COREDUMPSCMDIALOG_H_INCLUDED__

#include "coredumpuidialogs.h"

class CConfigureSCMDialog : public CCoreDumpDialogBase
	{
public:
	CConfigureSCMDialog(RCoreDumpSession &aCoreDumpSession);
	~CConfigureSCMDialog();

private:
	void PreLayoutDynInitL();
	void PostLayoutDynInitL();
	TBool OkToExitL(TInt aKeycode);
	void UpdateDialogDataL(void);
	void UpdateDialogDataReal(void);
	void UpdateDialogDataAll(void);
	void UpdatePriorityText(void); //updates the priority number value to correspond to the priority option selected

	};


#endif // __COREDUMPSCMDIALOG_H_INCLUDED__
