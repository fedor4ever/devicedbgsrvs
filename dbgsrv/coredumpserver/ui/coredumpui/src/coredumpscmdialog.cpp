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
// Implementation of CConfigureSCMDialog class 
//



/**
 @file
 @internalTechnology 
 @released
 @see CCoreDumpFormatter
*/

#include "coredumpui.h"
#include "coredumpscmdialog.h"


CConfigureSCMDialog::CConfigureSCMDialog(RCoreDumpSession &aCoreDumpSession)
    : CCoreDumpDialogBase(aCoreDumpSession)
    {
    }

CConfigureSCMDialog::~CConfigureSCMDialog()
	{
	iConfs.ResetAndDestroy();
	}

void CConfigureSCMDialog::PreLayoutDynInitL()
	{
	UpdateDialogDataL();
	}

void CConfigureSCMDialog::PostLayoutDynInitL()
	{
	}

void CConfigureSCMDialog::UpdateDialogDataL()
	{
	// We are loading up edwins as defined in rss file here
	// A better approach may be to create the items dynamically if time allows
	// example code for this at http://wiki.forum.nokia.com/index.php/Create_Modal_dialog_and_add_controls_dynamically
	
	//update dialog
	RPointerArray<CEikEdwin> edwins;
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin0)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin1)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin2)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin3)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin4)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin5)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin6)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin7)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin8)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin9)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin10)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin11)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin12)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin13)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin14)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin15)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin16)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin17)));
	edwins.Append(static_cast<CEikEdwin*>(Control(EEdwin18)));
	
	for (TInt i = 0; i < edwins.Count(); i++)
		{
		edwins[i]->SetDimmed(ETrue);
		edwins[i]->SetReadOnly(ETrue);
		}

	RPointerArray<CEikLabel> labels;
	labels.Append(static_cast<CEikLabel*>(Control(ELabel0)));  	// 0
	labels.Append(static_cast<CEikLabel*>(Control(ELabel1)));  	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel2)));	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel3)));	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel4)));	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel5)));	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel6)));	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel7)));	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel8)));	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel9)));	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel10)));	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel11)));	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel12)));	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel13)));	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel14)));	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel15)));	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel16)));	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel17)));	
	labels.Append(static_cast<CEikLabel*>(Control(ELabel18)));	

	// Read config parameters from CDS
	TInt noConfigParams = 0;
	iConfs.ResetAndDestroy();
	
    TRAPD(err, noConfigParams = iCoreDumpSession.GetNumberConfigParametersL());
    if(err != KErrNone)
        {
		TBuf<23> error;
		error.Format(_L("Error: %d"), err);
		CEikonEnv::InfoWinL(_L("Could not get number of config params!"), error);
        User::Leave(err);
        }
	TInt control = 0;

	for (TInt i = 0; i < noConfigParams; i++)
		{
		COptionConfig* conf;
		conf = iCoreDumpSession.GetConfigParameterL(i);
		if (conf->Source()== COptionConfig::ESCMConfig && control < edwins.Count())
			{
			iConfs.Append(conf);
			
			TBuf<255> buf = conf->Prompt();
			switch (conf->Type())
				{
				// SCM configuration options are all TInt (TInt priority)
				case COptionConfig::ETInt:
					{
					buf.Append(_L("\n(priority (0-255) "));
					TBuf<100> value;
					value.Format(_L("%d"), iConfs[control]->Value());
					edwins[control]->SetTextL(&value);
					// save the edwin in the tag field of option config 
					// this will make unloading the data easier
					conf->SetTag(edwins[control]);
					}
					break;
				default:
					break;
				}
			labels[control]->SetTextL(buf);
			edwins[control]->SetDimmed(EFalse);
			edwins[control++]->SetReadOnly(EFalse);
			}
        else
            {
            delete conf;
            }
		}
	edwins.Close();
	labels.Close();
	}


TBool CConfigureSCMDialog::OkToExitL(TInt aButtonId)
	{
	
	TBool ret = ETrue;
	TInt err = KErrNone;
	
	
	// for the scm dialog - each edwin contains an integer value priority 
	for (TInt i = 0; i <iConfs.Count(); i++)
		{
		COptionConfig* conf = iConfs[i];
		if(!conf)
			{
			continue;
			}


		// retrieve the edwin pointer (saved in iTag member)
		CEikEdwin* edwin = (CEikEdwin*) conf->Tag();
		if(edwin)
			{
			HBufC* hBuf = edwin->GetTextInHBufL();
			CleanupStack::PushL(hBuf);
			//hBuf->Des()
			TLex lex(*hBuf);
			TInt32 priority;
			err = lex.Val(priority);
			
			if(err != KErrNone)
				{
				LOG_MSG("  error from lex.Val(conf) -> CConfigureSCMDialog::CEikonEnv::Static()->InfoMsg;" );
				CEikonEnv::Static()->InfoMsg(_L("Value must be an integer"));
				}			
			else if(priority != conf->Value())
				{
				iConfs[i]->Value(priority);

				LOG_MSG4("CConfigureSCMDialog::OkToExitL modifying item uid=%d instance=%d value=%d",
						conf->Uid(), conf->Instance(), conf->Value());

				
				
				TRAP( err, iCoreDumpSession.SetConfigParameterL( *iConfs[i]) );
				if( err != KErrNone)
					{
					CEikonEnv::Static()->InfoMsg(_L("Error changing config param "));
					}
				
				}

			CleanupStack::PopAndDestroy(hBuf);
			}

		}
	
	return ret;
	}
