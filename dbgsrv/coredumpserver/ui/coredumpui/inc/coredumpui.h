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

#ifndef __CoreDumpUI_H
#define __CoreDumpUI_H

/**
@file
@internalTechnology
@released
*/

#include "coredumpui.hrh"
//for storing resource data
#include "coredumpuidatastructs.h"

#include <coeccntx.h>

#include <eikenv.h>
#include <eikappui.h>
#include <eikapp.h>
#include <eikdoc.h>
#include <eikmenup.h>

#include "coredumpuidialogs.h"

#include <coredumpui.rsg>
#include <eikchkbx.h>
#include <eikchlst.h>
#include <eiklbbut.h>

#include <eiklabel.h>
#include <eikcmbut.h>

#include <eikhopbt.h>

#include <eikclb.h> //for column list box
#include <eikclbd.h> // for CColumnListBoxData

// for CEikButtonGroupContainer
#include <eikbtgpc.h>

#include <eikon.hrh>


#include <e32base.h>
#include <e32cons.h>
#include <e32hal.h>
#include <e32std.h>

#include <eikedwin.h>
#include <smut.h>
#include <f32file.h>
#include <f32dbg.h>
#include <e32property.h> //KMaxPropertySize

#include <coredumpinterface.h>


_LIT(KFormatDec, "%d");
_LIT(KFormatDec64, "%ld");
_LIT(KFormatUDec,"%u");
_LIT(KFormatHex, "%x");
_LIT(KAsterisk, "*");
_LIT(KFillName, "ResMan");
_LIT(KYes, "Yes");
_LIT(KSpaces, "   ");


enum TObjectsViewed
	{
	EObjectNone,
	EObjectProcess,
	EObjectThread,
	EObjectPlugin,
	EObjectPluginInstance,
	EObjectExecutable,
	EObjectCrashList	
	};

// Forward declarations

class CResourceAppUi;
class CResourceAppView;
class CResourceListView;
class CResourceView;
class CCrashNotifier;
//
//
// CResourceApplication
//
//

class CResourceApplication :  public CEikApplication
	{
private: 
	           // Inherited from class CApaApplication
	CApaDocument* CreateDocumentL();
	TUid AppDllUid() const;
	};
//
//
// CResourceDocument
//
//
class CResourceDocument : public CEikDocument
	{
public:
	static CResourceDocument* NewL(CEikApplication& aApp);
	CResourceDocument(CEikApplication& aApp);
	void ConstructL();
private: 
	           // Inherited from CEikDocument
	CEikAppUi* CreateAppUiL();
	};
//
//
// CResourceAppUi
//
//
class CResourceAppUi : public CEikAppUi
    {
public:
    void ConstructL();
	~CResourceAppUi();

private:
	// Inherited from class CEikAppUi
	void HandleCommandL(TInt aCommand);
	
private:
	void SetCDSL(void);
	void SetSCM();
	void CancelCrashL(void);

private:
	CResourceView* iListBoxView;
    CCrashNotifier *iCrashNotifier;
	};


#define KTargetString (250)

class TObservationRequest
{
public:

	/** Name of the debug target */
	TBuf<KTargetString>		iTargetName;

	/**
	If the target is a thread, the name of its process
	If a process, can be left empty 
	*/
	TBuf<KTargetString>		iTargetOwnerName;

	static const TInt iOffset;
	
private:
	TSglQueLink iLink;
};

//
//
// CResourceList
//
//
class CResourceList: public CCoeControl, public MCoeControlObserver
{
public:
	void ConstructL(TInt aTablePosition);
	~CResourceList();
    RCoreDumpSession &CoreDumpInterface() { return iCoreDumpSession; }
	
protected:
	void UpdateResourceCount(TUint aNumberOfResource) {iResourceCount = aNumberOfResource;}
	void UpdateListDataL();
	
	CCoeControl* GetListBox() const;
	void SetObservedL();	
	void ConfigurePluginL();
	void LoadPluginL();
	void UnloadPluginL();
	void BindPluginsL();
	void ProcessCrashL();
	void DeleteCrashLogL();
	void DeleteEntireCrashPartitionL();

private:
	void SetUpProcessTableL();
	void SetUpThreadTableL();
	void SetUpPluginTableL();
	void SetUpExecutableTableL();
	void SetUpPluginInstanceTableL();
	void SetUpCrashListTableL();
	void SetUpListL(); //uses single column tables
	
	void GetResourceListL();
	void AddRowL(const TUIProcessData& aProcessData);
	void AddRowL(const TUIThreadData& aThreadData);
	void AddRowL(const TUIPluginData& aChunkData);
	void AddRowL(const TListData& aListData);
	void AddRowL(const TUIExecutableData& aExeData);
	void AddRowL(const TUIPluginInstanceData& aPluginInstance);
	void AddRowL(const TCrashInfo& aCrashInfo);

	void GetThreadListL(void);
	void GetProcessListL(void);
	void GetPluginListL(void);
	void GetExecutableListL(void);
	void GetPluginInstancesListL(void);
	void GetCrashListL(void);

	void EmptyList();
	static void CleanupEComArray(TAny* aArray);
    static void CleanupSelection(TAny *aList);
    static void CleanupProcessList(TAny *aArray);
    static void CleanupThreadList(TAny *aArray);
    static void CleanupExecutableList(TAny *aArray);
    static void CleanupPluginList(TAny *aArray);
    static void CleanupCrashList(TAny* aArray);
    
	void BuildSelectedListL(TSglQue<TObservationRequest> &aSelectedList);
	void SetObservedProcessesL();	
	void SetObservedThreadsL();	
	void SetObservedExecutablesL();
	
	TCrashInfo* GetCrashInfoL(const TInt aCrashId);
	
	TInt GetNumberFormattersL(TUid& aUid);
	TInt GetNumberWritersL(TUid& aUid);
	
	RArray<TInt>* GetMarkedCrashIDsFromUIL();
		
	CEikColumnListBox* iListBox;
	CTextListBoxModel* iModel;
	MDesCArray* iTextArray;
	CDesCArray* iListBoxItems;
	CColumnListBoxData* iListBoxData;
    RCoreDumpSession iCoreDumpSession;
    RBuf iRowItem;
	
protected:
	TUIProcessData iProcessData;
	TUIThreadData iThreadData;
	TUIPluginData iPluginData;
	TUIPluginInstanceData iPluginInstanceData;
	TUIExecutableData iExeData;
	
	TInt iBorder;
	TUint iResourceCount; // Count of number of items in table
    TBuf<RProperty::KMaxPropertySize> iCrashProgress; //crash progress state
	TObjectsViewed iObjectsViewed; // What view is showing
	
private:	
	RCrashInfoPointerList iCrashList;  //what crashes we have stored in the log
};
 
//
//
// CResourceView
//
//
class CResourceView : public CResourceList
    {
public:
	static CResourceView* NewL(const TRect& aRect, CResourceAppUi* aAppUi);
	
	~CResourceView();
	
	void SetCurrentObjects(TObjectsViewed aViewed)
		{iObjectsViewed = aViewed;};

	void UpdateViewL();
    void UpdateCrashProgressL();

    TDes& CrashProgress();

private:
	// Private constructor to force NewL
	CResourceView();
	void ConstructL(const TRect& aRect, CResourceAppUi* aAppUi);
	void InvalidateView(void) const;
	void UpdateTitleL(void);

protected:

	// Inherited from CCoeControl
	virtual TInt CountComponentControls() const;				
	virtual CCoeControl* ComponentControl(TInt aIndex) const;
	virtual void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);

private:
	
	// Inherited from CCoeControl
	void Draw(const TRect& /*aRect*/) const;

private:
	
	CResourceAppUi* iAppUi;

	// State Data
	TBuf<RProperty::KMaxPropertySize> iTitle; // Title above table
	// Controls
	CEikCommandButton* iCommandButton;
	CEikCommandButton* iObserveButton;
	CEikCommandButton* iLoadButton;
    };

#endif

