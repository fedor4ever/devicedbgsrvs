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
#include <optionconfig.h>
#include <bautils.h>


 // CDialogBase //

CCoreDumpDialogBase::CCoreDumpDialogBase(RCoreDumpSession &aCoreDumpSession)
: iCoreDumpSession(aCoreDumpSession)
	{
	}

CCoreDumpDialogBase::~CCoreDumpDialogBase()
	{
	iConfs.ResetAndDestroy();
	}

void CCoreDumpDialogBase::DisplayErrorL(TDesC& aMessage, TInt aErrorNumber)
	{
	//could check doesn't go out of bounds
	TBuf<250> errorMssge(aMessage);
	errorMssge.Append(_L(" fails with system error no. "));
	TBuf<3> errorNum;
	_LIT(KFormatDec, "%d");
	errorNum.Format(KFormatDec, aErrorNumber);
	errorMssge.Append(errorNum);
	iEikonEnv->InfoWinL(_L("Error"), errorMssge);
	}

void CCoreDumpDialogBase::DisplayErrorL(TDesC& aMessage)
	{
	//could check don't go out of bounds
	TBuf<250> errorMssge(aMessage);
	iEikonEnv->InfoWinL(_L("Error"), errorMssge);
	}


// CConfigurePluginDialog //

CConfigurePluginDialog::CConfigurePluginDialog(TUid aUid, RCoreDumpSession &aCoreDumpSession, TInt aInstance)
    : CCoreDumpDialogBase(aCoreDumpSession), iUid(aUid), iInstance(aInstance)
	{
	}

CConfigurePluginDialog::~CConfigurePluginDialog()
	{
	iConfs.ResetAndDestroy();
	}

void CConfigurePluginDialog::PreLayoutDynInitL()
	{
	UpdateDialogDataL();
	}

void CConfigurePluginDialog::PostLayoutDynInitL()
	{
	}

void CConfigurePluginDialog::UpdateDialogDataL()
	{
	//update dialog
	//RDebug::Print(_L("CConfigurePluginDialog::UpdateDialogDataL()"));
	LOG_MSG("->[UI]CConfigurePluginDialog::UpdateDialogDataL()");
	
	RPointerArray<CEikEdwin> edwins;
	RPointerArray<CEikLabel> labels;
	CEikEdwin *edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin1));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin2));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin3));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin4));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin5));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin6));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin7));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin8));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin9));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin10));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin11));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin12));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin13));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin14));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin15));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin16));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin17));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin18));
	edwins.Append(edwin);
	for (TInt i = 0; i < edwins.Count(); i++)
		{
		edwins[i]->SetDimmed(ETrue);
		edwins[i]->SetReadOnly(ETrue);
		}

	CEikLabel *label = static_cast<CEikLabel*>(Control(EPluginLabel1));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel2));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel3));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel4));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel5));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel6));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel7));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel8));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel9));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel10));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel11));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel12));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel13));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel14));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel15));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel16));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel17));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(EPluginLabel18));
	labels.Append(label);
	

	// Read config parameters from CDS
	//RDebug::Print(_L("Read config parameters from CDS"));
	TInt noConfigParams = 0;
	iConfs.ResetAndDestroy();
	
	LOG_MSG("\tiCoreDumpSession.GetNumberConfigParametersL()");
	TRAPD(err, noConfigParams = iCoreDumpSession.GetNumberConfigParametersL());
    if(err != KErrNone)
        {
		TBuf<23> error;
		error.Format(_L("Error: %d"), err);
		CEikonEnv::InfoWinL(_L("Could not get number of config params!"), error);
        User::Leave(err);
        }

	TInt control = 0;
	LOG_MSG2( "CConfigurePluginDialog::UpdateDialogDataL: noConfigParams: %d", noConfigParams);
	for (TInt i = 0; i < noConfigParams; i++)
		{
		COptionConfig* conf;
		RDebug::Printf("Getting ID = %d", i);
		conf = iCoreDumpSession.GetConfigParameterL(i);
		if (iUid == TUid::Uid(conf->Uid()) && iInstance == conf->Instance())
			{			
			iConfs.Append(conf);
			TBuf<255> buf = conf->Prompt();
			switch (conf->Type())
				{
				case COptionConfig::ETInt:
					{
					buf.Append(_L("\n(Signed int)"));
					TBuf<255> value;
					value.Format(_L("%d"), iConfs[control]->Value());
					edwins[control]->SetTextL(&value);
					}
					break;

				case COptionConfig::ETUInt:
					{
					buf.Append(_L("\n(Unsigned int)"));
					TBuf<255> value;
					value.Format(_L("%d"), iConfs[control]->Value());
					edwins[control]->SetTextL(&value);
					}
					break;

				case COptionConfig::ETString:
					{
					buf.Append(_L("\n(String)"));
					edwins[control]->SetTextL(&(iConfs[control]->ValueAsDesc()));
					}
					break;

				case COptionConfig::ETFileName:
					{
					buf.Append(_L("\n(File name)"));
					/*
					RDebug::Print(_L("edwins[control]->SetTextL(ValueAsDesc() Length=%d"),
						(iConfs[control]->ValueAsDesc()).Size() );
					*/
					edwins[control]->SetTextL(&(iConfs[control]->ValueAsDesc()));
					}
					break;

				case COptionConfig::ETSingleEntryEnum:
					{
					buf.Append(_L("\n"));
					buf.Append(conf->Options());
					edwins[control]->SetTextL(&(iConfs[control]->ValueAsDesc()));
					}
					break;

				case COptionConfig::ETMultiEntryEnum:
					{
					buf.Append(_L("\n"));
					buf.Append(conf->Options());
					edwins[control]->SetTextL(&(iConfs[control]->ValueAsDesc()));
					}
					break;

				case COptionConfig::ETBool:
					{
					buf.Append(_L("\n"));
					buf.Append(conf->Options());
					TBuf<255> value;
					iConfs[control]->Value() ? value = _L("True") : value = _L("False");
					edwins[control]->SetTextL(&value);
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

TBool CConfigurePluginDialog::OkToExitL(TInt aButtonId)
	{
	LOG_MSG( "-> CConfigurePluginDialog::::OkToExitL" );
	TBool ret = ETrue;
	TInt err;

	RPointerArray<CEikEdwin> edwins;
	CEikEdwin *edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin1));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin2));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin3));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin4));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin5));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin6));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin7));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin8));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin9));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin10));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin11));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin12));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin13));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin14));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin15));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin16));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin17));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EPluginEdwin18));
	edwins.Append(edwin);

	switch (aButtonId)
		{
		case EEikBidCancel:
			{
			//RDebug::Print(_L("CConfigurePluginDialog::OkToExitL: Cancel pressed"));
			}
			break;
		case EBidPluginConfigure:
			{
			RDebug::Print(_L("CConfigurePluginDialog::OkToExitL: iConfs.Count() == %d"), iConfs.Count());
			for (TInt i = 0; i < iConfs.Count(); i++)
				{
				err = KErrNone;
				HBufC *input = NULL;
				if (edwins[i]->TextLength() > 0)
					{
					input = edwins[i]->GetTextInHBufL();
					}
				if ( ! input )
					{
					input = HBufC::NewL(KMaxFileName);
					TPtr ptr(input->Des());
					ptr = KNullDesC;
					}

				RDebug::Print(_L("CConfigurePluginDialog::OkToExitL: input == %S"), input);
				RDebug::Print(_L("CConfigurePluginDialog::OkToExitL: iConfs[i]->Type() == %d"), iConfs[i]->Type());

				switch (iConfs[i]->Type())
					{
					case COptionConfig::ETInt:
					case COptionConfig::ETUInt:
						{
						TLex opts(*input);
						TInt32 conf;
						//RDebug::Print(_L("err = opts.Val(conf);"));
						err = opts.Val(conf);
						//RDebug::Print(_L("CConfigurePluginDialog::err = opts.Val(conf);"));
						if ( err )
							{
							LOG_MSG("CConfigurePluginDialog::CEikonEnv::Static()->InfoMsg;");
							CEikonEnv::Static()->InfoMsg(_L("Value must be an integer"));
							break;
							}

						iConfs[i]->Value(conf);
						TRAP( err, iCoreDumpSession.SetConfigParameterL( *iConfs[i]) );
						}
						break;
					case COptionConfig::ETFileName:
					case COptionConfig::ETString:
					case COptionConfig::ETSingleEntryEnum:
					case COptionConfig::ETMultiEntryEnum:
						{
						TRAP( err, iConfs[i]->ValueL(*input) );
						if ( KErrNone == err )
							{
							TRAP( err, iCoreDumpSession.SetConfigParameterL( *iConfs[i]) );
							}
						}
						break;

					case COptionConfig::ETBool:
						if (input->CompareF(_L("True"))== 0)
							{
							//RDebug::Print(_L("input->CompareF(_L(True))== 0"));
							iConfs[i]->Value(ETrue);
							iConfs[i]->ValueL(_L("True"));
							}
						else if (input->CompareF(_L("False"))== 0)
							{
							//RDebug::Print(_L("input->CompareF(_L(False))== 0"));
							iConfs[i]->Value(EFalse);
							iConfs[i]->ValueL(_L("False"));
							}
						else
							{
							LOG_MSG( "  CConfigurePluginDialog::OkToExitL err = KErrCorrupt" );
							err = KErrCorrupt;
							}

						if( KErrNone == err )
							{
							TRAP( err, iCoreDumpSession.SetConfigParameterL( *iConfs[i]) );
							RDebug::Print(_L("CConfigurePluginDialog::OkToExitL: SetConfigParameterL ret %d"), err);
							}
						break;

					default:
						CEikonEnv::Static()->InfoMsg(_L("Error with parameter type"));
						LOG_MSG("CConfigurePluginDialog::OkToExitL: none of the above" );
						LOG_MSG("default err = KErrCorrupt");
						err = KErrCorrupt;						
						break;
					}

				if ( err != KErrNone )
					{
					RBuf errorString;
					CleanupClosePushL( errorString );
					errorString.CreateL( 128 );
					errorString.Append( _L("Config Plugin - Error ") ); 
					errorString.AppendNum( err );
					errorString.Append( _L(" setting parameter number ") );
					errorString.AppendNum( i );
					CEikonEnv::InfoWinL( errorString, KNullDesC );
					CleanupStack::PopAndDestroy( &errorString );
					ret = EFalse;
					}
                if(input)
                    {
                    delete input;
                    input = NULL;
                    }
				}//for
			}//switch
			break;
		default:
			break;
		}
	//RDebug::Print(_L("<--------- CConfigurePluginDialog::OkToExitL"));
    edwins.Close();
	return ret;
	}

