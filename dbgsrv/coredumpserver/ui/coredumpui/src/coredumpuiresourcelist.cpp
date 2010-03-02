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


#include <eiktxlbm.h> //CTextListBoxModel

#include <eiklbv.h> // CListBoxView
#include <e32std.h>

#include "coredumpui.h"
//#include <tefexportconst.h>
//#include <e32def.h>

#ifndef HW
#include <formatterapi.h>
#include <crashdatasave.h>
#include <optionconfig.h>

#endif

const TInt TObservationRequest::iOffset=  _FOFF(TObservationRequest, iLink);

void CResourceList::ConstructL(TInt aTablePosition)
	{
	// Create the basic list box
	iListBox = new(ELeave) CEikColumnListBox;
	//This view is parent. Also constructs the associated list box model and item drawer for the list box.
	iListBox->ConstructL( this, CEikListBox::EMultipleSelection);

	iListBox->SetContainerWindowL( *this);
	iListBox->CreateScrollBarFrameL();
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EAuto,
			CEikScrollBarFrame::EAuto);

	iListBox->SetPosition(TPoint(iBorder, aTablePosition + iBorder));

	iListBox->SetSize(Rect().Size()-TSize(0, aTablePosition)-TSize(iBorder*2, iBorder*2));

	iListBox->ActivateL();

	iModel = iListBox->Model();

	iTextArray = iModel->ItemTextArray();
	iListBoxItems = static_cast<CDesCArray*>(iTextArray);
	iListBoxData = static_cast<CColumnListBoxItemDrawer*>(iListBox->View()->ItemDrawer())->ColumnData();
	
	iRowItem.CreateL(500);

#ifdef HW
	LOG_MSG("CResourceList::ConstructL: iCoreDumpSession->Connect() \n");
    TInt err = iCoreDumpSession.Connect();
	if (err != KErrNone)
		{
		LOG_MSG2("CResourceList::ConstructL: iCoreDumpSession->Connect() failed!, err:%d\n", err);
        User::Leave(err);
		}
#endif
	}

CResourceList::~CResourceList()
	{
	delete iListBox;
	iModel = NULL;
	iTextArray = NULL;
	iListBoxItems = NULL;
	iListBoxData = NULL;
	iCoreDumpSession.Disconnect();
	iRowItem.Close();
	
	iCrashList.ResetAndDestroy();
	iCrashList.Close();
	
	}

CCoeControl* CResourceList::GetListBox() const
	{
	return iListBox;
	}

void CResourceList::EmptyList()
	{
	iListBoxItems->Reset();
	iListBox->Reset();
	}

void CResourceList::ConfigurePluginL()
	{
	LOG_MSG( "->[UI]CResourceList::ConfigurePlugin" );
	const CListBoxView::CSelectionIndexArray * indexes = iListBox->SelectionIndexes();

	if (indexes->Count()> 0)
		{
		TPtrC e = (*iListBoxItems)[indexes->At(0)];
				
		TLex input(e);	
		
		//Get the instance ID
		input.Mark();
		input.SkipCharacters();
		TInt instanceId;
		// extract ID
		if (input.TokenLength()) // if valid potential token
			{
			TLex inst(input.MarkedToken());
			inst.Val(instanceId);
			}
		LOG_MSG2("\tPlugin Instance ID = %d", instanceId);

		do 
			{
			input.Get();
			}
		while (input.Peek()!= '\t');

		
		input.SkipSpace();
		input.Mark();
		input.SkipCharacters();
		TUint u;
		// extract UID
		if (input.TokenLength()) // if valid potential token
			{
			TPtrC p = input.MarkedToken();
			TLex value(p.Mid(2)); // leave out 0x
	        value.Val(u, EHex);
			}
		TUid uid = TUid::Uid(u);
		LOG_MSG2( "\tCResourceList::ConfigurePlugin, UID in hex: %x" , u);

		RPluginList data;
		CleanupClosePushL(data);
		LOG_MSG( "\tCResourceList::GetPluginList iCoreDumpSession->GetPluginsL(data)" );
		TRAPD(err, iCoreDumpSession.GetPluginListL(data));
		if (err != KErrNone)
			{
			TBuf<23> buf;
			buf.Format(_L("Error: %d"), err);
			CEikonEnv::InfoWinL(_L("Could not get plugin list from CDS!"), buf);
            User::Leave(err);
			}

		CleanupStack::PopAndDestroy(&data);
		CConfigurePluginDialog* dialog = new(ELeave) CConfigurePluginDialog(uid, iCoreDumpSession, instanceId);
		
		dialog->ExecuteLD(R_CONFIGURE_PLUGIN_DIALOG);
		}

	}

void CResourceList::BindPluginsL()
	{
	//Get selected items
	LOG_MSG("->[UI]CResourceList::BindPluginsL()");
	const CListBoxView::CSelectionIndexArray* indexes = iListBox->SelectionIndexes();
	TInt numSelected = indexes->Count();
	
	TBuf<50> errBuf;
	errBuf.Format(_L("Must Select one formatter and one writer to bind"));
	
	TInt writerInstance = -1;
	TInt formatterInstance = -1;

	for(TInt cnt =0; cnt < numSelected; cnt++)
		{
		TPtrC e = (*iListBoxItems)[indexes->At(cnt)];
		
		TLex input(e);	
		
		//Get the instance ID
		input.Mark();
		input.SkipCharacters();
		TInt instanceId;
		// extract ID
		if (input.TokenLength()) // if valid potential token
			{
			TLex inst(input.MarkedToken());
			inst.Val(instanceId);
			}
		LOG_MSG2("\tPlugin Instance ID = %d", instanceId);

		do 
			{
			input.Get();
			}
		while (input.Peek()!= '\t');

		//head on to the last tab
		input.SkipSpace();
		input.Mark();
		input.SkipCharacters();			
		
		input.SkipSpace();
		input.Mark();
		input.SkipCharacters();
		// Extract plugin type
		if (input.TokenLength()) // if valid potential token
			{
			TPtrC p = input.MarkedToken();
			if (p.Compare(_L("Formatter"))== 0)
				{
				formatterInstance = instanceId;
				}
			else if (p.Compare(_L("Writer"))== 0)
				{
				writerInstance = instanceId;
				}
			}
		
		}
	
	LOG_MSG3("\tFormatter Instance ID = %d will be bound to Writer Instance ID = %d", formatterInstance, writerInstance);
	
	if(formatterInstance < 0 || writerInstance < 0)
		{
		CEikonEnv::InfoWinL(_L("Cannot bind"), errBuf);
		return;
		}

	//Now we have a fomatter and writer and so can bind
	TPluginRequest bindReq;
	bindReq.iIndex = formatterInstance;
	bindReq.iPair = writerInstance;
	bindReq.iUid = TUid::Uid(0);
	
#ifdef HW	
	TRAPD(ret, iCoreDumpSession.PluginRequestL(bindReq));
	if(ret != KErrNone)
		{
		errBuf.Format(_L("Error: %d"), ret);
		CEikonEnv::InfoWinL(_L("Cannot bind"), errBuf);
		return;
		}	
#endif
	}

void CResourceList::DeleteEntireCrashPartitionL()
	{
	LOG_MSG("->[UI]CResourceList::DeleteCrashPartitionL()");
	
	iCoreDumpSession.DeleteCrashPartitionL();
	}

/**
 * Deletes the crash via the core dump server interface, using the UI's currently selected crash(es)
 * @return void
 */
void CResourceList::DeleteCrashLogL()
	{
	LOG_MSG("->[UI]CResourceList::DeleteCrashLogL()");	
	
	RArray<TInt>* crashIds = GetMarkedCrashIDsFromUIL();
	
	for(TInt cnt = crashIds->Count() - 1; cnt >= 0; cnt--)
		{	
		iCoreDumpSession.DeleteCrashLogL((*crashIds)[cnt]);
		}
	
	crashIds->Close();
	delete crashIds;
	}

/**
 * Processes the crash via the core dump server interface, using the UI's currently selected crash(es)
 * @return void
 */
void CResourceList::ProcessCrashL()
	{
	LOG_MSG("->[UI]CResourceList::ProcessCrashL()");
	
	RArray<TInt>* crashIds = GetMarkedCrashIDsFromUIL();
	
	if(!crashIds)
		{
		CEikonEnv::Static()->InfoMsg(_L("No crashes Selected (null crashIds"));		
		}
	else if (crashIds->Count() ==  0)
		{
		CEikonEnv::Static()->InfoMsg(_L("No crashes Selected"));
		}
	else
		{		
		for(TInt cnt = crashIds->Count() -1; cnt >= 0; cnt--)
			{
	/*		TCrashInfo* inf = GetCrashInfoL((*crashIds)[cnt]);		
			iCoreDumpSession.ProcessCrashLogL(*inf);*/
			iCoreDumpSession.ProcessCrashLogL((*crashIds)[cnt]);
			}	
		}
	crashIds->Close();
	delete crashIds;
	}

/**
 * This helper method looks at the UI Panel and returns an RArray of crashID's for those marked .
 * Note: Ownership of this array is passed on to the caller
 * @return RArray of the crashID's marked
 */
RArray<TInt>* CResourceList::GetMarkedCrashIDsFromUIL()
	{
	LOG_MSG("->[UI]CResourceList::GetMarkedCrashIDsFromUIL()");
	
	RArray<TInt>* crashIds = new RArray<TInt>();
	
	const CListBoxView::CSelectionIndexArray* indexes = iListBox->SelectionIndexes();
	
	for(TInt cnt = 0; cnt < indexes->Count(); cnt++)
		{
		TPtrC item = (*iListBoxItems)[indexes->At(cnt)];
		TLex input(item);
		
		//we want to get the crash ID from the UI display - its the first character
		input.SkipSpace();
		input.Mark();
		input.SkipCharacters();
		
		TUint crashId;
		if (input.TokenLength()) // if valid potential token
			{
			TLex value(input.MarkedToken());
			value.Val(crashId, EDecimal);
			
			crashIds->AppendL(crashId);
			}		
		}
	
	return crashIds;
	}

/**
 * This returns a pointer to the TCrashInfo object in our cached array. Leaves if not found.
 * @param aCrashId
 * @return Pointer to crash info object
 * @leave one of the OS wide Codes
 */
TCrashInfo* CResourceList::GetCrashInfoL(const TInt aCrashId)
	{	
	LOG_MSG2("->[UI]CResourceList::GetCrashInfoL(crashID = [%d])", aCrashId);
	
	for(TInt cnt = iCrashList.Count() -1; cnt >= 0; cnt--)
		{
		if(iCrashList[cnt]->iCrashId == aCrashId)
			{
			return iCrashList[cnt];
			}
		}
	
	//if we get here, we havent found it
	User::Leave(KErrNotFound);
	
	return NULL; //unreachable, but avoid compiler warnings
	}

void CResourceList::UnloadPluginL()
	{
	LOG_MSG("->[UI]CResourceList::UnloadPlugin()");
	TPluginRequest unloadReq;
	
	const CListBoxView::CSelectionIndexArray* indexes = iListBox->SelectionIndexes();
	
	for(TInt cnt = 0; cnt < indexes->Count(); cnt++)
		{
		TPtrC item = (*iListBoxItems)[indexes->At(cnt)];
		TLex input(item);
		
		TInt tabCnt =0;
		while(tabCnt < 2)
			{
			do 
				{
				input.Get();
				}
			while (input.Peek()!= '\t');
			tabCnt++;
			}
		input.SkipSpace();
		input.Mark();
		input.SkipCharacters();
		TUint u;
		// extract UID
		if (input.TokenLength()) // if valid potential token
			{
			TPtrC p = input.MarkedToken();
			TLex value(p.Mid(2)); // skip 0x
			value.Val(u, EHex);
			}
		TUid uid = TUid::Uid(u);
		unloadReq.iUid = uid;
		LOG_MSG2("\tUID to unload = %x", uid);
		
		//Now get the type...
		input.SkipSpace();
		input.Mark();
		input.SkipCharacters();
		// Extract plugin type
		if (input.TokenLength()) // if valid potential token
			{
			TPtrC p = input.MarkedToken();
			if (p.Compare(_L("Formatter"))== 0)
				{
				unloadReq.iPluginType = TPluginRequest::EFormatter;
				}
			else if (p.Compare(_L("Writer"))== 0)
				{
				unloadReq.iPluginType = TPluginRequest::EWriter;
				}
			}				
		}
	
	unloadReq.iLoad = EFalse;
	
#ifdef HW
	TRAPD(err, iCoreDumpSession.PluginRequestL(unloadReq));
	if (err != KErrNone)
		{
		TBuf<23> buf;
		buf.Format(_L("Error: %d"), err);
		CEikonEnv::InfoWinL(_L("Could not load plugin"), buf);
		LOG_MSG2("\tCResourceList::LoadPlugin could not load plugin: %d", err);
		}
#endif	
	}

void CResourceList::LoadPluginL()
	{
	LOG_MSG("->[UI]CResourceList::LoadPluginL()");
	const CListBoxView::CSelectionIndexArray* indexes = iListBox->SelectionIndexes();
	TInt numSelected = indexes->Count();
	
	for(TInt cnt =0; cnt < numSelected; cnt++)
		{
		TPtrC e = (*iListBoxItems)[indexes->At(cnt)];	
		TPluginRequest loadReq;

		TLex input(e);
		do // skip all text until first \t
			{
			input.Get();
			}
		while (input.Peek()!= '\t');
		input.SkipSpace();
		input.Mark();
		input.SkipCharacters();
		TUint u;
		// extract UID
		if (input.TokenLength()) // if valid potential token
			{
			TPtrC p = input.MarkedToken();
			TLex value(p.Mid(2)); // skip 0x
			value.Val(u, EHex);
			}
		TUid uid = TUid::Uid(u);
		loadReq.iUid = uid;
		input.SkipSpace();
		input.Mark();
		input.SkipCharacters();
		
		
		// Extract plugin type
		if (input.TokenLength()) // if valid potential token
			{
			TPtrC p = input.MarkedToken();
			if (p.Compare(_L("Formatter"))== 0)
				{
				loadReq.iPluginType = TPluginRequest::EFormatter;
				}
			else if (p.Compare(_L("Writer"))== 0)
				{
				loadReq.iPluginType = TPluginRequest::EWriter;
				}
			}

		loadReq.iLoad = ETrue;					

#ifdef HW
		TRAPD(err, iCoreDumpSession.PluginRequestL(loadReq));
		if (err != KErrNone)
			{
			TBuf<23> buf;
			buf.Format(_L("Error: %d"), err);
			CEikonEnv::InfoWinL(_L("Could not load plugin"), buf);
			LOG_MSG2("\tCResourceList::LoadPlugin could not load plugin: %d", err);
			}
#endif
		}
	}

void CResourceList::BuildSelectedListL(TSglQue<TObservationRequest> &aSelectedList)
	{
	const CListBoxView::CSelectionIndexArray
			* indexes = iListBox->SelectionIndexes();

    LOG_MSG2("CResoutceList::BuildSelectedListL - no of indexes:%d\n", indexes->Count());

	for (TInt i = 0; i < indexes->Count(); i++)
		{
		// extract the data
		TObservationRequest* cdsData = new (ELeave)TObservationRequest;
		TPtrC line = (*iListBoxItems)[indexes->At(i)];
		switch (iObjectsViewed)
			{
			case EObjectProcess:
				{
				TPtrC namePlus = line.Mid(1+line.Locate('\t')); //skip index
                TPtrC onlyName = namePlus.Mid(0, namePlus.Locate('\t')); //cut off the rest

                //LOG_DES(onlyName);
				cdsData->iTargetName = onlyName;
				cdsData->iTargetOwnerName = onlyName;
				}
				break;

			case EObjectThread:
				{
                TPtrC namePlus = line.Mid(1+line.Locate('\t')); //skip index
                TPtrC onlyName = namePlus.Mid(0, namePlus.Locate('\t')); //cut off the rest
                cdsData->iTargetName = onlyName;

                TPtrC priorityPlus = namePlus.Mid(1+namePlus.Locate('\t')); //skip name
                TPtrC ownerPlus = priorityPlus.Mid(1+priorityPlus.Locate('\t')); //skip priority

                //LOG_DES(ownerPlus);
                TLex value(ownerPlus);
				TInt ownerId;
                value.Val(ownerId);

                LOG_MSG2("owner id:%d\n", ownerId);
				RProcess rp;
				TInt err = rp.Open(ownerId);
                if(err != KErrNone)
                    {
                    LOG_MSG2("unable to open proces handle! err:%d", err);
                    delete cdsData;
                    User::Leave(err);
                    }
                cdsData->iTargetOwnerName.Copy(rp.FileName());
                rp.Close();
				}
				break;

			case EObjectExecutable:
				{
				TPtrC name = line.Mid(0, line.Locate('\t'));

				cdsData->iTargetName = name;
				cdsData->iTargetOwnerName = name;
				}

			default:
				break;
			}
		aSelectedList.AddLast(*cdsData);
		}
	}

void CResourceList::SetObservedL()
	{

	switch (iObjectsViewed)
		{
		case EObjectProcess:
			{
			SetObservedProcessesL();
		    break;
			}

		case EObjectThread:
			{
			SetObservedThreadsL();
		    break;
			}
		case EObjectExecutable:
			{
			SetObservedExecutablesL();
			break;
			}

		default:
		break;
		}
	}

/**
    Cleanup item implementation for selection list in SetObserved() methods.
 */
void CResourceList::CleanupSelection(TAny *aList)
{
	TSglQue<TObservationRequest> *selectedList = static_cast<TSglQue<TObservationRequest>*>(aList);
	TSglQueIter<TObservationRequest> selIter(*selectedList);
    selIter.SetToFirst();
    TObservationRequest *selItem;
    while( (selItem = selIter++) != NULL)
        {
        selectedList->Remove(*selItem);
        delete selItem;
        }
}

/**
    Cleanup item implementation for process list in SetObservedProcesses() method.
 */
void CResourceList::CleanupProcessList(TAny *aArray)
{
	RProcessPointerList *processList = static_cast<RProcessPointerList*> (aArray);
    processList->ResetAndDestroy();
    processList->Close();
}

void CResourceList::SetObservedProcessesL()
{
    LOG_MSG("->CResourceList::SetObservedProcesses()\n");

	RProcessPointerList processList;
    TCleanupItem cleanupProcesses(CResourceList::CleanupProcessList, (TAny*)&processList);
    CleanupStack::PushL(cleanupProcesses);

   	TRAPD(err, iCoreDumpSession.GetProcessesL(processList));
    if(err != KErrNone)
        {
        LOG_MSG2("CResourceList::SetObservedProcesses - unable get process list! err:%d\n", err);
        TBuf<23> error;
        error.Format(_L("Error: %d"), err);
        CEikonEnv::InfoWinL(_L("Unable to get process list!"), error);
        User::Leave(err);
        }

	TSglQue<TObservationRequest> selectedList(TObservationRequest::iOffset);
    TCleanupItem cleanupSelection(CResourceList::CleanupSelection, (TAny*)&selectedList);
    CleanupStack::PushL(cleanupSelection);
	BuildSelectedListL(selectedList);

	TSglQueIter<TObservationRequest> selIter(selectedList);
    TObservationRequest *selItem;
    selIter.SetToFirst();
    while((selItem = selIter++) != NULL)
        {
        //locate and inspect the target
        TBool observe = ETrue;
        for(TInt i = 0; i < processList.Count(); i++)
            {
            if(processList[i]->Name() == selItem->iTargetName)
                {
                //choose the right action - toggle states
                observe = !processList[i]->Observed();
                break;
                }
            }

	    LOG_MSG("CResourceList::SetObservedProcesses -> iCoreDumpSession->ObservationRequestL()\n");
        TRAP(err, iCoreDumpSession.ObservationRequestL(selItem->iTargetName, selItem->iTargetOwnerName, observe));
        if (err != KErrNone)
            {
	        LOG_MSG3("CResourceList::SetObservedProcesses - unable to perform action:%d! err:%d\n", observe, err);
            TBuf<23> error;
            RBuf buf;
            buf.CreateL(KMaxFullName);
            error.Format(_L("Error: %d"), err);
            if(observe)
                buf.Append(_L("Unable to attach to "));
            else
                buf.Append(_L("Unable to detach from "));

            buf.AppendFormat(_L("process:%S"), &selItem->iTargetName);
            CEikonEnv::InfoWinL(buf, error);
            buf.Close();
            }
        }
        CleanupStack::PopAndDestroy(2); //results in calling CleanupProcessList and CleanupSelection
}

void CResourceList::SetObservedExecutablesL()
	{
	LOG_MSG("->CResourceList::SetObservedExecutablesL()\n");

	RExecutablePointerList exeList;
	TCleanupItem cleanupExes(CResourceList::CleanupExecutableList, (TAny*)&exeList);
	CleanupStack::PushL(cleanupExes);

	TRAPD(err, iCoreDumpSession.GetExecutablesL(exeList));
    if(err != KErrNone)
        {
        LOG_MSG2("CResourceList::SetObservedExecutablesL - unable get exe list! err:%d\n", err);
        TBuf<23> error;
        error.Format(_L("Error: %d"), err);
        CEikonEnv::InfoWinL(_L("Unable to get executable list"), error);
        User::Leave(err);
        }

	TSglQue<TObservationRequest> selectedList(TObservationRequest::iOffset);
    TCleanupItem cleanupSelection(CResourceList::CleanupSelection, (TAny*)&selectedList);
    CleanupStack::PushL(cleanupSelection);
	BuildSelectedListL(selectedList);
    LOG_MSG("selected list built");

    TSglQueIter<TObservationRequest> selIter(selectedList);
    TObservationRequest *selItem;
    selIter.SetToFirst();
    while((selItem = selIter++) != NULL)
        {
        //locate and inspect the target
        TBool observe = ETrue;
        LOG_MSG("Looking through the exe list to mark as observed");
        for(TInt i = 0; i < exeList.Count(); i++)
            {
            if(exeList[i]->Name() == selItem->iTargetName)
                {
                //choose the right action - toggle states
                observe = !exeList[i]->Observed();
                break;
                }
            }

	    LOG_MSG("CResourceList::SetObservedExecutables -> iCoreDumpSession->ObservationRequestL()  Observing:");
        TRAP(err, iCoreDumpSession.ObservationRequestL(selItem->iTargetName, selItem->iTargetOwnerName, observe));
        if (err != KErrNone)
            {
	        LOG_MSG3("CResourceList::SetObservedExecutables - unable to perform action:%d! err:%d\n", observe, err);
            TBuf<23> error;
            RBuf buf;
            buf.CreateL(KMaxFullName);
            error.Format(_L("Error: %d"), err);
            if(observe)
                buf.Append(_L("Unable to attach to "));
            else
                buf.Append(_L("Unable to detach from "));

            buf.AppendFormat(_L("executable:%S"), &selItem->iTargetName);
            CEikonEnv::InfoWinL(buf, error);
            buf.Close();
            }
        }
        CleanupStack::PopAndDestroy(2); //results in calling CleanupProcessList and CleanupSelection

	}

/**
    Cleanup item implementation for thread list in SetObservedThreads() method.
 */
void CResourceList::CleanupThreadList(TAny *aArray)
{
	RThreadPointerList *threadList = static_cast<RThreadPointerList*> (aArray);
    threadList->ResetAndDestroy();
    threadList->Close();
}

void CResourceList::CleanupExecutableList(TAny *aArray)
	{
	RExecutablePointerList *exeList = static_cast<RExecutablePointerList*> (aArray);
	exeList->ResetAndDestroy();
	exeList->Close();
	}

void CResourceList::CleanupPluginList(TAny* aArray)
	{
	RPluginPointerList* list = static_cast<RPluginPointerList*> (aArray);
	list->ResetAndDestroy();
	list->Close();
	}

void CResourceList::CleanupCrashList(TAny* aArray)
	{
	RCrashInfoPointerList* list = static_cast<RCrashInfoPointerList*> (aArray);
	list->ResetAndDestroy();
	list->Close();
	}

void CResourceList::SetObservedThreadsL()
    {
    LOG_MSG("->CResourceList::SetObservedThreads()\n");

	RThreadPointerList threadList;
    TCleanupItem cleanup(CResourceList::CleanupThreadList, (TAny*)&threadList);
    CleanupStack::PushL(cleanup);


   	TRAPD(err, iCoreDumpSession.GetThreadsL(threadList));
    if(err != KErrNone)
        {
        LOG_MSG2("CResourceList::SetObservedProcesses - unable get thread list! err:%d\n", err);
        TBuf<23> error;
        error.Format(_L("Error: %d"), err);
        CEikonEnv::InfoWinL(_L("Unable to get thread list!"), error);
        User::Leave(err);
        }

	TSglQue<TObservationRequest> selectedList(TObservationRequest::iOffset);
    TCleanupItem cleanupSelection(CResourceList::CleanupSelection, (TAny*)&selectedList);
    CleanupStack::PushL(cleanupSelection);
	BuildSelectedListL(selectedList);

	TSglQueIter<TObservationRequest> selIter(selectedList);
    TObservationRequest *selItem;
    selIter.SetToFirst();
    while((selItem = selIter++) != NULL)
        {
        //locate and inspect the target
        TBool observe = ETrue;
        for(TInt i = 0; i < threadList.Count(); i++)
            {
            if(threadList[i]->Name() == selItem->iTargetName)
                {
                //choose the right action - toggle states
                observe = !threadList[i]->Observed();
                break;
                }
            }

	    LOG_MSG("CResourceList::SetObservedThreads -> iCoreDumpSession->ObservationRequestL()\n");
        TRAP(err, iCoreDumpSession.ObservationRequestL(selItem->iTargetName, selItem->iTargetOwnerName, observe));
        if (err != KErrNone)
            {
	        LOG_MSG3("CResourceList::SetObservedThreads - unable to perform action:%d! err:%d\n", observe, err);
            TBuf<23> error;
            RBuf buf;
            buf.CreateL(KMaxFullName);
            error.Format(_L("Error: %d"), err);
            if(observe)
                buf.Append(_L("Unable to attach to "));
            else
                buf.Append(_L("Unable to detach from "));

            buf.AppendFormat(_L("thread:%S"), &selItem->iTargetName);
            CEikonEnv::InfoWinL(buf, error);
            buf.Close();
            }
        }
        CleanupStack::PopAndDestroy(2); //results in calling CleanupThreadList and CleanupSelection
    }

#define CW_Id 25
#define CW_ProcessName 195
#define CW_Medium 90
#define CW_UID 100
#define CW_Bound 100
#define CW_PluginName 350
#define GapWidth 25
#define CW_Short 60
#define CW_ThreadName 380
#define CW_Priority 85
#define CW_ProcessId 95
#define CW_Writeable 90
#define CW_Read 70
#define CW_FileName 500
#define CW_ExeName 195
#define CW_PluginId 25
#define CW_CRASHID	75
#define CW_CRASHTIME 120
#define CW_CRASHPROCESSED 100

void CResourceList::SetUpProcessTableL()
	{
	// deal with the columns
	iListBoxData->SetColumnWidthPixelL( 0, CW_Id); // Id
	iListBoxData->SetColumnWidthPixelL( 1, CW_ProcessName); // Name
	iListBoxData->SetColumnAlignmentL( 1, CGraphicsContext::ELeft);
	iListBoxData->SetColumnWidthPixelL( 2, CW_FileName); //Observed?
	iListBoxData->SetColumnHorizontalGapL( 2, GapWidth);
	iListBoxData->SetColumnAlignmentL( 2, CGraphicsContext::ELeft);

	_LIT(KTitles, "Id\tName\tObserved");
	iListBoxItems->AppendL(KTitles);
	}

void CResourceList::SetUpThreadTableL()
	{
	iListBoxData->SetColumnWidthPixelL( 0, CW_Id);
	iListBoxData->SetColumnWidthPixelL( 1, CW_ThreadName);
	iListBoxData->SetColumnAlignmentL( 1, CGraphicsContext::ELeft);
	iListBoxData->SetColumnWidthPixelL( 2, CW_Priority); //Priority
	iListBoxData->SetColumnAlignmentL( 2, CGraphicsContext::ERight);
	iListBoxData->SetColumnWidthPixelL( 3, CW_ProcessId); //ProcessId
	iListBoxData->SetColumnAlignmentL( 3, CGraphicsContext::ERight);
	iListBoxData->SetColumnWidthPixelL( 4, CW_FileName); //Observed?
	iListBoxData->SetColumnHorizontalGapL( 4, GapWidth);
	iListBoxData->SetColumnAlignmentL( 4, CGraphicsContext::ECenter);

	_LIT(KTitles, "Id\tName\tPriority\tProcess\tObserved");
	iListBoxItems->AppendL(KTitles);
	}

void CResourceList::SetUpPluginTableL()
	{	
	iListBoxData->SetColumnWidthPixelL( 0, CW_PluginName);
	iListBoxData->SetColumnWidthPixelL( 1, CW_Medium); // Uid
	iListBoxData->SetColumnAlignmentL(1, CGraphicsContext::ERight);
	iListBoxData->SetColumnWidthPixelL( 2, CW_Medium + GapWidth); // Type
	iListBoxData->SetColumnAlignmentL(2, CGraphicsContext::ERight);
	iListBoxData->SetColumnHorizontalGapL( 2, GapWidth);
	iListBoxData->SetColumnWidthPixelL( 3, CW_Medium); // Loaded?
	iListBoxData->SetColumnAlignmentL(3, CGraphicsContext::ERight);

	_LIT(KTitles, "Name\tUid\tType\tInstances");
	iListBoxItems->AppendL(KTitles);
	}

void CResourceList::SetUpExecutableTableL()
	{
	iListBoxData->SetColumnWidthPixelL(0, CW_ExeName); //Name of exe
	iListBoxData->SetColumnAlignmentL(0, CGraphicsContext::ELeft);
	iListBoxData->SetColumnWidthPixelL(1, CW_FileName); //Observed?
	iListBoxData->SetColumnHorizontalGapL( 1, GapWidth);
	iListBoxData->SetColumnAlignmentL( 1, CGraphicsContext::ELeft);

	_LIT(KTitles, "Name\tObserved");
	iListBoxItems->AppendL(KTitles);
	}

void CResourceList::SetUpPluginInstanceTableL()
	{
	iListBoxData->SetColumnWidthPixelL( 0, CW_PluginId);
	iListBoxData->SetColumnWidthPixelL( 1, CW_PluginName);
	iListBoxData->SetColumnAlignmentL( 1, CGraphicsContext::ELeft);
	iListBoxData->SetColumnWidthPixelL( 2, CW_UID); // Uid
	iListBoxData->SetColumnAlignmentL(2, CGraphicsContext::ERight);
	iListBoxData->SetColumnWidthPixelL( 3, CW_Medium + GapWidth); // Type
	iListBoxData->SetColumnAlignmentL(3, CGraphicsContext::ERight);
	iListBoxData->SetColumnHorizontalGapL( 3, GapWidth);
	iListBoxData->SetColumnWidthPixelL(4, CW_Bound);
	iListBoxData->SetColumnAlignmentL(4, CGraphicsContext::ERight);

	_LIT(KTitles, "ID\tName\tUid\tType\tBound To");
	iListBoxItems->AppendL(KTitles);
	}

void CResourceList::SetUpCrashListTableL()
	{
	iListBoxData->SetColumnWidthPixelL( 0, CW_CRASHID);
	iListBoxData->SetColumnAlignmentL(0, CGraphicsContext::ELeft);
	
	iListBoxData->SetColumnWidthPixelL(1, CW_CRASHTIME);
	iListBoxData->SetColumnAlignmentL(1, CGraphicsContext::ELeft);
	
	iListBoxData->SetColumnWidthPixelL(2, CW_CRASHPROCESSED); 
	iListBoxData->SetColumnAlignmentL(2, CGraphicsContext::ERight);
	
	_LIT(KTitles, "Crash ID\tTime Of Crash\tProcessed");
	iListBoxItems->AppendL(KTitles);
	}

void CResourceList::SetUpListL()
	{
	iListBoxData->SetColumnWidthPixelL( 0, 300);
	}

void CResourceList::UpdateListDataL()
	{
	LOG_MSG("->[UI]CResourceList::UpdateListDataL()");
	switch (iObjectsViewed)
		{
		case EObjectProcess:
			{
			EmptyList();
			SetUpProcessTableL();
			break;
			}
		case EObjectThread:
			{
			EmptyList();
			SetUpThreadTableL();
			break;
			}
		case EObjectPlugin:
			{
			EmptyList();
			SetUpPluginTableL();
			break;
			}
		case EObjectPluginInstance:
			{
			EmptyList();
			SetUpPluginInstanceTableL();
			break;
			}
		case EObjectExecutable:
			{
			EmptyList();
			SetUpExecutableTableL();
			break;
			}
		case EObjectCrashList:
			{
			EmptyList();
			SetUpCrashListTableL();
			break;
			}
		default:
			{
			break;
			}
		}
	GetResourceListL();
	}

void CResourceList::GetResourceListL()
	{	
	switch (iObjectsViewed)
		{
		case EObjectProcess:
			{
			GetProcessListL();
			break;
			}
		case EObjectThread:
			{
			GetThreadListL();
			break;
			}
		case EObjectPlugin:
			{
			GetPluginListL();
			break;
			}
		case EObjectPluginInstance:
			{
			GetPluginInstancesListL();
			break;
			}
		case EObjectExecutable:
			{
			GetExecutableListL();
			break;
			}
		case EObjectCrashList:
			{
			GetCrashListL();
			break;
			}
		default:
			{
			break;
			}
		}
	
	iListBox->HandleItemAdditionL();
	iListBox->DrawDeferred();	
	
	}

/**
 * Gets the crash list from the CDS server
 * @leave one of the OS wide error codes
 */
void CResourceList::GetCrashListL()
	{
	LOG_MSG("->[UI]CResourceList::GetCrashListL()");
	
	//Get the list and for each crash, add it to the UI
	iCoreDumpSession.ListCrashesInFlashL(iCrashList);	
	TInt crashCnt = iCrashList.Count() -1;
	
	for(; crashCnt >=0; crashCnt--)
		{
		AddRowL(*(iCrashList[crashCnt]));		
		}
	
	UpdateResourceCount(iCrashList.Count());
	}

void CResourceList::GetPluginInstancesListL()
	{
	//Get loaded plugin instances
	LOG_MSG("->[UI]CResourceList::GetPluginInstancesListL()");
    RPluginPointerList pluginPtrList;
	TCleanupItem cleanup(CResourceList::CleanupPluginList, (TAny*)&pluginPtrList);
    CleanupStack::PushL(cleanup);
    TInt totalPluginCount = 0;
    
    LOG_MSG("\tCResourceList::GetPluginInstancesListL()  iCoreDumpSession.GetFormattersL(pluginPtrList);");
    iCoreDumpSession.GetFormattersL(pluginPtrList);
    LOG_MSG2("\tCResourceList::GetPluginInstancesListL()  Found %d formatters", pluginPtrList.Count());
    
    for(TInt cnt = 0; cnt < pluginPtrList.Count(); cnt ++)
    	{    	
    	iPluginInstanceData.iIdString.Format(_L("%d"), cnt);
    	iPluginInstanceData.iName = pluginPtrList[cnt]->Name();    	
    	iPluginInstanceData.iUid.Format(_L("0x%X"), pluginPtrList[cnt]->Uid());
    	iPluginInstanceData.iType = _L("Formatter");
    	if(pluginPtrList[cnt]->Pair() == KMaxTUint32)
    		{
    		iPluginInstanceData.iBoundTo.Format(_L("-"));
    		}
    	else
    		{
    		iPluginInstanceData.iBoundTo.Format(_L("%u"), pluginPtrList[cnt]->Pair());
    		}
    	
    	AddRowL(iPluginInstanceData);
    	
    	totalPluginCount++;
    	}

    CleanupStack::PopAndDestroy(&pluginPtrList);     
    
    //Now we get the writer list                
    LOG_MSG("\tCResourceList::GetPluginInstancesListL()  iCoreDumpSession.GetWritersL(pluginPtrList);");
    iCoreDumpSession.GetWritersL(pluginPtrList);
    LOG_MSG2("\tCResourceList::GetPluginInstancesListL()  Found %d writers", pluginPtrList.Count());
    CleanupStack::PushL(cleanup);
    
    for(TInt cnt = 0; cnt < pluginPtrList.Count(); cnt++)
    	{
    	iPluginInstanceData.iIdString.Format(_L("%d"), cnt);
    	iPluginInstanceData.iName = pluginPtrList[cnt]->Name();    	
    	iPluginInstanceData.iUid.Format(_L("0x%X"), pluginPtrList[cnt]->Uid());
    	iPluginInstanceData.iType = _L("Writer");
    	iPluginInstanceData.iBoundTo.Format(_L(""));
    	AddRowL(iPluginInstanceData);
    	
    	totalPluginCount++;
    	}
    

    UpdateResourceCount(totalPluginCount);
    CleanupStack::PopAndDestroy(&pluginPtrList);
	}

void CResourceList::GetExecutableListL()
	{
	//Get executables
	RExecutablePointerList exes;
	LOG_MSG("CResourceList::GetExecutableListL()  iCoreDumpSession.GetExecutablesL(exes)");
	TRAPD(err, iCoreDumpSession.GetExecutablesL(exes));
	if( err != KErrNone )
		{
		LOG_MSG2("CResourceList::GetExecutableListL() - unable to get exe list! err:%d", err);
        TBuf<23> error;
		error.Format(_L("Error: %d"), err);
		CEikonEnv::InfoWinL(_L("Unable to get executable list!"), error);
        User::Leave(err);
		}

	for( TInt i = 0; i < exes.Count(); i++)
		{
		iExeData.iExeName.Copy(exes[i]->Name());
		iExeData.iObserved.Copy(KSpaces);

		if(exes[i]->Observed())
            {
            iExeData.iObserved.Copy(KYes);
            }

		AddRowL(iExeData);
		}
	UpdateResourceCount(exes.Count());
	exes.ResetAndDestroy();
	}

void CResourceList::GetProcessListL()
	{
	// get data from CDS
	RProcessPointerList processes;
	LOG_MSG(" CResourceList::GetProcessList iCoreDumpSession->GetProcessesL(processes)" );
	TRAPD(err, iCoreDumpSession.GetProcessesL(processes));
	if (err != KErrNone)
		{
        LOG_MSG2("CResourceList::GetProcessListL() - unable to get process list! err:%d", err);
        TBuf<23> error;
		error.Format(_L("Error: %d"), err);
		CEikonEnv::InfoWinL(_L("Unable to get process list!"), error);
        User::Leave(err);
		}

	for (TInt i = 0; i < processes.Count(); i++)
		{
		iProcessData.iIdString.Format(KFormatDec, processes[i]->Id());
		iProcessData.iProcessName.Copy(processes[i]->Name());

		iProcessData.iObserved.Copy(KSpaces);
        if(processes[i]->Observed())
            {
            iProcessData.iObserved.Copy(KYes);
            }

		AddRowL(iProcessData);
		}
	UpdateResourceCount(processes.Count());
	processes.ResetAndDestroy();
	}

void CResourceList::GetThreadListL(void)
	{
	// get data from CDS
	RThreadPointerList threads;
	LOG_MSG( "CResourceList::GetThreadList iCoreDumpSession->GetThreadsL(&threads);" );
	TRAPD(err, iCoreDumpSession.GetThreadsL(threads));
	if (err != KErrNone)
		{
        LOG_MSG2("CResourceList::GetThreadListL() - unable to get thread list! err:%d", err);
        TBuf<23> error;
		error.Format(_L("Error: %d"), err);
		CEikonEnv::InfoWinL(_L("Unable to get thread list!"), error);
        User::Leave(err);
		}

	LOG_MSG2( "CResourceList::GetThreadList threads->Count() = %d", threads.Count());
	for (TInt i = 0; i<threads.Count(); i++)
		{

		iThreadData.iIdString.Format(KFormatDec, threads[i]->Id());

		// Name
		iThreadData.iThreadName.Copy(threads[i]->Name());

		// Priority
		iThreadData.iPriorityString.Format(KFormatDec, threads[i]->Priority());

		// Owner Process
		iThreadData.iOwnerIdString.Format(KFormatDec, threads[i]->ProcessId());

        // Observed
		iThreadData.iObserved.Copy(KSpaces);
        if(threads[i]->Observed())
            {
            iThreadData.iObserved.Copy(KYes);
            }        

		AddRowL(iThreadData);
		}
	UpdateResourceCount(threads.Count());
	threads.ResetAndDestroy();
	}

void CResourceList::CleanupEComArray(TAny* aArray)
	{
	(static_cast<RImplInfoPtrArray*> (aArray))->ResetAndDestroy();
	(static_cast<RImplInfoPtrArray*> (aArray))->Close();
	}

void CResourceList::GetPluginListL()
	{
	// get data from CDS
	RPluginList data;
	CleanupClosePushL(data);
	LOG_MSG( "CResourceList::GetPluginList iCoreDumpSession->GetFormatters(&data)" );
	TRAPD(err, iCoreDumpSession.GetPluginListL(data));
	if (err != KErrNone)
		{
		TBuf<23> error;
		error.Format(_L("Error: %d"), err);
		CEikonEnv::InfoWinL(_L("Could not get plugin list!"), error);
        User::Leave(err);
		}

	LOG_MSG2( "CResourceList::GetPluginList No of plugins is/are: %d", data.Count() );

	TUIPluginData uiData;

	for (TInt i = 0; i < data.Count(); i++)
		{
		uiData.iName = data[i].iName;
		TUid uid = data[i].iUid;
		uiData.iUid.Format(_L("0x%x"), uid);
		if (data[i].iType == TPluginRequest::EFormatter)
			{
			uiData.iType = _L("Formatter");
			uiData.iInstances.Format(_L("%d"), GetNumberFormattersL(uid));
			}
		else if (data[i].iType == TPluginRequest::EWriter)
			{
			uiData.iType = _L("Writer");
			uiData.iInstances.Format(_L("%d"), GetNumberWritersL(uid));
			}
		
		AddRowL(uiData);
		}
	UpdateResourceCount(data.Count());
	CleanupStack::PopAndDestroy(&data);
	}

void CResourceList::AddRowL(const TUIProcessData& aProcessData)
	{
	// The actual item text format depends on the listbox type, see tables with listbox types
	iRowItem.Format(_L("%S\t%S\t%S"), &aProcessData.iIdString,
	&aProcessData.iProcessName,
	&aProcessData.iObserved);

	iListBoxItems->AppendL(iRowItem);
	}

void CResourceList::AddRowL(const TUIPluginData& aPluginData)
	{
	// The actual item text format depends on the listbox type, see tables with listbox types
	iRowItem.Format(_L("%S\t%S\t%S\t%S"), &aPluginData.iName,
	&aPluginData.iUid,
	&aPluginData.iType,
	&aPluginData.iInstances);

	iListBoxItems->AppendL(iRowItem);
	}

void CResourceList::AddRowL(const TUIPluginInstanceData& aPluginInstance)
	{
	iRowItem.Format(_L("%S\t%S\t%S\t%S\t%S"), 
			&aPluginInstance.iIdString,
			&aPluginInstance.iName,
			&aPluginInstance.iUid,
			&aPluginInstance.iType,
			&aPluginInstance.iBoundTo);
	
	iListBoxItems->AppendL(iRowItem);
	}

void CResourceList::AddRowL(const TUIThreadData& aThreadData)
	{
	// The actual item text format depends on the listbox type, see tables with listbox types
	iRowItem.Format(_L("%S\t%S\t%S\t%S\t%S"), &aThreadData.iIdString,
	&aThreadData.iThreadName,
	&aThreadData.iPriorityString,
	&aThreadData.iOwnerIdString,
	&aThreadData.iObserved);

	iListBoxItems->AppendL(iRowItem);
	}

void CResourceList::AddRowL(const TListData& aListData)
	{
	// The actual item text format depends on the listbox type, see tables with listbox types
	iRowItem.Format(_L("%S"), &aListData.iName);

	iListBoxItems->AppendL(iRowItem);
	}

void CResourceList::AddRowL(const TUIExecutableData& aExeData)
	{
	// The actual item text format depends on the listbox type, see tables with listbox types
	iRowItem.Format(_L("%S\t%S"), &aExeData.iExeName, &aExeData.iObserved);

	iListBoxItems->AppendL(iRowItem);
	}

void CResourceList::AddRowL(const TCrashInfo& aCrashInfo)
	{	
	iRowItem.Format(_L("%d\t%lu"), aCrashInfo.iCrashId, aCrashInfo.iTime);
	
	iListBoxItems->AppendL(iRowItem);
	}

TInt CResourceList::GetNumberFormattersL(TUid& aUid)
	{
	TInt numFormatters = 0;
	
	//Get loaded plugin instances
	LOG_MSG("->[UI]CResourceList::GetNumberFormatters(TUid& aUid)");
    RPluginPointerList pluginPtrList;
	TCleanupItem cleanup(CResourceList::CleanupPluginList, (TAny*)&pluginPtrList);
    CleanupStack::PushL(cleanup);
    
    LOG_MSG("\tCResourceList::GetNumberFormatters(TUid& aUid)  iCoreDumpSession.GetFormattersL(pluginPtrList);");
    LOG_MSG2("\tLooking for Formatters of UID of %x", aUid.iUid);
    iCoreDumpSession.GetFormattersL(pluginPtrList);
    
    for(TInt cnt = 0; cnt < pluginPtrList.Count(); cnt ++)
    	{ 
    	TUid uid = TUid::Uid(pluginPtrList[cnt]->Uid());
    	if(uid == aUid)
    		{
    		numFormatters++;
    		}
    	}

    CleanupStack::PopAndDestroy(&pluginPtrList); 	
	
	return numFormatters;
	}

TInt CResourceList::GetNumberWritersL(TUid& aUid)
	{
	TInt numWriters = 0;
	
	//Get loaded plugin instances
	LOG_MSG("->[UI]CResourceList::GetNumberWritersL(TUid& aUid)");
    RPluginPointerList pluginPtrList;
	TCleanupItem cleanup(CResourceList::CleanupPluginList, (TAny*)&pluginPtrList);
    CleanupStack::PushL(cleanup);
    
    LOG_MSG("\tCResourceList::GetNumberWritersL(TUid& aUid)  iCoreDumpSession.GetFormattersL(pluginPtrList);");
    LOG_MSG2("\tLooking for Writers of UID of %x", aUid.iUid);
    iCoreDumpSession.GetWritersL(pluginPtrList);
    
    for(TInt cnt = 0; cnt < pluginPtrList.Count(); cnt ++)
    	{ 
    	TUid uid = TUid::Uid(pluginPtrList[cnt]->Uid());
    	if(uid == aUid)
    		{
    		numWriters++;
    		}
    	}

    CleanupStack::PopAndDestroy(&pluginPtrList); 
	
	return numWriters;
	}

