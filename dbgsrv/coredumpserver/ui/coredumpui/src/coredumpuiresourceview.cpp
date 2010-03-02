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
// Source file for the implementation of the 
// application view class - CResourceView
//

#include "coredumpui.h"

CResourceView::CResourceView()
	{
	}

CResourceView* CResourceView::NewL(const TRect& aRect, CResourceAppUi *aAppUi)
	{
	CResourceView* self = new(ELeave) CResourceView();
	CleanupStack::PushL(self);
	self->ConstructL(aRect, aAppUi);
	CleanupStack::Pop();
	return self;
	}

CResourceView::~CResourceView()
	{
    delete iCommandButton;
    delete iObserveButton;
    delete iLoadButton;
	}

void CResourceView::ConstructL(const TRect& aRect, CResourceAppUi* aAppUi)
    {
	// Initialisation
	iAppUi = aAppUi;
	iObjectsViewed = EObjectProcess;

	CreateWindowL();
	           
	// Whole extent of view
	SetRect(aRect);

	// Border around components in pixels
	iBorder = 1;

	// Place the refresh button	
	iCommandButton = new(ELeave) CEikCommandButton;
	iCommandButton->SetTextL(_L("Refresh"));
	iCommandButton->SetSize(iCommandButton->MinimumSize());
	iCommandButton->SetPosition(TPoint(Rect().Width() - iCommandButton->Size().iWidth - iBorder, iBorder));
	iCommandButton->SetContainerWindowL(*this);
	iCommandButton->SetObserver(this);

	// Place the observe button	
	iObserveButton = new(ELeave) CEikCommandButton;
	iObserveButton->SetTextL(_L("Observe"));
	iObserveButton->SetSize(iObserveButton->MinimumSize());
	TInt x = Rect().Width() - iObserveButton->Size().iWidth - iCommandButton->Size().iWidth - iBorder;
	iObserveButton->SetPosition(TPoint(x , iBorder));
	iObserveButton->SetContainerWindowL(*this);
	iObserveButton->SetObserver(this);

	// Place the load button
	iLoadButton = new(ELeave) CEikCommandButton;
	iLoadButton->SetTextL(_L("Load"));
	iLoadButton->SetSize(iLoadButton->MinimumSize());
	x -= iLoadButton->Size().iWidth;
	iLoadButton->SetPosition(TPoint(x , iBorder));
	iLoadButton->SetContainerWindowL(*this);
	iLoadButton->SetObserver(this);
	iLoadButton->SetDimmed(ETrue);

	TInt buttonHeight = iCommandButton->Size().iHeight;	

	// Get the title font height
	TInt titleHeight = iEikonEnv->TitleFont()->HeightInPixels();
	TInt tablePosition = 0;
	if (titleHeight>=buttonHeight) 
		tablePosition=titleHeight;
	else
		tablePosition=buttonHeight;

	CResourceList::ConstructL(tablePosition);// Activate the main window control - this will also activate the contained controls
	
	ActivateL();
	DrawNow();
	
	}

TDes& CResourceView::CrashProgress()
	{
    return iCrashProgress;
	}

void CResourceView::Draw(const TRect& /*aRect*/) const
	{
	CWindowGc& gc = SystemGc();                     

	gc.Clear();
		
	// Put title on view
	const CFont* fontUsed = iEikonEnv->TitleFont();
	gc.UseFont(fontUsed);
	gc.SetPenColor(KRgbBlack);
	TPoint pos(iBorder,fontUsed->AscentInPixels()+iBorder);
	gc.DrawText(iTitle, pos);

	gc.DiscardFont();
	}

TInt CResourceView::CountComponentControls() const
	{
	return 4;
	}

CCoeControl* CResourceView::ComponentControl(TInt aIndex) const
	{
	CCoeControl* cmd = NULL;
	switch (aIndex)
		{
		case 0:
			{
			cmd = iCommandButton;
			break;
			}		
		case 1:
			{
			cmd = iObserveButton;
			break;
			}		
		case 2:
			{
			cmd = iLoadButton;
			break;
			}		
		default:
			{
			cmd = CResourceList::GetListBox();
			}
		}
	return cmd;
	}

// for use by refresh button control
void CResourceView::HandleControlEventL(CCoeControl* aControl,TCoeEvent /*aEventType*/)
	{	
	// This code is not nice. A better solution is to extend the CEikCommandButton class to have an enumerator 
	// that distinguishes the buttons purpose in a given context. This avoids relying on the text of the label. I dont
	// have the time to do it now. Also, the button names make no sense. They were named originally in the context of 
	// 1 function, now they serve many
	
	LOG_MSG("->[UI]CResourceView::HandleControlEventL()");
	if (aControl == iCommandButton)
		{
		if(iCommandButton->Label()->Text()->Compare(_L("Refresh")) == 0)
			{
			UpdateViewL();
			}
		else if(iCommandButton->Label()->Text()->Compare(_L("Bind")) == 0)
			{
			BindPluginsL();
			}
		UpdateViewL();
		}
	else if (aControl == iObserveButton)
		{
		if (iObserveButton->Label()->Text()->Compare(_L("Configure"))==0)
			{
			ConfigurePluginL();
			}
		else if(iObserveButton->Label()->Text()->Compare(_L("Observe"))==0)
			{
			SetObservedL();
			}
		else if(iObserveButton->Label()->Text()->Compare(_L("Process"))==0)
			{
			ProcessCrashL();
			}
		UpdateViewL();
		}
	else if (aControl == iLoadButton)
		{
		if(iLoadButton->Label()->Text()->Compare(_L("Load")) == 0)
			{
			LoadPluginL(); 
			}
		else if(iLoadButton->Label()->Text()->Compare(_L("Unload")) == 0)
			{
			UnloadPluginL(); 
			}
		else if(iLoadButton->Label()->Text()->Compare(_L("Delete")) == 0)
			{
			DeleteEntireCrashPartitionL();
			}
		UpdateViewL();
		}
	}

void CResourceView::InvalidateView(void) const
	{
	Window().Invalidate(Rect()); //calls Draw()
	}

void CResourceView::UpdateViewL(void)
	{
	LOG_MSG("->[UI]CResourceView::UpdateView(void)");
	UpdateListDataL();	
	InvalidateView();
	UpdateTitleL();
	}

void CResourceView::UpdateCrashProgressL()
{    
    InvalidateView();
    UpdateTitleL();
}

void CResourceView::UpdateTitleL(void)
	{
	// assume the table data, iResourceCount and iObjectViewed data are up to date
	TBufC<20> title;

	// Set the title appropriately
	switch (iObjectsViewed)
		{
		case EObjectProcess: 
			{
			iLoadButton->SetDimmed(ETrue);
			iObserveButton->SetTextL(_L("Observe"));
			title = _L("Processes");
			iCommandButton->SetTextL(_L("Refresh"));
			break;
			}
		case EObjectThread:
			{
			iLoadButton->SetDimmed(ETrue);
			iObserveButton->SetTextL(_L("Observe"));
			title = _L("Threads");
			iCommandButton->SetTextL(_L("Refresh"));
			break;
			}
		case EObjectPlugin:
			{
			iLoadButton->SetDimmed(EFalse);
			title = _L("Plugins");
			// for plugins, also change the label of the observe button to Configure
			iObserveButton->SetDimmed(ETrue);
			iLoadButton->SetTextL(_L("Load"));
			iLoadButton->SetDimmed(EFalse);
			iCommandButton->SetTextL(_L("Refresh"));
			break;
			}
		case EObjectExecutable:
			{
			iLoadButton->SetDimmed(ETrue);
			title = _L("Executables");
			iObserveButton->SetTextL(_L("Observe"));
			iObserveButton->SetDimmed(EFalse);
			iCommandButton->SetTextL(_L("Refresh"));
			break;
			}
		case EObjectPluginInstance:
			{
			title = _L("Plugin Instances");
			iObserveButton->SetTextL(_L("Configure"));
			iObserveButton->SetDimmed(EFalse);
			iLoadButton->SetTextL(_L("Unload"));
			iLoadButton->SetDimmed(EFalse);
			iCommandButton->SetTextL(_L("Bind"));
			break;
			}
		case EObjectCrashList:
			{
			title = _L("Crash List in Flash");
			iObserveButton->SetTextL(_L("Process"));
			iObserveButton->SetDimmed(EFalse);
			iLoadButton->SetTextL(_L("Delete"));
			iLoadButton->SetDimmed(EFalse);
			iCommandButton->SetTextL(_L(""));
			iCommandButton->SetDimmed(ETrue);
			break;
			}
		default:
			{
			break;
			}
		}
		
	iObserveButton->SetSize(iObserveButton->MinimumSize());
	TInt x = Rect().Width() - iObserveButton->Size().iWidth - iCommandButton->Size().iWidth - iBorder;
	iObserveButton->SetPosition(TPoint(x, iBorder));
	iObserveButton->SetContainerWindowL(*this);
	iObserveButton->SetObserver(this);
    
	iLoadButton->SetSize(iLoadButton->MinimumSize());
	x -= iLoadButton->Size().iWidth;
	iLoadButton->SetPosition(TPoint(x, iBorder));
	iLoadButton->SetContainerWindowL(*this);
	iLoadButton->SetObserver(this);

	//set title and resource count
	TBuf<2> openingBracket(_L(" ("));
	TBuf<1> closingBracket(_L(")"));
	iTitle.Format(_L("%S%S%u%S %S"), &title, &openingBracket, iResourceCount, &closingBracket, &iCrashProgress);
	}
