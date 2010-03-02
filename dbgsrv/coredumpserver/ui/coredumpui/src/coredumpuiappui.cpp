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


#include "coredumpui.h"
#include <coredumpui.rsg>
#include "coredumpuinotifier.h"

#include "coredumpcdsdialog.h"
#include "coredumpscmdialog.h"

void CResourceAppUi::ConstructL()
{
	BaseConstructL();
	iListBoxView = CResourceView::NewL(ClientRect(), this);
    iCrashNotifier = CCrashNotifier::NewL(*iListBoxView);
	//construct a default view
	HandleCommandL(EResourceMenu1Item0);
}

CResourceAppUi::~CResourceAppUi()
{
    delete iCrashNotifier;
	delete iListBoxView;
}

void CResourceAppUi::HandleCommandL(TInt aCommand)
{
	LOG_MSG("->[UI]CResourceAppUi::HandleCommandL(TInt aCommand)");
	// Increase this threads priority for a proper snapshot
	RThread thisThread;
	thisThread.SetPriority(EPriorityAbsoluteHigh);
	thisThread.Close();

	TObjectsViewed objectViewed = EObjectNone;

	switch (aCommand)
		{	
		// Menu 1	
		case EResourceMenu1Item0:
			{
			objectViewed = EObjectProcess;
			break;
			}
		case EResourceMenu1Item1:
			{
			objectViewed = EObjectThread;
			break;
			}
		case EResourceMenu1Item2:
			{
			objectViewed = EObjectPlugin;
			break;
			}
		case EResourceMenu1Executables:
			{
			objectViewed = EObjectExecutable;
			break;
			}
		case EResourceMenu1PluginInstances:
			{
			objectViewed = EObjectPluginInstance;
			break;
			}
		// Menu 2
		case EResMenu2ConfCDS:
			{
			SetCDSL();
			break;
			}
		case EResMenu2ConfSCM:
			{
			SetSCM();
			break;
			}
		case EResMenu2CancelCrash:
			{
			CancelCrashL();
			break;
			}
			
		//Flash Menu
		case EResMenu3ListCrashes:
			{
			objectViewed = EObjectCrashList;
			break;
			}
		case EEikCmdExit: // necessary for uninstalling
			{
			Exit();
			break;
			}
		default:
			{
			break;
			}
		}

	// Set view data
	if (objectViewed != EObjectNone)
	{
		iListBoxView->SetCurrentObjects(objectViewed);
		iListBoxView->UpdateViewL();
	}

	// ResetPriority
	RThread thisThread2;
	thisThread2.SetPriority(EPriorityNormal);
	thisThread2.Close();

}

void CResourceAppUi::CancelCrashL()
    {
    LOG_MSG("->CResourceAppUI::CancelCrashL()");
    User::LeaveIfError(RProperty::Set(KCoreDumpServUid, ECancelCrash, ETrue));
    }

void CResourceAppUi::SetCDSL()
	{
	CConfigureCDSDialog* dialog = new(ELeave) CConfigureCDSDialog(iListBoxView->CoreDumpInterface());	
	dialog->ExecuteLD(R_CONFIGURE_CDS_DIALOG);
	}


void CResourceAppUi::SetSCM()
	{
	CConfigureSCMDialog* dialog = new (ELeave) CConfigureSCMDialog(iListBoxView->CoreDumpInterface());	
	dialog->ExecuteLD(R_CONFIGURE_SCM_DIALOG);
	}

