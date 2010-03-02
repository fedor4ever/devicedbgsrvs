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


#include "coredumpui.h"
#include "coredumpcdsdialog.h"

// CConfigureCDSDialog //

CConfigureCDSDialog::CConfigureCDSDialog(RCoreDumpSession &aCoreDumpSession)
    : CCoreDumpDialogBase(aCoreDumpSession)
    {
    }

CConfigureCDSDialog::~CConfigureCDSDialog()
	{
	iConfs.ResetAndDestroy();
	}

void CConfigureCDSDialog::PreLayoutDynInitL()
	{
	UpdateDialogDataL();
	}

void CConfigureCDSDialog::PostLayoutDynInitL()
	{
	}

void CConfigureCDSDialog::UpdateDialogDataL()
	{
	//update dialog
	RPointerArray<CEikEdwin> edwins;
	CEikEdwin *edwin = static_cast<CEikEdwin*>(Control(EEdwin1));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EEdwin2));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EEdwin3));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EEdwin4));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EEdwin5));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EEdwin6));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EEdwin7));
	edwins.Append(edwin);
	for (TInt i = 0; i < edwins.Count(); i++)
		{
		edwins[i]->SetDimmed(ETrue);
		edwins[i]->SetReadOnly(ETrue);
		}

	RPointerArray<CEikLabel> labels;
	CEikLabel *label = static_cast<CEikLabel*>(Control(ELabel1));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(ELabel2));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(ELabel3));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(ELabel4));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(ELabel5));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(ELabel6));
	labels.Append(label);
	label = static_cast<CEikLabel*>(Control(ELabel7));
	labels.Append(label);

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
	//RDebug::Print(_L("CConfigureCDSDialog::UpdateDialogDataL: noConfigParams: %d"), noConfigParams);
	for (TInt i = 0; i < noConfigParams; i++)
		{
		COptionConfig* conf;
		conf = iCoreDumpSession.GetConfigParameterL(i);
		if (conf->Source()== COptionConfig::ECoreDumpServer)
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

TBool CConfigureCDSDialog::OkToExitL(TInt aButtonId)
	{
	LOG_MSG( "-> CConfigureCDSDialog::OkToExitL" );
	TBool ret = ETrue;
	TInt err;

	RPointerArray<CEikEdwin> edwins;
	CEikEdwin *edwin = static_cast<CEikEdwin*>(Control(EEdwin1));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EEdwin2));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EEdwin3));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EEdwin4));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EEdwin5));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EEdwin6));
	edwins.Append(edwin);
	edwin = static_cast<CEikEdwin*>(Control(EEdwin7));
	edwins.Append(edwin);

	switch (aButtonId)
		{
		case EEikBidCancel:
			{
			//RDebug::Print(_L("CConfigureCDSDialog::OkToExitL: Cancel pressed"));
			}
			break;
		case EBidCDSConfigure:
			{
			//RDebug::Print(_L("CConfigureCDSDialog::OkToExitL: iConfs.Count() == %d"), iConfs.Count());
			for (TInt i = 0; ( i < iConfs.Count() ) && ret; i++)
				{
				err = KErrNone;

				HBufC *input = NULL;
				if (edwins[i]->TextLength() > 0)
					{
					input = edwins[i]->GetTextInHBufL();
					}
				if ( ! input )
					{
					LOG_MSG2( "  NULL input for param %d" , i );
					continue;
					}

				//RDebug::Print(_L("CConfigureCDSDialog::OkToExitL: iConfs[i]->Type() == %d"), iConfs[i]->Type());

				switch (iConfs[i]->Type())
					{
					case COptionConfig::ETInt:
					case COptionConfig::ETUInt:
						{
						TLex opts(*input);
						TInt32 conf;
						err = opts.Val(conf);
						if ( err )
							{
							LOG_MSG("  error from opts.Val(conf) -> CConfigureCDSDialog::CEikonEnv::Static()->InfoMsg;" );
							CEikonEnv::Static()->InfoMsg(_L("Value must be an integer"));
							break;
							}

						//RDebug::Print(_L("CConfigureCDSDialog::OkToExitL: conf == %d"), conf);
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
							iConfs[i]->Value(ETrue);
							iConfs[i]->ValueL(_L("True"));
							}
						else if (input->CompareF(_L("False"))== 0)
							{
							iConfs[i]->Value(EFalse);
							iConfs[i]->ValueL(_L("False"));
							}
						else
							{
							err = KErrCorrupt;
							}

						if ( KErrNone == err )
							{
							TRAP( err, iCoreDumpSession.SetConfigParameterL( *iConfs[i]) );
							}

						break;

					default:
						CEikonEnv::Static()->InfoMsg(_L("Error with parameter type"));
						LOG_MSG( "CConfigureCDSDialog::OkToExitL: none of the above" );
						err = KErrCorrupt;
						break;

					}//switch

				if ( err )
					{
					RBuf errorString;
					CleanupClosePushL( errorString );
					errorString.CreateL( 128 );
					errorString.Append( _L("CDS Error ") ); 
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
				}
			}
			break;
		default:
			break;
		}

    edwins.Close();
	return ret;
	}


