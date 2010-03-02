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
// Core dump progress observer active object class definition.
//



/**
 @file
 @internalTechnology
 @released
*/

#ifndef CORE_DUMP_UI_NOTIFIER_H
#define CORE_DUMP_UI_NOTIFIER_H

#include "coredumpui.h"

class CCrashNotifier : public CActive
{

public:
    static CCrashNotifier *NewL(CResourceView &aResourceView);
    ~CCrashNotifier();

protected:
    CCrashNotifier(CResourceView &aResourceView);
    void ConstructL();
    void RunL();
    void DoCancel();

private:
    RProperty iProperty; //property that stores crash progress
    CResourceView &iResourceView; //ui view to be notified about the progress
};
#endif //CORE_DUMP_UI_NOTIFIER_H
